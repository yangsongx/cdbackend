// DEMO how to handle token process.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#ifndef __cplusplus
#define __USE_XOPEN
#endif

#include <time.h>

#ifdef CHECK_MEM_LEAK
#include <mcheck.h>
#endif

/* libmemcached library */
#include <libmemcached/memcached.h>

#include "cds_public.h"
#include "token_def.h"


#define TAUTH_CFGFILE "cds_cfg.xml"
#define TAUTH_LOGFILE "tauth.log"


#define PING_ALIVE_TESTING(x) (strcmp((x), "13911111111") == 0)

#define COMPOSE_RESP(_err)   *(int *)resp = (_err); \
                             *len = sizeof(int); \
                             ret = (_err); \
                             goto failed

static MYSQL *glb_msql = NULL;
static memcached_st *glb_memc = NULL;

pthread_mutex_t sql_mutex;

/* set a defult value if no config found */
struct sql_server_info server_cfg = {
    "127.0.0.1",
    0,
    "root",
    "njcm",
    "ucen"
};

static char glb_memc_cfg[128];

extern int get_token_info(memcached_st *memc, MYSQL *ms, struct token_string_info *info, struct token_data_wrapper *result);


/**
 *
 * @cfg_file : specify the config XML file
 *
 */
int init_tauth_config(const char *cfg_file)
{
    char buffer[128];
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
            get_node_via_xpath("/config/token_auth/@logfile", ctx,
                    buffer, sizeof(buffer));
            if(strncmp(buffer, "false", 5))
            {
                _log_file = fopen(CONFIG_PREFIX "/tauth.log", "a+");
            }

            get_node_via_xpath("/config/token_auth/sqlserver/ip", ctx,
                    server_cfg.ssi_server_ip, 32);
            get_node_via_xpath("/config/token_auth/sqlserver/user", ctx,
                    server_cfg.ssi_user_name, 32);
            get_node_via_xpath("/config/token_auth/sqlserver/password", ctx,
                    server_cfg.ssi_user_password, 32);


            get_node_via_xpath("/config/token_auth/memcache/ip", ctx,
                    buffer, sizeof(buffer));

            sprintf(glb_memc_cfg, "--SERVER=%s:", buffer);

            get_node_via_xpath("/config/token_auth/memcache/port", ctx,
                    buffer, sizeof(buffer));
            strcat(glb_memc_cfg, buffer);

            xmlXPathFreeContext(ctx);
        }

        xmlFreeDoc(doc);

    }

    return 0;
}

int extract_token_string_data(char *token_data, struct token_string_info *info)
{
    char *uid,*login,*aid,*rsa;
    char *saveptr;

    if((uid = strtok_r(token_data, "#", &saveptr)) != NULL)
    {
        info->tsi_userid = uid;

        if((aid = strtok_r(NULL, "#", &saveptr)) != NULL)
        {
            info->tsi_appid = aid;
            if((login = strtok_r(NULL, "#", &saveptr)) != NULL)
            {
                info->tsi_login = atoi(login);
                if((rsa = strtok_r(NULL, "#", &saveptr)) != NULL)
                {
                    info->tsi_rsastr = rsa;
                }
            }
        }
    }
    return 0;
}

int extract_cipher_token_string_data(char *token_data, struct cipher_token_string_info *info)
{
    char *uid,*expire,*aid;
    char *saveptr;

    if((uid = strtok_r(token_data, "#", &saveptr)) != NULL)
    {
        info->csi_userid = uid;
        if((expire = strtok_r(NULL, "#", &saveptr)) != NULL)
        {
            info->csi_expire = expire;
            if((aid = strtok_r(NULL, "#", &saveptr)) != NULL)
            {
                info->csi_appid = aid;

                /* NOTE - as next tails are reserved ,
                   we don't use them currently.

                   so further strotok() is not necessary
                 */
            }
        }
    }
    return 0;
}

/**
 * Compare basic info(such as user id, app id)
 * return 1 means basic info match, otherwise 0
 */
int match_basic_info(struct token_string_info *s, struct cipher_token_string_info *c)
{
    int ret = 1;
    /* Note - for XMPP case,
       the user name would like 13002561122@caredear.talk.com,
       so we just need compare the number before '@'
     */
    if(strncmp(s->tsi_userid, c->csi_userid, strlen(c->csi_userid)) || strcmp(s->tsi_appid, c->csi_appid))
    {
        ret = 0;
    }
    return ret;
}

/**
 * Check user's login time and his expired time.
 *
 * return 1 means user not expired(may auto-update DB if needed), otherwise return 0.
 */
int is_valid_user(const char *user_id)
{
    char lastlogin_line[32]; /* string format as 'YYYY-MM-DD HH:MM:SS' */
    char expire_dead_line[32]; /* string format as 'YYYY-MM-DD HH:MM:SS' */
    int  rc = 0;

    // FIXME 2015-1-21 don't use this DB flag yet....
#if 0
    if(BYPASS_DB_COOKIE_ON)
    {
        INFO("PASS DB ON - return is_valid with 1 directly");
        return 1;
    }
#endif
#if 0 // a22301  below code is not needed any more
    rc = fetch_expire_from_db(glb_msql, NULL, c->csi_expire,
            lastlogin_line, sizeof(lastlogin_line),
            expire_dead_line, sizeof(expire_dead_line));

    if(rc == -1)
    {
        /* failure on SQL operation, set as invalid user */
        return 0;
    }
    else if (rc == -2)
    {
        ERR("we need re-connect MySQL...\n");
        glb_msql = GET_MYSQL(&server_cfg);
        if(glb_msql != NULL)
        {
            rc = fetch_expire_from_db(glb_msql, NULL, c->csi_expire,
                    lastlogin_line, sizeof(lastlogin_line),
                    expire_dead_line, sizeof(expire_dead_line));
        }
        else
        {
            ERR("still failed re-connect to MySQL.\n");
            return 0;
        }
    }
    else if (rc == -3)
    {
        char db_user_token[128];
        if(get_token_from_db(glb_msql, s->tsi_userid, db_user_token, sizeof(db_user_token)) == 0)
        {
            if(!strcmp(db_user_token, c->csi_expire))
            {
                // try again
                rc = fetch_expire_from_db(glb_msql, NULL, c->csi_expire,
                        lastlogin_line, sizeof(lastlogin_line),
                        expire_dead_line, sizeof(expire_dead_line));
            }
            else
            {
                ERR("Obviously, request TOKEN != TOKEN_IN_DB\n");
                return 0;
            }
        }
    }

    if(rc != 0)
    {
        ERR("what a pity, failed even we connect to MySQL again!\n");
        return 0;
    }
    LOG("DB expiration string:%s\n", expire_dead_line);

    struct tm  t, l;
    memset(&t, 0x00, sizeof(t));
    memset(&l, 0x00, sizeof(l));
    if(strptime(expire_dead_line, "%Y-%m-%d %H:%M:%S", &t) == NULL)
    {
        ERR("failed call strptime on expire:%d\n", errno);
        return 0;
    }
    if(strptime(lastlogin_line, "%Y-%m-%d %H:%M:%S", &l) == NULL)
    {
        ERR("failed call strptime on lastlogin, data:%s, error:%d\n",
                lastlogin_line, errno);
        return 0;
    }

    if(PING_ALIVE_TESTING(s->tsi_userid))
    {
        INFO("===> A PING Payload, return valid to tell it I am alive\n");
        return 1;
    }

    time_t cal = mktime(&t);
    if(cal == (time_t) -1)
    {
        ERR("failed call mktime:%d\n", errno);
        return 0;
    }
    LOG("DB expiration integer:%d\n", (int)cal);

    time_t last = mktime(&l);
    if(last == (time_t) -1)
    {
        ERR("failed call mktime for last login:%d\n", errno);
        return 0;
    }

    if(s->tsi_login > cal)
    {
        // Token is too old, expried!
        return 0;
    }

    update_token_time_to_db(glb_msql, s, c, last, cal);
#endif
    return 1;
}

/**
 * @auth_data : a parsed data request from Web
 *
 * return CDS_XXX constant, which CDS_OK is succesful.
 */
int do_token_authentication(struct token_string_info *auth_data)
{
    int ret = 0;
    //struct token_data_wrapper base_data;

    ret = get_token_info(glb_memc, glb_msql, auth_data, NULL /*&base_data*/);
    if(ret == CDS_ERR_SQL_DISCONNECTED)
    {
        ERR("MySQL disconnected, try re-conn it one more time...\n");
        glb_msql = GET_MYSQL(&server_cfg);
        if(glb_msql != NULL)
        {
            INFO("Reconnecting to the MySQL successfully, do TAUTH again!\n");
            ret = get_token_info(glb_memc, glb_msql, auth_data, NULL);
        }
        else
        {
            INFO("A pity for reconn MySQL,I consider this req as invalid user!\n");
        }
    }
    else
    {
        // TODO how to handle other cases?
        // Currrently, we don't do anything for this.
    }

    return ret;
}

/**
 * call back function, handle Token
 *
 * return error values if sth wrong.
 */
int token_handler(int size, void *req, int *len, void *resp)
{
    int ret = CDS_OK;
    char *plain_data = NULL;
#ifndef CONFIG_SINGLE_APPTOKEN
    char *key;
#endif
    struct cipher_token_string_info cs;
    memset(&cs, 0x00, sizeof(cs));

    if(size >= 1024)
    {
        /* Note - actually, len checking already done at .SO side... */
        ERR("Exceed limit(%d > %d), don't handle this request\n",
                size, 1024);
        COMPOSE_RESP(CDS_ERR_REQ_TOOLONG);
    }

    LOG("got %d size (%s) from client\n",
                    size, (char *)req);

    struct token_string_info ts;
    extract_token_string_data((char *)req, &ts);
    LOG("Phase-I extraction is complete:\n");
    LOG("user ID = %s, login time = %d, app ID = %s, rsa str = %s\n",
       ts.tsi_userid, ts.tsi_login, ts.tsi_appid, ts.tsi_rsastr);

#ifndef CONFIG_SINGLE_APPTOKEN
    key = match_aes_key(ts.tsi_appid);
    if(key == NULL)
    {
        ERR("failed find the correct decrypt key!\n");
        COMPOSE_RESP(CDS_ERR_REQ_INVALID);
    }

    /* next, try decrypt the ecrypt string data */
    plain_data = decrypt_token_string_with_aes(ts.tsi_rsastr, key);
    if(!plain_data)
    {
        ERR("got a NULL pointer data, ignore this handling\n");
        COMPOSE_RESP(CDS_ERR_REQ_INVALID);
    }

    /*the decrypted toke is composed as:
      ------------------------------------------------------------------
      | USER ID  | Expire time | APP ID | auth ID | Auth Level |  Info  |
      ------------------------------------------------------------------
     */
    extract_cipher_token_string_data(plain_data, &cs);

    if(!match_basic_info(&ts, &cs))
    {
        ERR("Un-match info, reject do further job!\n");
        COMPOSE_RESP(CDS_ERR_UMATCH_USER_INFO);
    }
#else
    /* for single APP TOKEN case */
    cs.csi_expire = ts.tsi_rsastr;
#endif

    /// 2015 new code change begin
    //
    ret = do_token_authentication(&ts);
    //
    // 2015 new code end

    /* If comes here, it is OK case */
    *(int *)resp = ret;
    *len = sizeof(int);

failed:

    if(plain_data != NULL)
        free(plain_data);

    return ret;
}

/**
 * Handler the ping alive request
 *
 */
int ping_tauth_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret = 0;

    ret = keep_tauth_db_connected(glb_msql);
    LOG("tauth ping result=%d\n", ret);

    *len_resp = 4;
    *(int *)resp = ret;

    return ret;
}


int main(int argc, char **argv)
{
    struct addition_config cfg;
#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/tauth.memleak", 1);
    mtrace();
#endif

    char buffer[512];
    memset(buffer, 0x00, sizeof(buffer));

    /**
     * [Note] 2015-1-27
     *  Below getcwd() will take no effect if started
     *  by deamon tools(because it will be chdir("/"))
     */
    if(getcwd(buffer, sizeof(buffer)) != NULL)
    {
        if(buffer[strlen(buffer) - 1 ] == '/')
        {

            strcat(buffer, TAUTH_CFGFILE);
        }
        else
        {
            strcat(buffer, "/");
            strcat(buffer, TAUTH_CFGFILE);
        }
    }
    else
    {
        strcpy(buffer, TAUTH_CFGFILE);
    }

    init_tauth_config("/opt/tokenauth/cds_cfg.xml");

    /* Whole Program's Log show up~~ */
    INFO("(%d)== TAUTH Program==\nSQL Server IP : %s Port : %d, user name : %s, DB name : %s\n",
            BUILD_NUMBER, /* automatically generated via make */
            server_cfg.ssi_server_ip, server_cfg.ssi_server_port,
            server_cfg.ssi_user_name, server_cfg.ssi_database);
    INFO("memcached config:%s\n", glb_memc_cfg);

    // 2015-1-21 try add memcached support

    glb_memc = memcached(glb_memc_cfg, strlen(glb_memc_cfg));
    if(!glb_memc)
    {
        ERR("Warning, failed connect to the memcached!\n");
        // FIXME when acceess mem simultaneous, IPC wanted or not wanted?
    }
    // end of 2015-1-21 memcached support

    if(!BYPASS_DB_COOKIE_ON)
    {
        /* do real SQL connection */
        glb_msql = GET_MYSQL(&server_cfg);
        if(glb_msql == NULL)
        {
            ERR("failed connect to MySQL!\n");
            return -1;
        }

        if(pthread_mutex_init(&sql_mutex, NULL) != 0)
        {
            FREE_MYSQL(glb_msql);
            ERR("Failed create IPC objs:%d\n", errno);
            return -2;
        }
    }

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = token_handler;
    cfg.ping_handler = ping_tauth_handler;

    /* 2015-1-23 - we use ASCII leading len type
     * for back compatability
     */
    cfg.ac_lentype = LEN_TYPE_ASCII;
    cds_init(&cfg, argc, argv);

    pthread_mutex_destroy(&sql_mutex);

    if(!BYPASS_DB_COOKIE_ON)
    {
        FREE_MYSQL(glb_msql);
    }

    if(glb_memc != NULL)
    {
        memcached_free(glb_memc);
    }

    return 0;
}
