#include "PasswordOperation.h"

int PasswordOperation::modify_existed_password(const char *new_passwd, uint64_t cid)
{
    char buf[80];
    char targetdata[64];
    char sqlcmd[1024];

    snprintf(buf, sizeof(buf),
            "%ld-%s", cid, new_passwd);
    get_md5(buf, strlen(buf), targetdata);

    INFO("new code [%s] --> new target data [%s]\n", new_passwd, targetdata);

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET loginpassword=\'%s\' WHERE id=%ld",
            USERCENTER_MAIN_TBL, targetdata, cid);

    return sql_cmd(sqlcmd, NULL/* UPDATE don't need callback */);
}

int PasswordOperation::handling_request(::google::protobuf::Message *passwd_req, ::google::protobuf::Message *passwd_resp, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    PasswordManagerRequest *reqobj = (PasswordManagerRequest *)passwd_req;
    PasswordManagerResponse *respobj = (PasswordManagerResponse *)passwd_resp;

    ret = modify_existed_password(reqobj->new_passwd().c_str(), reqobj->caredear_id());

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
