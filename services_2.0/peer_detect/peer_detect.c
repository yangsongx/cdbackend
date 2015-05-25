/**
 * for peer detect(IP address) under a WAN
 *
 *\page APK Calling Sample Code
 *
 *\code
 *  int port = 2121;
 *\endcode
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
    PD_LOG("\n");
    /* FIXME - below flag NOT supported yet! */
    PD_LOG("-t : temply pause the broadcasting\n");
    PD_LOG("-r : resume the broadcasting\n");
    PD_LOG("-q : quit the whole boradcasting and exit the program\n");
}

int glb_port = CONTROLLER_PORT;
sem_t sem_trigger;
sem_t sem_stopclient;

#ifdef _JNISO

static int static_recv_broadcast_did = 0;

// JNI need query the info long time, put boradcasting checking into a thread.
void *recv_broadcast_thd(void *param)
{
    int p = *(int *) param;

    PD_ERR("in background thread for port %d \n", p);
    if(sem_init(&sem_stopclient, 1, 0) != 0)
    {
        PD_ERR("Warning, failed init unamed sem(%d)\n", errno);
    }

#if 0
    char tmp[32];
    detect_dev_ssdp_quick(p, tmp);
    PD_ERR("tmp=%s\n", tmp);
#else
    detect_available_devices_ssdp(p);
#endif

    return NULL;
}

/* JNI implementation, mainly for Android APK */
JNIEXPORT void JNICALL Java_com_caredear_detecter_MainActivity_startServer(JNIEnv *env, jobject obj, jint port)
{
    PD_ERR("Java --> C to setup server with port %d\n", port);

    prepare_ssdp_server(port);
}

JNIEXPORT jstring JNICALL Java_com_caredear_detecter_MainActivity_fetchTargetDev(JNIEnv *env, jobject obj, jint port)
{
    jstring ret;

    char fulllist[256]; /* FIXME - hope this is enough for currently scenario */
    char buf[100];
    struct listnode *pos;
    available_dev_t *elem;
    pthread_t tid;

    PD_ERR("entering the C++ world...(flag=%d)\n", static_recv_broadcast_did);

#if 1
    detect_dev_ssdp_quick(port, fulllist);
    if(strlen(fulllist) > 1 && strlen(fulllist) < sizeof(fulllist))
    {
        ret = (*env)->NewStringUTF(env, fulllist);
    }
    else
    {
        ret = (*env)->NewStringUTF(env, "NOT FOUND YET");
    }

#else
    if(static_recv_broadcast_did == 0)
    {
        /* first of all, we need let us can receive server's boradcast... */

        pthread_create(&tid, NULL, recv_broadcast_thd, &port);
        static_recv_broadcast_did = 1;
        sleep(5);
    }

    fulllist[0] = '\0';

    if(list_empty(&dev_list))
    {
        PD_ERR("NOT found the target yet!\n");
    }
    else
    {
        list_for_each(pos, &dev_list)
        {
            elem = node_to_item(pos, available_dev_t, list);
            snprintf(buf, sizeof(buf),
                    "`%s|%s", inet_ntoa(elem->dev_ip), elem->dev_name);
            strcat(fulllist, buf);
        }
    }

    /* TODO - in the future, we need a better way to handle multiple-dev case */
    if(strlen(fulllist) > 1 && strlen(fulllist) < sizeof(fulllist))
    {
        ret = (*env)->NewStringUTF(env, fulllist);
    }
    else
    {
        ret = (*env)->NewStringUTF(env, "NOT FOUND YET");
    }
#endif
    return ret;
}

JNIEXPORT void JNICALL Java_com_caredear_detecter_MainActivity_stopServer (JNIEnv *env, jobject obj)
{
    /* Java just want the server don't broadcasting anymore,
       so we can just notify it via sem */
    int i = -1;
    if(sem_getvalue(&sem_trigger, &i) == 0)
    {
        PD_ERR("before stop, the sem value:%d\n", i);
    }
    else
    {
        PD_ERR("failed get sem value(%d)\n", errno);
    }
    sem_post(&sem_trigger);
}

JNIEXPORT void JNICALL Java_com_caredear_detecter_MainActivity_stopClient (JNIEnv *env, jobject obj)
{
    int i = -1;
    if(sem_getvalue(&sem_stopclient, &i) == 0)
    {
        PD_ERR("before stop, the sem value:%d\n", i);
    }
    else
    {
        PD_ERR("failed get sem value(%d)\n", errno);
    }
    sem_post(&sem_stopclient);
}
#else
int main(int argc, char **argv)
{
    int c;
    int running_mode = 0; /* 0 - for server boradcasing, 1 - for client */
    int quit_broadcast = 0;

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

        case 'q':
            // try let the running broadcast program quit
            quit_broadcast = 1;
            break;

        default:
            break;
        }
    }

    /* first of all, check the q/t/r control flags... */
    if(quit_broadcast == 1)
    {
        // TODO
        exit(1);
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
