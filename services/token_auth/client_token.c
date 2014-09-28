#include <stdio.h>
#include <bits/sockaddr.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "cds_public.h"

#define DEF_IP   "127.0.0.1"
#define DEF_PORT  11234

char def_ip[128] = DEF_IP;
int def_port = DEF_PORT;


/* first 2-bytes(4-digit) is data len! */
/* FIXME - need use a config-able XML for testing data! */
static char *glb_toke_data[] = {
    "006a13302525882#com.caredear.awake#1403578305#17d5vkox54y2snctz4brfb0muofxyqrhwuoif5rrf5i90rojvkn1ciziux4ylgt7",
    "006a13102525882#com.caredear.awake#1403578305#15d5vkox54y2snctz4brfb0muofxyqrhwuoif5rrf5i90rojvkn1ciziux4ylgt7",

#if 0

    // BELOW is RSA Ecryption...

    /* A good data section */
    "00D613002525882#com.caredear.xmpp#1403578305#r5O16lD7rQKGldNxpFsCNKQkJL8zcxEroQfDvakciLM9TdAjVlZeIiTfte8QirkGqXpHrBUwOBLkgOcvEPqOKEKf1bu4jI0LxjuQsCSnRTw9oB+CU0eSWjlEXp0/s2gUJR99mFX18FtpCANknnfsLztZviLgr2bg64NYXLRv1CY=",
    /* un-match user name */
    "00D613802525882#com.caredear.xmpp#1403578305#r5O16lD7rQKGldNxpFsCNKQkJL8zcxEroQfDvakciLM9TdAjVlZeIiTfte8QirkGqXpHrBUwOBLkgOcvEPqOKEKf1bu4jI0LxjuQsCSnRTw9oB+CU0eSWjlEXp0/s2gUJR99mFX18FtpCANknnfsLztZviLgr2bg64NYXLRv1CY=",
    /* un-match app name */
    "00D713002525882#com.caredear.awake#1403578305#r5O16lD7rQKGldNxpFsCNKQkJL8zcxEroQfDvakciLM9TdAjVlZeIiTfte8QirkGqXpHrBUwOBLkgOcvEPqOKEKf1bu4jI0LxjuQsCSnRTw9oB+CU0eSWjlEXp0/s2gUJR99mFX18FtpCANknnfsLztZviLgr2bg64NYXLRv1CY=",
    /* ERROR case send a bad decrypted string */
    "00D613002525882#com.caredear.xmpp#1403578305#r5O16lD7rQKGl34567sCNKQkJL8zcxEroQfDvakciLM9TdAjVlZeIiTfte8QirkGqXpHrBUwOBLkgOcvEPqOKEKf1bu4jI0LxjuQsCSnRTw9oB+CU0eSWjlEXp0/s2gUJR99mFX18FtpCANknnfsLztZviLgr2bg64NYXLRv1CY=",
    /* Bad Length(Exceed real length, but data is correct) */
    "040013002525882#com.caredear.xmpp#1403578305#r5O16lD7rQKGldNxpFsCNKQkJL8zcxEroQfDvakciLM9TdAjVlZeIiTfte8QirkGqXpHrBUwOBLkgOcvEPqOKEKf1bu4jI0LxjuQsCSnRTw9oB+CU0eSWjlEXp0/s2gUJR99mFX18FtpCANknnfsLztZviLgr2bg64NYXLRv1CY=",
    /* Exceed the max length capability */
    "040113002525882#com.caredear.xmpp#1403578305#r5O16lD7rQKGldNxpFsCNKQkJL8zcxEroQfDvakciLM9TdAjVlZeIiTfte8QirkGqXpHrBUwOBLkgOcvEPqOKEKf1bu4jI0LxjuQsCSnRTw9oB+CU0eSWjlEXp0/s2gUJR99mFX18FtpCANknnfsLztZviLgr2bg64NYXLRv1CY=",
#endif
};


int main(int argc, char **argv)
{
    int s;
    int i;
    int len;
    char buf[1024];
    ssize_t size;

    printf("client simulation\n");
    if(argc > 2)
    {
        strcpy(def_ip, argv[1]);
        def_port = atoi(argv[2]);
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1)
    {
        ERR("***failed create socket\n");
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(def_ip);
    addr.sin_port = htons(def_port);

    LOG("try connect to %s via port %d...\n",
            def_ip, def_port);

    if (connect(s, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        ERR("*** failed connect to %s, error:%s\n", def_ip, strerror(errno));
        goto failed;
    }
    LOG("connected to %s [OK]\n", def_ip);
#if 1
    len = sizeof(glb_toke_data)/sizeof(char *);
    LOG("will try sending totatlly %d token string to server...\n", len);
    for(i = 0; i < len; i++)
    {
        LOG("[%d] token len-%d\n", i, (int)(strlen(glb_toke_data[i]) + 1));
        size = write(s, glb_toke_data[i], strlen(glb_toke_data[i]) + 1);
        LOG("[%d] wrote %d to server\n", i, (int)size);
        size = read(s, buf, sizeof(buf));

        LOG("SERVER tell me with %d len, val=%d(%s), errno=%d\n",
                (int)size, *(int *)buf, convert_err_to_str(*(int *)buf), errno);
        sleep(3);
    }
#endif
failed:
    close(s);

    LOG("Client program quit.\n");
    return 0;
}
