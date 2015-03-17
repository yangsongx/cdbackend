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
    MYSQL *ms = m_pCfg->m_Sql;
    MYSQL_RES *mresult;

    snprintf(sqlcmd, sizeof(sqlcmd), "SELECT %s FROM %s",
            col_name, db_tbl);

    pthread_mutex_lock(&m_pCfg->m_SqlMutex);
    if(mysql_query(ms, sqlcmd))
    {
        pthread_mutex_unlock(&m_pCfg->m_SqlMutex);
        ERR("failed execute the ping sql cmd:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    mresult = mysql_store_result(ms);
    pthread_mutex_unlock(&m_pCfg->m_SqlMutex);

    if(mresult)
    {
        // call fetch a row cmd here, do NOTING else
        mysql_fetch_row(mresult);
        ret = 0;

        mysql_free_result(mresult);
    }

    return ret;
}

int Operation::set_mem_value()
{
    return 0;
}

/**
 * Using CAS to set a mem value.
 *
 */
int Operation::set_mem_value_with_cas(const char *key, uint64_t cas)
{
    return 0;
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
 * Wrapper util for visiting SQL, can auto-reconnect to SQL
 * after idle timeout.
 */
int Operation::sql_cmd(const char *cmd, cb_sqlfunc sql_cb)
{
    int ret;

    ret = execute_sql(cmd, sql_cb);
    if(ret == CDS_ERR_SQL_DISCONNECTED)
    {
        // try reconnect the SQL, as it probably
        // disconnected after idle timeout
        INFO("SQL disconnected, try reconnect...\n");
        if(m_pCfg->reconnect_sql() == 0)
        {
            // do it again
            ret = execute_sql(cmd, sql_cb);
        }
    }

    return ret;
}

/**
 * Inner util for calling MySQL
 *
 * If SQL server gone when max-idle exceed, will return CDS_ERR_SQL_DISCONNECTED, caller need try re-connect and call this util again!
 */
int Operation::execute_sql(const char *cmd, cb_sqlfunc sql_cb)
{
    MYSQL_RES *mresult;
    MYSQL     *ms = m_pCfg->m_Sql;

    pthread_mutex_lock(&m_pCfg->m_SqlMutex);
    if(mysql_query(ms, cmd))
    {
        pthread_mutex_unlock(&m_pCfg->m_SqlMutex);
        ERR("**failed execute SQL cmd:%s\n", mysql_error(ms));

        if(mysql_errno(ms) == CR_SERVER_GONE_ERROR)
        {
            return CDS_ERR_SQL_DISCONNECTED;
        }

        return CDS_ERR_SQL_EXECUTE_FAILED;
    }
    mresult = mysql_store_result(ms);
    pthread_mutex_unlock(&m_pCfg->m_SqlMutex);

    if(sql_cb != NULL)
    {
        //drop into caller's callback function
        sql_cb(mresult);
    }

    // release un-used resource here.
    if(mresult != NULL)
    {
        mysql_free_result(mresult);
    }

    return CDS_OK;
}
