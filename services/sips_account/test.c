/* gcc test.c */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int s;
    struct sockaddr_in addr;
    char *mobile = "13815882359";
    char ipaddr[32] = "127.0.0.1";

    if(argc > 1)
    {
        strcpy(ipaddr, argv[1]);
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1)
    {
        printf("failed open socket\n");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipaddr);
    addr.sin_port = htons(12002);

    if(connect(s, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        printf("failed connecting to server:%d\n", errno);
    }
    else
    {
        char reqdata[128];
        char backdata[128];

        memset(reqdata, '\0', sizeof(reqdata));
        unsigned short len = (strlen(mobile) + 1);
        printf("sizeof(unsigned short) = %ld\n", sizeof(len));
        memcpy(reqdata, &len, 2);
        memcpy(reqdata + 2, mobile, strlen(mobile));
        ssize_t rc;
        rc = write(s, reqdata, sizeof(reqdata));
        printf("client wroten %ld bytes, waiting for Server response...\n\n", rc);

        rc = read(s, backdata, sizeof(backdata));
        printf("Server--> Client with %ld bytes data\n", rc);
        int code = -1;
        memcpy(&code, backdata, 4);
        if(code == 0)
        {
            printf("user's token = %s\n", backdata + 4);
        }
        else
        {
            printf("error code = %d\n", code);
        }
    }

    close(s);

    printf("quit from test program\n");
    return 0;
}
