/**
 * NewUser Service(NUS) code
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#ifndef __cplusplus
#define __USE_XOPEN
#endif

#include <time.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#ifdef CHECK_MEM_LEAK
#include <mcheck.h>
#endif

#include <openssl/hmac.h>
#include <openssl/md5.h>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "cds_public.h"
#include "newusr.h"
#include "NewUserMessage.pb.h"

/* Now, we're going to C++ world */
using namespace std;
using namespace memcache;
using namespace google::protobuf::io;
using namespace com::caredear;

static MYSQL *nus_sql = NULL;

/* Use a token generation encrypt key */
static unsigned char encrypt_aeskey[16] = {
        0xB7, 0xc3, 0x55, 0x57, 0x42, 0xd4, 0x74, 0x9b,
        0xb8, 0x5a, 0x3f, 0x4f, 0x41, 0xb8, 0x3d, 0xed
};

memcached_st *memc = NULL;

static int get_nus_config(const char *cfg_file, struct nus_config *p_cfg)
{
    int ret = -1;
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;
    char buf[32];

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
            get_node_via_xpath("/config/new_usertoken/memcache/ip", ctx,
                    buf, sizeof(buf));
            strncpy(p_cfg->memcach_ip, buf, sizeof(buf));

            get_node_via_xpath("/config/new_usertoken/memcache/port", ctx,
                    buf, sizeof(buf));
            p_cfg->memcach_port = atoi(buf);

            get_node_via_xpath("/config/new_usertoken/sqlserver/ip", ctx,
                    p_cfg->sql_cfg.ssi_server_ip, 32);

            get_node_via_xpath("/config/new_usertoken/sqlserver/user", ctx,
                    p_cfg->sql_cfg.ssi_user_name, 32);

            get_node_via_xpath("/config/new_usertoken/sqlserver/password", ctx,
                    p_cfg->sql_cfg.ssi_user_password, 32);
            // FIXME , I don't parse database name here, as we will
            // use database.dbtable in SQL command.

            INFO("the new user toke SQL info = %s\n",
                    p_cfg->sql_cfg.ssi_server_ip);

            xmlXPathFreeContext(ctx);
            ret = 0;
        }

        xmlFreeDoc(doc);

    }

    nus_sql = GET_NUSSQL(&(p_cfg->sql_cfg));
    if(nus_sql == NULL)
    {
        ERR("failed connecting to the SQL server!\n");
        return -1;
    }

    return ret;
}

int get_md5(const char *filename, char *p_md5)
{
    int ret = -1;
    int len = 0;
    char buf[2046];
    MD5_CTX ctx;
    FILE *f = fopen(filename, "rb");

    if(f != NULL)
    {
        MD5_Init(&ctx);

        while((len = fread(buf, 1, sizeof(buf), f)) != 0)
        {
            MD5_Update(&ctx, buf, len);
        }

        MD5_Final((unsigned char *)p_md5, &ctx);

        fclose(f);

        ret = 0;
    }

    return ret;
}

/**
 *
 *@md5 : the md5 checksum in Qiniu netdisk
 *
 */



/**
 * Wraper for send back data, via protobuf
 *
 *@result_code: the CDS_XXX error code
 *@p_respdata: the raw response buffer(embeded into CDS)
 *
 * reutrn 0 for successful, otherwise return -1
 */
static int sendback_response(int result_code, const char *errmsg, NewUserResponse *p_nur, int *p_resplen, void *p_respdata)
{
    unsigned short len;

    p_nur->set_result_code(result_code);

    if(result_code != CDS_OK)
    {
        p_nur->set_user_token("BAD");
    }

    len = p_nur->ByteSize();

    if(len >= 1024)
    {
        ERR("FATAL ERROR, response exceed 1k!, should we continue?\n");
    }

    // we use 2-byte as leading length
    *p_resplen = (len + 2);

    ArrayOutputStream aos(p_respdata, *p_resplen);
    CodedOutputStream cos(&aos);

    // add the leading-len
    cos.WriteRaw(&len, sizeof(len));
    return ((p_nur->SerializeToCodedStream(&cos) == true) ? 0 : -1);
}

/**
 * Created the upload policy
 *
 */

/**
 * Debug cookie flag
 *
 */
static int in_debug_mode()
{
    int debug = 0;

    if(access("/tmp/nu.debug", F_OK) == 0)
    {
        debug = 1;
    }

    return debug;
}

/**
 * Just test code for uploading via C,
 * should NEVER be triggered in product
 * release.
 */

/**
 * handler for user try uploading a file to netdisk. This function will just return
 * upload token to caller.
 *
 */


/**
 * o Mapp the file to MD5 key in Qiniu
 * o Try delete to Qiniu
 * o update our server's DB
 */

/**
 * this is a handler for ping alive package, if sth wrong happended,
 * we need probably send out an email for this!
 *
 */
int ping_nus_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret = 0;

    LOG("A PING alive handler\n");

    ret = keep_db_connected(nus_sql);

    /* for ping case, we don't need Protobuf's sendbackresponse...
     */
    *len_resp = 4;
    *(int *)resp = ret;

    return ret;
}


/**
 * This will:
 * 
 * o create a new token
 * o update memcach and DB
 * o send back the response to caller.
 *
 *@user_token [output]: store user token string
 *
 */
static int new_user_token(const char *username, char *user_token)
{
    char buf[64];
    struct timeval tv;
    if(gettimeofday(&tv, NULL) != 0)
    {
        // this should NEVER happen
        tv.tv_sec = 123;
    }

    snprintf(buf, sizeof(buf), "%s#%ld#com.caredear.service",
           username, tv.tv_sec);

    char *val = encrypt_token_string_with_aes(buf, (char *)encrypt_aeskey);

    //FIXME - caller should provide big enough size to avoid overflow
    if(val != NULL)
    {
        strcpy(user_token, val);
        free(val);
    }
    else
    {
        user_token[0] = '\0';
    }

    return 0;
}

/**
 * call back function, handle obj is
 *
 * return error values if sth wrong.
 */
int nus_handler(int size, void *req, int *len_resp, void *resp)
{
    unsigned short len;
    int ret = CDS_OK;
    bool ok = false;
    string username;
    NewUserRequest reqobj;
    NewUserResponse nu_resp;

    if(size >= 1024)
    {
        /* Note - actually, len checking already done at .SO side... */
        ERR("Exceed limit(%d > %d), don't handle this request\n",
                size, 1024);
        ret = CDS_ERR_REQ_TOOLONG;
        if(sendback_response(ret, NULL, &nu_resp, len_resp, resp) != 0)
        {
            ERR("WARNING Failed Serialize the too-long error data\n");
        }

        return ret;
    }

    LOG("got %d size from client\n", size);


    ArrayInputStream in(req, size);
    CodedInputStream is(&in);

    ok = reqobj.ParseFromCodedStream(&is);

    if(ok)
    {
        // After go here, all data section got
        // and stored in NewUserRequest obj.

        username = reqobj.user();

        //new_user_token();

        //TODO new user token....

    }
    else
    {
        ERR("**Failed parse the request obj in protobuf!\n");
        ret = CDS_ERR_REQ_INVALID;
        if(sendback_response(ret, NULL, &nu_resp, len_resp, resp) != 0)
        {
            ERR("WARNING Failed Serialize the req-invalide error data\n");
        }
    }

    return ret;
}

int main(int argc, char **argv)
{
    struct addition_config cfg; // this config is passing to CDS BASE component
    struct nus_config nuscfg; // nus own's config
    char buffer[256];

#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/nus.memleak", 1);
    mtrace();
#endif

    if(get_nus_config("/etc/cds_cfg.xml", &nuscfg) != 0)
    {
        ERR("Failed init and get config info! quit this new user service!\n");
        return -1;
    }

    snprintf(buffer, sizeof(buffer), "--SERVER=%s:%d",
            nuscfg.memcach_ip, nuscfg.memcach_port);
    memc = memcached(buffer, strlen(buffer));
    if(memc == NULL)
    {
        ERR("**failed to connect \'%s\' memcached server!\n", buffer);
    }
    else
    {
        INFO("connect to memcached server(\'%s\') [OK]\n", buffer);
    }

    if(INIT_DB_MUTEX() != 0)
    {
        FREE_NUSSQL(nus_sql);
        ERR("Failed create IPC objs:%d\n", errno);
        return -2;
    }
#if 0
    //just debug code...
    struct timeval tv;
    gettimeofday(&tv, NULL);
    char *plain = "13815882359#1419492935#com.caredear.service";
    char * value = encrypt_token_string_with_aes(plain, encrypt_aeskey);

    printf("%s-> val:%s\n",  plain, value);

    char *v2 = decrypt_token_string_with_aes(value, encrypt_aeskey);
    printf("\n%s\n", v2);

    free(v2);
    free(value);
#endif

    ///
    // Now, drop into cds_base loop...
    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = nus_handler;
    cfg.ping_handler = ping_nus_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */
    cds_init(&cfg, argc, argv);

    // quit action...
    if(memc != NULL)
    {
        memcached_free(memc);
    }

    CLEAN_DB_MUTEX();
    FREE_NUSSQL(nus_sql);

    return 0;
}
