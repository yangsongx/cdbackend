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


#include "cds_public.h"
#include "newusr.h"


#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "NewUserMessage.pb.h"

/* Now, we're going to C++ world */
using namespace std;
using namespace google::protobuf::io;
using namespace com::caredear;

static MYSQL *nus_sql = NULL;

/* QINIU SDK KEYS... */


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
int sendback_response(int result_code, const char *errmsg, NewUserResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    unsigned short len;

    p_ndr->set_result_code(result_code);

    return 0;
}

/**
 * Created the upload policy
 *
 */

/**
 * Debug cookie flag
 *
 */
int in_debug_mode()
{
    int debug = 0;

    if(access("/tmp/nd.debug", F_OK) == 0)
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
 * update the DB as we found APK upload the file successfully
 *
 */

/**
 * o Mapp the file to MD5 key in Qiniu
 * o Try delete to Qiniu
 * o update our server's DB
 */

/**
 * handler for user want to download a netdisk file...
 *
 */

int ping_nus_handler(int size, void *req, int *len_resp, void *resp)
{
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
#if 0
    NetdiskResponse nd_resp;

    if(size >= 1024)
    {
        /* Note - actually, len checking already done at .SO side... */
        ERR("Exceed limit(%d > %d), don't handle this request\n",
                size, 1024);
        ret = CDS_ERR_REQ_TOOLONG;
        if(sendback_response(ret, "too much long length in req", &nd_resp, len_resp, resp) != 0)
        {
            ERR("***Failed Serialize the too-long error data\n");
        }

        return ret;
    }

    LOG("got %d size from client\n", size);

    NetdiskRequest reqobj;
    ArrayInputStream in(req, size);
    CodedInputStream is(&in);

    ok = reqobj.ParseFromCodedStream(&is);
    if(ok)
    {
        // After go here, all data section got
        // and stored in NetdiskRequest obj.

        switch(reqobj.opcode())
        {
            case UPLOADING:
                LOG("\n=== UPLOADING case===\n");
                LOG("User:%s, File:%s, size:%d, MD5:%s\n",
                        reqobj.user().c_str(),
                        reqobj.filename().c_str(),
                        reqobj.filesize(),
                        reqobj.md5().c_str());
                LOG("==================\n\n");

                ret = do_upload(&reqobj, &nd_resp, len_resp, resp);
                break;

            case UPLOADED:
                LOG("\n=== UPLOADED case===\n");
                LOG("User:%s, File:%s, size:%d, MD5:%s\n",
                        reqobj.user().c_str(),
                        reqobj.filename().c_str(),
                        reqobj.filesize(),
                        reqobj.md5().c_str());
                LOG("==================\n\n");
                ret = complete_upload(&reqobj, &nd_resp, len_resp, resp);
                break;

            case DOWNLOADURL:
                LOG("\n=== DOWNLOAD case===\n");
                LOG("User:%s, File:%s\n",
                        reqobj.user().c_str(), reqobj.filename().c_str());
                LOG("==================\n\n");
                ret = do_download(&reqobj, &nd_resp, len_resp, resp);
                break;

            case DELETE:
                LOG("\n=== DELETE case===\n");
                LOG("User:%s, File:%s\n",
                        reqobj.user().c_str(), reqobj.filename().c_str());
                LOG("==================\n\n");

                ret = do_deletion(&reqobj, &nd_resp, len_resp, resp);
                break;

            case SHARE:
                LOG("\n=== SHARE case===\n");
                LOG("Try sharing File:%s\n",
                        reqobj.filename().c_str());
                LOG("==================\n\n");

                ret = do_sharing(&reqobj, &nd_resp, len_resp, resp);
                break;

            case RENAME: // such as user move one file to another plaee
                // this is just change DB record, won't modify any stuff
                // on Qiniu Server
                LOG("\n=== RENAME case===\n");
                LOG("Try renaming %s --> %s\n",
                        reqobj.filename().c_str(), reqobj.newfile().c_str());
                LOG("==================\n\n");

                ret = do_rename(&reqobj, &nd_resp, len_resp, resp);
                break;

            case LISTFILE:
                //
                //TODO, this is a complicated operation
                //
                break;

            default:
                break;
        }


    }
    else
    {
        ERR("**Failed parse the request obj in protobuf!\n");
    }
#endif
    return ret;
}

int main(int argc, char **argv)
{
    struct addition_config cfg; // this config is passing to CDS BASE component

    struct nus_config nuscfg; // nus own's config

#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/nus.memleak", 1);
    mtrace();
#endif

    if(get_nus_config("/etc/cds_cfg.xml", &nuscfg) != 0)
    {
        ERR("Failed init and get config info! quit this new user service!\n");
        return -1;
    }

    if(INIT_DB_MUTEX() != 0)
    {
        FREE_NUSSQL(nus_sql);
        ERR("Failed create IPC objs:%d\n", errno);
        return -2;
    }

    //
    LOG(".. debuging...\n\n");
    set_memcache(&nuscfg, "13022593515", "tbb8w01xsi4bqkfcmpcyxlx9gn2eahljyzt0pfb5d9crsnz3sy8s850mn548pkr3 2014-12-24 07:54:42");
    ///

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = nus_handler;
    cfg.ping_handler = ping_nus_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */
    cds_init(&cfg, argc, argv);

    CLEAN_DB_MUTEX();
    FREE_NUSSQL(nus_sql);

    return 0;
}
