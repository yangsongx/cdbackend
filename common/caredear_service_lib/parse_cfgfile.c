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
            }
        }

        xmlXPathFreeObject(obj);
    }

    return 0;
}

void fill_server_info(xmlXPathContextPtr ctx, struct sql_server_info *server)
{
    char buf[32];

    get_node_via_xpath("/config/sqlserver/ip", ctx, server->ssi_server_ip, 32);
    get_node_via_xpath("/config/sqlserver/port", ctx, buf, sizeof(buf));
    get_node_via_xpath("/config/sqlserver/user", ctx, server->ssi_user_name, 32);
    get_node_via_xpath("/config/sqlserver/password", ctx, server->ssi_user_password, 32);
    get_node_via_xpath("/config/sqlserver/databasename", ctx, server->ssi_database, 32);

    server->ssi_server_port = atoi(buf);
}


/**
 *parse the config XML file.
 *
 *@cfg_name : the XML file name.
 */
int parse_config_file(const char *cfg_name, struct sql_server_info *info)
{
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

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
