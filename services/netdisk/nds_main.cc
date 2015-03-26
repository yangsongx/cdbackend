/**
 * NetDisk Service(NDS) code
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
unsigned int qiniu_quota = 10000; // set 10000 here just for debug..., correct should go into config...


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

            get_node_via_xpath("/config/netdisk/qiniu/quota", ctx,
                    domain, sizeof(domain));
            qiniu_quota = atoi(domain);

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
int get_download_url(const char *md5, NetdiskResponse *p_resp)
{
    int ret = -1;

    char domain_str[512]; // FIXME domain should never had much long str...
    Qiniu_RS_GetPolicy get_policy;
    get_policy.expires = qiniu_expires;

    snprintf(domain_str, sizeof(domain_str),
            "%s%s", qiniu_bucket, qiniu_domain);

    char *baseurl = Qiniu_RS_MakeBaseUrl(domain_str, md5);
    char *download_url = Qiniu_RS_GetPolicy_MakeRequest(&get_policy, baseurl, NULL);

    if(download_url != NULL)
    {
        ret = 0;
        LOG("download url=%s\n", download_url);
        p_resp->set_downloadurl(download_url);
        Qiniu_Free(download_url);
    }

    Qiniu_Free(baseurl);

    return ret;
}



/**
 * Wraper for send back data, via protobuf
 *
 *@result_code: the CDS_XXX error code
 *@p_respdata: the raw response buffer(embeded into CDS)
 *
 * reutrn 0 for successful, otherwise return -1
 */
int sendback_response(int result_code, const char *errmsg, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    unsigned short len;

    p_ndr->set_result_code(result_code);

    /* the error message is only valid for error case */
    if(result_code != CDS_OK && errmsg != NULL)
    {
        p_ndr->set_errormsg(errmsg);
    }

    len = p_ndr->ByteSize();
    LOG("the response payload len=%d\n", len);

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
    return ((p_ndr->SerializeToCodedStream(&cos) == true) ? 0 : -1);
}

/**
 * Created the upload policy
 *
 */
int generate_upload_token(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    Qiniu_RS_PutPolicy put_policy;

    // per req creates one
    Qiniu_Client_InitMacAuth(&qn, 1024, NULL);

    memset(&put_policy, 0x00, sizeof(put_policy));
    put_policy.scope = qiniu_bucket;
    put_policy.expires = qiniu_expires;

    char *uptoken= Qiniu_RS_PutPolicy_Token(&put_policy, NULL);
    if(uptoken == NULL)
    {
        ERR("*** got NULL mem pointer");
        sendback_response(CDS_ERR_NOMEMORY, "no memory", p_ndr, p_resplen, p_respdata);
        return CDS_ERR_NOMEMORY;
    }

    LOG("upload token:%s\n", uptoken);
    p_ndr->set_uploadtoken(uptoken);

    // compose the response data....
    if(sendback_response(CDS_OK, NULL, p_ndr, p_resplen, p_respdata) != 0)
    {
        ERR("Warning, failed serialize upload response data\n");
    }

    // DO NOT FORGET release resource...
    Qiniu_Free(uptoken);
    Qiniu_Client_Cleanup(&qn);

    return CDS_OK;
}

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
int simulate_client_upload(NetdiskResponse *p_ndr)
{
    // get all file list under a test dir...
    const char *d = "/tmp/nds/";
    DIR *dir;
    struct dirent *ent;
    char buf[512];
    char md5[16];
    char strmd5[34];
    Qiniu_Error err;
    Qiniu_Io_PutRet putret;

    Qiniu_Client_InitMacAuth(&qn, 1024, NULL);

    dir = opendir(d);
    if(!dir)
    {
        ERR("** failed open the \'%s\' dir:%d\n",
                d, errno);
        return -1;
    }

    while((ent = readdir(dir)) != NULL)
    {
        if(!(ent->d_type & DT_DIR))
        {
            snprintf(buf, sizeof(buf), "%s%s",
                    d, ent->d_name);

            memset(md5, 0x00, sizeof(md5));
            memset(strmd5, 0x00, sizeof(strmd5));
            get_md5(buf, md5);
            for(int i = 0; i < 16; i ++)
            {
                char tmp[12];
                sprintf(tmp, "%02x", (unsigned char)md5[i]);
                strcat(strmd5, tmp);
            }

            LOG("The file:%s, md5:%s\n", buf, strmd5);
            // upload this guy to netdisk...
#if 1
            err = Qiniu_Io_PutFile(&qn, &putret, p_ndr->uploadtoken().c_str(),
                    strmd5, buf, NULL);
            LOG("the http upload status code:%d\n", err.code);
            if(err.code != 200)
            {
                LOG("the failure msg:%s\n", err.message);
            }
            else
            {
                LOG("\n\nUpload %s with MD5 %s [OK]\n",
                        buf, strmd5);
            }
#endif
        }
    }

    closedir(dir);

    Qiniu_Client_Cleanup(&qn);

    return 0;
}

/**
 * this is a handler for ping alive package, if sth wrong happended,
 * we need probably send out an email for this!
 *
 * return value is also CDS_XXX constants.
 */
int ping_nds_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret = 0;

    ret = keep_nds_db_connected(nds_sql);
    LOG("nds ping result=%d\n", ret);

    /* for ping case, we don't need Protobuf's sendbackresponse...
     */
    *len_resp = 4;
    *(int *)resp = ret;

    return ret;
}

/**
 * handler for user try uploading a file to netdisk. This function will just return
 * upload token to caller.
 *
 */
int do_upload(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    int ret = CDS_OK;

    p_ndr->set_opcode(UPLOADING);

    ret = preprocess_upload_req(nds_sql, p_obj);
    if(ret == CDS_ERR_SQL_DISCONNECTED) {
        ERR("SQL idle timeout, need reconnet\n");
        mysql_close(nds_sql);

        nds_sql = GET_CMSSQL(&sql_cfg);
        if(nds_sql == NULL) {
            ERR("failed reconnect to SQL\n");
            return CDS_ERR_SQL_DISCONNECTED;

        } else {
            INFO("Reconnect to netdisk DB [OK]\n");

            ret = preprocess_upload_req(nds_sql, p_obj);
        }
       
    }

    switch(ret)
    {
        case CDS_ERR_SQL_EXECUTE_FAILED:
            break;

        case CDS_FILE_ALREADY_EXISTED:
            if(sendback_response(ret, "already existed", p_ndr, p_resplen, p_respdata) != 0)
            {
                ERR("Warning, failed serialize already existed data\n");
            }
            return ret;

        default:
            ; // continue...
    }

    if(exceed_quota(nds_sql, p_obj) != 0)
    {
        INFO("User exceed the quota!");
        // Exceed quota!
        if(sendback_response(CDS_ERR_EXCEED_QUOTA, "exceed quota", p_ndr, p_resplen, p_respdata) != 0)
        {
            ERR("Warning, failed serialize exceed quota data\n");
        }

        return CDS_ERR_EXCEED_QUOTA;
    }

    ret = generate_upload_token(p_obj, p_ndr, p_resplen, p_respdata);

#if 0
    if(in_debug_mode())
    {
        // debug mode, we will upload file by ourself
        simulate_client_upload(p_ndr);
    }
#endif

    return ret;
}

/**
 * update the DB as we found APK upload the file successfully
 *
 */
int complete_upload(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    int ret = CDS_OK;

    p_ndr->set_opcode(UPLOADED);
    ret = update_user_uploaded_data(nds_sql, p_obj);
    if(ret != CDS_OK)
    {

        if(ret == CDS_ERR_SQL_DISCONNECTED) {
            ERR("SQL idle timeout, need reconnet\n");
            mysql_close(nds_sql);
            nds_sql = GET_CMSSQL(&sql_cfg);

            if(nds_sql != NULL) {
                INFO("Reconnect to netdisk DB [OK]\n");

                ret = update_user_uploaded_data(nds_sql, p_obj);
            } else {
                ERR("failed reconnect to SQL\n");
                return CDS_ERR_SQL_DISCONNECTED;
            }

        } else {
        ERR("** failed update the DB\n");
        }
    }

    // Actually, we need few UPLOADED action result here...
    if(sendback_response(ret, NULL, p_ndr, p_resplen, p_respdata) != 0)
    {
        ERR("Warning, failed serialize the update DB response\n");
    }

    return ret;
}

/**
 * o Mapp the file to MD5 key in Qiniu
 * o Try delete to Qiniu
 * o update our server's DB
 */
int do_deletion(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    int ret = CDS_OK;

    p_ndr->set_opcode(DELETE);

    ret = remove_file_from_db(nds_sql, p_obj);

    if(sendback_response(ret, NULL, p_ndr, p_resplen, p_respdata) != 0)
    {
        ERR("Warning, failed serialize response for deletion req\n");
    }

    return ret;
}

/**
 * handler for user want to download a netdisk file...
 *
 */
int do_download(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    int ret = CDS_OK;
    char md5[34];

    p_ndr->set_opcode(DOWNLOADURL);

    ret = get_netdisk_key(nds_sql, p_obj, md5);

    if(ret == CDS_ERR_SQL_DISCONNECTED) {
        ERR("SQL idle timeout, need reconnet\n");
        mysql_close(nds_sql);
        nds_sql = GET_CMSSQL(&sql_cfg);
        if(nds_sql == NULL) {
            ret = -1; // to let later code send back error
        } else {
            INFO("Reconnect to netdisk DB [OK]\n");

            ret = get_netdisk_key(nds_sql, p_obj, md5);
        }
    }

    if(ret != 0)
    {
        ret = CDS_ERR_FILE_NOTFOUND;
        if(sendback_response(ret, "can't find file md5", p_ndr, p_resplen, p_respdata) != 0)
        {
            ERR("Warning, failed serialize download file not found error data\n");
        }

        return ret;
    }

    if(get_download_url(md5, p_ndr) != 0)
    {
        ret = CDS_ERR_NO_RESOURCE;
        if(sendback_response(ret, "failed compose downloadurl", p_ndr, p_resplen, p_respdata) != 0)
        {
            ERR("Warning, failed serialize failure in composing downloadurl data\n");
        }

        return ret;
    }

    if(sendback_response(ret, NULL, p_ndr, p_resplen, p_respdata) != 0)
    {
        ERR("Warning, failed serialize download response data\n");
    }

    return ret;
}

int do_sharing(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    int ret = CDS_OK;
    char md5[42]; // MD5SUM is actually 33(including '\0') chars...

    p_ndr->set_opcode(SHARE);

    if(share_file(nds_sql, p_obj, md5) != 0)
    {
        ret = CDS_ERR_SQL_EXECUTE_FAILED;
    }

    return ret;
}

int do_rename(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    int ret = CDS_OK;


    //TODO code

    return ret;
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
    cfg.ping_handler = ping_nds_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */
    cds_init(&cfg, argc, argv);

    CLEAN_DB_MUTEX();
    FREE_CMSSQL(nds_sql);

    return 0;
}
