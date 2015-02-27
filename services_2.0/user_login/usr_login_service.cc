/**
 * User Login Service (ULS)
 *
 */
#ifdef CHECK_MEM_LEAK
#include <mcheck.h>
#endif

#include "cds_public.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

int register_handler(int size, void *req, int *len_resp, void *resp)
{
    return 0;
}

int ping_reg_handler(int size, void *req, int *len_resp, void *resp)
{
    return 0;
}

int main(int argc, char **argv)
{
    struct addition_config cfg;

#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/usrlogin.memleak", 1);
    mtrace();
#endif

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = register_handler;
    cfg.ping_handler = ping_reg_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */

    cds_init(&cfg, argc, argv);

    return 0;
}
