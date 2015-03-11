
#include "usr_login_db.h"
#include "uuid.h"



using namespace com::caredear;

extern pthread_mutex_t uls_mutex; // defined in the main()

/**
 * Store a UUID in @result
 *
 * Currently just return 0
 */
int gen_uuid(char *result)
{
    uuid_t id;
        
    uuid_generate(id);

    /* FIXME - actually, we can call uuid_unparse() to below code line! */
    sprintf(result,
           "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
           id[0], id[1], id[2], id[3],
           id[4], id[5],
           id[6], id[7],
           id[8], id[9],
           id[10], id[11], id[12], id[13], id[14], id[15]);
        
    return 0;
}

/**
 * Compare @targetdata with DB, which id is @cid.
 *
 */
int compare_user_password_wth_cid(MYSQL *ms, LoginRequest *reqobj, const char *targetdata, unsigned long cid)
{
    int ret = CDS_GENERIC_ERROR;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id,status FROM %s WHERE id=%ld AND loginpassword=\'%s\'",
            USER_MAIN_TABLE, cid, targetdata);

    LOCK_CDS(uls_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(uls_mutex);
        ERR("Failed check the login password:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(uls_mutex);

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            if(mysql_num_rows(mresult) != 1)
            {
                INFO("Warning, this SHOULD NEVER HAPPEND, as only unique-ID queryed!\n");
            }
            // set as login with password correctly.
            if(row[1] != NULL && atoi(row[1]) == 0)
            {
                // not activated yet
                INFO("You Login correctly, but still activated yet!\n");
                ret = CDS_ERR_INACTIVATED;
            }
            else
            {
                ret = CDS_OK;
            }
        }
        else
        {
            // don't match any password
            ret = CDS_ERR_UMATCH_USER_INFO;
        }
    }
    else
    {
        ERR("compare passwd SQL should NEVER meet NULL result!\n");
    }

    return ret;
}

/**
 * Check the user's login info is confirmed by DB data, i.e, login correctly
 *
 * This including two steps:
 * - get user's CID
 * - try compose MD5 together with the request's password,
 * - compare it within DB's
 *
 * @p_cid: Caredear ID(CID) of the valid user if his login is successful.
 *
 * return CDS_OK for a correct login, otherwise return an error code
 */
int match_user_credential_in_db(MYSQL *ms, LoginRequest *reqobj, unsigned long *p_cid)
{
    int ret = CDS_OK;
    char sqlcmd[1024];

    switch(reqobj->login_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE usermobile=\'%s\'",
                    USER_MAIN_TABLE,
                    reqobj->login_name().c_str());
            break;

        case RegLoginType::NAME_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE username=\'%s\'",
                    USER_MAIN_TABLE,
                    reqobj->login_name().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE email=\'%s\'",
                    USER_MAIN_TABLE,
                    reqobj->login_name().c_str());
            break;

        case RegLoginType::OTHERS:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE third=\'%s\'",
                    USER_MAIN_TABLE,
                    reqobj->login_name().c_str());
            break;

        case RegLoginType::CID_PASSWD:
            // TODO how to handle with CareDear ID login?
            break;

        default:
            break;
    }

    LOCK_CDS(uls_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(uls_mutex);
        ERR("Failed check the login cid:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(uls_mutex);

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row == NULL)
        {
            // row be NULL means no such login data matched in DB
            // So need tell caller that his/her login is incorrect.
            ret = CDS_ERR_UMATCH_USER_INFO;
        }
        else
        {
            // here means User's Login is confirmed by DB data.
            // set CID
            if(mysql_num_rows(mresult) != 1)
            {
                INFO("Warning, CID matching returned NON-1 result, please check the DB!\n");
            }
            *p_cid = atol(row[0]); // take first-match if meet multiple result...
            LOG("name[%s] ==> cid[%ld]\n",
                    reqobj->login_name().c_str(), *p_cid);

            char md5data[64];
            // can re-use the sqlcmd here.
            sprintf(sqlcmd, "%ld-%s",
                    *p_cid, reqobj->login_password().c_str());
            get_md5(sqlcmd, strlen(sqlcmd), md5data);
            LOG("phase-I cipher[%s] ==> phase-II cipher[%s]\n",
                    sqlcmd, md5data);

            ret = compare_user_password_wth_cid(ms, reqobj, md5data, *p_cid);
        }
    }
    else
    {
        ERR("**Got NULL sql result when try matching login info in DB\n");
        ret = CDS_GENERIC_ERROR;
    }

    return ret;
}


int record_user_session_in_db(MYSQL *ms)
{
    //char sqlcmd[1024];
    /* TODO */
    return 0;
}

int overwrite_existed_session_in_db(MYSQL *ms, struct user_session *session)
{
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "UPDATE %s SET ticket=\'%s\',lastoperatetime=NOW() WHERE caredearid=%ld AND session=\'%s\'",
            USER_SESSION_TABLE, session->us_token,
            session->us_cid, session->us_sessionid);

    LOCK_CDS(uls_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(uls_mutex);
        ERR("Failed overwrite existed session(cid=%ld):%s\n", session->us_cid, mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(uls_mutex);

    if(mresult)
    {
        ERR("Warning, UPDATE should NEVER return non-NULL result!\n");
    }
    else
    {
        LOG("overwrite existed session(cid-%ld) [OK]\n", session->us_cid);
    }

    return 0;
}

int insert_new_session_in_db(MYSQL *ms, struct user_session *session)
{
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO %s (caredearid,ticket,session,lastoperatetime) VALUES "
            "(%ld,\'%s\',\'%s\',NOW())",
            USER_SESSION_TABLE,
            session->us_cid, session->us_token, session->us_sessionid);

    LOCK_CDS(uls_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(uls_mutex);
        ERR("Failed insert new session(cid=%ld):%s\n", session->us_cid, mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(uls_mutex);

    if(mresult)
    {
        ERR("Warning, INSERT should NEVER return non-NULL result!\n");
    }
    else
    {
        LOG("insert the new session(cid-%ld) [OK]\n", session->us_cid);
    }
    return 0;
}

/**
 *
 * return 1 will store the old session ticket(token) to @old.
 */
int set_session_info_to_db(MYSQL *ms, struct user_session *session, char *old)
{
    int ret = 0;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT ticket FROM %s WHERE caredearid=%ld AND session=\'%s\'",
            USER_SESSION_TABLE,
            session->us_cid, session->us_sessionid);

    LOCK_CDS(uls_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(uls_mutex);
        ERR("Failed select session(cid=%ld):%s\n", session->us_cid, mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(uls_mutex);

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row != NULL)
        {
            // Already existed a seesion, should overwrite it!
            // And DO NOT forget to remove memcached later
            INFO("Will obsolete token(%s)...\n", row[0]);
            strcpy(old, row[0]);
            overwrite_existed_session_in_db(ms, session);
            ret = 1;
        }
        else
        {
            // A new User Session, insert it
            insert_new_session_in_db(ms, session);
        }
    }
    else
    {
        ERR("Warning, mysql_store_result should NEVER be NULL for SELECT\n");
    }

    return ret;
}
//
