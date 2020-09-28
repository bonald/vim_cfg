/****************************************************************************
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Justin
 * Date          : 2007-011-14
 * Reason        : First Create.
 ****************************************************************************/
#include "gen/tbl_ipsg_fib_define.h"
#include "gen/tbl_ipsg_fib.h"
#include "gen/tbl_dhcbinding_define.h"
#include "gen/tbl_dhcbinding.h"
#include "gen/tbl_dhcsnooping_define.h"
#include "gen/tbl_dhcsnooping.h"

#define IPSOUR_LOG_DEBUG(typeenum, fmt, args...) \
do {\
    PM_DEBUG_OUT_INFO(IPSOUR, ipsg, typeenum, fmt, ##args);\
}while(0)

#define IPSOUR_DEBUG_FUNC(typeenum) \
    IPSOUR_LOG_DEBUG(typeenum, "Enter into %s.", __FUNCTION__)

#define IPSOUR_DEBUG(typeenum, fmt, args...) \
do {\
    IPSOUR_LOG_DEBUG(typeenum, fmt, ##args);\
}while(0)

enum ipsour_return
{
      IPSOUR_OK = 0,
      IPSOUR_NULL_POINTER,
      IPSOUR_DUPLICATE,
      IPSOUR_NOT_MATCH,
      IPSOUR_NOT_NULL,
      IPSOUR_ALLOC_ERROR = 5,
      IPSOUR_REACH_PORT_MAX,
      IPSOUR_REACH_TOTAL_MAX,
      IPSOUR_NO_SUCH_ITEM,
      IPSOUR_HAL_FAILED,
      IPSOUR_FAILED,
      IPSOUR_NO_SUCH_TYPE = 10,
      /*for bug 4360 specify duplicate type*/
      IPSOUR_DUPLICATE_MAC,
      IPSOUR_DUPLICATE_IP,
#ifdef HAVE_IPV6
      IPV6SOUR_REACH_TOTAL_MAX,
#endif
      IPSOUR_MAX,
};


int32
ipsg_cmd_process_ip_source_guard(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ipsg_cmd_process_ipsg_mac(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ipsg_cmd_process_ipsg_ip(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ipsour_type_change_process(uint32 new_type, uint32 old_type, tbl_interface_t *p_db_if);

int32
ipsg_cmd_process_ipsg_dhcs(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
ipsour_update_item_by_interface2(tbl_interface_t *p_db_if, u_int8_t new_type);

int
ipsour_static_add_check_static(tbl_ipsg_fib_t *p_ipsg_fib);

int
ipsour_static_del_check_static(tbl_ipsg_fib_t *p_ipsg_fib);

int32 
ipsour_if_delete_cb(tbl_interface_t *p_db_if);

int32 
ipsour_if_delete_cb2(tbl_interface_t *p_db_if);

int32 
ipsour_dynamic_add_callback(tbl_dhcbinding_t *pItem, tbl_interface_t *p_db_if);

int32
ipsour_dynamic_del_callback(tbl_dhcbinding_t *pItem, tbl_interface_t *p_db_if);

int32 
ipsour_cmd_add_item(prefix_t *ip_addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index, char *ifname, u_int8_t cType);

int32 
ipsour_cmd_del_item(prefix_t *ip_addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index, char *ifname, u_int8_t old_type);

int32
ipsg_cmd_del_static_binding_callback(tbl_ipsg_fib_t *p_ipsg_fib, tbl_interface_t *p_db_if, uint32 show_log);

int32
ipsg_cmd_add_db_entry(prefix_t *ip_addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index, char *ifname, u_int8_t new_type);

int32
ipsg_cmd_del_db_entry(prefix_t *ip_addr, u_int8_t *mac, vid_t vlan_id, u_int32_t if_index, char *ifname, u_int8_t new_type);

int32
ipsour_port_mode_update(uint32 cType, tbl_interface_t *p_db_if);

int32
ipsour_port_enable(uint32 cType, tbl_interface_t *p_db_if);

int32 
ipsour_port_disable(uint32 cType, tbl_interface_t *p_db_if);

int32
ipsour_ipsg_count_inc();

int32
ipsour_ipsg_count_dec();

uint32
ipsour_ipsg_count_get_max();

uint32
ipsour_ipsg_count_get();



int32
ipsour_api_init();

#define IPSOUR_DBG_INFO(FMT, ARGS...) \
do { \
    log_sys(M_MOD_IPSOUR, E_INFORMATIONAL, FMT,  ##ARGS); \
} while (0)

#define IPSOUR_DBG_WARN(FMT, ARGS...) \
do { \
    log_sys(M_MOD_IPSOUR, E_WARNING,  FMT,  ##ARGS); \
} while (0)

#if 0
#define IPSOUR_DBG_ERR(FMT, ARGS...) \
do { \
    log_sys(M_MOD_IPSOUR, E_ERROR,  FMT,  ##ARGS); \
} while (0)
#endif

#define IPSOUR_DBG_CRIT(FMT, ARGS...) \
do { \
    log_sys(M_MOD_IPSOUR, E_CRITICAL,  FMT,  ##ARGS); \
} while (0)

#define IPSOUR_DBG_DEBUG(FMT, ARGS...) \
do { \
    log_sys(M_MOD_IPSOUR, E_DEBUG,  FMT,  ##ARGS); \
} while (0)


/*under controlled by debug ipsg cmd start*/
#define IPSOUR_DBG_ERR(FMT, ARGS...) \
do { \
    IPSOUR_DEBUG(IPSG_ERROR, FMT, ##ARGS);\
} while (0)

#define IPSOUR_DBG_EVENT(FMT, ARGS...) \
do { \
    IPSOUR_DEBUG(IPSG_EVENT, FMT, ##ARGS);\
} while (0)
/*end*/
