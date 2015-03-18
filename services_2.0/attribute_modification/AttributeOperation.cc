#include "AttributeOperation.h"

int AttributeOperation::m_attrRecord = 0;

/**
 *
 *
 *
 */
int AttributeOperation::cb_check_attr_existence(MYSQL_RES *p_result)
{
    MYSQL_ROW  row;

    LOG("==CALLBACK==\n");
    m_attrRecord = 0;

    if(!p_result)
    {
        return -1;
    }

    row = mysql_fetch_row(p_result);
    if(row != NULL)
    {
        if(mysql_num_rows(p_result) != 1)
        {
            INFO("Warning, there're multi-lines for the SELECT!\n");
        }
        m_attrRecord = 1;
    }

    return 0;
}

/**
 *
 * return 1 means DB already contained this attribute item.
 */
int AttributeOperation::user_attribute_existed(uint64_t cid)
{
    int ret;
    char sqlcmd[512];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id FROM %s WHERE caredearid=%lu",
            USERCENTER_ATTR_TBL, cid);

    ret = sql_cmd(sqlcmd, cb_check_attr_existence);
    if(ret == CDS_OK && m_attrRecord == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int AttributeOperation::insert_usr_attr_to_db(AttributeModifyRequest *reqobj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    char insert_str[950];
    char buffer[512];
    size_t len;

    sprintf(insert_str, "(caredearid,");

    if(reqobj->has_real_name())
    {
        strcat(insert_str, "realname,");
    }
    if(reqobj->has_nick_name())
    {
        strcat(insert_str, "nickname,");
    }
    if(reqobj->has_gender())
    {
        strcat(insert_str, "sex,");
    }
    if(reqobj->has_birthday())
    {
        strcat(insert_str, "birthday,");
    }
    if(reqobj->has_head_image())
    {
        strcat(insert_str, "headimg,");
    }
    if(reqobj->has_head_image2())
    {
        strcat(insert_str, "headimg2,");
    }

    len = strlen(insert_str);
    insert_str[len - 1] = ')';

    // next, finished VALUEs section...

    sprintf(buffer, " VALUES (%lu,", reqobj->caredear_id());
    strcat(insert_str, buffer);

    if(reqobj->has_real_name())
    {
        snprintf(buffer, sizeof(buffer), "\'%s\',",
                reqobj->real_name().c_str());
        strcat(insert_str, buffer);
    }
    if(reqobj->has_nick_name())
    {
        snprintf(buffer, sizeof(buffer), "\'%s\',",
                reqobj->nick_name().c_str());
        strcat(insert_str, buffer);
    }
    if(reqobj->has_gender())
    {
        snprintf(buffer, sizeof(buffer), "%d,",
                reqobj->gender() == GenderType::MALE ? 1 : 0);
        strcat(insert_str, buffer);
    }
    if(reqobj->has_birthday())
    {
        snprintf(buffer, sizeof(buffer), "\'%s\',",
                reqobj->birthday().c_str());
        strcat(insert_str, buffer);
    }
    if(reqobj->has_head_image())
    {
        snprintf(buffer, sizeof(buffer), "\'%s\',",
                reqobj->head_image().c_str());
        strcat(insert_str, buffer);
    }
    if(reqobj->has_head_image2())
    {
        snprintf(buffer, sizeof(buffer), "\'%s\',",
                reqobj->head_image2().c_str());
        strcat(insert_str, buffer);
    }

    len = strlen(insert_str);
    insert_str [len - 1] = ')';

    if(len >= sizeof(insert_str))
    {
        ERR("Overwrite memory found, check data length!\n");
        return CDS_ERR_REQ_TOOLONG;
    }

    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s %s",
            USERCENTER_ATTR_TBL, insert_str);

    LOG("the whole insert SQL:%s\n", sqlcmd);
    ret = sql_cmd(sqlcmd, NULL);

    return ret;
}

int AttributeOperation::update_usr_attr_to_db(AttributeModifyRequest *reqobj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    char set_str[950];
    char buffer[512];

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

    return ret;
}

int AttributeOperation::handling_request(::google::protobuf::Message *attr_req, ::google::protobuf::Message *attr_resp, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    AttributeModifyRequest  *reqobj = (AttributeModifyRequest *) attr_req;
    AttributeModifyResponse *respobj = (AttributeModifyResponse *) attr_resp;

    if(user_attribute_existed(reqobj->caredear_id()) == 1)
    {
        // update an old record
        ret = update_usr_attr_to_db(reqobj);
    }
    else
    {
        // insert a new record
        ret = insert_usr_attr_to_db(reqobj);
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
