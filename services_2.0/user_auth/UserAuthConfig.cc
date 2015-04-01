#include "UserAuthConfig.h"

UserAuthConfig::UserAuthConfig()
{
}

UserAuthConfig::~UserAuthConfig()
{
}


int UserAuthConfig::parse_cfg(const char *config_file)
{
    char buffer[32]; // be consistent with UserRegConfig's data member size
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    if(access(config_file, F_OK) != 0)
    {
        ERR("\'%s\' not existed!\n", config_file);
        return -1;
    }

    doc = xmlParseFile(config_file);
    if(doc != NULL)
    {
        ctx = xmlXPathNewContext(doc);
        if(ctx != NULL)
        {
            get_node_via_xpath("//service_2/user_auth_service/sqlserver/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlIP, buffer, sizeof(m_strSqlIP));

            get_node_via_xpath("//service_2/user_auth_service/sqlserver/user",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserName, buffer, sizeof(m_strSqlUserName));

            get_node_via_xpath("//service_2/user_auth_service/sqlserver/password",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserPassword, buffer, sizeof(m_strSqlUserPassword));

            LOG("the SQL server ip:%s, user:%s, password:%s\n",
                    m_strSqlIP, m_strSqlUserName, m_strSqlUserPassword);

            get_node_via_xpath("//service_2/user_auth_service/memcache/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strMemIP, buffer, sizeof(m_strMemIP));

            get_node_via_xpath("//service_2/user_auth_service/memcache/port",
                    ctx, buffer, sizeof(buffer));
            m_iMemPort = atoi(buffer);

            LOG("the memcached ip:%s, port:%d\n", m_strMemIP, m_iMemPort);

            xmlXPathFreeContext(ctx);

            INFO("XML config parse finished [OK]\n");
        }

        xmlFreeDoc(doc);
    }

    if(prepare_db_and_mem() == 0)
    {
        readout_db_session_conf();
        return 0;
    }
    else
    {
        return -1;
    }
}

int UserAuthConfig::readout_db_session_conf()
{

    if(!m_Sql)
    {
        ERR("NULL SQL, don't try get session config data from DB\n");
        return -1;
    }

    /* FIXME, don't use mutex here as it is only happen in init procedure */

    if(mysql_query(m_Sql, "SELECT sysid,isorder,lefttime,type FROM uc.uc_sys_sessionconf"))
    {
        ERR("Failed check the login password:%s\n", mysql_error(m_Sql));
        return -1;
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(m_Sql);

    if(mresult)
    {
        int sysid;
        session_db_cfg_t t;

        if(mysql_num_fields(mresult) != 4)
        {
            ERR("Warning, session conf select col should be 4, don't process this!\n");
            return -1;
        }

        // store the whole data one-by-one
        while((row = mysql_fetch_row(mresult)) != NULL)
        {
            sysid = atoi(row[0]);
            t.sfg_allow_multilogin = atoi(row[1]);
            t.sfg_expiration = atol(row[2]);
            t.sfg_type = atoi(row[3]);
            m_sessionCfg.insert(map<int, session_db_cfg_t>::value_type(sysid, t));
        }

        mysql_free_result(mresult);
    }
    else
    {
        ERR("should never happen here.");
    }

    return 0;
}
