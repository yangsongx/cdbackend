/**
 * Wrapper for db access, we use memcached + MySQL
 */
#include <libmemcached/memcached.h>

#include "cds_public.h"
#include "token_def.h"

extern char *get_by_key(memcached_st *memc, const char *key, size_t *vallen);
extern int set_by_key(memcached_st *memc, const char *key, const char *value, time_t val_expir);
extern int delete_by_key(memcached_st *memc, const char *key);


/**
 *
 * @key : memcache key, currently is token string.
 *
 */
int map_into_mem(memcached_st *memc, const char *key, struct token_data_wrapper *base)
{
    char value[128];

    snprintf(value, sizeof(value),
            "%s %d %ld %ld",
            base->tdw_userid, base->tdw_deviceid, base->tdw_lastlogin, base->tdw_expire);

    set_by_key(memc, key, value, MEMKEY_VALID);

    return 0;
}



/**
 * update latest token info data into mem/db
 *
 */
int refresh_token_info_data(memcached_st *memc, MYSQL *ms, struct token_data_wrapper *tokeninfo)
{
    int ret = 0;

    // step 1 - mem
    map_into_mem(memc, tokeninfo->tdw_token, tokeninfo);

    // step 2 - db
    KPI("==>Begin WRITE DB for TAUTH\n");
    ret = push_tokeninfo_to_db(ms, tokeninfo);
    KPI("==>Finished WRITE DB for TAUTH(code=%d)\n", ret);

    /* FIXME - still return 0 here, even sth wrong when updating DB */
    return 0;
}

/**
 * this will do compare with token data such as time delta, and other more stuff
 *
 * @info : the incoming user's request data(uid, token, log in time)
 * @base : the DB/mem token data, as a compare base line.
 *
 * return CDS_XXX code
 */
int process_token_data(memcached_st *memc, MYSQL *ms, struct token_string_info *info, struct token_data_wrapper *base)
{
    int ret = CDS_OK;
    int commit_level = 0; // > 1 means need update mem/DB

    if(strcmp(info->tsi_userid, base->tdw_userid))
    {
       // Log this unmatch info to help debug
       ERR("%s->%s | while in db/mem it is:%s, addr:%p\n",
               info->tsi_rsastr, info->tsi_userid,
               base->tdw_userid, base);

       return CDS_ERR_UMATCH_USER_INFO;
    }

    if(info->tsi_login > base->tdw_expire)
    {
        ERR("this is expired login case, need re-register!\n");
        ret = CDS_ERR_USER_TOKEN_EXPIRED;
        delete_by_key(memc, info->tsi_rsastr);
    }
    else
    {
        // all is ok, get the token from request, we need do this
        // to help process DB update.(as token is mem KEY)
        base->tdw_token = info->tsi_rsastr;

        if((base->tdw_expire - info->tsi_login) < DELTA_OF_TOKEN_UPDATE)
        {
            // we need enlarge the expiration(enlarge 1 month later)
            base->tdw_expire += (4*DELTA_OF_TOKEN_UPDATE);
            commit_level ++;
        }
    
        if((info->tsi_login - base->tdw_lastlogin) > FREQUENT_VISIT_TIME)
        {
            base->tdw_lastlogin = info->tsi_login;
            commit_level ++;
        }

        if(commit_level > 0)
        {
            refresh_token_info_data(memc, ms, base);
        }
        else
        {
            LOG("Uer login within short time, avoid touch mem/DB!\n");
        }
    }


    return ret;
}


/**
 * @strings: value stored in memcached, in following format:
 *           -------------------------------------------
 *           |uid | device id | lastlogin | expiration |
 *           -------------------------------------------
 * @p_info: Store splited data sections
 *
 */
void extract_memcach_value(const char *strings, struct token_data_wrapper *p_info)
{
    char *t, *d, *l, *e; // token + last login + expire
    char *saveptr;

    if(strings == NULL)
    {
        ERR("a NULL memcach value, ignore it\n");
        return;
    }

    /*
     * Memcached layout are:
     * ============================================================
     * |  key(token)  |  user_id  device_id  lastlogin  expiration
     * ============================================================
     */
    if((t = strtok_r((char *)strings, " ", &saveptr)) != NULL)
    {
        p_info->tdw_userid = t;
        if((d = strtok_r(NULL, " ", &saveptr)) != NULL)
        {
            p_info->tdw_deviceid = atol(d);
            if((l = strtok_r(NULL, " " , &saveptr)) != NULL)
            {
                p_info->tdw_lastlogin = atol(l);
                if((e = strtok_r(NULL, " ", &saveptr)))
                {
                    p_info->tdw_expire = atol(e);
                }
            }
        }
    }
}

/**
 * A util wrapper for token info from DB, instead of mem
 *
 * return a CDS_XXX constant
 */
int tokeninfo_operation_on_db(memcached_st *memc, MYSQL *ms, struct token_string_info *info)
{
   int  ret = CDS_OK;
   char userid_line[128];  // user's mobile number
   char devid_line[32];    // user's device id
   char lastlogin_line[32]; /* string format as 'YYYY-MM-DD HH:MM:SS' */
   char expire_dead_line[32]; /* string format as 'YYYY-MM-DD HH:MM:SS' */
   struct token_data_wrapper datasection;
   memset(&datasection, 0x00, sizeof(datasection));

   ret = fetch_tokeninfo_from_db(ms,
                              info,
                              userid_line, sizeof(userid_line),
                              devid_line, sizeof(devid_line),
                              lastlogin_line, sizeof(lastlogin_line),
                              expire_dead_line, sizeof(expire_dead_line));
   if(ret == CDS_OK)
   {
       struct tm lst;
       struct tm epr;
       struct tm *ptr;

       memset(&lst, 0x00, sizeof(lst));
       memset(&epr, 0x00, sizeof(epr));


       if(strptime(lastlogin_line, "%Y-%m-%d %H:%M:%S", &lst) == NULL)
       {
           ERR("failed call strptime on lastlogin, data:%s, error:%d\n",
               lastlogin_line, errno);
           if(!strncmp(lastlogin_line, "0000-00-00", 10))
           {
               /* FIXME for the incorrect date time,
                * I think need silient set it to a
                * correct value, and don't treat it
                * as an authentication failure.
                */
               time_t cur_time;
               time(&cur_time);
               cur_time -= (24*60*60); // make it a little earlier
               ptr = localtime(&cur_time);
               memcpy(&lst, ptr, sizeof(lst));
#if 1 // just debug
                    LOG("DEBUG  == fake a last login :%s", asctime(&lst));
#endif
       }
       else
       {
           return CDS_ERR_LIBC_FAILURE;
       }
   }
   if(strptime(expire_dead_line, "%Y-%m-%d %H:%M:%S", &epr) == NULL)
   {
       ERR("failed call strptime on expire:%d\n", errno);
       if(!strncmp(lastlogin_line, "0000-00-00", 10))
       {
           /* FIXME for the incorrect date time,
            * I think need silient set it to a
            * correct value, and don't treat it
            * as an authentication failure.
            */
           time_t cur_time;
           time(&cur_time);
           cur_time += (24*60*60); // expiration should be later than current time
           ptr = localtime(&cur_time);
           memcpy(&epr, ptr, sizeof(epr));
#if 1 // just debug
                    LOG("DEBUG  == fake a expiration :%s", asctime(&epr));
#endif
           }
           else
           {
               return CDS_ERR_LIBC_FAILURE;
           }
       }


       // next, try compare if user's request is expired
       time_t last = mktime(&lst);
       if(last == (time_t) -1)
       {
           ERR("failed call mktime for last login:%d\n", errno);
           return CDS_ERR_LIBC_FAILURE;
       }

       time_t expire = mktime(&epr);
       if(expire == -1)
       {
           ERR("failed call mktime for expire:%d\n", errno);
           return CDS_ERR_LIBC_FAILURE;
       }


       datasection.tdw_userid = userid_line;
       datasection.tdw_deviceid = atoi(devid_line);
       datasection.tdw_lastlogin = last;
       datasection.tdw_expire = expire;
       // if we come here, time_t value is got.
       map_into_mem(memc, info->tsi_rsastr, &datasection);

       ret = process_token_data(memc, ms, info, &datasection);
   }
   else
   {
       // FIXME for MySQL disconnected case, upper caller(do_token_authentication())
       // will consider to re-connected it or not.
   }

   return ret;
}


/**
 * This wrapper will try get from memcached first, and
 * will fall back to MySQL if memcached not found the
 * key entry.
 *
 * We only interested in the specified user's:
 * - token string
 * - Last login time
 * - Expiration time
 *
 * @memc: memcached address
 * @result: store the user's data(token+last login+expiration) TODO - probably obsolet this one.
 *
 * return CDS_XXXX constant code
 */
int get_token_info(memcached_st *memc, MYSQL *ms, struct token_string_info *info, struct token_data_wrapper *result)
{
    int ret = CDS_OK;
    char *uid = info->tsi_userid;
    char *mem_val = NULL;
    size_t memval_len = 0;
    struct token_data_wrapper datasection;
    memset(&datasection, 0x00, sizeof(datasection));

    /*
     * First , try get data from mem, based on the token(we take token as mem key)
     */
    if(!memc || (mem_val = get_by_key(memc, info->tsi_rsastr, &memval_len)) == NULL)
    {
        // no memcached, fall back to MySQL
        LOG("got NULL in mem, go into MySQL...\n");
        ret = tokeninfo_operation_on_db(memc, ms, info);
    }
    else
    {
        // got a valid value in mem, split the mem sections..
        LOG("memval key:%s,val:%s,base addr:%p\n",
                info->tsi_rsastr, mem_val == NULL ? "null" : mem_val,
                &datasection);
        extract_memcach_value(mem_val, &datasection);

        ret = process_token_data(memc, ms, info, &datasection);

        // 2015-2-25 code change
        //if(ret != CDS_OK)
        {
            if(ret == CDS_ERR_UMATCH_USER_INFO)
            {
                // Note for 2015-2-25, sometime token->user id mapping is incorrect,
                // which cause above api return un-match error, we need take value from DB again
                //
                // Keep in mind this is workaround as the root cause is token->user id mapping
                // failed between User-Req and Memcache, I guess this is caused by race condition!

                INFO("As meet this unmatch case, we try to fetch from raw DB...\n");
                ret = tokeninfo_operation_on_db(memc, ms, info);
            }
        }
        // 2015-2-25 code end

        // need free this one!
        free(mem_val);
    }

    return ret;
}


