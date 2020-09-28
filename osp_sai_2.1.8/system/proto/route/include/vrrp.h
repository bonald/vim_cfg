
#ifndef __VRRP_H__
#define __VRRP_H__

#include "gen/tbl_route_debug_define.h"
#include "gen/tbl_kernel_if_define.h"

#define VRRP_LOG_EMERG(fmt, args...)    log_sys(M_MOD_VRRP, E_EMERGENCY, fmt, ##args)
#define VRRP_LOG_ALERT(fmt, args...)    log_sys(M_MOD_VRRP, E_ALERT, fmt, ##args)
#define VRRP_LOG_CRIT(fmt, args...)     log_sys(M_MOD_VRRP, E_CRITICAL, fmt, ##args)
#define VRRP_LOG_ERR(fmt, args...)      log_sys(M_MOD_VRRP, E_ERROR, fmt, ##args)
#define VRRP_LOG_WARN(fmt, args...)     log_sys(M_MOD_VRRP, E_WARNING, fmt, ##args)
#define VRRP_LOG_NOTICE(fmt, args...)   log_sys(M_MOD_VRRP, E_NOTICE, fmt, ##args)
#define VRRP_LOG_INFO(fmt, args...)     log_sys(M_MOD_VRRP, E_INFORMATIONAL, fmt, ##args)

bool vrrp_debug_is_on(uint32 flag);

#define VRRP_LOG_DEBUG(flag, fmt, args...) \
do { \
    if (vrrp_debug_is_on(flag)) \
        log_sys(M_MOD_VRRP, E_DEBUG, fmt, ##args); \
} while (0)

#define VRRP_LOG_PRINT(fmt, args...) \
    log_sys(M_MOD_VRRP, E_DEBUG, fmt, ##args);

#define     VRRP_PROTOCOL_IP            1

#define     VRRP_ADMIN_STATE_UP         1
#define     VRRP_ADMIN_STATE_DOWN       2

#define     VRRP_DEFAULT_IP_OWNER_PRIORITY      255
#define     VRRP_DEFAULT_DELTA_PRIORITY         10

#define     MIN_VRID                1
#define     MAX_VRID                255
#define     VRRP_VERSION            2
#define     VRRP_ADVT_TYPE          1
#define     IPPROTO_VRRP            112
#define     VRRP_MCAST_ADDR         0xe0000012
#define     VRRP_IP_TTL             255

#define     VRRP_SESS_MAX           3 

#define     VRRP_SKEW_PRIORITY      128

#define     VRRP_MAX_STATE_LEN      200

#define     VRRP_IP_LENGTH          256

#define     VRRP_AUTH_NONE          0

#define     VRRP_MIN_PKT_LEN        60

#define     VRRP_V3_MAX_ADVT_INTERVAL  40

#define     VRRP_SET            0
#define     VRRP_UNSET          -1

#define VRRP_PTR_CHECK(ptr) \
    do { \
        if (NULL == ptr) \
        {\
            sal_printf("Invaid pointer, %s:%d", __FUNCTION__, __LINE__);\
            return(PM_E_INVALID_PARAM); \
        }\
    }while(0)

#define     VRRP_MCAST_ADDR         0xe0000012

#define     VRRP_IP_TTL             255

#define     AUTH_TYPE_INVALID         1
#define     AUTH_TYPE_MISMATCH        2
#define     AUTH_TYPE_FAILURE         3  

#define     VRRP_AUTH_NONE              0
#define     VRRP_AUTH_SIMPLE_PASSWORD   1
#define     VRRP_AUTH_MD5               2

/*
 * ---------------
 * VRRP Structures 
 * ---------------
 */

/* VRRP State. */
typedef enum {
  VRRP_STATE_INIT = 1,
  VRRP_STATE_BACKUP,
  VRRP_STATE_MASTER
} vrrp_state_t;

/* VRRP_IF State. */
typedef enum {
  VRRP_IF_UP      = 0x0001,
  VRRP_IF_DOWN    = 0x0002,
  VRRP_IF_BFD     = 0x0004,
  VRRP_IF_NON_BFD = 0x0008,
} vrrp_if_state_t;

/* Structure to hold the VRRP Packet. */
struct vrrp_advt_info 
{
  /* Version & Type. */
  uint8    ver_type;

  /* VrId. */
  uint8    vrid;

  /* Priority. */
  uint8    priority;

  /* Number of IP addresses to backup. */
  uint8    num_ip_addrs;

  /* Auth type. */
  uint8    auth_type;

  /* Advertisement interval. */
  uint8    advt_interval;

  /* VRRP Checksum. */
  uint16   cksum;

  /* Virtual IP address. */
  uint32   ip_addr;

  /* Auth data. */
  uint32   auth_data[2];
};

typedef struct vrrp_message_header_s
{
    uint8   dst_mac[6];
    uint8   src_mac[6];
    uint16  eth_type;
} vrrp_message_header_t;

typedef struct
{
#if _GLB_HOST_IS_LE
    uint8   ihl:4;                  /* header length */
    uint8   version:4;              /* version */
#else
    uint8   version:4;              /* version */
    uint8   ihl:4;                  /* header length */
#endif
    uint8   tos;                    /* type of service */
    uint16  total_length;           /* total length */
    
    uint16  identification;         /* identification */
    uint16  offset;                 /* fragment offset field */
    
    uint8   ttl;                    /* time to live */
    uint8   protocol;               /* protocol */
    uint16  checksum;               /* checksum */
    
    uint32  src_ip_addr;            /* src ip */    
    uint32  dest_ip_addr;           /* dest ip */
} vrrp_ip_header_t;


#define   SIZE_VRRP_ADVT   sizeof(struct vrrp_advt_info)

#define   SIZE_VRRP_AUTH   (2 * sizeof(uint32))


#define   VRRP_PKT_MIN_LEN  (SIZE_VRRP_ADVT + 20 + 14) 

#define   VRRP_STATE_MAX_STR_LEN  6 

#define VRRP_ENCODE_PUT(P, L)                                  \
        do {                                                   \
            sal_memcpy((void*)(pnt), (void*)(P), (L));         \
            pnt += (L);                                        \
            *size += (L);                                      \
        } while (0)

struct vrrp_in6_pseudo_hdr
{
    /* when support ipv6, should use sal_in6_addr as follows:
      sal_in6_addr in6_src;
      sal_in6_addr in6_dst;*/
    
    uint32       in6_src;
    uint32       in6_dst; 
    uint32       length;
    uint8        zero [3];
    uint8        nxt_hdr;
};

int32
vrrp_cmd_process_session(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
vrrp_start();

void
rt_cmd_set_vrrp_process(void);

#endif /* !__VRRP_H__ */

