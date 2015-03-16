#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
    FILE *p;
    int s;
    struct sockaddr_in addr;
    char line[64];

    p = fopen("check_memleak.testdata", "r");
    if(!p)
    {
        printf("*** failed open file\n");
        exit(0);
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family= AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(argc > 1) {
    addr.sin_port = htons(atoi(argv[1]));
    } else {
    addr.sin_port = htons(12002);
    }

    if(connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
          printf("**failed conn \n");
          close(s);
          return -1;
    }

    printf("Connecting to Server.....[OK]\n");

    size_t rc;
    char tx_buf[32];
    char rx_buf[128];
    unsigned short len;
    while(fgets(line, sizeof(line), p) != NULL) {
        memset(tx_buf, 0x00, sizeof(tx_buf));
        len = strlen(line);
        printf("line len=%d, ", len);
        line[len - 1] = '\0';
        printf("chomped line text:%s\n", line);
        *(unsigned short *)tx_buf = (len-1);
        memcpy(tx_buf + 2, line, len);

        rc = write(s, tx_buf, 1+len);
        printf("Clinet==>Server with(%ld) bytes\n", rc);

        printf("Waiting Response....");
        rc = read(s, rx_buf, sizeof(rx_buf));
        printf("Got (%ld) bytes from Server\n", rc);
        if(rc > 0){
            printf("the ret value:%d\n", *(int *)rx_buf);
            printf("the token mapping: %s=====>%s\n",
                    line, &rx_buf[4]);
        }

        printf("Finished\n\n");
    }

    getchar();

    close(s);
    fclose(p);
    return 0;
}
