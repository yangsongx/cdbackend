#include "NetdiskMessage.pb.h"
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>

#include <openssl/md5.h>
#include <qiniu/base.h>
#include <qiniu/io.h>
#include <qiniu/rs.h>


#include <list>

#include "cds_public.h"

#define DB_TEST_DATA_FILE  "nds_test_data.sql"
#define TESTING_SERVER_IP  "127.0.0.1"
#define execute_ut_case(casename) \
    printf("====Begin "); \
    printf(#casename); \
    printf("  testing ===\n"); \
    if(casename()) { \
        gFail++; \
        printf("---> Case Failed\n"); \
    } else { \
        gPass++; \
    }

using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

int     mSock;
MYSQL  *mSql;
int     gPass = 0;
int     gFail = 0;
char    gBuffer[1024];
char    gUploadToken[256];

Qiniu_Client gQn;

typedef struct _file_info{
    char f_name[32];
    int  f_size;
    char f_md5[34];
}file_info_t;

list<file_info_t>  g_FileList;

int get_node_via_xpath(const char *xpath, xmlXPathContextPtr ctx, char *result, int result_size)
{
    xmlXPathObjectPtr  obj;
    obj = xmlXPathEvalExpression((xmlChar *)xpath, ctx);
    if(obj != NULL && obj->nodesetval != NULL)
    {
        int size = obj->nodesetval->nodeNr;
        if(size > 0)
        {
            xmlNode *node = obj->nodesetval->nodeTab[0];

            if(node->children != NULL)
            {
                node = node->children;
                strncpy(result, (const char *)node->content, result_size);
            }
        }

        xmlXPathFreeObject(obj);
    }

    return 0;
}

int try_conn_db(const char *config_file)
{
    char sqlip[32];
    char sqluser[32];
    char sqlpasswd[32];
    xmlDocPtr doc;
    xmlXPathContextPtr ctx;

    if(access(config_file, F_OK) != 0)
    {
        printf("\'%s\' not existed!\n", config_file);
        return -1;
    }

    doc = xmlParseFile(config_file);
    if(doc != NULL)
    {
        ctx = xmlXPathNewContext(doc);
        if(ctx != NULL)
        {
            get_node_via_xpath("//netdisk/sqlserver/ip",
                    ctx, sqlip, sizeof(sqlip));

            get_node_via_xpath("//netdisk/sqlserver/user",
                    ctx, sqluser, sizeof(sqluser));

            get_node_via_xpath("//netdisk/sqlserver/password",
                    ctx, sqlpasswd, sizeof(sqlpasswd));

            printf("the SQL server ip:%s, user:%s, password:%s\n",
                    sqlip, sqluser,sqlpasswd);


            mSql = mysql_init(NULL);
            if(mSql != NULL){
                int a = 1;
                /* begin set some options before real connect */
                //mysql_options(mSql, MYSQL_OPT_RECONNECT, &a);
                a = 2;
                mysql_options(mSql, MYSQL_OPT_CONNECT_TIMEOUT, &a);
                mysql_options(mSql, MYSQL_OPT_READ_TIMEOUT, &a);
                printf("Set SQL options...\n");

                if(!mysql_real_connect(mSql, sqlip, sqluser, sqlpasswd,
                            "netdisk", 0, NULL, 0)) {
                    printf("error for mysql_real_connect():%s\n",
                            mysql_error(mSql));
                    return -1;
                }
            }

        }

        xmlFreeDoc(doc);
    }

    return 0;
}

int prepare_db_test_data()
{
    FILE *p = fopen(DB_TEST_DATA_FILE, "r");
    char line_buf[512];

    if(!p){
        return -1;
    }

    while(fgets(line_buf, sizeof(line_buf), p) != NULL) {
        if(!strncmp(line_buf, "/*", 2)) {
            // sql cmd comments
            continue;
        }
        if(mysql_query(mSql, line_buf)) {
            printf("**failed query SQL cmd:%s\n", mysql_error(mSql));
        }
    }

    fclose(p);

    return 0;
}
int get_md5(const char *filename, char *p_md5) {
  int ret = -1;
  char buf[2046];
  int  len = 0;
  MD5_CTX ctx;
  unsigned char bin[16];
  char tmp[4];

  FILE *f = fopen(filename, "rb");
  if(f != NULL) {
    MD5_Init(&ctx);
    
    while((len = fread(buf, 1, sizeof(buf), f)) != 0) {
      MD5_Update(&ctx, buf, len);
    }
    
    MD5_Final(bin, &ctx);
    p_md5[0] = '\0';
    for(ret =0; ret < 16; ret ++)
    {
        sprintf(tmp, "%02x", (unsigned char)bin[ret]);
        strcat(p_md5, tmp);
    }
    fclose(f);
    ret = 0;
  }
  
  return ret;
}

void fill_file_info(const char *fn, file_info_t *inf)
{
    struct stat t;
    char fullname[128];

    sprintf(fullname, "./test/%s", fn);
    printf("the full name:%s", fullname);
    memset(inf, 0x00, sizeof(file_info_t));
    if(stat(fullname, &t) == 0) {
        printf("stat OK\n");
        inf->f_size = t.st_size;
        strcpy(inf->f_name, fn);
        get_md5(fullname, (inf->f_md5));
    }
}

void traverse_test_file()
{
    DIR  *dir;
    struct dirent *ent;
    file_info_t elem;

    dir = opendir("./test");
    if(!dir)
    {
        printf("can't find the test dir\n");
        return;
    }

    while((ent = readdir(dir)) != NULL)
    {
        if(!(ent->d_type & DT_DIR)) {
            printf("find %s\n", ent->d_name);
            fill_file_info(ent->d_name, &elem);
            g_FileList.push_back(elem);
        }
    }

    closedir(dir);
}

int _send_nds_req(const char *usr, Opcode op, const char *fn, int size, const char *md5, int passflag)
{
    int ret = -1;
    NetdiskRequest req;
    NetdiskResponse resp;

    req.set_user(usr);
    req.set_opcode(op);
    if(fn)
    {
        req.set_filename(fn);
    }

    if(size != -1)
    {
        req.set_filesize(size);
    }

    if(md5)
    {
        req.set_md5(md5);
    }

    unsigned short len = req.ByteSize();
    printf("    the reqobj's len = %d\n", len);

    size_t rc;
    char *b = (char *)malloc(len + 2);
    if(!b) {
        printf("***malloc return NULL\n");
        return -1;
    }

    ArrayOutputStream aos(b, len + 2);
    CodedOutputStream cos(&aos);
    cos.WriteRaw(&len, sizeof(len));
    if(req.SerializeToCodedStream(&cos)) {
        rc = write(mSock, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", rc);
    }
    free(b);

    rc = read(mSock, gBuffer, sizeof(gBuffer));
    printf("    <==Server leading-len = %d\n", *(unsigned short *)gBuffer);
    if(*(unsigned short *)gBuffer > 0) {
        ArrayInputStream in(gBuffer + 2, *(unsigned short *)gBuffer);
        CodedInputStream is(&in);
        if(resp.ParseFromCodedStream(&is)) {
            printf("     result_code = %d\n", resp.result_code());
            if(resp.has_errormsg()) {
                printf("   extra msg = %s\n", resp.errormsg().c_str());
            }

            if(resp.has_uploadtoken()) {
                printf("   upload token=%s\n", resp.uploadtoken().c_str());
                strcpy(gUploadToken, resp.uploadtoken().c_str());
            }

            if(resp.result_code() == passflag) {
                ret = 0;
            }
        }
    }


    return ret;
}

// a good file uploading(TianFeng's image, using 130 username
int test_normal_file_upload()
{
    file_info_t item = g_FileList.front();
    return _send_nds_req(
            "13022593515", // DO NOT MODIFY, this is for tianfeng
            Opcode::UPLOADING,
            item.f_name,
            item.f_size,
            item.f_md5,
            CDS_OK);
}

// try upload the pysical file with previous upload token
int test_normal_file_upload2()
{
    int ret = -1;
    file_info_t item = g_FileList.front();
    char fullpath[256];
    Qiniu_Error err;
    Qiniu_Io_PutRet putret;
    Qiniu_Client_InitMacAuth(&gQn, 1024, NULL);

    sprintf(fullpath, "./test/%s", item.f_name);
    err = Qiniu_Io_PutFile(&gQn,
            &putret,
            gUploadToken,
            item.f_md5,
            fullpath,
            NULL);
     printf("the http upload status code:%d\n", err.code);
     if(err.code != 200)
     {
         printf("the failure msg:%s\n", err.message);
     }
     else
     {
         printf("\n\nUploaded %s to Qiniu, with MD5 %s [OK]\n",
                 fullpath, item.f_md5);
         ret = 0;
     }

    Qiniu_Client_Cleanup(&gQn);

    return ret;
}

// try check if DB updated when we finished a successful upload
int test_normal_file_upload3()
{
    file_info_t item = g_FileList.front();

    return _send_nds_req(
            "13022593515", // DO NOT MODIFY, this is for tianfeng
            Opcode::UPLOADED,
            item.f_name,
            item.f_size,
            item.f_md5,
            CDS_OK);
}

// test upload an already existed file with a different User
int test_duplicated_file_upload()
{
    file_info_t item = g_FileList.front();

    return _send_nds_req(
            "100", // DO NOT MODIFY, this is for duplicated with TianFeng(13022593515)
            Opcode::UPLOADING,
            "duuplicated.bin",
            item.f_size,
            item.f_md5,
            CDS_FILE_ALREADY_EXISTED);
}

// check previously uplodaed(exsited atually) DB updated or NOT
int test_duplicated_file_upload2()
{
    char cmd[1024];

    file_info_t item = g_FileList.front();
    sprintf(cmd, "SELECT FILENAME,MD5 FROM FILES WHERE MD5=\'%s\'",
            item.f_md5);

    if(mysql_query(mSql, cmd)){
        printf("**failed query the file in DB:%s\n", mysql_error(mSql));
        return -1;
    }

    MYSQL_ROW *row;
    MYSQL_RES *mresult;
    mresult = mysql_store_result(mSql);
    if(mresult)
    {
        // TODO more code here.
        mysql_free_result(mresult);
        printf("TODO test code\n");
        return -1;
    }
    else
    {
        printf("got a NULLresult for selection\n");
        return -1;
    }
}
// try upload an already exceed quota file
int test_bad_file_upload()
{
    return _send_nds_req(
            "1", // DO NOT MODIFY, as user-1 is prebuilt with max exceed;
            Opcode::UPLOADING,
            "hello",
            32,
            "badmd45",
            CDS_ERR_EXCEED_QUOTA);
}


// a good case for download tianfeng image
int test_normal_download_file()
{
    return -1;
}

int test_delete_file()
{
    return -1;
}

int main(int argc, char **argv) {
    bool confirm = false;

    if(argc > 1 && !strcmp(argv[1], "-c"))
        confirm = true;

    printf("begin testing on netdisk\n");

    struct sockaddr_in addr;
    mSock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(TESTING_SERVER_IP);
    addr.sin_port = htons(12001); // nds port

    if(connect(mSock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("***failed connect to nds server:%d\n", errno);
        return -1;
    }

    printf("Connecting to netdisk service...[OK]\n");


    if(try_conn_db("/etc/cds_cfg.xml") == -1)
    {
        printf("Conn to SQL ... [Failed]\n");
        return -1;
    }
    printf("Conn to SQL ... [OK]\n");

    if(prepare_db_test_data() == -1)
    {
        printf("Prepare DB... [Failed]\n");
        return -1;
    }

    printf("Prepare DB... [OK]\n");

    traverse_test_file();
    printf("after traverse the file, they are:\n");
    list<file_info_t>::iterator it;

    for(it = g_FileList.begin(); it != g_FileList.end(); ++it) {
        printf("[name:%s] [size=%d] [md5=%s]\n",
                it->f_name, it->f_size, it->f_md5);
    }


    if(confirm){
        printf("All prepration completed, you can check the DB right now,\n"
                "press any key to continue\n");
        getchar();
    }

    // test case one-by-one
    //
    //

    execute_ut_case(test_normal_file_upload);
    execute_ut_case(test_normal_file_upload2);
    execute_ut_case(test_normal_file_upload3);

    execute_ut_case(test_duplicated_file_upload);
    execute_ut_case(test_duplicated_file_upload2);

    execute_ut_case(test_bad_file_upload);

    execute_ut_case(test_normal_download_file);

    execute_ut_case(test_delete_file);

    //
    // summary
    //
    printf("\nEnd of Unit Testing\n");
    printf("Totally %d case tested, %d [OK], %d [Failed]\n",
            gPass + gFail, gPass, gFail);

    close(mSock);

    return 0;
}
