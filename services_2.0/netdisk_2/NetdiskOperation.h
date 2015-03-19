#ifndef _NDS_OPR_H
#define _NDS_OPR_H

#include <qiniu/base.h>
#include <qiniu/io.h>
#include <qiniu/rs.h>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "NetdiskMessage.pb.h"
#include "Operation.h"

using namespace google::protobuf::io;

// the file type mapped to DB
enum FILE_TYPE {
    FT_IMAGE = 0,
    FT_MUSIC = 1,
    FT_VIDEO,
    FT_CONTACTS,    /**< backup contacts data file */
    FT_SMS,         /**< backup short SMS data file */
    FT_DOC          /**< others will be considered as documents... */
};

class NetdiskOperation : public com::caredear::Operation {

    Qiniu_Client m_qn;

    int do_completed_qiniu_upload(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata);
    int do_qiniu_upload(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata);

    // /////////////////////////////////////////////////////////////////////////////
    // static variables....
    static int m_cbFlag; // A flag for SQL callback result processing

    static int cb_query_user_entry(MYSQL_RES *p_result);
    static int cb_query_file_md5(MYSQL_RES *p_result);
    static int cb_query_quota(MYSQL_RES *p_result);
    // END OF static variables....
    // /////////////////////////////////////////////////////////////////////////////

protected:
    int mapping_file_type(const char *filename);

    int add_new_user_entry_in_db(NetdiskRequest *p_obj);
    int record_file_info_to_db(NetdiskRequest *p_obj);

    int preprocess_upload_req(NetdiskRequest *p_obj);

    int generate_upload_token(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata);


public:
    NetdiskOperation() {
    }

    NetdiskOperation(Config *c) : com::caredear::Operation(c) {
    }

    virtual int handling_request(::google::protobuf::Message *p_obj,
            ::google::protobuf::Message *p_ndr,
            int *len_resp,
            void *resp);

    virtual int compose_result(int code, const char *errmsg,
            ::google::protobuf::Message *obj,
            int *p_resplen,
            void *p_respdata);
};

#endif
