/**
 * main entry for User Registration Service(URS)
 *
 */
#ifdef CHECK_MEM_LEAK
#include <mcheck.h>
#endif

#include "cds_public.h"

#include <libmemcached/memcached.hpp>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "UserRegister.pb.h"

using namespace std;

/**
 * Handler entry for a user registration request
 *
 */
int register_handler(int size, void *req, int *len_resp, void *resp)
{
    if(size >= DATA_BUFFER_SIZE)
    {
    }

    return 0;
}

int ping_reg_handler(int size, void *req, int *len_resp, void *resp)
{
    return 0;
}

/**
 * World entry point.
 *
 */
int main(int argc, char **argv)
{
    struct addition_config cfg;

#ifdef CHECK_MEM_LEAK
    setenv("MALLOC_TRACE", "/tmp/urs.memleak", 1);
    mtrace();
#endif

    cfg.ac_cfgfile = NULL;
    cfg.ac_handler = register_handler;
    cfg.ping_handler = ping_reg_handler;
	cfg.ac_lentype = LEN_TYPE_BIN; /* we use binary leading type */

#if 1 // test code for debug
    memcache::Memcache memc;
    bool res = memc.addServer("127.0.0.1", 11211);
    if(res == true)
    {
        printf("memcached conn [OK]\n");

        vector<char> value;
        if(memc.get("woep9ge0s5doghu3rbsyxchsdjvzwc9cb54nxzbh4h9q8408n6yhwbz5vjsd5x4b", value))
        {
            printf("get key [OK] :%s\n", &value[0]);
        }
    }
    else
    {
        printf("memcached conn [false]\n");
    }
#endif

    cds_init(&cfg, argc, argv);

    return 0;
}
