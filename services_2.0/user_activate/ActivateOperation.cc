#include "ActivateOperation.h"


/**
 * Override the handling entry point
 *
 */
int ActivateOperation::handling_request(::google::protobuf::Message *reqobj, ::google::protobuf::Message *respobj, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    ActivateRequest *req = (ActivateRequest *)reqobj;

    LOG("will verify %s(%s)...\n", req->activate_name().c_str(), req->activate_code().c_str());
    ret = verify_activation_code(m_pCfg->m_Sql, req);
    if(ret == CDS_ERR_SQL_DISCONNECTED)
    {
        if(m_pCfg->reconnect_sql() == 0)
        {
            ret = verify_activation_code(m_pCfg->m_Sql, req);
        }
    }

    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("** failed Serialize result of activation result!\n");
    }

    return ret;
}

int ActivateOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *p_obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    ActivateResponse *resp = (ActivateResponse *)p_obj;

    resp->set_result_code(code);
    if(code != CDS_OK && errmsg != NULL)
    {
        resp->set_extra_msg(errmsg);
    }

    len = resp->ByteSize();
    if(len >= DATA_BUFFER_SIZE)
    {
        ERR("Attention, exceed the max len, set it to a safe len\n");
        len = 4;
    }

    *p_resplen = (len + 2);
    ArrayOutputStream aos(p_respdata, *p_resplen);
    CodedOutputStream cos(&aos);
    // add the leading-length
    cos.WriteRaw(&len, sizeof(len));

    return ((p_obj->SerializeToCodedStream(&cos) == true) ? 0 : -1);
}

