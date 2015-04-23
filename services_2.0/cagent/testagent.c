#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


// default target IP:port setting
#define IP  "127.0.0.1"
#define PORT   14000


#define execute_ut_case(casename) \
    printf("====Begin "); \
    printf(#casename); \
    printf("  testing ===\n"); \
    if(casename()) { \
        gFail++; \
        printf("---> Case Failed\n"); \
    } else { \
        gPass++; \
    }

int     gPass = 0;
int     gFail = 0;

char    gIP[32] = IP;
int     gPort = PORT;
int     gMainSock;

int test_hash_function() {
    int i;
    int idx = 0;
    int last_idx = -1;
    int cnt = 1;

    for(i = 0; i < cnt; i++) {
        idx = (last_idx + 1 ) % cnt;
        last_idx = idx;
        printf("the hash value:%d\n", idx);
    }
    printf("\nend of %d-count testing...\n", cnt);

    cnt = 2;
    last_idx = -1;
    for(i = 0; i < cnt; i++) {
        idx = (last_idx + 1 ) % cnt;
        last_idx = idx;
        printf("the hash value:%d\n", idx);
    }
    printf("\nend of %d-count testing...\n", cnt);

    cnt = 7;
    last_idx = -1;
    for(i = 0; i < cnt; i++) {
        idx = (last_idx + 1 ) % cnt;
        last_idx = idx;
        printf("the hash value:%d\n", idx);
    }
    printf("\nend of %d-count testing...\n", cnt);
    return 0;
}

int test_simple_request() {
    int s;
    struct sockaddr_in addr;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1) {
        printf("**failed create socket:%d\n", errno);
        return -1;
    }
    printf("created a %d socket\n", s);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(gIP);
    addr.sin_port = htons(gPort);
    if(connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("failed connecting to server:%d\n", errno);
        close(s);
        return -1;
    }

    printf("connecting to %s:%d [OK]\n", gIP, gPort);

    printf("do you want to send data?\n");
    getchar();

    char payload[2] = {0x34, 0x12};
    char buf[32];
    size_t size;

    size = write(s, payload, sizeof(payload));
    printf("sock(%d) -----%ld byte----->Server\n", s, size);

    size = read(s, buf, sizeof(buf));
    printf("sock(%d) <-----%ld byte-----Server\n", s, size);
    //
    // more code

    close(s);
    return 0;
}

int main(int argc, char **argv)
{
    if(argc > 2) {
        strcpy(gIP, argv[1]);
        gPort = atoi(argv[2]);
    }

    gMainSock = socket(AF_INET, SOCK_STREAM, 0);
    if(gMainSock == -1) {
        printf("**failed create socket:%d\n", errno);
        return -1;
    }
    printf("Main socket %d created\n", gMainSock);

    // more code

    close(gMainSock);

    execute_ut_case(test_hash_function);
    execute_ut_case(test_simple_request);

    // test result summary
    printf("end of testing code\n");
    printf("Totally %d case tested, %d [OK], %d [Failed]\n",
            gPass + gFail, gPass, gFail);

    return 0;
}
