#include "AttributeOperation.h"

int AttributeOperation::m_attrRecord = 0;
AttributeModifyResponse AttributeOperation::m_QueryInfo;


int AttributeOperation::cb_query_attribute(MYSQL_RES *p_result)
{
    MYSQL_ROW  row;
    int col_count;

    if(!p_result)
    {
        return -1;
    }

    row = mysql_fetch_row(p_result);
    if(!row)
    {
        INFO("Got blank result for query user info\n");
        return -1;
    }

    /* keep field-map be consistent with SELECT sql */
    col_count = mysql_num_fields(p_result);
    if(col_count != 10 || mysql_num_rows(p_result) != 1)
    {
        ERR("query result probably incorrect(row:%llu,col:%d)\n",
               mysql_num_rows(p_result), col_count);
        return -1;
    }

    if(row[0] != NULL && strlen(row[0]) > 0)
    {
        m_QueryInfo.set_user_name(row[0]);
    }

    if(row[1] != NULL && strlen(row[1]) > 0)
    {
        m_QueryInfo.set_user_mobile(row[1]);
    }

#if 1
    if(row[2] == NULL || (row[2] != NULL && strlen(row[2]) == 0))
    {
        INFO("a blank password\n");
        m_QueryInfo.set_contain_passwd(0);
    }
    else
    {
        m_QueryInfo.set_contain_passwd(1);
    }
#endif

    if(row[3] != NULL && strlen(row[3]) > 0)
    {
        m_QueryInfo.set_user_email(row[3]);
    }

    if(row[4] != NULL && strlen(row[4]) > 0)
    {
        m_QueryInfo.set_real_name(row[4]);
    }

    if(row[5] != NULL && strlen(row[5]) > 0)
    {
        m_QueryInfo.set_nick_name(row[5]);
    }

    if(row[6] != NULL && strlen(row[6]) > 0)
    {
        if(atoi(row[6]) == 1)
        {
            m_QueryInfo.set_gender(GenderType::MALE);
        }
        else
        {
            m_QueryInfo.set_gender(GenderType::FEMAL);
        }
    }

    if(row[7] != NULL && strlen(row[7]) > 0)
    {
        m_QueryInfo.set_birthday(row[7]);
    }

    if(row[8] != NULL && strlen(row[8]) > 0)
    {
        m_QueryInfo.set_head_image(row[8]);
    }

    if(row[9] != NULL && strlen(row[9]) > 0)
    {
        m_QueryInfo.set_head_image2(row[9]);
    }

    return 0;
}

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
    if(reqobj->has_existed_in_shenzhen())
    {
        strcat(insert_str, "issync,");
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
    if(reqobj->has_existed_in_shenzhen())
    {
        snprintf(buffer, sizeof(buffer), "%d,",
                reqobj->existed_in_shenzhen());
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

    if(reqobj->has_existed_in_shenzhen())
    {
        snprintf(buffer, sizeof(buffer), "issync=%d,",
                reqobj->existed_in_shenzhen());
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

int AttributeOperation::modify_user_attribute(AttributeModifyRequest  *reqobj)
{
    int ret;

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

    return ret;
}

int AttributeOperation::query_user_attribute_from_db(AttributeModifyRequest *reqobj, AttributeModifyResponse *respobj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];

#if 1 // use a new SQL format
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT %s.username,%s.usermobile,%s.loginpassword,%s.email,"
            "%s.realname,%s.nickname,%s.sex,%s.birthday,%s.headimg,%s.headimg2 "
            "FROM %s LEFT JOIN %s ON %s.id=%s.caredearid WHERE %s.id=%lu",
            USERCENTER_MAIN_TBL, USERCENTER_MAIN_TBL, USERCENTER_MAIN_TBL, USERCENTER_MAIN_TBL,
            USERCENTER_ATTR_TBL, USERCENTER_ATTR_TBL, USERCENTER_ATTR_TBL, USERCENTER_ATTR_TBL, USERCENTER_ATTR_TBL, USERCENTER_ATTR_TBL,
            USERCENTER_MAIN_TBL, USERCENTER_ATTR_TBL, USERCENTER_MAIN_TBL, USERCENTER_ATTR_TBL, USERCENTER_MAIN_TBL, reqobj->caredear_id());
#else
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT %s.username,%s.usermobile,%s.loginpassword,%s.email,"
            "%s.realname,%s.nickname,%s.sex,%s.birthday,%s.headimg,%s.headimg2 "
            "FROM %s,%s WHERE %s.id=%s.caredearid AND %s.id=%lu",
            USERCENTER_MAIN_TBL, USERCENTER_MAIN_TBL, USERCENTER_MAIN_TBL, USERCENTER_MAIN_TBL,
            USERCENTER_ATTR_TBL, USERCENTER_ATTR_TBL, USERCENTER_ATTR_TBL, USERCENTER_ATTR_TBL, USERCENTER_ATTR_TBL, USERCENTER_ATTR_TBL,
            USERCENTER_MAIN_TBL, USERCENTER_ATTR_TBL, USERCENTER_MAIN_TBL, USERCENTER_ATTR_TBL, USERCENTER_MAIN_TBL, reqobj->caredear_id());
#endif

    ret = sql_cmd(sqlcmd, cb_query_attribute);
    if(ret == CDS_OK)
    {
        // assign operator
        *respobj = m_QueryInfo;
    }

    return ret;
}


int AttributeOperation::handling_request(::google::protobuf::Message *attr_req, ::google::protobuf::Message *attr_resp, int *len_resp, void *resp)
{
    int ret = CDS_OK;
    AttributeModifyRequest  *reqobj = (AttributeModifyRequest *) attr_req;
    AttributeModifyResponse *respobj = (AttributeModifyResponse *) attr_resp;

    switch(reqobj->req_type())
    {
        case AttributeType::MODIFY:
            ret = modify_user_attribute(reqobj);
            break;

        case AttributeType::QUERY:
            // As query won't be so frequently, we don't add any
            // memcach, so directly choose from DB.
            ret = query_user_attribute_from_db(reqobj, respobj);
            break;

        default:
            ERR("SHOULD NEVER come here as unknow req type\n");
            ret = CDS_GENERIC_ERROR;
            break;
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
