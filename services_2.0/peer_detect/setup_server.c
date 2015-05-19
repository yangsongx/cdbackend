#include "peer_detect.h"


int prepare_server(int sock)
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(CONTROLLER_PORT);
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

int setup_server(int sock)
{
    if(prepare_server(sock) == -1)
    {
        PD_ERR("can't prepare server\n");
        return -1;
    }

    PD_LOG("Now, server ready, try accept incoming client...\n");

    // probably need select here?
    // because we need consider the client disconnect and re-connect again case

    return 0;
}
