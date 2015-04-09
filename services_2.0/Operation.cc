/**
 * A Base Class for all daemon operation, which usually manipulate the
 * DB or memcached.
 *
 * \history
 * [2015-04-10] Let SQL command support extra parameter, which would avoid using static variable in caller
 * [2015-04-01] Try fix the SQL time-out reconnecting BUG
 */
#include "Operation.h"
#include <libmemcached/memcached.h>

#include <stdio.h>
#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>

Operation::Operation()
{
}

Operation::Operation(Config *c)
{
    m_pCfg = c;
}

/**
 * If caller didn't use Operation(c) format, can call this to
 * set inner config obj.
 *
 */
int Operation::set_conf(Config *c)
{
    m_pCfg = c;
    return 0;
}

/**
 * the handler for ping alive request.
 *
 * @db_tbl : the SQL DB table name
 * @col_name : the column name in @db_tbl table.
 *
 */
int Operation::keep_alive(const char *db_tbl, const char *col_name/* = "id" */)
{
    int ret = -1;
    char sqlcmd[128];

    snprintf(sqlcmd, sizeof(sqlcmd), "SELECT %s FROM %s",
            col_name, db_tbl);

    ret = sql_cmd(sqlcmd, NULL, NULL);

    return ret;
}

/**
 * Directly set a key mem value
 *
 */
int Operation::set_mem_value(const char *key, const char *value, uint32_t flag /* = 0 */, time_t expiration /* = 0 */)
{
    memcached_return_t rc;

    rc = memcached_set(m_pCfg->m_Memc,
            key, strlen(key),
            value, strlen(value),
            expiration, flag);

    return rc;
}

/**
 * Using CAS to set a mem value.
 *
 */
int Operation::set_mem_value_with_cas(const char *key, const char *value, uint32_t flag /* = 0 */, time_t expiration /* = 0 */ )
{
    char  *val = NULL;
    size_t val_len;
    memcached_return_t rc;
    uint64_t cas_value;

    val = memcached_get_by_key(m_pCfg->m_Memc,
            NULL,
            0,
            key,
            strlen(key),
            &val_len,
            0, &rc);
    if(val != NULL)
    {
        if(rc == MEMCACHED_SUCCESS)
        {
            cas_value = (m_pCfg->m_Memc->result).item_cas;
            LOG("%s -> CAS %ld\n", key, cas_value);

            rc = memcached_cas(m_pCfg->m_Memc,
                    key, strlen(key),
                    value, strlen(value),
                    expiration,
                    flag,
                    cas_value);
        }

        free(val);
    }
    else if(rc == MEMCACHED_NOTFOUND)
    {
        INFO("CAS found it is new item, so directly set mem here\n");
        rc = (memcached_return_t)set_mem_value(key, value, flag, expiration);
    }

    /* NOTE - If the item already changed by other person,
     * libmemcached API would return MEMCACHED_DATA_EXISTS,
     *
     * caller need re-call this API again.
     */

    return rc;
}

/**
 * TODO I think this API should use memcached_return_t, not int, for
 * return value
 *
 */
int Operation::rm_mem_value(const char *key)
{
    memcached_return_t rc;

    rc = memcached_delete(m_pCfg->m_Memc, key, strlen(key), 0/* expiration */);

    return (rc == MEMCACHED_SUCCESS ? 0 : 1);
}

/**
 *
 *
 * return NULL if sth wrong, otherwise, callee need take responsibility to
 * free this returned pointer value data.
 */
char *Operation::get_mem_value(const char *key, size_t *p_valen, uint64_t *p_cas)
{
    memcached_return_t rc;
    char *val;

    val = memcached_get_by_key(m_pCfg->m_Memc, NULL, 0,
            key, strlen(key), p_valen, 0, &rc);
    if(val != NULL)
    {
        if(rc != MEMCACHED_SUCCESS)
        {
            // for failure case even got non-NULL val
            ERR("though mem val != NULL, rc still failed:%d\n", rc);
            free(val);
            val = NULL;
        }
        else
        {
            if(p_cas != NULL)
            {
                // still need get the CAS
                *p_cas = (m_pCfg->m_Memc->result).item_cas;
                LOG("%s -> CAS %ld\n", key, *p_cas);
            }
        }
    }

    /* DO NOT FORGET to free this if don't need it */
    return val;
}

/**
 * execute the SQL via trasaction(which can rollback if sth wrong)
 *
 */
int Operation::sql_cmd_via_transaction(int argc, char **argv, cb_sqlfunc sql_cb)
{
    int i;
    MYSQL *ms = m_pCfg->m_Sql;
    MYSQL_RES *mresult;

    pthread_mutex_lock(&m_pCfg->m_SqlMutex);
    if(mysql_query(ms, "begin"))
    {
        pthread_mutex_unlock(&m_pCfg->m_SqlMutex);
        ERR("failed do the begin transaction:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    for(i = 0; i < argc; i++)
    {
        if(mysql_query(ms, argv[i]))
        {
            ERR("sth wrong in DB trasaction(%s), will rollback...\n",
                    mysql_error(ms));
            goto need_rollback;
        }

        mresult = mysql_store_result(ms);
        if(mresult)
            mysql_free_result(mresult);
    }

    INFO("Now, update all tables in one commit...\n");
    if(mysql_commit(ms) == 0)
    {
        INFO("[OK]\n");
    }
    else
    {
        ERR("Failed commit:%s\n", mysql_error(ms));
        goto need_rollback;
    }

    pthread_mutex_unlock(&m_pCfg->m_SqlMutex);

    return CDS_OK;

need_rollback:
    pthread_mutex_unlock(&m_pCfg->m_SqlMutex);
    INFO("will rollback as sth wrong...");
    if(mysql_rollback(ms) == 0)
    {
        INFO("[OK]\n");
    }
    else
    {
        INFO("[failed:%s]\n", mysql_error(ms));
    }

    return CDS_ERR_SQL_EXECUTE_FAILED;
}

/**
 * Wrapper util for visiting SQL, can auto-reconnect to SQL
 * after idle timeout.
 */
int Operation::sql_cmd(const char *cmd, cb_sqlfunc sql_cb, void *p_extra)
{
    int ret;

    // BY default, use the default m_Sql data member
    ret = sql_cmd_with_specify_server(&(m_pCfg->m_Sql), cmd, sql_cb, p_extra);

    return ret;
}

/**
 * Another API, which require caller specify @pms, which is a pointer to pointer.
 * We need this as this MySQL * variable probably can been overwritten if SQL idle
 * for 8 hours re-connection.
 *
 */
int Operation::sql_cmd_with_specify_server(MYSQL **pms, const char *cmd, cb_sqlfunc sql_cb, void *p_extra)
{
    int ret;

    /* SQL error case protection */
    if(pms == NULL)
    {
        ERR("MySQL connection not existed at all!\n");
        return CDS_ERR_SQL_DISCONNECTED;
    }

    ret = execute_sql(pms, cmd, sql_cb, p_extra);
    if(ret == CDS_ERR_SQL_DISCONNECTED)
    {
        // try reconnect the SQL, as it probably
        // disconnected after idle timeout
        INFO("SQL disconnected, try reconnect...\n");
        *pms = m_pCfg->reconnect_sql(*pms,
                m_pCfg->m_strSqlIP,
                m_pCfg->m_strSqlUserName,
                m_pCfg->m_strSqlUserPassword);

        if(*pms != NULL)
        {
            // do it again
            ret = execute_sql(pms, cmd, sql_cb, p_extra);
        }
        else
        {
            ERR("What a pity, failed reconnecting to the SQL\n");
        }
    }

    return ret;
}

/**
 * Inner util for calling MySQL
 *
 * If SQL server gone when max-idle exceed, will return CDS_ERR_SQL_DISCONNECTED, caller need try re-connect and call this util again!
 */
int Operation::execute_sql(MYSQL **pms, const char *cmd, cb_sqlfunc sql_cb, void *p_extra)
{
    MYSQL_RES *mresult;

    pthread_mutex_lock(&m_pCfg->m_SqlMutex);
    if(mysql_query(*pms, cmd))
    {
        pthread_mutex_unlock(&m_pCfg->m_SqlMutex);
        ERR("**failed execute SQL cmd:(%d)%s\n",
                mysql_errno(*pms), mysql_error(*pms));

        if(mysql_errno(*pms) == CR_SERVER_GONE_ERROR)
        {
            return CDS_ERR_SQL_DISCONNECTED;
        }

        return CDS_ERR_SQL_EXECUTE_FAILED;
    }
    mresult = mysql_store_result(*pms);
    pthread_mutex_unlock(&m_pCfg->m_SqlMutex);

    if(sql_cb != NULL)
    {
        //drop into caller's callback function
        sql_cb(mresult, p_extra);
    }

    // release un-used resource here.
    if(mresult != NULL)
    {
        mysql_free_result(mresult);
    }

    return CDS_OK;
}
