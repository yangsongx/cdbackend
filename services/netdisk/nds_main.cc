/**
 * NetDisk Service(NDS) code
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#define __USE_XOPEN
#include <time.h>

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
const char *QINIU_ACCESS_KEY = "5haoQZguw4iGPnjUuJnhOGufZMjrQnuSdySzGboj";
const char *QINIU_SECRET_KEY = "OADMEtVegAXAhCJBhRSXXeEd_YRYzEPyHwzJDs95";

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
 * call back function, handle obj is NetdiskMessage
 *
 * return error values if sth wrong.
 */
int nds_handler(int size, void *req, int *len, void *resp)
{
    int ret = CDS_OK;
    bool ok = false;

    if(size >= 1024)
    {
        /* Note - actually, len checking already done at .SO side... */
        ERR("Exceed limit(%d > %d), don't handle this request\n",
                size, 1024);
        //COMPOSE_RESP(CDS_ERR_REQ_TOOLONG);
        // TODO - DON"T use goto here, to avoid C++ compilor error
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
        printf("==== DUMP the objs... ====\n");
        printf("user:%s, filename:%s\n",
                reqobj.user().c_str(), reqobj.filename().c_str());
        printf("==== END DUMP ====\n");
#endif

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
    mtrace();
#endif

    /* Init Qiniu stuff,as we need it's service */
    Qiniu_Servend_Init(-1);

    //try_upload("/tmp/abc.png");
    //another_test("/tmp/abc.png");

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
    cfg.ac_handler = nds_handler;
    cds_init(&cfg, argc, argv);

    CLEAN_DB_MUTEX();
    FREE_CMSSQL(nds_sql);

    return 0;
}
