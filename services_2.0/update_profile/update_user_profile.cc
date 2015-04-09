
#ifdef CHECK_MEM_LEAK
#include <mcheck.h>
#endif

#include <errno.h>
#include <stdio.h>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "cds_public.h"

#include "UpdateProfile.pb.h"
#include "UpdateProfileConfig.h"
#include "UpdateProfileOperation.h"
#include "data_access.h"


using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

/* TODO below mutex SHOULD be obsoleted soon... */
pthread_mutex_t  uup_mutex;
UpdateProfileConfig  g_info;


int uup_handler(int size, void *req, int *len_resp, void *resp)
{
    bool ok = false;
    int ret = CDS_OK;
    UpdateProfileOperation opr;
    UpdateRequest  reqobj;
    UpdateResponse respobj;

    opr.set_conf(&g_info);

    if(size >= DATA_BUFFER_SIZE)
    {
        ERR("Too much long data, ignore it\n");
        if(opr.compose_result(CDS_ERR_REQ_TOOLONG, "too much long",
                    &respobj, len_resp, resp) != 0)
        {
            ERR("***Failed Serialize the too-long error data\n");
        }

        return CDS_ERR_REQ_TOOLONG;
    }


    ArrayInputStream in(req, size);
    CodedInputStream is(&in);

    ok = reqobj.ParseFromCodedStream(&is);
    if(ok)
    {
    	if (0==check_user_vcode(g_info.m_Sql,&reqobj,&respobj, &g_info))
    	{
            if(is_dup_record(g_info.m_Sql,&reqobj,&respobj, &g_info)!=0)
            {
                if(opr.compose_result(19,NULL,&respobj, len_resp, resp))
                {
                    ERR("** failed seriliaze for the error case\n");
                }
            }
            else
            {
                int DB_returns = record_user_login_info(g_info.m_Sql,&reqobj,&respobj, &g_info);
    		    if(0==DB_returns)
    		    {
    			      if(opr.compose_result(0,NULL,&respobj, len_resp, resp))
    		          {
                            ERR("** failed seriliaze for the error case\n");
                       }
    		    }
    		    else 
    		    {
    			     if(opr.compose_result(DB_returns,NULL,&respobj, len_resp, resp))
    		         {
                         ERR("** failed seriliaze for the error case\n");
                     }
    		    }
            }
    	}
    	else
    	{
    		if(opr.compose_result(16,NULL,&respobj, len_resp, resp))
    		{
                ERR("** failed seriliaze for the error case\n");
            }
    	}

    }
    else
    {
        ERR("***Failed pare reqobj in protobuf!\n");
        if(opr.compose_result(CDS_GENERIC_ERROR, "failed parse in protobuf",
                &respobj, len_resp, resp) != 0)
        {
            ERR("** failed seriliaze for the error case\n");
        }
    }

    return ret;
}

int uup_ping_handler(int size, void *req, int *len_resp, void *resp)
{
    return 0;
}


int main(int argc, char **argv)
{
    struct addition_config cfg;

#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/uls.memleak", 1);
    mtrace();
#endif

    if(g_info.parse_cfg("/etc/cds_cfg.xml") != 0)
    {
        ERR("*** Warning Failed init the whole service!\n");
    }

    if(pthread_mutex_init(&uup_mutex, NULL) != 0)
    {
       ERR("*** Warning, failed create mutex IPC objs:%d\n", errno);
    }

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = uup_handler;
    cfg.ping_handler = uup_ping_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */

    cds_init(&cfg, argc, argv);

    return 0;
}
