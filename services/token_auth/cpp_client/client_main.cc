// g++
//
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const char packet[] = "006013770648418#com.xmpp#1421991638#q7k54eisoukjvyg35b08dg8cqghmegzvt7nqd8suhffsb1avsn08v0uz8187qglm";
const char packet2[] = "006013815882359#com.xmpp#1421978655#9jh8y4fevnxgv9ikido483msyn41uaohz6qe61g5o9p63taj4yz48xvtgpf58j3h";
const char packet3[] = "006013501552303#com.xmpp#1421978655#vaivsmwi3237fozkzpedsrr28odkti5xawo7uh50hlboxhsos3shhopgqdocwr89";
const char packet4[] = "006015850512694#com.xmpp#1421978655#5kb79uoffuewq0sjgcqi9fm9quggtg1swnkqpp8ehmltlm69g3n0glccyj9p30wb";

int main(int argc, char **argv)
{
    int s;
    char token_server_ip[32] = "127.0.0.1";
    int  token_server_port = 12306;
    struct sockaddr_in addr;
    size_t size;
    char buf[32];

    if(argc > 2)
    {
        strcpy(token_server_ip, argv[1]);
        token_server_port = atoi(argv[2]);
    }

    s = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(token_server_ip);
    addr.sin_port = htons(token_server_port);
    if(connect(s, (struct sockaddr *) &addr, sizeof(addr)) != -1)
    {
        printf("Connecting to %s:%d [OK]\n",
                token_server_ip, token_server_port);
        size = write(s, packet, strlen(packet));
        printf("Client==>Server with %ld bytes data sent\n", size);

        size = read(s, buf, sizeof(buf));
        printf("Client<==Server with %ld bytes data read\n", size);
        if(size == 4)
        {
            printf("\nconsider as a code like : %d\n", *(int *) buf);
        }
    }

    close(s);

    printf("quit from test client program\n");
    return 0;
}
