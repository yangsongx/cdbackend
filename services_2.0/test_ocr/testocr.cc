/**
 *
 *
 */
#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <list>
#include <map>
#include <string>

#include <curl/curl.h>



#define LOG printf


using namespace std;



int get_ocr_result(const char *fn, const char *bin, const char *url, int port, CURL *curl)
{
    char posturl[1024];
    char *postdata = NULL;
    char type[32];
    int ret = -1;
    CURLcode cret = CURLE_OK;

    if(port == 0)
    {
        snprintf(posturl, sizeof(posturl),
                "%s/readit/",
                url);
    }
    else
    {
        snprintf(posturl, sizeof(posturl),
                "%s:%d/readit/",
                url, port);
    }

    int len = strlen(fn);
    if(fn[len - 1] == '.')
    {
        type[0] = '\0';
    }
    else
    {
        while(fn[--len] != '.' && len > 0) ;

        if( len != 0)
        {
            strcpy(type, fn + len + 1);
        }
    }

    LOG("-->type:%s\n", type);

    LOG("bin-->%s\n", bin);


    postdata = (char *)malloc(strlen(bin) + 1024);
    assert(postdata != NULL);

    snprintf(postdata, (strlen(bin) + 1024),
            "{"
             "\"suffix\":\"%s\","
             "\"data\":\"%s\""
            "}", type, bin);

    LOG("the post data are: %s\n", postdata);
    curl_easy_setopt(curl, CURLOPT_URL, posturl);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
    cret = curl_easy_perform(curl);
    LOG("the curl ret = %d\n", cret);
    if (cret == CURLE_OK)
        ret = 0;

    return ret;
}

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

char *get_img_data(const char *fn)
{
    char  *ret = NULL;
    char  *ptr = NULL;
    struct stat st;

    if(stat(fn, &st) == 0)
    {
        int len = (int)st.st_size;
        ptr = (char *)malloc(len);
        if(ptr)
        {
            FILE *fp = fopen(fn, "rb");
            if(fp)
            {
                fread(ptr, len, 1, fp);
                fclose(fp);

                ret = base64_encode(ptr, len, false);
            }

            free(ptr);
        }
    }
    else
    {
        LOG("File not existed at all\n");
    }


    return ret;
}

/**
 * testing for send a pic to OCR server, and return the text data
 *
 * ========================================
 * ./a.out -s x.x.x.x -p 9002 -i foo.jpg
 * ========================================
 */
int main(int argc, char **argv)
{
    int c;
    CURL *cu;
    char ocr_ip[32] = "192.168.1.108";
    int ocr_port = 0;
    char image[1024];
    char *img_ptr = NULL;

    image[0] = '\0';

    while((c = getopt(argc, argv, "s:p:i:")) != -1) {
        switch(c){
            case 's':
                strcpy(ocr_ip, optarg);
                break;

            case 'p':
                ocr_port = atoi(optarg);
                break;

            case 'i':
                strcpy(image, optarg);
                break;

            default:
                break;
        }
    }

    cu = curl_easy_init();
    if (!cu)
    {
        LOG("failed init the CURL.\n");
        return -1;
    }

    LOG("== Init libcurl == [OK]\n");

    if(strlen(image) <= 0)
    {
        LOG("MUST specified the picture name(-i file.type)\n");
        goto failed1;
    }

    img_ptr = get_img_data((const char *)image);
    if(img_ptr)
    {
        // keep going
        LOG("the base64 of image data\n%s\n", img_ptr);
        LOG("the whole length=%d\n", strlen(img_ptr));
        get_ocr_result(image, img_ptr, ocr_ip, ocr_port, cu);
        free(img_ptr);
    }
    else
    {
        LOG("the %s file contained ZERO data!\n", image);
    }



    /* Running every 40 hours... */
failed1:
    curl_easy_cleanup(cu);

    LOG("exit the program\n");
    return 0;
}
