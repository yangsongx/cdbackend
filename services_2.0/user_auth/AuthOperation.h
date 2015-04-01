#ifndef _UAS_AUTHOPR_H
#define _UAS_AUTHOPR_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "UserAuth.pb.h"

#include "UserAuthConfig.h"
#include "Operation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

// don't update lastlogin within 0.5 hours
#define MAX_FREQUENT_VISIT  (30*60)

/**
 * Wrapper for mem val's splitted fields
 */
struct auth_data_wrapper{
    unsigned long adw_cid;
    char          adw_session[64];  // act like device id
    time_t        adw_lastlogin;
};

class AuthOperation : public com::caredear::Operation {

    static struct auth_data_wrapper  m_AuthWrapper;
    static int cb_token_info_query(MYSQL_RES *p_result);

    bool is_xmpp_auth(AuthRequest *reqobj);

    int check_token(AuthRequest *reqobj, struct auth_data_wrapper *w);
    bool is_allowed_access(AuthRequest *reqobj);
    int auth_token_in_session(AuthRequest *reqobj, AuthResponse *respobj, struct auth_data_wrapper *w);
    int update_session_lastoperatetime(AuthRequest *reqobj, struct auth_data_wrapper *w);

    int set_token_info_to_db(AuthRequest *reqobj);
    int get_token_info_from_db(AuthRequest *reqobj, AuthResponse *respobj, struct auth_data_wrapper *w);

protected:
    int split_val_into_fields(char *value, struct auth_data_wrapper *w);

public:
    AuthOperation() {
    }

    /**
     * Helper constructor
     */
    AuthOperation(Config *c) : com::caredear::Operation(c) {
    }

    virtual int handling_request(::google::protobuf::Message *reqobj,
            ::google::protobuf::Message *respobj,
            int *len_resp,
            void *resp);

    virtual int compose_result(int code, const char *errmsg,
            ::google::protobuf::Message *obj,
            int *p_resplen,
            void *p_respdata);
};

#endif
