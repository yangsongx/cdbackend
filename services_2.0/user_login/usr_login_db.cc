
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
 * Check the user's login info is confirmed by DB data, i.e, login correctly
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
        case Logintype::MOBILE_PHONE:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE",
                    USER_MAIN_TABLE);
            break;

        case Logintype::NAME_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE username=\'%s\' AND loginpassword=\'%s\'",
                    USER_MAIN_TABLE,
                    reqobj->login_name().c_str(), reqobj->login_password().c_str());
            break;

        case Logintype::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                    "SELECT id FROM %s WHERE email=\'%s\' AND loginpassword=\'%s\'",
                    USER_MAIN_TABLE,
                    reqobj->login_name().c_str(), reqobj->login_password().c_str());
            break;

        case CID_PASSWD:
            // TODO how to handle with CareDear ID login?
            break;

        default:
            break;
    }

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
            *p_cid = atol(row[0]);
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
    char sqlcmd[1024];
    return 0;
}

int  update_usercenter_session(MYSQL *ms, struct user_session *session)
{
    char sqlcmd[1024];
    return 0;
}
//
