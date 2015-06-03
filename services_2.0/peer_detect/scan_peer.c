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
    addr.sin_port = htons(glb_port);
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

int add_ip_to_dev_list(struct listnode *hdr, available_dev_t *node)
{
    struct listnode *pos;
    available_dev_t *elem;

    if(!node)
    {
        return -1;
    }
#if 1
    int i = 0;
    PD_ERR("first, try know the list length...\n");
    list_for_each(pos, hdr)
    {
        elem = node_to_item(pos, available_dev_t, list);
        i++;
    }

    PD_ERR("totally %d  on the list\n", i);

#endif
    // empty would not check any elem
    if(list_empty(hdr))
    {
        PD_ERR("empty, directly insert\n");
        list_add_tail(hdr, &(node->list));
    }

    list_for_each(pos, hdr)
    {
        elem = node_to_item(pos, available_dev_t, list);
        PD_ERR("the elem on list's data:%s\n", inet_ntoa(elem->dev_ip));
        if(memcmp(&(node->dev_ip), &(elem->dev_ip), sizeof(struct in_addr)))
        {
            PD_ERR("will insert...\n");
            list_add_tail(hdr, &(node->list));
            PD_ERR("insertion OK\n");
        }
        else
        {
            PD_ERR("the same device, ignore it\n");
            free(node);
        }
    }

    return 0;
}

int detect_available_devices_ssdp(int port)
{
    int udpSock;
    char buf[256];
    ssize_t size;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    available_dev_t *item;

    udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if(udpSock < 0)
    {
        PD_ERR("failed create the UPD socket(%d)\n", errno);
        return -1;
    }

    PD_LOG("UDP socket=%d, the port=%d\n", udpSock, port);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if(bind(udpSock, (struct sockaddr*) &addr, sizeof(addr)) == -1)
    {
        PD_ERR("**failed bind() UPD(%d)\n", errno);
    }

    struct ip_mreq remote;
    len = sizeof(remote);
    remote.imr_multiaddr.s_addr = inet_addr(SSDP_MCAST_ADDR);
    remote.imr_interface.s_addr = htonl(INADDR_ANY);

    if(setsockopt(udpSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &remote, len) == -1)
    {
        PD_ERR("**failed add membership(%d)\n", errno);
    }

    while(1)
    {
        if(sem_trywait(&sem_stopclient) == 0)
        {
            PD_LOG("Wow, stop the client\n");
            break;
        }
        sleep(4);
        len = sizeof(addr);
        PD_ERR("a22301---try recv...\n");
        size = recvfrom(udpSock, buf, sizeof(buf), 0,
                (struct sockaddr *)&addr, &len);
        PD_ERR("Totally %ld bytes got\n", size);
        if(size > 0) {
            PD_ERR("+buf:%s\n", buf);
            if(!strncmp(buf, PAYLOAD, strlen(PAYLOAD)))
            {
                PD_ERR("IP:%s\n", inet_ntoa(addr.sin_addr));
                PD_ERR("+full data from server:%s\n", buf);

                item = (available_dev_t *) malloc(sizeof(available_dev_t));
                if(item != NULL)
                {
                    memset(item, 0x00, sizeof(available_dev_t));
                    item->dev_ip = addr.sin_addr;
                    strncpy(item->dev_name, buf + strlen(PAYLOAD), 200);
                    add_ip_to_dev_list(&dev_list, item);
                }
            }
        }
    }

    PD_LOG("free the client's resource...\n");
    sem_destroy(&sem_stopclient);
    close(udpSock);

    return 0;
}

/**
 * This will only get the first meet device
 *
 */
int detect_dev_ssdp_quick(int port, char *ip)
{
    int rc;
    int udpSock;
    char buf[128];
    ssize_t size;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    available_dev_t *item;
    fd_set read_fds;
    struct timeval timeout;

    udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if(udpSock < 0)
    {
        PD_ERR("failed create the UPD socket(%d)\n", errno);
        return -1;
    }

    PD_LOG("the UDP socket=%d\n", udpSock);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if(bind(udpSock, (struct sockaddr*) &addr, sizeof(addr)) == -1)
    {
        PD_ERR("**failed bind() UPD(%d)\n", errno);
    }

    struct ip_mreq remote;
    len = sizeof(remote);
    remote.imr_multiaddr.s_addr = inet_addr(SSDP_MCAST_ADDR);
    remote.imr_interface.s_addr = htonl(INADDR_ANY);

    if(setsockopt(udpSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &remote, len) == -1)
    {
        PD_ERR("**failed add membership(%d)\n", errno);
    }

    if(sem_init(&sem_stopclient, 1, 0) != 0)
    {
        PD_ERR("Warning, failed init unamed sem(%d)\n", errno);
    }

    while(1)
    {
        FD_ZERO(&read_fds);
        FD_SET(udpSock, &read_fds);

        timeout.tv_sec = 4; // every 4 seconds return a value.
        timeout.tv_usec = 0;

        rc = select(udpSock + 1, &read_fds, NULL, NULL, &timeout);

        if(rc < 0)
        {
            if(errno == EINTR || errno == EAGAIN)
                continue;
            //for other case, breakout
            PD_ERR("select() got %d errno\n", errno);
            break;
        }
        else if(!rc)
        {
            PD_ERR("time out case\n");
            // timeout, check if Java want to quit the scanning or NOT
            if(sem_trywait(&sem_stopclient) == 0)
            {
                PD_LOG("Wow, stop the client\n");
                sprintf(ip, "NOT FOUND YET");
                break;
            }
        }
        else {

            if(FD_ISSET(udpSock, &read_fds))
            {

        len = sizeof(addr);
        size = recvfrom(udpSock, buf, sizeof(buf), 0,
                (struct sockaddr *)&addr, &len);
        PD_ERR("@Totally %ld bytes got\n", size);
        if(size > 0) {
            PD_ERR("-buf:%s\n", buf);
            if(!strncmp(buf, PAYLOAD, strlen(PAYLOAD)))
            {
                PD_ERR("IP:%s\n", inet_ntoa(addr.sin_addr));
                sprintf(ip, "%s|%s", inet_ntoa(addr.sin_addr),
                        buf + strlen(PAYLOAD));
                break;
            }
            else
            {
                sprintf(ip, "NOT FOUND YET");
                break;
            }
        }

        } /* FD_ISSET */


        } /* else */
    }

    sem_destroy(&sem_stopclient);
    close(udpSock);

    return 0;
}

int scan_all_available_devices(int method)
{
    switch(method)
    {
        case METHOD_UNICAST:
            PD_ERR("Not support yet!(we don't want to use TCP to do this)\n");
            break;

        case METHOD_SSDP:
            detect_available_devices_ssdp(glb_port);
            break;

        default:
            break;
    }
    return 0;
}
