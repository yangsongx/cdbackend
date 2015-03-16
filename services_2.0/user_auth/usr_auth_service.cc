/**
 * User Auth Service (UAS)
 *
 */

#ifdef CHECK_MEM_LEAK
#include <mcheck.h>
#endif

#include <errno.h>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "cds_public.h"

#include "UserAuth.pb.h"
#include "AuthOperation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

UserAuthConfig  g_info;
pthread_mutex_t  uas_mutex;

/**
 * Auth handling entry point
 *
 */
int uas_handler(int size, void *req, int *len_resp, void *resp)
{
    bool ok = false;
    int ret = CDS_OK;
    AuthOperation opr;
    AuthRequest   reqobj;
    AuthResponse  respobj;

    opr.set_conf(&g_info);

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
        // take final auth action!
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

int uas_ping_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret = 0;
    AuthOperation opr;
    opr.set_conf(&g_info);

    ret = opr.keep_alive("uc.uc_session");
    LOG("PING ALIVE result=%d\n", ret);

    *len_resp = 4;
    *(int *)resp = ret;

    return ret;
}

int main(int argc, char **argv)
{
    struct addition_config cfg;

#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/uas.memleak", 1);
    mtrace();
#endif

    if(g_info.parse_cfg("/etc/cds_cfg.xml") != 0)
    {
        ERR("*** Warning Failed init the whole service!\n");
    }

    /* TODO 2015-3-13
     * Actually, the Config obj had a mutex as well,
     * temp use a global mutex here,
     *
     * check the Config.h header comments for more details...
     */
    if(pthread_mutex_init(&uas_mutex, NULL) != 0)
    {
        ERR("*** Warning, failed create mutex IPC objs:%d\n", errno);
    }

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = uas_handler;
    cfg.ping_handler = uas_ping_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */

    cds_init(&cfg, argc, argv);

    return 0;
}
