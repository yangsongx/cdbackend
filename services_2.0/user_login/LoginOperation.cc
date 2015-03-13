#include "LoginOperation.h"
#include "usr_login_db.h"

/**
 * Inner wrapper util for mem+DB set
 *
 * Need obsolete existed token if found.
 */
int LoginOperation::update_usercenter_session(LoginRequest *reqobj, struct user_session *u)
{
    memcached_return_t rc;

    //MEM
    rc = set_session_info_to_mem(m_pCfg->m_Memc, reqobj, u);
    if(rc != MEMCACHED_SUCCESS)
    {
        // FIXME as login set mem won't be the same as all existed mem key,
        // so we don't need CAS here.
        ERR("***Failed update token info to mem:%d\n", rc);
    }

    // SQL
    char oldtoken[64]; // store old token before overwrite the DB
    if(set_session_info_to_db(m_pCfg->m_Sql, u, oldtoken) == 1)
    {
        // need delete old token's memcach
        memcached_return_t rc = rm_session_info_from_mem(m_pCfg->m_Memc, oldtoken);
        if(rc != MEMCACHED_SUCCESS)
        {
            ERR("**Failed delete key(%s) from mem:%d\n",
                    oldtoken, rc);
        }
        else
        {
            LOG("delete key(%s) [OK]\n", oldtoken);
        }
    }

    return 0;
}

int LoginOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    LoginResponse *p_obj = (LoginResponse *)obj;

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


/**
 * Begin do the User's Login action
 *
 *
 */
int LoginOperation::handling_request(::google::protobuf::Message *login_req, ::google::protobuf::Message *login_resp, int *len_resp, void *resp)
{
    int ret = -1;
    unsigned long cid = -1;
    LoginRequest *reqobj = (LoginRequest *)login_req;
    LoginResponse *respobj = (LoginResponse *) login_resp;

    ret = match_user_credential_in_db(m_pCfg->m_Sql, reqobj, &cid);

    if(ret == CDS_ERR_SQL_DISCONNECTED)
    {
        ERR("Oh, found MySQL disconnected, try reconnecting...\n");
        if(m_pCfg->reconnect_sql() == 0)
        {
            ret = match_user_credential_in_db(m_pCfg->m_Sql, reqobj, &cid);
        }
    }

    if(ret == CDS_OK)
    {
        // login info data is correct.
        INFO("%s ==> %ld,[Login OK]\n", reqobj->login_name().c_str(), cid);

        char uuiddata[64]; // string like '3ab554e6-1533-4cea-9f6d-26edfd869c6e'
        gen_uuid(uuiddata);
        struct user_session us;
        us.us_cid = cid;
        us.us_sessionid = reqobj->login_session().c_str();
        us.us_token = uuiddata;

        update_usercenter_session(reqobj, &us);

        respobj->set_token(uuiddata);
    }
    else
    {
        // failure case...
        ERR("User login is incorrect!\n");
    }

    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for login result\n");
    }

    return ret;
}
