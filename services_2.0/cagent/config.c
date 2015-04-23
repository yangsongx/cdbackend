/**
 * Parse config file(XML format)
 *
 */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "listreq.h"

LIST_HEAD(cfg_server_hdr);

static int get_data_via_xpath(const char *xpath, xmlXPathContextPtr ctx)
{
    int ret = -1;
    xmlNode *node;
    struct _xmlAttr *attr;
    xmlXPathObjectPtr  obj;

    obj = xmlXPathEvalExpression((xmlChar *)xpath, ctx);
    if(obj != NULL && obj->nodesetval != NULL)
    {
        int size = obj->nodesetval->nodeNr;
        printf("Totally %d background server\n", size);

        if(size > 0)
        {
            g_totally_servers = size;
            g_server = (struct server_info *)malloc(sizeof(struct server_info)*size);
            if(!g_server)
            {
                ERR("**Fatal error, not enough memory\n");
            }
            else
            {
                for(ret = 0; ret < size; ret++)
                {
                    node = obj->nodesetval->nodeTab[ret];
                    printf("node name:%s, node contenet:%s ... ", node->name, node->content);
                    attr = node->properties;
                    while(attr != NULL)
                    {
                        /* store server info */
                        if(!strcmp((const char *)attr->name, "ip"))
                        {
                            strcpy(g_server[ret].ip, (const char *)attr->children->content);
                        }
                        else if(!strcmp((const char *)attr->name, "port"))
                        {
                            g_server[ret].port = atoi((const char *)attr->children->content);
                        }

                        printf("[%d] server ip :%s, port:%d\n",
                                ret, g_server[ret].ip, g_server[ret].port);

                        g_server[ret].addr.sin_family = AF_INET;
                        g_server[ret].addr.sin_addr.s_addr = inet_addr(g_server[ret].ip);
                        g_server[ret].addr.sin_port = htons(g_server[ret].port);

                        attr = attr->next;
                    }
                }
            }
        }

        xmlXPathFreeObject(obj);
    }

    return ret;
}

int get_server_from_config(const char *fn)
{
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    if(access(fn, F_OK) != 0)
    {
        ERR("\'%s\' not existed!\n", fn);
        return -1;
    }

    doc = xmlParseFile(fn);
    if(doc != NULL)
    {
        ctx = xmlXPathNewContext(doc);
        if(ctx != NULL)
        {
            get_data_via_xpath("/config/service_2/cluster/server", ctx);
        }

        xmlFreeDoc(doc);
    }

    return 0;
}
