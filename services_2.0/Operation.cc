#include "Operation.h"

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

