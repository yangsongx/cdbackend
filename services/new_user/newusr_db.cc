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
#include "newusr.h"

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

#if 0
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
#endif
    return msql;
}

void FREE_CMSSQL(MYSQL *m)
{
    if(m != NULL)
        mysql_close(m);
}


/**
 *
 *return 1 means exceed quota
 */

/**
 *
 *return CDS_XXX  for the request
 */



/**
 * This is delete user file from DB, we implemented this
 * like Unix's unlink behavior.
 *
 * return CDS_OK(0) for successful case
 */

/**
 *
 *return -1 for failure, 0 for OK.
 */
/**
 *
 *return -1 for failure, 0 for OK.
 */

