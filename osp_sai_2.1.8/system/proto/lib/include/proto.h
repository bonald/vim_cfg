
#ifndef __PROTO_H__
#define __PROTO_H__

/* OS */
#include "proto_config.h"
#include "sal.h"

/* global defination */
#include "glb_macro.h"
#include "glb_const.h"
#include "glb_debug_define.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "glb_ip_define.h"
#include "glb_cpu_traffic_define.h"
#include "glb_acl_define.h"
#include "glb_stats_define.h"
#include "glb_flow_define.h"
#include "glb_qos_define.h"
#include "glb_ratelimit_define.h"
#include "glb_stm_define.h"
#include "glb_iptables_define.h"
#include "glb_ptp_define.h"
#include "glb_ns_port_forwarding.h"
#include "glb_arpacl_define.h"
#include "glb_g8131_define.h"
#include "glb_nat_define.h"

/* Basic Data Structure */
#include "ctclib_slist.h"
#include "ctclib_list.h"
#include "ctclib_hash.h"
#include "ctclib_queue.h"
#include "ctclib_opb.h"
#include "ctclib_if.h"
#include "ctclib_memory.h"
#include "ctc_task.h"
#include "genlog.h"

#include "proto_error.h"
#include "proto_debug.h"
#include "proto_type.h"
#include "proto_module.h"

#include "cdb_const.h"
#include "ctc_sock.h"
#include "cfg_cmd.h"

#include "glb_tp_oam.h"

/* Data Base */
#include "cdb_ds.h"
#include "cdb_sync.h"
#include "cdb.h"

#include "lib.h"
#include "lib_cmd.h"
#include "lib_spec.h"
#include "lib_load.h"

#include "ctclib_if.h"
#include "ctclib_opf.h"
#include "rdb_hiredis.h"

#include "ctclib_memory.h"
#include "ctc_sock.h"
#include "ipc.h"
#include "prefix.h"

#include "config_file_parser.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_lsp_aps_group_define.h"
#include "gen/tbl_lsp_aps_group.h"
#include "gen/tbl_pw_aps_group_define.h"
#include "gen/tbl_pw_aps_group.h"

#ifdef _KERNEL_VER_2_6_
#define NETNS_CMD_PREFIX "chvrf 64"
#else
#define NETNS_CMD_PREFIX "ip netns exec mgmt"
#endif

/*add by zhw for mc*/
#define PIM_MODE_NONE		0
#define PIM_MODE_DM			1
#define PIM_MODE_SM			2
#define RIP_DEFAULT_INSTANCE   1
/*end add zhw*/

#define RIPNG_DEFAULT_INSTANCE   1


#define DEFAULT_VRF_ID 0
//#define LDB_SYNC_TO_RDB 1

#define RIP_DEFAULT_INSTANCE   1

#define RIP_DEFAULT_INFORMATION_NONE         0
#define RIP_DEFAULT_INFORMATION        1

#define RIP_PASSIVE_IF_DEFAULT_NONE         0
#define RIP_PASSIVE_IF_DEFAULT_ALL        	1

#define RIPNG_DEFAULT_INFORMATION_NONE         0
#define RIPNG_DEFAULT_INFORMATION        1


#define OSPF_DEFAULT_INSTANCE   1
#define OSPF_AREA_AUTH_TYPE_MD5 1
#define OSPF_DEAD_INTERVAL_DEFAULT          40
#define OSPF_RIF_AUTH_TYPE_DEFAULT  0 /* djd */
#define OSPF_RIF_AUTH_TYPE_MD5      1
#define OSPF_RIF_AUTH_TYPE_SIMPLE   2 /* djd */
#define OSPF_HELLO_INTERVAL_DEFAULT         10

#define OSPF_NETWORK_TYPE_DEFAULT         0
#define OSPF_NETWORK_TYPE_POINT_TO_POINT         1

/* modified by wangjj for ospf, 2017-03-13 */
#define OSPF_DEFAULT_ORIGINATE_NONE         0
#define OSPF_DEFAULT_ORIGINATE_ZEBRA        1
#define OSPF_DEFAULT_ORIGINATE_ALWAYS       2

/* copy from Zebra route's types. */
#define RT_ROUTE_SYSTEM               0
#define RT_ROUTE_KERNEL               1
#define RT_ROUTE_CONNECT              2
#define RT_ROUTE_STATIC               3
#define RT_ROUTE_RIP                  4
#define RT_ROUTE_RIPNG                5
#define RT_ROUTE_OSPF                 6
#define RT_ROUTE_OSPF6                7
#define RT_ROUTE_ISIS                 8
#define RT_ROUTE_BGP                  9
#define RT_ROUTE_PIM                  10
#define RT_ROUTE_HSLS                 11
#define RT_ROUTE_OLSR                 12
#define RT_ROUTE_BABEL                13
#define RT_ROUTE_MAX                  14

#define FEA_INIT_DONE_FILE      "/tmp/fea_done"
#define BHM_WARMBOOT_FILE       "/tmp/warmboot"

static inline void todo_assert_failed(const char *expr, const char *file, int line)
{
    sal_log(SAL_LL_FATAL, file, line, "TODO Assertion failed: %s", expr);
    abort();
}

#define TODO_ASSERT(e) if (!(e)) todo_assert_failed(#e, __FILE__, __LINE__)

#if 1
#define PM_E_RETURN(op) \
{ \
    rc = (op); \
    if (rc < 0) \
    { \
        return (rc); \
    } \
}
#else
#define PM_E_RETURN(op) \
{ \
    rc = (op); \
    if (rc < 0) \
    { \
        SAL_ASSERT(0); \
        return (rc); \
    } \
}
#endif

#define PM_E_RETURN_LOG(op) \
{ \
    rc = (op); \
    if (rc < 0) \
    { \
        log_sys(M_MOD_LIB, E_ERROR, "PM error at Func %s() Line:%d, rc = %d\n", __FUNCTION__, __LINE__, rc); \
        return (rc); \
    } \
}

const char*
pm_err_desc(int32 error);

#endif /* !__PROTO_H__ */

