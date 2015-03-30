#include "UserRegConfig.h"

/**
 * Override from Config::prepare_db_and_mem()
 *
 * Currently, directly return 0
 */
int UserRegConfig::prepare_db_and_mem()
{
    // parent funciton...
    Config::prepare_db_and_mem();

    // need take extra care of SIPs' DB
    m_SipsSql = conn_to_mysql(m_sipIP, m_sipUser, m_sipPasswd);
    if(!m_SipsSql)
    {
        ERR("*** Failed connect to the SIPs SQL server!\n");
    }

    return 0;
}

/**
 * Read out init config XML file
 *
 * @config_file : the XML config file name(currently, it is /etc/cds_cfg.xml)
 *
 */
int UserRegConfig::parse_cfg(const char *config_file)
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
            get_node_via_xpath("//service_2/user_register_service/sqlserver/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlIP, buffer, sizeof(m_strSqlIP));

            get_node_via_xpath("//service_2/user_register_service/sqlserver/user",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserName, buffer, sizeof(m_strSqlUserName));

            get_node_via_xpath("//service_2/user_register_service/sqlserver/password",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserPassword, buffer, sizeof(m_strSqlUserPassword));

            LOG("the SQL server ip:%s, user:%s, password:%s\n",
                    m_strSqlIP, m_strSqlUserName, m_strSqlUserPassword);

            get_node_via_xpath("//service_2/user_register_service/memcache/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strMemIP, buffer, sizeof(m_strMemIP));

            get_node_via_xpath("//service_2/user_register_service/memcache/port",
                    ctx, buffer, sizeof(buffer));
            m_iMemPort = atoi(buffer);

            LOG("the memcached ip:%s, port:%d\n", m_strMemIP, m_iMemPort);

            get_node_via_xpath("//service_2/user_register_service/mobile_verifycode_expiration",
                    ctx, buffer, sizeof(buffer));
            m_iMobileVerifyExpir = atoi(buffer);

            get_node_via_xpath("//service_2/user_register_service/email_verifycode_expiration",
                    ctx, buffer, sizeof(buffer));
            m_iEmailVerifyExpir = atoi(buffer);

            /* next, will */
            get_node_via_xpath("//service_2/user_register_service/sipsserver/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_sipIP, buffer, sizeof(m_sipIP));

            get_node_via_xpath("//service_2/user_register_service/sipsserver/user",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_sipUser, buffer, sizeof(m_sipUser));

            get_node_via_xpath("//service_2/user_register_service/sipsserver/password",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_sipPasswd, buffer, sizeof(m_sipPasswd));

            LOG("OpenSIPs DB ip:%s\n", m_sipIP);


            // free useless resource...
            xmlXPathFreeContext(ctx);

            INFO("XML config parse finished [OK]\n");
        }

        xmlFreeDoc(doc);
    }

    return prepare_db_and_mem();
}
