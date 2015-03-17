/**
 * Copyright Caredear Inc. (C) 2014 ~ 2015. All rights reserved.
 * Author:  chenyang@caredare.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <time.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>


#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define PADDING RSA_PKCS1_PADDING
#define CAREDEAR_DOMAIN "talk.caredear.com"
#define CAREDEAR_DOMAIN_DEBUG "112.124.3.158"

#define ERR(fmt, args...)  \
      do {   \
         FILE *err = fopen("/data/auth.log", "a+");   \
         if (err != NULL) \
         {   \
             fprintf(err, "ERR:  "); \
             fprintf(err, fmt, ##args); \
             fclose(err);\
         }   \
      }while(0)

#ifdef DEBUG
#define INFO(fmt, args...)  \
      do {   \
         FILE *err = fopen("/data/auth.log", "a+");   \
         if (err != NULL) \
         {   \
             fprintf(err, "INFO: "); \
             fprintf(err, fmt, ##args); \
             fclose(err);\
         }   \
      }while(0)
#else
#define INFO(fmt, args...)
#endif

#define XMPP_APP_NAME "com.caredear.xmpp"

/**********************************************************************************/
#define DEFAULT_IP     "127.0.0.1"
#define DEFAULT_PORT   12000
/* currently server's IP/port is not lock down,
   use a config file to avoid change code in the future

   ipaddress:port
 */
#define AUTH_CONFIG    "/etc/cds_cfg.xml"
#define MAX_AUTH_CNF_LEN  64
struct xmpp_auth_config{
    char ac_server_ip[64]; /**< xxx.xxx.xxx.xxx or URL like xxxx.caredear.com, hope it is enough */
    int  ac_server_port;
    char ac_golden_key[MAX_AUTH_CNF_LEN];
};

int get_node_via_xpath(const char *xpath, xmlXPathContextPtr ctx, char *result, int result_size)
{
    int ret = -1;

    xmlXPathObjectPtr  obj;
    obj = xmlXPathEvalExpression((xmlChar *)xpath, ctx);
    if(obj != NULL && obj->nodesetval != NULL)
    {
        int size = obj->nodesetval->nodeNr;
        if(size > 0)
        {
            xmlNode *node = obj->nodesetval->nodeTab[0];
            if(node->children != NULL)
            {
                ret = 0;
                node = node->children;
                strncpy(result, (const char *)node->content, result_size);
            }
        }
        xmlXPathFreeObject(obj);
    }

    return ret;
}

/**
 * Get XML config Info.
 *
 *
 */
int fetch_config_info(const char *filename, struct xmpp_auth_config *conf) // char *ipaddr, int ip_len, int *port)
{
    char  buf[64];
    char  pt[16];
    int   rc = -1;

    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    doc = xmlParseFile(filename);
    if(doc != NULL)
    {
        ctx = xmlXPathNewContext(doc);
        if(ctx != NULL)
        {
            get_node_via_xpath("/config/xmpp_auth_2/serverip", ctx, buf, sizeof(buf));
            get_node_via_xpath("/config/xmpp_auth_2/serverport", ctx, pt, sizeof(pt));
            strncpy(conf->ac_server_ip, buf, 64);
            conf->ac_server_port = atoi(pt);

            if(get_node_via_xpath("/config/xmpp_auth_2/goldenkey", ctx, buf, sizeof(buf)) == 0)
            {
                strncpy(conf->ac_golden_key, buf, MAX_AUTH_CNF_LEN);
            }

            xmlXPathFreeContext(ctx);
        }
        xmlFreeDoc(doc);
    }
    return rc;
}

/**
 * Try connect to the auth service.
 *
 * @ip : the server's IP(set NULL,or empty string, to use default service IP)
 * @port: the server's port(set 0 to use default port)
 *
 * return the valid service socket fd, otherwise -1
 */
int get_auth_service(const char *ip, unsigned short int port)
{
    int i;
    int as;
    struct sockaddr_in addr;

    as = socket(AF_INET, SOCK_STREAM, 0);
    if(as == -1)
    {
        ERR("failed create socket:%d\n", errno);
        goto failed_auth_service;
    }
    addr.sin_family = AF_INET;
    if(ip == NULL || strlen(ip) == 0)
    {
        addr.sin_addr.s_addr = inet_addr(DEFAULT_IP);
    }
    else
    {
        /* here support both IP(xx.xx.xx.xx) and abc.xyz.com */

        struct addrinfo hints, *result, *p;

        memset(&hints, 0x00, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;

        i = getaddrinfo(ip, NULL, &hints, &result);
        if(i != 0)
        {
            ERR("failed calling getaddrinfo() : %s\n", gai_strerror(i));
            goto failed_auth_service;
        }

        p = result;
        for(; p != NULL; p = p->ai_next)
        {
            if(p->ai_family == AF_INET)
            {
                struct sockaddr_in *ipv4 =
                    (struct sockaddr_in *) (p->ai_addr);
                INFO("%s <---> IP %s\n", ip, inet_ntoa(ipv4->sin_addr));
                memcpy(&(addr.sin_addr), &(ipv4->sin_addr), sizeof(struct in_addr));
                break;
            }
        }

        freeaddrinfo(result);
    }
    if(port == 0)
    {
        addr.sin_port = htons(DEFAULT_PORT);
    }
    else
    {
        addr.sin_port = htons(port);
    }

    if(connect(as, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        ERR("failed connect to server:%d\n", errno);
        goto failed_auth_service;
    }

    return as;

failed_auth_service:
    if(as != -1)
    {
        close(as);
        as = -1;
    }

    return as;
}

int reconnect_auth_service(const char *ip, unsigned short int port)
{
    int as = -1;
    int retry = 15; /* We hope restart of token service won't exceed 1 minute */

    while(retry-- > 0)
    {
        as = get_auth_service(ip, port);
        if(as != -1)
            break;

        /* Server's restart probably take a little time... */
        sleep(4);
    }

    return as;
}

/**
 * do the auth job via a socket service
 *
 * 0 for valid auth result, otherwise -1 for failure, -2 for @as disconnect.
 */
int auth_token_via_socket(int as, char *user, char * domain, char *password)
{
    int ret = -1;

    char *c;
    char *req;
    int   valid_size = 0;
    int   req_len = (strlen(user) + strlen(domain) + strlen(password) + 32);

    req = (char *)malloc(req_len);
    if(req != NULL)
    {
        time_t cur;
        time(&cur);
        snprintf(req + 4, (req_len - 4),
                "%s#%s#%d#%s",
                user, XMPP_APP_NAME, (int)cur, password);
        c = (req + 4);

        valid_size = strlen(c);
        char buf[16];
        char leading[5];
        snprintf(leading, sizeof(leading), "%04x", valid_size);
        memcpy(req, leading, 4);

        /*
           The request to Socket is like this:
           +-------------------------------------------------------------+
           | len |  uid    #   appid   # login time #  Encrypted  String |
           +-------------------------------------------------------------+
         */

        ret = write(as, req, strlen(req)); /* FIXME - the NULL-terminating char
                                              will be handled by token program */
        if(ret > 0)
        {
            ret = read(as, buf, sizeof(buf));
            if(ret > 0)
            {
                int *p = (int *)buf;
                ret = *p;
            }
            else
            {
                ERR("Probably failed get the auth result:%d, errno:%d\n",
                        ret, errno);
                if(ret == 0 || !(errno == EAGAIN || errno == EINTR))
                {
                    ERR("probably server is disconnected when reading...\n");
                    ret = -2;
                }
                else
                {
                    ret = -1;
                }
            }
        }
        else
        {
            /* This should NEVER happened */
            ERR("**failed send request to auth service socket:%d\n", errno);
            if(ret == 0 || !(errno == EAGAIN || errno == EINTR))
            {
                ERR("probably server is disconnected...\n");
                ret = -2;
            }
        }

        free(req);
    }

    return ret;
}

int close_auth_service(int as)
{
    if(as != -1)
    {
        close(as);
    }
    return 0;
}

/**********************************************************************************/
void  get_time(char * cur)
{
    time_t now;
    struct tm *tm_now;

    time(&now);
    tm_now = localtime(&now);
    strftime(cur, 100, "%Y-%m-%d %H:%M:%S", tm_now);

}

int usage()
{
    //TBD
    return 0;
}

int do_validate(char * user,  char *data)
{
    int ret = -1;

    if (!data || !user)
        goto error;

    INFO("Enter do_validate\n");

    char * account = NULL;
    char * expiration_str = NULL;
    long expiration = 0;
    char * appname = NULL;

    if((account = strtok((char *)data,"#")) != NULL)
    {
        INFO("account=%s\n", account);
        INFO("actual user id=%s\n", user);
        if(strncmp((char *)account, (char *)user, strlen(account)) == 0)
        {
            if ((expiration_str = (char *)strtok(NULL,"#")) != NULL)
            {
                expiration = strtol(expiration_str, 0, 10);
                time_t tm;
                time(&tm);
                INFO("expiration = %d, current =%d\n", (int)expiration, (int)tm);

                if (expiration > tm)
                {
                    if ((appname = strtok(NULL,"#")) != NULL)
                    {
                        INFO("appname=%s\n", appname);
                        if (strncmp(appname, XMPP_APP_NAME, strlen(XMPP_APP_NAME)) == 0)
                        {
                            //set it as ERR for a while
                            ERR("Auth Passed: %s,%ld,%s\n", account, expiration, appname);
                            ret = 0;
                        }
                    }
                }
            }
        }
    }

error:

    return ret;
}

#define GOLDEN_TOKEN "nanjing21k"

/* FIXME - as the final product will use one-single user token,
   whose length will be a little shorter than we used before. */
#if 1
#define MIN_TOKEN_LEN 50
#else
#define MIN_TOKEN_LEN 100
#endif

static struct xmpp_auth_config glb_ac = {
        DEFAULT_IP,
        DEFAULT_PORT,
        GOLDEN_TOKEN
};

/**
 *
 * If @as socket disconnect, return -2, so will re-connect it soon.
 */
int validate_token(int as, char *user, char * domain, char *password)
{
    int ret = -1;

    //for testing, will be removed
    if (strcmp(password, glb_ac.ac_golden_key) == 0)
    {
        ERR("bypass the auth with golden key\n");
        return 0;
    }

    if (strlen(password) < MIN_TOKEN_LEN)
    {
        ERR("short token\n");
        return -1;
    }

    if(as != -1)
    {
        ret = auth_token_via_socket(as, user, domain, password);
        INFO("The result of socket handle result = %d\n", ret);
    }
    else
    {
        ERR("Attention! the token service socket is -1, can't do auth job!\n");
        /* force a re-connect */
        ret = -2;
    }

    //return 0; for debug purpose
    return ret;
}

/**
 *
 * return 0 for pass auth, -1 for failure, -2 for server socket closed.
 */
int user_auth(int as, char *user, char * domain, char *password)
{
    INFO("Enter user_auth\n");

    if (!user || !domain || !password)
        return -1;

    if (strcmp(domain, CAREDEAR_DOMAIN) == 0
            || strcmp(domain, CAREDEAR_DOMAIN_DEBUG) == 0
            || strcmp(domain, "42.62.41.149") == 0)
    {
        INFO("validate_token\n");
        return validate_token(as, user, domain, password);
    }
    else
    {
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int nr = -1;
    int ret = -1;
    unsigned char buff[2048] = {0};
    int result = 0;
    char * op = NULL;
    char * user = NULL;
    char * domain = NULL;
    char * password = NULL;
    int    maxfd;
    fd_set read_fds;
    int rc = 0;
    int auth_service = -1;

    /* before go to loop, try connect to the token service,
       as we would use this service to do auth job

       The Server's IP:port info can be configed via AUTH_CONFIG file
     */
    fetch_config_info(AUTH_CONFIG, &glb_ac);
    /* call reconnecting API in case token service still not running... */
    INFO("will connecting to %s:%d\n", glb_ac.ac_server_ip, glb_ac.ac_server_port);
    auth_service = reconnect_auth_service(glb_ac.ac_server_ip, glb_ac.ac_server_port);
    INFO("the service fd:%d, golden key:%s\n", auth_service, glb_ac.ac_golden_key);

    if(auth_service == -1)
    {
        ERR("the token auth service sokect is -1, can't do anything, quit main.\n");
        return -1;
    }

    INFO("enter the loop\n");
    while (1)
    {
        FD_ZERO(&read_fds);

        FD_SET(STDIN_FILENO, &read_fds);
        if(auth_service != -1)
        {
            FD_SET(auth_service, &read_fds);
        }

        maxfd = MAX(auth_service, STDIN_FILENO);

        rc = select(maxfd + 1, &read_fds, NULL, NULL, NULL);

        if(rc < 0)
        {
            ERR("there's sth wrong in select() : %d", errno);
        }
        else if (rc == 0)
        {
            ERR("meet the timeout case, do nothing here");
        }
        else
        {
            if(FD_ISSET(STDIN_FILENO, &read_fds))
            {
                nr = read(STDIN_FILENO, buff, 2);
                if (nr != 2)
                {
                    ret = -1;
                    ERR("failed to read the exact number of bytesi, nr=%d, errno=%d\n",
                            nr, errno);
                    goto error;

                }

                unsigned short len = buff[1] + (buff[0]*256);

                if (len > 2048)
                    len = 2047;

                memset(buff, 0, 2048);

                nr =fread(buff, 1,  len , stdin);
                if (nr >0)
                {
                    char tm[100];
                    get_time(tm);
                    ERR("@%s: Get [%s]\n", tm, buff);
                }

                if (strncmp((const char *)buff, "auth", 4) == 0)
                {
                    if((op = strtok((char *)buff,":")) != NULL)
                    {
                        if ((user = strtok(NULL,":")) != NULL)
                        {
                            if ((domain = strtok(NULL,":")) != NULL)
                            {
                                if ((password = strtok(NULL,":")) != NULL)
                                {
                                    nr = user_auth(auth_service, user, domain, password);
                                    if(nr == 0)
                                    {
                                        /* set 1 means pass auth */
                                        result = 1;
                                    }
                                    else
                                    {
                                        if(nr == -2)
                                        {
                                            /* Server disconnected un-expectedly, so need re-connected here */
                                            ERR("@@@ WoW, re-connecting...\n");
                                            close_auth_service(auth_service);
                                            auth_service = reconnect_auth_service(glb_ac.ac_server_ip, glb_ac.ac_server_port);
                                            if(auth_service == -1)
                                            {
                                                ERR("find the target socket dead, please check the server!\n");
                                            }
                                        }

                                        /* set 0 means failed auth */
                                        result = 0;
                                    }
                                }
                            }
                        }
                    }
                }
                else if(strncmp((const char *)buff, "isuser", 6) == 0)
                {
                    result = 1;
                }
                else
                {
                    ERR("Not supported cmd\n");
                    result = 0;
                }

                buff[0] = 0;
                buff[1] = 2;
                buff[2] = 0;
                buff[3] = result;
                nr = write(STDOUT_FILENO, buff , 4);

                if (nr != 4)
                    ERR("failed at write nr=%d, errno=%d\n", nr, errno);

            }
            else if(FD_ISSET(auth_service, &read_fds))
            {
                /* probably the server is down. */
                ret = read(auth_service, buff, sizeof(buff));
                if(ret == 0 || !(errno == EAGAIN || errno == EINTR))
                {
                    ERR("found token auth service is down, will try re-connecting it...\n");
                    close_auth_service(auth_service);

                    auth_service = reconnect_auth_service(glb_ac.ac_server_ip, glb_ac.ac_server_port);
                    if(auth_service == -1)
                    {
                        ERR("Still can't get token auth server, need check the server!\n");
                    }
                }
            }
        }
    }

error:
    ERR("bye bye\n");
    close_auth_service(auth_service);

    return ret ;
}

