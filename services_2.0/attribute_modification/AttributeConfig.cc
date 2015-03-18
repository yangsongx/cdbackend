#include "AttributeConfig.h"

int AttributeConfig::parse_cfg(const char *config_file)
{
    char buffer[32]; // be consistent with the data member size
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
            get_node_via_xpath("//service_2/attr_modify/sqlserver/ip",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlIP, buffer, sizeof(m_strSqlIP));

            get_node_via_xpath("//service_2/attr_modify/sqlserver/user",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserName, buffer, sizeof(m_strSqlUserName));

            get_node_via_xpath("//service_2/attr_modify/sqlserver/password",
                    ctx, buffer, sizeof(buffer));
            strncpy(m_strSqlUserPassword, buffer, sizeof(m_strSqlUserPassword));

            LOG("the SQL server ip:%s, user:%s, password:%s\n",
                    m_strSqlIP, m_strSqlUserName, m_strSqlUserPassword);

            if(get_node_via_xpath("//service_2/attr_modify/memcache/ip",
                    ctx, buffer, sizeof(buffer)) == 0)
            {
                strncpy(m_strMemIP, buffer, sizeof(m_strMemIP));
            }

            if(get_node_via_xpath("//service_2/attr_modify/memcache/port",
                    ctx, buffer, sizeof(buffer)) == 0)
            {
                m_iMemPort = atoi(buffer);
            }

            xmlXPathFreeContext(ctx);

            INFO("XML config parse finished [OK]\n");
        }

        xmlFreeDoc(doc);
    }

    return prepare_db_and_mem();
}
