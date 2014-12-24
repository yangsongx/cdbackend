/**
 * The SQL code for NDS
 *
 */
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <my_global.h>
#include <mysql.h>

#include "cds_public.h"
#include "newusr.h"

#define ND_USER_TBL   "netdisk.USERS"
#define ND_FILE_TBL   "netdisk.FILES"

/* DB lock protection */
pthread_mutex_t sql_mutex;

#if 0
/* set a defult SQL server value if no config found */
struct sql_server_info sql_cfg = {
    "127.0.0.1",
    0,
    "root",
    "njcm",
    ""
};

#endif

/**
 * Act like 'set key 0 0 xx\r\nvalue\r\n' in memcached
 *
 */
int set_memcache(struct nus_config *p_cfg, const char *key, const char *value)
{
    int mem_sock;
    struct sockaddr_in addr;
    char buf[256];
    int len = strlen(value);

    mem_sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(mem_sock != -1);

    LOG("memcache ip:%s, port:%d\n", p_cfg->memcach_ip, p_cfg->memcach_port);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(p_cfg->memcach_ip);
    addr.sin_port = htons(p_cfg->memcach_port);

    if(connect(mem_sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        ERR("Failed calling connect() : %d\n", errno);
    }
    else
    {
        // setting.
        snprintf(buf, sizeof(buf),
                "set %s 0 0 %d\r\n",
                key, len);
        write(mem_sock, buf, strlen(buf));

        snprintf(buf, sizeof(buf),
                "%s\r\n", value);
        write(mem_sock, buf, strlen(buf));
        //FIXME will we read?

        len = read(mem_sock, buf, sizeof(buf));
        if(len > 0)
        {
            LOG("<== read=%d bytes\n", len);
            LOG("<-- out from memcached:%s", buf);
            LOG("|| ENDOF MARK\n");
        }

    }

    close(mem_sock);

    return 0;
}
/**
 * updating new user token into DB(including both cache and SQL)
 *
 *
 *
 */
int insert_new_usertoken(const char *diskkey)
{

    return 0;
}

/**
 *
 *@filename : try mapp via file suffix
 *
 *return a FT_XXX type
 */

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
MYSQL *GET_NUSSQL(struct sql_server_info *server)
{
    MYSQL *msql;

    LOG("mysql info:MYSQL-%s\n", mysql_get_client_info());
    msql = mysql_init(NULL);
    if(msql != NULL)
    {
        if(!mysql_real_connect(msql, server->ssi_server_ip,
                    server->ssi_user_name,
                    server->ssi_user_password,
                    "", // we will use db.table.xx syntax in sql cmd
                    0, // 0 by default
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

void FREE_NUSSQL(MYSQL *m)
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

