#include "RegOperation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;


int RegOperation::sendback_reg_result(int code, const char *errmsg, RegisterResponse *p_obj, int *p_resplen, void *p_respdata)
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

int RegOperation::set_conf(UserRegConfig *c)
{
    m_cfgInfo = c;
    return 0;
}

/**
 * After parsed all request fields, begin handle it one-by-one
 *
 */
int RegOperation::process_register_req(RegisterRequest *reqobj)
{
    if(m_cfgInfo == NULL)
    {
        ERR("Non-Config info found!\n");
        return -1;
    }

    if(user_already_exist(m_cfgInfo->m_Sql, reqobj))
    {
    }
    else
    {
    }

    return 0;
}
