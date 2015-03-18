#ifndef CDS_ATTR_OPR_H
#define CDS_ATTR_OPR_H

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "AttributeModify.pb.h"
#include "Operation.h"

using namespace std;
using namespace google::protobuf::io;

class AttributeOperation : public com::caredear::Operation {

    static int  m_attrRecord; // 1 - there're specified attribute item in DB, otherwise is 0

    static int cb_check_attr_existence(MYSQL_RES *p_result);

    int user_attribute_existed(uint64_t cid);
    int insert_usr_attr_to_db(AttributeModifyRequest *attrobj);
    int update_usr_attr_to_db(AttributeModifyRequest *attrobj);

public:
    AttributeOperation() {
    }

    AttributeOperation(Config *c) : com::caredear::Operation(c) {
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
