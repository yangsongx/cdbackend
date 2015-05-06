/**
 *
 * [2015-05-06] For password change case, older tokens(in other deivces) should be marked as invalid
 *
 */
#include "PasswordOperation.h"

char PasswordOperation::m_md5[36];

int PasswordOperation::cb_get_md5_in_db(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row = mysql_fetch_row(mresult);
    char *data = (char *)p_extra;

    if(row != NULL)
    {
        if(row[0] != NULL)
            strcpy(data, row[0]);
    }
    else
    {
        ERR("Warning, we didn't find the password field in DB!\n");
    }

    return 0;
}

int PasswordOperation::cb_get_obsolete_token_in_db(MYSQL_RES *mresult, void *p_extra)
{
    string s;
    list<string> *data = (list<string> *)p_extra;
    MYSQL_ROW row;

    while((row = mysql_fetch_row(mresult)) != NULL)
    {
        if(row[0] != NULL)
            s = row[0];

        /* add each token to list */
        data->push_back(s);
    }

    return 0;
}

/**
 *
 * return 0 means password is correct
 */
int PasswordOperation::validation_user_password(PasswordManagerRequest *reqobj)
{
    int ret = -1;
    char sqlcmd[1024];
    char buf[256];
    char md5[36]; // sotore the final DB passwd data

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT loginpassword FROM %s WHERE id=%lu",
            USERCENTER_MAIN_TBL, reqobj->caredear_id());

    if(sql_cmd(sqlcmd, cb_get_md5_in_db, md5) == CDS_OK)
    {
        char md5data[64];
        // get the md5 in db, compare it with incoming passwd
        sprintf(buf, "%s%lu", reqobj->old_passwd().c_str(), reqobj->caredear_id());
        get_md5(buf, strlen(buf), md5data);
        if(!strcmp(md5data, md5))
        {
            INFO("Good, password correct\n");
            ret = 0;
        }
    }
    else
    {
        LOG("the select passwd failed\n");
    }

    return ret;
}

int PasswordOperation::write_user_password_to_db(PasswordManagerRequest *reqobj)
{
    char sqlcmd[1024];
    char md5data[64];
    char buf[128];

    sprintf(buf, "%s%lu", reqobj->new_passwd().c_str(), reqobj->caredear_id());
    get_md5(buf, strlen(buf), md5data);

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET loginpassword=\'%s\' WHERE id=%ld",
            USERCENTER_MAIN_TBL, md5data, reqobj->caredear_id());

    return sql_cmd(sqlcmd, NULL/* UPDATE don't need callback */, NULL);
}


int PasswordOperation::modify_existed_password(PasswordManagerRequest *reqobj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    char md5[36];

    if(reqobj->has_old_passwd())
    {
        printf("you have old password\n");
        // User contained the old password, need match existed password before overwite
        // this with the new passwd
        if(validation_user_password(reqobj) == 0)
        {
            return write_user_password_to_db(reqobj);
        }
        else
        {
            ERR("the old password is incorrect, reject do anything.\n");
            ret = CDS_ERR_INCORRECT_CODE;
        }
    }
    else
    {
        LOG("the req type:%d\n", reqobj->type());

        if(reqobj->type() == PasswordType::FORGET)
        {
            // for forget password, we directly overwrite the DB pasword
            ret = write_user_password_to_db(reqobj);
        }
        else
        {

        // didn't contained the old password, need make sure DB also be blank
        m_md5[0] = 0x31;
        snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT loginpassword FROM %s WHERE id=%lu",
            USERCENTER_MAIN_TBL, reqobj->caredear_id());

        md5[0] = '\0';
        ret = sql_cmd(sqlcmd, cb_get_md5_in_db, md5);
        if(ret == CDS_OK && strlen(md5) == 0)
        {
            INFO("User original password is blank, so directly set new passwd\n");
            ret = write_user_password_to_db(reqobj);
        }
        else
        {
            ERR("seems you already had a passwd, need validate that one before change to a new passwd\n");
            ret = CDS_ERR_INCORRECT_CODE;
        }

        }
    }

    return ret;
}

/**
 * This included two steps:
 * - from mem
 * - from DB
 */
int PasswordOperation::obsolete_older_token(PasswordManagerRequest *reqobj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    list<string> all_tokens;

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT ticket FROM %s WHERE caredearid=%lu\n",
            USERCENTER_SESSION_TBL, reqobj->caredear_id());

    ret = sql_cmd(sqlcmd, cb_get_obsolete_token_in_db, &all_tokens);
    LOG("all token count = %lu\n", all_tokens.size());
    if(ret == CDS_OK && !all_tokens.empty())
    {
        list<string>::iterator it;
        for(it = all_tokens.begin(); it != all_tokens.end(); ++it)
        {
            if(reqobj->type() == PasswordType::FORGET || (reqobj->has_cur_token() && *it != reqobj->cur_token()))
            {
                delete_token_from_db(it->c_str());
                delete_token_from_mem(it->c_str());
            }
            else
            {
                INFO("Oh, keep this guy(%s)...\n", it->c_str());
            }
        }
    }

    return ret;
}

int PasswordOperation::delete_token_from_db(const char *token)
{
    int ret = CDS_OK;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "DELETE FROM %s WHERE ticket=\'%s\'",
            USERCENTER_SESSION_TBL, token);
    ret = sql_cmd(sqlcmd, NULL, NULL);
    if(ret == CDS_OK)
    {
        LOG("token(%s) deleted due to password changed by someone\n", token);
    }

    return 0;
}

int PasswordOperation::delete_token_from_mem(const char *token)
{
    memcached_return_t rc;

    rc = rm_mem_value(token);
    if(rc != MEMCACHED_SUCCESS)
    {
        ERR("failed rm the key(%s) from mem: %d\n",
                token, rc);
    }

    return 0;
}

/**
 * Entry point of the password manager handling operation
 *
 */
int PasswordOperation::handling_request(::google::protobuf::Message *passwd_req, ::google::protobuf::Message *passwd_resp, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    PasswordManagerRequest *reqobj = (PasswordManagerRequest *)passwd_req;
    PasswordManagerResponse *respobj = (PasswordManagerResponse *)passwd_resp;

    ret = modify_existed_password(reqobj);

    if(ret == CDS_OK)
    {
        INFO("FOR Password OK case, need try obsoleted older tokens immediately\n");
        obsolete_older_token(reqobj);
    }

    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for password manager result\n");
    }

    return ret;
}

int PasswordOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    PasswordManagerResponse *p_obj = (PasswordManagerResponse *) obj;

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
