/**
 *gr.c - Get Registered User Info
 *
 * This is a program select all wanted User data from Server,
 * for statistic the QingLiang APK's data, analysis some patterns.
 *
 *\history
 * [2015-04-14] try test on the new 2.0 arch user center design.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <my_global.h>
#include <mysql.h>

/* All default data */
#define DEFAULT_FULLDATA   "fulluser.txt"
#define DEFAULT_SQLDATA    "my.sql"

void get_node_via_xpath(const char *xpath, xmlXPathContextPtr ctx, char *result, int result_size)
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
}

void usage()
{
    printf("gr - get user mobile from DB, options:\n");
    printf("     -s sqlfile    specify input SQL file\n");
    printf("     -o outfile    store SQL result to outfile(in *.txt by convention)\n");
    printf("     -a            combile all data file(.txt) into zip,and auto-upload.\n");
}

MYSQL *GET_MYSQL()
{
    char sql_ip[32];
    int  sql_port;
    char sql_usr[32];
    char sql_passwd[32];
    char buf[32];

    MYSQL *msql = NULL;
    printf("MySQL Client Info : %s\n", mysql_get_client_info());

    /* before conn, try get the config info from XML file... */
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    if(access("/etc/cds_cfg.xml", F_OK) != 0)
        return NULL;

    doc = xmlParseFile("/etc/cds_cfg.xml");
    if(doc != NULL)
    {
        ctx = xmlXPathNewContext(doc);
        if(ctx != NULL)
        {
            get_node_via_xpath("/config/service_2/user_register_service/sqlserver/ip",
                    ctx, sql_ip, sizeof(sql_ip));

            get_node_via_xpath("/config/service_2/user_register_service/sqlserver/port",
                    ctx, buf, sizeof(buf));
            sql_port = atoi(buf);

            get_node_via_xpath("/config/service_2/user_register_service/sqlserver/user",
                    ctx, sql_usr, sizeof(sql_usr));
            get_node_via_xpath("/config/service_2/user_register_service/sqlserver/password",
                    ctx, sql_passwd, sizeof(sql_passwd));

            msql = mysql_init(NULL);
            if(msql != NULL)
            {
                if(!mysql_real_connect(msql, sql_ip, sql_usr, sql_passwd, "uc", sql_port, NULL, 0))
                {
                    printf("** Failed connect to MySQL:%s\n", mysql_error(msql));
                    mysql_close(msql);
                    msql = NULL;
                }
                else
                {
                    printf("\n\n Connect to %s : %d, with user %s [OK]\n",
                            sql_ip, sql_port, sql_usr);
                }
            }

            xmlXPathFreeContext(ctx);
        }
        xmlFreeDoc(doc);
    }

    return msql;
}

void FREE_MYSQL(MYSQL *m)
{
    if(m != NULL)
        mysql_close(m);
}


/**
 *
 *@m : MySQL DB
 *@f : the result file handler
 */
void begin_extract_user_info_from_db(MYSQL *m, FILE *outf, FILE *sqlf)
{
    char sqlcmd [1024];

    fgets(sqlcmd, sizeof(sqlcmd), sqlf);

    if(mysql_query(m, sqlcmd))
    {
        printf("failed execute the sql command:%s\n", mysql_error(m));
        return;
    }

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(m);
    if(mresult)
    {
        int b = mysql_num_fields(mresult);
        //printf("=>We got totally %d rows in DB\n", b);
        while(row = mysql_fetch_row(mresult))
        {
            fprintf(outf, "%s\n", row[0]);
        }
    }
}

int main(int argc, char **argv)
{
    int c;
    MYSQL *msql;

    char sql_file[32];
    char out_file[32];

    strcpy(sql_file, DEFAULT_SQLDATA);
    strcpy(sql_file, DEFAULT_FULLDATA);

    while((c = getopt(argc, argv, "ahs:o:")) != -1)
    {
        switch(c)
        {
            case 's':
                strcpy(sql_file, optarg);
                break;

            case 'o':
                strcpy(out_file, optarg);
                break;

            case 'h':
                usage();
                exit(0);
                break;

            case 'a':
                // TODO need do automatic job for this
                break;

            default:
                break;
        }
    }

    msql = GET_MYSQL();
    if(!msql)
        exit(0);

    FILE *out_fp = fopen(out_file, "w");
    if(out_fp != NULL)
    {
        FILE *sql_fp = fopen(sql_file, "r");
        if(sql_fp != NULL)
        {
            begin_extract_user_info_from_db(msql, out_fp, sql_fp);
            fclose(sql_fp);
        }

        fclose(out_fp);
    }

    printf("\"%s\" ==> \"%s\"\n", sql_file, out_file);

    FREE_MYSQL(msql);

    return 0;
}
