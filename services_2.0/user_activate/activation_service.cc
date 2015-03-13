/**
 * main entry for Activation Service(acts)
 *
 */
#ifdef CHECK_MEM_LEAK
#include <mcheck.h>
#endif

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "cds_public.h"

#include "UserActivation.pb.h"
#include "ActivationConfig.h"
#include "ActivateOperation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

pthread_mutex_t  acts_mutex;

ActivationConfig g_info;

/**
 * Handler entry for a user registration request
 *
 */
int activate_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    bool ok;
    ActivateOperation opr;
    ActivateRequest   reqobj;
    ActivateResponse  respobj;

    opr.set_conf(&g_info);

    if(size >= DATA_BUFFER_SIZE)
    {
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
        // Parse the request OK. drop into
        // the handling world...
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

    return ret;
}

int ping_acts_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret = 0;
    ActivateOperation opr;
    opr.set_conf(&g_info);

    ret = opr.keep_alive(ACTIVATION_MAIN_TABLE);
    LOG("PING ALIVE result=%d\n", ret);

    *len_resp = 4;
    *(int *)resp = ret;

    return ret;
}

/**
 * World entry point.
 *
 */
int main(int argc, char **argv)
{
    struct addition_config cfg;

#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/acts.memleak", 1);
    mtrace();
#endif

    if(g_info.parse_cfg("/etc/cds_cfg.xml") != 0)
    {
        ERR("*** Warning Failed init the whole service!\n");
    }

    if(pthread_mutex_init(&acts_mutex, NULL) != 0)
    {
       ERR("*** Warning, failed create mutex IPC objs:%d\n", errno);
    }


    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = activate_handler;
    cfg.ping_handler = ping_acts_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */

    cds_init(&cfg, argc, argv);

    return 0;
}
