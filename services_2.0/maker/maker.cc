#include <stdio.h>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "cds_public.h"

#include "MakerOperation.h"

using namespace std;
using namespace google::protobuf::io;
using namespace com::caredear;

/* a start time entry, used for keep alive report the
 * program running life time */
time_t  g_start;

MakerConfig  g_info;

/**
 * Handling entry pointer
 *
 */
int maker_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    bool ok = false;
    MakerOperation opr(&g_info);
    MakerRequest  reqobj;
    MakerResponse respobj;

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

        ret = CDS_ERR_REQ_PROTOBUF_INCORRECT;
        if(opr.compose_result(CDS_ERR_REQ_PROTOBUF_INCORRECT, "failed parse in protobuf",
                &respobj, len_resp, resp) != 0)
        {
            ERR("** failed seriliaze for the error case\n");
        }
    }

    return ret;
}

int ping_maker_handler(int size, void *req, int *len_resp, void *resp)
{
    // TODO currently not implemented yet
    return 0;
}

int main(int argc, char **argv)
{
    struct addition_config cfg;

    if(g_info.parse_cfg("/etc/cds_cfg.xml") != 0)
    {
        ERR("*** Warning Failed init the config XML file!\n");
    }

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = maker_handler;
    cfg.ping_handler = ping_maker_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */
    cds_init(&cfg, argc, argv);

    return 0;
}
