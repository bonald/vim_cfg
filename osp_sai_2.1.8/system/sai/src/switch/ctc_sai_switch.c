#include <sal.h>
#include <sai.h>
#include <memory.h>
#include "ctc_api.h"
#include "ctc_init.h"
#include "ctc_warmboot.h"
#include <saitypes.h>
#include <saihash.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_switch.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_port.h>
#include <ctc_sai_hash.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_copp.h>
#include <ctc_sai_warmboot.h>

#include <ctc_app_cfg_datapath_profile.h>

#undef __MODULE__
#define __MODULE__ SAI_API_SWITCH

#ifdef _GLB_UML_SYSTEM_
#undef CTC_ETC_BASE_DIR
#define CTC_ETC_BASE_DIR                    "/centec_switch"
#else
#undef CTC_ETC_BASE_DIR
#define CTC_ETC_BASE_DIR                    "/switch"
#endif
#define SAI_STM_PROFILE_FILE_DEFAULT        CTC_ETC_BASE_DIR"/etc/mem_profile_default.cfg"
#define SAI_STM_PROFILE_FILE_ACCESS         CTC_ETC_BASE_DIR"/etc/mem_profile_L2.cfg"
#define SAI_STM_PROFILE_FILE_IPUCV4         CTC_ETC_BASE_DIR"/etc/mem_profile_L3.cfg"
#define SAI_STM_PROFILE_FILE_IPUCV6         CTC_ETC_BASE_DIR"/etc/mem_profile_IPv6.cfg"
#define SAI_STM_PROFILE_FILE_HYBRID_DEFAULT CTC_ETC_BASE_DIR"/etc/mem_profile_hybrid_default.cfg"
#define SAI_STM_PROFILE_FILE_TAP_DEFAULT    CTC_ETC_BASE_DIR"/etc/mem_profile_tap_default.cfg"

#define SAI_FDB_DEFAULT_AGING_TIME 300

#define SAI_SWITCH_TRUNCTION_LENGTH_MIN 64
#define SAI_SWITCH_TRUNCTION_LENGTH_MAX 144

uint32 hybrid_profile_type = 0;
uint32 ftm_info_profile_type_gb = 0;
uint32 ftm_info_profile_type_d2 = 0;
uint32 ftm_info_profile_type_tm = 0;
uint32 ipv6_packet_paser = 0;

extern sai_status_t ctc_sai_qos_map_set_switch_qos_map(_In_ const sai_attribute_t* attr);
extern sai_status_t ctc_sai_tap_group_set_trunction_len(_In_  const sai_attribute_t *attr);
extern int32 ctc_packet_receive_from_sdk(ctc_pkt_rx_t* p_pkt_rx);
extern int32 ctc_master_cli(int32 is_ctc_shell);
extern int32 ctc_cli_read(int32 ctc_shell_mode);

int32  cut_through_en = 0;
int32  memory_check_pktdiscard = 0;
static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_SWITCH_ATTR_PORT_NUMBER,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_PORT_LIST,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_CPU_PORT,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_FDB_TABLE_SIZE,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_OPER_STATUS,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_MAX_TEMP,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID,
        .type   = SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_SWITCHING_MODE,
        .type   = SAI_ATTR_FLAG_DEFAULT | SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
        .default_value.u32 = SAI_SWITCHING_MODE_STORE_AND_FORWARD,
    },
    {
        .id     = SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_SRC_MAC_ADDRESS,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES,
        .type   = /*SAI_ATTR_FLAG_DEFAULT |*/ SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
        .default_value.u32 = 0,
    },
    {
        .id     = SAI_SWITCH_ATTR_FDB_AGING_TIME,
        .type   = SAI_ATTR_FLAG_DEFAULT | SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
        .default_value.u32 = 0,
    },
    {
        .id     = SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_SWITCH_ATTR_ECMP_MAX_PATHS,
        .type   = SAI_ATTR_FLAG_DEFAULT | SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
        .default_value.u32 = 64,
    },
    {
        .id     = SAI_SWITCH_ATTR_PORT_BREAKOUT,
        .type   = SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};


sai_status_t
ctc_sai_switch_set_monitor_buffer_enable(
    _In_  const sai_attribute_t *attr);
sai_status_t
ctc_sai_switch_set_monitor_buffer_threshold(
    _In_  const sai_attribute_t *attr);
sai_status_t
ctc_sai_switch_set_monitor_latency_range(
    _In_  const sai_attribute_t *attr);

static int32_t
__get_port_number_cb(
    _In_  sai_object_id_t   port_id,
    _In_  void              *pdata)
{
    uint32_t* pport_number = (uint32_t* )pdata;

    (*pport_number)++;

    return 0;
}

static sai_status_t
__get_port_list_cb(
    _In_  sai_object_id_t   port_id,
    _In_  void              *pdata)
{
    sai_attribute_t*   attr = (sai_attribute_t*)pdata;

    if(attr->value.objlist.count <= attr->id)
    {
        return -1;
    }

    attr->value.objlist.list[attr->id]= port_id;
    attr->id++;

    return 0;
}

static int32_t
__set_cut_through_en(uint8 enable)
{
    if (enable) {
        cut_through_en = 1;
    } else {
        cut_through_en = 0;
    }

    return 0;
}

sai_status_t
ctc_sai_get_cut_through_en()
{
    return cut_through_en;
}

sai_status_t
ctc_sai_switch_get_port_number(
    _Inout_ sai_attribute_t *attr)
{
    sai_status_t                ret         = SAI_STATUS_SUCCESS;
    uint32_t                    port_number = 0;

    ctc_sai_for_each_port_object(__get_port_number_cb, &port_number);
    attr->value.u32 = port_number;
    return ret;
}

sai_status_t
ctc_sai_switch_get_port_list_number(
    _Inout_  sai_attribute_t    *attr)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;

    attr->id    = 0;
    ctc_sai_for_each_port_object(__get_port_list_cb,attr);
    attr->value.objlist.count = attr->id;
    attr->id    = SAI_SWITCH_ATTR_PORT_LIST;

    return ret;
}

sai_status_t
ctc_sai_switch_set_switch_mode(
    _In_  const sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_get_switch_mode(
    _Inout_  sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}


sai_status_t
ctc_sai_switch_get_cpu_port(
    _Inout_  sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;

    return ret;
}


sai_status_t
ctc_sai_switch_get_vr_max_routers(
    _Inout_  sai_attribute_t *attr)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    attr->value.u32 = p_sai_glb->vrf_max_count;

    return ret;
}

sai_status_t
ctc_sai_switch_set_bcast_cpu_flood_enable(
    _In_  const sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_get_bcast_cpu_flood_enable(
    _Inout_  sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_set_mcast_cpu_flood_enable(
    _In_  const sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_get_mcast_cpu_flood_enable(
    _Inout_  sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_set_violation_ttl1_action(
    _In_  const sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_get_violation_ttl1_action(
    _Inout_  sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_set_src_mac_address(
    _In_  const sai_attribute_t *attr)
{
    mac_addr_t                  mac;

/* system modified by liwh for bug 53010, 2019-07-29 */
#ifdef GREATBELT
    ctc_oam_property_t prop;
   
    sal_memset(&prop, 0, sizeof(ctc_oam_property_t));

    prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;
    prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_BRIDGE_MAC;
    prop.u.y1731.p_value = &attr->value.mac;
    ctc_sai_get_error_from_sdk_error(ctc_oam_set_property(&prop));

    ctc_port_mac_prefix_t  mac_prefix;

    sal_memset(&mac_prefix, 0, sizeof(ctc_port_mac_prefix_t));
    CTC_SET_FLAG(mac_prefix.prefix_type, CTC_PORT_MAC_PREFIX_MAC_0);
    sal_memcpy(mac_prefix.port_mac[0], attr->value.mac, sizeof(mac_addr_t));
    ctc_sai_get_error_from_sdk_error(ctc_port_set_port_mac_prefix(&mac_prefix));
#endif
/* liwh end */

    sal_memcpy(mac,attr->value.mac,sizeof(sai_mac_t));

    return ctc_sai_get_error_from_sdk_error(ctc_l3if_set_router_mac(mac));;
}

sai_status_t
ctc_sai_switch_get_src_mac_address(
    _Inout_  sai_attribute_t *attr)
{
    int32_t                     sdk_ret = CTC_E_NONE;
    mac_addr_t                  mac;

    sal_memset(mac,0,sizeof(sai_mac_t));

    CTC_SAI_ERROR_GOTO(ctc_l3if_get_router_mac(mac),sdk_ret,out);

    sal_memcpy(attr->value.mac,mac,sizeof(sai_mac_t));

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_max_learned_address(
    _In_  const sai_attribute_t *attr)
{
    sai_status_t                ret     = SAI_STATUS_SUCCESS;
    int32_t                     sdk_ret = CTC_E_NONE;
    ctc_security_learn_limit_t  limit;

    sal_memset(&limit,0,sizeof(limit));

    limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM;
    CTC_SAI_ERROR_GOTO(ctc_mac_security_get_learn_limit(&limit),ret,out);

    limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM;
    limit.limit_num  = attr->value.u32;
    limit.limit_action = CTC_MACLIMIT_ACTION_FWD;

    CTC_SAI_ERROR_GOTO(ctc_mac_security_set_learn_limit(&limit),ret,out);

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_get_max_learned_address(
    _Inout_  sai_attribute_t *attr)
{
    int32_t                     sdk_ret = CTC_E_NONE;
    ctc_security_learn_limit_t  limit;

    sal_memset(&limit,0,sizeof(limit));

    limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM;
    CTC_SAI_ERROR_GOTO(ctc_mac_security_get_learn_limit(&limit),sdk_ret,out);

    attr->value.u32 = limit.limit_num;
out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_aging_time(
    _In_  const sai_attribute_t *attr)
{
    uint32_t            sec     = 0;
    int32_t             sdk_ret = CTC_E_NONE;

    sec = attr->value.u32;
    if (0 == sec)
    {
        sdk_ret = ctc_aging_set_property(CTC_AGING_TBL_MAC, CTC_AGING_PROP_AGING_SCAN_EN, FALSE);
    }
    else
    {
        sdk_ret = ctc_aging_set_property(CTC_AGING_TBL_MAC, CTC_AGING_PROP_INTERVAL, sec);
        sdk_ret = ctc_aging_set_property(CTC_AGING_TBL_MAC, CTC_AGING_PROP_AGING_SCAN_EN, TRUE);
    }

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_isolation_mode(
    _In_  const sai_attribute_t *attr)
{
    uint32_t l3_enable = FALSE;
    int32_t  sdk_ret = CTC_E_NONE;

    if (SAI_ISOLATION_MODE_L2 == attr->value.s32)
    {
        l3_enable = FALSE;
    }
    else
    {
        l3_enable = TRUE;
    }

    sdk_ret = ctc_port_isolation_set_route_obey_isolated_en(l3_enable);
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_trunction_length(
    _In_  const sai_attribute_t *attr)
{
    int32_t             sdk_ret = CTC_E_NONE;
    ctc_qos_glb_cfg_t   glb_cfg;
    uint32_t            length  = 0;

    if (0 == attr->value.s32)
    {
        length = SAI_SWITCH_TRUNCTION_LENGTH_MAX;               /* GG default value */
    }
    else
    {
        length = attr->value.s32;
        if(length < SAI_SWITCH_TRUNCTION_LENGTH_MIN) {
            length = SAI_SWITCH_TRUNCTION_LENGTH_MIN;
        }

        if(length > SAI_SWITCH_TRUNCTION_LENGTH_MAX) {
            length = SAI_SWITCH_TRUNCTION_LENGTH_MAX;
        }
    }

    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));
    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_TRUNCATION_LEN;
    glb_cfg.u.value = length;
    sdk_ret = ctc_qos_set_global_config(&glb_cfg);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_timestamp_ether_dmac(
    _In_  const sai_attribute_t *attr)
{
    sal_memcpy(ctc_sai_get_sys_info()->timestamp.dmac, 
                attr->value.mac,
                sizeof(sai_mac_t));
CTC_SAI_DEBUG("%s:%d#dmac=%02x:%02x:%02x:%02x:%02x:%02x\n",
            __FUNCTION__,__LINE__,
            ctc_sai_get_sys_info()->timestamp.dmac[0],
            ctc_sai_get_sys_info()->timestamp.dmac[1],
            ctc_sai_get_sys_info()->timestamp.dmac[2],
            ctc_sai_get_sys_info()->timestamp.dmac[3],
            ctc_sai_get_sys_info()->timestamp.dmac[4],
            ctc_sai_get_sys_info()->timestamp.dmac[5]);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_switch_set_timestamp_ether_smac(
    _In_  const sai_attribute_t *attr)
{
    sal_memcpy(ctc_sai_get_sys_info()->timestamp.smac, 
                attr->value.mac,
                sizeof(sai_mac_t));
CTC_SAI_DEBUG("%s:%d#smac=%02x:%02x:%02x:%02x:%02x:%02x\n",
            __FUNCTION__,__LINE__,
            ctc_sai_get_sys_info()->timestamp.smac[0],
            ctc_sai_get_sys_info()->timestamp.smac[1],
            ctc_sai_get_sys_info()->timestamp.smac[2],
            ctc_sai_get_sys_info()->timestamp.smac[3],
            ctc_sai_get_sys_info()->timestamp.smac[4],
            ctc_sai_get_sys_info()->timestamp.smac[5]);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_switch_set_timestamp_ether_type(
    _In_  const sai_attribute_t *attr)
{
    ctc_sai_get_sys_info()->timestamp.type = attr->value.u16;
CTC_SAI_DEBUG("%s:%d#type = 0x%04x\n",
            __FUNCTION__,__LINE__,
            ctc_sai_get_sys_info()->timestamp.type);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_switch_get_aging_time(
    _Inout_  sai_attribute_t *attr)
{
    uint32_t            sec     = 0;
    int32_t             sdk_ret = CTC_E_NONE;

    sec = attr->value.u32;

    CTC_SAI_ERROR_GOTO(
        ctc_aging_get_property(CTC_AGING_TBL_MAC,CTC_AGING_PROP_INTERVAL,&sec),
        sdk_ret,out);

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_unicast_miss_action(
    _In_  const sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_get_unicast_miss_action(
    _Inout_  sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_set_broadcast_miss_action(
    _In_  const sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_get_broadcast_miss_action(
    _Inout_  sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_set_multicast_miss_action(
    _In_  const sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_get_multicast_miss_action(
    _Inout_  sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_NOT_SUPPORTED;

    return ret;
}

sai_status_t
ctc_sai_switch_set_lag_hash_algo(
    _In_  const sai_attribute_t *attr)
{
    int32_t                 sdk_ret = CTC_E_NONE;
    ctc_parser_global_cfg_t cfg;

    sal_memset(&cfg,0,sizeof(ctc_parser_global_cfg_t));
    
    sdk_ret = ctc_parser_get_global_cfg(&cfg);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }
    
    sdk_ret = ctc_parser_get_global_cfg(&cfg);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }

    if (SAI_HASH_ALGORITHM_XOR == attr->value.s32)
    {
        cfg.linkagg_hash_type = CTC_PARSER_GEN_HASH_TYPE_XOR;
    }
    else if( SAI_HASH_ALGORITHM_CRC == attr->value.s32)
    {
        cfg.linkagg_hash_type = CTC_PARSER_GEN_HASH_TYPE_CRC;
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    sdk_ret = ctc_parser_set_global_cfg(&cfg);
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_get_lag_hash_algo(
    _Inout_  sai_attribute_t *attr)
{
    int32_t                 sdk_ret = 0;
    ctc_parser_global_cfg_t cfg;

    sal_memset(&cfg,0,sizeof(ctc_parser_global_cfg_t));

    CTC_SAI_ERROR_GOTO(ctc_parser_get_global_cfg(&cfg),sdk_ret,out);

    if(CTC_PARSER_GEN_HASH_TYPE_XOR == cfg.linkagg_hash_type)
    {
        attr->value.s32 = CTC_PARSER_GEN_HASH_TYPE_XOR;
    }
    else if(CTC_PARSER_GEN_HASH_TYPE_CRC == cfg.linkagg_hash_type)
    {
        attr->value.s32 = CTC_PARSER_GEN_HASH_TYPE_CRC;
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}


sai_status_t
ctc_sai_switch_set_lag_hash(
    _In_  const sai_attribute_t *attr)
{

    int32_t             sdk_ret     = CTC_E_NONE;
    sai_object_id_t     hash_id = 0;
    uint32              flags = 0;
    ctc_sai_hash_entry_t* p_hash_entry = NULL;
    
    ctc_linkagg_psc_t   psc;
    sal_memset(&psc, 0, sizeof(ctc_linkagg_psc_t));
    
    hash_id = attr->value.oid;
    p_hash_entry = ctc_sai_hash_entry_get(hash_id);
    if (NULL == p_hash_entry)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
    psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
    psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
    ctc_linkagg_set_psc(&psc);
    sal_memset(&psc, 0, sizeof(ctc_linkagg_psc_t));

    flags = p_hash_entry->flags;
    
    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_SRC_IP))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
        psc.ip_flag |= CTC_LINKAGG_PSC_IP_IPSA;
        psc.ip_flag |= CTC_LINKAGG_PSC_USE_IP;
    }
    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_DST_IP))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
        psc.ip_flag |= CTC_LINKAGG_PSC_IP_IPDA;
        psc.ip_flag |= CTC_LINKAGG_PSC_USE_IP;
    }
    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_IP_PROTOCOL))
    {
         psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
         psc.ip_flag |= CTC_LINKAGG_PSC_IP_PROTOCOL;
    }

#ifdef TSINGMA
    if (psc.psc_type_bitmap & CTC_LINKAGG_PSC_TYPE_IP)
    {
        sdk_ret = ctc_linkagg_set_psc(&psc);
        if (sdk_ret < 0)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
    sal_memset(&psc, 0, sizeof(ctc_linkagg_psc_t));
#endif

    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_VLAN_ID))
    {
         psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
         psc.l2_flag |= CTC_LINKAGG_PSC_L2_VLAN;
    }
    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_ETHERTYPE))
    {
         psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
         psc.l2_flag |= CTC_LINKAGG_PSC_L2_ETHERTYPE;
    }
    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_SRC_MAC))
    {
         psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
         psc.l2_flag |= CTC_LINKAGG_PSC_L2_MACSA;
    }
    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_DST_MAC))
    {
         psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
         psc.l2_flag |= CTC_LINKAGG_PSC_L2_MACDA;
    }

#ifdef TSINGMA
    if (psc.psc_type_bitmap & CTC_LINKAGG_PSC_TYPE_L2)
    {
        sdk_ret = ctc_linkagg_set_psc(&psc);
        if (sdk_ret < 0)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
    sal_memset(&psc, 0, sizeof(ctc_linkagg_psc_t));
#endif
    
    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_L4_SRC_PORT))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
        psc.l4_flag |= CTC_LINKAGG_PSC_L4_SRC_PORT;
        psc.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
    }
    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_L4_DST_PORT))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
        psc.l4_flag |= CTC_LINKAGG_PSC_L4_DST_PORT;
        psc.l4_flag |= CTC_LINKAGG_PSC_USE_L4;
    } 

    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_VXLAN_VNI))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
        psc.l4_flag |= CTC_LINKAGG_PSC_L4_VXLAN_VNI;
    }

    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_GRE_KEY))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
        psc.l4_flag |= CTC_LINKAGG_PSC_L4_GRE_KEY;
    }

    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_NVGRE_VNI))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
        psc.l4_flag |= CTC_LINKAGG_PSC_L4_NVGRE_VSID;
    }

    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_NVGRE_FLOWID))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
        psc.l4_flag |= CTC_LINKAGG_PSC_L4_NVGRE_FLOW_ID;
    }

#ifdef TSINGMA
    if (psc.psc_type_bitmap & CTC_LINKAGG_PSC_TYPE_L4)
    {
        sdk_ret = ctc_linkagg_set_psc(&psc);
        if (sdk_ret < 0)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);
        }
    }
    sal_memset(&psc, 0, sizeof(ctc_linkagg_psc_t));
#endif

#ifndef TSINGMA
    sdk_ret = ctc_linkagg_set_psc(&psc);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }

/* config inner key */
    sal_memset(&psc, 0, sizeof(ctc_linkagg_psc_t));
#endif

    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_INNER_DST_MAC))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
        psc.l2_flag |= CTC_LINKAGG_PSC_L2_MACDA;
    }

    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_INNER_SRC_MAC))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
        psc.l2_flag |= CTC_LINKAGG_PSC_L2_MACSA;
    }

    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_INNER_SRC_IP))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
        psc.ip_flag |= CTC_LINKAGG_PSC_IP_IPSA;
    }

    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_INNER_DST_IP))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
        psc.ip_flag |= CTC_LINKAGG_PSC_IP_IPDA;
    }
    
    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_INNER_IP_PROTOCOL))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
        psc.ip_flag |= CTC_LINKAGG_PSC_IP_PROTOCOL;
    }
    
    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_INNER_L4_SRC_PORT))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
        psc.l4_flag |= CTC_LINKAGG_PSC_L4_SRC_PORT;
    }
    
    if (CTC_IS_BIT_SET(flags, SAI_NATIVE_HASH_FIELD_INNER_L4_DST_PORT))
    {
        psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
        psc.l4_flag |= CTC_LINKAGG_PSC_L4_DST_PORT;
    }
#if defined (GOLDENGATE) || defined (DUET2)
    psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_INNER;
    psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
    psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
    psc.psc_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
#endif

#if defined (GOLDENGATE) || defined (USW)
#ifdef TSINGMA
    if (0 == psc.psc_type_bitmap)
    {
        return SAI_STATUS_SUCCESS;
    }
#endif
    sdk_ret = ctc_linkagg_set_psc(&psc);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }
#endif
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_switch_set_lag_hash_inner_field_en(
    _In_  const sai_attribute_t *attr)
{
#if defined GOLDENGATE || defined (USW)
    int32_t             sdk_ret     = CTC_E_NONE;
    ctc_parser_global_cfg_t global_cfg = {};
    
    sdk_ret = ctc_parser_get_global_cfg(&global_cfg);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }

#if 0
  GLB_LAG_HASH_ALGO_INNER_MODE_OFF,    /* hash-calc only outer-field */
  GLB_LAG_HASH_ALGO_INNER_MODE_ON,	   /* hash-calc inner and outer-field, default type*/
  GLB_LAG_HASH_ALGO_INNER_MODE_ONLY,   /* hash-calc tunnel only inner, else outer */
#endif

    switch(attr->value.u32){
	case 1: 
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_VXLAN] 
            = CTC_PARSER_TUNNEL_HASH_MODE_MERGE;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_NVGRE] 
            = CTC_PARSER_TUNNEL_HASH_MODE_MERGE;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_GRE] 
            = CTC_PARSER_TUNNEL_HASH_MODE_MERGE;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_TRILL] 
            = CTC_PARSER_TUNNEL_HASH_MODE_MERGE;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_IP] 
            = CTC_PARSER_TUNNEL_HASH_MODE_MERGE;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_MPLS] 
            = CTC_PARSER_TUNNEL_HASH_MODE_MERGE;  /* Merge */
		break;
		
    case 0:
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_VXLAN] 
            = CTC_PARSER_TUNNEL_HASH_MODE_OUTER;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_NVGRE] 
            = CTC_PARSER_TUNNEL_HASH_MODE_OUTER;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_GRE] 
            = CTC_PARSER_TUNNEL_HASH_MODE_OUTER;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_TRILL] 
            = CTC_PARSER_TUNNEL_HASH_MODE_OUTER;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_IP] 
            = CTC_PARSER_TUNNEL_HASH_MODE_OUTER;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_MPLS] 
            = CTC_PARSER_TUNNEL_HASH_MODE_OUTER;  /* Merge */
		break;
		
    default:
		global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_VXLAN] 
            = CTC_PARSER_TUNNEL_HASH_MODE_INNER;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_NVGRE] 
            = CTC_PARSER_TUNNEL_HASH_MODE_INNER;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_GRE] 
            = CTC_PARSER_TUNNEL_HASH_MODE_INNER;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_TRILL] 
            = CTC_PARSER_TUNNEL_HASH_MODE_INNER;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_IP] 
            = CTC_PARSER_TUNNEL_HASH_MODE_INNER;  /* Merge */
        global_cfg.linkagg_tunnel_hash_mode[CTC_PARSER_TUNNEL_TYPE_MPLS] 
            = CTC_PARSER_TUNNEL_HASH_MODE_INNER;  /* Merge */
		break;
    }

    sdk_ret = ctc_parser_set_global_cfg(&global_cfg);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }    
#endif    
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_switch_get_lag_fields(
    _Inout_  sai_attribute_t *attr)
{
#if 0
    uint32_t            list_idx    = 0;
    int32_t             sdk_ret     = CTC_E_NONE;
    ctc_linkagg_psc_t   psc;

    sal_memset(&psc,0,sizeof(ctc_linkagg_psc_t));

    CTC_SET_FLAG(psc.psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_IP);
    CTC_SET_FLAG(psc.psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_L2);
    CTC_SET_FLAG(psc.psc_type_bitmap, CTC_LINKAGG_PSC_TYPE_L4);
    sdk_ret = ctc_linkagg_get_psc(&psc);
    if(CTC_E_NONE != sdk_ret)
    {
        goto out;
    }


    if(CTC_FLAG_ISSET(psc.ip_flag, CTC_LINKAGG_PSC_IP_IPSA))
    {
        attr->value.s32list.list[list_idx++] = SAI_HASH_SRC_IP;
    }

    if(CTC_FLAG_ISSET(psc.ip_flag, CTC_LINKAGG_PSC_IP_IPDA))
    {
        attr->value.s32list.list[list_idx++] = SAI_HASH_DST_IP;
    }

    if(CTC_FLAG_ISSET(psc.ip_flag, CTC_LINKAGG_PSC_IP_PROTOCOL))
    {
        attr->value.s32list.list[list_idx++] = SAI_HASH_IP_PROTOCOL;
    }



    if(CTC_FLAG_ISSET(psc.l2_flag, CTC_LINKAGG_PSC_L2_ETHERTYPE))
    {
        attr->value.s32list.list[list_idx++] = SAI_HASH_ETHERTYPE;
    }

    if(CTC_FLAG_ISSET(psc.l2_flag, CTC_LINKAGG_PSC_L2_MACSA))
    {
        attr->value.s32list.list[list_idx++] = SAI_HASH_SOURCE_MAC;
    }

    if(CTC_FLAG_ISSET(psc.l2_flag, CTC_LINKAGG_PSC_L2_MACDA))
    {
        attr->value.s32list.list[list_idx++] = SAI_HASH_DEST_MAC;
    }

    if(CTC_FLAG_ISSET(psc.l2_flag, CTC_LINKAGG_PSC_L2_VLAN))
    {
        attr->value.s32list.list[list_idx++] = SAI_HASH_VLAN_ID;
    }


    if(CTC_FLAG_ISSET(psc.l4_flag, CTC_LINKAGG_PSC_L4_SRC_PORT))
    {
        attr->value.s32list.list[list_idx++] = SAI_HASH_L4_SOURCE_PORT;
    }

    if(CTC_FLAG_ISSET(psc.l4_flag, CTC_LINKAGG_PSC_L4_DST_PORT))
    {
        attr->value.s32list.list[list_idx++] = SAI_HASH_L4_DEST_PORT;
    }

    attr->value.s32list.count = list_idx;


out:
#endif
    int sdk_ret = 0;
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}


sai_status_t
ctc_sai_switch_set_ecmp_hash_algo(
    _In_  const sai_attribute_t *attr)
{
    int32_t                 sdk_ret = CTC_E_NONE;
    ctc_parser_global_cfg_t cfg;

    sal_memset(&cfg,0,sizeof(ctc_parser_global_cfg_t));
    sdk_ret = ctc_parser_get_global_cfg(&cfg);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }

    sdk_ret = ctc_parser_get_global_cfg(&cfg);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }
    if(SAI_HASH_ALGORITHM_XOR == attr->value.s32)
    {
        cfg.ecmp_hash_type = CTC_PARSER_GEN_HASH_TYPE_XOR;
    }
    else if(SAI_HASH_ALGORITHM_CRC == attr->value.s32)
    {
        cfg.ecmp_hash_type = CTC_PARSER_GEN_HASH_TYPE_CRC;
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    sdk_ret = ctc_parser_set_global_cfg(&cfg);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_ecmp_load_balance_mode(
    _In_  const sai_attribute_t *attr)
{
    int32_t sdk_ret = CTC_E_NONE;
#ifdef USW
    uint32 type = 0;
    uint32 mode = 0;
    switch (attr->value.u32)
    {
    case GLB_ECMP_DYNAMIC_LOAD_BALANCE_NORMAL:
        type = CTC_GLOBAL_ECMP_DLB_MODE;
        mode = CTC_GLOBAL_ECMP_DLB_MODE_ALL;
        break;
    case GLB_ECMP_DYNAMIC_LOAD_BALANCE_FIRST:
        type = CTC_GLOBAL_ECMP_REBALANCE_MODE;
        mode = CTC_GLOBAL_ECMP_REBALANCE_MODE_FIRST;
        break;
    case GLB_ECMP_DYNAMIC_LOAD_BALANCE_PACKET:
        type = CTC_GLOBAL_ECMP_REBALANCE_MODE;
        mode = CTC_GLOBAL_ECMP_REBALANCE_MODE_PACKET;
        break;
    case GLB_ECMP_DYNAMIC_LOAD_BALANCE_ELEPHANT:
        type = CTC_GLOBAL_ECMP_DLB_MODE;
        mode = CTC_GLOBAL_ECMP_DLB_MODE_ELEPHANT;
        break;
    case GLB_ECMP_DYNAMIC_LOAD_BALANCE_TCP:
        type = CTC_GLOBAL_ECMP_DLB_MODE;
        mode = CTC_GLOBAL_ECMP_DLB_MODE_TCP;
        break;
    default:
        break;
    }
    sdk_ret = ctc_global_ctl_set(type, &mode);
#else
    ctc_nh_global_cfg_t     nh_glb_cfg;
    sal_memset(&nh_glb_cfg, 0, sizeof(ctc_nh_global_cfg_t));
    nh_glb_cfg.ecmp_dynamic_mode = attr->value.u32;

    sdk_ret = ctc_nh_set_global_config(&nh_glb_cfg);
#endif

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}


sai_status_t
ctc_sai_switch_get_ecmp_hash_algo(
    _Inout_  sai_attribute_t *attr)
{
    int32_t                 sdk_ret = CTC_E_NONE;
    ctc_parser_global_cfg_t cfg;

    sal_memset(&cfg,0,sizeof(ctc_parser_global_cfg_t));

    CTC_SAI_ERROR_GOTO(ctc_parser_get_global_cfg(&cfg),sdk_ret,out);

    if(CTC_PARSER_GEN_HASH_TYPE_XOR == cfg.linkagg_hash_type)
    {
        attr->value.s32 = SAI_HASH_ALGORITHM_XOR;
    }
    else if(CTC_PARSER_GEN_HASH_TYPE_CRC == cfg.linkagg_hash_type)
    {
        attr->value.s32 = SAI_HASH_ALGORITHM_CRC;
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}


sai_status_t
ctc_sai_switch_set_ecmp_hash(
    _In_  const sai_attribute_t *attr)
{
    int32_t     sdk_ret    = CTC_E_NONE;
    ctc_parser_ecmp_hash_ctl_t hash_ctl;
    uint32 hash_keys;

    hash_keys = attr->value.u32;

    sal_memset(&hash_ctl, 0, sizeof(hash_ctl));
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP);
    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4);

    /* 0. set l2 hash key */
    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_SRC_MAC))
    {
        CTC_SET_FLAG(hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACSA);
    }

    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_DST_MAC))
    {
        CTC_SET_FLAG(hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACDA);
    }
    
    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_ETHERTYPE))
    {
        CTC_SET_FLAG(hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_ETHERTYPE);
    }

    /* 1. set ip hash key */
    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_SRC_IP))
    {
        CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA);
    }

    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_DST_IP))
    {
        CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA);
    }

    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_IP_PROTOCOL))
    {
        CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL);
    }

    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_DSCP))
    {
        CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_DSCP);
    }

    /* 2. set layer4 hash key */
    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_L4_SRC_PORT))
    {
        CTC_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT);
        /* Added by weizj for tunnel bug 33501 */
        CTC_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_VXLAN_L4_SRC_PORT);
    }

    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_L4_DST_PORT))
    {
        CTC_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT);
    }

    /* 4. set hash ctl to sdk */
    sdk_ret = ctc_parser_set_ecmp_hash_field(&hash_ctl);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }

    /* config inner key */
    sal_memset(&hash_ctl, 0, sizeof(hash_ctl));

    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_INNER_SRC_MAC))
    {
        CTC_SET_FLAG(hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACSA);
    }
    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_INNER_DST_MAC))
    {
        CTC_SET_FLAG(hash_ctl.l2_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACDA);
    }

    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_INNER_SRC_IP))
    {
        CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA);
    }
    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_INNER_DST_IP))
    {
        CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA);
    }
    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_INNER_IP_PROTOCOL))
    {
        CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL);
    }
    
    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_INNER_L4_SRC_PORT))
    {
        CTC_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT);
    }
    if (CTC_IS_BIT_SET(hash_keys, SAI_NATIVE_HASH_FIELD_INNER_L4_DST_PORT))
    {
        CTC_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT);
    }

#if defined GOLDENGATE || defined USW
    hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L2;
    hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP;
    hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4;
    hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_INNER;

    sdk_ret = ctc_parser_set_ecmp_hash_field(&hash_ctl);
    if (sdk_ret < 0)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);
    }
#endif
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_switch_set_hash_fields(
    _In_  const sai_attribute_t *attr)
{
#if 0
    uint32_t                    list_idx    = 0;
    int32_t                     sdk_ret     = CTC_E_NONE;
    ctc_parser_ecmp_hash_ctl_t  ecmp_hash_ctl;

    sal_memset(&ecmp_hash_ctl,0,sizeof(ctc_parser_ecmp_hash_ctl_t));

    for(list_idx = 0; list_idx < attr->value.s32list.count; list_idx++)
    {
        switch(attr->value.s32list.list[list_idx])
        {
            case SAI_HASH_SRC_IP:
                ecmp_hash_ctl.hash_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
                ecmp_hash_ctl.ip_flag |= CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA;
                break;
            case SAI_HASH_DST_IP:
                ecmp_hash_ctl.hash_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
                ecmp_hash_ctl.ip_flag |= CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA;
                break;
            case SAI_HASH_VLAN_ID:
                ecmp_hash_ctl.hash_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
                ecmp_hash_ctl.l2_flag |= CTC_PARSER_L2_HASH_FLAGS_CTAG_VID;
                ecmp_hash_ctl.l2_flag |= CTC_PARSER_L2_HASH_FLAGS_STAG_VID;
                break;
            case SAI_HASH_IP_PROTOCOL:
                ecmp_hash_ctl.hash_type_bitmap |= CTC_LINKAGG_PSC_TYPE_IP;
                ecmp_hash_ctl.ip_flag |= CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL;
                break;
            case SAI_HASH_ETHERTYPE:
                ecmp_hash_ctl.hash_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
                ecmp_hash_ctl.l2_flag |= CTC_PARSER_L2_HASH_FLAGS_ETHERTYPE;
                break;
            case SAI_HASH_L4_SOURCE_PORT:
                ecmp_hash_ctl.hash_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
                ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT;
                break;
            case SAI_HASH_L4_DEST_PORT:
                ecmp_hash_ctl.hash_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L4;
                ecmp_hash_ctl.l4_flag |= CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT;
                break;
            case SAI_HASH_SOURCE_MAC:
                ecmp_hash_ctl.hash_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
                ecmp_hash_ctl.l2_flag |= CTC_PARSER_L2_ECMP_HASH_FLAGS_MACSA;
                break;
            case SAI_HASH_DEST_MAC:
                ecmp_hash_ctl.hash_type_bitmap |= CTC_LINKAGG_PSC_TYPE_L2;
                ecmp_hash_ctl.l2_flag |= CTC_PARSER_L2_ECMP_HASH_FLAGS_MACDA;
                break;
            case SAI_HASH_DEST_PORT:
                break;
            case SAI_HASH_SOURCE_PORT:
                break;
        }
    }

    sdk_ret = ctc_parser_set_ecmp_hash_field(&ecmp_hash_ctl);
#endif
    int sdk_ret = 0;
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_get_hash_fields(
    _Inout_  sai_attribute_t *attr)
{
#if 0
    uint32_t                    list_idx = 0;
    int32_t                     sdk_ret = CTC_E_NONE;
    ctc_parser_ecmp_hash_ctl_t  ecmp_hash_ctl;

    sal_memset(&ecmp_hash_ctl,0,sizeof(ctc_parser_ecmp_hash_ctl_t));

    CTC_SAI_ERROR_GOTO(ctc_parser_get_ecmp_hash_field(&ecmp_hash_ctl),sdk_ret,out);

    for(list_idx = 0; list_idx < attr->value.s32list.count; list_idx++)
    {
        if(CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_LINKAGG_PSC_TYPE_IP))
        {
            if(CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA))
            {
                attr->value.s32list.list[list_idx] = SAI_HASH_SRC_IP;
                continue;
            }

            if(CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA))
            {
                attr->value.s32list.list[list_idx] = SAI_HASH_DST_IP;
                continue;
            }

            if(CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL))
            {
                attr->value.s32list.list[list_idx] = SAI_HASH_IP_PROTOCOL;
                continue;
            }
        }

        if(CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_LINKAGG_PSC_TYPE_L2))
        {
            if(CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_L2_HASH_FLAGS_ETHERTYPE))
            {
                attr->value.s32list.list[list_idx] = SAI_HASH_ETHERTYPE;
                continue;
            }

            if(CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACSA))
            {
                attr->value.s32list.list[list_idx] = SAI_HASH_SOURCE_MAC;
                continue;
            }

            if(CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_L2_ECMP_HASH_FLAGS_MACDA))
            {
                attr->value.s32list.list[list_idx] = SAI_HASH_DEST_MAC;
                continue;
            }

            if(CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_L2_HASH_FLAGS_CTAG_VID))
            {
                attr->value.s32list.list[list_idx] = SAI_HASH_VLAN_ID;
                continue;
            }
        }

        if(CTC_FLAG_ISSET(ecmp_hash_ctl.hash_type_bitmap, CTC_LINKAGG_PSC_TYPE_L4))
        {
            if(CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT))
            {
                attr->value.s32list.list[list_idx] = SAI_HASH_L4_SOURCE_PORT;
                continue;
            }

            if(CTC_FLAG_ISSET(ecmp_hash_ctl.ip_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT))
            {
                attr->value.s32list.list[list_idx] = SAI_HASH_L4_DEST_PORT;
                continue;
            }
        }
    }

out:
#endif
    int sdk_ret = 0;

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}


sai_status_t
ctc_sai_switch_set_max_paths(
    _In_  const sai_attribute_t *attr)
{
    uint16_t            max_ecmp    = 0;
    int32_t             sdk_ret     = CTC_E_NONE;

    max_ecmp = attr->value.u32;

    sdk_ret = ctc_nh_set_max_ecmp(max_ecmp);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_get_max_paths(
    _Inout_  sai_attribute_t *attr)
{
    uint16_t            max_ecmp    = 0;
    int32_t             sdk_ret     = CTC_E_NONE;

    CTC_SAI_ERROR_GOTO(ctc_nh_get_max_ecmp(&max_ecmp),sdk_ret,out);

    attr->value.u32 = max_ecmp;

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_qos_map(
    _In_ const sai_attribute_t *attr)
{
    return ctc_sai_qos_map_set_switch_qos_map(attr);
}

sai_status_t
ctc_sai_switch_set_ipg_storm_ctl(
    _In_ const sai_attribute_t *attr)
{
   int32_t sdk_ret = 0;
   uint8   enable = 0;
   
   ctc_security_stmctl_glb_cfg_t glb_cfg;
   sal_memset(&glb_cfg,0,sizeof(ctc_security_stmctl_glb_cfg_t));
   
   sdk_ret = ctc_storm_ctl_get_global_cfg(&glb_cfg);
   if (sdk_ret != CTC_E_NONE)
   {
      return ctc_sai_get_error_from_sdk_error(sdk_ret);
   }
   enable = attr->value.booldata;
   glb_cfg.ipg_en = enable;
   sdk_ret = ctc_storm_ctl_set_global_cfg(&glb_cfg);

   return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_discard_same_macdasa_pkt(
    _Inout_ const sai_attribute_t *attr)
{
    uint32_t            discard_pkt     = 0;
    int32_t             sdk_ret = CTC_E_NONE;

    if(attr->value.booldata){
        discard_pkt = 0;
    }else{
        discard_pkt = 1;
    }

    CTC_SAI_ERROR_GOTO(
        ctc_global_ctl_set(CTC_GLOBAL_DISCARD_SAME_MACDASA_PKT, &discard_pkt),
        sdk_ret,out);

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_discard_same_ipdasa_pkt(
    _Inout_ const sai_attribute_t *attr)
{
    uint32_t            discard_pkt     = 0;
    int32_t             sdk_ret = CTC_E_NONE;

    if(attr->value.booldata){
        discard_pkt = 0;
    }else{
        discard_pkt = 1;
    }

    CTC_SAI_ERROR_GOTO(
        ctc_global_ctl_set(CTC_GLOBAL_DISCARD_SAME_IPDASA_PKT, &discard_pkt),
        sdk_ret,out);

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_discard_same_mcast_sa_pkt(
    _Inout_ const sai_attribute_t *attr)
{
    uint32_t            discard_pkt     = 0;
    int32_t             sdk_ret = CTC_E_NONE;

    if(attr->value.booldata){
        discard_pkt = 0;
    }else{
        discard_pkt = 1;
    }

    CTC_SAI_ERROR_GOTO(
        ctc_global_ctl_set(CTC_GLOBAL_DISCARD_MCAST_SA_PKT, &discard_pkt),
        sdk_ret,out);

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_ipv6_enable(
    _Inout_ const sai_attribute_t *attr)
{
    uint32_t  ipv6_enable = 0;
    int32_t   sdk_ret = CTC_E_NONE;

    if(attr->value.u32){
        ipv6_enable = TRUE;
    }else{
        ipv6_enable = FALSE;
    }

    sdk_ret = ctc_parser_enable_l3_type(CTC_PARSER_L3_TYPE_IPV6, ipv6_enable);
    if (sdk_ret < 0)
    {
        return sdk_ret;
    }
    ipv6_packet_paser = ipv6_enable;
    return CTC_E_NONE;
}

sai_status_t
ctc_sai_switch_set_qos_ipg_enable(
    _Inout_ const sai_attribute_t *attr)
{
    ctc_qos_glb_cfg_t glb_cfg;
    int32_t sdk_ret = CTC_E_NONE;
    
    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));

    if(attr->value.u8){
        glb_cfg.u.value = 1;
    }else{
        glb_cfg.u.value = 0;
    }

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_SHAPE_IPG_EN;
    CTC_SAI_ERROR_GOTO(
        ctc_qos_set_global_config(&glb_cfg),
        sdk_ret,out);

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_IPG_EN;
    CTC_SAI_ERROR_GOTO(
        ctc_qos_set_global_config(&glb_cfg),
        sdk_ret,out);

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

/*added by yejl for gg1.0 queue stats, bug 54212, 2019-10-31*/  
sai_status_t
ctc_sai_switch_set_qos_queue_shape_enable(
    _Inout_ const sai_attribute_t *attr)
{
#ifdef GOLDENGATE
    int32_t sdk_ret = CTC_E_NONE;
    ctc_qos_glb_cfg_t glb_cfg;
    sal_memset(&glb_cfg, 0, sizeof(ctc_qos_glb_cfg_t));
    
    if (attr->value.u8)
    {
        glb_cfg.u.value = TRUE;
    }
    else
    {
        glb_cfg.u.value = FALSE;
    }

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_QUE_SHAPE_EN_GG_1;
    CTC_SAI_ERROR_GOTO(ctc_qos_set_global_config(&glb_cfg),sdk_ret,out);

    glb_cfg.cfg_type = CTC_QOS_GLB_CFG_GROUP_SHAPE_EN_GG_1;
    CTC_SAI_ERROR_GOTO(ctc_qos_set_global_config(&glb_cfg),sdk_ret,out);

out:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
#else
    return CTC_E_NONE;
#endif
}
/*ended by yejl*/

sai_status_t
ctc_sai_switch_set_memory_check_state(
    _Inout_ const sai_attribute_t *attr)
{
    uint32_t  discard_enable = 0;

    if (attr->value.u32)
    {
        discard_enable = TRUE;
    }
    else
    {
        discard_enable = FALSE;
    }
    memory_check_pktdiscard = discard_enable;
    return CTC_E_NONE;
}

sai_status_t
ctc_sai_switch_set_svlan_tpid(
    _In_  const sai_attribute_t *attr)
{
    int32_t    sdk_ret = CTC_E_NONE;
    uint16     tpid    = attr->value.s32 & 0xffff;

    if (!(attr->value.s32 >> 16))
    {
        sdk_ret = ctc_parser_set_tpid(CTC_PARSER_L2_TPID_SVLAN_TPID_0, tpid);
    }
    else
    {
        sdk_ret = ctc_parser_set_tpid(CTC_PARSER_L2_TPID_SVLAN_TPID_1, tpid);
    }

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_set_cvlan_tpid(
    _In_  const sai_attribute_t *attr)
{
    int32_t    sdk_ret = CTC_E_NONE;
    uint16     tpid    = attr->value.s32 & 0xffff;

    sdk_ret = ctc_parser_set_tpid(CTC_PARSER_L2_TPID_CVLAN_TPID, tpid);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_sai_switch_get_svlan_tpid(
    _Inout_  sai_attribute_t *attr)
{
    int32_t    sdk_ret = CTC_E_NONE;
    uint16_t   tpid    = 0;


    if (!(attr->value.s32 >> 16))
    {
        sdk_ret = ctc_parser_get_tpid(CTC_PARSER_L2_TPID_SVLAN_TPID_0, &tpid);
    }
    else
    {
        sdk_ret = ctc_parser_get_tpid(CTC_PARSER_L2_TPID_SVLAN_TPID_1, &tpid);
    }

    attr->value.u32 += tpid;
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

#define ________SAI_SWITCH_SAI_INNER_API_FUNC
sai_status_t
__init_traverse_api_reg(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *data)
{
    return preg->init_func(preg,preg->private_data);
}

sai_status_t
__exit_traverse_api_reg(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *data)
{
    return preg->exit_func(preg,preg->private_data);
}


#define ________SAI_SWITCH_SAI_API_FUNC


#define ________SAI_SWITCH_DEBUG_FUNC
sai_status_t ctc_sai_set_switch_attribute_debug_param(
    _In_ const sai_attribute_t *attr)
{
    switch(attr->id)
    {        
    case SAI_SWITCH_ATTR_ECMP_HASH:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_ECMP_HASH 0x%llx", attr->value.oid);
        break;
        
    case SAI_SWITCH_ATTR_LAG_HASH:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_LAG_HASH 0x%llx", attr->value.oid);
        break;

    case SAI_SWITCH_ATTR_SWITCHING_MODE:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_SWITCHING_MODE");
        break;

    case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE");
        break;

    case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE");
        break;

    case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_SRC_MAC_ADDRESS %02x%02x.%02x%02x.%02x%02x", 
            attr->value.mac[0],attr->value.mac[1],attr->value.mac[2],
            attr->value.mac[3],attr->value.mac[4],attr->value.mac[5]);
        break;

    case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES %u", attr->value.u32);
        break;

    case SAI_SWITCH_ATTR_FDB_AGING_TIME:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_FDB_AGING_TIME %u", attr->value.u32);
        break;

    case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION");
        break;

    case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION");
        break;

    case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION");
        break;

    case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_FDB_AGING_TIME %d", attr->value.s32);
        break;

    case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM %d", attr->value.s32);
        break;

    case SAI_SWITCH_ATTR_ECMP_MAX_PATHS:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_ECMP_MAX_PATHS %u", attr->value.u32);
        break;

    case SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_ECMP_MAX_PATHS %u", attr->value.booldata);
        break;

    case SAI_SWITCH_ATTR_PORT_BREAKOUT:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_PORT_BREAKOUT");
        break;
    case SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP:
    case SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP:
    case SAI_SWITCH_ATTR_QOS_EXP_TO_TC_AND_COLOR_MAP:
    case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
    case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
    case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_EXP_MAP:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP");
        break;
 
    case SAI_SWITCH_ATTR_ISOLATION_MODE:
        CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_ISOLATION_MODE %d", attr->value.s32);
        break;
    }
    return SAI_STATUS_SUCCESS;
}
sai_status_t ctc_sai_get_switch_attribute_debug_param(
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    uint32_t                    attr_idx    = 0;
    const sai_attribute_t *attr = NULL;

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_SWITCH_ATTR_PORT_NUMBER:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_PORT_NUMBER %u", attr->value.u32);
            break;

        case SAI_SWITCH_ATTR_PORT_LIST:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_PORT_LIST %u", attr->value.objlist.count);
            break;

        case SAI_SWITCH_ATTR_CPU_PORT:
        case SAI_SWITCH_ATTR_FDB_TABLE_SIZE:
        case SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED:
        case SAI_SWITCH_ATTR_OPER_STATUS:
        case SAI_SWITCH_ATTR_MAX_TEMP:
        case SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY:

            CTC_SAI_DEBUG("in:NOT SUPPORT");
            break;

        case SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS %u", attr->value.u32);
            break;

        case SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS %u", attr->value.oid);
            break;

        case SAI_SWITCH_ATTR_SWITCHING_MODE:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_SWITCHING_MODE");
            break;

        case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE");
            break;

        case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE");
            break;

        case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_SRC_MAC_ADDRESS");
            break;

        case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES %u", attr->value.u32);
            break;

        case SAI_SWITCH_ATTR_FDB_AGING_TIME:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_FDB_AGING_TIME %u", attr->value.u32);
            break;

        case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION");
            break;

        case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION");
            break;

        case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION");
            break;

        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM %u", attr->value.u32);
            break;

        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM %d", attr->value.s32);
            break;

        case SAI_SWITCH_ATTR_ECMP_MAX_PATHS:
            CTC_SAI_DEBUG("in:SAI_SWITCH_ATTR_ECMP_MAX_PATHS %u", attr->value.u32);
            break;

        case SAI_SWITCH_ATTR_PORT_BREAKOUT:
            CTC_SAI_DEBUG("in:NOT SUPPORT");
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

#define ________SAI_SWITCH_API_FUNC
#define COUNTOF(_array_)    ((uint32)(sizeof((_array_)) / sizeof((_array_)[0])))

/*
default interrupt configuration provide two types interrupt confuguration: msi and pin
    for pin interrupt:
         using ctc_intr_config for interrupt mapping to group
         using ctc_intr_group_config for register interrupt, default using 2 interrupt pin
    for msi interrupt:
         using ctc_intr_config_msi for interrupt mapping to group
         using ctc_intr_group_config_msi for register interrupt, now only support 1 msi interrupt
*/
#define __PIN_INTERRUPT_CONFIG__

#if ((defined _CTC_ARM_HI3535_) || (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_))
ctc_intr_mapping_t ctc_app_intr_config[] =
{
    {0,    CTC_INTR_CHIP_FATAL                 },
    {0,    CTC_INTR_CHIP_NORMAL                },
    {INVG,  CTC_INTR_FUNC_PTP_TS_CAPTURE        },
    {0,  CTC_INTR_FUNC_PTP_MAC_TX_TS_CAPTURE },
    {0,  CTC_INTR_FUNC_PTP_TOD_PULSE_IN      },
    {0,  CTC_INTR_FUNC_PTP_TOD_CODE_IN_RDY   },
    {INVG,  CTC_INTR_FUNC_PTP_SYNC_PULSE_IN     },
    {INVG,  CTC_INTR_FUNC_PTP_SYNC_CODE_IN_RDY  },
    {0,       CTC_INTR_FUNC_OAM_DEFECT_CACHE      },
    {0,  CTC_INTR_FUNC_MDIO_CHANGE           },
#ifdef GREATBELT
    {0,    CTC_INTR_FUNC_MDIO_1G_CHANGE        },
    {0,    CTC_INTR_DMA_NORMAL                 },
#else
    {0,      CTC_INTR_FUNC_LINK_CHAGNE       },
#endif
    {0,      CTC_INTR_FUNC_LINKAGG_FAILOVER      },
    {0,      CTC_INTR_FUNC_IPE_LEARN_CACHE       },
    {0,      CTC_INTR_FUNC_IPE_AGING_FIFO        },
    {0,      CTC_INTR_FUNC_STATS_FIFO            },
    {INVG, CTC_INTR_FUNC_APS_FAILOVER          },
    {0,      CTC_INTR_DMA_FUNC                   },
};
#else
ctc_intr_mapping_t ctc_app_intr_config[] =
{
    {1,    CTC_INTR_CHIP_FATAL                 },
    {0,    CTC_INTR_CHIP_NORMAL                },
#if defined(GOLDENGATE) || defined(USW)
    {0,  CTC_INTR_FUNC_PTP_TS_CAPTURE        },
#else
    {INVG,  CTC_INTR_FUNC_PTP_TS_CAPTURE        },
#endif
    {0,  CTC_INTR_FUNC_PTP_MAC_TX_TS_CAPTURE },
    {0,  CTC_INTR_FUNC_PTP_TOD_PULSE_IN      },
    {0,  CTC_INTR_FUNC_PTP_TOD_CODE_IN_RDY   },
    {INVG,  CTC_INTR_FUNC_PTP_SYNC_PULSE_IN     },
    {INVG,  CTC_INTR_FUNC_PTP_SYNC_CODE_IN_RDY  },
    {0,       CTC_INTR_FUNC_OAM_DEFECT_CACHE      },
    {0,  CTC_INTR_FUNC_MDIO_CHANGE           },
#ifdef GREATBELT
    {0,    CTC_INTR_FUNC_MDIO_1G_CHANGE        },
    {0,    CTC_INTR_DMA_NORMAL                 },
#else
    {0,      CTC_INTR_FUNC_LINK_CHAGNE       },
#endif
    {0,      CTC_INTR_FUNC_LINKAGG_FAILOVER      },
    {0,      CTC_INTR_FUNC_IPE_LEARN_CACHE       },
    {0,      CTC_INTR_FUNC_IPE_AGING_FIFO        },
    {0,      CTC_INTR_FUNC_STATS_FIFO            },
    {INVG, CTC_INTR_FUNC_APS_FAILOVER          },
    {1,      CTC_INTR_DMA_FUNC                   },
};
#endif
ctc_intr_mapping_t ctc_app_intr_config_msi[] =
{
    {0,    CTC_INTR_CHIP_FATAL                 },
    {0,    CTC_INTR_CHIP_NORMAL                },
#if defined(GOLDENGATE) || defined(USW)
    {0,  CTC_INTR_FUNC_PTP_TS_CAPTURE        },
#else
    {INVG,  CTC_INTR_FUNC_PTP_TS_CAPTURE        },
#endif
    {0,  CTC_INTR_FUNC_PTP_MAC_TX_TS_CAPTURE },
    {0,  CTC_INTR_FUNC_PTP_TOD_PULSE_IN      },
    {0,  CTC_INTR_FUNC_PTP_TOD_CODE_IN_RDY   },
    {INVG,  CTC_INTR_FUNC_PTP_SYNC_PULSE_IN     },
    {INVG,  CTC_INTR_FUNC_PTP_SYNC_CODE_IN_RDY  },
    {0,       CTC_INTR_FUNC_OAM_DEFECT_CACHE      },
    {0,  CTC_INTR_FUNC_MDIO_CHANGE           },
#ifdef GREATBELT
    {0,    CTC_INTR_FUNC_MDIO_1G_CHANGE        },
    {0,    CTC_INTR_DMA_NORMAL                 },
#else
    {0,      CTC_INTR_FUNC_LINK_CHAGNE       },
#endif
    {0,      CTC_INTR_FUNC_LINKAGG_FAILOVER      },
    {0,      CTC_INTR_FUNC_IPE_LEARN_CACHE       },
    {0,      CTC_INTR_FUNC_IPE_AGING_FIFO        },
    {0, CTC_INTR_FUNC_STATS_FIFO            },
    {INVG, CTC_INTR_FUNC_APS_FAILOVER          },
    {0,      CTC_INTR_DMA_FUNC                   },
};

/* default interrupt group configuration, need to change the IRQ and priority of groups based on your requirement
 * the priority range is [1, 139] for linux, thereinto [1, 99] is realtime schedule; and [0, 255] for vxworks
 */
#ifdef _CTC_ARM_HI3535_
ctc_intr_group_t ctc_app_intr_group_config[] =
{
    {0, 91, SAL_TASK_PRIO_DEF,  "other interrupt group"},
    //{1, 17, 100,                "DMA and abnormal group"}
};
#elif defined _CTC_ARM_LS1023A_
ctc_intr_group_t ctc_app_intr_group_config[] =
{
    {0,  67, SAL_TASK_PRIO_DEF, "Function normal group"},
};
#elif defined _CTC_ARM_CTC5236_
ctc_intr_group_t ctc_app_intr_group_config[] =
{
    /* only use group 0, sdk will get intr num represent group 0 from kernel dts */
    {0,  0xff, SAL_TASK_PRIO_DEF, "Function normal group"},
};
#else
ctc_intr_group_t ctc_app_intr_group_config[] =
{
    {0, 16, SAL_TASK_PRIO_DEF,  "other interrupt group"},
    {1, 17, 100,                "DMA and abnormal group"}
};
#endif

/* default interrupt group configuration, need to change the IRQ and priority of groups based on your requirement
 * the priority range is [1, 139] for linux, thereinto [1, 99] is realtime schedule; and [0, 255] for vxworks
 */
ctc_intr_group_t ctc_app_intr_group_config_msi[] =
{
    {0, 0, SAL_TASK_PRIO_DEF,  "msi interrupt group"},
};

uint8 port_mapping_mode = 0;

static ctc_chip_phy_mapping_para_t g_chip_phy_mapping_para;

/**
 @brief get interrupt configuration
*/

static int32
_ctc_app_get_intr_cfg(ctc_intr_global_cfg_t* p_intr_cfg, uint8 interrupt_mode)
{
    uint32 group_count = 0;
    uint32 intr_count = 0;

    /* use default global configuration in sys */
    group_count = interrupt_mode?(COUNTOF(ctc_app_intr_group_config_msi)):(COUNTOF(ctc_app_intr_group_config));
    intr_count = COUNTOF(ctc_app_intr_config);
    if ((group_count > CTC_INTR_MAX_GROUP) || (group_count > CTC_INTR_MAX_GROUP))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((intr_count > CTC_INTR_MAX_INTR) || (intr_count > CTC_INTR_MAX_INTR))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (interrupt_mode)
    {
        p_intr_cfg->group_count = group_count;
        p_intr_cfg->intr_count = intr_count;
        p_intr_cfg->p_group = ctc_app_intr_group_config_msi;
        p_intr_cfg->p_intr = ctc_app_intr_config_msi;
        p_intr_cfg->intr_mode = 1;
    }
    else
    {
        p_intr_cfg->group_count = group_count;
        p_intr_cfg->intr_count = intr_count;
        p_intr_cfg->p_group = ctc_app_intr_group_config;
        p_intr_cfg->p_intr = ctc_app_intr_config;
        p_intr_cfg->intr_mode = 0;
    }
    return CTC_E_NONE;
}


/*
* Routine Description:
*    Set switch attribute value
*
* Arguments:
*    [in] attr - switch attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t ctc_sai_set_switch_attribute(
    _In_ const sai_attribute_t *attr)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    ctc_sai_set_switch_attribute_debug_param(attr);
    CTC_SAI_PTR_VALID_CHECK(attr);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_write_attr(g_sai_attr_entries,attr),ret,out);

    switch(attr->id)
    {        
    case SAI_SWITCH_ATTR_ECMP_HASH:
        ret = ctc_sai_switch_set_ecmp_hash(attr);
        break;
        
    case SAI_SWITCH_ATTR_LAG_HASH:
        ret = ctc_sai_switch_set_lag_hash(attr);
        break;

    case SAI_SWITCH_ATTR_SWITCHING_MODE:
        ret = ctc_sai_switch_set_switch_mode(attr);
        break;

    case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
        ret = ctc_sai_switch_set_bcast_cpu_flood_enable(attr);
        break;

    case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
        ret = ctc_sai_switch_set_mcast_cpu_flood_enable(attr);
        break;

    case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
        ret = ctc_sai_switch_set_src_mac_address(attr);
        break;

    case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
        ret = ctc_sai_switch_set_max_learned_address(attr);
        break;

    case SAI_SWITCH_ATTR_FDB_AGING_TIME:
        ret = ctc_sai_switch_set_aging_time(attr);
        break;

    case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION:
        ret = ctc_sai_switch_set_unicast_miss_action(attr);
        break;

    case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION:
        ret = ctc_sai_switch_set_broadcast_miss_action(attr);
        break;

    case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION:
        ret = ctc_sai_switch_set_multicast_miss_action(attr);
        break;

    case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
        ret = ctc_sai_switch_set_lag_hash_algo(attr);
        break;

    case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
        ret = ctc_sai_switch_set_ecmp_hash_algo(attr);
        break;
        
    case SAI_SWITCH_ATTR_CUSTOM_ECMP_LOAD_BALANCE_MODE:
        ret = ctc_sai_switch_set_ecmp_load_balance_mode(attr);
        break;

    case SAI_SWITCH_ATTR_ECMP_MAX_PATHS:
        ret = ctc_sai_switch_set_max_paths(attr);
        break;

    case SAI_SWITCH_ATTR_IPG_STORM_CONTROL:
        ret = ctc_sai_switch_set_ipg_storm_ctl(attr);
        break;

    case SAI_SWITCH_ATTR_PORT_BREAKOUT:
        ret = SAI_STATUS_NOT_SUPPORTED;
        break;
    case SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP:
    case SAI_SWITCH_ATTR_QOS_EXP_TO_TC_AND_COLOR_MAP:
    case SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP:
    case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
    case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_EXP_MAP:
    case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
        ret = ctc_sai_switch_set_qos_map(attr);
        break;
 
    case SAI_SWITCH_ATTR_ISOLATION_MODE:
        ret = ctc_sai_switch_set_isolation_mode(attr);
        break;
    case SAI_SWITCH_ATTR_CUSTOM_TRUNCTION_LENGTH:
        ret = ctc_sai_switch_set_trunction_length(attr);
#ifdef TAPPRODUCT
        ret = ctc_sai_tap_group_set_trunction_len(attr);
#endif
        break;
    case SAI_SWITCH_ATTR_CUSTOM_TIMESTAMP_ETHER_DMAC:
        ret = ctc_sai_switch_set_timestamp_ether_dmac(attr);
        break;
    case SAI_SWITCH_ATTR_CUSTOM_TIMESTAMP_ETHER_SMAC:
        ret = ctc_sai_switch_set_timestamp_ether_smac(attr);
        break;
    case SAI_SWITCH_ATTR_CUSTOM_TIMESTAMP_ETHER_TYPE:
        ret = ctc_sai_switch_set_timestamp_ether_type(attr);
        break;
    case SAI_SWITCH_ATTR_CUSTOM_DISCARD_SAME_MACDASA_PKT:
        ret = ctc_sai_switch_set_discard_same_macdasa_pkt(attr);
        break;
    case SAI_SWITCH_ATTR_CUSTOM_DISCARD_SAME_IPDASA_PKT:
        ret = ctc_sai_switch_set_discard_same_ipdasa_pkt(attr);
        break;   
    case SAI_SWITCH_ATTR_CUSTOM_DISCARD_MCAST_SA_PKT:
        ret = ctc_sai_switch_set_discard_same_mcast_sa_pkt(attr);
        break;

    case SAI_SWITCH_ATTR_CUSTOM_LAG_HASH_INNER_FIELD_EN:
        ret = ctc_sai_switch_set_lag_hash_inner_field_en(attr);
        break;

    case SAI_SWITCH_ATTR_CUSTOM_SVLAN_TPID:
        ret = ctc_sai_switch_set_svlan_tpid(attr);
        break;

    case SAI_SWITCH_ATTR_CUSTOM_CVLAN_TPID:
        ret = ctc_sai_switch_set_cvlan_tpid(attr);
        break;

    /*added by yejl to discard the ospf packet when system memory is not enough, bug43529, 2017-03-31*/
    case SAI_SWITCH_ATTR_MEMORY_CHECK_STATE:
        ret = ctc_sai_switch_set_memory_check_state(attr);
        break;

    case SAI_SWITCH_ATTR_CUSTOM_IPV6_ENABLE:
        ret = ctc_sai_switch_set_ipv6_enable(attr);
        break;
 
#if defined(GOLDENGATE) || defined(USW)    
    case SAI_SWITCH_ATTR_MONITOR_BUFFER_EN:
        ret = ctc_sai_switch_set_monitor_buffer_enable(attr);   
        break;

/* Added by kcao for bug 50702 : buffer threshold should have gport param */
#if defined(GOLDENGATE)
    case SAI_SWITCH_ATTR_MONITOR_BUFFER_THRESHOLD:
        ret = ctc_sai_switch_set_monitor_buffer_threshold(attr);	
        break;
#endif
  
    case SAI_SWITCH_ATTR_MONITOR_LATENCY_RANGE:
        ret = ctc_sai_switch_set_monitor_latency_range(attr);	
        break;
#endif
    case SAI_SWITCH_ATTR_QOS_IPG_ENABLE:
        ret =  ctc_sai_switch_set_qos_ipg_enable(attr);
        break;
    /*added by yejl for gg1.0 queue stats, bug 54212, 2019-10-31*/  
    case SAI_SWITCH_ATTR_QOS_QUEUE_SHAPE_ENABLE:
        ret =  ctc_sai_switch_set_qos_queue_shape_enable(attr);
        break;

    default:
        break;
       
    }

out:
    return ret;
}


/*
* Routine Description:
*    Get switch attribute value
*
* Arguments:
*    [in] attr_count - number of switch attributes
*    [inout] attr_list - array of switch attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t ctc_sai_get_switch_attribute(
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t                ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t*            attr        = NULL;
    uint32_t                    attr_idx    = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }

        switch(attr->id)
        {
        case SAI_SWITCH_ATTR_PORT_NUMBER:
            ret = ctc_sai_switch_get_port_number(attr);
            break;

        case SAI_SWITCH_ATTR_PORT_LIST:
            ret = ctc_sai_switch_get_port_list_number(attr);
            break;

        case SAI_SWITCH_ATTR_CPU_PORT:
            ret = ctc_sai_switch_get_cpu_port(attr);
            break;

        case SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS:
            ret = ctc_sai_switch_get_vr_max_routers(attr);
            break;

        case SAI_SWITCH_ATTR_FDB_TABLE_SIZE:
            ret = SAI_STATUS_NOT_SUPPORTED;
            break;

        case SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED:
            ret = SAI_STATUS_NOT_SUPPORTED;
            break;

        case SAI_SWITCH_ATTR_OPER_STATUS:
            ret = SAI_STATUS_NOT_SUPPORTED;
            break;

        case SAI_SWITCH_ATTR_MAX_TEMP:
            ret = SAI_STATUS_NOT_SUPPORTED;
            break;

        case SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY:
            ret = SAI_STATUS_NOT_SUPPORTED;
            break;

        case SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY:
            ret = SAI_STATUS_NOT_SUPPORTED;
            break;

        case SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY:
            ret = SAI_STATUS_NOT_SUPPORTED;
            break;

        case SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY:
            ret = SAI_STATUS_NOT_SUPPORTED;
            break;

        case SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID:
            ret = SAI_STATUS_NOT_SUPPORTED;
            break;

        case SAI_SWITCH_ATTR_SWITCHING_MODE:
            ret = ctc_sai_switch_get_switch_mode(attr);
            break;

        case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
            ret = ctc_sai_switch_get_bcast_cpu_flood_enable(attr);
            break;

        case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
            ret = ctc_sai_switch_get_mcast_cpu_flood_enable(attr);
            break;

        case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
            ret = ctc_sai_switch_get_src_mac_address(attr);
            break;

        case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
            ret = ctc_sai_switch_get_max_learned_address(attr);
            break;

        case SAI_SWITCH_ATTR_FDB_AGING_TIME:
            ret = ctc_sai_switch_get_aging_time(attr);
            break;

        case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION:
            ret = ctc_sai_switch_get_unicast_miss_action(attr);
            break;

        case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION:
            ret = ctc_sai_switch_get_broadcast_miss_action(attr);
            break;

        case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION:
            ret = ctc_sai_switch_get_multicast_miss_action(attr);
            break;

        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
            ret = ctc_sai_switch_get_lag_hash_algo(attr);
            break;

        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
            ret = ctc_sai_switch_get_ecmp_hash_algo(attr);
            break;

        case SAI_SWITCH_ATTR_ECMP_MAX_PATHS:
            ret = ctc_sai_switch_get_max_paths(attr);
            break;

        case SAI_SWITCH_ATTR_CUSTOM_SVLAN_TPID:
            ret = ctc_sai_switch_get_svlan_tpid(attr);
            break;

        case SAI_SWITCH_ATTR_PORT_BREAKOUT:
            ret = SAI_STATUS_NOT_SUPPORTED;
            break;
        default:
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

    ctc_sai_get_switch_attribute_debug_param(attr_count, attr_list);
    return ret;
}


sai_status_t
ctc_sai_switch_get_chip_profile(ctc_init_cfg_t * p_init_config, ctc_sai_sdk_init_info_t* p_chip_info)
{
    /*set chip profile default */
    p_chip_info->local_chip_num = 1;
    p_chip_info->gchip[0] = 0;
    p_chip_info->board_type = 0;
    p_chip_info->port_mapping_en = 1;
    p_chip_info->port_phy_mapping_en = 0;
#ifdef _CTC_X86_64_
    p_chip_info->interrupt_mode = 1;
#else
    p_chip_info->interrupt_mode = 0;
#endif
    p_chip_info->profile_type = 0;
    p_chip_info->nh_dedit_mode = 0;
    p_chip_info->ext_nexthop_num = 16384;
#ifdef OFPRODUCT
    p_chip_info->mpls_tunnel_num = 10000;
#else
    p_chip_info->mpls_tunnel_num = 1024;
#endif
    p_chip_info->mpls_entry_num[0] = 7400;
    p_chip_info->mpls_entry_num[1] = 0;
    p_chip_info->mpls_entry_num[2] = 0;
    p_chip_info->mpls_entry_num[3] = 0;
    p_chip_info->fdb_hw_learning_mode = 0;
    p_chip_info->logic_port_num = 1024;
    p_chip_info->stp_mode = 0;
    p_chip_info->stats_queue_deq_en = 1;
    p_chip_info->stats_queue_drop_en = 1;
    p_chip_info->stats_flow_policer_en = 1;
    p_chip_info->stats_vlan_en = 1;
    p_chip_info->stats_vrf_en = 1;
    /*modified by wangl, 1400 stats for flow meter and 200 for port policer, 100 for copp, if need more policer, such as acl policeer enable ,this num should be more*/
    p_chip_info->stats_policer_num = 1700;
    p_chip_info->cut_through_speed = 0;
    p_chip_info->bfd_mep_num = 2002;
    p_chip_info->bfd_vccv_with_ip_en = 1;
    p_chip_info->cpu_port_en = 0;
    p_chip_info->fabric_mode = 0;

    /* modified by liwh for bug 49424, 2018-10-15 for cut through */
    p_chip_info->cut_through_speed_bitmap = 0;
    /* liwh end */

    return CTC_E_NONE;
}

extern int32
ctc_app_read_ftm_profile(const int8* file_name,
                     ctc_ftm_profile_info_t* profile_info);

sai_status_t
ctc_sai_switch_get_cut_through_speed(ctc_sai_sdk_init_info_t* p_sdk_info,  char* profile_name)
{
    FILE *startup_config_fd = NULL;
    char  buf[256];
    bool  cut_through_en = FALSE;
    char  cut_through_enable[7] = {0};
#if defined(GOLDENGATE)
    uint8 cut_through_speed = 1;    
#endif
    char  cut_through_speed_str[32] = {0};
    int32 ret = CTC_E_NONE;

    /*1. first system cut-through enable */
    startup_config_fd = sal_fopen("/mnt/flash/startup-config.conf", "r");
    if (startup_config_fd)
    {
        while (sal_fgets(buf, 128, startup_config_fd))
        {
            ret = sal_sscanf(buf, "cut-through-forwarding %s %s", cut_through_enable, cut_through_speed_str);
            if (1 == ret)
            {
                cut_through_en = TRUE;
                break;
            }
            else if (2 == ret)
            {
                cut_through_en = TRUE;
                /* cut_through_speed for GG : 1:10/40/100G mixed;2:1/10/100G mixed 3:1/10/40G mixed */
#if defined(GOLDENGATE)
                if (!sal_strncmp(cut_through_speed_str, "1G-10G-100G", sal_strlen("1G-10G-100G")))
                {
                    cut_through_speed = 2;
                }
                else if (!sal_strncmp(cut_through_speed_str, "1G-10G-40G", sal_strlen("1G-10G-40G")))
                {
                    cut_through_speed = 3;
                }
                break;
#endif
            }

            if (!(sal_strncmp(buf, "interface", sal_strlen("interface"))))
            {
                break;
            }
        }
        sal_fclose(startup_config_fd);
    }

    if (FALSE == cut_through_en)
    {
        return SAI_STATUS_SUCCESS;
    }

    /*2. set port-speed-mode based board type */

/* modified by liwh for bug 49424, 2018-10-15 for cut through */
#if defined (USW)
    CTC_BIT_SET(p_sdk_info->cut_through_speed_bitmap, CTC_PORT_SPEED_1G);
    CTC_BIT_SET(p_sdk_info->cut_through_speed_bitmap, CTC_PORT_SPEED_2G5);
    CTC_BIT_SET(p_sdk_info->cut_through_speed_bitmap, CTC_PORT_SPEED_5G);
    CTC_BIT_SET(p_sdk_info->cut_through_speed_bitmap, CTC_PORT_SPEED_10G);
    CTC_BIT_SET(p_sdk_info->cut_through_speed_bitmap, CTC_PORT_SPEED_25G);
    CTC_BIT_SET(p_sdk_info->cut_through_speed_bitmap, CTC_PORT_SPEED_40G);
    CTC_BIT_SET(p_sdk_info->cut_through_speed_bitmap, CTC_PORT_SPEED_100G);
/* liwh end */
#elif defined(GOLDENGATE)
    /* GG get cut_through_speed from system configuration */
    p_sdk_info->cut_through_speed = cut_through_speed;
#else
    /* GB get cut_through_speed from profile */
    sal_memset(buf, 0, 256);
    sal_sscanf(profile_name, CTC_ETC_BASE_DIR"/etc/spec/%s", buf);
    if ( 0 == sal_strncmp(buf, "E580_48X2Q4Z_100G.txt", sal_strlen("E580_48X2Q4Z_100G.txt"))
        || 0 == sal_strncmp(buf, "E580_48X2Q4Z.txt", sal_strlen("E580_48X2Q4Z.txt")))    /* 1: 10G-40G-100G */
    {
        p_sdk_info->cut_through_speed = 1;
    }
    else if (0 == sal_strncmp(buf, "E350_48T4XG.txt", sal_strlen("E350_48T4XG.txt"))
        || 0 == sal_strncmp(buf, "E350_PEGATRON.txt", sal_strlen("E350_PEGATRON.txt")))  /* 2: 1G-10G-100G */
    {
        p_sdk_info->cut_through_speed = 2;
    }
    else if (0 == sal_strncmp(buf, "GoldGate_24q.txt", sal_strlen("GoldGate_24q.txt"))
        || 0 == sal_strncmp(buf, "E580_48X6Q.txt", sal_strlen("E580_48X6Q.txt"))
        || 0 == sal_strncmp(buf, "E580_32X2Q.txt", sal_strlen("E580_32X2Q.txt"))
        || 0 == sal_strncmp(buf, "E350_48T4X2Q.txt", sal_strlen("E350_48T4X2Q.txt")))  /* 3: 1G-10G-40G */
    {
        p_sdk_info->cut_through_speed = 3;
    }
    else  /* default */
    {
        p_sdk_info->cut_through_speed = 3;
    }
#endif
 
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_switch_get_sdk_profile(ctc_init_cfg_t * p_init_config, void *arg)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_sai_sdk_init_info_t* p_init_info = NULL;
    uint8 index = 0;
    uint8 mac_id = 0;
    uint8 i = 0;

    p_init_info = (ctc_sai_sdk_init_info_t*)arg;
    
    sal_memset(&g_chip_phy_mapping_para, 0, sizeof(ctc_chip_phy_mapping_para_t));
    p_init_config->phy_mapping_para[0] = &g_chip_phy_mapping_para;
    
    for (index = 0; index < CTC_MAX_PHY_PORT; index++)
    {
        g_chip_phy_mapping_para.port_mdio_mapping_tbl[index] = 0xff;
        g_chip_phy_mapping_para.port_phy_mapping_tbl[index] = 0xff;
        for (i = 0; i < p_init_info->port_num; i++)
        {
            mac_id = p_init_info->phy_map[i].mac_idx;
            if (mac_id == index)
            {
                g_chip_phy_mapping_para.port_mdio_mapping_tbl[index] = p_init_info->phy_map[i].mdio_bus_addr;
                g_chip_phy_mapping_para.port_phy_mapping_tbl[index] = p_init_info->phy_map[i].phy_addr;
            }
        }
    }
    
    /*read chip profile.cfg*/
    CTC_SAI_ERROR_GOTO(ctc_sai_switch_get_chip_profile(p_init_config, p_init_info), ret, out);

    hybrid_profile_type = p_init_info->ftm_profile_type;
#ifdef GOLDENGATE 
    const int8 *file_name = NULL;
    #ifdef OFPRODUCT
    file_name = (const int8 *)SAI_STM_PROFILE_FILE_HYBRID_DEFAULT;
    #else
    file_name = (const int8 *)SAI_STM_PROFILE_FILE_DEFAULT;
    #endif
#ifdef TAPPRODUCT  
    file_name = (const int8 *)SAI_STM_PROFILE_FILE_TAP_DEFAULT;
#endif
    p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_USER_DEFINE;
    ret = ctc_app_read_ftm_profile(file_name, &p_init_config->ftm_info);
    if (ret != CTC_E_NONE)
    {
        p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_0;
    }
#endif
    
#ifdef DUET2
    const int8 *file_name = NULL;

    #ifdef OFPRODUCT
    file_name = (const int8 *)SAI_STM_PROFILE_FILE_HYBRID_DEFAULT;
    #else
    if (GLB_STM_MODE_IPUCV6 == p_init_info->ftm_profile_type) 
    {
        file_name = (const int8 *)SAI_STM_PROFILE_FILE_IPUCV6;
    }
    else
    {
        file_name = (const int8 *)SAI_STM_PROFILE_FILE_DEFAULT;
    }
    #endif
    
#ifdef TAPPRODUCT  
    file_name = (const int8 *)SAI_STM_PROFILE_FILE_TAP_DEFAULT;
#endif
    p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_USER_DEFINE;
    ret = ctc_app_read_ftm_profile(file_name, &p_init_config->ftm_info);
    if (ret != CTC_E_NONE)
    {
        p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_0;
    }
    ftm_info_profile_type_d2 = p_init_info->ftm_profile_type;
#endif

#ifdef TSINGMA
    const int8 *file_name = NULL;

    #ifdef OFPRODUCT
    file_name = (const int8 *)SAI_STM_PROFILE_FILE_HYBRID_DEFAULT;
    #else
    if (GLB_STM_MODE_IPUCV6 == p_init_info->ftm_profile_type) 
    {
        file_name = (const int8 *)SAI_STM_PROFILE_FILE_IPUCV6;
    }
    else
    {
        file_name = (const int8 *)SAI_STM_PROFILE_FILE_DEFAULT;
    }
    #endif
    
#ifdef TAPPRODUCT  
    file_name = (const int8 *)SAI_STM_PROFILE_FILE_TAP_DEFAULT;
#endif
    p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_USER_DEFINE;
    ret = ctc_app_read_ftm_profile(file_name, &p_init_config->ftm_info);
    if (ret != CTC_E_NONE)
    {
        p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_0;
    }
    ftm_info_profile_type_tm = p_init_info->ftm_profile_type;
#endif
 
#ifdef GREATBELT
#if defined(OFDPAPRODUCT)
    p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_0;
#elif defined(TAPPRODUCT)
	if(0 == p_init_info->ftm_profile_type)
	{
		p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_3;
	}else{
		p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_11;
	}
#elif defined(OFPRODUCT)
    if (GLB_STM_MODE_OF_IPV6 == p_init_info->ftm_profile_type)
    {
        p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_5;
    }
    else
    {
        p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_3;
    }
#else
    /* modified by liwh for bug 47547, 2018-08-16 */
    if(4 == p_init_info->ftm_profile_type)
    {
        p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_5;
    }
    /* liwh end */
    else
    {
        p_init_config->ftm_info.profile_type = CTC_FTM_PROFILE_3;
    }
    ftm_info_profile_type_gb = p_init_config->ftm_info.profile_type;
#endif
#endif
    
    /*read datapath_cfg.cfg*/
    ret = ctc_app_get_datapath_profile(p_init_info->datapath_config_file, p_init_config->p_datapath_cfg);
    if (ret < 0)
    {
        if (1 == SDK_WORK_PLATFORM)
        {
            for (index = 0; index < CTC_DATAPATH_SERDES_NUM; index++)
            {
                p_init_config->p_datapath_cfg->serdes[index].mode = CTC_CHIP_SERDES_XFI_MODE;
            }

            p_init_config->p_datapath_cfg->core_frequency_a = 600;
            p_init_config->p_datapath_cfg->core_frequency_b = 500;
        }
    }

    ctc_sai_switch_get_cut_through_speed(p_init_info, (char *)p_init_info->datapath_config_file);

    return SAI_STATUS_SUCCESS;
out:
    return ret;
}

sai_status_t
ctc_sai_switch_get_sdk_dma_cfg(ctc_dma_global_cfg_t* p_dma_cfg)
{
    uint32 pkt_rx_size_per_desc = 0;
    
    CTC_PTR_VALID_CHECK(p_dma_cfg);

    sal_memset(p_dma_cfg, 0, sizeof(ctc_dma_global_cfg_t));
    
#ifdef _GLB_UML_SYSTEM_
    pkt_rx_size_per_desc    = 256 * 8;
#else
    pkt_rx_size_per_desc    = 256 * 38;
#endif

#if defined (GREATBELT)
    p_dma_cfg->stats.desc_num          = 16;
    p_dma_cfg->stats.priority          = SAL_TASK_PRIO_DEF;
    p_dma_cfg->learning.desc_num       = 1024;
    p_dma_cfg->learning.priority       = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[0].desc_num      = 512;
    p_dma_cfg->pkt_rx[0].priority      = SAL_TASK_PRIO_NICE_HIGH;
    p_dma_cfg->pkt_rx[1].desc_num      = 64;
    p_dma_cfg->pkt_rx[1].priority      = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[2].desc_num      = 64;
    p_dma_cfg->pkt_rx[2].priority      = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[3].desc_num      = 64;
    p_dma_cfg->pkt_rx[3].priority      = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx_chan_num         = 1;
    p_dma_cfg->pkt_tx_desc_num         = 16;
    p_dma_cfg->table_r_desc_num        = 1;
    p_dma_cfg->table_w_desc_num        = 1;
    p_dma_cfg->pkt_rx_size_per_desc    = pkt_rx_size_per_desc;
    p_dma_cfg->learning_proc_func      = NULL;
    p_dma_cfg->func_en_bitmap          = 0;
    p_dma_cfg->hw_learning_sync_en = 0;
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_PACKET_RX);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_PACKET_TX);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_TABLE_W);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_TABLE_R);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_HW_LEARNING);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_STATS);
#else
    p_dma_cfg->learning.desc_num   = 128;
    p_dma_cfg->learning.priority       = SAL_TASK_PRIO_DEF;
    p_dma_cfg->learning.dmasel       = 0;

    p_dma_cfg->ipfix.desc_num        = 16;
    p_dma_cfg->ipfix.priority            = SAL_TASK_PRIO_DEF;
    p_dma_cfg->ipfix.dmasel            = 0;

    /* must config 4 channel for dma packet rx TODO, channel number should get from cpureason module*/
    p_dma_cfg->pkt_rx_chan_num = 4;
    p_dma_cfg->pkt_rx[0].desc_num  = 64; 
    p_dma_cfg->pkt_rx[0].priority      = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[0].dmasel      = 0;
    p_dma_cfg->pkt_rx[0].data         = pkt_rx_size_per_desc;
    p_dma_cfg->pkt_rx[1].desc_num  = 32;
    p_dma_cfg->pkt_rx[1].priority      = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[1].dmasel      = 0;
    p_dma_cfg->pkt_rx[1].data         = pkt_rx_size_per_desc;
    p_dma_cfg->pkt_rx[2].desc_num  = 32;
    p_dma_cfg->pkt_rx[2].priority      = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[2].dmasel      = 0;
    p_dma_cfg->pkt_rx[2].data      = pkt_rx_size_per_desc;
    p_dma_cfg->pkt_rx[3].desc_num      = 32;
    p_dma_cfg->pkt_rx[3].priority      = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[3].dmasel      = 0;
    p_dma_cfg->pkt_rx[3].data      = pkt_rx_size_per_desc;

    p_dma_cfg->pkt_tx.desc_num = 32;
    p_dma_cfg->pkt_tx.priority = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_tx.dmasel = 0;

    p_dma_cfg->func_en_bitmap          = 0;
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_PACKET_RX);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_PACKET_TX);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_HW_LEARNING);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_IPFIX);

    /*UML do not enable monitor and stats */
    if (0 == SDK_WORK_PLATFORM)
    {
        CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_MONITOR);
        CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_STATS);
    }
#endif


#if 0
    uint32 pkt_rx_size_per_desc = 0;

    CTC_PTR_VALID_CHECK(p_dma_cfg);

    sal_memset(p_dma_cfg, 0, sizeof(ctc_dma_global_cfg_t));
#ifdef _GLB_UML_SYSTEM_
    pkt_rx_size_per_desc    = 256 ;
#else
    pkt_rx_size_per_desc    = 256 ;
#endif

//#if defined (GREATBELT)
#ifdef GREATBELT
    /* init dma */
    p_dma_cfg->stats.desc_num          = 16;
    p_dma_cfg->stats.priority          = SAL_TASK_PRIO_DEF;
    p_dma_cfg->learning.desc_num       = 1024;
    p_dma_cfg->learning.priority       = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[0].desc_num      = 512;
    p_dma_cfg->pkt_rx[0].priority      = SAL_TASK_PRIO_NICE_HIGH;
    p_dma_cfg->pkt_rx[1].desc_num      = 64;
    p_dma_cfg->pkt_rx[1].priority      = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[2].desc_num      = 64;
    p_dma_cfg->pkt_rx[2].priority      = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[3].desc_num      = 64;
    p_dma_cfg->pkt_rx[3].priority      = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx_chan_num         = 1;   /* Modified by Percy Wang for bug 26248, 2014-03-01 */
    p_dma_cfg->pkt_tx_desc_num         = 16;
    p_dma_cfg->table_r_desc_num        = 1;
    p_dma_cfg->table_w_desc_num        = 1;
    //p_dma_cfg->pkt_rx_size_per_desc    = 1536;
    /* modified by wangjj for fix dma packet length, 2016-12-14 */
    p_dma_cfg->pkt_rx_size_per_desc    = pkt_rx_size_per_desc;
    p_dma_cfg->func_en_bitmap          = 0;

    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_PACKET_RX);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_PACKET_TX);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_TABLE_W);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_TABLE_R);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_HW_LEARNING);

#else
    p_dma_cfg->learning.desc_num   = 64;    /* modified by kcao for HW FDB, DMA Cache, SW FDB consistent */
    p_dma_cfg->learning.priority       = SAL_TASK_PRIO_DEF;
    p_dma_cfg->learning.dmasel       = 0;

    p_dma_cfg->ipfix.desc_num        = 64;
    p_dma_cfg->ipfix.priority            = SAL_TASK_PRIO_DEF;
    p_dma_cfg->ipfix.dmasel            = 0;

    /* must config 4 channel for dma packet rx TODO, channel number should get from cpureason module*/
    p_dma_cfg->pkt_rx_chan_num          = 1;
    p_dma_cfg->pkt_rx[0].desc_num       = 64;
    p_dma_cfg->pkt_rx[0].priority       = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[0].dmasel         = 0;
    p_dma_cfg->pkt_rx[0].data           = pkt_rx_size_per_desc;
    p_dma_cfg->pkt_rx[1].desc_num       = 64;
    p_dma_cfg->pkt_rx[1].priority       = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[1].dmasel         = 0;
    p_dma_cfg->pkt_rx[1].data           = pkt_rx_size_per_desc;
    p_dma_cfg->pkt_rx[2].desc_num       = 64;
    p_dma_cfg->pkt_rx[2].priority       = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[2].dmasel         = 0;
    p_dma_cfg->pkt_rx[2].data           = pkt_rx_size_per_desc;
    p_dma_cfg->pkt_rx[3].desc_num       = 64;
    p_dma_cfg->pkt_rx[3].priority       = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_rx[3].dmasel         = 0;
    p_dma_cfg->pkt_rx[3].data           = pkt_rx_size_per_desc;

    p_dma_cfg->pkt_tx.desc_num = 32;
    p_dma_cfg->pkt_tx.priority = SAL_TASK_PRIO_DEF;
    p_dma_cfg->pkt_tx.dmasel = 0;

    p_dma_cfg->func_en_bitmap          = 0;
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_PACKET_RX);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_PACKET_TX);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_HW_LEARNING);
    CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_IPFIX);

    /*UML do not enable monitor and stats */
#ifndef _GLB_UML_SYSTEM_
    {
        //CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_MONITOR);
        CTC_BIT_SET(p_dma_cfg->func_en_bitmap, CTC_DMA_FUNC_STATS);
    }
#endif
#endif
#endif

    return SAI_STATUS_SUCCESS;
}

/* Get sdk module init param*/
sai_status_t
ctc_sai_switch_get_sdk_init_para(ctc_init_cfg_t * p_init_config, ctc_sai_sdk_init_info_t* p_chip_info)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    uint8 space_id = 0;
    uint32 size = 0;
    uint32 size_type = 0;

    CTC_PTR_VALID_CHECK(p_init_config);
    CTC_PTR_VALID_CHECK(p_chip_info);
    
    /*1. Get Nexthop module init parameter*/

#define MAX_EXTERNAL_NHNUM          16384
#define ACL_REDIRECT_FWD_PTR_NUM    1024
#define CTC_APP_MPLS_TBL_BLOCK_SIZE 64
    /*1. Get Nexthop module init parameter*/
    p_init_config->p_nh_cfg->max_external_nhid        = p_chip_info->ext_nexthop_num
        ? p_chip_info->ext_nexthop_num : MAX_EXTERNAL_NHNUM;
    p_init_config->p_nh_cfg->acl_redirect_fwd_ptr_num = ACL_REDIRECT_FWD_PTR_NUM;

    /*add by chenc , fix bug 44975*/
    #if defined GOLDENGATE || defined (USW)
    p_init_config->p_nh_cfg->max_ecmp                 = CTC_SAI_DEFAULT_MAX_ECMP_NUM;
    #else
    p_init_config->p_nh_cfg->max_ecmp                 = CTC_SAI_DEFAULT_MAX_ECMP_NUM_GB;
    #endif
    
    p_init_config->p_nh_cfg->nh_edit_mode             = p_chip_info->nh_dedit_mode;
    p_init_config->p_nh_cfg->max_tunnel_id            = p_chip_info->mpls_tunnel_num
        ? p_chip_info->mpls_tunnel_num : CTC_NH_DEFAULT_MAX_MPLS_TUNNEL_NUM;
    
    #if defined (USW)
    p_init_config->p_port_cfg->isolation_group_mode = CTC_ISOLATION_GROUP_MODE_64;
    p_init_config->p_port_cfg->use_isolation_id = 1;
    #endif
    
    /*2. Get Oam module init parameter*/
#if defined(GOLDENGATE) || defined(GREATBELT)
    p_init_config->p_oam_cfg->is_p2p_mode                  = 1;
#endif
    p_init_config->p_oam_cfg->maid_len_format              = 3;
    /* modified by liwh for bug 53010, 2019-07-22 */
    //p_init_config->p_oam_cfg->mep_index_alloc_by_sdk       = 1;
    p_init_config->p_oam_cfg->mep_index_alloc_by_sdk       = 0;
    /* liwh end */
    p_init_config->p_oam_cfg->tp_section_oam_based_l3if    = 0;
    p_init_config->p_oam_cfg->tp_y1731_ach_chan_type       = 0x8902;
    p_init_config->p_oam_cfg->mep_1ms_num                  = p_chip_info->bfd_mep_num?
        p_chip_info->bfd_mep_num : 500;
    p_init_config->p_oam_cfg->tp_bfd_333ms                 = 0;
/* system modified by liwh for bug 53010, 2019-07-31 */
#ifdef GREATBELT
    p_init_config->p_oam_cfg->mpls_pw_vccv_with_ip_en      = 0;
#else
/* liwh end */
    p_init_config->p_oam_cfg->mpls_pw_vccv_with_ip_en      = p_chip_info->bfd_vccv_with_ip_en;
#endif
    p_init_config->p_oam_cfg->tp_csf_ach_chan_type         = 0x40;
    p_init_config->p_oam_cfg->tp_csf_clear                 = 7;
    p_init_config->p_oam_cfg->tp_csf_fdi                   = 1;
    p_init_config->p_oam_cfg->tp_csf_los                   = 0;
    p_init_config->p_oam_cfg->tp_csf_rdi                   = 2;
        
    /*3. Get Interrupt module init parameter*/
    _ctc_app_get_intr_cfg(p_init_config->p_intr_cfg, p_chip_info->interrupt_mode);

    /* Added by liuht for exceptive irq number definition which is defined in spec file */
    if(p_chip_info->irq_type_exception)
    {
        int i;
        
        if(p_chip_info->irq1)
        {
            p_init_config->p_intr_cfg->p_group[0].group = 0;
            p_init_config->p_intr_cfg->p_group[0].irq   = p_chip_info->irq1;
        }
        else
        {
            p_init_config->p_intr_cfg->p_group[0].group = -1;
        }

        if(p_chip_info->irq2)
        {
            p_init_config->p_intr_cfg->p_group[1].group = 1;
            p_init_config->p_intr_cfg->p_group[1].irq   = p_chip_info->irq2;
        }
        else
        {
            p_init_config->p_intr_cfg->p_group[1].group = -1;
            for(i = 0; i < p_init_config->p_intr_cfg->intr_count; i++)
            {
                p_init_config->p_intr_cfg->p_intr[i].group = 0;
            }
        }
    }

    /*4. Get Stats module init parameter*/
    p_init_config->p_stats_cfg->stats_bitmap     |= CTC_STATS_ECMP_STATS;
    p_init_config->p_stats_cfg->stats_bitmap     |= p_chip_info->stats_queue_deq_en ? CTC_STATS_QUEUE_DEQ_STATS
                                                    : p_init_config->p_stats_cfg->stats_bitmap;
    p_init_config->p_stats_cfg->stats_bitmap     |= p_chip_info->stats_queue_drop_en ? CTC_STATS_QUEUE_DROP_STATS
                                                    : p_init_config->p_stats_cfg->stats_bitmap;
    p_init_config->p_stats_cfg->stats_bitmap     |= p_chip_info->stats_flow_policer_en ? CTC_STATS_FLOW_POLICER_STATS
                                                    : p_init_config->p_stats_cfg->stats_bitmap;
    p_init_config->p_stats_cfg->stats_bitmap     |= p_chip_info->stats_vlan_en? CTC_STATS_VLAN_STATS
                                                    : p_init_config->p_stats_cfg->stats_bitmap;
    p_init_config->p_stats_cfg->stats_bitmap     |= p_chip_info->stats_vrf_en ? CTC_STATS_VRF_STATS
                                                    : p_init_config->p_stats_cfg->stats_bitmap;
    p_init_config->p_stats_cfg->policer_stats_num = p_chip_info->stats_policer_num;
    

    /*5. Get Fdb module init parameter*/
    p_init_config->p_l2_fdb_cfg->flush_fdb_cnt_per_loop = 0;
    p_init_config->p_l2_fdb_cfg->default_entry_rsv_num  = 4 * 1024 + p_chip_info->logic_port_num;
    p_init_config->p_l2_fdb_cfg->hw_learn_en            = p_chip_info->fdb_hw_learning_mode;
    p_init_config->p_l2_fdb_cfg->logic_port_num         = p_chip_info->logic_port_num;
    p_init_config->p_l2_fdb_cfg->stp_mode               = p_chip_info->stp_mode;

    /*6. Get Mpls module init parameter*/
    for (space_id = 0; space_id < 4; space_id++)
    {
        p_init_config->p_mpls_cfg->space_info[space_id].enable = FALSE;
        size = p_chip_info->mpls_entry_num[space_id]/CTC_APP_MPLS_TBL_BLOCK_SIZE;
        size_type = 0;
        if (size)
        {
            while (size > 1)
            {
                size >>= 1;
                size_type++;
            }

            p_init_config->p_mpls_cfg->space_info[space_id].enable = TRUE;
            p_init_config->p_mpls_cfg->space_info[space_id].sizetype = size_type;
        }
    }

    /*7. Get Dma module init parameter*/
    CTC_SAI_ERROR_GOTO(
    (ctc_sai_switch_get_sdk_dma_cfg(p_init_config->p_dma_cfg)), ret, out);


    /*8. Get packet module init parameter*/
    p_init_config->p_pkt_cfg->rx_cb         = ctc_packet_receive_from_sdk;

    /*9. Get bpe module init parameter*/
    p_init_config->p_bpe_cfg->is_port_extender = p_chip_info->bpe_br_pe_en;
    p_init_config->p_bpe_cfg->max_uc_ecid_id = p_chip_info->bpe_br_uc_max_ecid;
    p_init_config->p_bpe_cfg->max_mc_ecid_id = p_chip_info->bpe_br_mc_max_ecid;
    p_init_config->p_bpe_cfg->port_base = p_chip_info->bpe_br_port_base;



    /*11. Get parser module init parameter*/
    p_init_config->p_parser_cfg->ecmp_hash_type = 0;
    p_init_config->p_parser_cfg->linkagg_hash_type = 0;

    p_init_config->p_stats_cfg->stats_mode = CTC_STATS_MODE_DEFINE;
    p_init_config->p_stats_cfg->query_mode = CTC_STATS_QUERY_MODE_IO;
    p_init_config->p_stats_cfg->stats_bitmap = 
        (CTC_STATS_QUEUE_DEQ_STATS | CTC_STATS_QUEUE_DROP_STATS | CTC_STATS_PORT_POLICER_STATS | CTC_STATS_FLOW_POLICER_STATS | CTC_STATS_FLOW_STATS | CTC_STATS_ECMP_STATS);

    /*10 Get chip config*/
/* modified by liwh for bug 49424, 2018-10-15 for cut through */
#if defined (USW)
    p_init_config->p_chip_cfg->cut_through_en       = ((0 != p_chip_info->cut_through_speed_bitmap)?1:0);
    p_init_config->p_chip_cfg->cut_through_speed    = p_chip_info->cut_through_speed;
    p_init_config->p_chip_cfg->cut_through_speed_bitmap    = p_chip_info->cut_through_speed_bitmap;
#else
/* liwh end */
    p_init_config->p_chip_cfg->cut_through_en       = ((0 != p_chip_info->cut_through_speed)?1:0);
    p_init_config->p_chip_cfg->cut_through_speed    = p_chip_info->cut_through_speed;
#endif

    p_init_config->p_chip_cfg->ecc_recover_en       = p_chip_info->ecc_recover_en;
    p_init_config->p_chip_cfg->tcam_scan_en         = p_chip_info->tcam_scan_en;
    p_init_config->p_chip_cfg->cpu_port_en = p_chip_info->cpu_port_en;
    p_init_config->p_chip_cfg->cpu_port = p_chip_info->cpu_port;


    __set_cut_through_en(p_chip_info->cut_through_speed);

    if (p_init_config->p_chip_cfg->cpu_port_en)
    {
        p_init_config->p_chip_cfg->cpu_mac_da[0][0] = 0xFE;
        p_init_config->p_chip_cfg->cpu_mac_da[0][1] = 0xFD;
        p_init_config->p_chip_cfg->cpu_mac_da[0][2] = 0x00;
        p_init_config->p_chip_cfg->cpu_mac_da[0][3] = 0x00;
        p_init_config->p_chip_cfg->cpu_mac_da[0][4] = 0x00;
        p_init_config->p_chip_cfg->cpu_mac_da[0][5] = 0x00;

        p_init_config->p_chip_cfg->cpu_mac_sa[0] = 0xFE;
        p_init_config->p_chip_cfg->cpu_mac_sa[1] = 0xFD;
        p_init_config->p_chip_cfg->cpu_mac_sa[2] = 0x00;
        p_init_config->p_chip_cfg->cpu_mac_sa[3] = 0x00;
        p_init_config->p_chip_cfg->cpu_mac_sa[4] = 0x00;
        p_init_config->p_chip_cfg->cpu_mac_sa[5] = 0x01;

        p_init_config->p_chip_cfg->tpid = 0x8100;
        p_init_config->p_chip_cfg->vlanid = 0x23;
    }

    /*11 Get qos config*/
    if (p_init_config->p_qos_cfg)
    {
        p_init_config->p_qos_cfg->policer_num = p_chip_info->policer_num
                                                ? p_chip_info->policer_num : 4096;
#ifdef OFPRODUCT
        p_init_config->p_qos_cfg->policer_num = 4096;
#endif
#ifdef TSINGMA  /*modified by yejl for bug 53395, 2019-10-29*/
        p_init_config->p_qos_cfg->queue_num_per_network_port    = p_chip_info->queue_num_per_network_port
                                                                    ? p_chip_info->queue_num_per_network_port:8;
#else
        p_init_config->p_qos_cfg->queue_num_per_network_port    = p_chip_info->queue_num_per_network_port
                                                                    ? p_chip_info->queue_num_per_network_port:16;
#endif
        p_init_config->p_qos_cfg->queue_num_for_cpu_reason      = p_chip_info->queue_num_for_cpu_reason
                                                                    ? p_chip_info->queue_num_for_cpu_reason : 128;
        p_init_config->p_qos_cfg->max_cos_level = 3;
    }

    p_init_config->local_chip_num = p_chip_info->local_chip_num;
    sal_memcpy(p_init_config->gchip, p_chip_info->gchip, sizeof(uint8)*CTC_MAX_LOCAL_CHIP_NUM);

    /*12 Get stacking config*/
    p_init_config->p_stacking_cfg->hdr_glb.mac_da_chk_en     = 0;
    p_init_config->p_stacking_cfg->hdr_glb.ether_type_chk_en = 0;
    p_init_config->p_stacking_cfg->hdr_glb.vlan_tpid         = 0x8100;
    p_init_config->p_stacking_cfg->hdr_glb.ether_type        = 0x55bb;
    p_init_config->p_stacking_cfg->hdr_glb.ip_protocol       = 255;
    p_init_config->p_stacking_cfg->hdr_glb.udp_dest_port     = 0x1234;
    p_init_config->p_stacking_cfg->hdr_glb.udp_src_port      = 0x5678;
    p_init_config->p_stacking_cfg->hdr_glb.udp_en            = 0;
    p_init_config->p_stacking_cfg->hdr_glb.ip_ttl            = 255;
    p_init_config->p_stacking_cfg->hdr_glb.ip_dscp           = 63;
    p_init_config->p_stacking_cfg->hdr_glb.cos               = 7;
    p_init_config->p_stacking_cfg->hdr_glb.is_ipv4           = 1;
    p_init_config->p_stacking_cfg->hdr_glb.ipsa.ipv4         = 0x11223344;
    p_init_config->p_stacking_cfg->fabric_mode = p_chip_info->fabric_mode;

    p_init_config->p_stacking_cfg->connect_glb.member_num = 1;
    p_init_config->p_stacking_cfg->connect_glb.mode = 1;
/* modified by liul for gb osp, 2016-05-20 */
#if defined GOLDENGATE || defined (USW)
    /*13 added by kcao for bug 37906*/
#ifdef TAPPRODUCT
    p_init_config->p_linkagg_cfg->linkagg_mode = CTC_LINKAGG_MODE_16;
#else
    p_init_config->p_linkagg_cfg->linkagg_mode = CTC_LINKAGG_MODE_56;
#endif
#else /* GB only support MAX 64 */
    p_init_config->p_linkagg_cfg->linkagg_mode = CTC_LINKAGG_MODE_64;
#endif

#if 0 /* move to ctc_sai_switch_get_sdk_config:ctc_sai_switch_get_init_flag */
    /*14 get module init flag*/
    p_init_config->init_flag = p_chip_info->init_flag; 
#endif
    
    return SAI_STATUS_SUCCESS;
out:
    return ret;


}

static int32
ctc_sai_switch_get_init_flag(uint32* p_init_flag, void *arg)
{
#ifndef OFPRODUCT
    ctc_sai_sdk_init_info_t* p_init_info = NULL;
    p_init_info = (ctc_sai_sdk_init_info_t*)arg;
#endif

    CTC_SET_FLAG(*p_init_flag, CTC_INIT_MODULE_OVERLAY);
    CTC_SET_FLAG(*p_init_flag, CTC_INIT_MODULE_MONITOR);
    CTC_SET_FLAG(*p_init_flag, CTC_INIT_MODULE_EFD);

#ifdef OFPRODUCT
/* modified by liwh for bug 53843, 2019-10-08 */
//#if defined GOLDENGATE || defined (USW) 
#if defined GOLDENGATE || defined (USW) || defined (GREATBELT)
/* liwh end */
/*modified by yejl for d2 tpoam & aps-group, 2019-05-09*/
    CTC_SET_FLAG(*p_init_flag, CTC_INIT_MODULE_APS);
    CTC_SET_FLAG(*p_init_flag, CTC_INIT_MODULE_OAM);
#endif
#else
#ifndef TAPPRODUCT   
#if defined GOLDENGATE || defined (GREATBELT)
    CTC_SET_FLAG(*p_init_flag, CTC_INIT_MODULE_APS);
    CTC_SET_FLAG(*p_init_flag, CTC_INIT_MODULE_OAM);
#endif
#endif
#endif

#ifndef OFPRODUCT
    /* Modified by liuht to support disable ptp, 2017-04-14 */
    if(!p_init_info->disable_ptp)
    {
        CTC_SET_FLAG(*p_init_flag, CTC_INIT_MODULE_PTP);
    }
    else
    {
        CTC_UNSET_FLAG(*p_init_flag, CTC_INIT_MODULE_PTP);
    }
    /* Modified by tongzb to support configure synce, 2019-07-10 */
    if(!p_init_info->disable_synce)
    {
        CTC_SET_FLAG(*p_init_flag, CTC_INIT_MODULE_SYNCE);
    }
    else
    {
        CTC_UNSET_FLAG(*p_init_flag, CTC_INIT_MODULE_SYNCE);
    }
#endif
    return CTC_E_NONE;
}

sai_status_t
ctc_sai_switch_get_sdk_config(ctc_init_cfg_t * p_init_config, void *arg)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_sai_sdk_init_info_t chip_info;
    CTC_PTR_VALID_CHECK(p_init_config);
    
    sal_memset(&chip_info, 0, sizeof(ctc_sai_sdk_init_info_t));

    /*Get init flag */
    CTC_ERROR_RETURN(ctc_sai_switch_get_init_flag(&p_init_config->init_flag, arg));
   
    /*Get profile parameter*/
    CTC_SAI_ERROR_GOTO((ctc_sai_switch_get_sdk_profile(p_init_config, arg)), ret, out);

    /*Get Module init parameter*/
    CTC_SAI_ERROR_GOTO((ctc_sai_switch_get_sdk_init_para(p_init_config, arg)), ret, out);

#if defined(OFDPAPRODUCT)
    /*
     * Enable all module, must put this behind ctc_sai_switch_get_sdk_init_para
     * because it cleared p_init_config->init_flag at its end
     */
    sal_memset(&p_init_config->init_flag, 0xff, sizeof p_init_config->init_flag);
#endif
    
    return SAI_STATUS_SUCCESS;
out:
    return ret;
}

void
ctc_sai_switch_sdk_cli_thread(void *arg)
{
    arg = arg;
    
    /* The function will block. */
    ctc_master_cli(TRUE);
#ifdef USW
    sal_task_sleep(5000);
    ctc_cli_read(TRUE);
#endif
}


sai_status_t
ctc_sai_switch_sdk_init(void* arg)
{
    int32 ret = 0;
    ctc_init_cfg_t* init_config = NULL;
    ctc_pkt_global_cfg_t pkt_cfg;
    ctc_nh_global_cfg_t    nh_global_cfg;
    ctc_oam_global_t  oam_global;
    ctc_l2_fdb_global_cfg_t l2_fdb_global_cfg;
    ctc_mpls_init_t* mpls_cfg = NULL;
    ctc_intr_global_cfg_t intr_cfg;
    ctc_dma_global_cfg_t* dma_cfg = NULL;
    ctc_bpe_global_cfg_t bpe_cfg;
    ctc_chip_global_cfg_t   chip_cfg;
    ctc_datapath_global_cfg_t* datapath_cfg = NULL;
    ctc_qos_global_cfg_t*      qos_cfg  =NULL;
    ctc_ipuc_global_cfg_t      ipuc_cfg;
    ctc_stats_global_cfg_t stats_cfg;
    ctc_stacking_glb_cfg_t*      stacking_cfg  =NULL;
//    uint8 lchip = 0;
    ctc_ftm_key_info_t* ftm_key_info = NULL;
    ctc_ftm_tbl_info_t* ftm_tbl_info = NULL;
    ctc_parser_global_cfg_t parser_cfg;
    ctc_linkagg_global_cfg_t lag_cfg;
    ctc_wb_api_t wb_api;
    ctc_overlay_tunnel_global_cfg_t overlay_cfg;
    ctc_port_global_cfg_t port_cfg;
    
    sal_task_t *sai_sdk_cli_thread;


    ftm_key_info = (ctc_ftm_key_info_t*)XMALLOC(MEM_CTC_SAI_INFO, sizeof(ctc_ftm_key_info_t)*CTC_FTM_KEY_TYPE_MAX*3);
    ftm_tbl_info = (ctc_ftm_tbl_info_t*)XMALLOC(MEM_CTC_SAI_INFO, sizeof(ctc_ftm_tbl_info_t)*CTC_FTM_TBL_TYPE_MAX);
    init_config = (ctc_init_cfg_t*)XMALLOC(MEM_CTC_SAI_INFO, sizeof(ctc_init_cfg_t));
    mpls_cfg = (ctc_mpls_init_t*)XMALLOC(MEM_CTC_SAI_INFO, sizeof(ctc_mpls_init_t));
    datapath_cfg = (ctc_datapath_global_cfg_t*)XMALLOC(MEM_CTC_SAI_INFO, sizeof(ctc_datapath_global_cfg_t));
    dma_cfg = (ctc_dma_global_cfg_t*)XMALLOC(MEM_CTC_SAI_INFO, sizeof(ctc_dma_global_cfg_t));
    qos_cfg = (ctc_qos_global_cfg_t*)XMALLOC(MEM_CTC_SAI_INFO, sizeof(ctc_qos_global_cfg_t));
    stacking_cfg = (ctc_stacking_glb_cfg_t*)XMALLOC(MEM_CTC_SAI_INFO, sizeof(ctc_stacking_glb_cfg_t));
    if ((NULL == ftm_key_info) || (NULL == ftm_tbl_info) || (NULL == init_config)
        || (NULL == mpls_cfg) || (NULL == datapath_cfg)
        || (NULL == dma_cfg) || (NULL == qos_cfg) || (NULL == stacking_cfg))
    {
        ret = CTC_E_NO_MEMORY;
        goto EXIT;
    }

    sal_memset(&wb_api, 0, sizeof(ctc_wb_api_t));
    sal_memset(init_config,0,sizeof(ctc_init_cfg_t));
    sal_memset(&pkt_cfg, 0,  sizeof(ctc_pkt_global_cfg_t));
    sal_memset(&nh_global_cfg,0,sizeof(ctc_nh_global_cfg_t));
    sal_memset(&oam_global,0,sizeof(ctc_oam_global_t));
    sal_memset(&l2_fdb_global_cfg,0,sizeof(ctc_l2_fdb_global_cfg_t));
    sal_memset(mpls_cfg,0,sizeof(ctc_mpls_init_t));
    sal_memset(&intr_cfg,0,sizeof(ctc_intr_global_cfg_t));
    sal_memset(dma_cfg,0,sizeof(ctc_dma_global_cfg_t));
    sal_memset(ftm_key_info, 0, 3*CTC_FTM_KEY_TYPE_MAX * sizeof(ctc_ftm_key_info_t));
    sal_memset(ftm_tbl_info, 0, CTC_FTM_TBL_TYPE_MAX * sizeof(ctc_ftm_tbl_info_t));
    sal_memset(&bpe_cfg,0,sizeof(ctc_bpe_global_cfg_t));
    sal_memset(&chip_cfg,0,sizeof(ctc_chip_global_cfg_t));
    sal_memset(datapath_cfg,0,sizeof(ctc_datapath_global_cfg_t));
    sal_memset(qos_cfg,0,sizeof(ctc_qos_global_cfg_t));
    sal_memset(&ipuc_cfg,0,sizeof(ctc_ipuc_global_cfg_t));
    sal_memset(&stats_cfg,0,sizeof(ctc_stats_global_cfg_t));
    sal_memset(stacking_cfg,0,sizeof(ctc_stacking_glb_cfg_t));
    sal_memset(&parser_cfg, 0, sizeof(parser_cfg));
    sal_memset(&lag_cfg, 0, sizeof(lag_cfg));
    sal_memset(&overlay_cfg,0,sizeof(ctc_overlay_tunnel_global_cfg_t));
    sal_memset(&port_cfg,0,sizeof(port_cfg));

    /* Config module init parameter, if set init_config.p_MODULE_cfg = NULL, using SDK default configration */
    init_config->p_pkt_cfg = &pkt_cfg;
    init_config->p_nh_cfg    = &nh_global_cfg;
    init_config->p_oam_cfg   = &oam_global;
    init_config->p_l2_fdb_cfg= &l2_fdb_global_cfg;
    init_config->p_mpls_cfg  = mpls_cfg;
    init_config->p_intr_cfg  = &intr_cfg;
    init_config->p_dma_cfg = dma_cfg;
    init_config->ftm_info.key_info = ftm_key_info;
    init_config->ftm_info.tbl_info = ftm_tbl_info;
    init_config->p_bpe_cfg = &bpe_cfg;
    init_config->p_chip_cfg = &chip_cfg;
    init_config->p_datapath_cfg = datapath_cfg;
    init_config->p_qos_cfg = qos_cfg;
    init_config->p_ipuc_cfg = &ipuc_cfg;
    init_config->p_stats_cfg = &stats_cfg;
    init_config->p_stacking_cfg = stacking_cfg;
    init_config->p_parser_cfg = &parser_cfg;
    init_config->p_linkagg_cfg = &lag_cfg;
    init_config->p_overlay_cfg = &overlay_cfg;
    init_config->p_port_cfg = &port_cfg;
    init_config->init_flag |= CTC_INIT_MODULE_APS;
#ifdef OFPRODUCT    
    init_config->init_flag |= CTC_INIT_MODULE_MPLS;
#endif
    /* get config info */
    ret = ctc_sai_switch_get_sdk_config(init_config, arg);
    if (ret != 0)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "ctc_app_get_config failed:%s@%d \n",  __FUNCTION__, __LINE__);
        return ret;
    }
    
    ctc_sai_warmboot_init();

    /* init sdk */
    ret = ctc_sdk_init(init_config);
    if (ret != 0)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "ctc_sdk_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
        return ret;
    }

    if (0 != sal_task_create(&sai_sdk_cli_thread,
                             "sai sdk cli thread",
                             256*1024, SAL_TASK_PRIO_DEF, ctc_sai_switch_sdk_cli_thread, NULL))
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "sai create sdk cli thread failed:%s@%d \n",  __FUNCTION__, __LINE__);
        return ret;
    }

EXIT:
    #if 0
    if (ftm_key_info)
    {
        sal_free(ftm_key_info);
    }

    if (ftm_tbl_info)
    {
        sal_free(ftm_tbl_info);
    }

    if (mpls_cfg)
    {
        sal_free(mpls_cfg);
    }

    if(datapath_cfg)
    {
        sal_free(datapath_cfg);
    }
    if(dma_cfg)
    {
        sal_free(dma_cfg);
    }
    if(qos_cfg)
    {
        sal_free(qos_cfg);
    }

    if(stacking_cfg)
    {
        sal_free(stacking_cfg);
    }

    if (init_config)
    {
        sal_free(init_config);
    }
    #endif

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_switch_get_hw_spec(ctc_sai_sdk_init_info_t* p_sdk_info, _In_  char* profile_name)
{
    FILE *fp;
    uint32 val;
    char buf[256];
    uint8 tmp[256];
    uint8 tmp1[256];
    char cp_cmd[256];

    fp = sal_fopen(profile_name, "r");
    if(NULL == fp)
    {
        p_sdk_info->gchip[0] = 0;
        p_sdk_info->local_chip_num = 1;
        return SAI_STATUS_SUCCESS;
    }

    while(sal_fgets(buf, 256, fp) != NULL)
    {
        if(!sal_strncmp(buf, "#", 1))
        {
            continue;
        }
        else if(!sal_strncmp(buf, "gchip_0", 7))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_sdk_info->gchip[0] = (uint8)val;
        }
        else if(!sal_strncmp(buf, "local_chip_num", 14))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_sdk_info->local_chip_num = (uint8)val;
        }
        else if(!sal_strncmp(buf, "port_mapping_en", 15))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_sdk_info->port_mapping_en = (uint8)val;
        }
        else if(!sal_strncmp(buf, "stacking_en", 11))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_sdk_info->stacking_en = (uint8)val;
        }
        else if(!sal_strncmp(buf, "datapath", 8))
        {
            sal_sscanf(buf, "%s %s", tmp, tmp1);
            sal_memcpy(p_sdk_info->datapath_config_file, tmp1, sizeof(tmp1));
            /* modified by liul for gb osp, 2016-05-20 */
            sprintf(cp_cmd, "cp %s %s", p_sdk_info->datapath_config_file, CTC_ETC_BASE_DIR"/etc/datapath_profile/datapath_cfg.txt" );
            system(cp_cmd);
        }
        else if(!sal_strncmp(buf, "irq_type_exception", 8))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_sdk_info->irq_type_exception = (uint8)val;
        }
        else if(!sal_strncmp(buf, "irq1", 4))
        {
            sal_sscanf(buf, "%s %d", tmp, &val);
            p_sdk_info->irq1 = (uint8)val;
        }
        else if(!sal_strncmp(buf, "irq2", 4))
        {
            sal_sscanf(buf, "%s %d", tmp, &val);
            p_sdk_info->irq2 = (uint8)val;
        }
        /* Added by liuht to support disable ptp, 2017-04-14 */
        else if(!sal_strncmp(buf, "disable_ptp", 11))
        {
            p_sdk_info->disable_ptp = 1;
        }
        /* Added by tongzb to support configure synce, 2019-07-10 */
        else if(!sal_strncmp(buf, "disable_synce", 13))
        {
            p_sdk_info->disable_synce = 1;
        }        
     }

    sal_fclose(fp);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_switch_init_sdk(_In_ sai_switch_profile_id_t profile_id,
    _In_  char* profile_name,
    _In_ uint16 port_num,
    _In_ void* phy_map)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_sai_sdk_init_info_t sdk_init_info;

    sal_memset(&sdk_init_info, 0, sizeof(sdk_init_info));
    sdk_init_info.port_num = port_num;
    sdk_init_info.phy_map = (ctc_sai_sdk_phy_maping_t*)phy_map;
    CTC_SAI_ERROR_GOTO(ctc_sai_switch_get_hw_spec(&sdk_init_info, profile_name), ret, out);
    sdk_init_info.ftm_profile_type = profile_id;

    CTC_SAI_ERROR_GOTO(ctc_sai_switch_sdk_init((void*)&sdk_init_info), ret, out);
    
    return SAI_STATUS_SUCCESS;
out:
    return ret;
}

sai_status_t
ctc_sai_switch_init_global_storm_control()
{
    int32 ret = SAI_STATUS_SUCCESS;
    
    ctc_security_stmctl_glb_cfg_t glb_cfg;
    sal_memset(&glb_cfg, 0, sizeof(ctc_security_stmctl_glb_cfg_t));
    
    glb_cfg.ipg_en = FALSE;
    glb_cfg.ustorm_ctl_mode = TRUE;
    glb_cfg.mstorm_ctl_mode = FALSE;
    glb_cfg.granularity = CTC_SECURITY_STORM_CTL_GRANU_1000;
    
    ret = ctc_storm_ctl_set_global_cfg(&glb_cfg);
    return ret;
}

/*
* Routine Description:
*   SDK initialization. After the call the capability attributes should be
*   ready for retrieval via sai_get_switch_attribute().
*
* Arguments:
*   [in] profile_id - Handle for the switch profile.
*   [in] switch_hardware_id - Switch hardware ID to open
*   [in/opt] microcode_module_name - Vendor specific name of the microcode
*                                     module to load
*   [in] switch_notifications - switch notification table
* Return Values:
*   SAI_STATUS_SUCCESS on success
*   Failure status code on error
*/
sai_status_t ctc_sai_initialize_switch(
    _In_ sai_switch_profile_id_t profile_id,
    _In_reads_z_(SAI_MAX_HARDWARE_ID_LEN) char* switch_hardware_id,
    _In_reads_opt_z_(SAI_MAX_MICROCODE_NAME_LEN) char* microcode_module_name,
    _In_ sai_switch_notification_t* switch_notifications,
    _In_ uint16 port_num,
    _In_ void* phy_map)
{
    sai_status_t                ret                 = SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_info_t   *psai_attr_entry    = NULL;
    sai_attribute_t             attr;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_ERROR_GOTO(ctc_sai_switch_init_sdk(profile_id, microcode_module_name,port_num,phy_map),ret,out);


    if(switch_notifications)
    {
        sal_memcpy( &ctc_sai_get_sys_info()->sai_switch_notification_table,
                    switch_notifications,
                    sizeof(sai_switch_notification_t));
    }

    /* install ctc_shell */

    /*added by yejl to fix bug38960,2016-06-13*/
    CTC_SAI_ERROR_GOTO(ctc_sai_switch_init_global_storm_control(),ret,out);

    ret = api_reg_traverse(__init_traverse_api_reg,NULL);

    psai_attr_entry = &g_sai_attr_entries[0];
    while(psai_attr_entry->id != SAI_ATTR_ID_END)
    {
        if((psai_attr_entry->type & SAI_ATTR_FLAG_DEFAULT) == SAI_ATTR_FLAG_DEFAULT)
        {
            attr.id = psai_attr_entry->id;
            attr.value = psai_attr_entry->default_value;
            ctc_sai_set_switch_attribute(&attr);
        }
        psai_attr_entry++;
    }

out:
    return ret;
}

/*
* Routine Description:
*   Release all resources associated with currently opened switch
*
* Arguments:
*   [in] warm_restart_hint - hint that indicates controlled warm restart.
*                            Since warm restart can be caused by crash
*                            (therefore there are no guarantees for this call),
*                            this hint is really a performance optimization.
*
* Return Values:
*   None
*/
void ctc_sai_shutdown_switch(
    _In_ bool warm_restart_hint)
{
    CTC_SAI_DEBUG_FUNC();

    if(ctc_sai_get_sys_info()->sai_switch_notification_table.on_switch_shutdown_request)
    {
        ctc_sai_get_sys_info()->sai_switch_notification_table.on_switch_shutdown_request();
    }

    api_reg_traverse(__exit_traverse_api_reg,NULL);
}

void
_ctc_sai_switch_monitor_data_report_cb(ctc_monitor_data_t * info, void* userdata) 
{
    ctc_monitor_msg_t*  p_msg = NULL;
    ctc_monitor_buffer_event_t*  p_buffer = NULL;
    ctc_monitor_latency_event_t*  p_latency = NULL;
    uint32_t index = 0;

    sai_object_id_t port_oid = 0;
    sai_monitor_type_t type = 0;
    sai_monitor_event_type_t event_type = 0;
    sai_attribute_t attr_list[3];
    
    if (NULL == ctc_sai_get_sys_info()->sai_switch_notification_table.on_monitor_event)
    {
        return;
    }
    
    for (index = 0; index < info->msg_num; index++)
    {
        p_msg = &info->p_msg[index];
        if (CTC_MONITOR_BUFFER == p_msg->monitor_type)
        {
            type = SAI_MONITOR_TYPE_BUFFER;
            p_buffer = &p_msg->u.buffer_event;
            port_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_buffer->gport);
            if (1 == p_buffer->event_state)
            {
                event_type = SAI_MONITOR_EVENT_START;
            }
            else
            {
                event_type = SAI_MONITOR_EVENT_END;
            }
            attr_list[2].value.u32 = p_buffer->uc_cnt + p_buffer->mc_cnt;
        }
        else if (CTC_MONITOR_LATENCY == p_msg->monitor_type)
        {
            type = SAI_MONITOR_TYPE_LATENCY;
            p_latency = &p_msg->u.latency_event;
            port_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_latency->gport);
            if (1 == p_latency->event_state)
            {
                event_type = SAI_MONITOR_EVENT_START;
            }
            else
            {
                event_type = SAI_MONITOR_EVENT_END;
            }
            attr_list[2].value.u64 = p_latency->latency;
        }

        attr_list[0].value.u32 = type;
        attr_list[1].value.u32 = event_type;
        ctc_sai_get_sys_info()->sai_switch_notification_table.on_monitor_event(port_oid, 3, attr_list);
        
   }
   return;
}

sai_status_t
ctc_sai_switch_monitor_init()
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    ctc_monitor_config_t monitor_config;

    ctc_monitor_register_cb((ctc_monitor_fn_t)_ctc_sai_switch_monitor_data_report_cb, NULL);

    sal_memset(&monitor_config, 0, sizeof(monitor_config));
    monitor_config.cfg_type = CTC_MONITOR_CONFIG_MON_SCAN_EN;
    monitor_config.monitor_type = CTC_MONITOR_BUFFER;
    monitor_config.value = 0;
    ctc_monitor_set_config(&monitor_config);

    monitor_config.cfg_type = CTC_MONITOR_CONFIG_MON_SCAN_EN;
    monitor_config.monitor_type = CTC_MONITOR_LATENCY;
    monitor_config.value = 0;
    ctc_monitor_set_config(&monitor_config);
    
    return ret;
}

sai_status_t
ctc_sai_switch_global_parser_init()
{
    int32_t ret = CTC_E_NONE;
    ctc_parser_ecmp_hash_ctl_t  hash_ctl;

    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_ecmp_hash_ctl_t));

#if defined(GOLDENGATE) || defined(USW)
    ctc_parser_global_cfg_t     parser_cfg;
    uint32 i = 0;
    sal_memset(&parser_cfg, 0, sizeof(ctc_parser_global_cfg_t));
    for (i = 0; i < CTC_PARSER_TUNNEL_TYPE_MAX; i++)
    {
        parser_cfg.ecmp_tunnel_hash_mode[i] = CTC_PARSER_TUNNEL_HASH_MODE_MERGE;
        parser_cfg.linkagg_tunnel_hash_mode[i] = CTC_PARSER_TUNNEL_HASH_MODE_MERGE;
    }
    CTC_SAI_ERROR_GOTO(ctc_parser_set_global_cfg(&parser_cfg),ret,out);
#endif

    hash_ctl.hash_type_bitmap |= CTC_PARSER_ECMP_HASH_TYPE_FLAGS_MPLS;

    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_IP);
    CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPSA);
    CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_IPDA);
    CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_ECMP_HASH_FLAGS_PROTOCOL);

    CTC_SET_FLAG(hash_ctl.hash_type_bitmap, CTC_PARSER_ECMP_HASH_TYPE_FLAGS_L4);
    CTC_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_SRC_PORT);
    CTC_SET_FLAG(hash_ctl.l4_flag, CTC_PARSER_L4_ECMP_HASH_FLAGS_DST_PORT);
    
    CTC_SAI_ERROR_GOTO(ctc_parser_set_ecmp_hash_field(&hash_ctl),ret,out);

out:
    return ret;
}

#define ________SAI_SWITCH_INNER_FUNC
static sai_status_t
__init_mode_fn(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *private_data)
{
    preg->init_status = INITIALIZED;
    ctc_sai_switch_monitor_init();
#ifndef TAPPRODUCT
    ctc_sai_switch_global_parser_init();
#endif
    return SAI_STATUS_SUCCESS;
}

static sai_status_t
__exit_mode_fn(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_switch_api_t      g_sai_switch_api_func = {
    .initialize_switch      = ctc_sai_initialize_switch,
    .shutdown_switch        = ctc_sai_shutdown_switch,
    .connect_switch         = NULL,
    .disconnect_switch      = NULL,
    .set_switch_attribute   = ctc_sai_set_switch_attribute,
    .get_switch_attribute   = ctc_sai_get_switch_attribute,
};

static ctc_sai_api_reg_info_t g_switch_api_reg_info = {
    .id         = SAI_API_SWITCH,
    .init_func  = __init_mode_fn,
    .exit_func  = __exit_mode_fn,
    .api_method_table = &g_sai_switch_api_func,
    .private_data     = NULL,
    .init_status      = INITIALIZED,
};


#define ________SAI_SWITCH_OUTER_FUNC
sai_status_t
ctc_sai_switch_init()
{
    api_reg_register_fn(&g_switch_api_reg_info);
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_switch_set_monitor_buffer_enable(
    _In_  const sai_attribute_t *attr)
{
    ctc_monitor_config_t monitor_config;
    int32_t             sdk_ret     = CTC_E_NONE;
 
    sal_memset(&monitor_config, 0, sizeof(monitor_config));		  
    monitor_config.value = attr->value.u32;
    monitor_config.cfg_type = CTC_MONITOR_CONFIG_MON_INFORM_EN;	
    monitor_config.monitor_type = CTC_MONITOR_BUFFER;
    sdk_ret = ctc_monitor_set_config(&monitor_config);

    return ctc_sai_get_error_from_sdk_error(sdk_ret);	
}

sai_status_t
ctc_sai_switch_set_monitor_buffer_threshold(
    _In_  const sai_attribute_t *attr)
{
    ctc_monitor_config_t monitor_config;
    int32_t             sdk_ret     = CTC_E_NONE;

    sal_memset(&monitor_config, 0, sizeof(monitor_config));	
    monitor_config.value = attr->value.u32list.list[0];
    monitor_config.monitor_type = CTC_MONITOR_BUFFER;
    monitor_config.cfg_type = CTC_MONITOR_CONFIG_MON_INFORM_MIN;
    sdk_ret = ctc_monitor_set_config(&monitor_config);
    if (sdk_ret != CTC_E_NONE)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);	 
    }

    monitor_config.value = attr->value.u32list.list[1];
    monitor_config.monitor_type = CTC_MONITOR_BUFFER;
    monitor_config.cfg_type = CTC_MONITOR_CONFIG_MON_INFORM_MAX;
    sdk_ret = ctc_monitor_set_config(&monitor_config);
    if (sdk_ret != CTC_E_NONE)
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);	 
    }
	
    return CTC_E_NONE;	 
}

sai_status_t
ctc_sai_switch_set_monitor_latency_range(
    _In_  const sai_attribute_t *attr)
{
    ctc_monitor_glb_cfg_t monitor_glb_cfg;
    int32_t             sdk_ret     = CTC_E_NONE;
    uint32 i = 0;

    sal_memset(&monitor_glb_cfg, 0, sizeof(monitor_glb_cfg));	
    monitor_glb_cfg.cfg_type = CTC_MONITOR_GLB_CONFIG_LATENCY_THRD;

    for (i=0; i< 8; i++)
    {
        monitor_glb_cfg.u.thrd.level = i;
        monitor_glb_cfg.u.thrd.threshold = attr->value.u32list.list[i];
        sdk_ret = ctc_monitor_set_global_config(&monitor_glb_cfg);
        if (sdk_ret != CTC_E_NONE)
        {
            return ctc_sai_get_error_from_sdk_error(sdk_ret);	 
        }
    }
    
    return CTC_E_NONE;	 
}

