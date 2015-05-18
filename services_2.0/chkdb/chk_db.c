/**
 * A test program to check DB SQL result
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
#include <netinet/in.h>
#include <arpa/inet.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>

MYSQL  *mSql;
char sqlip[32];
char sqluser[32];
char sqlpasswd[32];

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

int try_conn_db(const char *config_file)
{
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    if(access(config_file, F_OK) != 0)
    {
        printf("\'%s\' not existed!\n", config_file);
        return -1;
    }

    doc = xmlParseFile(config_file);
    if(doc != NULL)
    {
        ctx = xmlXPathNewContext(doc);
        if(ctx != NULL)
        {
            get_node_via_xpath("//service_2/user_register_service/sqlserver/ip",
                    ctx, sqlip, sizeof(sqlip));

            get_node_via_xpath("//service_2/user_register_service/sqlserver/user",
                    ctx, sqluser, sizeof(sqluser));

            get_node_via_xpath("//service_2/user_register_service/sqlserver/password",
                    ctx, sqlpasswd, sizeof(sqlpasswd));

            printf("the SQL server ip:%s, user:%s, password:%s\n",
                    sqlip, sqluser,sqlpasswd);


            mSql = mysql_init(NULL);
            if(mSql != NULL){
                int a = 1;
                /* begin set some options before real connect */
                //mysql_options(mSql, MYSQL_OPT_RECONNECT, &a);
                a = 2;
                mysql_options(mSql, MYSQL_OPT_CONNECT_TIMEOUT, &a);
                mysql_options(mSql, MYSQL_OPT_READ_TIMEOUT, &a);
                printf("Set SQL options...\n");

                if(!mysql_real_connect(mSql, sqlip, sqluser, sqlpasswd,
                            "uc", 0, NULL, 0)) {
                    printf("error for mysql_real_connect():%s\n",
                            mysql_error(mSql));
                    return -1;
                }
            }
        }

        xmlFreeDoc(doc);
    }

    return 0;
}

uint64_t get_account_id(const char *name)
{
    uint64_t id = (uint64_t) -1;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT id FROM uc_passport WHERE usermobile=\'%s\'",
            name);
    if(!mysql_query(mSql, sqlcmd))
    {
        MYSQL_RES *mresult = mysql_store_result(mSql);
        if(mresult)
        {
            MYSQL_ROW row = mysql_fetch_row(mresult);
            if(row != NULL && row[0] != NULL)
            {
                id = atol(row[0]);
                printf("\'%s\' ===> %lu\n", name, id);
            }
            mysql_free_result(mresult);
        }
    }
    else
    {
        printf("***failed get id from DB for \'%s\':%s\n",
                name, mysql_error(mSql));
    }

    return id;
}

int delete_account_from_db(const char *name)
{
    char sqlcmd[1024];
    uint64_t id;

    id = get_account_id(name);
    if(id != (uint64_t) -1)
    {
        snprintf(sqlcmd, sizeof(sqlcmd),
                "DELETE FROM uc_attributes WHERE caredearid=%lu\n", id);
        if(mysql_query(mSql, sqlcmd))
        {
            printf("failed delete from uc_attributes:%s\n", mysql_error(mSql));
        }
        else
        {
            printf("totally %llu recoreds deleted in uc_attributes\n",
                    mysql_affected_rows(mSql));
        }

        snprintf(sqlcmd, sizeof(sqlcmd),
                "DELETE FROM uc_session WHERE caredearid=%lu\n", id);
        if(mysql_query(mSql, sqlcmd))
        {
            printf("failed delete from uc_session:%s\n", mysql_error(mSql));
        }
        else
        {
            printf("totally %llu recoreds deleted in uc_session\n",
                    mysql_affected_rows(mSql));
        }

        snprintf(sqlcmd, sizeof(sqlcmd),
                "DELETE FROM uc_contact WHERE caredearid=%lu\n", id);
        if(mysql_query(mSql, sqlcmd))
        {
            printf("failed delete from uc_contact:%s\n", mysql_error(mSql));
        }
        else
        {
            printf("totally %llu recoreds deleted in uc_contact\n",
                    mysql_affected_rows(mSql));
        }

        snprintf(sqlcmd, sizeof(sqlcmd),
                "DELETE FROM uc_passport WHERE id=%lu\n", id);
        if(mysql_query(mSql, sqlcmd))
        {
            printf("failed delete from uc_passport:%s\n", mysql_error(mSql));
        }
        else
        {
            printf("totally %llu recoreds deleted in uc_passport\n",
                    mysql_affected_rows(mSql));
        }
    }

    return 0;
}

void show_usage()
{
    printf("Usage: ./chk_db -s sqlfile\n");
}

int execute_sql_cmd(const char *cmd)
{
    int i;

    if(mysql_query(mSql, cmd))
    {
        printf("***failed execute the cmd:%s\n", mysql_error(mSql));
        printf("\nThe whole SQL cmd:%s\n", cmd);
        return 1;
    }

    printf("SQL execution... [OK]\n");
    MYSQL_RES *mresult = mysql_store_result(mSql);
    if(mresult)
    {
        MYSQL_ROW row;
        int cols = mysql_num_fields(mresult);
        printf("totally %d column\n", cols);

        while((row = mysql_fetch_row(mresult)) != NULL){
            for(i = 0; i < cols; i++) {
                printf("%s |", row[i]);
            }
            printf("\n");
        }

        mysql_free_result(mresult);
    }
    return 0;
}

int trigger_cmd_in_file(const char *fn)
{
    char line[2048];
    int ret = -1;
    FILE *p;

    if(access(fn, F_OK) != 0)
    {
        printf("sql cmd file \'%s\' not existed!\n", fn);
        return -1;
    }

    p = fopen(fn, "r");
    if(!p)
    {
        printf("**** failed open \'%s\' file:%d\n", fn, errno);
        return -1;
    }

    while((fgets(line, sizeof(line), p)) != NULL)
    {
        printf("line:%s\n", line);
        line[strlen(line) - 1] = '\0'; // strip newline with NULL-terminating char
        execute_sql_cmd(line);
    }

    return ret;
}

/**
 * main entry
 */
int main(int argc, char **argv)
{
    int c;
    char sql_file_name[256];

    if(argc < 2)
    {
        show_usage();
        exit(1);
    }

    while((c = getopt(argc, argv, "s:h")) != -1)
    {
        switch(c) {
        case 'h':
            show_usage();
            break;

        case 's':
            strncpy(sql_file_name, optarg, sizeof(sql_file_name));
            break;

        default:
            break;
        }
    }

    printf("will execute SQL cmd in %s...\n", sql_file_name);

    if(try_conn_db("/etc/cds_cfg.xml") == -1)
    {
        printf("Conn to SQL ... [Failed]\n");
        return -1;
    }
    printf("Conn to SQL ... [OK]\n");

    trigger_cmd_in_file(sql_file_name);

    mysql_close(mSql);

    return 0;
}
