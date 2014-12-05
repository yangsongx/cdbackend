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
#include <qiniu/base.h>
#include <qiniu/io.h>
#include <qiniu/rs.h>


#include "cds_public.h"
#include "nds_def.h"

// max/min are arleady defined in STL...
#undef max
#undef min

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "NetdiskMessage.pb.h"

#define COMPOSE_RESP(_err)   *(int *)resp = (_err); \
                             *len = sizeof(int); \
                             ret = (_err); \
                             goto failed

/* Now, we're going to C++ world */
using namespace std;
using namespace google::protobuf::io;
using namespace com::caredear;

static MYSQL *nds_sql = NULL;

/* QINIU SDK KEYS... */
Qiniu_Client qn;
const char *QINIU_ACCESS_KEY;// = "5haoQZguw4iGPnjUuJnhOGufZMjrQnuSdySzGboj";
const char *QINIU_SECRET_KEY;// = "OADMEtVegAXAhCJBhRSXXeEd_YRYzEPyHwzJDs95";

int init_and_config(const char *cfg_file)
{
    int ret = -1;
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    static char access_key[128];
    static char secret_key[128];

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

            QINIU_ACCESS_KEY = access_key;
            QINIU_SECRET_KEY = secret_key;

            // FIXME , I don't parse database name here, as we will
            // use database.dbtable in SQL command.

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

void test_download_url()
{
    //char url[] = "http://caredear-cloud.qiniudn.com/300047\%2F14175708058725878";
    char url[] = "http://caredear-cloud.qiniudn.com/300047/14175708058725878";
    Qiniu_RS_GetPolicy getPolicy;
    getPolicy.expires = 1420162805;
    char *p = Qiniu_RS_GetPolicy_MakeRequest(&getPolicy, url, NULL);

    printf("the data\n%s\n", p);
}
// test code;
void try_upload(const char *filename)
{
#if 1
    test_download_url();
#else
    // per req creates one
    Qiniu_Client_InitMacAuth(&qn, 1024, NULL);

#if 0
    unsigned char putpolicy[] ="eyJzY29wZSI6Im15LWJ1Y2tldDpzdW5mbG93ZXIuanBnIiwiZGVhZGxpbmUiOjE0NTE0OTEyMDAs"
                      "InJldHVybkJvZHkiOiJ7XCJuYW1lXCI6JChmbmFtZSksXCJzaXplXCI6JChmc2l6ZSksXCJ3XCI6"
                      "JChpbWFnZUluZm8ud2lkdGgpLFwiaFwiOiQoaW1hZ2VJbmZvLmhlaWdodCksXCJoYXNoXCI6JChl"
                      "dGFnKX0ifQ==";
#else
    Qiniu_Client_InitMacAuth(&qn, 1024, NULL);
    char buf[1024];
    sprintf(buf, "caredear-pic:%s", filename);
    char json[1024];
    char fly[1024];
    int flen= sizeof(fly);

    Qiniu_RS_PutPolicy p;
    Qiniu_Zero(p);
    p.scope = buf;
    p.expires = 2451491200;

    printf("the scope:%s\n", p.scope);

    sprintf(json, "{\"scope\":\"%s\",\"fsizeLimit\":10000000,\"deadline\":1420162805}",
            "caredear-cloud:300047/14175708058725878");

    printf("json:\n%s\n", json);


    char *putpolicy = base64(json, strlen(json), &flen);

    /*
    char putpolicy[] ="eyJzY29wZSI6Im15LWJ1Y2tldDpzdW5mbG93ZXIuanBnIiwiZGVhZGxpbmUiOjE0NTE0OTEyMDAs"
                      "InJldHVybkJvZHkiOiJ7XCJuYW1lXCI6JChmbmFtZSksXCJzaXplXCI6JChmc2l6ZSksXCJ3XCI6"
                      "JChpbWFnZUluZm8ud2lkdGgpLFwiaFwiOiQoaW1hZ2VJbmZvLmhlaWdodCksXCJoYXNoXCI6JChl"
                      "dGFnKX0ifQo=";
                      */
#endif

    unsigned char key[] = "OADMEtVegAXAhCJBhRSXXeEd_YRYzEPyHwzJDs95";

    char md[1024];
    int mdlen = sizeof(md);
    memset(md, '\0', sizeof(md));


    printf("putpolic:\n%s\n", putpolicy);
    printf("sizeof(key)=%d\n", sizeof(key));
    printf("policy len=%d\n", strlen(putpolicy));

    char *bin = HMAC(EVP_sha1(),
            key,
            strlen(key),
            putpolicy,
            strlen(putpolicy),
            md,
           &mdlen);

    printf("the cmdlen=%d\n", mdlen);

    char *f = base64(bin, mdlen, &flen);

    printf("After base64, len=%d\n",flen);
    printf("the result:%s\n", f);

    sprintf(md, "%s:%s:%s",
            QINIU_ACCESS_KEY, f, putpolicy);

    printf("\n\n\t NOw, the whole token is:\n%s\n",
            md);

    // now, md is the upload token
    Qiniu_Error err;
    Qiniu_Io_PutRet putRet;
    char badkey[] ="300047/14175708058725878";
    err = Qiniu_Io_PutFile(&qn, &putRet, md, badkey, filename, NULL);

    printf("the status code = %d\n", err.code);
    if(err.code != 200)
    {
        printf("the msg:%s\n", err.message);
    }

#endif
    exit(0);
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
        ERR("\n\n$$$$$$$$$$$$$$GOOD$$$$$$$$$$$$$$$\n");
        // After go here, all data section got
        // and stored in NetdiskRequest obj.
#ifdef DEBUG // dump the content
        LOG("==== DUMP the objs... ====\n");
        LOG("user:%s, filename:%s, filesize=%d\n",
                reqobj.user().c_str(), reqobj.filename().c_str(),
                reqobj.filesize());
        LOG("==== END DUMP ====\n");
#endif

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

//failed:
    return ret;
}

int main(int argc, char **argv)
{
    struct addition_config cfg;
#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/nds.memleak", 1);
    mtrace();
#endif


    //try_upload("/tmp/abc.png");
    //another_test("/tmp/abc.png");

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
