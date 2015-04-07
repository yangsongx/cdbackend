#include <stdio.h>

#ifdef DEBUG
#include <mcheck.h>
#endif

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "cds_public.h"

#include "AttributeModify.pb.h"
#include "AttributeConfig.h"
#include "AttributeOperation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

AttributeConfig g_info;
time_t  g_start;

int attribute_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    bool ok;
    AttributeOperation opr(&g_info);
    AttributeModifyRequest  reqobj;
    AttributeModifyResponse respobj;

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

int ping_attr_handler(int size, void *req, int *len_resp, void *resp)
{
    int ret;
    AttributeOperation opr(&g_info);
    /* Above code is the same as:
     * RegOperation opr;
     * opr.set_conf(&g_inf);
     */
    ret = opr.keep_alive(USERCENTER_ATTR_TBL);
    LOG("PING ALIVE result=%d\n", ret);

    // Next, compose a response payload...
    int *ptr = (int *)resp;

    *ptr = ret;
    *(ptr + 1) = CDS_ATTRIBUTE_MODIFY; // tell ping source that who am I

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

#ifdef DEBUG
    setenv("MALLOC_TRACE", "/tmp/attr.memleak", 1);
    mtrace();
#endif

    time(&g_start);

    if(g_info.parse_cfg("/etc/cds_cfg.xml") != 0)
    {
        ERR("*** Warning Failed init the whole service!\n");
    }

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = attribute_handler;
    cfg.ping_handler = ping_attr_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */

    cds_init(&cfg, argc, argv);

    return 0;
}
