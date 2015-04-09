#ifndef _CDS_SIP_OPERATION_H
#define _CDS_SIP_OPERATION_H

#include "SipAccount.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "Operation.h"
#include "SipConfig.h"

using namespace google::protobuf::io;

class SipOperation : public com::caredear::Operation
{
    struct sips_info{
        uint64_t s_cid;
        char s_token[512];
    };

    static int cb_get_cid(MYSQL_RES *mresult, void *p_extra);
    static int cb_get_token(MYSQL_RES *mresult, void *p_extra);

public:
    SipOperation() {
    }

    /**
     * Helper constructor
     */
    SipOperation(Config *c) : com::caredear::Operation(c) {
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
