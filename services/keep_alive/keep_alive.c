/**
 * A ping program to make sure token server
 * alive.
 *
 * If there's sth wrong , this program would
 * send out notification mail.
 */
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LOG printf
#define ERR printf
#define INFO printf
#define KPI printf

/* default IP / port, if not confi info spcified */
#define KA_DEFIP    "127.0.0.1"
#define KA_DEFPORT  11234
#define KA_INTERVAL 10

#define KA_DEF_HEALTH_SUM  (3 * 24 * 3600)

// don't exceed 32 mail counts in 8 hours(4 mail/hr)
#define KA_MAXMAIL  32


#define KA_MAILMSG  "/tmp/msg.txt"
#define KA_PROFILE  ".keep_alive"

#if 1
#define KA_CURL_ARGV "-n", "--ssl-reqd", "--mail-from", "<service@caredear.com>", \
                     "--mail-rcpt", "<13614278@qq.com>", \
                     "--url", "smtps://smtp.exmail.qq.com:465", \
                     "-T", KA_MAILMSG, \
                     "-u", "service@caredear.com:nanjing21k"
#else
#define KA_CURL_ARGV "-n", "--ssl-reqd", "--mail-from", "<service@caredear.com>", \
                     "--mail-rcpt", "<13614278@qq.com>", \
                     "--mail-rcpt", "<server@caredear.com>", \
                     "--url", "smtps://smtp.exmail.qq.com:465", \
                     "-T", KA_MAILMSG, \
                     "-u", "service@caredear.com:nanjing21k"

#endif

#define KA_MAIL_MSG "From: \"Caredear Service\" <service@caredear.com>\n" \
                    "To: \"Caredear Server\" <server@caredear.com>\n"   \
                    "Subject: keep_alive PING report\n\n"

static int  global_mail_count = 0;

static int  meet_error = 0;

static char ka_ip[32] = KA_DEFIP;
static int  ka_port = KA_DEFPORT;
static int  ka_maxmail = KA_MAXMAIL;
static int  ka_health_sum = KA_DEF_HEALTH_SUM;
static int  ka_interval = 240; /* in minute unit */

time_t start_time;
time_t end_time;


enum {
    USR_REG = 0,
    USR_LOGIN,
    USR_ACTIVATION,
    USR_AUTH,
    USR_PASSWD,
    USR_NETDISK,
};

/* Be consistent with the CDS components... */
struct ping_response{
    int  pr_code;
    int  pr_component;
    time_t pr_life;
};


static void show_usage()
{
    fprintf(stderr, "the keep_aliv will ping token server to make sure it always NOT down:\n"
            "-a X.X.X.X  specify server IP address.\n"
            "-p num      specify server port number.\n"
            "-r day      report the service healthy summary every x day(default is 3 day).\n"
            "-m maxmail if sending mail count exceed maxmail, don't send anymore to avoid spam\n"
            "-t minute   specify ping interval(in minutes Unit).\n"
            "-h          print this help info text.\n");
}

static char *map_component_code(int code, char *component)
{
    // TODO map ping_response::pr_component to a textual string here.
    switch(code){
        case USR_REG:
            strcpy(component, "Register(urs)");
            break;

        case USR_LOGIN:
            strcpy(component, "Login(uls)");
            break;

        case USR_ACTIVATION:
            strcpy(component, "activation(acts)");
            break;

        case USR_AUTH:
            strcpy(component, "User Auth(uauth)");
            break;

        case USR_PASSWD:
            strcpy(component, "Password(passwdmgr)");
            break;

        case USR_NETDISK:
            strcpy(component, "Netdisk(nds)");
            break;

        default:
            strcpy(component, "Unknow");
                break;
    }

    return component;
}

static int current_datetime(char *stored_data, size_t size_data)
{
    time_t cur;
    time(&cur);

    /* strftime() return 0 for error */
    return (strftime(stored_data, size_data, "%Y-%m-%d %H:%M:%S",
                localtime(&cur)) == 0 ? -1 : 0);
}
/**
 * Send out the email to notify the error case
 *
 */
static void send_out_email(char *mail_body)
{
#if 0
    ERR("the response of body:%s\n", mail_body);
#else
    char mail_cmd[256];
    FILE *p;
    pid_t pid;
    struct timeval t1;

    p = fopen(KA_MAILMSG, "w");
    if(!p)
    {
        ERR("failed creating mail body data:%d\n", errno);
        return;
    }

    fputs(KA_MAIL_MSG, p);
    fputs(mail_body, p);

    fclose(p);

    LOG("%s", mail_cmd);

    global_mail_count++;
    gettimeofday(&t1, NULL);
    end_time = t1.tv_sec;

    if(((end_time - start_time) < 8*3600) && global_mail_count > ka_maxmail)
    {
        ERR("Too many error mails, stop sending to avoid spam\n");
        return;
    }


    pid = fork();
    if(pid == 0)
    {
        /* Child section */
        execl("/usr/bin/curl", "curl", KA_CURL_ARGV, NULL);
    }
    else if(pid > 0)
    {
        waitpid(pid, NULL, 0);
    }
    else
    {
        ERR("**Failed calling the fork():%d\n", errno);
    }
#endif
}

void healthy_summary(time_t elapse_time)
{
    char buf[128];
    snprintf(buf, sizeof(buf),
            "Service run-ed for %d days totally without any error.\n",
            (int)(elapse_time/(3600*24))
            );
    send_out_email(buf);
}

/**
 * the HOME/.keep_alive config file like this:
 *   ==================================
 *     IP         port  time interval
 *   ==================================
 *  123.12.32.45  11211 10
 *
 *  divided by space(0x20)
 */
int parse_cfgfile(const char *filename)
{
    FILE *p;
    char buf[256];
    char tmp[32];
    int  i = 0;
    int  ip_meet = 0;
    int  ip_index = 0;
    int  port_meet = 0;

    p = fopen(filename, "r");
    if(!p)
    {
        return 1;
    }

    if(fgets(buf, sizeof(buf), p) != NULL)
    {
        for(i = 0; i < (int)strlen(buf); i++)
        {
            if(buf[i] == ' ')
            {
                if(ip_meet == 0)
                {
                    ip_meet = 1;
                    ip_index = i;
                    strncpy(ka_ip, buf, ip_index);
                    continue;
                }

                if(port_meet == 0)
                {
                    port_meet = 1;
                    strncpy(tmp, buf+ip_index, i - ip_index);
                    ka_port = atoi(tmp);

                    strncpy(tmp, buf + i, strlen(buf) - i);
                    ka_interval = atoi(tmp);

                    break;
                }
            }
        }
        LOG("IP:%s, port:%d, interval:%d\n",
                ka_ip, ka_port, ka_interval);
    }

    fclose(p);

    return 0;
}

void alive_ping()
{
    int s;
    int result;
    struct sockaddr_in addr;
    ssize_t rc;
    time_t  t;
    struct ping_response resp;
    char    buff[256];
    char    errmsg[128];
    char    cur[64];
    char    comp[64];
    struct timeval t1;

    KPI("Ping begin!\n");

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1)
    {
        sprintf(errmsg, "failed create socket:%d\n", errno);

        goto fail_ping;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ka_ip);
    addr.sin_port = htons(ka_port);
    if(connect(s, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        sprintf(errmsg, "failed connect to server:%d\n", errno);
        goto fail_ping;
    }

    LOG("Now, connected to %s:%d [OK]\n", ka_ip, ka_port);

    /* NOTE - magic is 0x1234, but we write here byte-by-byte, so
       need a swap. */
    const char ka_ping_payload[2] = {0x34, 0x12};

    rc = write(s, ka_ping_payload, 2);
    LOG("Client-----(%d bytes data)---->Server\n", rc);
    if(rc > 0)
    {
        rc = read(s, &resp, sizeof(resp));
        if(rc > 0)
        {
            LOG("Client<----(%d bytes data)-----Server\n", rc);
            current_datetime(cur, sizeof(cur));

            snprintf(buff, sizeof(buff),
                    "%s:[%s] ===>me: code=%d, life=%.2f Days\n",
                        cur, map_component_code(resp.pr_component, comp), resp.pr_code, ((float)resp.pr_life / 86400.0f));
                ERR("%s", buff);

                send_out_email(buff);
        }
        else
        {
            current_datetime(cur, sizeof(cur));
            snprintf(buff, sizeof(buff),
                    "%s**Failed call read() after sent req,%ld:%d\n",
                    cur, rc, errno);
            ERR("%s", buff);

            send_out_email(buff);
        }
    }
    else
    {
        current_datetime(cur, sizeof(cur));
        snprintf(buff, sizeof(buff),
                "%s**Failed write the req to server socket:%d\n",
                cur, errno);
        ERR("%s", buff);
        if(errno != EINTR && errno != EAGAIN)
        {
            send_out_email(buff);
        }
    }

    if(s != -1)
        close(s);

    return;

fail_ping:
    send_out_email(errmsg);

    if(s != -1)
        close(s);
}

/**
 * main entry, the argc/argv config is higher
 * than config file at ~/.xxxx
 *
 */
int main(int argc, char **argv)
{
    char *homedir;
    int c;
    struct timeval t;
    char   cfgfile[256];

    struct timeval t1, t2;


    homedir = getenv("HOME");
    if(homedir != NULL)
    {
        snprintf(cfgfile, sizeof(cfgfile),
                "%s/%s", homedir, KA_PROFILE);
        LOG("the cfg file is : %s\n", cfgfile);
        parse_cfgfile(cfgfile);
    }

    /* arguments can override config options in HOME/.keep_alive cfg */
    while((c = getopt(argc, argv, "a:p:m:r:t:h")) != -1)
    {
        switch(c)
        {
            case 'a':
                strncpy(ka_ip, optarg, sizeof(ka_ip));
                break;

            case 'm':
                ka_maxmail = atoi(optarg);
                break;

            case 'p':
                ka_port = atoi(optarg);
                break;

            case 'r':
                ka_health_sum = (atoi(optarg) * (24*3600));
                break;

            case 't':
                ka_interval = atoi(optarg);
                break;

            case 'h':
                show_usage();
                exit(0);

            default:
                break;
        }
    }

    INFO("Now, the token server IP:%s, port:%d, time interval:%d minutes\n",
            ka_ip, ka_port, ka_interval);


    gettimeofday(&t1, NULL);
    start_time = t1.tv_sec;

    alive_ping();

    /* keep ping-ing the token server... */

    while(1)
    {
        t.tv_sec = (ka_interval*60);
        t.tv_usec = 0;

        c = select(0, NULL, NULL, NULL, &t);
        if(c < 0)
        {
            if(errno == EINTR || errno == EAGAIN) continue;

            ERR("**Failed call select():%d\n", errno);
            goto failed;
        }
        else if(c == 0)
        {
            /* keep ping the server between the time interval */
            alive_ping();
        }
        else
        {
            LOG("else case, should NEVER come here.\n");
        }

    }

failed:
    LOG("Quit from main\n");
    return 0;
}
