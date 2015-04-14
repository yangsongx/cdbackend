/**
 *
 *\history
 * [2015-04-14] Support legacy token auth support
 */
#include "SipOperation.h"


int SipOperation::cb_get_cid(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row;
    uint64_t *data = (uint64_t *)p_extra;

    if(!mresult)
    {
        ERR("in %s:null SQL result, so cid is invalide\n", __FUNCTION__);
        return -1;
    }

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        if(mysql_num_rows(mresult) > 1)
        {
            ERR("Warning, found more than one record for a mobile, check DB\n");
        }

        if(row[0] != NULL)
        {
            *data = atol(row[0]);
        }
    }

    return 0;
}

int SipOperation::cb_get_token(MYSQL_RES *mresult, void *p_extra)
{
    MYSQL_ROW row;
    struct sips_info *data = (struct sips_info *)p_extra;

    data->s_cid = (uint64_t) -1;
    data->s_token[0] = '\0';

    if(!mresult)
    {
        ERR("in %s:null SQL result, so cid is invalide\n", __FUNCTION__);
        return -1;
    }

    row = mysql_fetch_row(mresult);
    if(row != NULL)
    {
        if(mysql_num_rows(mresult) > 1)
        {
            ERR("Warning, found more than one record for a token, check DB\n");
        }

        if(row[0] != NULL)
        {
            data->s_cid = atol(row[0]);
        }
        if(row[1] != NULL)
        {
            /* FIXME - hope 512 is enough */
            strncpy(data->s_token, row[1], 512);
        }
    }

    return 0;
}
int SipOperation::handling_request(::google::protobuf::Message *p_obj, ::google::protobuf::Message *p_ndr, int *len_resp, void *resp)
{
    int ret;
    uint64_t cid = (uint64_t) -1;
    char sqlcmd[1024];
    SipAccountRequest *reqobj = (SipAccountRequest *)p_obj;
    SipAccountResponse *respobj = (SipAccountResponse *)p_ndr;

    LOG("==> %s |  %s | %d\n", reqobj->user_name().c_str(), reqobj->session().c_str(), reqobj->sysid());
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id FROM %s WHERE usermobile=\'%s\'",
            USERCENTER_MAIN_TBL, reqobj->user_name().c_str());
    ret = sql_cmd(sqlcmd, cb_get_cid, &cid);
    if(ret == CDS_OK)
    {
        LOG("User %s --> CID %lu\n", reqobj->user_name().c_str(), cid);
    }

    struct sips_info info;
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT caredearid,ticket FROM %s WHERE session=\'%s\' AND caredearid=%lu",
            USERCENTER_SESSION_TBL, reqobj->session().c_str(), cid);
    ret = sql_cmd(sqlcmd, cb_get_token, &info);
    if(ret == CDS_OK && info.s_cid != (uint64_t) -1)
    {
        if(cid != info.s_cid)
        {
            ERR("Warning, Pontential ERROR, mismatch cid: Main Table CID %lu <===> Session Table CID %lu\n",
                    cid, info.s_cid);
        }

        INFO("%s  =====> %s\n", reqobj->user_name().c_str(), info.s_token);
        // set the token, even above cid mis-match happen
        respobj->set_user_credential(info.s_token);
    }
    else
    {
        ERR("error found when choosing token(ret:%d), set a error resule code\n", ret);
        ret = CDS_ERR_UMATCH_USER_INFO;
    }

    if(compose_result(ret, NULL, p_ndr, len_resp, resp) != 0)
    {
        ERR("** failed seriliaze for the send back to caller\n");
    }

    return ret;
}

int SipOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    SipAccountResponse *p_obj = (SipAccountResponse *)obj;

    p_obj->set_code(code);

    /* As we don't contain extra msg field in response,
     * so don't process the @errmsg parameter here */

    len = p_obj->ByteSize();

    *p_resplen = (len + 2);
    LOG("Full length=%d\n", *p_resplen);
    ArrayOutputStream aos(p_respdata, *p_resplen);
    CodedOutputStream cos(&aos);
    cos.WriteRaw(&len, sizeof(len));

    return ((p_obj->SerializeToCodedStream(&cos)) == true ? 0 : -1);
}
