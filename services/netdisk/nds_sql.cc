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
 *return 1 means exceed quota
 */
int exceed_quota(MYSQL *ms, NetdiskRequest *p_obj)
{
    int exceed = 0;
    int filesize = p_obj->filesize();
    const char *username = p_obj->user().c_str();
    int used;
    char sqlcmd[128];

    //FIXME, currently, quota limit defined in configxml,
    // and updated it into DB,

    // When we compare, choose which one? (DB? or configxml?)
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT USED_SIZE FROM %s WHERE USER_NAME=\'%s\';",
            ND_USER_TBL, username);

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("**Failed get use size:%s, error:%s\n",
                sqlcmd, mysql_error(ms));
        return -1; // ???TODO???
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    UNLOCK_SQL;

    if(mresult)
    {
        row = row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            used = atoi(row[0]);
            LOG("used size=%d, file size=%d, quota=%d\n",
                    used, filesize, qiniu_quota);
            if((used + filesize) >= qiniu_quota)
            {
                // exceed the limit
                exceed = 1;
            }
        }
        else
        {
            ERR("got a blank DB record\n");
        }
    }

    return exceed;
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
            "SELECT %s.ID FROM %s WHERE %s.HASH_KEY=\'%s\';",
            ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, p_obj->md5().c_str());

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

int update_user_uploaded_data(MYSQL *ms, NetdiskRequest *p_obj)
{
    int ret = 0;
    const char *username = p_obj->user().c_str();
    const char *md5 = p_obj->md5().c_str();
    const char *filename = p_obj->filename().c_str();
    const int filesize = p_obj->filesize();

    char sqlcmd[1024];

    time_t t;
    struct tm re;
    char timeformat[24];

    time(&t);
    strftime(timeformat, sizeof(timeformat), "%Y-%m-%d %H:%M:%S", localtime_r(&t, &re));

    // First , updating USERS table
    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET USED_SIZE=USED_SIZE+%d,MODIFY_TIME=\'%s\' WHERE USER_NAME=\'%s\';",
            ND_USER_TBL, filesize, timeformat, username);

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        ERR("**failed update USERS:%s, error:%s\n",
                sqlcmd, mysql_error(ms));
        ret = -1;
    }

    UNLOCK_SQL;

    time(&t);
    strftime(timeformat, sizeof(timeformat), "%Y-%m-%d %H:%M:%S", localtime_r(&t, &re));
    // Next, updating FILES table
    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s (HASH_KEY,SIZE,FILENAME,CREATE_TIME,MODIFY_TIME,OWNER) VALUES "
            "(\'%s\',%d,\'%s\',\'%s\',\'%s\',\'%s\');",
            ND_FILE_TBL, md5, filesize, filename, timeformat, timeformat, username);

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        ERR("**failed update FILE:%s, error:%s\n",
                sqlcmd, mysql_error(ms));
        ret = -1;
    }

    UNLOCK_SQL;

    return ret;
}
