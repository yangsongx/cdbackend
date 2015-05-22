/**
 * for peer detect(IP address) under a WAN
 *
 * Note - 239.255.255.250 IP, 
 * keep sending out UDP on that IP, and check IF we can got it from client
 */

#include "peer_detect.h"

int main(int argc, char **argv)
{
    int s;

    if(argc < 2){
        PD_ERR("peer_detect -s/-c\n");
        exit(0);
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1)
    {
        PD_ERR("failed create the socket(%d)\n", errno);
        exit(0);
    }

    if(!strcmp(argv[1], "-s"))
    {
        PD_LOG("starting server...\n");
        setup_server(s, METHOD_SSDP);
    }
    else if(!strcmp(argv[1], "-c"))
    {
        PD_LOG("starting client(scan available devices...)...\n");
        scan_all_available_devices(s, METHOD_SSDP);
    }

    close(s);

    PD_LOG("exit the program\n");
    return 0;
}
