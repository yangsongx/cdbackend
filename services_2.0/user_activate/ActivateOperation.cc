#include "ActivateOperation.h"

// 0 means failed pass the verify, 1 means pass
int ActivateOperation::m_verify_result = 0;
uint64_t ActivateOperation::m_cid = -1;

int ActivateOperation::cb_verify_code(MYSQL_RES *mresult)
{
    MYSQL_ROW row;

    if(!mresult)
    {
        return -1;
    }

    m_verify_result = 0;

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        if(mysql_num_rows(mresult) != 1)
        {
            INFO("Warning, multiple result meet for the verify code\n");
        }

        LOG("pass, need update the status falg in DB\n");
        m_verify_result = 1;
    }

    return 0;
}


/**
 *
 * In this callback, the m_verify_result act as CDS_XXX constant,
 * which store the further of why code failure.
 */
int ActivateOperation::cb_check_code_failure(MYSQL_RES *mresult)
{
    MYSQL_ROW row;

    if(!mresult)
    {
        return -1;
    }

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        LOG("the code is expired\n");
        m_verify_result = CDS_ERR_CODE_EXPIRED;
    }
    else
    {
        m_verify_result = CDS_ERR_INCORRECT_CODE;
    }

    return 0;
}

int ActivateOperation::cb_get_cid(MYSQL_RES *mresult)
{
    MYSQL_ROW row;

    if(!mresult)
    {
        return -1;
    }

    m_cid = (uint64_t)-1;

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        if(mysql_num_rows(mresult) > 1)
        {
            ERR("Warning, check the CID SHOULD only one single result!\n");
        }

        if(row[0] != NULL)
        {
            m_cid = atol(row[0]);
        }
    }

    return 0;
}

int ActivateOperation::verify_activation_code(ActivateRequest *reqobj, ActivateResponse *respobj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];

    switch(reqobj->activate_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE usermobile=\'%s\' AND accode=\'%s\' AND codetime-UNIX_TIMESTAMP(NOW())>0",
                USERCENTER_MAIN_TBL,
                reqobj->activate_name().c_str(),
                reqobj->activate_code().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE email=\'%s\' AND accode=\'%s\' AND codetime-UNIX_TIMESTAMP(NOW())>0",
                USERCENTER_MAIN_TBL,
                reqobj->activate_name().c_str(),
                reqobj->activate_code().c_str());
            break;

        default:
            ERR("**Warning, un-support activate_type\n");
            break;
    }

    ret = sql_cmd(sqlcmd, cb_verify_code);
    if(ret == CDS_OK)
    {
        if(m_verify_result == 1)
        {
            // as pass, set status be "activated"(1)
            ret = set_user_status_flag_to_db(reqobj, respobj);
        }
        else
        {
            // For the blank result case, there are two possible scenario:
            // - code incorrect
            // - code correct, but time expired
            //
            // So, we need take extra checking for this case...
            ret = check_further_code_correctness(reqobj);
            LOG("the verify code failured reason:%d\n", ret);
        }
    }

    return ret;
}

/**
 *
 *
 * return the why verify code failure reason
 */
int ActivateOperation::check_further_code_correctness(ActivateRequest *reqobj)
{
    int ret = CDS_OK;
    int further_failure = -1;
    char sqlcmd[1024];

    switch(reqobj->activate_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE usermobile=\'%s\' AND accode=\'%s\'",
                USERCENTER_MAIN_TBL,
                reqobj->activate_name().c_str(),
                reqobj->activate_code().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE email=\'%s\' AND accode=\'%s\'",
                USERCENTER_MAIN_TBL,
                reqobj->activate_name().c_str(),
                reqobj->activate_code().c_str());
            break;

        default:
            break;
    }

    ret = sql_cmd(sqlcmd, cb_check_code_failure);
    if(ret == CDS_OK)
    {
        further_failure = m_verify_result;
    }

    return further_failure;
}

int ActivateOperation::get_user_cid_from_db(ActivateRequest *reqobj, ActivateResponse *respobj)
{
    int ret;
    char sqlcmd[1024];

    switch(reqobj->activate_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE usermobile=\'%s\'",
                USERCENTER_MAIN_TBL,
                reqobj->activate_name().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE email=\'%s\'",
                USERCENTER_MAIN_TBL,
                reqobj->activate_name().c_str());
            break;

        default:
            ERR("**Warning, un-support activate_type\n");
            break;
    }

    ret = sql_cmd(sqlcmd, cb_get_cid);
    if(ret == CDS_OK)
    {
        INFO("get the \'%s\' CID as %lu\n", reqobj->activate_name().c_str(), m_cid);
        respobj->set_caredear_id(m_cid);
    }

    return ret;
}

int ActivateOperation::set_user_status_flag_to_db(ActivateRequest *reqobj, ActivateResponse *respobj)
{
    int ret;
    char sqlcmd[1024];

    switch(reqobj->activate_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "UPDATE %s SET status=1 WHERE usermobile=\'%s\'",
                USERCENTER_MAIN_TBL,
                reqobj->activate_name().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "UPDATE %s SET status=1 WHERE email=\'%s\'",
                USERCENTER_MAIN_TBL,
                reqobj->activate_name().c_str());
            break;

        default:
            ERR("**Warning, un-support activate_type\n");
            break;
    }

    ret = sql_cmd(sqlcmd, NULL);
    if(ret == CDS_OK)
    {
        INFO("Set the \'%s\' with active flag [OK]\n",
               reqobj->activate_name().c_str());

        // for OK case, need tell caller the user's CID
        ret = get_user_cid_from_db(reqobj, respobj); 
    }

    return ret;
}

/**
 * Override the handling entry point
 *
 */
int ActivateOperation::handling_request(::google::protobuf::Message *reqobj, ::google::protobuf::Message *respobj, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    ActivateRequest *req = (ActivateRequest *)reqobj;
    ActivateResponse *ind = (ActivateResponse *)respobj;

    LOG("will verify %s(%s)...\n", req->activate_name().c_str(), req->activate_code().c_str());
    ret = verify_activation_code(req, ind);

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

