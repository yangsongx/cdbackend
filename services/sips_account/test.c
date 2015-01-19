/**
 * Stress Test for OpenSIPS, required sipp tool
 *
 * gcc test.c -pthread
 */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>

#define SIPSERVER "rdp.caredear.com"
#define SIPPORT 5060

struct testinfo{
    char *localip;
    char *srvip;
    int  count;
    int  port;
};

struct testinfo _info;

/**
 * TODO - ifconfig is NOT correct on this...
 */
int get_local_address(char *addr_ip)
{
    int ret = -1;
    int s;
    struct sockaddr addr;
    socklen_t len;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s >= 0)
    {
        if(getsockname(s, &addr, &len) == 0)
        {
            ret = 0;
            struct sockaddr_in *p = (struct sockaddr_in *) &addr;
            strcpy(addr_ip, inet_ntoa(p->sin_addr));
        }

        close(s);
    }

    return ret;
}

int get_dns_ip(char *server, char *ipaddr)
{
    int ret = -1;
    struct hostent *host;

    host = gethostbyname(server);

    if(!host)
    {
        printf("***failed DNS the server:%d\n", errno);
        return -1;
    }

    if(host->h_addrtype == AF_INET && host->h_length > 0)
    {
#if 1
        memcpy(&ret, host->h_addr_list[0], 4);
        printf("%s ==> %d.%d.%d.%d\n", server,
                ret & 0xFF,
                (ret & 0xFF00) >> 8,
                (ret & 0xFF0000) >> 16,
                (ret & 0xFF000000) >> 24);

        sprintf(ipaddr, "%d.%d.%d.%d", 
                ret & 0xFF,
                (ret & 0xFF00) >> 8,
                (ret & 0xFF0000) >> 16,
                (ret & 0xFF000000) >> 24);
#endif
    }

    return ret;
}


int execute_sipp_testcmd(const char *usr, const char *csv, const char *localip, const char *srvip, int port, int count)
{
    char cmd[1224];

    sprintf(cmd, "sipp -sf reglogin.xml -s %s -inf %s -i %s -m %d %s:%d",
                usr, csv, localip, count, srvip, port);

    printf("Will execute %s...\n", cmd);
    system(cmd);
}

void *child1(void *param)
{
    execute_sipp_testcmd("siptest1", "test1.cvs", _info.localip, _info.srvip, _info.port, _info.count);
}

void *child2(void *param)
{
    execute_sipp_testcmd("siptest2", "test2.csv", _info.localip, _info.srvip, _info.port, _info.count);
}

void *child3(void *param)
{
    execute_sipp_testcmd("siptest3", "test3.csv", _info.localip, _info.srvip, _info.port, _info.count);
}

void *child4(void *param)
{
    execute_sipp_testcmd("siptest4", "test4.csv", _info.localip, _info.srvip, _info.port, _info.count);
}

void stress_test_for_sips_login(const char *localip, const char *ipname, int port)
{
    pthread_t tid1, tid2, tid3, tid4;

    if(pthread_create(&tid1, NULL, child1, NULL) != 0)
    {
        printf("Warning, failed execute thread1(%d)\n", errno);
    }
#if 1
    if(pthread_create(&tid2, NULL, child2, NULL) != 0)
    {
        printf("Warning, failed execute thread1(%d)\n", errno);
    }

    if(pthread_create(&tid3, NULL, child3, NULL) != 0)
    {
        printf("Warning, failed execute thread1(%d)\n", errno);
    }

    if(pthread_create(&tid4, NULL, child4, NULL) != 0)
    {
        printf("Warning, failed execute thread1(%d)\n", errno);
    }
#endif
    pthread_join(tid1, NULL);
#if 1
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);
    pthread_join(tid4, NULL);
#endif
}

/**
 *
 * a.out -s serverip -p port
 *
 *
 */
int main(int argc, char **argv)
{
    int c;
    int count = 2500;
    char ip[32];
    char localip[32];
    int port = SIPPORT;

    while((c = getopt(argc, argv, "c:s:p:l:")) != -1)
    {
        switch(c)
        {
            case 'l':
                strcpy(localip, optarg);
                break;

            case 'c':
                count = atoi(optarg);
                break;

            case 's':
                strcpy(ip, optarg);
                break;

            case 'p':
                port = atoi(optarg);
                break;

            default:
                break;
        }
    }

    if(get_dns_ip (SIPSERVER, ip) != 0)
    {
        printf("*** failed DNS the server\n");
    }

    printf("SIPS IP:%s:%d, count=%d, local IP:%s\n",
            ip, port, count, localip);

    _info.localip = localip;
    _info.srvip = ip;
    _info.count = count;
    _info.port = port;


    // Testing case
    stress_test_for_sips_login(localip, ip, port);


    printf("quit from test program\n");
    return 0;
}
