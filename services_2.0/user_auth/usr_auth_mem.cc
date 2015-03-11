/**
 * memcached for User Auth
 *
 * layout:
 *
 *     Key         |        Value
 * ----------------------------------------------
 *   uuid(token)   | CID   sysid    lastlogin
 * ----------------------------------------------
 *
 */
#include "usr_auth_db.h"

/* Store memcached's CAS value, as we need update
 * memcached with CAS */
//map<string,uint64_t> glb_MemCas;

/**
 * Though this is not related with memcached, we put util here
 *
 */
int split_val_into_fields(char *value, struct auth_data_wrapper *w)
{
    char *c, *s, *l;
    char *saveptr;

    if(!value)
    {
        return -1;
    }

    if((c = strtok_r((char *)value, " ", &saveptr)) != NULL)
    {
        w->adw_cid = atol(c);
        if((s = strtok_r(NULL, " ", &saveptr)) != NULL)
        {
            w->adw_sysid = atoi(s);
            if((l = strtok_r(NULL, " " , &saveptr)) != NULL)
            {
                w->adw_lastlogin = atol(l);
            }
        }
    }

    return 0;
}

char *get_token_info_from_mem(memcached_st *memc, const char *key, size_t *p_vallen)
{
    char *val = NULL;
    memcached_return_t rc;

    val = memcached_get_by_key(memc,
            NULL,
            0,
            key,
            strlen(key),
            p_vallen,
            0, &rc);
    if(val != NULL)
    {
        if(rc != MEMCACHED_SUCCESS)
        {
            // for failure case even got non-NULL val
            ERR("though mem val != NULL, rc still failed:%d\n", rc);
            free(val);
            val = NULL;
        }
    }

    return val;
}


/**
 * Use CAS to set key in memcached
 *
 */
memcached_return_t set_token_info_to_mem(memcached_st *memc, const char *key, struct auth_data_wrapper *w)
{
    memcached_return_t rc = MEMCACHED_SUCCESS;
    uint64_t cas_value;
    size_t   len;
    char *val = NULL;

    val = memcached_get_by_key(memc,
            NULL, 0,
            key, strlen(key),
            &len, 0, &rc);
    if(val != NULL)
    {
        if(rc == MEMCACHED_SUCCESS)
        {
            cas_value = (memc->result).item_cas;
            LOG("%s -> CAS %ld\n", key, cas_value);

            char value[128];
            time_t current;
            time(&current);
            sprintf(value, "%ld %d %ld",
                    w->adw_cid, w->adw_sysid, current);

            rc = memcached_cas(memc,
                    key, strlen(key),
                    value, strlen(value),
                    0,  // expiration
                    0, // flag
                    cas_value);
        }
        free(val);
    }
    return rc;
}
