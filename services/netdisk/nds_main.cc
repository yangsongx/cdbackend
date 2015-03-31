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

#ifdef DEBUG
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

#include "NetdiskConfig.h"
#include "NetdiskOperation.h"

/* Now, we're going to C++ world */
using namespace std;
using namespace google::protobuf::io;
using namespace com::caredear;

static MYSQL *nds_sql = NULL;

NetdiskConfig g_info;
time_t        g_ndsStart;

/* QINIU SDK KEYS... */
Qiniu_Client qn;
//const char *QINIU_ACCESS_KEY;
//const char *QINIU_SECRET_KEY;
const char *qiniu_bucket;
const char *qiniu_domain;
unsigned int qiniu_expires = 2592000; // 30 dyas by default
unsigned int qiniu_quota = 10000; // set 10000 here just for debug..., correct should go into config...


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
 * this is a handler for ping alive package, if sth wrong happended,
 * we need probably send out an email for this!
 *
 * return value is also CDS_XXX constants.
 */
int ping_nds_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret = 0;
    NetdiskOperation opr(&g_info);

    INFO("PING ALIVE for nds...\n");
    ret = opr.keep_alive(NETDISK_FILE_TBL, "ID");
    INFO("PING finished with %d\n", ret);

    int *ptr = (int *)resp;
    *ptr = ret;
    *(ptr + 1) = CDS_NETDISK;
    time_t cur;
    time(&cur);
    cur -= g_ndsStart;
    memcpy(ptr + 2, &cur, 8);

    *len_resp = (4+4+8);

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
    NetdiskOperation opr(&g_info);
    NetdiskRequest  reqobj;
    NetdiskResponse respobj;
#if 1
    if(size >= DATA_BUFFER_SIZE)
    {
        /* Note - actually, len checking already done at .SO side... */
        ERR("Too much long data, drop it!\n");
        if(opr.compose_result(CDS_ERR_REQ_TOOLONG, "too much long",
                    &respobj, len_resp, resp) != 0)
        {
            ERR("***Failed Serialize the too-long error data\n");
        }

        return CDS_ERR_REQ_TOOLONG;
    }

    ArrayInputStream in(req, size);
    CodedInputStream is(&in);

    ok = reqobj.ParseFromCodedStream(&is);
    if(ok)
    {
        ret = opr.handling_request(&reqobj, &respobj, len_resp, resp);
    }
    else
    {
        ERR("***Failed pare reqobj in protobuf!\n");
        ret = CDS_ERR_REQ_PROTOBUF_INCORRECT;
        if(opr.compose_result(CDS_ERR_REQ_PROTOBUF_INCORRECT, "failed parse in protobuf",
                &respobj, len_resp, resp) != 0)
        {
            ERR("** failed seriliaze for the error case\n");
        }
    }
#else
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
    struct addition_config cfg;
#ifdef DEBUG
    setenv("MALLOC_TRACE", "/tmp/nds.memleak", 1);
    mtrace();
#endif

    time(&g_ndsStart);

    if(g_info.parse_cfg("/etc/cds_cfg.xml") != 0)
    {
        ERR("*** Warning Failed init the config XML file!\n");
    }

    // TODO, below SQL MUTEX is not used future....
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

    // TODO, SQL mUTEX not used future
    CLEAN_DB_MUTEX();
    FREE_CMSSQL(nds_sql);

    return 0;
}
