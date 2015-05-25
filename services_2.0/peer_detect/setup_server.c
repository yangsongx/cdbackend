/**
 * All server side related code would implemented in this file.
 *
 */
#include "peer_detect.h"


int prepare_server(int sock)
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(glb_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        PD_ERR("failed bind server:%d\n", errno);
        return -1;
    }

    if(listen(sock, 5) == -1) {
        PD_ERR("failed listen:%d\n", errno);
        return -1;
    }

    return 0;
}

/**
 * This is just let server don't broadcast the msg
 *
 */
void stop_ssdp_server()
{
}

int prepare_ssdp_server(int port)
{
    int udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    ssize_t size = 0;
    char buf[256];
    char devname[200];

    strcpy(buf, PAYLOAD);

    if(sem_init(&sem_trigger, 1, 0) != 0)
    {
        PD_ERR("failed init the unname sem(%d)\n", errno);
        // just output a warning..
    }

#ifdef ANDROID_ENV
    /* Android specific info */
    property_get("ro.product.model",
            devname,
            "generic-android");
    strcat(buf, devname);
#else
    /* Generic Linux system info */
    struct utsname u;
    if(uname(&u) == 0)
    {
        strcat(buf, u.sysname);
        strcat(buf, " ");
        strcat(buf, u.machine);
    }
    else
    {
        /* make sure there's a default value */
        strcat(buf, "generic-linux");
    }
#endif
    PD_ERR("the devinfo:%s\n", buf);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(SSDP_MCAST_ADDR);

    PD_ERR("try broadcasting...\n");

    while(1)
    {
        if(sem_trywait(&sem_trigger) == 0)
        {
            PD_LOG("Wow, stop broadcasting!\n");
            break;
        }

        sleep(5);
        PD_LOG("send out broadcasting\n");
        size = sendto(udpSock, buf, strlen(buf) + 1, 0,
                (struct sockaddr *)&addr, sizeof(addr));
        PD_ERR("totally %ld bytes wrote\n", size);
    }

    PD_ERR("free resource...\n");
    sem_destroy(&sem_trigger);
    close(udpSock);

    return 0;
}

/**
 *
 *@method : see @enum METHOD_XXX definition
 */
int setup_server(int method)
{
    switch(method)
    {
        case METHOD_UNICAST:
            PD_ERR("Currently NOT support yet(don't use tcp)\n");
            break;

        case METHOD_SSDP:
            prepare_ssdp_server(glb_port);
            break;

        default:
            break;
    }

    // probably need select here?
    // because we need consider the client disconnect and re-connect again case

    return 0;
}
