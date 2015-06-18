/**
 * Demo how to act as consumer(like Server)
 * in C world
 */
#include <stdio.h>
#include <assert.h>

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>
static void dump_row(long count, int numinrow, int *chs)
{
      int i;

        printf("%08lX:", count - numinrow);

          if (numinrow > 0) {
                  for (i = 0; i < numinrow; i++) {
                            if (i == 8) {
                                        printf(" :");
                                              }
                                  printf(" %02X", chs[i]);
                                      }
                      for (i = numinrow; i < 16; i++) {
                                if (i == 8) {
                                            printf(" :");
                                                  }
                                      printf("   ");
                                          }
                          printf("  ");
                              for (i = 0; i < numinrow; i++) {
                                        if (isprint(chs[i])) {
                                                    printf("%c", chs[i]);
                                                          } else {
                                                                      printf(".");
                                                                            }
                                            }
                                }
            printf("\n");
}

static int rows_eq(int *a, int *b)
{
      int i;

        for (i=0; i<16; i++)
                if (a[i] != b[i]) {
                          return 0;
                              }

          return 1;
}
void amqp_dump(void const *buffer, size_t len)
{
      unsigned char *buf = (unsigned char *) buffer;
        long count = 0;
          int numinrow = 0;
            int chs[16];
              int oldchs[16] = {0};
                int showed_dots = 0;
                  size_t i;

                    for (i = 0; i < len; i++) {
                            int ch = buf[i];

                                if (numinrow == 16) {
                                          int i;

                                                if (rows_eq(oldchs, chs)) {
                                                            if (!showed_dots) {
                                                                          showed_dots = 1;
                                                                                    printf("          .. .. .. .. .. .. .. .. : .. .. .. .. .. .. .. ..\n");
                                                                                            }
                                                                  } else {
                                                                              showed_dots = 0;
                                                                                      dump_row(count, numinrow, chs);
                                                                                            }

                                                      for (i=0; i<16; i++) {
                                                                  oldchs[i] = chs[i];
                                                                        }

                                                            numinrow = 0;
                                                                }

                                    count++;
                                        chs[numinrow++] = ch;
                                          }

                      dump_row(count, numinrow, chs);

                        if (numinrow != 0) {
                                printf("%08lX:\n", count);
                                  }
}

int main(int argc, char **argv)
{
    int i;
    amqp_bytes_t queuename;
    amqp_connection_state_t conn = NULL;
    amqp_socket_t *socket = NULL;
    amqp_rpc_reply_t ret;

    conn = amqp_new_connection();
    assert(conn != NULL);

    socket = amqp_tcp_socket_new(conn);
    if(!socket)
    {
        printf("**failed new tcp socket on the conn\n");
        goto failed;
    }

    printf("Socket new... [OK]\n");

    i = amqp_socket_open(socket, "localhost", 5672);
    if(i)
    {
        printf("*** failed open socket...(%d)\n", i);
        goto failed;
    }

    printf("opening the new-ed socket on port and host...[OK]\n");

    ret = amqp_login(conn,
            "/" , // vhost
            0, // channel_max
            131072, // max_frame
            0, // hearbeat,
            AMQP_SASL_METHOD_PLAIN, // sasl_method
            "guest", "guest");

    printf("the login reply_type:%d\n", ret.reply_type);

    if(amqp_channel_open(conn, 1) == NULL)
    {
        printf("failed open channel..\n");
        goto failed;
    }

    printf("Channel opened [OK]\n");


    // next will try declear queue...
    amqp_queue_declare_ok_t *r = NULL;
    r = amqp_queue_declare(conn, 1, amqp_empty_bytes,
            0, 0, 0,
            1,  // auto delete
            amqp_empty_table);
    if(r == NULL)
    {
        printf("failed declear the queue...\n");
        goto failed;
    }

    queuename = amqp_bytes_malloc_dup(r->queue);
    if(queuename.bytes == NULL)
    {
        printf("failed get queuename, pointer is NULL\n");
        goto failed;
    }

    printf("the queuename : %s\n", (const char *)queuename.bytes);

#if 1
    // bind with exchange and key
    if(amqp_queue_bind(conn, 1, queuename,
            amqp_cstring_bytes("amq.direct"),
            amqp_cstring_bytes("test"),
            amqp_empty_table) == NULL)
    {
        printf("bind queue failed...\n");
        goto failed;
    }

    printf("bind a queue [OK]\n");
#endif
    amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
    printf("... return from basic_resume...\n");

    while(1) {
        amqp_envelope_t envelope;
    printf("... @@maybe...\n");

        //amqp_maybe_release_buffers(conn);

        printf("will consume from Queue...\n");
        ret = amqp_consume_message(conn, &envelope, NULL, 0);
        if(AMQP_RESPONSE_NORMAL != ret.reply_type) {
            break;
        }
      printf("Delivery %u, exchange %.*s routingkey %.*s\n",
             (unsigned) envelope.delivery_tag,
             (int) envelope.exchange.len, (char *) envelope.exchange.bytes,
             (int) envelope.routing_key.len, (char *) envelope.routing_key.bytes);

      if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
        printf("Content-type: %.*s\n",
               (int) envelope.message.properties.content_type.len,
               (char *) envelope.message.properties.content_type.bytes);
      }
        printf("----->\n");
      amqp_dump(envelope.message.body.bytes, envelope.message.body.len);
      sleep(5);

      amqp_destroy_envelope(&envelope);
    }

failed:
    /* FIXME - amqp_connection_close() would 
     * close all channels as well??
     */

    // NEED I close channel?

    if(conn != NULL)
    {
        ret = amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
        printf("the close connection reply type:%d\n", ret.reply_type);
        if(ret.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION)
        {
            printf("  the close error msg:%s\n", amqp_error_string2(ret.library_error));
        }
    }

    printf("program existed\n");

    return 0;
}
