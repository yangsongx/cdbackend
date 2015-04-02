/**
 * SHOULD WE re-use legacy's 12002 port for this 2.0 arch service?
 *
 */
#include <stdio.h>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "cds_public.h"
#include "SipAccount.pb.h"
#include "SipConfig.h"
#include "SipOperation.h"

using namespace google::protobuf::io;

SipConfig g_info;
time_t g_start;

int pack_response_data(int code, const char *errmsg, SipAccountResponse *p_obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;

    p_obj->set_code(code);

    len = p_obj->ByteSize();

    *p_resplen = (len + 2);
    printf("Full length=%d\n", *p_resplen);
    ArrayOutputStream aos(p_respdata, *p_resplen);
    CodedOutputStream cos(&aos);
    cos.WriteRaw(&len, sizeof(len));

    return ((p_obj->SerializeToCodedStream(&cos)) == true ? 0 : -1);
}

int opas_ping(int size, void *req, int *len, void *resp)
{
    int ret = 0;
    SipOperation opr(&g_info);

    LOG("WOW, a PING for SIPS...\n");
    ret = opr.keep_alive(USERCENTER_MAIN_TBL);
    LOG("PING ALIVE result=%d\n", ret);

    int *ptr = (int *)resp;

    *ptr = ret;
    *(ptr + 1) = CDS_SIPS; // tell ping source that who am I

    time_t cur;
    time(&cur);
    cur -= g_start;
    INFO("delta time is (%lu)\n", cur);
    memcpy(ptr + 2, &cur, 8);

    *len = (4+4+8);
    
    return ret;
}

int opas_handler(int size, void *req, int *len_resp, void *resp)
{
    int                ret = CDS_OK;
    bool               ok = false;
    SipAccountRequest  reqobj;
    SipAccountResponse respobj;
    SipOperation       opr(&g_info);

    if(size >= DATA_BUFFER_SIZE)
    {
        ERR("exceed max len(%d)!\n", size);
        if(opr.compose_result(CDS_ERR_REQ_TOOLONG, NULL,
                    &respobj, len_resp, resp) != 0)
        {
            ERR("***Failed Serialize the too-long error data\n");
        }
        return CDS_ERR_REQ_TOOLONG;
    }

    ArrayInputStream in(req, size);
    CodedInputStream is(&in);
    ok = reqobj.ParseFromCodedStream(&is);
    if(ok == true)
    {
        ret = opr.handling_request(&reqobj, &respobj, len_resp, resp);
    }
    else
    {
        ERR("***failed parse the protobuf data!\n");
        if(opr.compose_result(CDS_GENERIC_ERROR, NULL,
                    &respobj, len_resp, resp) != 0)
        {
            ERR("** failed seriliaze for the error case\n");
        }

        ret = CDS_GENERIC_ERROR;
    }

    return ret;
}

int main(int argc, char **argv)
{
    struct addition_config cfg;

#ifdef MEM
    setenv("MALLOC_TRACE", "/tmp/opas.memleak", 1);
    mtrace();
#endif

    time(&g_start);

    if(g_info.parse_cfg("/etc/cds_cfg.xml") != 0)
    {
        ERR("*** Warning Failed init the whole service!\n");
    }

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = opas_handler;
    cfg.ping_handler = opas_ping;
    cfg.ac_lentype = LEN_TYPE_BIN;

    cds_init(&cfg, argc, argv);

    return 0;
}
