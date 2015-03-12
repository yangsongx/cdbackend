#include "activation_db.h"

extern pthread_mutex_t acts_mutex; // defined in the main()

/**
 *
 *
 *
 */
int check_code_corrrectness(MYSQL *ms, ActivateRequest *reqobj)
{
    int ret = CDS_OK;
    char sqlcmd[1024];

    switch(reqobj->activate_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE usermobile=\'%s\' AND accode=\'%s\'",
                ACTIVATION_MAIN_TABLE,
                reqobj->activate_name().c_str(),
                reqobj->activate_code().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE email=\'%s\' AND accode=\'%s\'",
                ACTIVATION_MAIN_TABLE,
                reqobj->activate_name().c_str(),
                reqobj->activate_code().c_str());
            break;

        default:
            break;
    }

    LOCK_CDS(acts_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(acts_mutex);
        ERR("Failed verify code-only:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(acts_mutex);

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row == NULL)
        {
            // code incorrect
            ret = CDS_ERR_INCORRECT_CODE;
        }
        else
        {
            // time expiration
            ret = CDS_ERR_CODE_EXPIRED;
        }
    }
    else
    {
        ERR("check code-only SHOULD NEVER got NULL!\n");
        ret = CDS_GENERIC_ERROR;
    }


    return ret;
}

/**
 *
 *
 */
int set_user_status_flag(MYSQL *ms, ActivateRequest *reqobj)
{
    char sqlcmd[1024];

    switch(reqobj->activate_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "UPDATE %s SET status=1 WHERE usermobile=\'%s\'",
                ACTIVATION_MAIN_TABLE,
                reqobj->activate_name().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "UPDATE %s SET status=1 WHERE email=\'%s\'",
                ACTIVATION_MAIN_TABLE,
                reqobj->activate_name().c_str());
            break;

        default:
            ERR("**Warning, un-support activate_type\n");
            break;
    }

    LOCK_CDS(acts_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(acts_mutex);
        ERR("Failed set status:%s\n", mysql_error(ms));
    }
    else
    {
        MYSQL_RES *mresult;
        mresult = mysql_store_result(ms);
        UNLOCK_CDS(acts_mutex);

        if(mresult != NULL)
        {
            INFO("WARNING, set status cmd should NEVER return NON-NULL!\n");
        }
        else
        {
            INFO("Set the \'%s\' with active flag [OK]\n",
                    reqobj->activate_name().c_str());
        }
    }

    return 0;
}

int verify_activation_code(MYSQL *ms, ActivateRequest *reqobj)
{
    char sqlcmd[1024];
    int ret = CDS_OK;

    switch(reqobj->activate_type())
    {
        case RegLoginType::MOBILE_PHONE:
        case RegLoginType::PHONE_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE usermobile=\'%s\' AND accode=\'%s\' AND codetime-UNIX_TIMESTAMP(NOW())>0",
                ACTIVATION_MAIN_TABLE,
                reqobj->activate_name().c_str(),
                reqobj->activate_code().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE email=\'%s\' AND accode=\'%s\' AND codetime-UNIX_TIMESTAMP(NOW())>0",
                ACTIVATION_MAIN_TABLE,
                reqobj->activate_name().c_str(),
                reqobj->activate_code().c_str());
            break;

        default:
            ERR("**Warning, un-support activate_type\n");
            break;
    }

    LOCK_CDS(acts_mutex);
    if(mysql_query(ms, sqlcmd))
    {
        UNLOCK_CDS(acts_mutex);
        if(mysql_errno(ms) == CR_SERVER_GONE_ERROR)
        {
            ERR("SQL GONE AWAY, need reconnecting\n");
            return CDS_ERR_SQL_DISCONNECTED;
        }
        else
        {
            ERR("Failed verify code SQL:%s\n", mysql_error(ms));
            return CDS_ERR_SQL_EXECUTE_FAILED;
        }
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(acts_mutex);

    if(mresult)
    {
        row = mysql_fetch_row(mresult);
        if(row == NULL)
        {
            // For the blank result case, there are two possible scenario:
            // - code incorrect
            // - code correct, but time expired
            ret = check_code_corrrectness(ms, reqobj);
            LOG("keep on...\n");
        }
        else
        {
            // PASS case, output a warning if possible
            if(mysql_num_rows(mresult) != 1)
            {
                INFO("Warning, multiple result meet for %s verify code\n",
                        reqobj->activate_code().c_str());
            }
            LOG("pass, update the status falg in DB\n");
            set_user_status_flag(ms, reqobj);
        }
    }
    else
    {
        ERR("store result got NULL for check verify code\n");
        ret = CDS_GENERIC_ERROR;
    }

    return ret;
}
