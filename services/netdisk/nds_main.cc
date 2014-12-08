/**
 * NetDisk Service(NDS) code
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#include <qiniu/base.h>
#include <qiniu/io.h>
#include <qiniu/rs.h>


#include "cds_public.h"
#include "nds_def.h"


#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "NetdiskMessage.pb.h"

/* Now, we're going to C++ world */
using namespace std;
using namespace google::protobuf::io;
using namespace com::caredear;

static MYSQL *nds_sql = NULL;

/* QINIU SDK KEYS... */
Qiniu_Client qn;
const char *QINIU_ACCESS_KEY;
const char *QINIU_SECRET_KEY;
const char *qiniu_bucket;
const char *qiniu_domain;
unsigned int qiniu_expires = 2592000; // 30 dyas by default

int init_and_config(const char *cfg_file)
{
    int ret = -1;
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    static char access_key[128];
    static char secret_key[128];
    static char bucket[128];
    static char domain[128];

    /* Init Qiniu stuff,as we need it's service */
    Qiniu_Servend_Init(-1);

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
            get_node_via_xpath("/config/netdisk/qiniu/access_key", ctx,
                    access_key, sizeof(access_key));

            get_node_via_xpath("/config/netdisk/qiniu/secret_key", ctx,
                    secret_key, sizeof(secret_key));

            get_node_via_xpath("/config/netdisk/sqlserver/ip", ctx,
                    sql_cfg.ssi_server_ip, 32);

            get_node_via_xpath("/config/netdisk/sqlserver/user", ctx,
                    sql_cfg.ssi_user_name, 32);

            get_node_via_xpath("/config/netdisk/sqlserver/password", ctx,
                    sql_cfg.ssi_user_password, 32);
            // FIXME , I don't parse database name here, as we will
            // use database.dbtable in SQL command.

            QINIU_ACCESS_KEY = access_key;
            QINIU_SECRET_KEY = secret_key;


            // re-use a static buf temp for store int...
            get_node_via_xpath("/config/netdisk/qiniu/expiration", ctx,
                    domain, sizeof(domain));
            qiniu_expires = atoi(domain);

            //next, overwrite the domain immediately
            get_node_via_xpath("/config/netdisk/qiniu/domain", ctx,
                    domain, sizeof(domain));

            get_node_via_xpath("/config/netdisk/qiniu/bucket", ctx,
                    bucket, sizeof(bucket));

            qiniu_domain = domain;
            qiniu_bucket = bucket;

            xmlXPathFreeContext(ctx);
            ret = 0;
        }

        xmlFreeDoc(doc);

        LOG("ACCESS KEY:%s, SECRET KEY:%s\n",
                QINIU_ACCESS_KEY, QINIU_SECRET_KEY);
        LOG("SQL Server IP : %s Port : %d, user name : %s\n",
                sql_cfg.ssi_server_ip, sql_cfg.ssi_server_port,
                sql_cfg.ssi_user_name);
    }

    nds_sql = GET_CMSSQL(&sql_cfg);
    if(nds_sql == NULL)
    {
        ERR("failed connecting to the SQL server!\n");
        return -1;
    }

    return ret;
}

void purely_api(const char *fn)
{
    Qiniu_Client_InitMacAuth(&qn, 1024, NULL);
    char buf[1024];
    sprintf(buf, "caredear-cloud:%s", fn);

    Qiniu_RS_PutPolicy p;
    Qiniu_Zero(p);
    p.scope = buf;
    printf("the scope:%s\n", p.scope);

    char *f = Qiniu_RS_PutPolicy_Token(&p, NULL);
    printf("the token:\n%s\n\n", f);
}

void another_test(const char *filename)
{
    Qiniu_Client_InitMacAuth(&qn, 1024, NULL);
    char buf[1024];
    sprintf(buf, "caredear-pic:%s", filename);

    Qiniu_RS_PutPolicy p;
    Qiniu_Zero(p);
    p.scope = buf;
    //p.expires = 2451491200;

    printf("the scope:%s\n", p.scope);

    char *f = Qiniu_RS_PutPolicy_Token(&p, NULL);
    printf("the token:\n%s\n\n", f);

    Qiniu_Error err;
    Qiniu_Io_PutRet putRet;
    err = Qiniu_Io_PutFile(&qn, &putRet, f, NULL, filename, NULL);

    printf("the status code = %d\n", err.code);
    if(err.code != 200)
    {
        printf("the msg:%s\n", err.message);
    }

    exit(0);
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

void test_download_url()
{
    //char url[] = "http://caredear-cloud.qiniudn.com/300047\%2F14175708058725878";
    char url[] = "http://caredear-cloud.qiniudn.com/300047/14175708058725878";
    Qiniu_RS_GetPolicy getPolicy;
    getPolicy.expires = 1420162805;
    char *p = Qiniu_RS_GetPolicy_MakeRequest(&getPolicy, url, NULL);

    printf("the data\n%s\n", p);
}

int get_download_url(NetdiskRequest *p_obj, NetdiskResponse *p_resp)
{
    Qiniu_RS_GetPolicy get_policy;
    get_policy.expires = qiniu_expires;
    char domain_str[512]; // a normal http://wwww.xxxx.xxx should not exceed 512 byte

    snprintf(domain_str, sizeof(domain_str),
            "%s%s", qiniu_bucket, qiniu_domain);

    char *baseurl = Qiniu_RS_MakeBaseUrl(domain_str, p_obj->md5().c_str());
    LOG("Now, baseurl=%s\n", baseurl);

    char *download_url = Qiniu_RS_GetPolicy_MakeRequest(&get_policy, baseurl, NULL);

    LOG("download url=%s\n", download_url);

    Qiniu_Free(baseurl);

    return 0;
}


/**
 * Created the upload policy
 *
 */
int generate_upload_token(NetdiskRequest *p_obj)
{
    Qiniu_RS_PutPolicy put_policy;

    // per req creates one
    Qiniu_Client_InitMacAuth(&qn, 1024, NULL);

    memset(&put_policy, 0x00, sizeof(put_policy));
    put_policy.scope = qiniu_bucket;
    put_policy.expires = qiniu_expires;

    char *uptoken= Qiniu_RS_PutPolicy_Token(&put_policy, NULL);

    LOG("the QINIU SDK created base64 string on policy:%s\n", uptoken);

    LOG("\n\nDOWNLOAD...\n\n");
    get_download_url(p_obj, NULL);
#if 0
    // 
    // TODO - below code is just testing ...
    // as upload should be done at apk side....

    // Actually, we need fill response fields...
    Qiniu_Error err;
    Qiniu_Io_PutRet putRet;
    LOG("Uploading to server with key %s\n", p_obj->md5().c_str());
    err = Qiniu_Io_PutFile(&qn, &putRet, uptoken, p_obj->md5().c_str(),
            p_obj->filename().c_str(), NULL);

    printf("the status code = %d\n", err.code);
    if(err.code != 200)
    {
        printf("the msg:%s\n", err.message);
    }
#endif
    // DO NOT FORGET release resource...
    Qiniu_Free(uptoken);
    Qiniu_Client_Cleanup(&qn);

    return 0;
}


/**
 * call back function, handle obj is NetdiskMessage(NetdiskRequest/NetdiskResponse)
 *
 * return error values if sth wrong.
 */
int nds_handler(int size, void *req, int *len_resp, void *resp)
{
    unsigned short len;
    int ret = CDS_OK;
    bool ok = false;
    NetdiskResponse nd_resp;

    if(size >= 1024)
    {
        /* Note - actually, len checking already done at .SO side... */
        ERR("Exceed limit(%d > %d), don't handle this request\n",
                size, 1024);
        ret = CDS_ERR_REQ_TOOLONG;
        nd_resp.set_result_code(ret);
        nd_resp.set_uploadurl("");
        nd_resp.set_downloadurl("");
        nd_resp.set_netdisckey("");
        len = nd_resp.ByteSize();
        ArrayOutputStream aos(resp, len);
        CodedOutputStream cos(&aos);

        LOG("sizeof(short)=%ld, value=%d\n",
                sizeof(len), len);
        // adding leading length
        cos.WriteRaw(&len, sizeof(len));
        if(nd_resp.SerializeToCodedStream(&cos))
        {
            INFO("composed the TOOLONG error response\n");
        }
        else
        {
            ERR("***Failed compose the TOOLOG Error Msg\n");
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
#ifdef DEBUG // dump the content
        LOG("\n==== DUMP the objs... ====\n");
        LOG("user:%s, filename:%s, MD5=%s\n",
                reqobj.user().c_str(), reqobj.filename().c_str(),
                reqobj.md5().c_str());
        LOG("==== END DUMP ====\n\n");
#endif

        if(already_existed(&reqobj) != 0)
        {
            //TODO, response with a already case data.
        }

        generate_upload_token(&reqobj);

        // Below code are POST code handling..
        LOG("After some processing, will try report the response..\n");

        nd_resp.set_result_code(CDS_OK);
        nd_resp.set_uploadurl("http://download.url?token=xxxx");
        nd_resp.set_downloadurl("download.zip");
        nd_resp.set_netdisckey("2003/hello");
        len = nd_resp.ByteSize();
        ArrayOutputStream as(resp, 1024);
        CodedOutputStream cs(&as);

        LOG("sizeof(short)=%ld, value=%d\n",
                sizeof(len), len);
        // adding leading length
        cs.WriteRaw(&len, sizeof(len));
        *len_resp = (len + 2);
        if(nd_resp.SerializeToCodedStream(&cs))
        {
            INFO("composed the TOOLONG error response\n");
        }
        else
        {
            ERR("***Failed compose the response Msg....\n");
        }
    }
    else
    {
        ERR("**Failed parse the obj in protobuf!\n");
    }

    return ret;
}

int main(int argc, char **argv)
{
    struct addition_config cfg;
#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/nds.memleak", 1);
    mtrace();
#endif

    if(init_and_config("/etc/cds_cfg.xml") != 0)
    {
        ERR("Failed init and get config info! quit this netdisk service!\n");
        return -1;
    }

    if(INIT_DB_MUTEX() != 0)
    {
        FREE_CMSSQL(nds_sql);
        ERR("Failed create IPC objs:%d\n", errno);
        return -2;
    }

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = nds_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */
	LOG("i set ac_lentyp=%d\n", cfg.ac_lentype);
    cds_init(&cfg, argc, argv);

    CLEAN_DB_MUTEX();
    FREE_CMSSQL(nds_sql);

    return 0;
}
