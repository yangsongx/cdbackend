#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "pipe.h"
#include "listreq.h"

LIBEVENT_THREAD *g_thread_info = NULL;

/* this variable aims to hash on @g_totally_servers */
static int last_background_server = -1;

/**
 * Connecting to a @hash-ed server
 *
 */
int connect_to_hashed_server(int hash, listreq_t *req)
{
    int ret = -1;
    struct server_info *svr = g_server + hash;

    req->lq_server->sv_machine = svr;
    if(connect(req->lq_server->sv_sock,
                (struct sockaddr *)(&(req->lq_server->sv_machine->addr)),
                sizeof(struct sockaddr_in)) == -1)
    {
        ERR("*** failed connecting to [hash %d] server(errno:%d)\n",
                hash, errno);
    }
    else
    {
        LOG("Connecting to [hash %d] OK\n", hash);
        ret = 0;
    }

    return ret;
}

void cb_server_back(int s, short event, void *arg)
{
    listreq_t *req = (listreq_t *)arg;
    size_t size;

    size = read(req->lq_server->sv_sock, req->lq_tx, 1024);
    if(size > 0)
    {
        LOG("   agent<----%ld byte----\n", size);
        // write back to caller
        size = write(req->lq_sock, req->lq_tx, size);
        if(size > 0)
        {
            LOG("client<---(%ld) byte--agent\n", size);
        }
        else
        {
            // TODO - for EINTR/EAGAIN case ?
            LOG("background server probably closed, need cleanup...\n");
            // TODO - for clean up job code..
        }
    }
    else
    {
        // TODO - for EINTR/EAGAIN case ?
        ERR("   agent<-- closed----\n");
        LOG("background server probably closed, need cleanup...\n");
        // TODO - for clean up job code..
    }
}

int route_request(listreq_t *req)
{
    int ret = -1;
    int hash_index = 0;
    size_t size;

    if(!req->lq_server)
    {
        // a new server from scratch
        req->lq_server = (dist_server_t *) malloc(sizeof(dist_server_t));
        if(req)
        {
            dist_server_t *svr = req->lq_server;
            svr->sv_sock = socket(AF_INET, SOCK_STREAM, 0);
            if(svr->sv_sock != -1)
            {
                /* before decide to connect socket, need hash it */
                hash_index = (last_background_server + 1) % g_totally_servers;
                last_background_server = hash_index;

                if(connect_to_hashed_server(hash_index, req) == 0)
                {
                    // Now, write to target, and register read back notify
                    LOG("Will try writing %d data to server(sock:%d)\n", req->lq_rxlen, svr->sv_sock);
                    size = write(svr->sv_sock, req->lq_rx, req->lq_rxlen);
                    LOG("-----(%ld) -----> [hash %d]\n", size, hash_index);
                    event_set(&req->lq_server->sv_event, req->lq_server->sv_sock, EV_PERSIST|EV_READ, cb_server_back, (void *)req);
                    event_base_set(main_base, &req->lq_server->sv_event);
                    if(event_add(&req->lq_server->sv_event, 0) == -1)
                    {
                        ERR("Failed add the event\n");
                    }
                }
                else
                {
                    ERR("***failed connect to target server\n");
                    // TODO, need choose a new one for another connection ?
                }
            }
            else
            {
                ERR("Fatal error, socket creation failed:%d\n", errno);
            }
        }
        else
        {
            ERR("FATAL error, not enuogh memory\n");
        }
    }
    else
    {
        // re-use the current existed connection!
        size = write(req->lq_server->sv_sock, req->lq_rx, req->lq_rxlen);
        LOG("    agent -----(%ld) -----> [hash %d]\n", size, hash_index);
        // FIXME hope don't need event_add here;
    }

    return ret;
}


/**
 * This is the real halding of data send from requester
 *
 */
void cb_handling_request(int fd, short which, void *arg)
{
    listreq_t *req = (listreq_t *)arg;
    size_t size;

    size = read(req->lq_sock, req->lq_rx, 1024); // suppose 1024 is max
    if(size > 0)
    {
        printf("%ld bytes got from the requester\n", size);
        req->lq_rxlen = size;

        if(route_request(req) == 0)
        {
            LOG("Cool route the req\n");
        }
        else
        {
            ERR("failed route the req\n");
        }
    }
    else
    {
        // FIXME - need handle EINTR/EAGAIN case...
        ERR("client probably closed.\n");
        close_reqlist(req);
    }
}


void cb_there_are_requests(int fd, short which, void *arg)
{
    LIBEVENT_THREAD *info = (LIBEVENT_THREAD *)arg;
    listreq_t *req = NULL;
    char data;
    size_t size;

    size = read(info->lt_notify[0], &data, 1);
    if(data != 'c' || size != 1)
    {
        LOG("bad notify result\n");
        return;
    }

    // code for got notified that there're data!
    printf("You(%d) wake me up!, try get node from list queue...\n", fd);
    req = pop_from_reqlist(&g_list_hdr, &g_list_mutex);
    if(req)
    {
        // use a libevent on this request's socket
        event_set(&req->lq_event, req->lq_sock, EV_PERSIST|EV_READ, cb_handling_request, (void *)req);
        event_base_set(info->lt_base, &req->lq_event); // using main base?
        if(event_add(&req->lq_event, 0) == -1)
        {
            ERR("failed add the handling socket event\n");
        }
    }
}

/**
 * Consumer from the pipe request
 *
 */
void *thread_func(void *param)
{
    LIBEVENT_THREAD *info = (LIBEVENT_THREAD *)param;

    LOG("CHILD THREAD..\n");
    if(pipe(info->lt_notify) != 0)
    {
        ERR("**failed create the pipe(%d)\n", errno);
    }
    else
    {
        LOG("Thread created the pipe (%d) (%d)[OK]\n", info->lt_notify[0], info->lt_notify[1]);
    }

    info->lt_base = event_init();
    event_set(&info->lt_event, info->lt_notify[0], EV_READ|EV_PERSIST, cb_there_are_requests, param);

    event_base_set(info->lt_base, &info->lt_event);
    if(event_add(&info->lt_event, 0) == -1)
    {
        ERR("failed add event\n");
    }

    /* thread go into the loop */
    event_base_loop(info->lt_base, 0);

    return NULL;
}


/**
 *@nr : the thread counts(based on CPU core)
 *
 */
void spawn_worker_thread(int nr)
{
    int i;
    pthread_t  tid;

    if(nr <= 0)
    {
        ERR("not thread created..\n");
        return;
    }

    g_thread_info = (LIBEVENT_THREAD *) malloc(sizeof(LIBEVENT_THREAD) * nr);
    if(!g_thread_info)
    {
        ERR("no momoery\n");
        return;
    }

    for(i = 0; i < nr; i++)
    {
        // passing each thread with each info
        if(pthread_create(&tid, NULL, thread_func, (void *)(g_thread_info + i)) != 0)
        {
            ERR("**failed create the thread!\n");
        }
        else
        {
            INFO("Child thread[%d] creation [OK]\n", i);
        }
    }
}
