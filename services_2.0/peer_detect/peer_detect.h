#ifndef _CAREDEAR_PEER_DETECT_H
#define _CAREDEAR_PEER_DETECT_H

#ifdef ANDROID_ENV

#define LOG_TAG  "PEERDETECT"
#include <cutils/logger.h>
#include <cutils/logd.h>
#include <cutils/sockets.h>
#include <cutils/logprint.h>
#include <cutils/log.h>
#include <cutils/list.h>
#include <cutils/properties.h>

#else /* ANDROID */

#include <sys/utsname.h>
#include "list.h" // only avaialbe for Non-Android

#endif /* Generic Linux system */

/* COMMON headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <semaphore.h>

/* PD (Peer Detect) specific log macros... */
#ifdef ANDROID_ENV
#define PD_LOG LOGE
#define PD_ERR LOGE
#else
#define PD_LOG printf
#define PD_ERR printf
#endif

/* CONSTANT DEFINITION */
#define MAXINTERFACES 16
#define CONTROLLER_PORT  2121

#define PAYLOAD "21KE:"
#define RESPONSE "YES"

#define SSDP_MCAST_ADDR  "239.255.255.250"

/**
 * Method for device detection
 */
enum{
    METHOD_UNICAST = 0, /**< traditional TCP connection */
    METHOD_SSDP
};

enum{
    SCAN_STAGE_1 = 0, /**< 0~99 */
    SCAN_STAGE_2,    /**< 100~200 */
    SCAN_STAGE_3     /*< 200~255 */
};

typedef struct _available_dev_t{
    struct listnode list;
    struct in_addr  dev_ip;
    char            dev_name[256]; /**< a device name descripter */
    /* FIXME maybe we need add more specific dev info data here */
}available_dev_t;

extern int glb_port;
extern sem_t sem_trigger;
extern sem_t sem_stopclient;
extern struct listnode dev_list;

/* FUNCTION PROTOTYPE */
extern int prepare_ssdp_server(int port);
extern int detect_available_devices_ssdp(int port);
extern int detect_dev_ssdp_quick(int port, char *ip);
extern int scan_all_available_devices(int method);
extern int setup_server(int method);

#endif
