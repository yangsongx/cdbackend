#include <stdio.h>

#ifdef CHECK_MEM_LEAK
#include <mcheck.h>
#endif

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "cds_public.h"

#include "PasswordManager.pb.h"
#include "PasswordConfig.h"
#include "PasswordOperation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

PasswordConfig  g_info;
time_t  g_start;

int passwd_handler(int size, void *req, int *len_resp, void *resp)
{
    bool ok = false;
    int ret = CDS_OK;
    PasswordOperation opr(&g_info);
    PasswordManagerRequest reqobj;
    PasswordManagerResponse respobj;

    if(size >= DATA_BUFFER_SIZE)
    {
        ERR("Too much long data, ignore it\n");
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
        if(opr.compose_result(CDS_GENERIC_ERROR, "failed parse in protobuf",
                &respobj, len_resp, resp) != 0)
        {
            ERR("** failed seriliaze for the error case\n");
        }
    }

    return ret;
}

int ping_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret = 0;
    PasswordOperation opr(&g_info);

    INFO("Entering the PING ALIVE handler...\n");
    ret = opr.keep_alive(USERCENTER_SESSION_TBL);
    INFO("the PING ALIVE result=%d\n", ret);

    // Next, compose a response payload...
    int *ptr = (int *)resp;

    *ptr = ret;
    *(ptr + 1) = CDS_USR_PASSWD; // tell ping source that who am I

    time_t cur;
    time(&cur);
    cur -= g_start;
    LOG("delta time is (%lu)\n", cur);
    memcpy(ptr + 2, &cur, 8); // 64-bit machine, it is 8-byte for long

    // all length
    *len_resp = (4+4+8);
    return ret;
}


int main(int argc, char **argv)
{
    struct addition_config cfg;

#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/passwdmgr.memleak", 1);
    mtrace();
#endif

    time(&g_start);

    if(g_info.parse_cfg("/etc/cds_cfg.xml") != 0)
    {
        ERR("*** Warning Failed init the whole service!\n");
    }

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = passwd_handler;
    cfg.ping_handler = ping_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */

    cds_init(&cfg, argc, argv);

    return 0;
}
