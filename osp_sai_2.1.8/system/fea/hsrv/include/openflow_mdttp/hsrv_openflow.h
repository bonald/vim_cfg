#ifndef __HSRV_OPENFLOW_H__
#define __HSRV_OPENFLOW_H__

#include "sal.h"
#include "ctclib_list.h"
#include "ctclib_opf.h"

#include "glb_macro.h"
#include "ofp_hash.h"
#include "glb_openflow_define.h"

#include "ofp_types.h"
#include "ofp_group.h"
#include "ofp_meter.h"
#include "ofp_queue.h"
#include "ofp_const.h"
#include "ofp_macro.h"
#include "ofp_tunnel.h"
#include "ofp_flow.h"

#include "hsrv_msg_openflow.h"
#include "hsrv_openflow_ttp_common.h"
#include "hsrv_openflow_ttp_group.h"
#include "hsrv_openflow_ttp_flow.h"
#include "hsrv_openflow_ttp_meter.h"
#include "hsrv_openflow_ttp_port.h"
#include "hsrv_openflow_ttp_stats.h"
#include "hsrv_openflow_ttp_tunnel.h"

#include "ctc_ttp_drv_nexthop.h"
#include "ctc_ttp_drv_scl.h"
#include "ctc_ttp_drv_acl.h"
#include "ctc_ttp_drv_fdb.h"
#include "ctc_ttp_drv_ipuc.h"
#include "ctc_ttp_drv_mirror.h"
#include "ctc_ttp_drv_port.h"
#include "ctc_ttp_drv_stats.h"
#include "ctc_ttp_drv_policer.h"

#ifdef USW
#define HSRV_OPENFLOW_PHY_PORT_LABEL_BASE  1
#define HSRV_OPENFLOW_AGG_PORT_LABEL_BASE  100
#define HSRV_OPENFLOW_GLOBAL_LABEL_BASE  255
#endif
#ifdef GOLDENGATE
#define HSRV_OPENFLOW_PHY_PORT_LABEL_BASE  1
#define HSRV_OPENFLOW_AGG_PORT_LABEL_BASE  100
#define HSRV_OPENFLOW_GLOBAL_LABEL_BASE  255
#endif

#ifdef GREATBELT
#define HSRV_OPENFLOW_PHY_PORT_LABEL_BASE  900
#define HSRV_OPENFLOW_AGG_PORT_LABEL_BASE  1000
#define HSRV_OPENFLOW_GLOBAL_LABEL_BASE  1023
#endif

/*== hsrv openflow log & debug related macros ==*/

#define HSRV_OPENFLOW_LOG_EMERG(fmt, args...)   HSRV_LOG_EMERG(fmt, ##args)
#define HSRV_OPENFLOW_LOG_ALERT(fmt, args...)   HSRV_LOG_ALERT(fmt, ##args)
#define HSRV_OPENFLOW_LOG_CRIT(fmt, args...)    HSRV_LOG_CRIT(fmt, ##args)
#define HSRV_OPENFLOW_LOG_ERR(fmt, args...)     HSRV_LOG_ERR(fmt, ##args)
#define HSRV_OPENFLOW_LOG_WARN(fmt, args...)    HSRV_LOG_WARN(fmt, ##args)
#define HSRV_OPENFLOW_LOG_NOTICE(fmt, args...)  HSRV_LOG_NOTICE(fmt, ##args)
#define HSRV_OPENFLOW_LOG_INFO(fmt, args...)    HSRV_LOG_INFO(fmt, ##args)


/*== hsrv openflow memory related macros ==*/

#define HSRV_OPENFLOW_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW, size)
#define HSRV_OPENFLOW_CALLOC(size) HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW, size)
#define HSRV_OPENFLOW_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, ptr);  \
    ptr = NULL; \
}while(0)

#define HSRV_OPENFLOW_MEM_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    { \
        HSRV_OPENFLOW_LOG_ALERT("malloc/calloc failed, %s()@%d", __FUNCTION__, __LINE__); \
        return(HSRV_E_NO_MEMORY); \
    } \
}while(0)

/*== hsrv openflow sanity check macros ==*/

#define HSRV_OPENFLOW_PTR_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    { \
        HSRV_OPENFLOW_LOG_ERR("Invaid pointer, %s()@%d", __FUNCTION__, __LINE__); \
        return(HSRV_E_INVALID_PTR); \
    } \
}while(0)

/* hsrv openflow master */
struct hsrv_openflow_master_s
{
    
};
typedef struct hsrv_openflow_master_s hsrv_openflow_master_t;
extern hsrv_openflow_master_t *g_pst_openflow_master;


int32
hsrv_openflow_show_openflow(FILE *fp, char **argv, int32 argc);

int32
hsrv_openflow_show_ttp_openflow(FILE *fp, char **argv, int32 argc);

int32 
hsrv_openflow_module_start(void);

int32 
hsrv_openflow_module_init(hsrv_system_spec_info_t* p_system_info);

#endif

