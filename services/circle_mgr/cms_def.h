/**
 * Common Definition for Circle Management Service(CMS)
 */
#ifndef _CMS_DEF_H
#define _CMS_DEF_H

#include <my_global.h>
#include <mysql.h>


/**
 * A circle SYNC req format
 */
struct circle_sync_req{
    char     *sreq_token;    /**< used for authentication */
    char     *sreq_uid;      /**< User ID */
    int       sreq_from;     /**< from when(time), for us to SYNC */
};

/**
 * The Circle SYNC data send back to caller.
 *
 */
struct circle_sync_data{
    int       sync_result;     /**< See the CDS_ERR_XXX for details, following members only usable when result code is CDS_OK */
    int       sync_len;        /**< length of data, excluding leading 2 ints */
    char      sync_uid[12];    /**< User ID(phone number usually), indicate who wants to SYNC circle data*/
    char      sync_payload[0]; /**< a vary-len palyload divided by '\0', dependency on @sync_count */
};

extern struct sql_server_info sql_cfg;

extern int INIT_DB_MUTEX();
extern int CLEAN_DB_MUTEX();

extern MYSQL *GET_CMSSQL(struct sql_server_info *server);
extern void FREE_CMSSQL(MYSQL *m);


extern int fetch_sync_data(MYSQL *ms, struct circle_sync_req *req, struct circle_sync_data *response);
extern int cms_auth(char *str, struct circle_sync_req *sync_req);

#endif
