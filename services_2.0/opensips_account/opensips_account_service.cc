/**
 * default port set as 12003 (NOT using legacy's 12002)
 *
 */
#include <stdio.h>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "cds_public.h"
#include "SipAccount.pb.h"

using namespace google::protobuf::io;

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
    LOG("WOW, a PING for SIPS...\n");

    //ret = peek_db(msql);
    
    return ret;
}

int opas_handler(int size, void *req, int *len, void *resp)
{
    bool               ok = false;
    unsigned short     length;
    SipAccountRequest  reqobj;
    SipAccountResponse sa_response;


    if(size >= DATA_BUFFER_SIZE)
    {
        ERR("exceed max len(%d)!\n", size);
        sa_response.set_code(CDS_ERR_REQ_TOOLONG);
        length= sa_response.ByteSize();
        *len = (length + 2);
        ArrayOutputStream aos(resp, *len);
        CodedOutputStream  cos(&aos);
        cos.WriteRaw(&length, sizeof(length));
        ok = sa_response.SerializeToCodedStream(&cos);
        if(ok == true)
        {
            printf("True for serialize\n");
        }
        else
        {
            printf("false for serialize\n");
        }
    }

    ArrayInputStream in(req, size);
    CodedInputStream is(&in);
    ok = reqobj.ParseFromCodedStream(&is);
    if(ok == true)
    {
        printf("the packed user name=%s\n", reqobj.user_name().c_str());
        sa_response.set_code(0);
        sa_response.set_user_credential("hello, moto");
        if(pack_response_data(0, NULL, &sa_response, len, resp) == 0)
        {
            LOG("Serialize OK\n");
        }
        else
        {
            ERR("SerializeToCodedStream failed\n");
        }
    }
    else
    {
        ERR("***failed parse the protobuf data!\n");
    }

    return 0;
}

int main(int argc, char **argv)
{
    struct addition_config cfg;

#ifdef MEM
    setenv("MALLOC_TRACE", "/tmp/opas.memleak", 1);
    mtrace();
#endif

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = opas_handler;
    cfg.ping_handler = opas_ping;
    cfg.ac_lentype = LEN_TYPE_BIN;
    cds_init(&cfg, argc, argv);

    return 0;
}
