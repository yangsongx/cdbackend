#include "RegOperation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

/**
 * Generate a random 6-digit number, used for email/sms verify code
 *
 */
int RegOperation::gen_verifycode(char *result)
{
    char a[2];
    uuid_t id;
    char a1,a2,a3,a4,a5,a6;

    uuid_generate(id);

    // extract first-two and last-one digit hex as verifiy code
    *(unsigned short *) a  = (unsigned short )(id[0] << 16 | id[1] << 8 | id[15]);

    a1 = (a[0] & 0x0F);
    a2 = ((a[0] & 0xF0) >> 4);
    a3 = (a[1] & 0x0F);
    a4 = ((a[1] & 0xF0) >> 4);
    a5 = (a[2] & 0x0F);
    a6 = ((a[2] & 0xF0) >> 4);

    sprintf(result, "%c%c%c%c%c%c",
             a1 > 9 ? (a1 - 6) + 0x30 : a1 + 0x30,
             a2 > 9 ? (a2 - 6) + 0x30  : a2 + 0x30,
             a3 > 9 ? (a3 - 6) +0x30 : a3 + 0x30,
             a4 > 9 ? (a4 - 6) + 0x30 : a4 + 0x30,
             a5 > 9 ? (a5 - 6) +0x30 : a5 + 0x30,
             a6 > 9 ? (a6 - 6) + 0x30 : a6 + 0x30
            );

    return *(int *) a;
}

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
int RegOperation::process_register_req(RegisterRequest *reqobj, RegisterResponse *respobj, int *len_resp, void *resp)
{
    int ret = 0;

    if(m_cfgInfo == NULL)
    {
        ERR("Non-Config info found!\n");
        return CDS_GENERIC_ERROR;
    }

    if(user_already_exist(m_cfgInfo->m_Sql, reqobj))
    {
        ret = CDS_ERR_USER_ALREADY_EXISTED;
        sendback_reg_result(ret, "User Existed Already", respobj, len_resp, resp);
    }
    else
    {
        // a new user, record it into DB
        ret = add_new_user_entry(m_cfgInfo->m_Sql, reqobj);

        if( ret == CDS_OK
                && (reqobj->reg_type() == Regtype::MOBILE_PHONE
                        || reqobj->reg_type() == Regtype::EMAIL))
        {
            // tell a verification code here
            char verifycode[12] = {'8'};
            gen_verifycode(verifycode);
            respobj->set_reg_verifycode(verifycode);
        }

        sendback_reg_result(ret, NULL, respobj, len_resp, resp);
    }


    return ret;
}
