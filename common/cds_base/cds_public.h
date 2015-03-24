#ifndef _CDS_PUBLIC_H
#define _CDS_PUBLIC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

/* max SQL command length */
#define CDS_MAX_SQL  512

/* the CDS max read/write buffer won't exceed 1k */
#define DATA_BUFFER_SIZE 1024

/* if leading-length is 0x1234, we consider the requets
   as a ping alive package, and will fall back to the
   possible ping handler */
#define MAGIC_PINGALIVE  0x1234

/* a testing magic uid */
#define MAGIC_UID   "13911111111"


/* Error Code if sth wrong when handle client's request */
enum {
    CDS_OK = 0,
    CDS_ERR_REQ_TOOLONG = 1,
    CDS_ERR_NOMEMORY,
    CDS_ERR_REQ_INVALID,
    CDS_ERR_UMATCH_USER_INFO,
    CDS_ERR_USER_TOKEN_EXPIRED,
    CDS_ERR_SQL_DISCONNECTED,
    CDS_ERR_SQL_EXECUTE_FAILED,
    CDS_ERR_SQL_NORECORD_FOUND,
    CDS_ERR_NO_RESOURCE,
    CDS_FILE_ALREADY_EXISTED,        /**< 10 - file already existed in netdisk */

    CDS_ERR_EXCEED_QUOTA,
    CDS_ERR_FILE_NOTFOUND,
    CDS_ERR_LIBC_FAILURE,
    CDS_ERR_USER_ALREADY_EXISTED,
    CDS_ERR_REQ_PROTOBUF_INCORRECT, /**< 15 - failed parse incoming protobuf data */
    CDS_ERR_INCORRECT_CODE,
    CDS_ERR_CODE_EXPIRED,   /**< verify code is expired */
    CDS_ERR_INACTIVATED,    /**< For Email/Phone + Password case */
    CDS_ERR_REJECT_LOGIN,   /**< not allow user to login/auth */

    CDS_GENERIC_ERROR
};

enum {
    LEN_TYPE_BIN = 0,
    LEN_TYPE_ASCII
};

/* All CDS components' ID, this is used in PING ALIVE handler function,
 * so ping caller knew the target be ping-ed by him */
enum {
    CDS_USR_REG = 0,
    CDS_USR_LOGIN,
    CDS_USR_ACTIVATION,
    CDS_USR_AUTH,
    CDS_USR_PASSWD,
};


typedef int (*cb_func)(int size, void *req, int *len, void *resp);

/**
 * Config that pass to service framework, this aims to be
 * a config with large data block, not like getopt()'s option
 * passed by argc/argv.
 */
struct addition_config
{
    const char *ac_cfgfile;  /**< we plan use XML as the cfg file */
    cb_func     ac_handler;  /**< Caller provided the handler file */
    cb_func     ping_handler; /**< Handler for PING alive request */
	int         ac_lentype;  /**< LEN_TYPE_BIN/LEN_TYPE_ASCII */
};

/**
 * This SQL server's info is configed in an XML file,
 * which is spacified by struct addition_config::ac_cfgfile
 *
 */
struct sql_server_info{
    char  ssi_server_ip[32];  /**< The MySQL server's IP address */
    int   ssi_server_port;    /**< MySQL server's port(0 for default) */
    char  ssi_user_name[32];
    char  ssi_user_password[32];
    char  ssi_database[32];
};

extern struct sql_server_info server_cfg;

/* this is help program log into a file */
extern FILE *_log_file;

extern char *convert_err_to_str(int errcode);
extern int parse_config_file(const char *cfg_name, struct sql_server_info *info);
extern int get_node_via_xpath(const char *xpath, xmlXPathContextPtr ctx, char *result, int result_size);
extern pthread_mutex_t sql_mutex;
extern int cds_init(struct addition_config *adcfg, int argc, char **argv);

extern char *match_aes_key(const char *appid);
extern char *decrypt_token_string(const char *token_str);
extern char *decrypt_token_string_with_aes(const char *token_str, char *aes_key);
extern char *encrypt_token_string_with_aes(const char *token_str, char *aes_key);

extern int get_md5(const char *data, int length, char *result);
extern int current_datetime(char *stored_data, size_t size_data);

#define MAKEWORD(a,b) ((unsigned short) (((unsigned char) (a)) | ((unsigned short) ((unsigned char) (b))) << 8))

/* TODO below 2 SQL-specific macro should NOT used/put here,
   will obsolted them soon, all SQL-spcific stuff should put
   in a separate place! */
#define LOCK_SQL   do{pthread_mutex_lock(&sql_mutex);}while(0)
#define UNLOCK_SQL do{pthread_mutex_unlock(&sql_mutex);}while(0)

/* A wrapper macro for IPC lock/unlock */
#define LOCK_CDS(x)   do{pthread_mutex_lock(&(x));}while(0)
#define UNLOCK_CDS(x) do{pthread_mutex_unlock(&(x));}while(0)


#ifdef DEBUG
#define LOG(fmt, args...)  do { \
                             if(_log_file) { \
                                 fprintf(_log_file, fmt, ##args); \
                             } else { \
                                 printf(fmt, ##args); \
                             } \
                           } while(0)

#else
#define LOG(fmt, args...)  do{}while(0)
#endif

#define ERR(fmt, args...)  do { \
                             if(_log_file) { \
                                 fprintf(_log_file, fmt, ##args); \
                             } else { \
                                 printf(fmt, ##args); \
                             } \
                           } while(0)

#define INFO(fmt, args...)  do { \
                             if(_log_file) { \
                                 fprintf(_log_file, fmt, ##args); \
                             } else { \
                                 printf(fmt, ##args); \
                             } \
                           } while(0)

#define KPI(fmt, args...)  do { \
                             char _buf[20]; \
                             struct timeval _tv; \
                             gettimeofday(&_tv, NULL); \
                             time_t _n = time(NULL); \
                             strftime(_buf, sizeof(_buf), "%m-%d %H:%M:%S", localtime(&_n)); \
                             INFO("[%s", _buf); \
                             INFO(".%03ld] ", (_tv.tv_usec/1000)); \
                             INFO(fmt, ##args); \
                           }while(0)



#ifdef __cplusplus
}
#endif

#endif
