/**
 * Utils for parse XML config files.
 */

#include "memcached.h"

#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

int get_node_via_xpath(const char *xpath, xmlXPathContextPtr ctx, char *result, int result_size)
{
    int ret = -1;

    xmlXPathObjectPtr  obj;
    obj = xmlXPathEvalExpression((xmlChar *)xpath, ctx);
    if(obj != NULL && obj->nodesetval != NULL)
    {
        int size = obj->nodesetval->nodeNr;
        if(size > 0)
        {
            xmlNode *node = obj->nodesetval->nodeTab[0];

            if(node->children != NULL)
            {
                node = node->children;
                strncpy(result, (const char *)node->content, result_size);
                ret = 0;
            }
        }

        xmlXPathFreeObject(obj);
    }

    return ret;
}

void fill_server_info(xmlXPathContextPtr ctx, struct sql_server_info *server)
{
    char buf[32];

    /* TODO - need let below token_auth node moved to token_auth component */
    get_node_via_xpath("/config/token_auth/sqlserver/ip", ctx, server->ssi_server_ip, 32);
    get_node_via_xpath("/config/token_auth/sqlserver/port", ctx, buf, sizeof(buf));
    get_node_via_xpath("/config/token_auth/sqlserver/user", ctx, server->ssi_user_name, 32);
    get_node_via_xpath("/config/token_auth/sqlserver/password", ctx, server->ssi_user_password, 32);
    get_node_via_xpath("/config/token_auth/sqlserver/databasename", ctx, server->ssi_database, 32);

    server->ssi_server_port = atoi(buf);
}


/**
 *parse the config XML file.
 *
 *@cfg_name : the XML file name.
 *
 * return -1 for failure(such as not exist the file), 0 for successful
 */
int parse_config_file(const char *cfg_name, struct sql_server_info *info)
{
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    if(access(cfg_name, F_OK) != 0)
    {
        /* File not existed, don't do further job */
        return -1;
    }

    doc = xmlParseFile(cfg_name);
    if(doc != NULL)
    {
        ctx = xmlXPathNewContext(doc);
        if(ctx != NULL)
        {
            fill_server_info(ctx, info);

            xmlXPathFreeContext(ctx);
        }

        xmlFreeDoc(doc);
    }

    return 0;
}
