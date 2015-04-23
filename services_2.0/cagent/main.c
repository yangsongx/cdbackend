/**
 * Try to learn all flow steps of a libevent I/O framework
 * (based on memcached code)
 *
 */
#define _BSD_SOURCE

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mcheck.h>

/* libevent */
#include <event.h>

#include "listreq.h"
#include "pipe.h"


int g_port = 19000;

int                 g_totally_servers = 0;
struct server_info *g_server;

struct event_base *main_base;

static int g_thread_count = 1;
static int last_thread = -1; // let all thread hit in average rate

pthread_mutex_t g_list_mutex;
LIST_HEAD(g_list_hdr);
LIST_HEAD(g_free_list);

pipe_t *g_testP;
pipe_producer_t *g_prod = NULL;
pipe_consumer_t *g_cons = NULL;

/* simple hash the server's count for distributed arch */
unsigned short g_server_count = 0;

void test_pipe()
{
    pipe_t *p = pipe_new(sizeof(int), 0);
    pipe_producer_t* prod = pipe_producer_new(p);
    pipe_consumer_t* cons = pipe_consumer_new(p);

    printf("pipe:%p, prod:%p, cons:%p\n", p, prod, cons);

    pipe_free(p);

    int a[] = {1,2,3,4,5};
    int b[] = {6,7,8,9,10};
    pipe_push(prod, a, 5);
    pipe_push(prod, b, 5);

    pipe_producer_free(prod);

    size_t size;
    int buf[408999];
    size = pipe_pop(cons, buf, sizeof(buf));
    printf("got %ld from pipe_pop\n", size);

    printf("\n");
}


void study_pipe()
{
    g_testP = pipe_new(sizeof(listreq_t), 0);
    //listreq_t node;

    if(!g_testP)
    {
        ERR("**failed new mem, quit\n");
        return;
    }

    LOG("pipe(%p) created [OK]\n", g_testP);

    g_prod = pipe_producer_new(g_testP);
    g_cons = pipe_consumer_new(g_testP);

    printf("Now, the addr of prod:%#x, cons:%#x\n",
            *(int *)g_prod, *(int *)g_cons);


    // FIXME, free here???
    pipe_free(g_testP);
}


/**
 *
 *@index : specifiy which server to be choosed(based on a hash algorithm)
 *
 */


/**
 * Call this util after got the request data from incoming socket
 * connection.
 *
 *
 */


/**
 * This is for incoming connect() socket handler.
 *
 */
void on_accept(int s, short event, void *arg)
{
    int c;
    char data = 'c';
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    listreq_t *req;
    LIBEVENT_THREAD *pt = NULL;
    int i;

    LOG("------------Incoming Req on [%d]-------->\n", s);

    c = accept(s, (struct sockaddr *)&addr, &len);
    if(c == -1)
    {
        ERR("**failed accept the incoming connection(%d)\n", errno);
        return;
    }
    LOG("+--%s:%d----[%d]----->[OK]\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), c);

    // average distributed hashing
    i = (last_thread + 1 ) % g_thread_count;
    pt = g_thread_info + i;
    last_thread = i;

    // each incoming connection represented as a request list
    req = new_reqlist();
    if(!req)
    {
        ERR("*** failed malloc memory!\n");
        return;
    }

    req->lq_sock = c;

    /* adding the req to queue list */
    list_push_queue_safe(&g_list_hdr,
            &(req->list),
            &g_list_mutex);
    /* then notify thread to get it */
    printf("writing to (%d) ...", pt->lt_notify[1]);
    if(write(pt->lt_notify[1], &data, 1) != 1)
    {
        ERR("failed notify pipe(%d)\n", errno);
    }
    else
    {
        LOG("notify the thread to fetch it\n");
    }
}

void cleanup_res()
{
    if(g_server!= NULL)
        free(g_server);

    if(g_thread_info != NULL)
        free(g_thread_info);

    event_base_free(main_base);

    exit(0);
}

void sig_handler(int signo)
{
    switch(signo)
    {
        case SIGTERM:
        case SIGINT:
            printf("SIGINT/SIGTERM caught, clean up all resources...\n");
            cleanup_res();
            break;
    }

    exit(0);
}

int main(int argc, char **argv)
{
    int c;
    int i;
    char *p;
    int agent_s = -1;
    struct sockaddr_in addr;
    int flags = 1;
    struct linger ling = {0, 0};
    struct event listen_ev;
    struct server_info *curs = NULL;
#if 1
    setenv("MALLOC_TRACE", "/tmp/pip.mem", 1);
    mtrace();
#endif

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    while((c = getopt(argc, argv, "hs:p:")) != -1) {
        switch(c) {
            case 'h':
                break;

            case 's':
                printf("hello?(%d)\n", g_totally_servers);
                // store all server into a list
                if(g_totally_servers == 0)
                {
                    g_server = (struct server_info *)calloc(sizeof(struct server_info), 1);
                    if(g_server != NULL)
                    {
                        g_totally_servers++;
                        curs = g_server;
                    }
                }
                else
                {
                    g_totally_servers++;
                    g_server = (struct server_info *)realloc(g_server, sizeof(struct server_info)*(g_totally_servers));
                    if(g_server == NULL)
                    {
                        ERR("Warning, failed allocate the memory!\n");
                    }
                    else
                    {
                        curs = g_server + (g_totally_servers - 1);
                    }
                }
                p = strchr(optarg, ':');
                if(!p)
                {
                    strncpy(curs->ip, optarg, 32);
                    curs->port = 11211; //default port
                }
                else
                {
                    memset(curs, 0x00, sizeof(struct server_info));
                    memcpy(curs->ip, optarg, ((p - optarg)));
                    curs->port = atoi(p + 1);
                    LOG("ip:%s, port:%d\n", curs->ip, curs->port);
                }

                curs->addr.sin_family = AF_INET;
                curs->addr.sin_addr.s_addr = inet_addr(curs->ip);
                curs->addr.sin_port = htons(curs->port);
                break;

            case 'p':
                g_port = atoi(optarg);
                break;

            default:
                break;
        }
    }

    // only count on XML when user didn't input via '-s xxx:xx'
    if(g_totally_servers == 0)
    {
        printf("seems didn't specify server info in command, try get them from cfg file...\n");
        get_server_from_config("/etc/cds_cfg.xml");
    }

    printf("libevent version:%s\n", event_get_version());
    printf("\n\nTotally %d server in the list:\n", g_totally_servers);
    for(i = 0; i < g_totally_servers; i++)
    {
        printf(" [%d] IP : %s, Port : %d\n", i, inet_ntoa(g_server[i].addr.sin_addr), ntohs(g_server[i].addr.sin_port));
    }

    printf("\n\n");

    agent_s = socket(AF_INET, SOCK_STREAM, 0);
    if(agent_s == -1)
    {
        ERR("***failed create the server socket(%d)\n", errno);
        goto end_program;
    }

    if(setsockopt(agent_s, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags)) == -1)
    {
        ERR("Warning, set SO_REUSEADDR option failed(%d)\n", errno);
    }

    if(setsockopt(agent_s, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags)) == -1)
    {
        ERR("Warning, set SO_KEEPALIVE option failed(%d)\n", errno);
    }

    if(setsockopt(agent_s, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling)) == -1)
    {
        ERR("Warning, set SO_LINGER option failed(%d)\n", errno);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(g_port);
    if(bind(agent_s, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        ERR("**failed bind the server socket:%d\n", errno);
        goto end_program;
    }

    INFO("bind server socket ... [OK]\n");

    if(listen(agent_s, LISTEN_BACKLOG) == -1)
    {
        ERR("**failed listen on server socket:%d\n", errno);
        goto end_program;
    }

    //test_pipe();
    //study_pipe();

    /* create child threads... */
    spawn_worker_thread(g_thread_count);

    /* Now, begin libevent trigger */
    main_base = event_base_new();

    event_set(&listen_ev, agent_s, EV_READ|EV_PERSIST, on_accept, NULL);
    event_base_set(main_base, &listen_ev);
    event_add(&listen_ev, NULL);

    /* while() loop */
    event_base_loop(main_base, 0);

end_program:

    if(agent_s != -1)
        close(agent_s);

    return 0;
}
