/**
 * A ping program to make sure token server
 * alive.
 *
 * If there's sth wrong , this program would
 * send out notification mail.
 */
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef DEBUG
#include <mcheck.h>
#endif

#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>

#include "cds_public.h"

#define MAXRETRY 3

static MYSQL *msql = NULL;
static int already_reconn = 0; // a flag, which let's reconnecting SQL

struct sql_server_info sql_cfg;
pthread_mutex_t sql_mutex;

MYSQL *GET_MYSQL(struct sql_server_info *server)
{
    MYSQL *msql;

    LOG("mysql info:MYSQL-%s\n", mysql_get_client_info());
    msql = mysql_init(NULL);
    if(msql != NULL)
    {
        if(!mysql_real_connect(msql, server->ssi_server_ip,
                    server->ssi_user_name,
                    server->ssi_user_password,
                    "",//server->ssi_database,
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
            LOG("\tConnecting to MySQL Database....[OK]\n");
        }
    }

    return msql;
}

void FREE_MYSQL(MYSQL *m)
{
    if(m != NULL)
        mysql_close(m);
}

void cleanup_res(MYSQL *ms, pthread_mutex_t *m)
{
    LOG("...Free unused resources...\n");
    FREE_MYSQL(ms);
    pthread_mutex_destroy(m);
}

int read_config(const char *cfg_file)
{
    int ret = -1;
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    if(access(cfg_file, F_OK) != 0)
    {
        ERR("\'%s\' not existed!\n", cfg_file);
        return -1;
    }

    doc = xmlParseFile(cfg_file);
    if(doc != NULL)
    {
        ctx = xmlXPathNewContext(doc);
        if(ctx != NULL)
        {
            get_node_via_xpath("/config/sips_account/sqlserver/ip", ctx,
                    sql_cfg.ssi_server_ip, 32);

            get_node_via_xpath("/config/sips_account/sqlserver/user", ctx,
                    sql_cfg.ssi_user_name, 32);

            get_node_via_xpath("/config/sips_account/sqlserver/password", ctx,
                    sql_cfg.ssi_user_password, 32);
            // FIXME , I don't parse database name here, as we will
            // use database.dbtable in SQL command.

            xmlXPathFreeContext(ctx);
            ret = 0;
        }

        xmlFreeDoc(doc);

        LOG("SQL Server IP : %s Port : %d, user name : %s\n",
                sql_cfg.ssi_server_ip, sql_cfg.ssi_server_port,
                sql_cfg.ssi_user_name);
    }

    msql = GET_MYSQL(&sql_cfg);
    if(msql == NULL)
    {
        ERR("failed connecting to the SQL server!\n");
        return -1;
    }

    return ret;
}

int get_user_token(MYSQL *ms, const char *username, char *token_data)
{
    int ret = CDS_OK;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT USERS.username,USER_DETAILS.USER_TOKEN FROM "
            "ucen.USERS,ucen.USER_DETAILS WHERE USERS.ID=USER_DETAILS.USER_ID "
            "AND USERS.username=\'%s\';",
            username);

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("**failed query sql cmd:%s, error:%s\n",
                sqlcmd, mysql_error(ms));

        if(mysql_errno(ms) == CR_SERVER_GONE_ERROR)
        {
            if(already_reconn++ > MAXRETRY)
            {
                ERR("Now, we retry exceed max(%d), give up\n", already_reconn);
                ret = CDS_ERR_SQL_DISCONNECTED;
            }
            else
            {
                ERR("Wow, found server lost connection, need reconnecting...\n");
                FREE_MYSQL(ms);

                msql = GET_MYSQL(&sql_cfg);
                if(msql != NULL)
                {
                    // recursive calling
                    get_user_token(msql,username,token_data);
                }
                else
                {
                    ERR("can't connect to SQL, abort\n");
                    ret = CDS_ERR_SQL_DISCONNECTED;
                }
            }
        }
        else
        {
            ret = CDS_ERR_SQL_EXECUTE_FAILED;
        }
    }
    else
    {

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    UNLOCK_SQL;

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            strcpy(token_data + 4, row[1]);
            LOG("%s == > %s\n", username, (token_data + 4));
        }
        else
        {
            // didn't find the user token in DB
            ret = CDS_ERR_SQL_NORECORD_FOUND;
        }
    }
    else
    {
        ERR("empty result, should NEVER happen\n");
        ret = CDS_GENERIC_ERROR;
    }

    }

    memcpy(token_data, &ret, 4);

    return ret;
}

int sips_handler(int size, void *req, int *len, void *resp)
{
    int ret = CDS_OK;

    ret = get_user_token(msql, req, resp);

    *len = 128;

    return ret;
}


/**
 * main entry, the argc/argv config is higher
 * than config file at ~/.xxxx
 *
 */
int main(int argc, char **argv)
{
    struct addition_config cfg;
#ifdef DEBUG
    setenv("MALLOC_TRACE", "/tmp/sips.memleak", 1);
    mtrace();
#endif

    if(read_config("/etc/cds_cfg.xml") != 0)
    {
        ERR("Failed read config XML file!\n");
        return -1;
    }

    if(pthread_mutex_init(&sql_mutex, NULL) != 0)
    {
        ERR("failed init the mutex!\n");
        goto failed;
    }

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = sips_handler;
    cfg.ping_handler = NULL;
    cfg.ac_lentype = LEN_TYPE_BIN;
    cds_init(&cfg, argc, argv);

failed:
    cleanup_res(msql, &sql_mutex);

    return 0;
}
