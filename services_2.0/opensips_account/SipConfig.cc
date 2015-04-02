#include "SipConfig.h"


int SipConfig::parse_cfg(const char *config_file)
{
    char buffer[32];
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
            /* NOTE - re-use the 1.0 arch's config file node */
            get_node_via_xpath("/config/sips_account/sqlserver/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlIP, buffer, sizeof(m_strSqlIP));

            get_node_via_xpath("/config/sips_account/sqlserver/user",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserName, buffer, sizeof(m_strSqlUserName));

            get_node_via_xpath("/config/sips_account/sqlserver/password",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserPassword, buffer, sizeof(m_strSqlUserPassword));

            LOG("the SQL server ip:%s, user:%s, password:%s\n",
                    m_strSqlIP, m_strSqlUserName, m_strSqlUserPassword);

            /* FIXME compare with 1.0 arch,
             * I plan add memcached to improve the program efficiency */

            if(get_node_via_xpath("/config/sips_account/memcache/ip",
                    ctx, buffer, sizeof(buffer)) == 0)
            {
                strncpy(m_strMemIP, buffer, sizeof(m_strMemIP));
            }

            if(get_node_via_xpath("/config/sips_account/memcache/port",
                    ctx, buffer, sizeof(buffer)) == 0)
            {
                m_iMemPort = atoi(buffer);
            }

            //LOG("the memcached ip:%s, port:%d\n", m_strMemIP, m_iMemPort);

            // free useless resource...
            xmlXPathFreeContext(ctx);

            INFO("XML config parse finished [OK]\n");
        }

        xmlFreeDoc(doc);
    }

    return prepare_db_and_mem();
}
