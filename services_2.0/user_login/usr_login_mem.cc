#include "usr_login_db.h"

memcached_return_t set_session_info_to_mem(memcached_st *memc, LoginRequest *reqobj, struct user_session *u)
{
    memcached_return_t rc;
    char val[128];
    time_t current;

    time(&current);

    /* FIXME login sys id probably different with auth sys id ? */
    sprintf(val, "%ld %s %ld",
            u->us_cid, reqobj->login_sysid().c_str(), current);

    rc = memcached_set(memc,
            u->us_token, strlen(u->us_token),
            val, strlen(val),
            0,
            0);

    return rc;
}
