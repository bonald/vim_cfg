/**
 @file sys_usw_port.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-11-9

 @version v2.0

*/

#ifndef _SYS_USW_PORT_H
#define _SYS_USW_PORT_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"

#include "ctc_const.h"
#include "ctc_vlan.h"
#include "ctc_port.h"
#include "ctc_security.h"
#include "ctc_interrupt.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
  @brief   Define port property flags
*/
enum sys_port_internal_property_e
{
    SYS_PORT_PROP_DEFAULT_PCP = 0,
    SYS_PORT_PROP_DEFAULT_DEI = 1,
    SYS_PORT_PROP_OAM_TUNNEL_EN = 2,
    SYS_PORT_PROP_OAM_VALID = 3,
    SYS_PORT_PROP_DISCARD_NONE_8023OAM_EN = 5,
    SYS_PORT_PROP_REPLACE_TAG_EN = 6,
    SYS_PORT_PROP_USE_INNER_COS = 7,
    SYS_PORT_PROP_EXCEPTION_EN = 8,
    SYS_PORT_PROP_EXCEPTION_DISCARD = 9,
    SYS_PORT_PROP_SECURITY_EXCP_EN = 10,
    SYS_PORT_PROP_SECURITY_EN = 11,
    SYS_PORT_PROP_MAC_SECURITY_DISCARD = 12,
    SYS_PORT_PROP_QOS_POLICY,
    SYS_PORT_PROP_REPLACE_DSCP_EN,
    SYS_PORT_PROP_STMCTL_EN,
    SYS_PORT_PROP_STMCTL_OFFSET,
    SYS_PORT_PROP_LINK_OAM_EN,
    SYS_PORT_PROP_LINK_OAM_MEP_INDEX,
    SYS_PORT_PROP_SERVICE_POLICER_VALID,
    SYS_PORT_PROP_SPEED_MODE,
    SYS_PORT_PROP_DOT1AE_TX_SCI_EN,
    SYS_PORT_PROP_MAC_AUTH_EN,
    SYS_PORT_PROP_NUM
};
typedef enum sys_port_internal_property_e  sys_port_internal_property_t;

enum sys_port_internal_direction_property_e
{
    SYS_PORT_DIR_PROP_L2_SPAN_EN,
    SYS_PORT_DIR_PROP_L2_SPAN_ID,
    SYS_PORT_DIR_PROP_ETHER_OAM_EDGE_PORT,
    SYS_PORT_DIR_PROP_LINK_LM_TYPE,
    SYS_PORT_DIR_PROP_LINK_LM_COS,
    SYS_PORT_DIR_PROP_LINK_LM_COS_TYPE,
    SYS_PORT_DIR_PROP_LINK_LM_INDEX_BASE,
    SYS_PORT_DIR_PROP_ETHER_LM_VALID,
    SYS_PORT_DIR_PROP_MPLS_SECTION_LM_EN,
    SYS_PORT_DIR_PROP_DOT1AE_SA_IDX_BASE,
    SYS_PORT_DIR_PROP_NUM
};
typedef enum sys_port_internal_direction_property_e  sys_port_internal_direction_property_t;


enum sys_port_vlan_filter_mode_e
{
    SYS_PORT_VLAN_FILTER_MODE_VLAN,
    SYS_PORT_VLAN_FILTER_MODE_ENABLE,
    SYS_PORT_VLAN_FILTER_MODE_DISABLE,
    SYS_PORT_VLAN_FILTER_MODE_NUM
};
typedef enum sys_port_vlan_filter_mode_e sys_port_vlan_filter_mode_t;


#define SYS_USW_ISOLATION_GROUP_NUM            32
#define SYS_USW_ISOLATION_ID_NUM               64


#define SYS_INVALID_LOCAL_PHY_PORT                    0xFFFF

struct sys_igs_port_prop_s
{
    uint32 lbk_en             :1;
    uint32 subif_en           :1;
    uint32 link_status        :1;
    uint32 rsv        :12;
    uint32 rsv0               :17;

    uint32 inter_lport        :16;
    uint32 global_src_port    :16;

    uint16 flag;
    uint16 flag_ext;

    uint8 isolation_id;

    uint32 nhid;
};
typedef struct sys_igs_port_prop_s sys_igs_port_prop_t;

struct sys_egs_port_prop_s
{
    uint16 flag;
    uint16 flag_ext;

    uint8 isolation_id;
    uint8 isolation_ref_cnt;
};
typedef struct sys_egs_port_prop_s sys_egs_port_prop_t;

enum sys_port_scl_action_type_e
{
    SYS_PORT_SCL_ACTION_TYPE_SCL,
    SYS_PORT_SCL_ACTION_TYPE_FLOW,
    SYS_PORT_SCL_ACTION_TYPE_TUNNEL,
    SYS_PORT_SCL_ACTION_TYPE_MAX
};
typedef enum sys_port_scl_action_type_e sys_port_scl_action_type_t;

typedef int32 (*SYS_PORT_GET_RCHIP_GPORT_IDX_CB)(uint8 lchip, uint32 gport, uint16* p_glb_port_index);

struct sys_port_master_s
{
    sys_igs_port_prop_t* igs_port_prop;
    sys_egs_port_prop_t* egs_port_prop;
    void*                 phy_driver;
    uint16*               network_port; /*for wlan store network port with internal port*/
    sal_mutex_t*          p_port_mutex;

    uint8   cur_log_index;
    uint8   use_logic_port_check;
    uint8   use_isolation_id; /*use_isolation_id == 0 means configure basing port port isolation;
                                           use_isolation_id == 1 means configure basing group port isolation*/
    uint8   isolation_group_mode;
    uint32  mac_bmp[4]; /* used for TAP, record tap dest, 0~1 for slice0, 2~3 for slice1 */
    uint32  igs_isloation_bitmap[2];
    SYS_PORT_GET_RCHIP_GPORT_IDX_CB rchip_gport_idx_cb; /*this callback function is used for get global dest port index*/
    uint8   opf_type_port_sflow;
    uint8   rsv[3];
};
typedef struct sys_port_master_s sys_port_master_t;

extern sys_port_master_t* p_usw_port_master[CTC_MAX_LOCAL_CHIP_NUM];
#define PORT_LOCK \
    if (p_usw_port_master[lchip]->p_port_mutex) sal_mutex_lock(p_usw_port_master[lchip]->p_port_mutex)
#define PORT_UNLOCK \
    if (p_usw_port_master[lchip]->p_port_mutex) sal_mutex_unlock(p_usw_port_master[lchip]->p_port_mutex)

#define SYS_PORT_INIT_CHECK()            \
    do                                   \
    {                                    \
        LCHIP_CHECK(lchip); \
        if (NULL == p_usw_port_master[lchip]){      \
            return CTC_E_NOT_INIT; }     \
    }                                    \
    while (0)

#define CTC_ERROR_RETURN_WITH_PORT_UNLOCK(op) \
    do { \
        int32 rv; \
        if ((rv = (op)) < 0) \
        { \
            sal_mutex_unlock(p_usw_port_master[lchip]->p_port_mutex); \
            return (rv); \
        } \
    } while (0)
#define SYS_PORT_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(port, port, PORT_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)
/****************************************************************************
 *
* Function
*
*****************************************************************************/
extern int32
sys_usw_port_init(uint8 lchip, ctc_port_global_cfg_t* p_port_global_cfg);

extern int32
sys_usw_port_deinit(uint8 lchip);

extern int32
sys_usw_port_set_default_cfg(uint8 lchip, uint32 gport);

/*external property*/
extern int32
sys_usw_port_set_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value);

extern int32
sys_usw_port_get_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32* p_value);

extern int32
sys_usw_port_set_restriction(uint8 lchip, uint32 gport, ctc_port_restriction_t* p_restriction);

extern int32
sys_usw_port_get_restriction(uint8 lchip, uint32 gport, ctc_port_restriction_t* p_restriction);

extern int32
sys_usw_port_set_isolation_mode(uint8 lchip, uint8 choice_mode);

extern int32
sys_usw_port_get_isolation_mode(uint8 lchip, uint8* p_choice_mode);

extern int32
sys_usw_port_set_isolation(uint8 lchip, ctc_port_isolation_t* p_restriction);

extern int32
sys_usw_port_get_isolation(uint8 lchip, ctc_port_isolation_t* p_restriction);

extern int32
sys_usw_port_set_direction_property(uint8 lchip, uint32 gport, ctc_port_direction_property_t port_prop, ctc_direction_t dir, uint32 value);

extern int32
sys_usw_port_get_direction_property(uint8 lchip, uint32 gport, ctc_port_direction_property_t port_prop, ctc_direction_t dir, uint32* p_value);
/*internal property*/

extern int32
sys_usw_port_set_internal_property(uint8 lchip, uint32 gport, sys_port_internal_property_t port_prop, uint32 value);

extern int32
sys_usw_port_get_internal_property(uint8 lchip, uint32 gport, sys_port_internal_property_t port_prop, uint32* p_value);

extern int32
sys_usw_port_set_internal_direction_property(uint8 lchip, uint32 gport, sys_port_internal_direction_property_t port_prop, ctc_direction_t dir, uint32 value);

extern int32
sys_usw_port_get_internal_direction_property(uint8 lchip, uint32 gport, sys_port_internal_direction_property_t port_prop, ctc_direction_t dir, uint32* p_value);

extern int32
sys_usw_port_update_mc_linkagg(uint8 lchip, uint8 tid, uint16 lport, bool is_add);

extern int32
sys_usw_port_set_global_port(uint8 lchip, uint16 lport, uint32 gl_port, bool update_mc_linkagg);

extern int32
sys_usw_port_get_global_port(uint8 lchip, uint16 lport, uint16* p_gl_port);

extern int32
sys_usw_port_set_phy_if_en(uint8 lchip, uint32 gport, bool enable);

extern int32
sys_usw_port_get_phy_if_en(uint8 lchip, uint32 gport, uint16* p_l3if_id, bool* p_enable);

extern int32
sys_usw_port_set_sub_if_en(uint8 lchip, uint32 gport, bool enable);

extern int32
sys_usw_port_get_sub_if_en(uint8 lchip, uint32 gport, bool* p_enable);

extern int32
sys_usw_port_set_scl_key_type(uint8 lchip, uint32 gport, ctc_direction_t direction, uint8 sal_id, ctc_port_scl_key_type_t type);

extern int32
sys_usw_port_get_scl_key_type(uint8 lchip, uint32 gport, ctc_direction_t direction, uint8 sal_id, ctc_port_scl_key_type_t* p_type);

extern int32
sys_usw_port_set_vlan_range(uint8 lchip, uint32 gport, ctc_vlan_range_info_t* p_vrange_info, bool enable);

extern int32
sys_usw_port_get_vlan_range(uint8 lchip, uint32 gport, ctc_vlan_range_info_t* p_vrange_info, bool* p_enable);

extern int32
sys_usw_port_set_loopback(uint8 lchip, ctc_port_lbk_param_t* p_port_lbk);

extern int32
sys_usw_set_ipg_size(uint8 lchip, ctc_ipg_size_t index, uint8 size);

extern int32
sys_usw_get_ipg_size(uint8 lchip, ctc_ipg_size_t index, uint8* p_size);

extern int32
sys_usw_port_set_port_mac_prefix(uint8 lchip, ctc_port_mac_prefix_t* p_port_mac);

extern int32
sys_usw_port_set_port_mac_postfix(uint8 lchip, uint32 gport, ctc_port_mac_postfix_t* p_port_mac);

extern int32
sys_usw_port_get_port_mac(uint8 lchip, uint32 gport, mac_addr_t* p_port_mac);

extern int32
sys_usw_port_set_reflective_bridge_en(uint8 lchip, uint32 gport, bool enable);

extern int32
sys_usw_port_get_reflective_bridge_en(uint8 lchip, uint32 gport, bool* p_enable);

extern int32
sys_usw_port_set_port_cross_connect(uint8 lchip, uint32 gport, uint32 nhid);

extern int32
sys_usw_port_get_port_cross_connect(uint8 lchip, uint32 gport, uint32* p_value);

extern int32
sys_usw_port_set_scl_property(uint8 lchip, uint32 gport, ctc_port_scl_property_t* prop);

extern int32
sys_usw_port_get_scl_property(uint8 lchip, uint32 gport, ctc_port_scl_property_t* prop);

/* ipg is mac tx property. value: 3 is 12Bytes, 2 is 8Bytes, 1 is 4Bytes, default is 12Bytes */
extern int32
sys_usw_port_mac_set_ipg(uint8 lchip, uint32 gport, uint32 value);

extern int32
sys_usw_port_set_acl_property(uint8 lchip,uint32 gport, ctc_acl_property_t* p_prop);

extern int32
sys_usw_port_get_acl_property(uint8 lchip,uint32 gport, ctc_acl_property_t* p_prop);

extern int32
sys_usw_port_set_bpe_property(uint8 lchip, uint32 gport, ctc_port_bpe_property_t* p_prop);

extern int32
sys_usw_port_get_bpe_property(uint8 lchip, uint32 gport, ctc_port_bpe_property_t* p_prop);

extern int32
sys_usw_port_get_local_phy_port(uint8 lchip, uint32 gport, uint32* p_value);

extern int32
sys_usw_port_register_rchip_get_gport_idx_cb(uint8 lchip, SYS_PORT_GET_RCHIP_GPORT_IDX_CB cb);

extern int32
sys_usw_port_glb_dest_port_init(uint8 lchip, uint16 glb_port_index);

extern int32
sys_usw_port_set_mac_auth(uint8 lchip, uint32 gport, bool enable);

extern int32
sys_usw_port_get_mac_auth(uint8 lchip, uint32 gport, bool* enable);

extern int32
sys_usw_port_get_capability(uint8 lchip, uint32 gport, ctc_port_capability_type_t type, void* p_value);

extern int32
sys_usw_port_wb_sync(uint8 lchip);

extern int32
sys_usw_port_wb_restore(uint8 lchip);

extern int32
sys_usw_port_set_station_move(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value);

extern int32
sys_usw_port_lport_convert(uint8 lchip, uint16 internal_lport, uint16* p_value);

#ifdef __cplusplus
}
#endif

#endif

