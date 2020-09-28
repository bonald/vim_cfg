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
#include "hsrv_openflow_nexthop.h"
#include "hsrv_openflow_group.h"
#include "hsrv_openflow_meter.h"
#include "hsrv_openflow_queue.h"
#include "hsrv_openflow_flow.h"
#include "hsrv_openflow_port.h"
#include "hsrv_openflow_stats.h"
#include "hsrv_openflow_parser.h"
#include "hsrv_openflow_linkagg.h"
#include "hsrv_openflow_tunnel.h"
#include "hsrv_openflow_bond.h"
#include "hsrv_openflow_stub.h"
#include "hsrv_openflow_mpls.h"
#include "hsrv_openflow_inband.h"
#ifdef V580_openflow
#include "hsrv_openflow_efd.h"
#endif
#include "hsrv_openflow_port.h"

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

int32
hsrv_openflow_show_openflow(FILE *fp, char **argv, int32 argc);

int32 hsrv_openflow_post_startup();

int32 hsrv_openflow_module_start(void);

int32 hsrv_openflow_module_init(hsrv_system_spec_info_t* p_system_info);

#ifdef CONFIG_TPOAM
/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-19 */
int32 
hsrv_openflow_tpoam_start(void);

uint32
hsrv_openflow_tpoam_api_get_pw_mode(void);

int32
hsrv_openflow_tpoam_api_get_oam_session_id(uint32 oam_type, uint32 key, uint32 *p_oam_session_id);

int32
hsrv_openflow_tpoam_api_get_nhid_offset(uint32 oam_type, uint32 key, uint32 *p_nh_id, uint32 *p_nh_offset, uint32* p_lsp_aps, uint32* working);

int32
hsrv_openflow_tpoam_api_get_dest_gport(uint32 ifindex, uint16 *p_dest_gport, uint16 *p_src_gport);
/* liwh end */
#endif/* !CONFIG_TPOAM */
 
#endif /* __HSRV_OPENFLOW_H__ */
