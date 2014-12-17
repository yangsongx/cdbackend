/* It is a MySQL robust testing code,

   build method:
   gcc sql_robust_testing.c `mysql_config --cflags` `mysql_config --libs`

   execute command:
   ./a.out [simulatneous client request num]

 */

/* the MySQL document state some tips for simultaneous topics:

    Multiple threads cannot send a query to the MySQL server at the same time on the same connection.
    In particular, you must ensure that between calls to mysql_query() and mysql_store_result() in
    one thread, no other thread uses the same connection. You must have a mutex lock around your pair
    of mysql_query() and mysql_store_result() calls. After mysql_store_result() returns, the lock can
    be released and other threads may query the same connection.

    To use mysql_use_result(), you must ensure that no other thread is using the same connection until
    the result set is closed. However, it really is best for threaded clients that share the same
    connection to use mysql_store_result()
 */
#include <stdio.h>
#include <my_global.h>
#include <mysql.h>
#include <string.h>
#include <pthread.h>

#define DEFAULT_CLIENT 2
#define S printf

MYSQL *msql = NULL;
pthread_mutex_t sql_mutex;

void *testing_sql_thread(void *param)
{
    int c,i;
    memcpy(&c, param, sizeof(c));
    int s;
    char buf[32];
    int random_sleep = (rand()%3);

    S("[Child Thread-%d]-->\n",c);
    char sqlcmd[256];

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT * FROM USER_DETAILS;");
    pthread_mutex_lock(&sql_mutex);
    if(mysql_query(msql, sqlcmd))
    {
        printf("[Child Thread-%d]**failed execute the SQL command:%s\n",
                c, mysql_error(msql));
        return NULL;
    }

    printf("[Child Thread-%d] execute the (%s) [OK]\n", c, sqlcmd);

    MYSQL_RES *mresult;
    MYSQL_ROW  row;
    mresult = mysql_store_result(msql);
    pthread_mutex_unlock(&sql_mutex);

    if(mresult)
    {
        int a = mysql_num_rows(mresult);
        int b = mysql_num_fields(mresult);

        printf("\tSQL RESULT:\n\trow lines=%d, column numbers=%d\n",
                a, b);
        while((row = mysql_fetch_row(mresult)))
        {
            ;;;
#if 0
            for(i = 0; i < b; i++)
            {
                printf("%s | ", row[i] ? row[i] : "NULL");
            }
            printf("\n");
#endif
        }
    }

    printf("[Child Thread-%d]<-Quit\n", c);
    return NULL;
}

int main(int argc, char **argv)
{
    int i;
    int client_count = DEFAULT_CLIENT;

    if(pthread_mutex_init(&sql_mutex, NULL) != 0)
    {
        printf("**Failed init mutex\n");
        return -1;
    }

    if(argc > 1)
    {
        client_count = atoi(argv[1]);
    }

    msql = mysql_init(NULL);
    if(msql != NULL)
    {
        if(!mysql_real_connect(msql, "localhost",
                "root", // user
                "njcm", //password,
                "ucen", // db name
                0, // port
                NULL,
                0))
        {
            printf("**failed connect MySQL:%s\n",
                mysql_error(msql));
            mysql_close(msql);
            msql = NULL;
            return -2;
        }
        else
        {
            printf("\tConnecting to MySQL Database....[OK]\n");
        }
    }

    printf("simulate %d client requesting to MySQL...\n", client_count);
    pthread_t *tid = (pthread_t *)malloc(client_count * sizeof(pthread_t));
    int *cnt = (int *)malloc(client_count*sizeof(int));
    for(i = 0; i < client_count; i++)
    {
        cnt[i] = i;
        pthread_create(&tid[i],
                NULL, testing_sql_thread, &cnt[i]);
    }
    for(i = 0; i < client_count; i++)
    {
        pthread_join(tid[i], NULL);
    }


    printf("quit from robust testing.\n");

    return 0;
}
