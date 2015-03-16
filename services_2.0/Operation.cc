#include "Operation.h"
#include <libmemcached/memcached.h>

#include <stdio.h>

Operation::Operation()
{
}

Operation::Operation(Config *c)
{
    m_pCfg = c;
}

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
    }

    return ret;
}

int Operation::set_mem_value()
{
    return 0;
}

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
