#include <stdio.h>
#include <pthread.h>
#include <bits/sockaddr.h>
#include <linux/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define DEF_IP   "192.168.163.132"
#define DEF_PORT  11211
#define DEF_REQ   5

#define S printf

char def_ip[128] = DEF_IP;
int def_port = DEF_PORT;
int def_req = DEF_REQ;

void * child_thread(void *param)
{
    int c;
    memcpy(&c, param, sizeof(c));
    int s;
    char buf[32];

    S("[Child Thread-%d]-->\n",c);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1)
    {
        S("***failed create socket\n");
        return NULL;
    }
    S("[Child Thread-%d] use %d socket\n",c, s);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(def_ip);
    addr.sin_port = htons(def_port);
    S("[Child Thread-%d] try connect to %s via port %d\n",
            c,
            def_ip,
            def_port);
    if (connect(s, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        S("*** failed connect to %s, error:%s\n", def_ip, strerror(errno));
        goto failed;
    }
    S("[Child Thread-%d] connected to %s\n", c, def_ip);

    int rd;
    char *t;
    while(1)
    {
        int val = (rand()%17);
        if(val == 0) val ++;
        // try comminicate with the server?
        // send with \n ending
        sprintf(buf, "%d\n", val);
        S("[Child Thread-%d] C-->S(%d)...\n",c,val);
        write(s, buf, sizeof(buf));
        S("[Child Thread-%d] DONE. Will wait SERVER...\n",c);
        rd = read(s, buf, sizeof(buf));
        if(rd > 0){
            S("[Child Thread-%d] C<---S(%s)\n", c, buf);
            // test result checking!
            t = buf;
            if(val * 2 != atoi(t)){
                S("[Child Thread-%d] @@@EROR@@@\n", c);
            }
        }
        sleep(1);
    }

failed:
    close(s);

    S("[Child Thread-%d]<-Quit\n", c);
    return NULL;
}


int prepare_all_req_thread(int num_thread)
{
    int i;
    pthread_t *tid = (pthread_t *)malloc(num_thread * sizeof(pthread_t));
    int *cnt = (int *)malloc(num_thread*sizeof(int));
    memset(cnt,0x00, num_thread*sizeof(int));

    if(!tid)
    {
        S("**** error of malloc\n");
        return -2;
    }

    S("will prepared %d thread....\n", num_thread);

    for(i = 0; i < num_thread; i++)
    {
        cnt[i] = i;
        pthread_create(&tid[i],
                NULL,
                child_thread,
                &cnt[i]);
    }

    for(i = 0; i < num_thread; i++)
    {
        pthread_join(tid[i], NULL);
    }

    S("All thread exit\n");
    return 0;
}

int main(int argc, char **argv)
{

    if(argc > 3)
    {
        strcpy(def_ip, argv[1]);
        def_port = atoi(argv[2]);
        def_req = atoi(argv[3]);
    }

    prepare_all_req_thread(def_req);

    printf("exit the program...\n");
    return 0;
}
