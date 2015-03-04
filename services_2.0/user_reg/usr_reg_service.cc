/**
 * main entry for User Registration Service(URS)
 *
 */
#ifdef CHECK_MEM_LEAK
#include <mcheck.h>
#endif

#include "cds_public.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "cds_public.h"

#include "UserRegister.pb.h"
#include "UserRegConfig.h"
#include "RegOperation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

pthread_mutex_t  urs_mutex;
UserRegConfig    g_info;

/**
 * Handler entry for a user registration request
 *
 */
int register_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    bool ok;
    RegOperation opr;
    RegisterRequest   reqobj;
    RegisterResponse  respobj;

    opr.set_conf(&g_info);

    if(size >= DATA_BUFFER_SIZE)
    {
        ERR("Too much long data, drop it!\n");
        if(opr.sendback_reg_result(CDS_ERR_REQ_TOOLONG, "too much long",
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
#if 0
        printf("the req name:%s\n", reqobj.reg_name().c_str());
        opr.sendback_reg_result(CDS_ERR_REQ_TOOLONG, "hello world",
                &respobj, len_resp, resp);
#else
        // Parse the request OK. drop into
        // the handling world...
        ret = opr.process_register_req(&reqobj, &respobj, len_resp, resp);
#endif
    }
    else
    {
        ERR("***Failed pare reqobj in protobuf!\n");
        if(opr.sendback_reg_result(CDS_GENERIC_ERROR, "failed parse in protobuf",
                &respobj, len_resp, resp) != 0)
        {
            ERR("** failed seriliaze for the error case\n");
        }
    }

    return ret;
}

int ping_reg_handler(int size, void *req, int *len_resp, void *resp)
{
    return 0;
}

/**
 * World entry point.
 *
 */
int main(int argc, char **argv)
{
    struct addition_config cfg;

#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/urs.memleak", 1);
    mtrace();
#endif


    if(g_info.init("/etc/cds_cfg.xml") != 0)
    {
        ERR("*** Warning Failed init the whole service!\n");
    }

    if(pthread_mutex_init(&urs_mutex, NULL) != 0)
    {
       ERR("*** Warning, failed create mutex IPC objs:%d\n", errno);
    }


    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = register_handler;
    cfg.ping_handler = ping_reg_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */

    cds_init(&cfg, argc, argv);

    return 0;
}
