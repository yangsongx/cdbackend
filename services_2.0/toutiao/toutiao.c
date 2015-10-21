/**
 *
 *
 */
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <curl/curl.h>

#define LOG printf

#if 1
#define TARGET_URL  "192.168.1.108:9001/v1/maker/begin/"
#else
#endif


int feedback_to_toutiao(const char *token, unsigned long groupid, const char *post_target, CURL *curl)
{
    int ret = 0;
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
    LOG("the post data are: %s\n", post_data);

    curl_easy_setopt(curl, CURLOPT_URL, post_target);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    cret = curl_easy_perform(curl);
    LOG("the curl ret = %d\n", cret);

    return ret;
}

int main(int argc, char **argv)
{
    CURL *cu;
    CURLcode cret = CURLE_OK;

    cu = curl_easy_init();
    if (!cu)
    {
        LOG("failed init the CURL.\n");
        return -1;
    }

    LOG("== Init libcurl == [OK]\n");

    feedback_to_toutiao("774d80a993ff42c04a824f0da5d1365c0013",
            6206077161168044545,
            "http://open.snssdk.com/action/push/?signature=b82fd01adfdf763382297c1da094bdad5e9e1478&nonce=4734&timestamp=1444812732&partner=21ke",
            cu);


    curl_easy_cleanup(cu);

    LOG("exit the program\n");
    return 0;
}
