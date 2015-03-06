/**
 *
 * Unit Test Code for User Register/Login/etc... Service.
 *
 */
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "UserRegister.pb.h"
#include "UserLogin.pb.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <my_global.h>
#include <mysql.h>

#include <openssl/md5.h>

#include "cds_public.h"


#define TESTING_SERVER_IP  "127.0.0.1"
#define DB_TEST_DATA_FILE  "reg_test_db.sql"

#ifdef LOG
#undef LOG
#define LOG printf
#endif

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

using namespace com::caredear;
using namespace google::protobuf::io;

MYSQL  *mSql;
int     mSock;

int     gPass = 0;
int     gFail = 0;
char    gBuffer[1024];

// copy code from CDS to avoid dependent on that package
int the_md5(const char *data, int length, char *result)
{
    size_t i;
    char md5[16];
    char tmp[10];

    MD5_CTX ctx;

    MD5_Init(&ctx);
    MD5_Update(&ctx, data, length);
    MD5_Final((unsigned char *)md5, &ctx);

    result[0] = '\0';
    for(i = 0; i < sizeof(md5); i++)
    {
        sprintf(tmp, "%02x", (unsigned char)md5[i]);
        strcat(result, tmp);
    }

    return 0;
}

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
            get_node_via_xpath("//service_2/user_register_service/sqlserver/ip",
                    ctx, sqlip, sizeof(sqlip));

            get_node_via_xpath("//service_2/user_register_service/sqlserver/user",
                    ctx, sqluser, sizeof(sqluser));

            get_node_via_xpath("//service_2/user_register_service/sqlserver/password",
                    ctx, sqlpasswd, sizeof(sqlpasswd));

            LOG("the SQL server ip:%s, user:%s, password:%s\n",
                    sqlip, sqluser,sqlpasswd);


            mSql = mysql_init(NULL);
            if(mSql != NULL){
                if(!mysql_real_connect(mSql, sqlip, sqluser, sqlpasswd,
                            "uc", 0, NULL, 0)) {
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

/////////////////////////////////////////////////////////////////////
//  User + Password case
/////////////////////////////////////////////////////////////////////
int _user_testing(DeviceType dt, const char *source, const char *name, const char *password, int passflag) {
    int ret = -1;
    RegisterRequest  req;
    RegisterResponse resp;

    req.set_reg_type(RegLoginType::NAME_PASSWD);
    req.set_reg_device(dt);
    req.set_reg_source(source);
    req.set_reg_name(name);
    req.set_reg_password(password);

    size_t size;
    unsigned short len = req.ByteSize();
    printf("    the reqobj's len = %d\n", len);

    char *b = (char *)malloc(len + 2);
    if(!b) {
        printf("***malloc return NULL\n");
        return -1;
    }

    ArrayOutputStream aos(b, len + 2);
    CodedOutputStream cos(&aos);
    cos.WriteRaw(&len, sizeof(len));
    if(req.SerializeToCodedStream(&cos)) {
        size = write(mSock, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", size);
    }
    free(b);
   
    // Now, waiting for response..
    size = read(mSock, gBuffer, sizeof(gBuffer));
    printf("    <==Server leading-len = %d\n", *(unsigned short *)gBuffer);
    if(*(unsigned short *)gBuffer > 0) {
        ArrayInputStream in(gBuffer + 2, *(unsigned short *)gBuffer);
        CodedInputStream is(&in);
        if(resp.ParseFromCodedStream(&is)) {
            printf("     result_code = %d\n", resp.result_code());
            if(resp.has_extra_msg()) {
                printf("   extra msg = %s\n", resp.extra_msg().c_str());
            }

            if(resp.result_code() == passflag) {
                ret = 0;
            }
        }
    }

    return ret;
}

int test_insert_new_user_password() {
    const char *password="anamepassword"; //DO NOT change
    char md5str[36];
    //memset(md5str, 0x00, sizeof(md5str));
    the_md5(password, strlen(password), md5str);
    printf("%s --MD5--> %s\n",
            password, md5str);

    return _user_testing(DeviceType::IOS,
            "11", // SHOULD within 2-digit
            "a_name", //DO NOT MODIFY THIS, name using the email format.
            md5str,
            CDS_OK);
}

// user + pasword, username in xxx@xxx.com format
int test_insert_new_user_password2() {

    return _user_testing(DeviceType::CAREDEAROS,
            "12", // SHOULD within 2-digit
            "ilovecss@email.com", //DO NOT MODIFY THIS name using the email format.
            "csspassword",
            CDS_OK);
}

// a nearly max username case
int test_insert_new_user_password3() {

    return _user_testing(DeviceType::IOS,
            "11", // SHOULD within 2-digit
            "abcdefghijklmnopqr", // DO NOT MODIFY
            "6d8313bde626f844ade11d9ae132f1e4", // DO NOT MODIFY
            CDS_OK);
}

int test_insert_already_existed_user_password() {

    return _user_testing(DeviceType::ANDROID,
            "0",  // should within 2-digit
            "TianFeng",
            "TianFengpassword",
            CDS_ERR_USER_ALREADY_EXISTED);
}


/////////////////////////////////////////////////////////////////////
//  EMail + Password case
/////////////////////////////////////////////////////////////////////
int _email_testing(DeviceType dt, const char *source, const char *name, const char *password, int passflag) {
    int ret = -1;
    RegisterRequest  req;
    RegisterResponse resp;

    req.set_reg_type(RegLoginType::EMAIL_PASSWD);
    req.set_reg_device(dt);
    req.set_reg_source(source);
    req.set_reg_name(name);
    req.set_reg_password(password);

    size_t size;
    unsigned short len = req.ByteSize();
    printf("    the reqobj's len = %d\n", len);

    char *b = (char *)malloc(len + 2);
    if(!b) {
        printf("***malloc return NULL\n");
        return -1;
    }

    ArrayOutputStream aos(b, len + 2);
    CodedOutputStream cos(&aos);
    cos.WriteRaw(&len, sizeof(len));
    if(req.SerializeToCodedStream(&cos)) {
        size = write(mSock, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", size);
    }
    free(b);
   
    // Now, waiting for response..
    size = read(mSock, gBuffer, sizeof(gBuffer));
    printf("    <==Server leading-len = %d\n", *(unsigned short *)gBuffer);
    if(*(unsigned short *)gBuffer > 0) {
        ArrayInputStream in(gBuffer + 2, *(unsigned short *)gBuffer);
        CodedInputStream is(&in);
        if(resp.ParseFromCodedStream(&is)) {
            printf("     result_code = %d\n", resp.result_code());
            if(resp.has_extra_msg()) {
                printf("   extra msg = %s\n", resp.extra_msg().c_str());
            }

            if(resp.result_code() == passflag) {
                ret = 0;
            }
        }
    }

    return ret;
}

int test_email_already_existed() {
    return _email_testing(DeviceType::IOS, "appstore", 
            "active@mail.com",
            "activepassword",
            CDS_ERR_USER_ALREADY_EXISTED);
}

int test_email_overwrite_inactive() {

    return _email_testing(DeviceType::ANDROID,
            "0", 
            "499528974@qq.com",
            "overwritepassword",
            CDS_OK);
}


int test_email_overwrite_inactive_toolong_source() {

    return _email_testing(DeviceType::ANDROID,
            "com.xmpp",  //<-- HERE set a long source id
            "499528974@qq.com",
            "overwritepassword",
            CDS_ERR_SQL_EXECUTE_FAILED);
}


/////////////////////////////////////////////////////////////////////
//  User + Password Login case
/////////////////////////////////////////////////////////////////////
int test_normal_user_login() {
    return 0;
}

/**
 * Main Entry point
 *
 */
int main(int argc, char **argv)
{
    bool confirm = false;

    if(argc > 1 && !strcmp(argv[1], "-c"))
        confirm = true;

    printf("Begin Unit Testing....\n\n");

    struct sockaddr_in addr;
    mSock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(TESTING_SERVER_IP);
    addr.sin_port = htons(13000);

    if(connect(mSock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("***failed connect to register server:%d\n", errno);
        return -1;
    }

    printf("Connecting to register service...[OK]\n");

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

    if(confirm){
        printf("All prepration completed, you can check the DB right now,\n"
                "press any key to continue\n");
        getchar();
    }

    // next begin testing one-by one....
    execute_ut_case(test_insert_already_existed_user_password);
    execute_ut_case(test_insert_new_user_password);
    execute_ut_case(test_insert_new_user_password2);
    execute_ut_case(test_insert_new_user_password3);

    execute_ut_case(test_email_already_existed);
    execute_ut_case(test_email_overwrite_inactive);
    execute_ut_case(test_email_overwrite_inactive_toolong_source);


    // next will try testing the login feature...
    //
    close(mSock);

    mSock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(TESTING_SERVER_IP);
    addr.sin_port = htons(13001);

    if(connect(mSock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("***failed connect to Login server:%d\n", errno);
        return -1;
    }

    printf("Connecting to Login service...[OK]\n");

    if(confirm) {
        printf("Now, Login testing is ready, will you want to begin?\n");
        getchar();
    }


    //
    // summary
    //
    printf("\nEnd of Unit Testing\n");
    printf("Totally %d case tested, %d [OK], %d [Failed]\n",
            gPass + gFail, gPass, gFail);
    close(mSock);

    return 0;
}
