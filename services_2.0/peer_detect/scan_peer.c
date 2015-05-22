#include "peer_detect.h"

list_declare(dev_list);

int get_local_ip(int sock, struct in_addr *ip)
{
    int ret = -1;
    int interface = 0;
    struct ifconf ifc;
    struct ifreq  req;
    struct ifreq buf[MAXINTERFACES];

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
    if(!ioctl(sock, SIOCGIFCONF, (char *) &ifc))
    {
        interface = ifc.ifc_len / sizeof(struct ifreq);
        PD_LOG("totally interface count:%d\n", interface);
    }
    else
    {
        PD_ERR("failed ioctl step1(%d)\n", errno);
        return -1;
    }

    while(interface-->0) {

        if(!ioctl(sock, SIOCGIFADDR, &buf[interface]))
        {
            if(strcmp(buf[interface].ifr_name, "lo"))
            {
                memcpy(ip,
                       &(((struct sockaddr_in*) (&buf[interface].ifr_addr))->sin_addr),
                       sizeof(struct in_addr));
                PD_ERR("int[%s] ==> IP[%s]\n", buf[interface].ifr_name,
                        inet_ntoa(*ip));
                ret = 0;
                break;
            }
        }
        else
        {
            PD_ERR("**Failed ioctl the socket to get IP address(%d)", errno);
        }
    }

    return ret;
}


/**
 *
 */
int talk_to_device(int sock, struct in_addr *target_ip)
{
    struct sockaddr_in addr;
    available_dev_t *dev;
    char buf[32];
    int  ret = -1;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(CONTROLLER_PORT);
    addr.sin_addr = *target_ip;

    if(connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        PD_ERR("**failed connecting to %s server(%d)\n",
                inet_ntoa(addr.sin_addr), errno);
        return -1;
    }
    PD_LOG("Connect to server OK, try hand shaking...\n");
    if(write(sock, PAYLOAD, strlen(PAYLOAD) + 1) != (strlen(PAYLOAD) + 1))
    {
        PD_ERR("warning, write count unmatch!\n");
    }

    if(read(sock, buf, sizeof(buf)) > 0)
    {
        if(!strncmp(buf, RESPONSE, 3))
        {
            struct sockaddr_in peer_addr;
            socklen_t peer_len = sizeof(peer_addr);
            if(getpeername(sock, (struct sockaddr *)&peer_addr, &peer_len) == 0)
            {
                PD_ERR("===peer IP:%s, consider it as a usable device\n", inet_ntoa(peer_addr.sin_addr));
                dev = (available_dev_t *)malloc(sizeof(available_dev_t));
                if(dev)
                {
                    memcpy(&(dev->dev_ip), &(peer_addr.sin_addr), sizeof(struct in_addr));
                    list_add_tail(&dev_list, &(dev->list));
                }
                ret = 0;
            }
            else
            {
                PD_ERR("failed call getpeername:%d\n", errno);
            }
        }
    }
    else
    {
        PD_ERR("failed read the response from target device\n");
    }

    return ret;
}

/**
 *@ip : local IP
 *@stage : see SCAN_STAGE_x enum definition in header
 *@single : if 1, will stop scan more devices. otherwise, scan all possible device
 */
int try_peek_device(int sock, int stage, struct in_addr *ip, int single)
{
    int i;
    int rc;
    struct in_addr copy = *ip;
    unsigned char *p = (unsigned char *)&copy;

    switch(stage)
    {
        case SCAN_STAGE_1:
            for(i = 0; i < 100; i++)
            {
                p[3] = (unsigned char)i;
                if(memcpy(&copy, ip, 4))
                {
                    rc = talk_to_device(sock, &copy);
                    if(rc == 0 && single == 1)
                    {
                        break;
                    }
                }
            }
            break;

        case SCAN_STAGE_2:
            for(i = 100; i < 200; i++)
            {
                p[3] = (unsigned char)i;
                if(memcpy(&copy, ip, 4))
                {
                    rc = talk_to_device(sock, &copy);
                    if(rc == 0 && single == 1)
                    {
                        break;
                    }
                }
            }
            break;

        case SCAN_STAGE_3:
            for(i = 200; i < 256; i++)
            {
                p[3] = (unsigned char)i;
                if(memcpy(&copy, ip, 4))
                {
                    rc = talk_to_device(sock, &copy);
                    if(rc == 0 && single == 1)
                    {
                        break;
                    }
                }
            }
            break;

        default:
            PD_ERR("**unknow type value\n");
            break;
    }

    return 0;
}

int detect_available_devices_unicast(int sock)
{
    struct in_addr  local_ip;
    unsigned char *p; // store last fields of the IP

    if(get_local_ip(sock, &local_ip) != 0)
    {
        PD_ERR("can't get local IP, please check!\n");
        return -1;
    }

    p = (unsigned char *)&local_ip;

    PD_ERR("%#x %#x %#x %#x", p[0], p[1],p[2], p[3]);

    /* FIXME need optimize the search process */

    if(p[3] < 100)
    {
        try_peek_device(sock, SCAN_STAGE_1, &local_ip, 1 /* only match one device */);
    }
    else if(p[3] >= 100 && p[3] < 200)
    {
        try_peek_device(sock, SCAN_STAGE_2, &local_ip, 1 /* only match one device */);
    }
    else if(p[3] >= 200)
    {
        try_peek_device(sock, SCAN_STAGE_3, &local_ip, 1 /* only match one device */);
    }

    return 0;
}

int detect_available_devices_ssdp(int sock)
{
    int udpSock = (AF_INET, SOCK_DGRAM, 0);
    char buf[128];
    ssize_t size;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    while(1)
    {
        sleep(4);
        size = recvfrom(udpSock, buf, sizeof(buf), 0,
                (struct sockaddr *)&addr, &len);
        PD_ERR("Totally %ld bytes got\n", size);
        if(size > 0) {
        PD_ERR("buf:%s\n", buf);
        PD_ERR("IP:%s\n", inet_ntoa(addr.sin_addr));
        }
    }

    close(udpSock);
}

int scan_all_available_devices(int sock, int method)
{
    switch(method)
    {
        case METHOD_UNICAST:
            break;

        case METHOD_SSDP:
            detect_available_devices_ssdp(sock);
            break;

        default:
            break;
    }
    return 0;
}
