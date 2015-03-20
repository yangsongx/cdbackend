#include "NetdiskOperation.h"
#include "NetdiskConfig.h"

NetdiskOperation::file_md5_size_t NetdiskOperation::m_md5_size;

int NetdiskOperation::m_cbFlag = 0;

int NetdiskOperation::cb_query_quota(MYSQL_RES *p_result)
{
    MYSQL_ROW row;

    m_cbFlag = 0;
    row = mysql_fetch_row(p_result);
    if(row != NULL)
    {
        m_cbFlag = atoi(row[0]);
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
            "VALUES (%lu,0,%d,NOW(),NOW());",
            NETDISK_USER_TBL, p_obj->caredear_id(), cfg->m_qiniuQuota);

    ret = sql_cmd(sqlcmd, NULL/* INSERT SQL won't get SQL result */);

    return ret;
}

int NetdiskOperation::delete_file_info_from_db(NetdiskRequest *p_obj, const char *md5)
{
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "DELETE FROM %s WHERE MD5=\'%s\' AND OWNER=%lu",
            NETDISK_FILE_TBL, md5, p_obj->caredear_id());

    return 0;
}


/**
 * As we need modify multiple table, need support transaction
 */
int NetdiskOperation::record_file_info_to_db(NetdiskRequest *p_obj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];
    MYSQL *ms = m_pCfg->m_Sql;
    MYSQL_RES *mresult;

    const char *filename = p_obj->filename().c_str();
    const int filesize = p_obj->filesize();
    int type = mapping_file_type(filename);

    // add quota
    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET USED_SIZE=USED_SIZE+%d,MODIFY_TIME=NOW WHERE USER_NAME=%lu",
            NETDISK_USER_TBL, filesize, p_obj->caredear_id());

    pthread_mutex_lock(&m_pCfg->m_SqlMutex);
    if(mysql_query(ms, "begin"))
    {
        pthread_mutex_unlock(&m_pCfg->m_SqlMutex);
        ERR("failed do the begin transaction:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    if(mysql_query(ms, sqlcmd))
    {
        ERR("failed update user quota DB:%s\n", mysql_error(ms));
        goto need_rollback;
    }

    /* FIXME, need this for update/insert case ? */
    mresult = mysql_store_result(ms);
    if(mresult)
        mysql_free_result(mresult);


    // add to files
    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s (MD5,SIZE,FILENAME,CREATE_TIME,MODIFY_TIME,TYPE,OWNER) VALUES "
            "(\'%s\',%d,\'%s\',NOW(),NOW(),%d,%lu)",
            NETDISK_FILE_TBL,
            p_obj->md5().c_str(), filesize, filename,
            type, p_obj->caredear_id());
    if(mysql_query(ms, sqlcmd))
    {
        ERR("failed update file DB:%s\n", mysql_error(ms));
        goto need_rollback;
    }

    /* FIXME, need this for update/insert case ? */
    mresult = mysql_store_result(ms);
    if(mresult)
        mysql_free_result(mresult);


    INFO("Update the two table OK, commit them...");
    if(mysql_commit(ms) == 0)
    {
        INFO("[OK]\n");
        ret = CDS_OK;
    }
    else
    {
        INFO("Failed : %s\n", mysql_error(ms));
        ret = CDS_ERR_SQL_EXECUTE_FAILED;
    }

    pthread_mutex_unlock(&m_pCfg->m_SqlMutex);
    return ret;

    // ROLL BACK section
need_rollback:
    pthread_mutex_unlock(&m_pCfg->m_SqlMutex);
    INFO("will rollback as sth wrong...");
    if(mysql_rollback(ms) == 0)
    {
        INFO("[OK]\n");
    }
    else
    {
        INFO("[failed:%s]\n", mysql_error(ms));
    }
    return CDS_ERR_SQL_EXECUTE_FAILED;
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
            "SELECT ID FROM %s WHERE USERNAME=%lu;",
            NETDISK_USER_TBL, p_obj->caredear_id());

    ret = sql_cmd(sqlcmd, cb_query_user_entry); // cb's flag set 0 means new user
    if(m_cbFlag == 0)
    {
        // this is the first-time of User's netdisk request,
        // add the new entry point in DB
        ret = add_new_user_entry_in_db(p_obj);
        if(ret != CDS_OK)
        {
            ERR("Warning, failed insert new user entry:%d\n", ret);
        }
    }

    // 2 - check the file to be uploaded existed in DB or NOT
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT ID FROM %s WHERE MD5=\'%s\'",
            NETDISK_FILE_TBL, p_obj->md5().c_str());
    ret = sql_cmd(sqlcmd, cb_query_file_md5); //cb's flage set 1 means existed
    if(m_cbFlag == 1)
    {
        // File existed, we just add this entry to DB, before tell caller
        ret = record_file_info_to_db(p_obj);
        return CDS_FILE_ALREADY_EXISTED;
    }

    // 3 - check quota
    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT USED_SIZE FROM %s WHERE USER_NAME=%lu",
            NETDISK_USER_TBL, p_obj->caredear_id());

    ret = sql_cmd(sqlcmd, cb_query_quota);
    if(ret == CDS_OK)
    {
        // here m_cbFlag store used size by the user
        NetdiskConfig *cfg = (NetdiskConfig *)m_pCfg;
        LOG("used size=%d, file size=%d, quota=%d\n",
               m_cbFlag, p_obj->filesize(), cfg->m_qiniuQuota);
        if((m_cbFlag + p_obj->filesize()) >= cfg->m_qiniuQuota)
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
            // TODO code
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

            // TODO
            break;

        case Opcode::LISTFILE:
            /* FIXME, this list is done via Java */
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

int NetdiskOperation::do_qiniu_deletion(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata)
{
    int ret = CDS_OK;
    char md5[34];
    int  size;

    p_ndr->set_opcode(Opcode::DELETE);

    ret = map_file_to_md5_and_size(p_obj, md5, sizeof(md5), &size);
    if(ret == CDS_OK)
    {
        ret = delete_file_info_from_db(p_obj, md5);
    }

    // TODO

    return ret;
}

int NetdiskOperation::map_file_to_md5_and_size(NetdiskRequest *p_obj, char *p_md5, int len_md5, int *p_size )
{
    int ret = -1;
    char sqlcmd[256];
    MYSQL_RES *mresult;
    MYSQL_ROW  row;

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT MD5,SIZE FROM %s WHERE FILENAME=\'%s\' AND OWNER=%lu;",
            NETDISK_FILE_TBL, p_obj->filename().c_str(), p_obj->caredear_id());

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
