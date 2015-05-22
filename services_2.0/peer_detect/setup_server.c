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

int prepare_ssdp_server(int port)
{
    int udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    ssize_t size = 0;
    char *buf = "21KE";

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(SSDP_MCAST_ADDR);

    PD_ERR("try broadcasting...\n");

    while(1)
    {
        sleep(5);
        PD_LOG("send out broadcasting\n");
        size = sendto(udpSock, buf, strlen(buf) + 1, 0,
                (struct sockaddr *)&addr, sizeof(addr));
        PD_ERR("totally %ld bytes wrote\n", size);
    }

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
