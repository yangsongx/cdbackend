// DEMO how to handle token process.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __USE_XOPEN
#include <time.h>

#ifdef CHECK_MEM_LEAK
#include <mcheck.h>
#endif

#include "cds_public.h"
#include "token_def.h"

#define PING_ALIVE_TESTING(x) (strcmp((x), "13911111111") == 0)

#define COMPOSE_RESP(_err)   *(int *)resp = (_err); \
                             *len = sizeof(int); \
                             ret = (_err); \
                             goto failed

static MYSQL *glb_msql = NULL;
pthread_mutex_t sql_mutex;

/* set a defult value if no config found */
struct sql_server_info server_cfg = {
    "127.0.0.1",
    0,
    "root",
    "njcm",
    "ucen"
};

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
int is_valid_user(struct token_string_info *s, struct cipher_token_string_info *c)
{
    char lastlogin_line[32]; /* string format as 'YYYY-MM-DD HH:MM:SS' */
    char expire_dead_line[32]; /* string format as 'YYYY-MM-DD HH:MM:SS' */
    int  rc = 0;

    rc = fetch_expire_from_db(glb_msql, c->csi_expire,
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
            rc = fetch_expire_from_db(glb_msql, c->csi_expire,
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
                rc = fetch_expire_from_db(glb_msql, c->csi_expire,
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

    return 1;
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
    extract_token_string_data(req, &ts);
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
    if(PING_ALIVE_TESTING(ts.tsi_userid))
    {
        /* Need ping a SQL to avoid that server broken after 8 hours. */
        if(ping_sql(glb_msql) == 0)
        {
            INFO("+===> A PING Payload, mark it as CDS_OK as MySQL access is also OK\n");
            COMPOSE_RESP(CDS_OK);
        }
        else
        {
            ERR("---> A PING Payload, but MySQL access failed!\n");
            COMPOSE_RESP(CDS_ERR_SQL_EXECUTE_FAILED);
        }
    }
#endif
    if(!is_valid_user(&ts, &cs))
    {
        ERR("User token expired!\n");
        COMPOSE_RESP(CDS_ERR_USER_TOKEN_EXPIRED);
    }

    /* If comes here, it is OK case */
    *(int *)resp = CDS_OK;
    *len = sizeof(int);

failed:

    if(plain_data != NULL)
        free(plain_data);

    return ret;
}

int main(int argc, char **argv)
{
    struct addition_config cfg;
#ifdef CHECK_MEM_LEAK
    mtrace();
#endif

    parse_config_file("/etc/cds_cfg.xml", &server_cfg);
    LOG("(%d)SQL Server IP : %s Port : %d, user name : %s, DB name : %s\n",
            BUILD_NUMBER, /* automatically generated via make */
            server_cfg.ssi_server_ip, server_cfg.ssi_server_port,
            server_cfg.ssi_user_name, server_cfg.ssi_database);

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

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = token_handler;
    cds_init(&cfg, argc, argv);

    pthread_mutex_destroy(&sql_mutex);
    FREE_MYSQL(glb_msql);

    return 0;
}
