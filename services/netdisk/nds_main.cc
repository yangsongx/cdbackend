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
 * call back function, handle obj is NetdiskMessage(NetdiskRequest/NetdiskResponse)
 *
 * return error values if sth wrong.
 */
int nds_handler(int size, void *req, int *len_resp, void *resp)
{
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


    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = nds_handler;
    cfg.ping_handler = ping_nds_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */
    cds_init(&cfg, argc, argv);

    return 0;
}
