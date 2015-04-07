#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/md5.h>

#include "memcached.h"
#include "cds_public.h"

/* Avoid warnings on solaris, where isspace() is an index into an array, and gcc uses signed chars */
#define xisspace(c) isspace((unsigned char)c)

#define PADDING RSA_PKCS1_PADDING

const char *b64priv_key = "-----BEGIN RSA PRIVATE KEY-----\n"
                          "MIICXgIBAAKBgQDtTOwXXKOzeKOKIDHfGWfMKNzijDyS9p7ZxqRiPbAe1WK9MhbQ\n"
                          "jH5wi6mguL7k05z14YFs6ltLkY09EKyY3lxHKooC7k4cltAcV6IyiuO8xAHdTu8b\n"
                          "dUIcFH9UW9g6h8hnJvvvFTEwwjkplYAPADMZBW85Yri5ov96jJuk07VH1QIDAQAB\n"
                          "AoGBAOIZ5Ontjty2GbzGKD6Wilvwo2YIkL7G1VxS9NXWSTVXxBjY3PwgoWjC6gEF\n"
                          "hn2pU2jEUPlh41MBgD2q4YlWdTxT8ALr7EstDDxKl39dd95h2G5ibOoIzUADBfrL\n"
                          "RRhua0akGhQG1C0jaO44cGc6Vnq2L69rBYNhjhOixDBuMYsBAkEA/C/3xrJbny4V\n"
                          "IW6HFhxoGFJdiTq/K8Iz1oPZBDPtA5HDF7GuN3C566qtb7I7/EmRvu6LPZmDyOn7\n"
                          "5m7ovfRxEQJBAPDjVo5h2os1CXaRTFsu9Gc+BOr/f1cppZBceP4BywBR6FR9VbSK\n"
                          "gzy6daXuwRSMxepqBu7OPXspj9Tcz46N6oUCQQCUWzuuPlq8CAYSRs1dapSqSjoQ\n"
                          "cujnuzV3qtTOLiXuhZ95nuNYZg5Z32xsWzQCtNzyr65mLJtkZJH1+6UbqmOhAkEA\n"
                          "i4aidAYp18f4mzy9xXMFDWfW4WbsE0iEJYHqcvYG50CKGaYfJlu0eFBoJJyOKaEA\n"
                          "Yi7XpSrAYb5JGTps3l1FIQJASX65JX5iUsSF4oJhMcnhKmYKe8UtJ9wuMEvMcJyc\n"
                          "XKqOt9zyHRWzQCWMU3Q76pxWPXqisIDTw7F8xJX/U66QDw==\n"
                          "-----END RSA PRIVATE KEY-----\n";

const static char* b64="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;

// maps A=>0,B=>1..
const static unsigned char unb64[]={
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //10
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //20
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //30
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //40
  0,   0,   0,  62,   0,   0,   0,  63,  52,  53, //50
 54,  55,  56,  57,  58,  59,  60,  61,   0,   0, //60
  0,   0,   0,   0,   0,   0,   1,   2,   3,   4, //70
  5,   6,   7,   8,   9,  10,  11,  12,  13,  14, //80
 15,  16,  17,  18,  19,  20,  21,  22,  23,  24, //90
 25,   0,   0,   0,   0,   0,   0,  26,  27,  28, //100
 29,  30,  31,  32,  33,  34,  35,  36,  37,  38, //110
 39,  40,  41,  42,  43,  44,  45,  46,  47,  48, //120
 49,  50,  51,   0,   0,   0,   0,   0,   0,   0, //130
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //140
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //150
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //160
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //170
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //180
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //190
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //200
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //210
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //220
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //230
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //240
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //250
  0,   0,   0,   0,   0,   0,
}; // This array has 255 elements

/* All AES keys */
struct token_aes_key aes_keys[] = {

    {
        "com.caredear.pcare_parent",
        {
        0xB7, 0xc3, 0x55, 0x57, 0x42, 0xd4, 0x74, 0x9b,
        0xb8, 0x5a, 0x3f, 0x4f, 0x41, 0xb8, 0x3d, 0xed
        }
    },

    {
        "com.caredear.pcare_child",
        {
        0x85, 0xc1, 0x75, 0x4e, 0x0a, 0xfd, 0xf4, 0xb4,
        0xf7, 0x00, 0x34, 0xa4, 0x0f, 0x27, 0x28, 0xd0
        }
    },

    {
        "com.caredear.awake",
        {
        0x2F, 0xB2, 0x8F, 0x30, 0xdb, 0x2F, 0x11, 0xd5,
        0xEE, 0x82, 0xdd, 0xf9, 0xC9, 0xEB, 0x35, 0xCE
        }
    },

    {
        "com.caredear.xmpp",
        {
        0x35, 0xb2, 0x22, 0x2c, 0xcb, 0x2e, 0xd2, 0x80,
        0x02, 0x7b, 0xc4, 0xe8, 0x37, 0x8b, 0x57, 0x2f
        }
    }

};


// Converts binary data of length=len to base64 characters.
// Length of the resultant string is stored in flen
// (you must pass pointer flen).
char* base64( const void* binaryData, int len, int *flen )
{
  const unsigned char* bin = (const unsigned char*) binaryData ;
  char* res ;

  int rc = 0 ; // result counter
  int byteNo ; // I need this after the loop

  int modulusLen = len % 3 ;
  int pad = ((modulusLen&1)<<1) + ((modulusLen&2)>>1) ; // 2 gives 1 and 1 gives 2, but 0 gives 0.

  *flen = 4*(len + pad)/3 ;
  res = (char*) malloc( *flen + 1 ) ; // and one for the null
  if( !res )
  {
    puts( "ERROR: base64 could not allocate enough memory." ) ;
    puts( "I must stop because I could not get enough" ) ;
    return 0;
  }

  for( byteNo = 0 ; byteNo <= len-3 ; byteNo+=3 )
  {
    unsigned char BYTE0=bin[byteNo];
    unsigned char BYTE1=bin[byteNo+1];
    unsigned char BYTE2=bin[byteNo+2];
    res[rc++]  = b64[ BYTE0 >> 2 ] ;
    res[rc++]  = b64[ ((0x3&BYTE0)<<4) + (BYTE1 >> 4) ] ;
    res[rc++]  = b64[ ((0x0f&BYTE1)<<2) + (BYTE2>>6) ] ;
    res[rc++]  = b64[ 0x3f&BYTE2 ] ;
  }

  if( pad==2 )
  {
    res[rc++] = b64[ bin[byteNo] >> 2 ] ;
    res[rc++] = b64[ (0x3&bin[byteNo])<<4 ] ;
    res[rc++] = '=';
    res[rc++] = '=';
  }
  else if( pad==1 )
  {
    res[rc++]  = b64[ bin[byteNo] >> 2 ] ;
    res[rc++]  = b64[ ((0x3&bin[byteNo])<<4)   +   (bin[byteNo+1] >> 4) ] ;
    res[rc++]  = b64[ (0x0f&bin[byteNo+1])<<2 ] ;
    res[rc++] = '=';
  }

  res[rc]=0; // NULL TERMINATOR! ;)
  return res ;
}

unsigned char* unbase64( const char* ascii, int len, int *flen )
{
  const unsigned char *safeAsciiPtr = (const unsigned char*)ascii ;
  unsigned char *bin ;
  int cb=0;
  int charNo;
  int pad = 0 ;

  if( len < 2 ) { // 2 accesses below would be OOB.
    // catch empty string, return NULL as result.
    puts( "ERROR: You passed an invalid base64 string (too short). You get NULL back." ) ;
    *flen=0;
    return 0 ;
  }
  if( safeAsciiPtr[ len-1 ]=='=' )  ++pad ;
  if( safeAsciiPtr[ len-2 ]=='=' )  ++pad ;

  *flen = 3*len/4 - pad ;
  bin = (unsigned char*)malloc( *flen ) ;
  if( !bin )
  {
    puts( "ERROR: unbase64 could not allocate enough memory." ) ;
    puts( "I must stop because I could not get enough" ) ;
    return 0;
  }

  for( charNo=0; charNo <= len - 4 - pad ; charNo+=4 )
  {
    int A=unb64[safeAsciiPtr[charNo]];
    int B=unb64[safeAsciiPtr[charNo+1]];
    int C=unb64[safeAsciiPtr[charNo+2]];
    int D=unb64[safeAsciiPtr[charNo+3]];

    bin[cb++] = (A<<2) | (B>>4) ;
    bin[cb++] = (B<<4) | (C>>2) ;
    bin[cb++] = (C<<6) | (D) ;
  }

  if( pad==1 )
  {
    int A=unb64[safeAsciiPtr[charNo]];
    int B=unb64[safeAsciiPtr[charNo+1]];
    int C=unb64[safeAsciiPtr[charNo+2]];

    bin[cb++] = (A<<2) | (B>>4) ;
    bin[cb++] = (B<<4) | (C>>2) ;
  }
  else if( pad==2 )
  {
    int A=unb64[safeAsciiPtr[charNo]];
    int B=unb64[safeAsciiPtr[charNo+1]];

    bin[cb++] = (A<<2) | (B>>4) ;
  }

  return bin ;
}
unsigned char* rsaDecrypt( RSA *privKey, const unsigned char* encryptedData, int *resultLen )
{
    int rsaLen = RSA_size( privKey ) ; // That's how many bytes the decrypted data would be

    unsigned char *decryptedBin = (unsigned char*)malloc( rsaLen ) ;
    *resultLen = RSA_private_decrypt( RSA_size(privKey), encryptedData, decryptedBin, privKey, PADDING ) ;
    if( *resultLen == -1 )
        ERR( "ERROR: RSA_private_decrypt: %s\n", ERR_error_string(ERR_get_error(), NULL) ) ;

    return decryptedBin ;
}

RSA* loadPRIVATEKeyFromString( const char* privateKeyStr )
{
    BIO *bio = BIO_new_mem_buf( (void*)privateKeyStr, -1 );
    RSA* rsaPrivKey = PEM_read_bio_RSAPrivateKey( bio, NULL, NULL, NULL ) ;

    if ( !rsaPrivKey )
        ERR("ERROR: Could not load PRIVATE KEY!  PEM_read_bio_RSAPrivateKey FAILED: %s\n", ERR_error_string(ERR_get_error(), NULL));

    BIO_free( bio ) ;
    return rsaPrivKey ;
}

unsigned char* rsaDecryptThisBase64( RSA *privKey, char* base64String, int *outLen )
{
    int encBinLen ;
    unsigned char* encBin = unbase64( base64String, (int)strlen( base64String ), &encBinLen ) ;

    unsigned char *decryptedBin = rsaDecrypt( privKey, encBin, outLen ) ;
    free( encBin ) ;

    return decryptedBin ;
}

/**
 *
 */
void aes_encrypt(unsigned char* in, int inl, unsigned char *out, int* len, unsigned char * key)
{
    unsigned char iv[8];
    EVP_CIPHER_CTX ctx;

    EVP_CIPHER_CTX_init(&ctx);

    EVP_EncryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, key, iv);

    *len = 0;
    int outl = 0;

    EVP_EncryptUpdate(&ctx, out+*len, &outl, in+*len, inl);
    *len+=outl;

    int test = inl>>4;
    if(inl != test<<4){
        EVP_EncryptFinal_ex(&ctx,out+*len,&outl);
        *len+=outl;
    }
    EVP_CIPHER_CTX_cleanup(&ctx);
}

/**
 * match an appid to corresponding aes key
 */
char *match_aes_key(const char *appid)
{
    char *key = NULL;
    int   i;
    for(i = 0; i < (int)(sizeof(aes_keys)/sizeof(struct token_aes_key)); i++)
    {
        if(!strcmp(appid, aes_keys[i].tak_appid))
        {
            key = (char *)aes_keys[i].tak_aeskey;
            break;
        }
    }

    return key;
}
/**
 *
 *
 */
void aes_decrypt(unsigned char* in, int inl, unsigned char *out, unsigned char *key)
{
    unsigned char iv[8];
    EVP_CIPHER_CTX ctx;

    EVP_CIPHER_CTX_init(&ctx);

    EVP_DecryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, key, iv);
    int len = 0;
    int outl = 0;

    EVP_DecryptUpdate(&ctx, out+len, &outl, in+len, inl);
    len += outl;

    EVP_DecryptFinal_ex(&ctx, out+len, &outl);
    len+=outl;
    out[len]=0;
    EVP_CIPHER_CTX_cleanup(&ctx);
}

/*
 *public util entry for RSA decryption
 *
 * NULL if there's any wrong
 */
char *decrypt_token_string(const char *token_str)
{
    RSA *privKey = loadPRIVATEKeyFromString(b64priv_key);
    int rBinLen ;

    unsigned char* rBin = rsaDecryptThisBase64( privKey, (char *)token_str, &rBinLen ) ;

    if(rBinLen == -1)
    {
        /* failure case, we need return NULL */
        free(rBin);
        rBin = NULL;
    }
    else
    {
        LOG("Decrypted %d bytes, the recovered data is:\n%s\n\n", rBinLen, rBin ) ;
    }

    RSA_free(privKey);

    //DON'T free here.
    //free(rBin);

    // FIXME caller take responsiblity to free the alocated data!
    return (char *)rBin;
}

/*
 *public util entry, using AES crypt/decrypt
 *
 *@token_str : the encrypted string text data.
 *
 * NULL if there's any wrong, otherwise return the decrypted data
 */
char *decrypt_token_string_with_aes(const char *token_str, char *aes_key)
{
    unsigned char  base64_out[400];
    int   tok_len = strlen(token_str);

    memset(base64_out, 0x00, sizeof(base64_out));

    int   length = EVP_DecodeBlock(base64_out, (const unsigned char *)token_str, tok_len);

    while(token_str[--tok_len] == '=') length--;

    LOG("the EVP length = %d\n", length);

    char *result = (char *)malloc(400);
    if(result != NULL)
    {
        LOG("a22301->malloced..");
        memset(result, 0x00, 400);
        aes_decrypt(base64_out, length, (unsigned char *)result, (unsigned char *)aes_key);
        LOG("<--decrypt completed:%s\n", result);
        if(strlen(result) <= 0)
        {
            free(result);
            result = NULL;
        }
    }

    // FIXME caller take responsiblity to free the alocated data!
    return result;
}

/**
 * Encrypt the plain text into a base64 text data
 *
 *@plain_text: plain text data
 *@aes_key : the AES key to do the encryption
 *
 * return the base64 string ecrypted text, or NULL if there's sth wrong.
 */
char *encrypt_token_string_with_aes(const char *plain_text, char *aes_key)
{
    unsigned char iv[8];
    char out[400];
    int  outlen = 0;
    int  padding = 0;
    int  len = 0;
    int  plain_len = strlen(plain_text);
    char *result;
    EVP_CIPHER_CTX ctx;

    LOG("before, len=%d\n", len);
    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, (unsigned char *)aes_key, iv);

    EVP_EncryptUpdate(&ctx, (unsigned char *)out + len, &outlen,
            (unsigned char *)plain_text + len, plain_len);
    len += outlen;

    padding = plain_len;
    if(plain_len != padding << 4)
    {
        EVP_EncryptFinal_ex(&ctx, (unsigned char *)out + len, &outlen);
        len += outlen;
    }
    EVP_CIPHER_CTX_cleanup(&ctx);

    LOG("Now, len=%d\n", len);

    result = base64(out, len, &outlen/* re-use the varirable*/);

    /* if code come here, the whole encryption steps are like this:

       plain_text
         |
         +-------->out (binary, with len)
                    |
                    +----------> result(base64 text encrypted string)

     */

    // FIXME caller take responsiblity to free the alocated data!
    return result;
}
bool safe_strtoull(const char *str, uint64_t *out) {
    assert(out != NULL);
    errno = 0;
    *out = 0;
    char *endptr;
    unsigned long long ull = strtoull(str, &endptr, 10);
    if ((errno == ERANGE) || (str == endptr)) {
        return false;
    }

    if (xisspace(*endptr) || (*endptr == '\0' && endptr != str)) {
        if ((long long) ull < 0) {
            /* only check for negative signs in the uncommon case when
             * the unsigned number is so big that it's negative as a
             * signed number. */
            if (strchr(str, '-') != NULL) {
                return false;
            }
        }
        *out = ull;
        return true;
    }
    return false;
}

bool safe_strtoll(const char *str, int64_t *out) {
    assert(out != NULL);
    errno = 0;
    *out = 0;
    char *endptr;
    long long ll = strtoll(str, &endptr, 10);
    if ((errno == ERANGE) || (str == endptr)) {
        return false;
    }

    if (xisspace(*endptr) || (*endptr == '\0' && endptr != str)) {
        *out = ll;
        return true;
    }
    return false;
}

bool safe_strtoul(const char *str, uint32_t *out) {
    char *endptr = NULL;
    unsigned long l = 0;
    assert(out);
    assert(str);
    *out = 0;
    errno = 0;

    l = strtoul(str, &endptr, 10);
    if ((errno == ERANGE) || (str == endptr)) {
        return false;
    }

    if (xisspace(*endptr) || (*endptr == '\0' && endptr != str)) {
        if ((long) l < 0) {
            /* only check for negative signs in the uncommon case when
             * the unsigned number is so big that it's negative as a
             * signed number. */
            if (strchr(str, '-') != NULL) {
                return false;
            }
        }
        *out = l;
        return true;
    }

    return false;
}

bool safe_strtol(const char *str, int32_t *out) {
    assert(out != NULL);
    errno = 0;
    *out = 0;
    char *endptr;
    long l = strtol(str, &endptr, 10);
    if ((errno == ERANGE) || (str == endptr)) {
        return false;
    }

    if (xisspace(*endptr) || (*endptr == '\0' && endptr != str)) {
        *out = l;
        return true;
    }
    return false;
}

void vperror(const char *fmt, ...) {
    int old_errno = errno;
    char buf[1024];
    va_list ap;

    va_start(ap, fmt);
    if (vsnprintf(buf, sizeof(buf), fmt, ap) == -1) {
        buf[sizeof(buf) - 1] = '\0';
    }
    va_end(ap);

    errno = old_errno;

    perror(buf);
}

/**
 * Calculate the MD5 on @data, which is @length.
 *
 */
int get_md5(const char *data, int length, char *result)
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


/**
 * Get date time format string, like 'yyyy-mm-dd hh:mm:ss'
 *
 * return 0 for successful, otherwise return -1
 */
int current_datetime(char *stored_data, size_t size_data)
{
    time_t cur;
    time(&cur);

    /* strftime() return 0 for error */
    return (strftime(stored_data, size_data, "%Y-%m-%d %H:%M:%S",
                localtime(&cur)) == 0 ? -1 : 0);
}

#ifndef HAVE_HTONLL
static uint64_t mc_swap64(uint64_t in) {
#ifdef ENDIAN_LITTLE
    /* Little endian, flip the bytes around until someone makes a faster/better
    * way to do this. */
    int64_t rv = 0;
    int i = 0;
     for(i = 0; i<8; i++) {
        rv = (rv << 8) | (in & 0xff);
        in >>= 8;
     }
    return rv;
#else
    /* big-endian machines don't need byte swapping */
    return in;
#endif
}

uint64_t ntohll(uint64_t val) {
   return mc_swap64(val);
}

uint64_t htonll(uint64_t val) {
   return mc_swap64(val);
}
#endif

