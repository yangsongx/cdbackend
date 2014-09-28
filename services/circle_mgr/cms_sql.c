/**
 * The SQL code for CMS
 *
 * --------------------------------------------
 *  SQL Info for token:
 *
 *  DATABASES - ucen
 *  TABLE     - USERS | USER_DETAILS
 *
 *
 */
#include <string.h>
#include <my_global.h>
#include <mysql.h>
#include "cds_public.h"

#include "cms_def.h"

/* All database related info put here... */
#define CIRCLE_TBL  "CircleTest"

#define LOCK_DB      LOCK_CDS
#define UNLOCK_DB    UNLOCK_CDS

/* DB lock protection */
static pthread_mutex_t cms_sql_mutex;

/* set a defult SQL server value if no config found */
struct sql_server_info sql_cfg = {
    "127.0.0.1",
    0,
    "root",
    "njcm",
    "ucen"
};

/**
 *
 *
 */
int fill_up_the_response(MYSQL_RES *mres, struct circle_sync_data *response)
{
    MYSQL_ROW  sql_row;
    int        index = 0;
    int        count = 0;

    response->sync_result = CDS_OK;

    /*
       All matched Circle Sync data is in:
       --------------------------------------------
        URL1;UR2;URL3;...'\0'
       --------------------------------------------

     */
    while((sql_row = mysql_fetch_row(mres)))
    {
        count ++;

        strcat(response->sync_payload + index, sql_row[0]);
        index += strlen(sql_row[0]);
        strcat(response->sync_payload + index, ";");
        index += 1;
    }

    /* the len exclude leading 2 ints... */
    response->sync_len = (offsetof(struct circle_sync_data, sync_payload)
            + strlen(response->sync_payload))
            - offsetof(struct circle_sync_data, sync_uid);

    LOG("Payload len = %ld, so the sync_len would be %d\n", strlen(response->sync_payload),
            response->sync_len);
    //response->

    return 0;
}

/**
 * Pick up SYNC data from DB
 *
 * return a CDS_ERR_XXX enum if failure meet, otherwise, CDS_OK(0), and if
 * CDS_OK, the sync result data is stored in @response
 */
int fetch_sync_data(MYSQL *ms, struct circle_sync_req *req, struct circle_sync_data *response)
{
    int  ret = CDS_OK;
    char sqlcmd[CDS_MAX_SQL];
    int  circle_id = 0;


    /* TODO - we still need a step to do a mpping like this:

       ==============================
               UID                    -------> Which Circle? (CircleID)
       ==============================

     */

    if(!strcmp(req->sreq_uid, MAGIC_UID))
    {
        LOG("MAGIC UID found , just creating fake response\n");

        circle_id = 3;/* FIXME - use 3 temp, for MAGIC testing */
    }

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT data FROM %s WHERE circle_id=%d;",
            CIRCLE_TBL, circle_id);

    LOCK_DB(cms_sql_mutex);

    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_DB(cms_sql_mutex);

        ERR("**failed execute the SQL command:%s\n",
                mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    mresult = mysql_store_result(ms);
    /* FIXME - MySQL doc said lock/unock between _query and _store_result */

    UNLOCK_DB(cms_sql_mutex);

    if(mresult)
    {
        /* TODO handle all queryed SQL data here! */

        strcpy(response->sync_uid, req->sreq_uid);
        fill_up_the_response(mresult, response);
        LOG("the payload data is:%s\n", response->sync_payload);
    }
    else
    {
        ret = CDS_ERR_SQL_NORECORD_FOUND;
    }

    return ret;
}



/**
 * A wrapper for MUTEX protection
 */
int INIT_DB_MUTEX()
{
    return  pthread_mutex_init(&cms_sql_mutex, NULL);
}

int CLEAN_DB_MUTEX()
{
    return pthread_mutex_destroy(&cms_sql_mutex);
}

/**
 *@server: the MySQL server config info
 */
MYSQL *GET_CMSSQL(struct sql_server_info *server)
{
    MYSQL *msql;

    LOG("mysql info:MYSQL-%s\n", mysql_get_client_info());
    msql = mysql_init(NULL);
    if(msql != NULL)
    {
        if(!mysql_real_connect(msql, server->ssi_server_ip,
                    server->ssi_user_name,
                    server->ssi_user_password,
                    server->ssi_database,
                    server->ssi_server_port,
                    NULL,
                    0))
        {
            ERR("**failed connect MySQL:%s\n",
                mysql_error(msql));
            mysql_close(msql);
            msql = NULL;
        }
        else
        {
            INFO("\tConnecting to MySQL Database....[OK]\n");
        }
    }

    return msql;
}

void FREE_CMSSQL(MYSQL *m)
{
    if(m != NULL)
        mysql_close(m);
}
