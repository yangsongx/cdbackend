/**
 *
 * \history
 * [2015-04-13] Need get OpenSIPs DB config as well
 */
#include "UpdateProfileConfig.h"

int UpdateProfileConfig::prepare_db_and_mem()
{
    Config::prepare_db_and_mem();

    m_SipsSql = conn_to_mysql(m_sipIP, m_sipUser, m_sipPasswd);
    if(!m_SipsSql)
    {
        ERR("*** Failed connect to the SIPs SQL server!\n");
    }

    return 0;
}

int UpdateProfileConfig::parse_cfg(const char *config_file)
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
            get_node_via_xpath("//service_2/update_user_profile/sqlserver/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlIP, buffer, sizeof(m_strSqlIP));

            get_node_via_xpath("//service_2/update_user_profile/sqlserver/user",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserName, buffer, sizeof(m_strSqlUserName));

            get_node_via_xpath("//service_2/update_user_profile/sqlserver/password",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserPassword, buffer, sizeof(m_strSqlUserPassword));

            LOG("the SQL server ip:%s, user:%s, password:%s\n",
                    m_strSqlIP, m_strSqlUserName, m_strSqlUserPassword);

            // FIXME - don't use memcached currently

            // OpenSIPs section...
            get_node_via_xpath("//service_2/update_user_profile/sipsserver/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_sipIP, buffer, sizeof(m_sipIP));

            get_node_via_xpath("//service_2/update_user_profile/sipsserver/user",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_sipUser, buffer, sizeof(m_sipUser));

            get_node_via_xpath("//service_2/update_user_profile/sipsserver/password",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_sipPasswd, buffer, sizeof(m_sipPasswd));

            LOG("OpenSIPs DB ip:%s\n", m_sipIP);

            xmlXPathFreeContext(ctx);

            INFO("XML config parse finished [OK]\n");
        }

        xmlFreeDoc(doc);
    }

    return prepare_db_and_mem();
}

