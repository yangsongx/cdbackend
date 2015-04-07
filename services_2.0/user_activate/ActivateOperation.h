#ifndef _ACTS_AUTHOPR_H
#define _ACTS_AUTHOPR_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "UserActivation.pb.h"

#include "ActivationConfig.h"
#include "Operation.h"

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

class ActivateOperation : public com::caredear::Operation{

    static int m_verify_result;
    static uint64_t m_cid;

    static int cb_verify_code(MYSQL_RES *mrestul);
    static int cb_check_code_failure(MYSQL_RES *mrestul);
    static int cb_get_cid(MYSQL_RES *mresult);

    int verify_activation_code(ActivateRequest *reqobj, ActivateResponse *respobj);

    int set_user_status_flag_to_db(ActivateRequest *reqobj, ActivateResponse *respobj);
    int get_user_cid_from_db(ActivateRequest *reqobj, ActivateResponse *respobj);

    int check_further_code_correctness(ActivateRequest *reqobj);

public:

    virtual int handling_request(::google::protobuf::Message *reqobj,
            ::google::protobuf::Message *respobj,
            int *len_resp,
            void *resp);

    virtual int compose_result(int code, const char *errmsg,
            ::google::protobuf::Message *p_obj,
            int *p_resplen,
            void *p_respdata);
};

#endif
