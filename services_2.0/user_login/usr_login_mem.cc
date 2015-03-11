#include "usr_login_db.h"
#if 0
int split_val_into_fields(char *value, struct user_session *u)
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
#endif
memcached_return_t set_session_info_to_mem(memcached_st *memc, LoginRequest *reqobj, struct user_session *u)
{
    memcached_return_t rc;
    char val[128];
    time_t current;

    time(&current);

    /* FIXME login sys id probably different with auth sys id ? */
    sprintf(val, "%ld %s %ld",
            u->us_cid, reqobj->login_session().c_str(), current);

    rc = memcached_set(memc,
            u->us_token, strlen(u->us_token),
            val, strlen(val),
            0,
            0);

    return rc;
}

/**
 * Delete an obsoleted memcache item
 *
 */
memcached_return_t rm_session_info_from_mem(memcached_st *memc, const char *key)
{
    memcached_return_t rc;

    rc = memcached_delete(memc, key, strlen(key), 0/* expiration */);

    return rc;
}
