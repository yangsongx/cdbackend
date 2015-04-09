#ifndef _ULS_LOGINOPER_H
#define _ULS_LOGINOPER_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "UserLogin.pb.h"
#include "UserLoginConfig.h"
#include "Operation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class LoginOperation : public com::caredear::Operation {

    struct user_session {
        unsigned long  us_cid;       /**< caredear ID(unique for all users) */
        const char    *us_sessionid; /**< Session id description */
        const char    *us_token;     /**< This session's token string */
    };

    static int cb_get_shenzhen_flag(MYSQL_RES *mresult, void *p_extra);

    static int cb_check_name(MYSQL_RES *mresult, void *p_extra);

    static int cb_check_accode(MYSQL_RES *mresult, void *p_extra);
    static int cb_wr_db_session(MYSQL_RES *mrsult, void *p_extra);

    int update_usercenter_session(LoginRequest *reqobj, struct user_session *u);
    int set_session_info_to_db(struct user_session *u, char *old);
    int insert_new_session_in_db(struct user_session *u);
    int overwrite_existed_session_in_db(struct user_session *u);

    memcached_return_t set_session_info_to_mem(LoginRequest *reqobj, struct user_session *u);

    int delete_usercenter_session(LoginRequest *reqobj);
    int process_user_and_credential(LoginRequest *reqobj, LoginResponse *respobj);

    int add_device_type(LoginRequest *reqobj);
    int get_shenzhen_flag_from_db(uint64_t cid);

    int match_user_credential_in_db(LoginRequest *reqobj, unsigned long *p_cid);
    int compare_user_password_wth_cid(LoginRequest *reqobj, const char *targetdata, uint64_t cid);
    int compare_user_smscode_wth_cid(LoginRequest *reqobj, const char *code, uint64_t cid);

public:
    LoginOperation() {
    }

    LoginOperation(Config *c) : Operation(c) {
    }

    virtual int handling_request(::google::protobuf::Message *reg_req,
            ::google::protobuf::Message *reg_resp,
            int *len_resp,
            void *resp);

    virtual int compose_result(int code, const char *errmsg,
            ::google::protobuf::Message *obj,
            int *p_resplen,
            void *p_respdata);

};

#endif
