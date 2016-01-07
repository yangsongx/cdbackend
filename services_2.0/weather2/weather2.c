/**
 * Weather2 program for getting extra weather data
 *
 * FIXME - in the future, we need take these data
 * together with legacy data, to make Machine Learning(ML)
 * be possible to auto-predict these item.
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
#include <sqlite3.h>

#define LOG printf
#define ERR printf
#define INFO printf
#define KPI(fmt, args...)  do { \
              char _buf[20]; \
              struct timeval _tv; \
              gettimeofday(&_tv, NULL); \
              time_t _n = time(NULL); \
              strftime(_buf, sizeof(_buf), "%m-%d %H:%M:%S", localtime(&_n)); \
              FILE * _fp = fopen("body.txt", "a+"); \
              if (_fp) { \
                fprintf(_fp, "[%s", _buf); \
                fprintf(_fp, ".%03ld] ", (_tv.tv_usec/1000)); \
                fprintf(_fp, fmt, ##args); \
                fclose(_fp); \
              } \
            } while(0) \

/* default IP / port, if not confi info spcified */
#define KA_DEFIP    "127.0.0.1"
#define KA_DEFPORT  11234
#define KA_INTERVAL 10

#define KA_DEF_HEALTH_SUM  (3 * 24 * 3600)

// don't exceed 32 mail counts in 8 hours(4 mail/hr)
#define KA_MAXMAIL  32


#define KA_MAILMSG  "/tmp/msg.txt"
#define KA_PROFILE  ".keep_alive"

#define CITY_DB "city_code.db"

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

#define  CITY_LENGTH   12
struct city_info
{
    char id[CITY_LENGTH];
    char name[CITY_LENGTH];
};

static char ka_ip[32] = KA_DEFIP;
static int  ka_port = KA_DEFPORT;
static int  ka_maxmail = KA_MAXMAIL;
static int  ka_health_sum = KA_DEF_HEALTH_SUM;
static int  ka_interval = 480; /* in minute unit, default 8 hours */

static int nr_push= 0;
static int nr_cities = 0;
static struct city_info * city_list = NULL;

time_t start_time;
time_t end_time;


enum {
    USR_REG = 0,
    USR_LOGIN,
    USR_ACTIVATION,
    USR_AUTH,
    USR_PASSWD,
    USR_NETDISK,
    USR_SIPS,
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

static int get_citylist_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    strcpy( (city_list + nr_push)->id, argv[1]);
    strcpy( (city_list + nr_push)->name, argv[2]);
    nr_push ++;

    return 0;
}
static int count_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    printf("Count = %s\n",argv[0]);
    nr_cities = atoi(argv[0]);

    return 0;
}
int init_db()
{
    int rc = 0;
    sqlite3 *db;
    char sql[1024];
    char *zErrMsg = 0;


    rc = sqlite3_open(CITY_DB, &db);

    if(rc)
    {
        LOG("Can't open \'%s\' database: %s\n",
                CITY_DB, sqlite3_errmsg(db));
        rc = -1;

        return rc;
    }

    snprintf(sql, sizeof(sql),  "SELECT count(*) FROM cityinfo;");

    rc = sqlite3_exec(db, sql, count_callback,  0, &zErrMsg);

    if( rc != SQLITE_OK )
    {
        LOG("SQL error [%d]: %s\n",__LINE__,  zErrMsg);
        sqlite3_free(zErrMsg);
        return -2;
    }

    city_list = (struct city_info *) malloc (sizeof(struct city_info) * nr_cities);
    if(!city_list) {
        LOG("failed allocate memroy\n");
        return -1;
    }

    snprintf(sql, sizeof(sql),  "SELECT * FROM cityinfo;");
    rc = sqlite3_exec(db, sql, get_citylist_callback, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        LOG("SQL error [%d]: %s\n",__LINE__,  zErrMsg);
        sqlite3_free(zErrMsg);
    }

    return 0;
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

        case USR_SIPS:
            strcpy(component, "OpenSIPs(opas)");
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

void collecting_weather_data()
{
    int i;
    pid_t pid;
    ssize_t rc;
    struct ping_response resp;
    char    buff[256];
    char   *city_id;

    /* each collecting, clear previously result body text */
    if(access("body.txt", F_OK) == 0) {
        LOG("clear previous file...");
        if(unlink("body.txt") == 0) {
            LOG("delete [OK]\n");
        } else {
            LOG("delete [Failed]\n");
        }
    }

    KPI("Collecting begin!\n");

    for(i = 0; i < 5/*nr_cities*/; i ++)
    {
        city_id = (city_list + i)->id;

        LOG("[%s <->%s]\n", city_id, (city_list + i)->name);

        pid = fork();
        if(pid == 0){
            /* child section */
            execl("/usr/bin/perl", "perl", "zs.pl", city_id, NULL);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0);
            LOG("%d created, waiting....[OK]\n", pid);
        } else {
            LOG("failed fork:%d\n", errno);
        }
    }

    KPI("Finished data collecting\n");
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

    struct timeval t1;


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


    init_db();
    gettimeofday(&t1, NULL);
    start_time = t1.tv_sec;


    /* keep collecting from server... */

    collecting_weather_data();
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
            collecting_weather_data();
        }
        else
        {
            LOG("else case, should NEVER come here.\n");
        }

    }

failed:
    LOG("Quit from main\n");
    if(city_list)
        free(city_list);

    return 0;
}
