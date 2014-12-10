/**
 * The SQL code for NDS
 *
 */

#include <my_global.h>
#include <mysql.h>

#include "cds_public.h"
#include "nds_def.h"

#define ND_USER_TBL   "netdisk.USERS"
#define ND_FILE_TBL   "netdisk.FILES"

/* DB lock protection */
pthread_mutex_t sql_mutex;

/* set a defult SQL server value if no config found */
struct sql_server_info sql_cfg = {
    "127.0.0.1",
    0,
    "root",
    "njcm",
    ""
};

/**
 * A wrapper for MUTEX protection
 */
int INIT_DB_MUTEX()
{
    return  pthread_mutex_init(&sql_mutex, NULL);
}

int CLEAN_DB_MUTEX()
{
    return pthread_mutex_destroy(&sql_mutex);
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

int create_new_user(MYSQL *ms, NetdiskRequest *p_obj)
{
    int ret = -1;
    char sqlcmd[1024];

    time_t t;
    struct tm re;
    char timeformat[24];

    time(&t);
    strftime(timeformat, sizeof(timeformat), "%Y-%m-%d %H:%M:%S", localtime_r(&t, &re));

    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s (%s.USER_NAME,%s.USED_SIZE,%s.USER_QUOTA,%s.CREATE_TIME,%s.MODIFY_TIME) "
            "VALUES (\'%s\',0,%d,\'%s\',\'%s\');",
            ND_USER_TBL, ND_USER_TBL, ND_USER_TBL, ND_USER_TBL, ND_USER_TBL, ND_USER_TBL,
            p_obj->user().c_str(), qiniu_quota, timeformat, timeformat);

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        ERR("** failed execute SQL cmd:%s, error:%s\n",
                sqlcmd, mysql_error(ms));
    }
    else
    {
        INFO("--->new user record saved in DB\n");
        ret = 0;
    }
    UNLOCK_SQL;

    return ret;
}

/**
 *
 *return 1 means the user's file already existed in netdisk
 */
int already_existed(MYSQL *ms, NetdiskRequest *p_obj)
{
    int existed = 0;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT USERS.USER_NAME FROM %s WHERE USERS.USER_NAME=\'%s\';",
            ND_USER_TBL, p_obj->user().c_str());

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("** failed execute SQL cmd:%s, error:%s\n",
                sqlcmd, mysql_error(ms));

        // TODO - how should we handle if MySQL
        // disconnected after 8 hours idle?

        return 0; // for SQL error, how to set this?
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;

    mresult = mysql_store_result(ms);
    UNLOCK_SQL;

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            // Now , try find if user had file on netdisk or NOT
            LOG("TODO CODE~~~~~~~~~~~~~~~~~~\n");
        }
        else
        {
            INFO("A new user, insert the record\n");
            if(create_new_user(ms, p_obj) != 0)
            {
                ERR("*** Failed create the \'%s\' user in DB\n",
                        p_obj->user().c_str());
            }
        }
    }
    else
    {
        ERR("meet a NULL for mysql_store_result\n");
    }

    return existed;
}
