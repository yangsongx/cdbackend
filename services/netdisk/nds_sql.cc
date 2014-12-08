/**
 * The SQL code for NDS
 *
 */

#include <my_global.h>
#include <mysql.h>

#include "cds_public.h"
#include "nds_def.h"


/* DB lock protection */
static pthread_mutex_t cms_sql_mutex;

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


int already_existed(NetdiskRequest *p_obj)
{
    int existed = 0;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT * FROM;");

    return existed;
}
