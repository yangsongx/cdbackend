/**
 * for peer detect(IP address) under a WAN
 *
 */

#include "peer_detect.h"

#ifdef _JNISO
#include <jni.h>
#include "com_caredear_detecter_MainActivity.h"
#endif

static void usage()
{
    PD_LOG("-s : server side(broadcasting for his IP)\n");
    PD_LOG("-c : clinet side(checking target IP)\n");
    PD_LOG("-h : output this text\n");
    PD_LOG("-p 123 : speicfiy port (default is %d)\n", CONTROLLER_PORT);
}

int glb_port = CONTROLLER_PORT;


#ifdef _JNISO
/* JNI implementation, mainly for Android APK */
JNIEXPORT void JNICALL Java_com_caredear_detecter_MainActivity_startServer(JNIEnv *env, jobject obj, jint port)
{
    PD_ERR("Java --> C to setup server with port %d\n", port);

    prepare_ssdp_server(port);
}

JNIEXPORT jstring JNICALL Java_com_caredear_detecter_MainActivity_startClient (JNIEnv *env, jobject obj, jint port)
{
    char ip[32];
    jstring ret;

    ip[0] = '\0';

    detect_dev_ssdp_quick(port, ip);

    if(strlen(ip) > 1)
    {
        ret = (*env)->NewStringUTF(env, ip);
    }
    else
    {
        ret = (*env)->NewStringUTF(env, "unknow");
    }

    return ret;
}

#else
int main(int argc, char **argv)
{
    int c;
    int running_mode = 0; /* 0 - for server boradcasing, 1 - for client */

    if(argc < 2){
        PD_ERR("peer_detect -s/-c\n");
        exit(0);
    }

    while((c = getopt(argc, argv, "p:hsc")) != -1) {
        switch(c) {
        case 'p':
            glb_port = atoi(optarg);
            break;

        case 's':
            running_mode = 0;
            break;

        case 'c':
            running_mode = 1;
            break;

        case 'h':
            usage();
            exit(1);

        default:
            break;
        }
    }

    if(running_mode == 0)
    {
        PD_LOG("starting server...\n");
        setup_server(METHOD_SSDP);
    }
    else if(running_mode == 1)
    {
        PD_LOG("starting client(scan available devices...)...\n");
        scan_all_available_devices(METHOD_SSDP);
    }

    PD_LOG("exit the program\n");
    return 0;
}
#endif
