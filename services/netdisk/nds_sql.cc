/**
 * The SQL code for NDS
 *
 */

#include <my_global.h>
#include <mysql.h>

#include <qiniu/base.h>
#include <qiniu/io.h>
#include <qiniu/rs.h>

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

// declearations...
extern int update_existed_file_db(MYSQL *ms, NetdiskRequest *p_obj);
extern int update_user_uploaded_data(MYSQL *ms, NetdiskRequest *p_obj);

extern const char *qiniu_bucket; // defined at nds_main.cc

/**
 * Util to delete Qiniu's file
 *
 *@diskkey : the qiniu disk key(MD5SUM for this case)
 *
 */
int rm_qiniu_disk_file(const char *diskkey)
{
    Qiniu_Client theqn;
    Qiniu_Client_InitMacAuth(&theqn, 1024, NULL);

    Qiniu_Error err = Qiniu_RS_Delete(&theqn, qiniu_bucket, diskkey);

    if (err.code != 200)
    {
        //http status not OK
        ERR("*** failed delete this file:%s\n", err.message);
    }
    else
    {
        INFO("[From QiNiu] (%s) key deleted there.\n", diskkey);
    }

    Qiniu_Client_Cleanup(&theqn);

    return 0;
}

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
 *
 * OK for return 0, and p_md5 stored the corresponding file's MD5
 */
int mapping_file_md5(MYSQL *ms, NetdiskRequest *p_obj, char *p_md5, int len_md5)
{
    int ret = -1;
    char sqlcmd[256];
    MYSQL_RES *mresult;
    MYSQL_ROW  row;

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT %s.MD5 FROM %s WHERE %s.FILENAME=\'%s\' AND %s.OWNER=\'%s\';",
            ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, p_obj->filename().c_str(),
            ND_FILE_TBL, p_obj->user().c_str());

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("***failed find corresponding MD5:%s\n", mysql_error(ms));
        return -1;
    }

    mresult = mysql_store_result(ms);
    UNLOCK_SQL;

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            ret = 0;
            strncpy(p_md5, row[0], len_md5);
            LOG("\'%s\' ==> MD5:%s\n", p_obj->filename().c_str(), p_md5);
        }
        else
        {
            ret = CDS_ERR_FILE_NOTFOUND;
        }
    }

    return ret;
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
            "SELECT %s.USED_SIZE FROM %s WHERE %s.USER_NAME=\'%s\';",
            ND_USER_TBL, ND_USER_TBL, ND_USER_TBL, username);

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
            "SELECT %s.ID FROM %s WHERE %s.MD5=\'%s\';",
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
            LOG("file \'%s\' already existed\n",  p_obj->filename().c_str());
            ret = CDS_FILE_ALREADY_EXISTED;

            // don't forget add this file into DB as well,
            // as in this case, there won't UPLOADED req later

            //TODO code
            update_existed_file_db(ms, p_obj);
        }
    }
    else
    {
        ERR("meet a NULL for mysql_store_result\n");
    }

    return ret;
}

int update_existed_file_db(MYSQL *ms, NetdiskRequest *p_obj)
{
    char sqlcmd[1024];
    MYSQL_RES *mresult;
    MYSQL_ROW  row;

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT %s.ID FROM %s WHERE %s.MD5=\'%s\' AND %s.FILENAME=\'%s\';",
            ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, p_obj->md5().c_str(),
            ND_FILE_TBL, p_obj->filename().c_str());

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
            // a new record, insert it
            update_user_uploaded_data(ms, p_obj);
        }
    }
    else
    {
        ERR("meet a NULL for  mysql_store_result\n");
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    return 0;
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
    MYSQL_RES *mresult;

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

    mresult = mysql_store_result(ms);
    UNLOCK_SQL;


    type = mapping_file_type(filename);
    time(&t);
    strftime(timeformat, sizeof(timeformat), "%Y-%m-%d %H:%M:%S", localtime_r(&t, &re));


    // Next, updating FILES table
    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s (%s.MD5,%s.SIZE,%s.FILENAME,%s.CREATE_TIME,%s.MODIFY_TIME,%s.TYPE,%s.OWNER) VALUES "
            "(\'%s\',%d,\'%s\',\'%s\',\'%s\',%d,\'%s\');",
            ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL,
            md5, filesize, filename, timeformat, timeformat, type, username);

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("**failed update FILE tbl:%s, error:%s\n",
                sqlcmd, mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    mresult = mysql_store_result(ms);
    UNLOCK_SQL;

    return ret;
}

/**
 * This is delete user file from DB, we implemented this
 * like Unix's unlink behavior.
 *
 * return CDS_OK(0) for successful case
 */
int remove_file_from_db(MYSQL *ms, NetdiskRequest *p_obj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    int filecount = 0;
    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    char md5[34]; // 34 is enough for store MD5SUM...

    ret = mapping_file_md5(ms, p_obj, md5, sizeof(md5));
    if(ret != CDS_OK)
    {
        return ret;
    }

    snprintf(sqlcmd, sizeof(sqlcmd),
            "DELETE FROM %s WHERE %s.MD5=\'%s\' AND %s.OWNER=\'%s\';",
            ND_FILE_TBL, ND_FILE_TBL, md5,
            ND_FILE_TBL,p_obj->user().c_str());

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("*** failed delete db with specified md5/user,error:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    mresult = mysql_store_result(ms);
    UNLOCK_SQL;

    if(!mresult)
    {
        ERR("meet a NULL for  mysql_store_result\n");
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    // next ,check if we need really delete the physical file...
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT count(ID) FROM %s WHERE %s.MD5=\'%s\';",
            ND_FILE_TBL, ND_FILE_TBL, md5);

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("*** failed search file with specified md5,error:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    mresult = mysql_store_result(ms);
    UNLOCK_SQL;

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            filecount = atoi(row[0]);
            LOG("still %d files with the same MD5\n", filecount);
            if(filecount == 0)
            {
                // need really remote the file...
                rm_qiniu_disk_file(md5);
            }
        }
    }
    else
    {
        ERR("meet a NULL for  mysql_store_result\n");
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    return CDS_OK;
}

/**
 *
 *return -1 for failure, 0 for OK.
 */
int get_netdisk_key(MYSQL *ms, NetdiskRequest *p_obj, char *p_result)
{
    int ret = -1;
    char sqlcmd[1024];

    p_result[0] = '\0';

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT %s.MD5 FROM %s WHERE %s.OWNER=\'%s\' AND %s.FILENAME=\'%s\';",
            ND_FILE_TBL, ND_FILE_TBL, ND_FILE_TBL,
            p_obj->user().c_str(), ND_FILE_TBL, p_obj->filename().c_str()
            );

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("failed execute the md5-finding SQL cmd.\n");
        return ret;
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
            ret = 0;
            /* FIXME - md5sum is 32-len strings
               strncpy() won't add '\0' when row[0]==32
             */
            strncpy(p_result, row[0], 32);
            p_result[32] = '\0';
        }
        else
        {
            ERR("Didn't find this file(%s) in DB\n", p_obj->filename().c_str());
        }
    }
    else
    {
        ERR("got NULL on mysql_store_result...\n");
    }

    return ret;
}
