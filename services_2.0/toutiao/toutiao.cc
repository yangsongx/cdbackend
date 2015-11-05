/**
 *
 *
 */
#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

#include <list>
#include <map>
#include <string>

#include <curl/curl.h>

#include <my_global.h>
#include <mysql.h>

#include <libmemcached/memcached.h>

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#define LOG printf

#if 1
#define TARGET_URL  "192.168.1.108:9001/v1/maker/begin/"
#else
#endif

using namespace std;

static int glb_db_connected = 0;

struct user_info{
    char   info_token[64];
    time_t info_time;
    int    info_rand;
    char   info_sig[64];
    time_t info_expired;
};

map<string, struct user_info> glb_list;

char *base64_encode(const char *input, int len, bool with_newline)
{
    BIO *b64 = NULL;
    BIO *bmem = NULL;
    BUF_MEM *bptr = NULL;

    b64 = BIO_new(BIO_f_base64());
    if(!with_newline)
    {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }

    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, len);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    char *buff = (char *)malloc(bptr->length + 1);
    memcpy(buff, bptr->data, bptr->length);
    buff[bptr->length] = '\0';

    return buff;
}


int demo_post_with_header(CURL *curl, char *img)
{
    CURLcode cret = CURLE_OK;
    const char *url = "http://apis.baidu.com/apistore/idlocr/ocr";
    const char *key = "75ff8fa32eeb8de7186527b8acf6dde9";
    char post_data[1921024];
    char header_data[512];
    //CURLOPT_HEADERDATA
    sprintf(header_data, "apikey:%s", key);
    snprintf(post_data, sizeof(post_data),
      "fromdevice=pc&clientip=10.10.10.0&detecttype=LocateRecognize&languagetype=CHN_ENG&imagetype=1&image=%s",
      img);

    LOG("the post data:%s\n", post_data);
    LOG("the header:%s\n", header_data);

    curl_easy_setopt(curl, CURLOPT_HEADERDATA, header_data);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    LOG("trigger to baidu...\n");

    cret = curl_easy_perform(curl);
    LOG("the curl ret = %d\n", cret);

    return 0;
}

int send_ocr_to_baidu(CURL *curl, const char *fn)
{
    struct stat st;

    if(stat(fn, &st) == 0)
    {
        int len = (int)st.st_size;
        char *ptr = (char *)malloc(len);
        if(ptr)
        {
            FILE *fp = fopen(fn, "rb");
            if(fp)
            {
                fread(ptr, len, 1, fp);
                fclose(fp);

                char *ret = base64_encode(ptr, len, false);
                assert(ret != NULL);
                demo_post_with_header(curl, ret);

                free(ret);
            }

            free(ptr);
        }
    }
    else
    {
        LOG("File not existed at all\n");
    }
    return 0;
}

int post_each_gid(const char *token, unsigned long groupid, const char *post_target, CURL *curl)
{
    int ret = -1;
    char post_data[1024];
    CURLcode cret = CURLE_OK;
    time_t cur;

    time(&cur);

    snprintf(post_data, sizeof(post_data),
            "{"
             "\"access_token\":\"%s\","
             "\"actions\":"
                  "[{"
                    "\"type\":\"enter\", \
                    \"timestamp\":%ld, \
                    \"data\" : {\"group_id\":%ld}"
                  "}]"
            "}", token, cur, groupid);
    //LOG("the post data are: %s\n", post_data);

    curl_easy_setopt(curl, CURLOPT_URL, post_target);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    cret = curl_easy_perform(curl);
    LOG("the curl ret = %d\n", cret);
    if (cret == CURLE_OK)
        ret = 0;

    return ret;
}

int do_feedback(struct user_info *info, char *groupid, CURL *curl)
{
    char posturl[1024];
    char postdata[256];
    time_t cur;
    int ret = -1;
    CURLcode cret = CURLE_OK;

    time(&cur);

    snprintf(posturl, sizeof(posturl),
            "http://open.snssdk.com/action/push/?signature=%s&nonce=%d&timestamp=%ld&partner=%s",
            info->info_sig, info->info_rand, info->info_time, "21ke");

    snprintf(postdata, sizeof(postdata),
            "{"
             "\"access_token\":\"%s\","
             "\"actions\":"
                  "[{"
                    "\"type\":\"enter\","
                    "\"timestamp\":%ld,"
                    "\"data\" : {\"group_id\":%ld}"
                  "}]"
            "}", info->info_token, cur, atol(groupid));
    //LOG("the post data are: %s\n", postdata);
    curl_easy_setopt(curl, CURLOPT_URL, posturl);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
    cret = curl_easy_perform(curl);
    LOG("the curl ret = %d\n", cret);
    if (cret == CURLE_OK)
        ret = 0;

    return ret;
}

/**
 * I think this is a start point for sending groupid to toutiao.
 *
 * @who : indicate who view these @allgroups news, it is a KEID string
 */
int feedback_to_toutiao(const char *who, char *allgroups, CURL *curl)
{
    char *saveptr;
    char *groupid;
    map<string, struct user_info>::iterator it;

    groupid = strtok_r(allgroups, " ", &saveptr);
    while(groupid != NULL)
    {
        LOG("%s|\n", groupid);
        it = glb_list.find(who);
        if(it != glb_list.end())
        {
            do_feedback(&(it->second), groupid, curl);
        }
        else
        {
            LOG("the %s didn't existed in DB\n", who);
        }

        // keep going
        groupid = strtok_r(NULL, " ", &saveptr);
    }

    return 0;
}


int choose_user_from_db(MYSQL *ms)
{
    int ret = -1;
    char sqlcmd[1024];
    MYSQL_RES *mresult;

    snprintf(sqlcmd, sizeof(sqlcmd),
            "SELECT sn,access_token,time,rand,signature,UNIX_TIMESTAMP(token_expired) FROM navi_devices_info "
            "WHERE UNIX_TIMESTAMP(token_expired) - UNIX_TIMESTAMP(NOW()) > 3600");
    if (mysql_query(ms, sqlcmd))
    {
        LOG("**failed execute SQL cmd:(%d)%s\n",
                mysql_errno(ms), mysql_error(ms));
    }
    else
    {
        // SQL OK
        mresult = mysql_store_result(ms);

        if(mresult != NULL)
        {
            MYSQL_ROW row;
            struct user_info item;
            while((row = mysql_fetch_row(mresult)) != NULL)
            {
                // avoid too many outputs...
                //LOG("%s | %s | %s |\n", row[0], row[1], row[2]);
                strcpy(item.info_token, row[1]);
                item.info_time = atol(row[2]);
                item.info_rand = atoi(row[3]);
                strcpy(item.info_sig, row[4]);
                item.info_expired = atol(row[5]);

                // FIXME - map's key is unique, insert the same key-value element
                // would mark as failure, so below line code won't causing any
                // duplicate data.
                glb_list.insert(map<string, struct user_info>::value_type(row[0], item));
            }

            mysql_free_result(mresult);

            LOG("After traverse the whole DB, list length=%ld\n", glb_list.size());
        }
    }
    return ret;
}

/**
 * try get the group ID based on the user's click info
 */
int get_group_id_from_mem(memcached_st *memc, CURL *curl)
{
    char *memval = NULL;
    size_t val_len = 0;
    char  memkey[64];
    memcached_return_t rc;


    map<string, struct user_info>::iterator it;
    for(it = glb_list.begin(); it != glb_list.end(); it++)
    {
        snprintf(memkey, sizeof(memkey),
                ":1:%s", it->first.c_str());
        LOG("checking the mem(%s)\n", memkey);
        if (memcached_exist(memc, memkey, strlen(memkey)) == MEMCACHED_SUCCESS)
        {
            LOG("FOUND-ed...\n");
            memval = memcached_get_by_key(memc, NULL, 0,
                    memkey, strlen(memkey), &val_len, 0, &rc);
            if(memval != NULL)
            {
                if(rc == MEMCACHED_SUCCESS)
                {
                    LOG("KEY[%s] ==> [%s]\n", memkey, memval);
                    // split the strings

                    feedback_to_toutiao(it->first.c_str(), memval, curl);
                }

                free(memval);
            }

            // delete this key
            rc = memcached_delete(memc, memkey, strlen(memkey), 0);
            LOG("the deletion of memkey result=%d\n", rc);
        }
        else
        {
            LOG("not existed...\n");
        }
    }

    return 0;
}

int get_db_data(MYSQL *ms, const char *dbIP, const char *user, const char *passwd, const char *dbname)
{
    int ret = -1;

    if(glb_db_connected == 0)
    {
        if(!mysql_real_connect(ms, dbIP, user, passwd,
                dbname, 0, NULL, 0))
        {
            LOG("***failed connect to MySQL(%d) %s\n",
                mysql_errno(ms), mysql_error(ms));
        }
        else
        {
            glb_db_connected = 1;
            choose_user_from_db(ms);
        }
    }
    else
    {
        LOG("  ... connecting to DB...[connected]\n");
        choose_user_from_db(ms);
    }

    return ret;
}

/**
 * specified SQL IP(-s),  memcached IP(-m) and interval(-t, in seconds Unit)
 * if want to use http proxy, use -p option:
 *
 * ========================================
 * ./a.out -s x.x.x.x -m y.y.y.y:11211 -t 3600 -p x.x.x.x:1234
 * ========================================
 */
int main(int argc, char **argv)
{
    int c;
    CURL *cu;
    MYSQL *msql;
    memcached_st *memc;
    char sql_ip[32] = "192.168.1.19";
    char mem_ip[64] = "--SERVER=127.0.0.1:11211";
    char proxy[128];
    int  timeinterval = 20*60; // default is 20 miniutes

    proxy[0] = '\0';

    while((c = getopt(argc, argv, "s:m:p:")) != -1) {
        switch(c){
            case 's':
                strcpy(sql_ip, optarg);
                break;

            case 'm':
                snprintf(mem_ip, sizeof(mem_ip),
                        "--SERVER=%s", optarg);
                break;

            case 't':
                timeinterval = atoi(optarg);
                break;

            case 'p':
                snprintf(proxy, sizeof(proxy),
                        "http_proxy=http://%s", optarg);
                break;

            default:
                break;
        }
    }

    if(strlen(proxy) > 0)
    {
        LOG("the proxy : %s\n", proxy);
        setenv("http_proxy", proxy, 1/* overwrite */);
    }
    else
    {
        LOG("Not use the proxy\n");
    }

    cu = curl_easy_init();
    if (!cu)
    {
        LOG("failed init the CURL.\n");
        return -1;
    }

    LOG("== Init libcurl == [OK]\n");
    send_ocr_to_baidu(cu, "/home/yang/testbaidu.jpg");


    msql = mysql_init(NULL);
    if (!msql)
    {
        LOG("failed init the MySQL.\n");
        return -1;
    }

    LOG("== Init MySQL == [OK]\n");


    memc = memcached(mem_ip, strlen(mem_ip));
    if(!memc)
    {
        LOG("failed init the memcachd.\n");
        return -1;
    }

    LOG("== Conn to Memcached == [OK]\n");

    while(1) {

        get_db_data(msql, sql_ip, "root", "nanjing@!k", "rom");
        // after above function return,
        // all valid user name(keid) would
        // be stored in glb_list

        get_group_id_from_mem(memc, cu);
        sleep(timeinterval);
    }


    /* Running every 40 hours... */

    curl_easy_cleanup(cu);

    LOG("exit the program\n");
    return 0;
}
