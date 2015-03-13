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

int RegOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    RegisterResponse *p_obj = (RegisterResponse *) obj;

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

    return ((obj->SerializeToCodedStream(&cos) == true) ? 0 : -1);
}

/**
 * After parsed all request fields, begin handle it one-by-one
 *
 */
int RegOperation::handling_request(::google::protobuf::Message *reg_req, ::google::protobuf::Message *reg_resp, int *len_resp, void *resp)
{
    int ret = 0;
    RegisterRequest *reqobj = (RegisterRequest*)reg_req;
    RegisterResponse *respobj = (RegisterResponse *) reg_resp;

    if(m_pCfg == NULL)
    {
        ERR("Non-Config info found!\n");
        compose_result(CDS_GENERIC_ERROR, "NULL config obj", respobj, len_resp, resp);
        return CDS_GENERIC_ERROR;
    }

    unsigned long usr_id;
    int status_flag;
    bool existed = user_already_exist(m_pCfg->m_Sql, reqobj, &status_flag, &usr_id);

    // first of all, try re-connect if possible3
    if(existed && (status_flag == -1 && usr_id == (unsigned long) -1))
    {
        ERR("Oh, found MySQL disconnected, try reconnecting...\n");
        if(m_pCfg->reconnect_sql() == 0)
        {
            existed = user_already_exist(m_pCfg->m_Sql, reqobj, &status_flag, &usr_id);
        }
        else
        {
            ERR("Very Bad, reconnectiong still failed\n");
            if(compose_result(CDS_ERR_SQL_DISCONNECTED, NULL, respobj, len_resp, resp) != 0)
            {
                ERR("***failed serialize to resp data\n");
            }
            return CDS_ERR_SQL_DISCONNECTED;
        }

    }

    if(existed && status_flag == 1)
    {
        ret = CDS_ERR_USER_ALREADY_EXISTED;
        compose_result(ret, "User Existed Already", respobj, len_resp, resp);
    }
    else
    {
        if(existed == false)
        {
            // a new user, record it into DB
            ret = add_new_user_entry(m_pCfg->m_Sql, reqobj);
        }
        else
        {
            INFO("Found an existed user in DB(ID-%ld), but not activated, so overwrite it!\n", usr_id);
            ret = overwrite_inactive_user_entry(m_pCfg->m_Sql, reqobj, usr_id);
        }

        if( ret == CDS_OK
                && (reqobj->reg_type() == RegLoginType::MOBILE_PHONE
                        || reqobj->reg_type() == RegLoginType::EMAIL_PASSWD
                        || reqobj->reg_type() == RegLoginType::PHONE_PASSWD))
        {
            // tell a verification code here
            char verifycode[12] = {'8'};
            gen_verifycode(verifycode);
            respobj->set_reg_verifycode(verifycode);

            LOG("==>the verification code=%s\n", verifycode);

            // need update them into DB for later verification...
            ret = record_user_verifiy_code(m_pCfg->m_Sql, reqobj, respobj, (UserRegConfig *)m_pCfg);
        }

        compose_result(ret, NULL, respobj, len_resp, resp);
    }


    return ret;
}
