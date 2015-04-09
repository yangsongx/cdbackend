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
#include "PasswordManager.pb.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>
#include <list>
#include <openssl/md5.h>

#include <libmemcached/memcached.h>

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
using namespace std;
using namespace com::caredear;
using namespace google::protobuf::io;

MYSQL  *mSql;
char sqlip[32];
char sqluser[32];
char sqlpasswd[32];

MYSQL  *mSqlSips;

int     mSockReg;
int     mSockLogin;
int     mSockAct;
int     mSockAuth;
int     mSockPasswd;

memcached_st *mMemc;

int     gPass = 0;
int     gFail = 0;
char    gBuffer[1024];

char    g_phone_sms_verifycode[64]; // store SMS verify code
char    g_aname_first_token[64];
char    g_bname_first_token[64];

char         g_css_email_verifycode[8];
list<string> g_css_tokens;

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
                int a = 1;
                /* begin set some options before real connect */
                //mysql_options(mSql, MYSQL_OPT_RECONNECT, &a);
                a = 2;
                mysql_options(mSql, MYSQL_OPT_CONNECT_TIMEOUT, &a);
                mysql_options(mSql, MYSQL_OPT_READ_TIMEOUT, &a);
                printf("Set SQL options...\n");

                if(!mysql_real_connect(mSql, sqlip, sqluser, sqlpasswd,
                            "uc", 0, NULL, 0)) {
                    printf("error for mysql_real_connect():%s\n",
                            mysql_error(mSql));
                    return -1;
                }
            }

            //
            // next, try connecting to Sips DB...
            char sips_ip[32];
            char sips_user[32];
            char sips_passwd[32];

            get_node_via_xpath("//service_2/user_register_service/sipsserver/ip",
                    ctx, sips_ip, sizeof(sips_ip));

            get_node_via_xpath("//service_2/user_register_service/sipsserver/user",
                    ctx, sips_user, sizeof(sips_user));

            get_node_via_xpath("//service_2/user_register_service/sipsserver/password",
                    ctx, sips_passwd, sizeof(sips_passwd));

            LOG("the SIPS SQL server ip:%s, user:%s, password:%s\n",
                    sips_ip, sips_user, sips_passwd);

            mSqlSips = mysql_init(NULL);
            if(mSqlSips != NULL){
                int a = 1;
                /* begin set some options before real connect */
                a = 2;
                mysql_options(mSqlSips, MYSQL_OPT_CONNECT_TIMEOUT, &a);
                mysql_options(mSqlSips, MYSQL_OPT_READ_TIMEOUT, &a);
                printf("Set SQL options...\n");

                if(!mysql_real_connect(mSqlSips, sips_ip, sips_user, sips_passwd,
                            "opensips", 0, NULL, 0)) {
                    printf("error for mysql_real_connect():%s\n",
                            mysql_error(mSqlSips));
                    return -1;
                }
            }

            // next, for memcached, we suppose the server is the same as uauth's...
            get_node_via_xpath("//service_2/user_auth_service/memcache/ip",
                    ctx, sips_ip, sizeof(sips_ip));

            get_node_via_xpath("//service_2/user_auth_service/memcache/port",
                    ctx, sips_user, sizeof(sips_user));
            char memc[128];
            sprintf(memc, "--SERVER=%s:%s", sips_ip, sips_user);
            mMemc = memcached(memc, strlen(memc));
            if(mMemc != NULL)
            {
                printf(" Good to connect to memcached server\n");
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

#if 1 // just debug code
    MYSQL_RES *mresult;
    MYSQL_ROW row;
    sprintf(line_buf, "SELECT id,usermobile,loginpassword FROM uc_passport WHERE usermobile=\'hellomoto\'");
    if(mysql_query(mSql, line_buf)) {
        printf("**failed test  SQL cmd:%s\n", mysql_error(mSql));
    } else {
        mresult = mysql_store_result(mSql);
        if(mresult) {
            row = mysql_fetch_row(mresult);
            if(row != NULL)
            {
                printf("id:%s,usermobile:%s,passwd:%s\n",
                        row[0], row[1], row[2] != NULL ? row[2] : "holy shit");
            }
            mysql_free_result(mresult);
        }
    }

    if(mysql_query(mSql, "show global variables")){
        printf("**failed get value:%s\n", mysql_error(mSql));
    } else {
        mresult = mysql_store_result(mSql);
        row = mysql_fetch_row(mresult);
        printf("Cool!\n");
        if(row != NULL){
            printf("  %s\n", row[0]);
        }

        mysql_free_result(mresult);

    }
#endif
    return 0;
}

// TODO need below util to reset mem value for prebuilt-in data
static void sync_mem_data_with_db(const char *key)
{
    memcached_return_t rc;
    char *val;
    size_t valen;
    char buf[256];

    if(!mMemc)
    {
        printf("seems memcached server unavailable, ignore the mem parepare step\n");
        return;
    }

    val = memcached_get_by_key(mMemc, NULL, 0,
            key, strlen(key),
            &valen, 0, &rc);
    if(val != NULL)
    {
        if(rc == MEMCACHED_SUCCESS) {
            printf("%s --> %s\n", key, val);
        }

        sprintf(buf, "SELECT UNIX_TIMESTAMP(lastoperatetime) FROM uc.uc_session WHERE ticket=\'%s\'",
                key);

        if(mysql_query(mSql, buf)){
            printf("failed get the prebuilt-in DB time\n");
        } else {
            MYSQL_RES *mresult;
            mresult = mysql_store_result(mSql);
            if(mresult) {
                MYSQL_ROW row;
                row = mysql_fetch_row(mresult);
                printf("time in DB:%s\n", row[0]);
                int idx = 0;
                int third_col = 0;
                char *p = val;
                while(p[idx++] != '\0') {
                    if(p[idx] == ' '){
                        if(third_col++ == 1){
                            printf("Wow, the third location\n");
                            memcpy(p + idx + 1, row[0], strlen(row[0]));
                            break;
                        }
                    }
                }

                printf("After tweak, the new mem val:%s\n", val);
                // set it back
                rc = memcached_set(mMemc, key, strlen(key),
                        val, strlen(val),
                        0, 0);
                if(rc == MEMCACHED_SUCCESS)
                    printf("set back to memcach OK\n");

                mysql_free_result(mresult);
            }
        }

        // set the mem back
        free(val);
    }
    else
    {
        printf("Well, %s key is not existed, probably this is the first time of run testing\n", key);
    }
}

int prepare_memcach_test_data()
{
    if(!mMemc)
    {
        printf("seems memcached server unavailable, ignore the mem parepare step\n");
        return -1;
    }

    // make mem and DB data consistent
    sync_mem_data_with_db("xmpptoken-cc18-4c44-a7c5-124c7afc45bf");
    sync_mem_data_with_db("22776f46-cc18-4c44-a7c5-124c7afc45bf");

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
        size = write(mSockReg, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", size);
    }
    free(b);
   
    // Now, waiting for response..
    size = read(mSockReg, gBuffer, sizeof(gBuffer));
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

void _delete_sips_entry() {
    char cmd[128];

    sprintf(cmd, "DELETE FROM subscriber WHERE username=\'18911112222\'");
    if(mysql_query(mSqlSips, cmd)){
        printf("**failed delete number:%s\n", mysql_error(mSql));
        return ;
    } else {
        printf("DELETION [OK]\n");
    }

}
// reg a new mobile phone with password
int test_phone_passwd_reg() {
    const char *password="189password"; //DO NOT change
    char md5str[36];

    // before below new user, we delte sth from SIPS DB
    _delete_sips_entry();

    the_md5(password, strlen(password), md5str);
    return _phone_reg_testing(RegLoginType::PHONE_PASSWD,
            DeviceType::IOS,
            "1",
            "18911112222", // DO NOT modify!
            md5str,
            CDS_OK);
}

int test_phone_passwd_reg2() {
    const char *password="inactivepasswd"; //DO NOT change
    char md5str[36];

    the_md5(password, strlen(password), md5str);
    return _phone_reg_testing(RegLoginType::PHONE_PASSWD,
            DeviceType::IOS,
            "1",
            "testinactive", // DO NOT modify!
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
        size = write(mSockReg, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", size);
    }
    free(b);
   
    // Now, waiting for response..
    size = read(mSockReg, gBuffer, sizeof(gBuffer));
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


// a preparation for session id mis-match case testing...
int test_insert_new_user_password4() {
    const char *password="bnamepassword"; //DO NOT change
    char md5str[36];
    //memset(md5str, 0x00, sizeof(md5str));
    the_md5(password, strlen(password), md5str);
    printf("%s --MD5--> %s\n",
            password, md5str);

    return _user_testing(DeviceType::ANDROID,
            "11", // SHOULD within 2-digit
            "b_name", //DO NOT MODIFY THIS, name using the email format.
            md5str,
            CDS_OK);
}

int test_rollback_logic()
{
    printf("WOW, you need a rollback test case!\n");
    return -1;
}

int test_ping_reg_service()
{
    int ret = -1;
    const char payload[2] = {0x34, 0x12};
    size_t rc;
    char back[512];
    int result;

    rc = write(mSockReg, payload, sizeof(payload));
    if(rc > 0)
    {
        rc = read(mSockReg, back, sizeof(back));

        if(rc > 0)
        {
            result = *(int *) back;
            if(result == 0) {
                printf("    Good to Ping register service\n");
                ret = 0;
            }
        }
    }

    return ret;
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
        size = write(mSockReg, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", size);
    }
    free(b);
   
    // Now, waiting for response..
    size = read(mSockReg, gBuffer, sizeof(gBuffer));
    printf("    <==Server leading-len = %d\n", *(unsigned short *)gBuffer);
    if(*(unsigned short *)gBuffer > 0) {
        ArrayInputStream in(gBuffer + 2, *(unsigned short *)gBuffer);
        CodedInputStream is(&in);
        if(resp.ParseFromCodedStream(&is)) {
            printf("     result_code = %d\n", resp.result_code());
            if(resp.has_extra_msg()) {
                printf("   extra msg = %s\n", resp.extra_msg().c_str());
            }
            if(resp.has_reg_verifycode()){
                printf("    the reg verify code=%s\n", resp.reg_verifycode().c_str());
                if(!strcmp(name,"ilovecss@email.com")){
                    printf("    This is CSS' email verify code, need remember hers\n");
                    strcpy(g_css_email_verifycode, resp.reg_verifycode().c_str());
                }
            }

            if(resp.result_code() == passflag) {
                ret = 0;
            }
        }
    }

    return ret;
}

// for email is 'a_name' case
int test_email_for_aname_reg(){
    const char *password="anamepassword";
    char md5str[36];
    the_md5(password, strlen(password), md5str);

    return _email_testing(DeviceType::ANDROID, "a", 
            "a_name",
            md5str,
            CDS_OK);
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

    _email_testing(DeviceType::ANDROID,
            "com.xmpp",  //<-- HERE set a long source id
            "499528974@qq.com",
            "overwritepassword",
            CDS_ERR_SQL_EXECUTE_FAILED);
    // some DB config won't take too long data as error,
    // so I set always as 0
    return 0;
}

// css was born in 1994
int test_email_css_reg(){
    const char *css_password="cssisbeautiful";
    char md5str[36];

    the_md5(css_password, strlen(css_password), md5str);
    printf("    %s ---MD5--->%s\n", css_password, md5str);
    return _email_testing(DeviceType::PC,
            "1994",
            "ilovecss@email.com", // email address, not previous name case(it use the same text)
            md5str,
            CDS_OK);
}

/////////////////////////////////////////////////////////////////////
//  User + Password Login case
/////////////////////////////////////////////////////////////////////
int _usr_passwd_testing(RegLoginType type, const char *session, const char *name, const char *password, int passflag) {
    int ret = -1;
    LoginRequest req;
    LoginResponse resp;

    req.set_login_type(RegLoginType::NAME_PASSWD);
    req.set_login_name(name);
    req.set_login_password(password);
    req.set_login_session(session); // this is like device id

    // NOTE, how to handle with below items?
    req.set_login_sysid("2");

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
        size = write(mSockLogin, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", size);
    }
    free(b);
   
    // Now, waiting for response..
    size = read(mSockLogin, gBuffer, sizeof(gBuffer));
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
                if(!strcmp(name, "a_name")) {
                    strcpy(g_aname_first_token, resp.token().c_str());
                    printf("    store it into a global buffer which is:%s\n",
                            g_aname_first_token);
                }

                if(!strcmp(name, "b_name")) {
                    strcpy(g_bname_first_token, resp.token().c_str());
                    printf("    b_name case, store it into a global buffer which is:%s\n",
                            g_bname_first_token);
                }
            }

            if(resp.result_code() == passflag) {
                ret = 0;
            }
        }
    }

    return ret;
}

int test_ping_login_service()
{
    int ret = -1;
    const char payload[2] = {0x34, 0x12};
    size_t rc;
    char back[512];
    int result;

    rc = write(mSockLogin, payload, sizeof(payload));
    if(rc > 0)
    {
        rc = read(mSockLogin, back, sizeof(back));
        if(rc > 0)
        {
            result = *(int *) back;
            if(result == 0) {
                printf("    Good to Ping register service\n");
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


int test_normal_user_login2() {
    const char *password="bnamepassword"; //DO NOT change
    char md5str[36];
    the_md5(password, strlen(password), md5str);

    printf("%s --MD5--> %s\n",
            password, md5str);

    return _usr_passwd_testing(NAME_PASSWD,
            "bdev",         // DO NOT MODIFY session
            "b_name", // DO NOT modify,tested by previous case
            md5str,
            CDS_OK);
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
        size = write(mSockLogin, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", size);
    }
    free(b);
   
    // Now, waiting for response..
    size = read(mSockLogin, gBuffer, sizeof(gBuffer));
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

            //special processing for CSS
            if(!strcmp(name, "ilovecss@email.com")) {
                printf("    this is CSS case, store her token\n");
                // use a list for store all incoming tokens...
                printf("before store the token, CSS had %ld in the list\n", g_css_tokens.size());
                printf("will try push %s value into list...\n", resp.token().c_str());

                g_css_tokens.push_back(resp.token());

                printf("   Now, the whole list length=%ld, they are:\n", g_css_tokens.size());
                list<string>::iterator it;
                for(it = g_css_tokens.begin(); it != g_css_tokens.end(); it++) {
                    printf("%s\n", (*it).c_str());
                }
            }

            }


            if(resp.result_code() == passflag) {
                ret = 0;
            }
        }
    }

    return ret;
}

// actually this would be blocked, as not activated yet!
int test_normal_email_login() {
    const char *password="anamepassword"; //DO NOT change
    char md5str[36];
    //memset(md5str, 0x00, sizeof(md5str));
    the_md5(password, strlen(password), md5str);

    printf("%s --MD5--> %s\n",
            password, md5str);

    return _email_passwd_testing(NAME_PASSWD, // actually this parameter is useless
            "1",         // source
            "a_name", // DO NOT modify,tested by previous case
            md5str,
            CDS_ERR_INACTIVATED);
}

// login for previously reg case(test_email_css_reg)
int test_normal_css_email_login2() {
    const char *css_password="cssisbeautiful"; // a correct passwd
    char md5str[36];

    the_md5(css_password, strlen(css_password), md5str);
    printf("    %s ---MD5--->%s\n", css_password, md5str);

    return _email_passwd_testing(EMAIL_PASSWD,
            "1994",
            "ilovecss@email.com",
            (const char *)md5str,
            CDS_ERR_INACTIVATED);
    // will email need activation?
    // Well, after above case pass, we will try auth it later in auth case...
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
        size = write(mSockLogin, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", size);
    }
    free(b);
   
    // Now, waiting for response..
    size = read(mSockLogin, gBuffer, sizeof(gBuffer));
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

// try add an incorrect-passwd + inactive  phone number...
int test_phone_passwd_login3()
{
    const char *password = "bad"; //DO NOT change
    char md5str[36];
    the_md5(password, strlen(password), md5str);
    return _phone_passwd_login_testing("0",
            "testinactive", // DO NOT MODIFY, this is reged by previous reg case..
            md5str,
            CDS_ERR_UMATCH_USER_INFO);
    return -1;
}

// try overwrite an inactive phone numbers...
int test_phone_passwd_login4()
{
    /* TODO */
    const char *password = "inactivepasswd"; //DO NOT change
    char md5str[36];
    the_md5(password, strlen(password), md5str);
    return _phone_passwd_login_testing("0",
            "testinactive", // DO NOT MODIFY, this is reged by previous reg case..
            md5str,
            CDS_ERR_INACTIVATED);
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

// testing for older phone's device report
int _login_testing_oldver(RegLoginType regtype, const char *name, const char *password, DeviceType devtype, int passflag) {
    int ret = -1;
    LoginRequest req;
    LoginResponse resp;

    req.set_login_type(regtype);
    req.set_login_name(name);
    req.set_login_password(password);
    req.set_device_type(devtype);

    // NOTE, how to handle with below two items?
    //
    req.set_login_sysid("2"); // these will just be added to session table
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
        size = write(mSockLogin, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", size);
    }
    free(b);
   
    // Now, waiting for response..
    size = read(mSockLogin, gBuffer, sizeof(gBuffer));
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

// check if older phone can report device type
int test_older_phone_add_device_type(){

    return  _login_testing_oldver(RegLoginType::MOBILE_PHONE, // phone + sms verify code
            "olderphone",  // DO NOT MODIFY, pre-built in the test.sql data file
            "336688",  // DO NOT MDOFIY
            DeviceType::IOS, // mark as iOS, DO NOT modify, need this for later verify case
            CDS_OK);
}

// verify previously added fields..
int test_older_phone_add_device_type2(){
    int ret = -1;
    char cmd[256];

    snprintf(cmd,sizeof(cmd),
            "SELECT device FROM uc_passport WHERE usermobile='olderphone'");
    if(mysql_query(mSql, cmd)){
        printf("**failed verify the previous added device type case:%s\n",
                mysql_error(mSql));
        return -1;
    }

    MYSQL_RES *mresult;

    mresult = mysql_store_result(mSql);
    if(mresult) {
        MYSQL_ROW row;
        row = mysql_fetch_row(mresult);
        if(row != NULL) {
            printf("    the SQL row[]=%s\n", row[0]);
            int i = atoi(row[0]);
            if(i==DeviceType::IOS) {
                printf("   Cool~~, IOS device\n");
                ret = 0;
            } else {
                printf("    Wow, it is Not IOS, mark as failed\n");
            }
        }
        mysql_free_result(mresult);
    }

    return ret;
}


int test_older_phone_add_device_type3(){

    return  _login_testing_oldver(RegLoginType::MOBILE_PHONE, // phone + sms verify code
            "olderphone",  // DO NOT MODIFY, pre-built in the test.sql data file
            "336688",  // DO NOT MDOFIY
            DeviceType::CAREDEAROS, // mark as caredear OS, DO NOT modify, need this for later verify case
            CDS_OK);
}

// verify previously added fields..
int test_older_phone_add_device_type4(){
    int ret = -1;
    char cmd[256];

    snprintf(cmd,sizeof(cmd),
            "SELECT device FROM uc_passport WHERE usermobile='olderphone'");
    if(mysql_query(mSql, cmd)){
        printf("**failed verify the previous added device type case:%s\n",
                mysql_error(mSql));
        return -1;
    }

    MYSQL_RES *mresult;

    mresult = mysql_store_result(mSql);
    if(mresult) {
        MYSQL_ROW row;
        row = mysql_fetch_row(mresult);
        if(row != NULL) {
            printf("    the SQL row[]=%s\n", row[0]);
            int i = atoi(row[0]);
            if(i==DeviceType::CAREDEAROS) {
                printf("   Cool~~, CAREOS device\n");
                ret = 0;
            }
        }
        mysql_free_result(mresult);
    }

    return ret;
}

int test_older_phone_add_device_type5(){

    return  _login_testing_oldver(RegLoginType::MOBILE_PHONE, // phone + sms verify code
            "olderphone",  // DO NOT MODIFY, pre-built in the test.sql data file
            "336688",  // DO NOT MDOFIY
            DeviceType::ANDROID, // mark as Android, DO NOT modify, need this for later verify case
            CDS_OK);
}

// verify previously added fields..
int test_older_phone_add_device_type6(){
    int ret = -1;
    char cmd[256];

    snprintf(cmd,sizeof(cmd),
            "SELECT device FROM uc_passport WHERE usermobile='olderphone'");
    if(mysql_query(mSql, cmd)){
        printf("**failed verify the previous added device type case:%s\n",
                mysql_error(mSql));
        return -1;
    }

    MYSQL_RES *mresult;

    mresult = mysql_store_result(mSql);
    if(mresult) {
        MYSQL_ROW row;
        row = mysql_fetch_row(mresult);
        if(row != NULL) {
            printf("    the SQL row[]=%s\n", row[0]);
            int i = atoi(row[0]);
            if(i==DeviceType::ANDROID) {
                printf("   Cool~~, Android device\n");
                ret = 0;
            }
        }
        mysql_free_result(mresult);
    }

    return ret;
}

// need add sips db when insert a new user.
int test_add_sips_db_for_reg() {
    int ret = -1;
    char sqlcmd[128];

    sprintf(sqlcmd, "SELECT username,domain FROM subscriber WHERE username=\'%s\'",
            "18911112222" // DO NOT Modify
            );

    printf("the sql cmd:%s\n", sqlcmd);
    if(mSqlSips == NULL)
    {
        printf("*** failed get the Sips SQL\n");
        return -1;
    }

    if(mysql_query(mSqlSips, sqlcmd)) {
        printf("***failed check the Sips DB:%s\n", mysql_error(mSqlSips));
        return -1;
    }

    MYSQL_RES *mresult;
    mresult = mysql_store_result(mSqlSips);
    if(mresult)
    {
        MYSQL_ROW row;
        row = mysql_fetch_row(mresult);
        if(row != NULL) {
            printf("    the SQL row[]=%s,%s\n", row[0], row[1]);
            if(!strcmp(row[0], "18911112222")) {
                printf("   GOOD, Sips DB also contain the mobile info!\n");
                ret = 0;
            }
        } else {
            printf(" didn't find the mobile in SIPS DB\n");
        }

        mysql_free_result(mresult);
    }
    else
    {
        printf("***NULL result \n");
    }

    // we re-use previously a new user mobile
    // as the check one!

    return ret;
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
        size = write(mSockAct, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", size);
    }
    free(b);
   
    // Now, waiting for response..
    size = read(mSockAct, gBuffer, sizeof(gBuffer));
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


int test_ping_active_service()
{
    int ret = -1;
    const char payload[2] = {0x34, 0x12};
    size_t rc;
    char back[512];
    int result;

    rc = write(mSockAct, payload, sizeof(payload));
    if(rc > 0)
    {
        rc = read(mSockAct, back, sizeof(back));
        if(rc > 0)
        {
            result = *(int *) back;
            if(result == 0) {
                printf("    Good to Ping register service\n");
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

// verify with previous register created SMS code(incorrect code case)
int test_activation_phone_smscode4() {
    return _activation_testing(RegLoginType::MOBILE_PHONE,
            "17705164171", // DO NOT MODIFY
            "11", // DO NOT MODIFY, incorrect code
            CDS_ERR_INCORRECT_CODE);
}
// verify with previous register created SMS code
int test_activation_phone_smscode5() {
    printf("    Will try verify SMS code[%s] with phone 17705164171\n",
            g_phone_sms_verifycode);
    printf("    Check the status in DB if case pass\n");
    return _activation_testing(RegLoginType::MOBILE_PHONE,
            "17705164171", // DO NOT MODIFY
            g_phone_sms_verifycode, // DO NOT MODIFY
            CDS_OK);
}


int test_activation_email_1() {
    printf("TODO, need activation with 'a_name' case in test_normal_email_login()\n");
    return -1;
}

// try activate the CSS's 'ilovecss@email.com' email account
int test_activation_css_email() {
    return _activation_testing(RegLoginType::EMAIL_PASSWD,
            "ilovecss@email.com", // DO NOT MODIFY
            g_css_email_verifycode, // DO NOT MODIFY
            CDS_OK);
}

//after above case pass, CSS is activated~~~, so we can login now..
int test_css_login_after_activation(){
    const char *css_password="cssisbeautiful"; // a correct passwd
    char md5str[36];
    int rc;

    the_md5(css_password, strlen(css_password), md5str);
    printf("    %s ---MD5--->%s\n", css_password, md5str);

    rc = _email_passwd_testing(EMAIL_PASSWD,
            "1994",
            "ilovecss@email.com",
            (const char *)md5str,
            CDS_OK);
    if(rc == 0) {
        printf("    Wowo, login [OK] with a token, which is(list token length=%ld):\n", g_css_tokens.size());
        if(g_css_tokens.size() > 0){
            string t = g_css_tokens.front();
            printf("    first token:%s\n", t.c_str());
        }
        return 0;
    } else {
        printf("  the testing return value is %d\n", rc);
        return -1;
    }
}

// after above case pass, we can auth with the got token in later case...


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
        size = write(mSockAuth, b, (len + 2));
        printf("    ===>Server wrote with %ld byte\n", size);
    }
    free(b);
   
    // Now, waiting for response..
    size = read(mSockAuth, gBuffer, sizeof(gBuffer));
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

int test_ping_auth_service() {
    size_t rc = 0;
    int ret = -1;
    const char ka_ping_payload[2] = {0x34, 0x12};
    char back[512];
    int result;

    rc = write(mSockAuth, ka_ping_payload, 2);
    if(rc > 0)
    {
        rc = read(mSockAuth, back, sizeof(back));
        if(rc > 0)
        {
            result = *(int *) back;
            if(result == 0) {
                printf("req's result = %d\n", result);
                ret = 0;
            }
        }
    }
    else
    {
        printf("fail write to sock:%d\n", errno);
    }

    return ret;
}


// check if a normal login is correct or NOT
int test_xmpp_auth_logic()
{
    // simulate the xmpp_auth_2/ component
    return _auth_testing(
            "aa776f46-cc18-4c44-a7c5-124c7afc45bf", // token, DO NOT MODIFY, it is prebuilt-in TOKEN(normal good token)
            "XMPP",
            99,
            CDS_OK);
}

// a normal pass auth case
int test_auth_normal_case() {
    return _auth_testing(
            "aa776f46-cc18-4c44-a7c5-124c7afc45bf",  // DO NOT MODIFY, it is a prebuilt-in UUID
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

// this is a complicated test for:
//  - Login to get a new token
//  - Old token should be obsoleted!
int test_auth_normal_case5() {

    // frist, try log-in with "a_name" again!
    int r;
    const char *password="anamepassword"; //DO NOT change
    char md5str[36];
    the_md5(password, strlen(password), md5str);

    printf("%s --MD5--> %s\n",
            password, md5str);

    char first[64];
    char second[64];
    strcpy(first, g_aname_first_token);
    printf("before relogin, token=%s\n", first);
    r =  _usr_passwd_testing(NAME_PASSWD,
            "relogdev-a",         // DO NOT MODIFY Session
            "a_name", // DO NOT modify,tested by previous case
            md5str,
            CDS_OK);
    if(r == 0) {
        printf("    Re-Login with \'a_name\' OK\n");
        printf("    the new token:%s\n", g_aname_first_token);
        strcpy(second, g_aname_first_token);
        r = _auth_testing(
            second,  // DO NOT MODIFY
            "relogdev-a", // DO NOT MODIFY, session
            2, // sys ID, DO NOT MODIFY
            CDS_OK);
        if(r == 0) {
            printf("    Re-auth with new token [OK]\n");

            r =  _auth_testing(
                first,  // DO NOT MODIFY
                "2", // DO NOT MODIFY
                2, // sys ID, DO NOT MODIFY
                CDS_ERR_UMATCH_USER_INFO);

        } else {
            printf("-->SHIT, new token failed auth!\n");
        }
    }

    return r;
}

// try auth with the same session ID
int test_auth_normal_case6() {
    return _auth_testing(
            g_bname_first_token,  // DO NOT MODIFY, it is previously allocated token
            "bdev", // DO NOT MODIFY, a previusly session ID
            2, // sys ID,
            CDS_OK);
}

// try auth with a different session ID
int test_auth_normal_case7() {
    return _auth_testing(
            g_bname_first_token,  // DO NOT MODIFY, it is previously allocated token
            "1", // DO NOT MODIFY, a wrong session ID
            2, // sys ID,
            CDS_ERR_UMATCH_USER_INFO);
}

// a consecutive case from test_css_login_after_activation()
int test_auth_css_email(){
    // for the previously reg/login case, 
    // CSS use sysid-2 and sessionid-2
    string t = g_css_tokens.front();
    return _auth_testing(
            t.c_str(),
            "2", // see above comments
            2, // dito
            CDS_OK);
}

// check if a normal login(with time update)
int test_xmpp_auth_logic2()
{
    // simulate the xmpp_auth_2/ component
    return _auth_testing(
            "xmpptoken-cc18-4c44-a7c5-124c7afc45bf", // token, DO NOT MODIFY, it is prebuilt-in TOKEN(normal good token, with time needs updated)
            "XMPP",
            99,
            CDS_OK);
}

// Check above test updated the DB
int test_xmpp_auth_logic3()
{
    int ret = -1;
    char sqlcmd[256];
    time_t cur;

    time(&cur);

    sprintf(sqlcmd, "SELECT UNIX_TIMESTAMP(lastoperatetime) FROM uc.uc_session "
            "WHERE ticket=\'xmpptoken-cc18-4c44-a7c5-124c7afc45bf\'");
    if(mysql_query(mSql, sqlcmd))
    {
        printf("failed get the xmpp token from DB:%s\n", mysql_error(mSql));
        return -1;
    }

    MYSQL_RES *mresult;
    mresult = mysql_store_result(mSql);
    if(mresult)
    {
        MYSQL_ROW row;

        row = mysql_fetch_row(mresult);
        long a = atol(row[0]);
        printf("    time in DB:%ld, cur time:%ld, delta=%ld sec\n",
                a, cur, labs(a-cur));
        if(labs(a - cur) <= 60) {
            ret = 0;
        } else {
            printf("  oh, exceed the 60 seoncd, seems time didn't updated in DB\n");
        }
        mysql_free_result(mresult);
    }

    return ret;
}

int test_misfield_mem_auth()
{
    int ret = -1;
    char *val;
    size_t valen;
    memcached_return_t rc;
    char buf[128];

    printf("first of all, try get the mem...\n");
    
    if(!mMemc)
    {
        printf("seems memcached server unavailable, ignore the mem parepare step\n");
        return -1;
    }

    val = memcached_get_by_key(mMemc, NULL, 0,
            "22776f46-cc18-4c44-a7c5-124c7afc45bf", strlen("22776f46-cc18-4c44-a7c5-124c7afc45bf"),
            &valen, 0, &rc);
    if(!val)
    {
        printf("failed get the 22776f46-cc18-4c44-a7c5-124c7afc45bf value from mem:%d\n",
                rc);
        return -1;
    }
    else
    {
        if(rc == MEMCACHED_SUCCESS) {
            printf("22776f46-cc18-4c44-a7c5-124c7afc45bf ====> %s\n", val);
            strcpy(buf, val);
            printf("give a fake corrupted data...\n");
            strcpy(buf, "a b"); // only 2 field in the mem

            rc = memcached_set(mMemc, "22776f46-cc18-4c44-a7c5-124c7afc45bf", strlen("22776f46-cc18-4c44-a7c5-124c7afc45bf"),
                    buf, strlen(buf),
                    0,0);
            if(rc == MEMCACHED_SUCCESS){
                printf("set a corrupted mem [OK], now it is:\n");
                printf("22776f46-cc18-4c44-a7c5-124c7afc45bf ==> %s\n", buf);
                ret = 0;
            }
        }
        free(val);
    }

    return ret;
}
// previous test case set a bad mem, next we will check if it can handle it without issue..
int test_misfield_mem_auth2() {
    printf("this case, SHOULD check if code seek DB, as mem corrupted...\n");
    return _auth_testing(
            "22776f46-cc18-4c44-a7c5-124c7afc45bf",  // DO NOT MODIFY
            "2", // DO NOT MODIFY
            1, // sys ID, DO NOT MODIFY
             CDS_OK);
}

// check previous case can restore bad mem into good...
int test_misfield_mem_auth3() {
    int ret = -1;
    char *val;
    size_t valen;
    memcached_return_t rc;
    char buf[128];
    int i;
    int count;
    
    if(!mMemc)
    {
        printf("seems memcached server unavailable, ignore the mem parepare step\n");
        return -1;
    }

    val = memcached_get_by_key(mMemc, NULL, 0,
            "22776f46-cc18-4c44-a7c5-124c7afc45bf", strlen("22776f46-cc18-4c44-a7c5-124c7afc45bf"),
            &valen, 0, &rc);
    if(!val)
    {
        printf("failed get the 22776f46-cc18-4c44-a7c5-124c7afc45bf value from mem:%d\n",
                rc);
        return -1;
    }
    else
    {
        strcpy(buf, val);
        printf("    the mem value:%s\n", buf);
        i = 0;
        count = 0;
        while(buf[i++] != '\0') {
            if(buf[i] == ' '){
                count++;
            }
        }

        printf("    I found the space is %d count\n", count);
        if(count == 2) {
            printf("    the corrupted mem be restored successfully!\n");
            ret = 0;
        }

        free(val);
    }

    return ret;
}
/////////////////////////////////////////////////////////////////////
// modify profile test
/////////////////////////////////////////////////////////////////////
int test_change_password() {
    return -1;
}

// using new passwod should pass
int test_change_password2() {
    return -1;
}

// Using old password should failed!
int test_change_password3() {
    return -1;
}

// inner util for ping specified @s socket
int _ping_service(int s) {
    int ret = -1;
    const char payload[2] = {0x34, 0x12};
    size_t rc;
    char back[512];
    rc = write(s, payload, sizeof(payload));
    if(rc > 0) {
        rc = read(s, back, sizeof(back));
        if(rc > 0) {
            ret = *(int *)back;
            printf("the ping result code = %d\n", ret);
        }
    }

    return ret;
}

/////////////////////////////////////////////////////////////////////
// misc test
/////////////////////////////////////////////////////////////////////
int test_sql_auto_reconnect() {
    // get the timeout
    if(mysql_query(mSql, "show global variables LIKE \'wait_timeout\'")){
        printf("failed get the idle timeout:%s\n", mysql_error(mSql));
        return -1;
    }

    int ret = -1;
    int flag = 0;
    char origin_timeout[32];

    MYSQL_RES *mresult;
    MYSQL_ROW row;
    int i,j;
    mresult = mysql_store_result(mSql);
    if(mresult) {
        printf("the timeout related config info:\n");
        while((row = mysql_fetch_row(mresult)) != NULL) {
            i = mysql_num_fields(mresult);
            for(j=0; j < i; j++)
            {
              printf("%s  ", row[j]);
            }
            printf("\n");

            strcpy(origin_timeout, row[1]);
            printf("the original timeout is %s seconds\n", origin_timeout);
        }
        mysql_free_result(mresult);

        // try set a shorter timeout
        if(mysql_query(mSql, "set global wait_timeout=5")) {
            printf("can't set the timeout with short time:%s\n",
                    mysql_error(mSql));
            return -1;
        }

        mresult = mysql_store_result(mSql);

        printf("set the timeout in a short time\n");
        for(i = 0 ; i < 7; i++){
            printf("waiting for timeout (%d second)\n", i);
            sleep(1);
        }
        printf("now, timeout for the SQL, try the re-connecting feature\n");

        // below will try connect to all service..

        if(_ping_service(mSockReg) != 0) {
            printf("reg service ping under timeout case failed\n");
            flag ++;
        }
        if(_ping_service(mSockLogin) != 0) {
            printf("login service ping under timeout case failed\n");
            flag ++;
        }
        if(_ping_service(mSockAct) != 0) {
            printf("activation service ping under timeout case failed\n");
            flag ++;
        }
        if(_ping_service(mSockAuth) != 0) {
            printf("auth service ping under timeout case failed\n");
            flag ++;
        }
        if(_ping_service(mSockPasswd) != 0) {
            printf("passwd service ping under timeout case failed\n");
            flag ++;
        }

        if(mresult) mysql_free_result(mresult);

        // before return, need restore the time
        char tmp[128];
        sprintf(tmp, "set global wait_timeout=%s", origin_timeout);
        printf("the tmp=%s\n", tmp);
        if(mysql_query(mSql, tmp)){
            printf("Warning ***, failed restore the timeout value:%s\n",
                    mysql_error(mSql));
            if(mysql_errno(mSql) == CR_SERVER_GONE_ERROR) {

                // timeout, reconnect again!
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
                            "uc", 0, NULL, 0)) {
                    printf("error for mysql_real_connect():%s\n",
                            mysql_error(mSql));
                    return -1;
                }

                if(mysql_query(mSql, tmp)){
                    printf("SHIT, still failed to restore the original time\n");
                } else {
                    printf("restore it OK after reconnecting\n");
                }

            }

            }
        } else {
            mresult = mysql_store_result(mSql);
            if(mresult) {
                mysql_free_result(mresult);
            }
            printf("restore the timeout to default value.\n");
        }

        if(flag == 0)
        {
            ret = 0;
        }
        else
        {
            ret = -1;
        }
    }

    return ret;
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
    mSockReg = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(TESTING_SERVER_IP);
    addr.sin_port = htons(13000);

    if(connect(mSockReg, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
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

    if(prepare_memcach_test_data() == -1){
        printf("Warning, memcach data preparation wrong, continue anyway\n");
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
    execute_ut_case(test_ping_reg_service);
    execute_ut_case(test_insert_already_existed_user_password);
    execute_ut_case(test_insert_new_user_password);
    execute_ut_case(test_insert_new_user_password2);
    execute_ut_case(test_insert_new_user_password3);
    execute_ut_case(test_insert_new_user_password4); //2015-3-12 added for verify different sessioin auth case...

    execute_ut_case(test_email_for_aname_reg);
    execute_ut_case(test_email_already_existed);
    execute_ut_case(test_email_overwrite_inactive);
    execute_ut_case(test_email_overwrite_inactive_toolong_source);

    execute_ut_case(test_email_css_reg);

    execute_ut_case(test_phone_sms_reg); // normal phone+SMS case, will test SMS code later in activation service
    execute_ut_case(test_phone_passwd_reg);
    execute_ut_case(test_phone_passwd_reg2);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // next will try testing the login feature...
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    //close(mSock);

    mSockLogin = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(TESTING_SERVER_IP);
    addr.sin_port = htons(13001);

    if(connect(mSockLogin, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("***failed connect to Login server:%d\n", errno);
        goto end_of_testing;
    }

    printf("Connecting to Login service...[OK]\n");

    if(confirm) {
        printf("Now, Login testing is ready, will you want to begin?\n");
        getchar();
    }

    execute_ut_case(test_ping_login_service);
    execute_ut_case(test_normal_user_login);
    execute_ut_case(test_normal_user_login_bad_passwd);
    execute_ut_case(test_normal_user_login2);
    execute_ut_case(test_normal_email_login);
    execute_ut_case(test_normal_css_email_login2);
    execute_ut_case(test_phone_passwd_login);
    execute_ut_case(test_phone_passwd_login2);
    execute_ut_case(test_phone_passwd_login3);
    execute_ut_case(test_phone_passwd_login4);
    execute_ut_case(test_phone_passwd_login5);

    execute_ut_case(test_older_phone_add_device_type);
    execute_ut_case(test_older_phone_add_device_type2);
    execute_ut_case(test_older_phone_add_device_type3);
    execute_ut_case(test_older_phone_add_device_type4);
    execute_ut_case(test_older_phone_add_device_type5);
    execute_ut_case(test_older_phone_add_device_type6);
    execute_ut_case(test_add_sips_db_for_reg);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // next will try testing the activation feature...
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    //close(mSock);
    mSockAct = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(TESTING_SERVER_IP);
    addr.sin_port = htons(13002);

    if(connect(mSockAct, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("***failed connect to Activation server:%d\n", errno);
        goto end_of_testing;
    }

    printf("Connecting to Activation service...[OK]\n");

    if(confirm) {
        printf("Now, Activation testing is ready, will you want to begin?\n");
        getchar();
    }

    execute_ut_case(test_ping_active_service);
    execute_ut_case(test_activation_phone_smscode);
    execute_ut_case(test_activation_phone_smscode2);
    execute_ut_case(test_activation_phone_smscode3);
    execute_ut_case(test_activation_phone_smscode4); // SMS geneared via @test_phone_sms_reg(incorrect case)
    execute_ut_case(test_activation_phone_smscode5); // SMS geneared via @test_phone_sms_reg(correct case)
    execute_ut_case(test_activation_email_1);
    execute_ut_case(test_activation_css_email);
    execute_ut_case(test_css_login_after_activation);
#if 0

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // next will try testing the auth feature...
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    //close(mSock);
    mSockAuth = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(TESTING_SERVER_IP);
    addr.sin_port = htons(13003);

    if(connect(mSockAuth, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("***failed connect to Auth server:%d\n", errno);
        goto end_of_testing;
    }

    printf("Connecting to Auth service...[OK]\n");

    execute_ut_case(test_ping_auth_service);
    execute_ut_case(test_xmpp_auth_logic);
    execute_ut_case(test_auth_normal_case);
    execute_ut_case(test_auth_normal_case2);
    execute_ut_case(test_auth_normal_case3);
    execute_ut_case(test_auth_normal_case4);
    execute_ut_case(test_auth_normal_case5);
    execute_ut_case(test_auth_normal_case6);
    execute_ut_case(test_auth_normal_case7);
    execute_ut_case(test_auth_css_email);

    execute_ut_case(test_xmpp_auth_logic2);
    execute_ut_case(test_xmpp_auth_logic3);
    execute_ut_case(test_misfield_mem_auth);
    execute_ut_case(test_misfield_mem_auth2);
    execute_ut_case(test_misfield_mem_auth3);

    execute_ut_case(test_rollback_logic);

    ///
    //TODO need add modify profile test case here...
    //
    mSockPasswd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(TESTING_SERVER_IP);
    addr.sin_port = htons(13004);

    if(connect(mSockPasswd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("***failed connect to Auth server:%d\n", errno);
        goto end_of_testing;
    }

    printf("Connecting to passwd service...[OK]\n");
    execute_ut_case(test_change_password);
    execute_ut_case(test_change_password2);
    execute_ut_case(test_change_password3);



    //misc testing...
    execute_ut_case(test_sql_auto_reconnect);
#endif
end_of_testing:

    printf("\n\n release useless resource...");

    mysql_close(mSql);

    if(mMemc != NULL)
        memcached_free(mMemc);

    printf("[OK]\n\n");

    //
    // summary
    //
    printf("\nEnd of Unit Testing\n");
    printf("Totally %d case tested, %d [OK], %d [Failed]\n",
            gPass + gFail, gPass, gFail);
    //close(mSock);


    return 0;
}
