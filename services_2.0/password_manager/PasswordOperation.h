#ifndef _CDS_PASSWD_OPR_H
#define _CDS_PASSWD_OPR_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "PasswordManager.pb.h"
#include "Operation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class PasswordOperation : public com::caredear::Operation{

        static char m_md5[36];
        static int cb_get_md5_in_db(MYSQL_RES *mresult, void *p_extra);

        int modify_existed_password(PasswordManagerRequest *reqobj);

        int validation_user_password(PasswordManagerRequest *reqobj);

        int write_user_password_to_db(PasswordManagerRequest *reqobj);
    public:
        // constructors...
        PasswordOperation() {
        }

        PasswordOperation(Config *c) : com::caredear::Operation(c) {
        }
        // end of constructors...

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
