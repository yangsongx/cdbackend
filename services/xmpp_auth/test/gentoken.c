#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/pem.h>

// I'm not using BIO for base64 encoding/decoding.  It is difficult to use.
// Using superwills' Nibble And A Half instead
// https://github.com/superwills/NibbleAndAHalf/blob/master/NibbleAndAHalf/base64.h
#include "base64.h"

typedef int bool;
enum {false =0, true = 1};
// The PADDING parameter means RSA will pad your data for you
// if it is not exactly the right size
//#define PADDING RSA_PKCS1_OAEP_PADDING
#define PADDING RSA_PKCS1_PADDING
//#define PADDING RSA_NO_PADDING

RSA* loadPUBLICKeyFromString( const char* publicKeyStr )
{
    // A BIO is an I/O abstraction (Byte I/O?)

    // BIO_new_mem_buf: Create a read-only bio buf with data
    // in string passed. -1 means string is null terminated,
    // so BIO_new_mem_buf can find the dataLen itself.
    // Since BIO_new_mem_buf will be READ ONLY, it's fine that publicKeyStr is const.
    BIO* bio = BIO_new_mem_buf( (void*)publicKeyStr, -1 ) ; // -1: assume string is null terminated

    BIO_set_flags( bio, BIO_FLAGS_BASE64_NO_NL ) ; // NO NL

    // Load the RSA key from the BIO
    RSA* rsaPubKey = PEM_read_bio_RSA_PUBKEY( bio, NULL, NULL, NULL ) ;
    if( !rsaPubKey )
        printf( "ERROR: Could not load PUBLIC KEY!  PEM_read_bio_RSA_PUBKEY FAILED: %s\n", ERR_error_string( ERR_get_error(), NULL ) ) ;

    BIO_free( bio ) ;
    return rsaPubKey ;
}

RSA* loadPRIVATEKeyFromString( const char* privateKeyStr )
{
    BIO *bio = BIO_new_mem_buf( (void*)privateKeyStr, -1 );
    //BIO_set_flags( bio, BIO_FLAGS_BASE64_NO_NL ) ; // NO NL
    RSA* rsaPrivKey = PEM_read_bio_RSAPrivateKey( bio, NULL, NULL, NULL ) ;

    if ( !rsaPrivKey )
        printf("ERROR: Could not load PRIVATE KEY!  PEM_read_bio_RSAPrivateKey FAILED: %s\n", ERR_error_string(ERR_get_error(), NULL));

    BIO_free( bio ) ;
    return rsaPrivKey ;
}

unsigned char* rsaEncrypt( RSA *pubKey, const unsigned char* str, int dataSize, int *resultLen )
{
    int rsaLen = RSA_size( pubKey ) ;
    unsigned char* ed = (unsigned char*)malloc( rsaLen ) ;

    // RSA_public_encrypt() returns the size of the encrypted data
    // (i.e., RSA_size(rsa)). RSA_private_decrypt()
    // returns the size of the recovered plaintext.
    *resultLen = RSA_public_encrypt( dataSize, (const unsigned char*)str, ed, pubKey, PADDING ) ;
    if( *resultLen == -1 )
        printf("ERROR: RSA_public_encrypt: %s\n", ERR_error_string(ERR_get_error(), NULL));

    return ed ;
}

unsigned char* rsaDecrypt( RSA *privKey, const unsigned char* encryptedData, int *resultLen )
{
    int rsaLen = RSA_size( privKey ) ; // That's how many bytes the decrypted data would be

    unsigned char *decryptedBin = (unsigned char*)malloc( rsaLen ) ;
    *resultLen = RSA_private_decrypt( RSA_size(privKey), encryptedData, decryptedBin, privKey, PADDING ) ;
    if( *resultLen == -1 )
        printf( "ERROR: RSA_private_decrypt: %s\n", ERR_error_string(ERR_get_error(), NULL) ) ;

    return decryptedBin ;
}

unsigned char* makeAlphaString( int dataSize )
{
    unsigned char* s = (unsigned char*) malloc( dataSize ) ;

    int i;
    for( i = 0 ; i < dataSize ; i++ )
        s[i] = 65 + i ;
    s[i-1]=0;//NULL TERMINATOR ;)

    return s ;
}

// You may need to encrypt several blocks of binary data (each has a maximum size
// limited by pubKey).  You shoudn't try to encrypt more than
// RSA_LEN( pubKey ) bytes into some packet.
// returns base64( rsa encrypt( <<binary data>> ) )
// base64OfRsaEncrypted()
// base64StringOfRSAEncrypted
// rsaEncryptThenBase64
char* rsaEncryptThenBase64( RSA *pubKey, unsigned char* binaryData, int binaryDataLen, int *outLen )
{
    int encryptedDataLen ;

    // RSA encryption with public key
    unsigned char* encrypted = rsaEncrypt( pubKey, binaryData, binaryDataLen, &encryptedDataLen ) ;

    // To base 64
    int asciiBase64EncLen ;
    char* asciiBase64Enc = base64( encrypted, encryptedDataLen, &asciiBase64EncLen ) ;

    // Destroy the encrypted data (we are using the base64 version of it)
    free( encrypted ) ;

    // Return the base64 version of the encrypted data
    return asciiBase64Enc ;
}

// rsaDecryptOfUnbase64()
// rsaDecryptBase64String()
// unbase64ThenRSADecrypt()
// rsaDecryptThisBase64()
unsigned char* rsaDecryptThisBase64( RSA *privKey, char* base64String, int *outLen )
{
    int encBinLen ;
    unsigned char* encBin = unbase64( base64String, (int)strlen( base64String ), &encBinLen ) ;

    // rsaDecrypt assumes length of encBin based on privKey
    unsigned char *decryptedBin = rsaDecrypt( privKey, encBin, outLen ) ;
    free( encBin ) ;

    return decryptedBin ;
}



int main( int argc, const char* argv[] )
{

    if (argc != 2)
        exit(1);

    ERR_load_crypto_strings();

    puts( "We are going to: rsa_decrypt( unbase64( base64( rsa_encrypt( <<binary data>> ) ) ) )" );
    // public key
    // http://srdevspot.blogspot.ca/2011/08/openssl-error0906d064pem.html
    //1. The file must contain:
    //-----BEGIN CERTIFICATE-----
    //on a separate line (i.e. it must be terminated with a newline).
    //2. Each line of "gibberish" must be 64 characters wide.
    //3. The file must end with:
    //-----END CERTIFICATE-----
    // YOUR PUBLIC KEY MUST CONTAIN NEWLINES.  If it doesn't (ie if you generated it with
    // something like
    // ssh-keygen -t rsa -C "you@example.com"
    // ) THEN YOU MUST INSERT NEWLINES EVERY 64 CHRS (just line it up with how I have it here
    // or with how the ssh-keygen private key is formatted by default)
#if 0
    const char *b64_pKey = "-----BEGIN PUBLIC KEY-----\n"
                           "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCp2w+8HUdECo8V5yuKYrWJmUbL\n"
                           "tD6nSyVifN543axXvNSFzQfWNOGVkMsCo6W4hpl5eHv1p9Hqdcf/ZYQDWCK726u6\n"
                           "hsZA81AblAOOXKaUaxvFC+ZKRJf+MtUGnv0v7CrGoblm1mMC/OQI1JfSsYi68Epn\n"
                           "aOLepTZw+GLTnusQgwIDAQAB\n"
                           "-----END PUBLIC KEY-----\n";
#endif

    const char *b64_pKey = "-----BEGIN PUBLIC KEY-----\n"
                           "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDtTOwXXKOzeKOKIDHfGWfMKNzi\n"
                           "jDyS9p7ZxqRiPbAe1WK9MhbQjH5wi6mguL7k05z14YFs6ltLkY09EKyY3lxHKooC\n"
                           "7k4cltAcV6IyiuO8xAHdTu8bdUIcFH9UW9g6h8hnJvvvFTEwwjkplYAPADMZBW85\n"
                           "Yri5ov96jJuk07VH1QIDAQAB\n"
                           "-----END PUBLIC KEY-----\n";


    // private key
#if 0
    const char *b64priv_key = "-----BEGIN RSA PRIVATE KEY-----\n"
                              "MIICXAIBAAKBgQCp2w+8HUdECo8V5yuKYrWJmUbLtD6nSyVifN543axXvNSFzQfW\n"
                              "NOGVkMsCo6W4hpl5eHv1p9Hqdcf/ZYQDWCK726u6hsZA81AblAOOXKaUaxvFC+ZK\n"
                              "RJf+MtUGnv0v7CrGoblm1mMC/OQI1JfSsYi68EpnaOLepTZw+GLTnusQgwIDAQAB\n"
                              "AoGBAKDuq3PikblH/9YS11AgwjwC++7ZcltzeZJdGTSPY1El2n6Dip9ML0hUjeSM\n"
                              "ROIWtac/nsNcJCnvOnUjK/c3NIAaGJcfRPiH/S0Ga6ROiDfFj2UXAmk/v4wRRUzr\n"
                              "5lsA0jgEt5qcq2Xr/JPQVGB4wUgL/yQK0dDhW0EdrJ707e3BAkEA1aIHbmcVfCP8\n"
                              "Y/uWuK0lvWxrIWfR5MlHhI8tD9lvkot2kyXiV+jB6/gktwk1QaFsy7dCXn7w03+k\n"
                              "xrjEGGN+kQJBAMuKf55lDtU9K2Js3YSStTZAXP+Hz7XpoLxmbWFyGvBx806WjgAD\n"
                              "624irwS+0tBxkERbRcisfb2cXmAx8earT9MCQDZuVCpjBWxd1t66qYpgQ29iAmG+\n"
                              "jBIY3qn9uOOC6RSTiCCx1FvFqDMxRFmGdRVFxeyZwsVE3qNksF0Zko0MPKECQCEe\n"
                              "oDV97DP2iCCz5je0R5hUUM2jo8DOC0GcyR+aGZgWcqjPBrwp5x08t43mHxeb4wW8\n"
                              "dFZ6+trnntO4TMxkA9ECQB+yCPgO1zisJWYuD46KISoesYhwHe5C1BQElQgi9bio\n"
                              "U39fFo88w1pok23a2CZBEXguSvCvexeB68OggdDXvy0=\n"
                              "-----END RSA PRIVATE KEY-----\n";
#endif

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

    // String to encrypt, INCLUDING NULL TERMINATOR:
    int dataSize=100 ; // 128 for NO PADDING, __ANY SIZE UNDER 128 B__ for RSA_PKCS1_PADDING
    //unsigned char *str = makeAlphaString( dataSize ) ;
    //unsigned char *str = "13776670733#1396962608844#com.caredear.awake#null#null#null";
    unsigned char *str =   argv[1];

    printf( "\nThe original data is:\n%s\n\n", (char*)str ) ;

    // LOAD PUBLIC KEY
    RSA *pubKey = loadPUBLICKeyFromString( b64_pKey ) ;

    int asciiB64ELen ;
    //char* asciiB64E = rsaEncryptThenBase64( pubKey, str, dataSize, &asciiB64ELen ) ;
    char* asciiB64E = rsaEncryptThenBase64( pubKey, str, strlen(str),  &asciiB64ELen ) ;

    RSA_free( pubKey ) ; // free the public key when you are done all your encryption

    printf( "Sending base64_encoded ( rsa_encrypted ( <<binary data>> ) ):\n%s\n", asciiB64E ) ;
    puts( "<<----------------  SENDING DATA ACROSS INTERWEBS  ---------------->>" ) ;

    char* rxOverHTTP = asciiB64E ; // Simulate Internet connection by a pointer reference
    printf( "\nRECEIVED some base64 string:\n%s\n", rxOverHTTP ) ;
    puts( "\n * * * What could it be?" ) ;

    // Now decrypt this very string with the private key
    RSA *privKey = loadPRIVATEKeyFromString( b64priv_key ) ;

    // Now we got the data at the server.  Time to decrypt it.
    int rBinLen = 1024;
    unsigned char* rBin = rsaDecryptThisBase64( privKey, rxOverHTTP, &rBinLen ) ;

    //char shit[] ="q3N+AXgvGGvdZkpPGXkrEyrcyFU8LahTDntXTptahFvyZLjvhfkCa+PmhFjE8R84ks9EjSbj+lmwgdysxVj8HpLd95ZvLK59PkP1LpvSBAekINsGlSBBF27Molu2QI3foPwlDJ09LbZ/413aDdGme3YtlfydIGItxsTq/zlsYLs=";

   // unsigned char* rBin = rsaDecryptThisBase64( privKey, shit,  &rBinLen ) ;

    printf("Decrypted %d bytes, the recovered data is:\n%.*s\n\n", rBinLen, rBinLen, rBin ) ; // rBin is not necessarily NULL
    // terminated, so we only print rBinLen chrs

    RSA_free(privKey) ;

    bool allEq = true ;
    for( int i = 0 ; i < dataSize ; i++ )
        allEq &= (str[i] == rBin[i]) ;

    if( allEq ) puts( "DATA TRANSFERRED INTACT!" ) ;
    else puts( "ERROR, recovered binary does not match sent binary" ) ;
    //free( str ) ;
    free( asciiB64E ) ; // rxOverHTTP
    free( rBin ) ;
    ERR_free_strings();
}
