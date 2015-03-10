/**
 *
 * Unit Test Code for User Register/Login/etc... Service.
 *
 */
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include "UserRegister.pb.h"
#include "UserLogin.pb.h"
#include "UserActivation.pb.h"
#include "UserAuth.pb.h"

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

char    g_phone_sms_verifycode[64]; // store SMS verify code

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
//  Phone + SMS/password code reg case (later will try verify this via activation service for SMS)
/////////////////////////////////////////////////////////////////////
int _phone_reg_testing(RegLoginType type, DeviceType dt, const char *source, const char *name, const char *code, int passflag)
{
    int ret = -1;
    RegisterRequest  req;
    RegisterResponse resp;

    req.set_reg_type(type);
    req.set_reg_device(dt);
    req.set_reg_source(source);

    req.set_reg_name(name);
    // SMS won't contain such password
    if(type == RegLoginType::PHONE_PASSWD) {
        req.set_reg_password(code);
    }

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

            if(resp.has_reg_verifycode()) {
                printf("   SMS verify code = %s\n", resp.reg_verifycode().c_str());
                if(!strcmp(name, "17705164171")) {
                    printf("    prebuilt test number, store the code...\n");
                    strcpy(g_phone_sms_verifycode, resp.reg_verifycode().c_str());
                }
            }

            if(resp.result_code() == passflag) {
                ret = 0;
            }
        }
    }

    return ret;
}

// a normal phone, with SMS code
// NOTE - will tested later via activation service...
int test_phone_sms_reg() {
    return _phone_reg_testing(RegLoginType::MOBILE_PHONE,
            DeviceType::ANDROID,
            "2",
            "17705164171", // DO NOT modify!
            NULL, // password not handled here
            CDS_OK);
}

// reg a new mobile phone with password
int test_phone_passwd_reg() {
    const char *password="189password"; //DO NOT change
    char md5str[36];

    the_md5(password, strlen(password), md5str);
    return _phone_reg_testing(RegLoginType::PHONE_PASSWD,
            DeviceType::IOS,
            "1",
            "18911112222", // DO NOT modify!
            md5str,
            CDS_OK);
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
int _usr_passwd_testing(RegLoginType type, const char *source, const char *name, const char *password, int passflag) {
    int ret = -1;
    LoginRequest req;
    LoginResponse resp;

    req.set_login_type(RegLoginType::NAME_PASSWD);
    req.set_login_name(name);
    req.set_login_password(password);

    // NOTE, how to handle with below two items?
    req.set_login_sysid("2");
    req.set_login_session("2");

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

            if(resp.has_token()) {
                printf("   LOGIN [OK], token msg = %s\n", resp.token().c_str());
            }

            if(resp.result_code() == passflag) {
                ret = 0;
            }
        }
    }

    return ret;
}

// this is testing a normal user+password case, based on previous
// new registered normal user.
int test_normal_user_login() {
    const char *password="anamepassword"; //DO NOT change
    char md5str[36];
    //memset(md5str, 0x00, sizeof(md5str));
    the_md5(password, strlen(password), md5str);

    printf("%s --MD5--> %s\n",
            password, md5str);

    return _usr_passwd_testing(NAME_PASSWD,
            "1",         // source
            "a_name", // DO NOT modify,tested by previous case
            md5str,
            CDS_OK);
}

// this is testing an incorrect user+password case, based on previous
// new registered normal user.
int test_normal_user_login_bad_passwd() {
    const char *password="badpassword"; //DO NOT change
    char md5str[36];
    //memset(md5str, 0x00, sizeof(md5str));
    the_md5(password, strlen(password), md5str);

    printf("%s --MD5--> %s\n",
            password, md5str);

    return _usr_passwd_testing(NAME_PASSWD,
            "1",         // source
            "a_name", // DO NOT modify,tested by previous case
            md5str,
            CDS_ERR_UMATCH_USER_INFO);
}


/////////////////////////////////////////////////////////////////////
//  EMail + Password Login case
/////////////////////////////////////////////////////////////////////
int _email_passwd_testing(RegLoginType type, const char *source, const char *name, const char *password, int passflag) {
    int ret = -1;
    LoginRequest req;
    LoginResponse resp;

    req.set_login_type(RegLoginType::EMAIL_PASSWD);
    req.set_login_name(name);
    req.set_login_password(password);

    // NOTE, how to handle with below two items?
    req.set_login_sysid("2");
    req.set_login_session("2");

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

            if(resp.has_token()) {
                printf("   LOGIN [OK], token msg = %s\n", resp.token().c_str());
            }

            if(resp.result_code() == passflag) {
                ret = 0;
            }
        }
    }

    return ret;
}
int test_normal_email_login() {
    const char *password="anamepassword"; //DO NOT change
    char md5str[36];
    //memset(md5str, 0x00, sizeof(md5str));
    the_md5(password, strlen(password), md5str);

    printf("%s --MD5--> %s\n",
            password, md5str);

    return _email_passwd_testing(NAME_PASSWD,
            "1",         // source
            "a_name", // DO NOT modify,tested by previous case
            md5str,
            CDS_OK);
}

//////////////////////////////////////////////////////////////////////
// Phone+Password login
////////////////////////////////////////////////////////////////////
int _phone_passwd_login_testing(const char *source, const char *name, const char *password, int passflag) {
    int ret = -1;
    LoginRequest req;
    LoginResponse resp;

    req.set_login_type(RegLoginType::PHONE_PASSWD);
    req.set_login_name(name);
    req.set_login_password(password);

    // NOTE, how to handle with below two items?
    req.set_login_sysid("2");
    req.set_login_session("2");

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

            if(resp.has_token()) {
                printf("   LOGIN [OK], token msg = %s\n", resp.token().c_str());
            }

            if(resp.result_code() == passflag) {
                ret = 0;
            }
        }
    }

    return ret;
}

// a directly login, without activation.
int test_phone_passwd_login()
{
    const char *password="189password"; //DO NOT change
    char md5str[36];

    the_md5(password, strlen(password), md5str);
    return _phone_passwd_login_testing("0",
            "18911112222", // DO NOT MODIFY, this is reged by previous reg case..
            md5str,
            CDS_ERR_INACTIVATED);
}

// login without activation, and with wrong password
int test_phone_passwd_login2()
{
    const char *password="badpassword"; //DO NOT change
    char md5str[36];

    the_md5(password, strlen(password), md5str);
    return _phone_passwd_login_testing("0",
            "18911112222", // DO NOT MODIFY, this is reged by previous reg case..
            md5str,
            CDS_ERR_UMATCH_USER_INFO);
}

// try add an existed phone number...
int test_phone_passwd_login3()
{
    /* TODO */
    printf("STUB test code\n");
    return -1;
}

// try overwrite an inactive phone numbers...
int test_phone_passwd_login4()
{
    /* TODO */
    printf("STUB test code\n");
    return -1;
}

// try login with non-existed phone numbers..
int test_phone_passwd_login5()
{
    const char *password="password"; //DO NOT change
    char md5str[36];

    the_md5(password, strlen(password), md5str);
    return _phone_passwd_login_testing("0",
            "15150659598", // DO NOT MODIFY, this is NEVER reg in DB! Dai-MengQing's phone
            md5str,
            CDS_ERR_UMATCH_USER_INFO);

    return -1;
}
/////////////////////////////////////////////////////////////////////
// activation/login case
/////////////////////////////////////////////////////////////////////
int _activation_testing(RegLoginType type, const char *name, const char *code, int passflag) {
    int ret = -1;
    ActivateRequest req;
    ActivateResponse resp;

    req.set_activate_type(type);
    req.set_activate_name(name);
    req.set_activate_code(code);

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



// normal case
int test_activation_phone_smscode() {
    return _activation_testing(RegLoginType::MOBILE_PHONE,
            "13022593515", // DO NOT MODIFY
            "123456", // DO NOT MODIFY
            CDS_OK);
}

// correct code + expiration
int test_activation_phone_smscode2() {
    return _activation_testing(RegLoginType::MOBILE_PHONE,
            "13022593516", // DO NOT MODIFY
            "567890", // DO NOT MODIFY
            CDS_ERR_CODE_EXPIRED);
}

// incorrect code
int test_activation_phone_smscode3() {
    return _activation_testing(RegLoginType::MOBILE_PHONE,
            "13022593517", // DO NOT MODIFY
            "567843", // DO NOT MODIFY
            CDS_ERR_INCORRECT_CODE);
}

// verify with previous register created SMS code
int test_activation_phone_smscode4() {
    return _activation_testing(RegLoginType::MOBILE_PHONE,
            "17705164171", // DO NOT MODIFY
            g_phone_sms_verifycode, // DO NOT MODIFY
            CDS_OK);
}

/////////////////////////////////////////////////////////////////////
// auth test
/////////////////////////////////////////////////////////////////////
int _auth_testing(const char *token, const char *session, int sysid, int passflag) {
    int ret = -1;
    AuthRequest req;
    AuthResponse resp;

    req.set_auth_token(token);
    req.set_auth_session(session);
    req.set_auth_sysid(sysid);

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
            if(resp.has_caredear_id()) {
                printf("   user's cid = %ld\n", resp.caredear_id());
            }

            if(resp.result_code() == passflag) {
                ret = 0;
            }
        }
    }

    return ret;
}

// a normal pass auth case
int test_auth_normal_case() {
    return _auth_testing(
            "aa776f46-cc18-4c44-a7c5-124c7afc45bf",  // DO NOT MODIFY
            "2", // DO NOT MODIFY
            2, // sys ID, DO NOT MODIFY
            CDS_OK);
}

// test an longlong ago auth(expiration case)
int test_auth_normal_case2() {
    return _auth_testing(
            "11776f46-cc18-4c44-a7c5-124c7afc45bf",  // DO NOT MODIFY
            "2", // DO NOT MODIFY
            2, // sys ID, DO NOT MODIFY
            CDS_ERR_USER_TOKEN_EXPIRED);
}

// verify time updated when exceed the threshold of interval
int test_auth_normal_case3() {
    int ret;
    time_t cur;

    time(&cur);

    ret = _auth_testing(
            "22776f46-cc18-4c44-a7c5-124c7afc45bf",  // DO NOT MODIFY
            "2", // DO NOT MODIFY
            1, // sys ID, DO NOT MODIFY
            CDS_OK);

    if(ret == 0) {
        // keep going on..., check the time updated or NOT.
        char cmd[256];
        sprintf(cmd, "SELECT UNIX_TIMESTAMP(lastoperatetime) FROM uc.uc_session "
                "WHERE ticket=\'22776f46-cc18-4c44-a7c5-124c7afc45bf\'");
        if(mysql_query(mSql, cmd)) {
            printf("**failed call SQL cmd:%s\n", mysql_error(mSql));
            return -1;
        }

        MYSQL_RES *mresult;
        MYSQL_ROW  row;
        mresult = mysql_store_result(mSql);
        if(mresult) {
            row = mysql_fetch_row(mresult);
            long a = atol(row[0]);
            printf("    time in DB:%ld, cur time:%ld, delta=%ld sec\n",
                    a, cur, labs(a-cur));
            if(labs(a - cur) <= 60) {
                ret = 0;
            } else {
                printf("    time delta not within pass area!\n");
                ret = -1;
            }
        } else {
            ret = -1;
        }

    }

    return ret;
}

// test for NOT-Allowed case
int test_auth_normal_case4() {
    return _auth_testing(
            "11776f46-cc18-4c44-a7c5-124c7afc45bf",  // DO NOT MODIFY
            "2", // DO NOT MODIFY
            99, // sys ID, DO NOT MODIFY, this is for REJECT case!
            CDS_ERR_REJECT_LOGIN);
}
/////////////////////////////////////////////////////////////////////
// misc test
/////////////////////////////////////////////////////////////////////
int test_sql_auto_reconnect() {
    printf("SUB code, SQL auto-reconnect failed\n");
    return -1;
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

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // next begin testing one-by one....
    // First, Registration
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    execute_ut_case(test_insert_already_existed_user_password);
    execute_ut_case(test_insert_new_user_password);
    execute_ut_case(test_insert_new_user_password2);
    execute_ut_case(test_insert_new_user_password3);

    execute_ut_case(test_email_already_existed);
    execute_ut_case(test_email_overwrite_inactive);
    execute_ut_case(test_email_overwrite_inactive_toolong_source);

    execute_ut_case(test_phone_sms_reg); // normal phone+SMS case, will test SMS code later in activation service
    execute_ut_case(test_phone_passwd_reg);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // next will try testing the login feature...
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
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

    execute_ut_case(test_normal_user_login);
    execute_ut_case(test_normal_user_login_bad_passwd);
    execute_ut_case(test_phone_passwd_login);
    execute_ut_case(test_phone_passwd_login2);
    execute_ut_case(test_phone_passwd_login3);
    execute_ut_case(test_phone_passwd_login4);
    execute_ut_case(test_phone_passwd_login5);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // next will try testing the activation feature...
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    close(mSock);
    mSock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(TESTING_SERVER_IP);
    addr.sin_port = htons(13002);

    if(connect(mSock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("***failed connect to Activation server:%d\n", errno);
        return -1;
    }

    printf("Connecting to Activation service...[OK]\n");

    if(confirm) {
        printf("Now, Activation testing is ready, will you want to begin?\n");
        getchar();
    }

    execute_ut_case(test_activation_phone_smscode);
    execute_ut_case(test_activation_phone_smscode2);
    execute_ut_case(test_activation_phone_smscode3);
    execute_ut_case(test_activation_phone_smscode4); // SMS geneared via @test_phone_sms_reg

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // next will try testing the auth feature...
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    close(mSock);
    mSock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(TESTING_SERVER_IP);
    addr.sin_port = htons(13003);

    if(connect(mSock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("***failed connect to Auth server:%d\n", errno);
        return -1;
    }

    printf("Connecting to Auth service...[OK]\n");

    execute_ut_case(test_auth_normal_case);
    execute_ut_case(test_auth_normal_case2);
    execute_ut_case(test_auth_normal_case3);
    execute_ut_case(test_auth_normal_case4);


    //misc testing...
    execute_ut_case(test_sql_auto_reconnect);

    //
    // summary
    //
    printf("\nEnd of Unit Testing\n");
    printf("Totally %d case tested, %d [OK], %d [Failed]\n",
            gPass + gFail, gPass, gFail);
    close(mSock);

    return 0;
}
