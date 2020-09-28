
#ifndef __HAGT_H__
#define __HAGT_H__

#include "sal.h"
#include "ctc_error.h"
#include "hagt_error.h"
#include "hagt_debug.h"
#include "hagt_message.h"
#include "ctclib_memory.h"
#include "ctc_task.h"
#include "ctc_hash.h"

#define HAGT_MALLOC(type,size) XMALLOC(type,size)
#define HAGT_CALLOC(type,size) XCALLOC(type,size)
#define HAGT_FREE(type,ptr)    XFREE(type,ptr)

typedef enum hagt_module_id_e
{
    HAGT_MODULE_COMM = 0,
    HAGT_MODULE_INTERFACE, /*Added by ychen for interface, 2010-08-03*/
    HAGT_MODULE_VLAN,
    HAGT_MODULE_LINKAGG,
    HAGT_MODULE_INTR,   /* Add by kcao for Interrupt. 2010.07.30 */
    HAGT_MODULE_CPU_TRAFFIC,/* Add by yanxa for cpu traffic. 2010-08-02*/
    HAGT_MODULE_STATS,  /* Added by yanxa for stats, 2010-11-22 */
    HAGT_MODULE_PTP,    /* Add by kcao for PTP. 2010.07.25 */
    HAGT_MODULE_DEBUG,/*Add by huangxt for debug. 2010.07*/
    HAGT_MODULE_FDB,
    HAGT_MODULE_MIRROR,
    HAGT_MODULE_RATELIMIT,
    HAGT_MODULE_ACL,
    HAGT_MODULE_QOS,    /* Added by rongl for qos, 2010-09-18 */
    HAGT_MODULE_PBR,    /* add by chenzhonghua for bug 15103 policy based routing, 2011-06-29*/
    HAGT_MODULE_IPT,    /* add by huwx for iptables, 2011-07-27 */
    HAGT_MODULE_VLANCLASSIFIER,
    HAGT_MODULE_VLAN_MAPPING,
    HAGT_MODULE_DOT1X,  /* Added by Huangxt for dot1x, 2010-10-20 */
    HAGT_MODULE_IP_BINDING,  /* Added by Huangxt for ip_source_guard, 2010-11-8 */
    HAGT_MODULE_IPUC,
    HAGT_MODULE_SFLOW,
    HAGT_MODULE_VLAN_SECURITY,
    HAGT_MODULE_PHY_LOOPBACK,
    HAGT_MODULE_SYNCE,
    HAGT_MODULE_CFM, /* modified by liwh to support cfm for bug 14710 */
    HAGT_MODULE_TPOAM, /* Added by kcao for MPLS-TP OAM develop, bug 15633, 2011-07-28 */
    HAGT_MODULE_IVI,
    HAGT_MODULE_OPENFLOW,    /* Added by Yan Xing'an for OpenFlow, 2015-01-05 */
    HAGT_MODULE_MAX
} hagt_module_id_t;
enum hagt_feature_chk_e
{
    HAGT_FEA_OAM_ENABLED,
    HAGT_FEA_IPV6_ENABLED,
};
typedef enum hagt_feature_chk_e hagt_feature_chk_t;

typedef struct
{
    uint32  module_id; /* decides the operation order for each module when DB-SYN*/
    void   *p_data_info;/*module detail data information*/
} hagt_module_info_t;

typedef struct hagt_feature_check_s
{
    /*added by ychen in 2013-09-04 for ftm develop*/
    bool is_oam_supported;
    bool is_ipv6_supported;
    /*added by ychen in 2013-10-10 for support block1 entry number larger than block0*/
    bool use_block0_for_aclv4; 
    bool use_block0_for_aclv6;
    uint32 max_external_nhid;
    /*ychen end*/
} hagt_feature_check_t;

typedef struct
{
    hagt_module_info_t *past_module_info[HAGT_MODULE_MAX];
    void* p_messge_info;
    uint16 logic_slot_no;
    uint16 phy_slot_no;
    uint32 gchip_id;
    hagt_feature_check_t fea_chk;
    uint16 stm_mode;
    ctc_hash_t *openflow_port_mac_hash;
    uint8 udf_enable;
} hagt_master_t;

#define HAGT_OPENFLOW_FLOW_UDF_ENABLE     g_pst_hagt_master->udf_enable

uint16 hagt_get_logic_slotid();
void hagt_set_logic_slotid(uint16 logic_slot);
uint16 hagt_get_phy_slotid();
void hagt_set_phy_slotid(uint16 phy_slot);
void hagt_set_gchip_id(uint32 gchip_id);
uint32 hagt_get_gchip_id();
int32 hagt_is_enable_stack();
/*added by ychen in 2013-09-04 for ftm develop*/
bool hagt_gb_get_feature_enabled(uint32 fea);
int32 hagt_fiber_port_link_status_change_isr_init(hagt_master_t* pst_hagt_master);

int32
hagt_modules_start();


static inline int32
hagt_error_ctc_to_glb(int32 ctc_error)
{
    switch (ctc_error)
    {
    case CTC_E_NONE:
        return GLB_E_NONE;

    case CTC_E_NO_RESOURCE:
    case CTC_E_FDB_HASH_CONFLICT:
    case CTC_E_IPUC_TOO_MANY_FRAGMENT: /*added by ychen for bug 25368 in 2013-10-17*/
        return GLB_E_RESOURCE_FULL;

    default:
        HAGT_LOG_ERR("Unknown SDK error %d %s %d", ctc_error, __FUNCTION__, __LINE__);
        return ctc_error;
    }
}

#endif /* !__HAGT_H__ */

