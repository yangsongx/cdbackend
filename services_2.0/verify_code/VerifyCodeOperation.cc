/**
 *
 * \history
 * [2015-04-08] Refractor the code be consistent with all service modules
 */
#include "VerifyCodeOperation.h"

uint64_t VerifyCodeOperation::m_Cid;
int VerifyCodeOperation::m_result;

int VerifyCodeOperation::cb_get_cid(MYSQL_RES *mresult)
{
    MYSQL_ROW row;

    m_Cid = (uint64_t) -1;

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        if(row[0] != NULL)
        {
            m_Cid = atol(row[0]);
        }
    }

    return 0;
}

int VerifyCodeOperation::cb_check_passwd(MYSQL_RES *mresult)
{
    MYSQL_ROW row;

    m_result = 0;
    if(!mresult)
    {
        return -1;
    }

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        // NON-NULL here means the password is correct
        if(mysql_num_rows(mresult) != 1)
        {
            INFO("Warning, CID matching returned multiple result, please check the DB!\n");
        }
        m_result = 1;
    }

    return 0;
}

/**
 * Save verify code to DB
 *
 */
int VerifyCodeOperation::record_code_to_db(VerifyRequest *reqobj, const char *code)
{
    int ret;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET accode=\'%s\',codetime=UNIX_TIMESTAMP(date_add(now(), interval %d second)) "
            "WHERE id=(SELECT caredearid FROM %s WHERE ticket=\'%s\')",
            USERCENTER_MAIN_TBL, code, ((VerifyCodeConfig *)m_pCfg)->m_iMobileVerifyExpir,
            USERCENTER_SESSION_TBL, reqobj->ticket().c_str());

    ret = sql_cmd(sqlcmd, NULL);

    return ret;
}

int VerifyCodeOperation::check_password(VerifyRequest *reqobj, VerifyResponse *respobj)
{
    uint64_t cid;
    int ret;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT caredearid FROM %s WHERE ticket=\'%s\'",
            USERCENTER_SESSION_TBL, reqobj->ticket().c_str());

    ret = sql_cmd(sqlcmd, cb_get_cid);
    if(ret == CDS_OK)
    {
        if(m_Cid != (uint64_t) -1)
        {
            //here the m_Cid store target user's ID
            char tmpdata[64];
            char md5data[64];
            snprintf(tmpdata, sizeof(tmpdata),
                    "%s%lu", reqobj->has_passwd() ? reqobj->passwd().c_str() : "", m_Cid);
            get_md5(tmpdata, strlen(tmpdata), md5data);
            // compare with DB
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE id=%lu AND loginpassword=\'%s\'",
                    USERCENTER_MAIN_TBL, m_Cid, md5data);
            ret = sql_cmd(sqlcmd, cb_check_passwd);
            if(ret == CDS_OK && m_result == 1)
            {
                INFO("User's password is correct\n");
                ret = CDS_OK;
            }
        }
        else
        {
            // seems didn't find the valide user entry, set it as failure case
            ret = CDS_ERR_UMATCH_USER_INFO;
        }
    }

    return ret;
}

int VerifyCodeOperation::gen_verifycode(VerifyRequest *reqobj, VerifyResponse *respobj)
{
    int ret;
    char code[32] = {'8'};
    // first of all, new an UUID
    gen_random_code(code);

    // next, update it to DB
    ret = record_code_to_db(reqobj, code);
    // last, tell the caller with this code
    if(ret == CDS_OK)
    {
        INFO("tell caller the verify code is:%s\n", code);
        respobj->set_extra_msg(code);
    }

    return ret;
}

/**
 * Operation halding entry point
 *
 */
int VerifyCodeOperation::handling_request(::google::protobuf::Message *preqobj, ::google::protobuf::Message *prespobj, int *len_resp, void *resp)
{
    int ret;
    VerifyRequest  *reqobj = (VerifyRequest *)preqobj;
    VerifyResponse *respobj = (VerifyResponse *)prespobj;

    switch(reqobj->type())
    {
        case VerifyType::MOBILE_PHONE:
        case VerifyType::EMAIL:
            ret = gen_verifycode(reqobj, respobj);
            break;

        case VerifyType::CHECK_PASSWD_VALIDATION:
            ret = check_password(reqobj, respobj);
            break;

        default:
            break;
    }

    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for login result\n");
    }

    return ret;
}

int VerifyCodeOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *p_vcsobj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    VerifyResponse *p_obj = (VerifyResponse *) p_vcsobj;

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
