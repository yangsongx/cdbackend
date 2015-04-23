#ifndef _NEWCDAGENT_LISTREQ_H
#define _NEWCDAGENT_LISTREQ_H

#include <pthread.h>

/* libevent */
#include <event.h>
#include "pipe.h"

/* FIXME, don't let this program mised with libcds,
 * so macro re-defined here */

#ifdef DEBUG
#define LOG(fmt, args...)  printf(fmt, ##args)
#else
#define LOG(fmt, args...) do {}while(0)
#endif

#define ERR  printf
#define INFO printf

/* FIXME list queue count, steal from memcached source code */
#define LISTEN_BACKLOG  1024

struct list_head{
    struct list_head *prev;
    struct list_head *next;
};

typedef struct {
    pthread_t           lt_tid;
    int                 lt_notify[2]; // foreground notify background thread, we use pipe, so defined [2] here
    struct event_base  *lt_base;
    struct event        lt_event;
}LIBEVENT_THREAD;


struct server_info{
    char ip[32];
    int  port;
    struct sockaddr_in addr;
};

typedef struct _distributed_server{
    int                 sv_sock;
    struct server_info *sv_machine;
    struct event        sv_event;  /*< server also need libevent */
    struct list_head    list;
}dist_server_t;

typedef struct _listreq{
    int              lq_sock;
    dist_server_t   *lq_server; /**< target socket which handling this request */
    unsigned short   lq_rxlen;
    char             lq_rx[1024];
    unsigned short   lq_txlen;
    char             lq_tx[1024];
    struct event     lq_event;
    int              lq_event_flag;
    struct list_head list;
}listreq_t;

/* need use this as a full parameter passed in background thread */
typedef struct _full_param{
    listreq_t        *req;
    LIBEVENT_THREAD  *thd;
}full_param_t;

extern LIBEVENT_THREAD *g_thread_info;

/* defined in main.c */
extern struct event_base *main_base;
extern pthread_mutex_t  g_list_mutex;
extern struct list_head g_list_hdr;
extern struct list_head g_free_list;

extern int g_totally_servers;
extern struct server_info *g_server;

extern pipe_producer_t *g_prod;
extern pipe_consumer_t *g_cons;

extern listreq_t *new_reqlist();
extern void close_reqlist(listreq_t *r);
extern listreq_t *pop_from_reqlist(struct list_head *h, pthread_mutex_t *m);

extern void list_add_tail(struct list_head *n, struct list_head *h);
extern void list_push_queue_safe(struct list_head *h, struct list_head *item, pthread_mutex_t *m);
extern void spawn_worker_thread(int nr);


extern int get_server_from_config(const char *fn);

#define list_is_empty(list)   ((list) == (list)->next)

#define list_for_each(pos, head) \
            for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_entry(ptr, type, member) \
            ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define LIST_HEAD_INIT(name)  { &(name), &(name) }

#define LIST_HEAD(name) \
            struct list_head name = LIST_HEAD_INIT(name)

#endif
