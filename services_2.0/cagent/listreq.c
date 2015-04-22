#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "listreq.h"

void _list_add(struct list_head *n, struct list_head *prev, struct list_head *next)
{
    next->prev = n;
    n->next = next;
    n->prev = prev;
    prev->next = n;
}

void list_add_tail(struct list_head *n, struct list_head *h)
{
    _list_add(n, h, h->next);
}

void list_push_queue_safe(struct list_head *h, struct list_head *item, pthread_mutex_t *m)
{
    pthread_mutex_lock(m);
    list_add_tail(item, h);
    pthread_mutex_unlock(m);
}

void _list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

void list_del(struct list_head *entry)
{
    _list_del(entry->prev, entry->next);
}

listreq_t *pop_from_reqlist(struct list_head *h, pthread_mutex_t *m)
{
    listreq_t *r = NULL;
    struct list_head *pos;

    pthread_mutex_lock(m);
    if(!list_is_empty(h)) {
        pos = h->next;
        r = list_entry(pos, listreq_t, list);
    }
    pthread_mutex_unlock(m);

    return r;
}

/**
 * First try get already-malloced from free list, if failed to get,
 * then allocate a new node.
 *
 */
listreq_t *new_reqlist()
{
    listreq_t *r = NULL;
    struct list_head *pos;

    if(list_is_empty(&g_free_list))
    {
        // new from scratch
        r = (listreq_t *)malloc(sizeof(listreq_t));
        if(r)
        {
            r->lq_server = NULL;
            r->lq_rxlen = r->lq_txlen = 0;
        }
    }
    else
    {
        // pop from the free list
        pos = g_free_list.next;
        r = list_entry(pos, listreq_t, list);
        list_del(pos);
    }

    return r;
}

/**
 * TODO, aim to obsolete main.c:free_reequest()
 *
 */
void close_reqlist(listreq_t *r)
{
    if(r->lq_sock)
    {
        event_del(&(r->lq_event));
        close(r->lq_sock);
    }

    // don't free this pointer, put it to free list
    list_add_tail(&(r->list), &g_free_list);
}

