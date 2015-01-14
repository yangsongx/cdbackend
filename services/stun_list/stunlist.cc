/**
 * A ping program to make sure token server
 * alive.
 *
 * If there's sth wrong , this program would
 * send out notification mail.
 */
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <getopt.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <list>
#include <string>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#ifdef DEBUG
#include <mcheck.h>
#endif

#include "rfc3489.h"

using namespace std;


#define DAY_SECONDS (3600*24)
#define STUNLIST_PATH  "/opt/usercenter/conf/active.stun"

static int get_xpath_data(const char *xpath, xmlXPathContextPtr ctx, list<string> *server_list) 
{
    xmlXPathObjectPtr  obj;
    obj = xmlXPathEvalExpression((xmlChar *)xpath, ctx);
    if(obj != NULL && obj->nodesetval != NULL)
    {
        char buf[256];
        int size = obj->nodesetval->nodeNr;
        int i;
        xmlNode *node;
        for(i = 0; i < size; i ++)
        {
            node = obj->nodesetval->nodeTab[i];

            if(node->children != NULL)
            {
                node = node->children;
                strcpy(buf, (const char *)node->content);
                string s(buf);
                server_list->push_back(s);
            }
        }

        xmlXPathFreeObject(obj);
    }

    return 0;
}

int get_candiate_server(const char *filename, list<string> *server_list)
{
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    if(access(filename, F_OK) != 0)
    {
        fprintf(stderr, "\'%s\' not existed\n", filename);
        return -1;
    }

    doc = xmlParseFile(filename);
    if(doc != NULL)
    {
        ctx = xmlXPathNewContext(doc);
        if(ctx != NULL)
        {
            get_xpath_data("/config/stunlist/server", ctx, server_list);

            xmlXPathFreeContext(ctx);
        }

        xmlFreeDoc(doc);
    }

    return 0;
}

int update_active_stun_list(list<string> *server_list)
{
    FILE *p = fopen(STUNLIST_PATH, "w");

    if(p != NULL)
    {
        for(list<string>::iterator it = server_list->begin();
           it != server_list->end(); it++)
        {
            fputs(it->c_str(), p);
            fputc(';', p);
        }

        fclose(p);
    }

    return 0;
}

/**
 *
 *
 */
int check_active_stun_list(list<string> *server_list)
{
    list<string> down;

    for(list<string>::iterator it = server_list->begin();
           it != server_list->end(); it++)
    {
        if(check_available(it->c_str()) != 0)
        {
            printf("%s is down, remove it\n", it->c_str());
            down.push_back(*it);
        }
    }
    
    // remove down server from the list
    for(list<string>::iterator it = down.begin();
           it != down.end(); it++)
    {
        server_list->remove(*it);
    }

    printf("Now, active list are:\n");
    for(list<string>::iterator it = server_list->begin();
           it != server_list->end(); it++)
    {
        printf("%s\n", it->c_str());
    }

    update_active_stun_list(server_list);

    return 0;
}

/**
 * main entry, the argc/argv config is higher
 * than config file at ~/.xxxx
 *
 */
int main(int argc, char **argv)
{
    int c;
    int default_interval = 604800; // 7 days (in second)
    int days = 0;
    struct timeval tv;
    list<string> all_servers;

    while((c = getopt(argc, argv, "t:")) != -1)
    {
        switch(c)
        {
            case 't':
                days = atoi(optarg);
                default_interval = (days * DAY_SECONDS);
                break;

            default:
                break;
        }
    }
#if 0
    get_candiate_server("/etc/cds_cfg.xml", &all_servers);
    check_active_stun_list(&all_servers);

    printf("all candiate server are:\n");
    for(list<string>::iterator it = all_servers.begin();
            it != all_servers.end(); it++)
    {
        printf("%s\n", (*it).c_str());
    }
    printf("\n");
#endif
    /* drop into a interval loop... */
    while(1) {
        tv.tv_sec = default_interval;
        tv.tv_usec = 0;

        // empty the list before re-read the config file...
        all_servers.clear();
        get_candiate_server("/etc/cds_cfg.xml", &all_servers);
        check_active_stun_list(&all_servers);

        printf("all candiate server are:\n");
        for(list<string>::iterator it = all_servers.begin();
                it != all_servers.end(); it++)
        {
            printf("%s\n", (*it).c_str());
        }
        printf("\n");

        printf("update the stun server visable at every (%d) day\n",
                (default_interval/DAY_SECONDS));


        c = select(0, NULL, NULL, NULL, &tv);
        if(c < 0)
        {
            if(errno == EAGAIN || errno == EINTR)
            {
                continue;
            }
            fprintf(stderr, "**failed call select() :%d\n", errno);
            break;
        }
        else if(!c)
        {
            // timeout , so we need check active
            // FIXME dont' call this..
            //check_active_stun_list(&all_servers);
        }
        else
        {
            // SHOULD NEVER Happen here!
        }
    }

    printf("quit the stunlist ping program\n");

    return 0;
}
