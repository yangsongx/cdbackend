#include "NetdiskOperation.h"
#include "NetdiskConfig.h"

NetdiskOperation::file_md5_size_t NetdiskOperation::m_md5_size;

int NetdiskOperation::m_cbFlag = 0;
int NetdiskOperation::m_UserQuota = 0;

int NetdiskOperation::cb_query_quota(MYSQL_RES *p_result)
{
    MYSQL_ROW row;

    m_cbFlag = 0;
    row = mysql_fetch_row(p_result);
    if(row != NULL)
    {
        m_cbFlag = atoi(row[0]);
        m_UserQuota = atoi(row[1]);
    }

    return 0;
}

int NetdiskOperation::cb_query_user_entry(MYSQL_RES *p_result)
{
    MYSQL_ROW row;

    row = mysql_fetch_row(p_result);
    if(row != NULL)
    {
        if(mysql_num_rows(p_result) != 1)
        {
            ERR("Warning, there're multi-user entry, check DB!\n");
        }

        m_cbFlag = 1;
    }
    else
    {
        // not existed yet, so tell caller to new user entry.
        m_cbFlag = 0;
    }

    return 0;
}

int NetdiskOperation::cb_query_file_md5_and_size(MYSQL_RES *p_result)
{
    MYSQL_ROW row;
    row = mysql_fetch_row(p_result);
    if(row != NULL)
    {
        if(mysql_num_fields(p_result) != 2)
        {
            ERR("Warning, fields MUST be 2(md5|size)!check DB\n");
        }
        if(row[0] != NULL)
        {
            strncpy(m_md5_size.f_md5, row[0], 34);
        }
        if(row[1] != NULL)
        {
            m_md5_size.f_size = atoi(row[1]);
        }
    }
    else
    {
        ERR("didn't find any matching file md5_size info!\n");
    }

    return 0;
}

int NetdiskOperation::cb_query_netdisk_key(MYSQL_RES *p_result)
{
    MYSQL_ROW row;
    row = mysql_fetch_row(p_result);
    if(row != NULL)
    {
        if(mysql_num_fields(p_result) != 1)
        {
            ERR("Warning, fields MUST be 1(md5)!check DB\n");
        }
        if(row[0] != NULL)
        {
            strncpy(m_md5_size.f_md5, row[0], 34);
        }
    }
    else
    {
        ERR("didn't find any matching file md5_size info!\n");
    }
    return 0;
}

int NetdiskOperation::cb_query_file_md5(MYSQL_RES *p_result)
{
    MYSQL_ROW row;
    row = mysql_fetch_row(p_result);

    if(row != NULL)
    {
        m_cbFlag = 1;
    }
    else
    {
        m_cbFlag = 0;
    }

    return 0;
}

int NetdiskOperation::add_new_user_entry_in_db(NetdiskRequest *p_obj)
{
    int ret = CDS_OK;
    char  sqlcmd[1024];
    NetdiskConfig *cfg = (NetdiskConfig *)m_pCfg;

    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s (USER_NAME,USED_SIZE,USER_QUOTA,CREATE_TIME,MODIFY_TIME) "
            "VALUES (\'%s\',0,%d,NOW(),NOW());",
            NETDISK_USER_TBL, p_obj->user().c_str(), cfg->m_qiniuQuota);

    ret = sql_cmd(sqlcmd, NULL/* INSERT SQL won't get SQL result */);

    return ret;
}

int NetdiskOperation::delete_file_info_from_db(NetdiskRequest *p_obj, const char *md5)
{
    char sqlcmd[1024];
    int ret;

    // TODO, OWNER?? USER??
    snprintf(sqlcmd, sizeof(sqlcmd),
            "DELETE FROM %s WHERE MD5=\'%s\' AND OWNER=\'%s\'",
            NETDISK_FILE_TBL, md5, p_obj->user().c_str());

    ret = sql_cmd(sqlcmd, NULL);

    return ret;
}

int NetdiskOperation::reduce_used_size(NetdiskRequest *p_obj, int size)
{
    char sqlcmd[1024];
    int ret;

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET USED_SIZE=USED_SIZE-%d WHERE OWNER=%s",
            NETDISK_USER_TBL, size, p_obj->user().c_str());

    ret = sql_cmd(sqlcmd, NULL);

    return ret;
}

/**
 * As we need modify multiple table, need support transaction
 */
int NetdiskOperation::record_file_info_to_db(NetdiskRequest *p_obj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    char sqlcmd2[1024];
    const char *filename = p_obj->filename().c_str();
    const int filesize = p_obj->filesize();
    //int type = mapping_file_type(filename);

    // add quota
    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET USED_SIZE=USED_SIZE+%d,MODIFY_TIME=NOW() WHERE USER_NAME=\'%s\'",
            NETDISK_USER_TBL, filesize, p_obj->user().c_str());

    snprintf(sqlcmd2, sizeof(sqlcmd2),
            "INSERT INTO %s (MD5,SIZE,FILENAME,CREATE_TIME,MODIFY_TIME) VALUES "
            "(\'%s\',%d,\'%s\',NOW(),NOW())",
            NETDISK_FILE_TBL,
            p_obj->md5().c_str(), filesize, filename);

    char *allcmds[] = {sqlcmd, sqlcmd2};
    ret = sql_cmd_via_transaction(2, allcmds, NULL);

    return ret;
}

/**
 * Check if this file already existed in DB(Qiniu), or quota exceed
 *
 */
int NetdiskOperation::preprocess_upload_req(NetdiskRequest *p_obj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];

    // 1 - First, check if user is a new netdisk users...
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT ID FROM %s WHERE USER_NAME=\'%s\'",
            NETDISK_USER_TBL, p_obj->user().c_str());

    ret = sql_cmd(sqlcmd, cb_query_user_entry); // cb's flag set 0 means new user
    if(ret == CDS_OK)
    {
        if(m_cbFlag == 0)
        {
        // this is the first-time of User's netdisk request,
        // add the new entry point in DB
        INFO("the first-time of a User using netdisk\n");
        ret = add_new_user_entry_in_db(p_obj);
        if(ret != CDS_OK)
        {
            ERR("Warning, failed insert new user entry:%d\n", ret);
        }
        }
        else
        {
            // User already exsited
            // DO NOTHING here, later will check quota
            INFO("An existed user, wait for further processing\n");
        }
    }
    else
    {
        // TODO - how to handle the error case?
        INFO("~~TODO~~~, wrong case not handled here!!!\n");
    }

    // 2 - check the file to be uploaded existed in DB or NOT
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT ID FROM %s WHERE MD5=\'%s\'",
            NETDISK_FILE_TBL, p_obj->md5().c_str());
    ret = sql_cmd(sqlcmd, cb_query_file_md5); //cb's flage set 1 means existed
    if(m_cbFlag == 1)
    {
        INFO("An already existed file, just add entry in DB, no need to upload to Qiniu at all\n");
        ret = record_file_info_to_db(p_obj);
        return CDS_FILE_ALREADY_EXISTED;
    }

    // 3 - check quota
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT USED_SIZE,USER_QUOTA FROM %s WHERE USER_NAME=\'%s\'",
            NETDISK_USER_TBL, p_obj->user().c_str());

    ret = sql_cmd(sqlcmd, cb_query_quota);
    if(ret == CDS_OK)
    {
        // here m_cbFlag store used size by the user
        NetdiskConfig *cfg = (NetdiskConfig *)m_pCfg;
        LOG("used size=%d, file size=%d, user quota=%d, max quota=%d\n",
               m_cbFlag, p_obj->filesize(), m_UserQuota, cfg->m_qiniuQuota);
        /* FIXME We compare with User QUOTA, not the value in Cfg xml,
         * in case we can specify different user with different quota */
        if((m_cbFlag + p_obj->filesize()) >= m_UserQuota)
        {
            ret = CDS_ERR_EXCEED_QUOTA;
        }
    }

    return ret;
}

/**
 * Just record file info(index) in DB, as physical file already
 * uploaded to Qiniu Server.
 *
 */
int NetdiskOperation::do_completed_qiniu_upload(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    int ret = CDS_OK;

    p_ndr->set_opcode(UPLOADED);

    ret = record_file_info_to_db(p_obj);

    return ret;
}

int NetdiskOperation::do_qiniu_upload(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    int ret = CDS_OK;

    p_ndr->set_opcode(UPLOADING);

    ret = preprocess_upload_req(p_obj);
    switch(ret)
    {
        case CDS_ERR_SQL_EXECUTE_FAILED:
            break;

        case CDS_ERR_EXCEED_QUOTA:
        case CDS_FILE_ALREADY_EXISTED:
            // directly return, and caller should mark existence  as an OK case
            return ret;

        default:
            ; // continue...
    }

    // continue code here
    ret = generate_upload_token(p_obj, p_ndr, p_resplen, p_respdata);

    return ret;
}

int NetdiskOperation::handling_request(::google::protobuf::Message *p_obj, ::google::protobuf::Message *p_ndr, int *len_resp, void *resp)
{
    int ret = CDS_GENERIC_ERROR;
    NetdiskRequest  *reqobj = (NetdiskRequest *)p_obj;
    NetdiskResponse *respobj = (NetdiskResponse *)p_ndr;

    switch(reqobj->opcode())
    {
        case Opcode::UPLOADING:
            LOG("\n=== UPLOADING case===\n");
            LOG("User:%s, File:%s, size:%d, MD5:%s\n",
                    reqobj->user().c_str(),
                    reqobj->filename().c_str(),
                    reqobj->filesize(),
                    reqobj->md5().c_str());
            LOG("==================\n\n");

            ret = do_qiniu_upload(reqobj, respobj, len_resp, resp);
            break;

        case Opcode::UPLOADED:
            LOG("\n=== UPLOADED case===\n");
            LOG("User:%s, File:%s, size:%d, MD5:%s\n",
                    reqobj->user().c_str(),
                    reqobj->filename().c_str(),
                    reqobj->filesize(),
                    reqobj->md5().c_str());
            LOG("==================\n\n");
            ret = do_completed_qiniu_upload(reqobj, respobj, len_resp, resp);
            break;

        case Opcode::DOWNLOADURL:
            LOG("\n=== DOWNLOAD case===\n");
            LOG("User:%s, File:%s\n",
                    reqobj->user().c_str(), reqobj->filename().c_str());
            LOG("==================\n\n");
            ret = do_qiniu_downloadurl(reqobj, respobj, len_resp, resp);
            break;

        case Opcode::DELETE:
            LOG("\n=== DELETE case===\n");
            LOG("User:%s, File:%s\n",
                    reqobj->user().c_str(), reqobj->filename().c_str());
            LOG("==================\n\n");

            ret = do_qiniu_deletion(reqobj, respobj, len_resp, resp);
            break;

        case Opcode::SHARE:
            LOG("\n=== SHARE case===\n");
            LOG("Try sharing File:%s\n",
                    reqobj->filename().c_str());
            LOG("==================\n\n");
            // TODO

            break;

        case Opcode::RENAME:
            // this is just change DB record, won't modify any stuff
            // on Qiniu Server
            LOG("\n=== RENAME case===\n");
            LOG("Try renaming %s --> %s\n",
                    reqobj->filename().c_str(), reqobj->newfile().c_str());
            LOG("==================\n\n");

            LOG("RENAME NOTIMPLEMENTED inC++\n");
            // TODO
            break;

        case Opcode::LISTFILE:
            /* FIXME, this list is done via Java */
            break;

        case Opcode::GETTOKEN:
            // TODO code, a newly-added opcode by me
            break;

        default:
            break;
    }

    /* After finished all above processing, compose response obj */
    if(compose_result(ret, NULL, respobj, len_resp, resp) != 0)
    {
        ERR("**failed serialize data for password manager result\n");
    }

    return ret;
}

int NetdiskOperation::compose_result(int code, const char *errmsg, ::google::protobuf::Message *obj, int *p_resplen, void *p_respdata)
{
    unsigned short len;
    NetdiskResponse *p_obj = (NetdiskResponse *) obj;

    p_obj->set_result_code(code);
    if(code != CDS_OK && errmsg != NULL)
    {
        p_obj->set_errormsg(errmsg);
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

int NetdiskOperation::generate_upload_token(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    Qiniu_RS_PutPolicy put_policy;
    NetdiskConfig *cfg = (NetdiskConfig *)m_pCfg;

    // per req creates one
    Qiniu_Client_InitMacAuth(&m_qn, 1024, NULL);

    memset(&put_policy, 0x00, sizeof(put_policy));
    put_policy.scope = cfg->m_qiniuBucket;
    put_policy.expires = cfg->m_qiniuExpire;

    char *uptoken= Qiniu_RS_PutPolicy_Token(&put_policy, NULL);
    if(uptoken == NULL)
    {
        ERR("*** got NULL mem pointer");
        return CDS_ERR_NOMEMORY;
    }

    LOG("upload token:%s\n", uptoken);
    p_ndr->set_uploadtoken(uptoken);

    // DO NOT FORGET release resource...
    Qiniu_Free(uptoken);
    Qiniu_Client_Cleanup(&m_qn);

    return CDS_OK;
}

int NetdiskOperation::mapping_file_type(const char *filename)
{
    int type = FT_DOC;
    int len = strlen(filename);
    char *suffix;

    if(filename[len - 1] == '.')
    {
        // file ending with dot(i.e, 'xxx.')
        return type;
    }

    while(filename[--len] != '.' && len > 0) ;

    if(len != 0)
    {
        suffix = (char *) &filename[len + 1]; // + 1 to surpass '.' char
        LOG("%s ===> %s\n", filename, suffix);
        if(!strncmp(suffix, "jpg", 3) || !strncmp(suffix, "jpeg", 4)
          || !strncmp(suffix, "png", 3) || !strncmp(suffix, "bmp", 3)
          || !strncmp(suffix, "gif", 3))
        {
            type = FT_IMAGE;
        }
        else if(!strncmp(suffix, "3gp", 3) || !strncmp(suffix, "mp3", 3)
                || !strncmp(suffix, "wma", 3))
        {
            type = FT_MUSIC;
        }
        else if(!strncmp(suffix, "mp4", 3) || !strncmp(suffix, "3gp", 3))
        {
            type = FT_VIDEO;
        }
        else if(!strncmp(suffix, "vcf", 3))
        {
            type = FT_CONTACTS;
        }
        else if(!strncmp(suffix, "msg", 3))
        {
            type = FT_SMS;
        }

        // all others will be considered as doc
    }

    return type;
}

int NetdiskOperation::do_qiniu_downloadurl(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    int ret = CDS_OK;
    char md5[34];

    p_ndr->set_opcode(DOWNLOADURL);

    if(get_netdisk_key(p_obj, md5) != CDS_OK)
    {
        ret = CDS_ERR_FILE_NOTFOUND;
        if(compose_result(ret, "can't find file md5", p_ndr, p_resplen, p_respdata) != 0)
        {
            ERR("Warning, failed serialize download file not found error data\n");
        }

        return ret;
    }


    if(gen_download_url(md5, p_ndr) != 0)
    {
        ret = CDS_ERR_NO_RESOURCE;
        if(compose_result(ret, "failed compose downloadurl", p_ndr, p_resplen, p_respdata) != 0)
        {
            ERR("Warning, failed serialize failure in composing downloadurl data\n");
        }

        return ret;
    }

    if(compose_result(ret, NULL, p_ndr, p_resplen, p_respdata) != 0)
    {
        ERR("Warning, failed serialize download response data\n");
    }

    return ret;
}

int NetdiskOperation::do_qiniu_deletion(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    int ret = CDS_OK;
    char md5[34];
    int  size;

    p_ndr->set_opcode(Opcode::DELETE);

    ret = map_file_to_md5_and_size(p_obj, md5, sizeof(md5), &size);
    if(ret == CDS_OK)
    {
        delete_file_info_from_db(p_obj, md5);
        reduce_used_size(p_obj, size);
    }

    // TODO
    //
    // As above deletion is actually unlink,
    //
    // below code probably check the file table again, if found not such MD5 file link,
    // he SHOULD delete the physical file in Qiniu.
    //
    // But as Heqi said, we can simply keep that file on Qiniu [He said this at 2015-3-25].

    return ret;
}

int NetdiskOperation::map_file_to_md5_and_size(NetdiskRequest *p_obj, char *p_md5, int len_md5, int *p_size )
{
    int ret = -1;
    char sqlcmd[256];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT MD5,SIZE FROM %s WHERE FILENAME=\'%s\' AND OWNER=%lu;",
            NETDISK_FILE_TBL, p_obj->filename().c_str(), p_obj->user().c_str());

    ret = sql_cmd(sqlcmd, cb_query_file_md5_and_size);
    if(ret == CDS_OK)
    {
        /* After exe SQL OK, target stored in m_md5_size variable */
        strncpy(p_md5, m_md5_size.f_md5, len_md5);
        *p_size = m_md5_size.f_size;
    }
    else
    {
        ERR("**Failed mapping %s file's md5|size info\n", p_obj->filename().c_str());
    }

    return ret;
}

/**
 * The netdisk key is a terminology for Qiniu, as we use file MD5 as the Qiniu key
 * so the netdisk key here is file MD5
 */
int NetdiskOperation::get_netdisk_key(NetdiskRequest *p_obj, char *p_result)
{
    int ret = CDS_OK;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT MD5 FROM %s WHERE OWNER=\'%s\' AND FILENAME=\'%s\';",
            NETDISK_FILE_TBL, p_obj->user().c_str(), p_obj->filename().c_str());

    ret = sql_cmd(sqlcmd, cb_query_netdisk_key);
    if(ret == CDS_OK)
    {
        strcpy(p_result, m_md5_size.f_md5);
    }

    return ret;
}

int NetdiskOperation::gen_download_url(const char *md5, NetdiskResponse *p_resp)
{
    int ret = -1;

    char domain_str[512]; // FIXME domain should never had much long str...

    Qiniu_RS_GetPolicy get_policy;
    get_policy.expires = ((NetdiskConfig *)m_pCfg)->m_qiniuExpire;

    snprintf(domain_str, sizeof(domain_str),
            "%s%s", ((NetdiskConfig *)m_pCfg)->m_qiniuBucket, ((NetdiskConfig *)m_pCfg)->m_qiniuDomain);

    char *baseurl = Qiniu_RS_MakeBaseUrl(domain_str, md5);
    char *download_url = Qiniu_RS_GetPolicy_MakeRequest(&get_policy, baseurl, NULL);

    if(download_url != NULL)
    {
        ret = 0;
        LOG("download url=%s\n", download_url);
        p_resp->set_downloadurl(download_url);
        Qiniu_Free(download_url);
    }

    Qiniu_Free(baseurl);

    return ret;
}
