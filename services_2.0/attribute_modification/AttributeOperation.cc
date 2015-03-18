#include "AttributeOperation.h"

int AttributeOperation::handling_request(::google::protobuf::Message *attr_req, ::google::protobuf::Message *attr_resp, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    char set_str[950];
    char buffer[512];
    AttributeModifyRequest  *reqobj = (AttributeModifyRequest *) attr_req;
    AttributeModifyResponse *respobj = (AttributeModifyResponse *) attr_resp;

    strcpy(set_str, "SET ");
    // First, try compose all possible fields...
    if(reqobj->has_real_name())
    {
        snprintf(buffer, sizeof(buffer), "realname=\'%s\',",
                reqobj->real_name().c_str());
        strcat(set_str, buffer);
    }

    if(reqobj->has_nick_name())
    {
        snprintf(buffer, sizeof(buffer), "nickname=\'%s\',",
                reqobj->nick_name().c_str());
        strcat(set_str, buffer);
    }

    if(reqobj->has_gender())
    {
        snprintf(buffer, sizeof(buffer), "sex=\'%d\',",
                reqobj->gender() == GenderType::MALE ? 1 : 0);
        strcat(set_str, buffer);
    }

    if(reqobj->has_birthday())
    {
        snprintf(buffer, sizeof(buffer), "birthday=\'%s\',",
                reqobj->birthday().c_str());
        strcat(set_str, buffer);
    }

    if(reqobj->has_head_image())
    {
        snprintf(buffer, sizeof(buffer), "headimg=\'%s\',",
                reqobj->head_image().c_str());
        strcat(set_str, buffer);
    }

    if(reqobj->has_head_image2())
    {
        snprintf(buffer, sizeof(buffer), "headimg2=\'%s\',",
                reqobj->head_image2().c_str());
        strcat(set_str, buffer);
    }

    ret = strlen(set_str);
    if(ret >= (int)sizeof(set_str))
    {
        ERR("exceed the max buffer(%d V.S. %ld)\n",
            ret, sizeof(set_str));

        ret = CDS_ERR_REQ_TOOLONG;
    }
    else
    {
        set_str[ret - 1] = ' ';

        snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s %sWHERE caredearid=%ld",
            USERCENTER_ATTR_TBL, set_str, reqobj->caredear_id());

        LOG("the whole SQL:%s\n", sqlcmd);
        ret = sql_cmd(sqlcmd, NULL);

    }
    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for password manager result\n");
    }

    return ret;
}

int AttributeOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    AttributeModifyResponse *p_obj = (AttributeModifyResponse *) obj;

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
