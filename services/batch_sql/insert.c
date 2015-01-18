/**
 * batch insert a bunch of data
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <my_global.h>
#include <mysql.h>




static struct option myoptions[] = {
    {"help", no_argument, 0, 'h'},
    {"add",  required_argument, 0, 0},
    {"sql",  required_argument, 0, 's'},
    {0, 0, 0, 0}

};

void insert_user_into_db(MYSQL *ms, const char *username)
{
    int ret = 0;
    char sqlcmd[1024];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "INSERT INTO subscriber (username,domain) VALUES (\'%s\',\'rdp.caredear.com\');",
            username);

    ret = mysql_query(ms, sqlcmd);
    if(ret != 0)
    {
        printf("error : %s\n", mysql_error(ms));
        return;
    }

    printf("SQL QUERY [OK]\n\n");

    MYSQL_RES  *mresult = mysql_store_result(ms);
    // FIXME for such update case, mresult is actually NULL.
    if(mresult)
    {
        // do nothing;
    }
}

void insert_all_data(MYSQL *ms, const char *filename)
{
    int count = 0;
    FILE *p;
    char buf[256];

    p = fopen(filename, "r");

    if(p == NULL)
    {
        printf("Failed open %s file...\n", filename);
        return;
    }

    while(fgets(buf, sizeof(buf), p) != NULL)
    {
        count++;
        printf("name:%s\n", buf);
        insert_user_into_db(ms, buf);
    }

    printf("Totally %d users updated into DB...\n", count);

    fclose(p);
}

int main(int argc, char **argv)
{
    int rc;
    MYSQL *ms;
    ms = mysql_init(NULL);
#if 0
    rc = mysql_real_connect(ms,
            "127.0.0.1",  // SQL SERVER IP
            "opensips",   // name
            "opensipsrw",  // password
            "opensips", // DB
            0, NULL, 0);

    if(rc != 0)
    {
        printf("*** Failed connect to DB\n");
        return -1;
    }
#endif
    printf("Connect to the DB OK!\n");
#if 1

    insert_all_data(ms, "user.txt");
#else
    int c;
    int option_index;
    char filename[128];
    while((c = getopt_long(argc, argv, "s:h", myoptions, &option_index)) != -1)
    {
        switch(c)
        {
            case 0:
                printf("option %s  ", myoptions[option_index].name);
                if(optarg)
                {
                    printf("with arg %s", optarg);
                }
                printf("\n");
                break;

            case 'h':
                printf("help info\n");
                exit(1);
                break;

            case 's':
                strcpy(filename, optarg);
                printf("SQL file : %s\n", filename);
                break;

            default:
                break;
        }
    }

    char sqlcmd[328];
    FILE *p = fopen(filename, "r");
    if(p)
    {
        fgets(sqlcmd, sizeof(sqlcmd), p);
        printf("the input SQL cmd is:%s\n", sqlcmd);
        fclose(p);
    }


    int ret = 0;

    ret = mysql_query(ms, sqlcmd);
    printf("the mysql_query returned %d\n", ret);

    if(ret != 0)
    {
        printf("error : %s\n", mysql_error(ms));
        goto failed;
    }

    printf("SQL QUERY [OK]\n\n");

    MYSQL_RES  *mresult = mysql_store_result(ms);
    if(mresult)
    {
        int i;
        MYSQL_ROW row;
        row = mysql_fetch_row(mresult);
        if(row == NULL)
        {
            printf("Empty data result!\n");
        }
        else
        {
            int col = mysql_num_fields(mresult);
            printf("data:\n\n");
            for(i = 0; i < col; i++)
            {
                printf("%s |", row[i]);
            }
            printf("\n");
        }
    }
    else
    {
        printf("Oh, get a NULL mresult:%s\n", mysql_error(ms));
    }
#endif

failed:
    mysql_close(ms);
    printf("exit the program\n");
    return 0;
}
