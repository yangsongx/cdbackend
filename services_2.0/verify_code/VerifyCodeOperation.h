/**
 *
 *\history
 * [2015-04-14] remove the uuid.h header including
 */
#ifndef _VCS_LOGINOPER_H
#define _VCS_LOGINOPER_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "VerifyCode.pb.h"
#include "VerifyCodeConfig.h"

#include "Operation.h"


using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class VerifyCodeOperation : public com::caredear::Operation {

    static int cb_get_cid(MYSQL_RES *mresult, void *p_extra);
    static int cb_check_passwd(MYSQL_RES *mresult, void *p_extra);

    int gen_verifycode(VerifyRequest *reqobj, VerifyResponse *respobj);
    int check_password(VerifyRequest *reqobj, VerifyResponse *respobj);
    int record_code_to_db(VerifyRequest *reqobj, const char *code);

    int add_code_to_db(VerifyType type, const char *value, const char *code);

public:
    VerifyCodeOperation() {
    }

    VerifyCodeOperation(Config *c) : Operation(c){
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
