/**
 *
 *\history
 * [2015-04-13] Need add SIPs DB for mobile phone case
 */
#ifndef _UUP_OPER_H
#define _UUP_OPER_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "UpdateProfile.pb.h"
#include "UpdateProfileConfig.h"
#include "Operation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class UpdateProfileOperation : public com::caredear::Operation {

    static int cb_get_repeat_id(MYSQL_RES *mresult, void *p_extra);
    static int cb_check_code(MYSQL_RES *mresult, void *p_extra);
    static int cb_check_sipaccount(MYSQL_RES *mresult, void *p_extra);

    int add_user_mobile_phone(UpdateRequest *reqobj);
    int add_mobile_to_sips_db(UpdateRequest *reqobj);
    int add_user_password(UpdateRequest *reqobj);
    int add_user_name(UpdateRequest *reqobj);
    int pass_code_verify(UpdateRequest *reqobj);
    int unique_record(UpdateRequest *reqobj);
    int add_user_email(UpdateRequest *reqobj);

    void strip_repeat_mobile_number(UpdateRequest *reqobj);
public:
    UpdateProfileOperation(){
    }

    UpdateProfileOperation(Config *c) : Operation(c) {
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
