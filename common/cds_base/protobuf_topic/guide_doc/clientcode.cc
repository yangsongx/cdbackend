/*

   How message divied and when begin,when end, all is up to you.
   protobuf only concern about message data's codec.


   choose from Stackoverflow:

   Protocol Buffers is a structured data serialization (and de-serialization) framework. It is only concerned with encoding a selection of pre-defined data types into a data stream. What you do with that stream is up to you. To quote the wiki:

   If you want to write multiple messages to a single file or stream, it is up to you to keep track of where one message ends and the next begins. The Protocol Buffer wire format is not self-delimiting, so protocol buffer parsers cannot determine where a message ends on their own. The easiest way to solve this problem is to write the size of each message before you write the message itself. When you read the messages back in, you read the size, then read the bytes into a separate buffer, then parse from that buffer.

   (http://stackoverflow.com/questions/11640864/length-prefix-for-protobuf-messages-in-c)

 */
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

#include "rpcdata.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

using namespace com::caredear;
using namespace google::protobuf::io;

/* client send req with RPC */
int send_server_data(int s)
{
    bool ok;
    int ret = 0;
    CircleContent p;
    char buf[1024];
    int len;
    ssize_t size;

    printf("prepare-ed the obj OK, the size=%d, GetCachedSize()=%d\n",
            p.ByteSize(), p.GetCachedSize());

    p.set_uid("13022593515");
    printf("after set uid, the size=%d, GetCachedSize()=%d\n",
            p.ByteSize(), p.GetCachedSize());

    p.set_startpoint(123456);
    printf("after set uid, the size=%d, GetCachedSize()=%d\n",
            p.ByteSize(), p.GetCachedSize());

    p.set_syn_data("http://qiniu.com/url/png");
    printf("after set uid, the size=%d, GetCachedSize()=%d\n",
            p.ByteSize(), p.GetCachedSize());

    len = p.ByteSize();

    ArrayOutputStream    d(buf, sizeof(buf));
    CodedOutputStream    sm(&d);

    sm.WriteRaw(&len, sizeof(int));

    ok = p.SerializeToCodedStream(&sm);
    if(ok)
    {
        size = write(s, buf, len + sizeof(int));
        printf("wrote %ld bytes to Server\n", size);
    }

    ret = ((ok == true) ? 0 : -1);

    return ret;
}

int main(int argc, char **argv)
{
    int s;
    int rc;
    ssize_t size;
    char buf[1024];
    fd_set read_fds;
    struct sockaddr_in addr;
    CircleContent resp;//TODO need another response .proto def

    s = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(11011);
    if(connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("failed connecting to server:%d\n", errno);
        close(s);
        return -1;
    }
    printf("Connected to client!\n");

    while(1){
        if(send_server_data(s) == 0)
        {
            printf("Good to send request, waiting response...\n");

            size = read(s, buf, 4);
            if(size > 0)
            {
                printf("ACK len=%d\n", *(int *)buf);
                rc = *(int *)buf;
                size = read(s, buf, rc)
                {
                }
            }


            if(resp.ParseFromFileDescriptor(s) == true)
            {
                printf("good return from waiting.\n");
            }
            else
            {
                printf("failed waiting.\n");
            }

        }
        else
        {
            printf("**failed send request, server probably broken!\n");
            break;
        }
    }


    close(s);

    printf("exit the client program\n");

    return 0;
}
