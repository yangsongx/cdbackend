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
 *
 *@filename : try mapp via file suffix
 *
 *return a FT_XXX type
 */
int mapping_file_type(const char *filename)
{
    int type = FT_DOC;
    int len = strlen(filename);
    char *suffix;

    if(filename[len - 1] == '.')
    {
        // file ending with dot(i.e, 'xxx.')
        return type;
    }

    while(filename[--len] != '.' && len > 0) ;

    if(len != 0)
    {
        suffix = (char *) &filename[len + 1]; // + 1 to surpass '.' char
        LOG("%s ===> %s\n", filename, suffix);
        if(!strncmp(suffix, "jpg", 3) || !strncmp(suffix, "jpeg", 4)
          || !strncmp(suffix, "png", 3) || !strncmp(suffix, "bmp", 3)
          || !strncmp(suffix, "gif", 3))
        {
            type = FT_IMAGE;
        }
        else if(!strncmp(suffix, "3gp", 3) || !strncmp(suffix, "mp3", 3)
                || !strncmp(suffix, "wma", 3))
        {
            type = FT_MUSIC;
        }
        else if(!strncmp(suffix, "mp4", 3) || !strncmp(suffix, "3gp", 3))
        {
            type = FT_VIDEO;
        }

        // all others be considered as doc
    }

    return type;
}

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
        return CDS_ERR_SQL_EXECUTE_FAILED;
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
 *return CDS_XXX  for the request
 */
int preprocess_upload_req (MYSQL *ms, NetdiskRequest *p_obj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    MYSQL_RES *mresult;
    MYSQL_ROW  row;


    // First, check if user is a new netdisk users...
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT %s.USER_NAME FROM %s WHERE %s.USER_NAME=\'%s\';",
            ND_USER_TBL, ND_USER_TBL, ND_USER_TBL, p_obj->user().c_str());

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("** failed find user:%s,error:%s\n",
                sqlcmd, mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    mresult = mysql_store_result(ms);
    UNLOCK_SQL;

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row == NULL)
        {
            INFO("A new user, insert the record\n");
            if(create_new_user(ms, p_obj) != 0)
            {
                ERR("*** Failed create the \'%s\' user in DB\n",
                        p_obj->user().c_str());

                return CDS_ERR_SQL_EXECUTE_FAILED;
            }
        }
    }
    else
    {
        ERR("meet a NULL for  mysql_store_result\n");
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }


    // Here, we already make sure user existed in the DB
    // so check if file already existed...

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

        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    mresult = mysql_store_result(ms);
    UNLOCK_SQL;

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            // this means file already existed!
            ret = CDS_FILE_ALREADY_EXISTED;
        }
    }
    else
    {
        ERR("meet a NULL for mysql_store_result\n");
    }

    return ret;
}

int update_user_uploaded_data(MYSQL *ms, NetdiskRequest *p_obj)
{
    int ret = 0;
    int type = 0;
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
        UNLOCK_SQL;
        ERR("**failed update USERS:%s, error:%s\n",
                sqlcmd, mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    UNLOCK_SQL;


    type = mapping_file_type(filename);
    time(&t);
    strftime(timeformat, sizeof(timeformat), "%Y-%m-%d %H:%M:%S", localtime_r(&t, &re));


    // Next, updating FILES table
#if 1 // TODO, we probably need remove the TYPE constraint in DB, so here just don't set file type...
    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s (%s.HASH_KEY,%s.SIZE,%s.FILENAME,%s.CREATE_TIME,%s.MODIFY_TIME,%s.OWNER) VALUES "
            "(\'%s\',%d,\'%s\',\'%s\',\'%s\',\'%s\');",
            ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL,
            md5, filesize, filename, timeformat, timeformat, username);
#else
    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s (%s.HASH_KEY,%s.SIZE,%s.FILENAME,%s.CREATE_TIME,%s.MODIFY_TIME,%s.TYPE,%s.OWNER) VALUES "
            "(\'%s\',%d,\'%s\',\'%s\',\'%s\',%d,\'%s\');",
            ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL,
            md5, filesize, filename, timeformat, timeformat, type, username);
#endif

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("**failed update FILE tbl:%s, error:%s\n",
                sqlcmd, mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    UNLOCK_SQL;

    return ret;
}

char *get_netdisk_key(MYSQL *ms, NetdiskRequest *p_obj, char *p_result)
{
    char sqlcmd[1024];

    p_result[0] = '\0';

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT %s.HASH_KEY FROM %s WHERE %s.OWNER=\'%s\';",
            ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL,
            p_obj->user().c_str()
            );

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("failed find the file's MD5 in DB\n");
        return p_result;
    }

    UNLOCK_SQL;
    MYSQL_RES *mresult;
    MYSQL_ROW  row;

    mresult = mysql_store_result(ms);
    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
        }
        else
        {
            ERR("Didn't find this file(%s) in DB\n", p_obj->user().c_str());
        }
    }
    else
    {
        ERR("got NULL on mysql_store_result...\n");
    }

    return p_result;
}
