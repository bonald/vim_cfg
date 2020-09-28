#ifndef __CHSM_H__
#define __CHSM_H__

#include "sal.h"
#include "lib_image.h"


#define CHSM_INVALID_SLOT_NO    -1
#define CHSM_MAC_ADDR_LEN       6
#define CHSM_IMAGE_COMPILE_TIME_LEN 32
#define ZONE_CONFIG_PATH        "/tmp/zone.conf"
#define CHSM_DEBIAN_BOOT_CONFIG_FILE    "/etc/cnos_url.conf"
#define CHSM_DEBIAN_UIMAGE_PATH         "file:///mnt/flash/boot/uImage"
#define ZONE_LINE_BUF   256
#define CHSM_MAX_ZONE_NAME_LEN 200
#define MAX_PROC_LEN            128
#define CHSM_FLAG_DST     1
#define CHSM_FLAG_NO_DST  0
#define CHSM_TIME_DST                0x02
#define CHSM_ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define CHSM_MAX_TIME_LEN 200
#define chsm_sal_time_sys_current(z) time(z)
#define CHSM_M_TIMEZONE_NAME_MAXSIZE      32
#define CHSM_M_TIMEZONE_NAME_MINSIZE      3 
#define CHSM_TIME_ZONE               0x01
#define CHSM_TIME_ERROR_RANGE 1  /*seconds*/
#define sal_time_sys_current(z) time(z)
#if HAVE___SECURE_GETENV && !defined(__LCLINT__)
#define	chsm_getenv(_s)	__secure_getenv(_s)
#endif
typedef enum tagZoneOP
{
   OP_NEGATIVE = 0,
   OP_POSITIVE
}E_ZoneOP;
/*process control structure*/

enum chsm_chassis_type_e
{
    CHSM_CHASSIS_PIZZA_BOX,
    CHSM_CHASSIS_CHASSIS,
    CHSM_CHASSIS_STACKING,    
    CHSM_CHASSIS_TYPE_MAX    
};

typedef struct
{    
    int32   low;
    int32   high;
    int32   crit;
} chsm_tmpr_cfg_t;

typedef struct
{    
    int32   bottom;
    int32   low;
    int32   high;
    int32   full;
} chsm_fan_cfg_t;

#define CHSM_TMPR_HYST 2

#define CHSM_DEFAULT_LOW_TMPR    5
#define CHSM_DEFAULT_HIGH_TMPR   65
#define CHSM_DEFAULT_CRIT_TMPR   80

typedef struct
{    
    uint8   enable;
    uint32   power_max;
} chsm_poe_cfg_t;

#define module_control_is_enable_stack() FALSE

/* TODO */
#define CHSM_LOG_USER(eSeverity, code, ...)

#define CHSM_DBG_INFO(FMT, ARGS...)                     \
do {                                     \
    log_sys(M_MOD_CHSM, E_INFORMATIONAL,  FMT,  ##ARGS);   \
} while (0)

#define CHSM_DBG_WARN(  FMT, ARGS...)                     \
do {                                     \
    log_sys(M_MOD_CHSM, E_WARNING,  FMT,  ##ARGS);   \
} while (0)

#define CHSM_DBG_ERR(FMT, ARGS...)                     \
do {                                     \
    log_sys(M_MOD_CHSM, E_ERROR,  FMT,  ##ARGS);   \
} while (0)

#define CHSM_DBG_CRIT(FMT, ARGS...)                     \
do {                                     \
    log_sys(M_MOD_CHSM, E_CRITICAL,  FMT,  ##ARGS);   \
} while (0)

#define CHSM_ALARM_TEMP     1<<0
#define CHSM_ALARM_PSU      1<<1
#define CHSM_ALARM_FAN      1<<2
#define CHSM_ALARM_ALL      0xff
#define CLOCK_CHANGE_DELTA_FILE   "/tmp/clockchange.conf"

typedef enum
{
    CHSM_VENDOR_TYPE_CENTEC = 1,
    CHSM_VENDOR_TYPE_MAX
} chsm_verdor_type_e;

#define CHSM_CTC_CHK_SLOT(_SLOT)                                                     \
do {                                                                            \
    if (((_SLOT) = dev_get_logic_slot()) == CHSM_INVALID_SLOT_NO)                    \
    {                                                                           \
        CHSM_DBG_ERR("%s invalid slot no", __FUNCTION__);    \
        return -1;                                                              \
    }                                                                           \
}while(0)

#define CHSM_CTC_CHK_PTR(_PTR)                                                   \
do {                                                                        \
    if ((_PTR) == NULL)                                                     \
    {                                                                       \
        CHSM_DBG_ERR("%s NULL pointer", __FUNCTION__);   \
        return -1;                                                          \
    }                                                                       \
}while(0)


int32
chsm_lcmtlk_init();
int32
chsm_set_tmpr_threshold(chsm_tmpr_cfg_t *tmpr_cfg);
int32
chsm_cmd_init();
int32
chsm_init(uint16 daemon_mode);

typedef struct chsm_master_s
{
    int32 chsm_srv_sock;
} chsm_master_t;

int32
chsm_start_phase2();

#endif /* !__CHSM_H__ */
