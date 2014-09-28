/**
 * NetDisk Service(NDS) code
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __USE_XOPEN
#include <time.h>

#ifdef CHECK_MEM_LEAK
#include <mcheck.h>
#endif

#include "cds_public.h"
#include "nds_def.h"

#define COMPOSE_RESP(_err)   *(int *)resp = (_err); \
                             *len = sizeof(int); \
                             ret = (_err); \
                             goto failed

static MYSQL *nds_sql = NULL;


/**
 * call back function, handle Token
 *
 * return error values if sth wrong.
 */
int cms_handler(int size, void *req, int *len, void *resp)
{
    int ret = CDS_OK;

    if(size >= 1024)
    {
        /* Note - actually, len checking already done at .SO side... */
        ERR("Exceed limit(%d > %d), don't handle this request\n",
                size, 1024);
        COMPOSE_RESP(CDS_ERR_REQ_TOOLONG);
    }

    LOG("got %d size (%s) from client\n",
                    size, (char *)req);

failed:
    return ret;
}

int main(int argc, char **argv)
{
    struct addition_config cfg;
#ifdef CHECK_MEM_LEAK
    mtrace();
#endif

    /* try get the SQL server info */
    parse_config_file("/etc/cds_cfg.xml", &sql_cfg);
    LOG("SQL Server IP : %s Port : %d, user name : %s, DB name : %s\n",
            sql_cfg.ssi_server_ip, sql_cfg.ssi_server_port,
            sql_cfg.ssi_user_name, sql_cfg.ssi_database);
    nds_sql = GET_CMSSQL(&sql_cfg);
    if(nds_sql == NULL)
    {
        ERR("failed connecting to the SQL server!\n");
        return -1;
    }

    if(INIT_DB_MUTEX() != 0)
    {
        FREE_CMSSQL(nds_sql);
        ERR("Failed create IPC objs:%d\n", errno);
        return -2;
    }

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = cms_handler;
    cds_init(&cfg, argc, argv);

    CLEAN_DB_MUTEX();
    FREE_CMSSQL(nds_sql);

    return 0;
}
