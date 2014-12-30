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

#define NU_USER_TBL   "ucen.USERS"

/* DB lock protection */
pthread_mutex_t sql_mutex;

/**
 * This is to ensure memcached and DB are both healthy
 *
 * MySQL will auto-disconnect you if you idle > 8 hours
 *
 * return the CDS_XXX error code
 */
int keep_db_connected(MYSQL *ms)
{
    int ret = CDS_OK;
    char sqlcmd[128];
    MYSQL_RES *mresult;

    sprintf(sqlcmd, "SELECT ID FROM %s;", NU_USER_TBL);

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_SQL;
        ERR("failed execute the ping sql cmd:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    mresult = mysql_store_result(ms);
    UNLOCK_SQL;

    if(mresult)
    {
        MYSQL_ROW row;
        row = mysql_fetch_row(mresult);
        //DO NOTHING HERE, we just call a store result code,
        //since MySQL connection timeout is removed when
        //code come here.
    }
    else
    {
        ERR("got a null result for ping SQL cmd\n");
        ret = CDS_ERR_SQL_EXECUTE_FAILED;
    }

    return ret;
}

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
 * check in memcached if user(key) existed or not.
 *
 *return 1 means existed, otherwise 0
 */
static int get_username_in_mem(const char *username)
{
    int existed = 0;
    size_t len = 0;

    if(memc != NULL)
    {
        memcached_return_t rc;
        char *val = memcached_get_by_key(memc, NULL, 0,
                username, strlen(username), &len, 0, &rc);

        if(val != NULL && rc == MEMCACHED_SUCCESS) {
            existed = 1;
            free(val);
        }
    }

    return existed;
}

/**
 * find user name info in DB
 *
 *return 1 means existed, otherwise 0
 */
static int get_username_in_db(MYSQL *ms, const char *username)
{
    int existed = 0;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT username FROM %s WHERE username=\'%s\';",
            NU_USER_TBL, username);

    LOCK_SQL;
    if(mysql_query(ms, sqlcmd))
    {
        // TODO code
    }
    else
    {
        // TODO code
    }

    UNLOCK_SQL;

    return existed;
}
/**
 *
 * reuturn 1 means user already registered before, otherwise return 0.
 */
int is_user_registered(MYSQL *ms, const char *username)
{
    int existed = 0;

    existed = get_username_in_mem(username);

    if(existed == 0)
    {
        // if memcache not found, check it in DB again...
        existed = get_username_in_db(ms, username);
    }

    return existed;
}

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

