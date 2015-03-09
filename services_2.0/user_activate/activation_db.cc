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
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE usermobile=\'%s\' AND code=\'%s\'",
                ACTIVATION_MAIN_TABLE,
                reqobj->activate_name().c_str(),
                reqobj->activate_code().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE email=\'%s\' AND code=\'%s\'",
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

int verify_activation_code(MYSQL *ms, ActivateRequest *reqobj)
{
    char sqlcmd[1024];
    int ret = CDS_OK;

    switch(reqobj->activate_type())
    {
        case RegLoginType::MOBILE_PHONE:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE usermobile=\'%s\' AND code=\'%s\' AND codetime-UNIX_TIMESTAMP(NOW())>0",
                ACTIVATION_MAIN_TABLE,
                reqobj->activate_name().c_str(),
                reqobj->activate_code().c_str());
            break;

        case RegLoginType::EMAIL_PASSWD:
            snprintf(sqlcmd, sizeof(sqlcmd),
                "SELECT id FROM %s WHERE email=\'%s\' AND code=\'%s\' AND codetime-UNIX_TIMESTAMP(NOW())>0",
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
        ERR("Failed verify code:%s\n", mysql_error(ms));
        return CDS_ERR_SQL_EXECUTE_FAILED;
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(ms);
    UNLOCK_CDS(acts_mutex);

   LOG("sqlcmd:%s\n", sqlcmd);
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
            LOG("pass\n");
        }
    }
    else
    {
        ERR("store result got NULL for check verify code\n");
        ret = CDS_GENERIC_ERROR;
    }

    return ret;
}
