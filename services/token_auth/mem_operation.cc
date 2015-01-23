/**
 * memcached access util
 *
 */

#include <libmemcached/memcached.h>
#include "cds_public.h"
#include "token_def.h"

/**
 *
 * @key : value's key name
 * @val_len : [output]indicate the value's length
 *
 * return value indicated by @key, be note that this should be
 * free-ed by caller if not used anymore.
 */
char *get_by_key(memcached_st *memc, const char *key, size_t *p_vallen)
{
    memcached_return_t rc;
    char *v = memcached_get_by_key(memc, NULL, 0,
            key, strlen(key), p_vallen,
            0, &rc);
    if(v != NULL)
    {
        if(rc == MEMCACHED_SUCCESS)
        {
            return v;
        }
        else
        {
            free(v);
            v = NULL;
        }
    }
    return NULL;
}

int set_by_key(memcached_st *memc, const char *key, const char *value, time_t val_expir)
{
    memcached_return_t rc;
    rc = memcached_set(memc, key, strlen(key),
            value, strlen(value), val_expir, 0);
    if(rc != MEMCACHED_SUCCESS)
    {
        ERR("Warning, failed SET in mem:%d\n", rc);
    }

    return 0;
}

int delete_by_key(memcached_st *memc, const char *key)
{
    memcached_return_t rc;

    rc = memcached_delete(memc, key, strlen(key), 0);
    if(rc != MEMCACHED_SUCCESS)
    {
        ERR("Warning, failed DELETE in mem:%d\n", rc);
    }

    return 0;
}
