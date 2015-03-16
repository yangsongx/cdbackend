
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
    int s;
    int c;
    struct sockaddr_in addr, caddr;
    socklen_t addr_len;

    s = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family= AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(14000);

    if(bind(s, (struct sockaddr*)&addr,sizeof(addr)) == -1)
    {
        printf("***failed bind..\n");
        exit(1);
    }

    printf("bind [OK]\n");
    listen(s, 5);

    c = accept(s, (struct sockaddr*)&caddr, &addr_len);

    size_t rc;
    char rx_buf[32];
    char tx_buf[128];
    while(1) {
        rc = read(c, rx_buf, sizeof(rx_buf));
        printf("Server <---got (%ld) ---- client\n", rc);
        printf("  clinet want %s\n", rx_buf + 2);

        *(int *)tx_buf = 0;
        strcpy(tx_buf+4, "hello world");
        rc = write(c, tx_buf, sizeof(tx_buf));
        printf("Server ----(%ld) --->client\n", rc);
    }

    return 0;
}
