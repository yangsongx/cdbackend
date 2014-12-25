#include <stdio.h>
#include <string.h>
#include <mcheck.h>
#include <libmemcached/memcached.hpp>

using namespace std;
//using namespace memcache;

int main(int argc, char **argv)
{
#if 1
    const char *cfg = "--SERVER=192.168.1.108:12000";

    setenv("MALLOC_TRACE", "/tmp/mem.chk", 1);
    mtrace();

    memcached_st *memc = memcached(cfg, strlen(cfg));
    if(memc != NULL)
    {
        printf("[OK] for get server...\n");
        const char *key="123";
        const char *value="yangsongxiang";

        memcached_return_t rc = memcached_set(memc, key, strlen(key),
                value, strlen(value), 0, 0);
        printf("the set return code = %d\n", rc);

        char *kk;
        size_t val_len = 0;
        rc = MEMCACHED_READ_FAILURE;
        kk = memcached_get_by_key(memc, NULL, 0, "key11", strlen("key11"), &val_len, 0, &rc);
        if(kk != NULL)
        {
            printf("\nGET [OK]\n");
            printf("the val len = %ld, rc=%d\n", val_len, rc);
            printf("the val : %s\n", kk);

            printf("try free %#x\n", (long)kk);
            free(kk);
        }
        memcached_free(memc);
    }

#else
    Memcache memch("192.168.1.23:12306");

    map< string, map<string, string> > my_stats;
    memch.getStats(my_stats);
    map< string, map<string, string> >::iterator it=
        my_stats.begin();
    while(it != my_stats.end())
    {
        printf("working with server:%s\n", (*it).first.c_str());

        map<string, string> serv_stats= (*it).second;
        map<string, string>::iterator iter= serv_stats.begin();
        while (iter != serv_stats.end())
        {
            printf("%s:%s\n", (*iter).first.c_str(), (*iter).second.c_str());
            ++iter;
        }
        ++it;
    }
    bool ret = memch.set("13815882359", "tianfeng", strlen("tianfeng"), 0, 0);
    printf("the result of set:%s\n", ret == true ? "true" : "false");
#endif            
    return 0;
}
