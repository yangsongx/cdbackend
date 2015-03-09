#include "ActivateOperation.h"
#include "activation_db.h"

int ActivateOperation::set_conf(ActivationConfig *c)
{
    m_cfgInfo = c;
    return 0;
}

int ActivateOperation::begin_activation(ActivateRequest *reqobj, ActivateResponse *respobj, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    LOG("will verify %s(%s)...\n", reqobj->activate_name().c_str(), reqobj->activate_code().c_str());
    ret = verify_activation_code(m_cfgInfo->m_Sql, reqobj);
    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("** failed Serialize result of activation result!\n");
    }

    return ret;
}

int ActivateOperation::compose_result(int code, const char *errmsg, ActivateResponse *p_obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;

    p_obj->set_result_code(code);
    if(code != CDS_OK && errmsg != NULL)
    {
        p_obj->set_extra_msg(errmsg);
    }

    len = p_obj->ByteSize();
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

