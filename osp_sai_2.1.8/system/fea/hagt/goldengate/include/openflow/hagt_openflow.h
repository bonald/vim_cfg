#ifndef __HAGT_OPENFLOW_H__
#define __HAGT_OPENFLOW_H__

#include "ctclib_hash.h"
#include "ofp_hash.h"
#include "glb_stats_define.h"
#include "glb_cpu_traffic_define.h"
#include "glb_openflow_define.h"

#include "hagt_openflow_port.h"
#include "hagt_openflow_flow.h"
#include "hagt_openflow_group.h"
#include "hagt_openflow_meter.h"
#include "hagt_openflow_nexthop.h"
#include "hagt_openflow_stats.h"
#include "hagt_openflow_parser.h"
#include "hagt_openflow_linkagg.h"
#include "hagt_openflow_tunnel.h"
#include "hagt_openflow_bond.h"
#include "hagt_openflow_queue.h"
#include "hagt_openflow_efd.h"

/*== hagt openflow sanity check related macros ==*/

/*******************************************************************
*debug
********************************************************************/
#define HAGT_OPENFLOW_LOG_EMERG(fmt, args...)   HAGT_LOG_EMERG(fmt, ##args)
#define HAGT_OPENFLOW_LOG_ALERT(fmt, args...)   HAGT_LOG_ALERT(fmt, ##args)
#define HAGT_OPENFLOW_LOG_CRIT(fmt, args...)    HAGT_LOG_CRIT(fmt, ##args)
#define HAGT_OPENFLOW_LOG_ERR(fmt, args...)     HAGT_LOG_ERR(fmt, ##args)
#define HAGT_OPENFLOW_LOG_WARN(fmt, args...)    HAGT_LOG_WARN(fmt, ##args)
#define HAGT_OPENFLOW_LOG_NOTICE(fmt, args...)  HAGT_LOG_NOTICE(fmt, ##args)
#define HAGT_OPENFLOW_LOG_INFO(fmt, args...)    HAGT_LOG_INFO(fmt, ##args)

#define HAGT_OPENFLOW_PTR_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    { \
        HAGT_OPENFLOW_LOG_ERR("Invaid pointer, %s()@%d", __FUNCTION__, __LINE__); \
        return(HAGT_E_INVALID_PTR); \
    } \
}while(0)


#define HAGT_OPENFLOW_MEM_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    { \
        HAGT_OPENFLOW_LOG_ALERT("malloc/calloc failed, %s()@%d", __FUNCTION__, __LINE__); \
        return(HAGT_E_NO_MEMORY); \
    } \
}while(0)

/*== hagt openflow memory related macros ==*/

#define HAGT_OPENFLOW_CALLOC(size) HAGT_CALLOC(MEM_FEA_HAGT_OPENFLOW, size)
#define HAGT_OPENFLOW_MALLOC(size) HAGT_MALLOC(MEM_FEA_HAGT_OPENFLOW, size)
#define HAGT_OPENFLOW_FREE(ptr) \
do { \
    HAGT_FREE(MEM_FEA_HAGT_OPENFLOW, ptr);  \
    ptr = NULL; \
}while(0)

/*== hagt openflow log & debug related macros ==*/

#define HAGT_OPENFLOW_LOG_EMERG(fmt, args...)   HAGT_LOG_EMERG(fmt, ##args)
#define HAGT_OPENFLOW_LOG_ALERT(fmt, args...)   HAGT_LOG_ALERT(fmt, ##args)
#define HAGT_OPENFLOW_LOG_CRIT(fmt, args...)    HAGT_LOG_CRIT(fmt, ##args)
#define HAGT_OPENFLOW_LOG_ERR(fmt, args...)     HAGT_LOG_ERR(fmt, ##args)
#define HAGT_OPENFLOW_LOG_WARN(fmt, args...)    HAGT_LOG_WARN(fmt, ##args)
#define HAGT_OPENFLOW_LOG_NOTICE(fmt, args...)  HAGT_LOG_NOTICE(fmt, ##args)
#define HAGT_OPENFLOW_LOG_INFO(fmt, args...)    HAGT_LOG_INFO(fmt, ##args)


enum hagt_openflow_stats_resource_type_e
{
    HAGT_OPENFLOW_STATS_RESOURCE_TYPE_FLOW = 0,
    HAGT_OPENFLOW_STATS_RESOURCE_TYPE_METER,
    HAGT_OPENFLOW_STATS_RESOURCE_TYPE_GROUP,

    HAGT_OPENFLOW_STATS_RESOURCE_TYPE_MAX
};
typedef enum hagt_openflow_stats_resource_type_e hagt_openflow_stats_resource_type_t;

struct hagt_openflow_stats_info_e
{
    uint32 stats_id;
    uint32 rsv_1;
    glb_openflow_stats_t stats;
};
typedef struct hagt_openflow_stats_info_e hagt_openflow_stats_info_t;

struct hagt_openflow_slave_info_e
{
    uint8   tid;                                  /* bond id */
    uint16  gport;                                /* slave gport */
};
typedef struct hagt_openflow_slave_info_e hagt_openflow_slave_info_t;

struct hagt_openflow_statsid_e
{
    uint32 stats_id;
    uint32 ctc_stats_id;
};
typedef struct hagt_openflow_statsid_e hagt_openflow_statsid_t;

struct hagt_openflow_entryid_e
{
    uint32 entry_id;
    uint32 ctc_entry_id;

    /* for hybrid fdb_based_flow */
    mac_addr_t mac;
    uint16     fid;
};
typedef struct hagt_openflow_entryid_e hagt_openflow_entryid_t;

struct hagt_openflow_policerid_e
{
    uint32 meter_id;
    uint32 ctc_policer_id;
};
typedef struct hagt_openflow_policerid_e hagt_openflow_policerid_t;

/* hagt openflow master */
struct hagt_openflow_master_s 
{
    uint32 more;

    ctclib_hash_t* pst_flow_stats_info_hash;
    sal_mutex_t *p_flow_stats_mutex;
    sal_task_t*  p_flow_stats_thread;
    uint32 p_flow_poll_cnt;

    ctclib_hash_t* pst_meter_stats_info_hash;
    sal_mutex_t *p_meter_stats_mutex;
    sal_task_t*  p_meter_stats_thread;
    uint32 p_meter_poll_cnt;

    ctclib_hash_t* pst_group_stats_info_hash;
    ctclib_hash_t* pst_mcast_mem_hash;
    sal_mutex_t *p_group_stats_mutex;
    sal_task_t*  p_group_stats_thread;
    uint32 p_group_poll_cnt;

    ctclib_hash_t* pst_slave_hash;

    ctclib_hash_t* pst_openflow_statsid_hash;

    ctclib_hash_t* pst_openflow_entryid_hash;

    ctclib_hash_t* pst_openflow_policerid_hash;
    
};
typedef struct hagt_openflow_master_s hagt_openflow_master_t;

extern hagt_openflow_master_t *g_pst_hagt_openflow_master;

int32
hagt_openflow_init (hagt_master_t* p_hagt_master);

#endif /* !__HAGT_OPENFLOW_H__ */
