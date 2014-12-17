#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>

#include "rpcdata.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

#ifndef MAX
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#endif

using namespace com::caredear;
using namespace google::protobuf::io;

int handle_request_data(int fd)
{
    CircleContent data;
    bool ok;
    int  validlen = 0;
    char buf[1024];
    ssize_t size;

    printf("blocking here?\n");
    size = read(fd, &validlen, 4);
    if(size > 0)
    {
        printf("client tell me valid len = %d bytes\n", validlen);
        size = read(fd, buf, validlen);
        if(size > 0)
        {
            ArrayInputStream in(buf, validlen);
            CodedInputStream is(&in);

            ok = data.ParseFromCodedStream(&is);

        if(ok)
    {
        printf("good to parse the client data.\n");
        printf("uid:%s,t:%d,data:%s\n",
                data.uid().c_str(), data.startpoint(), data.syn_data().c_str());


        data.set_uid("good");
        data.set_startpoint(9876);
        data.set_syn_data("motorola");
        ok = data.SerializeToFileDescriptor(fd);
        if(ok)
        {
            printf("Cool, ACK finished\n");
        }

    }
    else
    {
        printf("failed parse the client data\n");
    }
        }
    }
    else
    {
        printf("SHIT, leading-length is unknow!\n");
    }


    return ok ? 0 : -1;
}

int main(int argc, char **argv)
{
    int sock_fd = -1, cs = -1;
    fd_set read_fds;
    bool   ok;
    int    max_fd, rc;
    struct sockaddr_in saddr, caddr;
    socklen_t caddr_len = sizeof(caddr);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(11011);

    bind(sock_fd, (struct sockaddr *)&saddr, sizeof(saddr));

    listen(sock_fd, 10);
    printf("Server ready to waiting client connection..(cs=%d)\n", cs);


    while(1){
        FD_ZERO(&read_fds);

        FD_SET(sock_fd, &read_fds);
        if(cs != -1)
        {
            FD_SET(cs, &read_fds);
            max_fd = MAX(sock_fd, cs);
        }
        else
            max_fd = sock_fd;

        rc = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if(rc < 0)
        {
            if(errno == EINTR || errno == EAGAIN) continue;

            printf("failed call the select():%d\n", errno);
            break;
        }
        else if(!rc)
        {
            printf("timeout case, SHOULD NEVER happen!\n");
            continue;
        }
        else
        {
            if(FD_ISSET(sock_fd, &read_fds))
            {
                /* this server socke FD_ISSET would probably
                   caused by new in-coming connection */

                cs = accept(sock_fd, (struct sockaddr*)&caddr, &caddr_len);
                printf("Got  a %d incoming client...\n", cs);

            }
            else if(FD_ISSET(cs, &read_fds))
            {
                printf("client's event...\n");

                if(handle_request_data(cs)!= 0)
                {
                    printf("since the handling is failed, client probably broken\n");
                    close(cs);
                    cs = -1;
                }
            }
        }
    }

    if(cs != -1)
        close(cs);

    printf("quit the server main\n");
    return 0;
}
