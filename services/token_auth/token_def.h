#ifndef _TOKE_DEF_H
#define _TOKE_DEF_H

#include <my_global.h>
#include <mysql.h>

/* if within 7-day, we should update DB's expiration field */
#define DELTA_OF_TOKEN_UPDATE (7*24*3600)

/* if the same user do token auth within 0.5 hour,
   we will NOT update the LAST_LOGIN DB item, to avoid
   DB overhead */
#define FREQUENT_VISIT_TIME  (30*60)

struct token_string_info{
    char *tsi_userid;
    int   tsi_login;
    char *tsi_appid;
    char *tsi_rsastr;  /**< see @cipher_token_string_info */
};


struct cipher_token_string_info{
    char *csi_userid;
    char *csi_expire;
    char *csi_appid;
    char *csi_authid;     /**< below 3 members are all reserved currently */
    char *csi_authlevel;
    char *csi_content;
};

extern MYSQL *GET_MYSQL(struct sql_server_info *server);
extern void   FREE_MYSQL(MYSQL *m);
extern int fetch_expire_from_db(MYSQL *ms, const char *cexpir_str, char *lastlogin_str, int lastlogin_size, char *expir_str, int expir_size);
extern int update_token_time_to_db(MYSQL *ms, struct token_string_info *s, struct cipher_token_string_info *c, int lastlogin_sec, int expiration_sec);
extern int get_token_from_db(MYSQL *ms, char *uid, char *token_in_db, int token_size);
extern int ping_sql(MYSQL *ms);

#endif
