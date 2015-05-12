#ifndef _NDS_OPR_H
#define _NDS_OPR_H

#include <qiniu/base.h>
#include <qiniu/io.h>
#include <qiniu/rs.h>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include "NetdiskMessage.pb.h"
#include "Operation.h"
#include <list>

using namespace std;
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

struct isdelete_info{
    int   ii_id;  /**< ID in db */
    int   ii_del; /**< ISDELETE in db */
};

class NetdiskOperation : public com::caredear::Operation {

    struct file_info{
        char f_md5[34];
        int  f_size;
        int  f_quota;
    };

    Qiniu_Client m_qn;

    int do_completed_qiniu_upload(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata);
    int do_qiniu_upload(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata);
    int do_qiniu_deletion(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata);
    int do_qiniu_downloadurl(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata);

    // /////////////////////////////////////////////////////////////////////////////
    static int cb_query_user_entry(MYSQL_RES *p_result, void *p_extra);
    static int cb_query_file_md5_and_size(MYSQL_RES *p_result, void *p_extra);
    static int cb_query_file_md5(MYSQL_RES *p_result, void *p_extra);
    static int cb_query_netdisk_key(MYSQL_RES *p_result, void *p_extra);
    static int cb_query_quota(MYSQL_RES *p_result, void *p_extra);

    static int cb_query_all_del_entry(MYSQL_RES *p_result, void *p_extra);
    // /////////////////////////////////////////////////////////////////////////////

protected:
    int map_file_to_md5_and_size(NetdiskRequest *p_obj, char *p_md5, int len_md5, int *p_size);
    int mapping_file_type(const char *filename);
    int reduce_used_size(NetdiskRequest *p_obj, int size);

    int gen_download_url(const char *md5, NetdiskResponse *p_resp);
    int get_netdisk_key(NetdiskRequest *p_obj, char *p_result);
    int add_new_user_entry_in_db(NetdiskRequest *p_obj);
    int delete_file_info_from_db(NetdiskRequest *p_obj, const char *md5);
    int record_file_info_to_db(NetdiskRequest *p_obj);

    int preprocess_upload_req(NetdiskRequest *p_obj);

    int generate_upload_token(NetdiskRequest *p_obj, NetdiskResponse *p_ndr, int *p_resplen, void *p_respdata);

    int cleanup_del_flag(int id);
    int unique_isdelete_flag(NetdiskRequest *p_obj, int id);

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
