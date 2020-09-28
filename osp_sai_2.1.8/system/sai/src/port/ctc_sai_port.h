#if !defined (__SAI_PORT_H_)
#define __SAI_PORT_H_

#include <saiport.h>
#include <ctc_port.h>
#include <ctc_stats.h>
#include <sal.h>
#include "ctc_sai_common.h"

#define CTC_SAI_PORT_SPEED_10M      10
#define CTC_SAI_PORT_SPEED_100M     100
#define CTC_SAI_PORT_SPEED_1000M    1000
#define CTC_SAI_PORT_SPEED_2G5      2500
#define CTC_SAI_PORT_SPEED_10G      10000
#define CTC_SAI_PORT_SPEED_40G      40000
#define CTC_SAI_PORT_SPEED_100G     100000

#define  SAI_PORT_PFC_DROP_THRD   512    
#define  SAI_PORT_PFC_XOFF_THRD   192    
#define  SAI_PORT_PFC_XON_THRD   128    

#define CTC_SAI_PORT_GROUP_LAG_NUM      64
#define CTC_SAI_PORT_GROUP_IGS_ID_BASE  129
#define CTC_SAI_PORT_GROUP_EGS_ID_BASE  257
#define CTC_SAI_PORT_CLASS_ID_BASE      65
#define CTC_SAI_PORT_CLASS_ID_MAX       250 /*reserve 255-251 for iloop*/

/* added by hansf for ttp acl group */
#define CTC_SAI_PORT_TTP_GROUP_IGS_ID_BASE  385
#define CTC_SAI_PORT_TTP_GROUP_IGS_ID_MAX  128

#define CTC_SAI_PORT_GROUP_IGS_ID_MAX  128
#define CTC_SAI_PORT_GROUP_EGS_ID_MAX  128

#define CTC_SAI_PORT_TYPE_ACCESS  1
#define CTC_SAI_PORT_TYPE_TRUNK   0


typedef uint32  ctc_sai_port_flag_t; 

#define SAI_PORT_FLAG_RUNNING                (1 << 0)
#define SAI_PORT_FLAG_ROUTED_EN              (1 << 1)
#define SAI_PORT_FLAG_ALLOW_TAGGED           (1 << 2)
#define SAI_PORT_FLAG_ALLOW_UNTAGGED         (1 << 3)
#define SAI_PORT_FLAG_MIRROR_DEST            (1 << 4)
#define SAI_PORT_FLAG_AGGREGATED             (1 << 5)
#define SAI_PORT_FLAG_AGG_OPRED              (1 << 6)
#define SAI_PORT_FLAG_DIS_DISTRIBUTOR        (1 << 8)
#define SAI_PORT_FLAG_DIS_COLLECTOR          (1 << 9)
#define SAI_PORT_FLAG_DOT1X_UNAUTHED         (1 << 10)
#define SAI_PORT_FLAG_DOT1X_DIR_IN           (1 << 11)
#define SAI_PORT_FLAG_DOT1X_DIR_OUT          (1 << 12)
#define SAI_PORT_FLAG_EFM_RX_DISCARD         (1 << 14)
#define SAI_PORT_FALG_L2VPN_PORT_MODE        (1 << 15)
#define SAI_PORT_FALG_L2VPN_VLAN_MODE        (1 << 16)
#define SAI_PORT_FALG_L2VPN_ENABLE           (1<<17)
#define SAI_PORT_FLAG_VLAN_TRANSLATION_ENABLE    (1 << 18) 
#define SAI_PORT_FLAG_VLAN_MAPPING_QINQ_ENABLE   (1 << 19) 
#define SAI_PORT_FLAG_VLAN_MAPPING_QINQ_SELECTIVE     (1 << 20) 
#define SAI_PORT_FLAG_VLAN_CLASS_ENABLE     (1 << 21)
#define SAI_PORT_FLAG_INGRESS_FILTER_EN     (1 << 22)
#define SAI_PORT_FLAG_EGRESS_FILTER_EN     (1 << 23)
#define SAI_PORT_FLAG_EFM_PAR_ACTION_LB    (1 << 24)
#define SAI_PORT_FLAG_EFM_PAR_ACTION_DISCARD    (1 << 25)
#define SAI_PORT_FLAG_EFM_MUX_ACTION_DISCARD    (1 << 26)
#define SAI_PORT_FLAG_EFM_LINK_MONITOR    (1 << 27)
#define SAI_PORT_FLAG_PHY_LPBK_EXTERNAL   (1 << 28)
#define SAI_PORT_FLAG_HYBRID_VLAN_FILTER   (1 << 29)
#define SAI_PORT_FLAG_HYBRID_OPENFLOW_ENABLE   (1 << 30)
#define SAI_PORT_FLAG_AGG_IS_ACTIVE              (1 << 31)

typedef struct ctc_sai_port_filed_info_s
{
    ctc_vlantag_ctl_t tag_ctrl;
    ctc_dot1q_type_t  dot1q_type;
    bool              transmit_en;
    bool              receive_en;
    bool              keep_vlan_tag;
    bool              ingress_vlan_filter;
    bool              egress_vlan_filter;
    bool              src_discard;
    bool              src_outer_is_svlan;
    bool              lag_bind_en;
}ctc_sai_port_filed_info_t;

typedef struct ctc_sai_acl_en_port_node_s
{
    sai_object_id_t             port_oid;
    sai_uint32_t                dir;
    sai_uint32_t                entries_cnt;
} ctc_sai_acl_en_port_node_t;

typedef struct ctc_sai_acl_get_group_id_by_class_id_node_s
{
    sai_uint32_t                class_id;
    sai_uint32_t                dir;
    sai_uint32_t                group_id;
} ctc_sai_acl_get_group_id_by_class_id_node_t;

typedef sai_status_t (*port_object_cb)(sai_object_id_t, void*);

int32_t
ctc_sai_port_set_port_flag (sai_object_id_t port_id, ctc_sai_port_flag_t flag, bool enable);

int32_t
ctc_sai_port_set_port_sflow(sai_object_id_t port_id, bool dir, uint16 threshold, bool enable);

int32_t
ctc_sai_port_set_phy_en (sai_object_id_t port_id, bool enable);

int32_t
ctc_sai_port_get_port_flag(sai_object_id_t port_id, ctc_sai_port_flag_t* p_flag);

int32_t
ctc_sai_port_set_erps_pdu_action (sai_object_id_t port_id, uint8 action);

int32_t
ctc_sai_port_set_dot1x_pdu_action (sai_object_id_t port_id, uint8 action);

sai_status_t
ctc_sai_for_each_port_object(port_object_cb fn, void* pdata);

sai_status_t ctc_sai_clear_port_all_stats(_In_ sai_object_id_t port_id);

int32_t
ctc_sai_port_get_lag_id(
    _In_  uint32 port_id);

sai_status_t
ctc_sai_port_bind_lag_id(
	_In_  sai_object_id_t ,
	_In_  sai_object_id_t );

sai_status_t
ctc_sai_get_port_attribute(
	_In_ sai_object_id_t ,
	_In_ uint32_t ,
	_Inout_ sai_attribute_t *);

sai_status_t
ctc_sai_set_port_attribute(
	_In_ sai_object_id_t ,
	_In_ const sai_attribute_t *);

sai_status_t
ctc_sai_port_set_default(
	_In_  sai_object_id_t 	);

sai_status_t
ctc_sai_port_set_all_default();

sai_status_t
ctc_sai_port_set_vlanclass_port_flag(uint16 gport, uint8 type_is_access, bool enable);

sai_status_t
ctc_sai_port_objectid_to_gport( sai_object_id_t port_oid, uint32 *pgport);

sai_status_t
ctc_sai_port_gport_to_objectid(uint32 gport, sai_object_id_t *pst_port_oid);

sai_status_t
ctc_sai_port_objectid_to_classid(sai_object_id_t port_oid, uint16 *pclassid);

int32_t
ctc_sai_acl_get_group_id_by_class_id(sai_uint32_t class_id, sai_uint32_t dirction);

uint32
ctc_sai_port_make_acl_group_id(uint32 dir, uint32 gport);

uint32
ctc_sai_port_get_ttp_acl_group_id(uint32 dir, uint32 gport);

int32
ctc_sai_port_set_mac(uint32 gport, mac_addr_t mac_addr);

#ifdef GREATBELT
int32
ctc_sai_port_set_oam_mac(uint32 gport, const sai_attribute_t *attr);
#endif

stbl_port_t*
ctc_sai_port_entry_get_by_gport(uint32 gport);

stbl_port_t*
ctc_sai_port_entry_get_by_portid(sai_object_id_t port_oid);

stbl_port_t*
ctc_sai_port_entry_db_alloc(sai_object_id_t port_oid);

int32
ctc_sai_port_entry_db_free(sai_object_id_t port_oid);

sai_status_t
ctc_sai_port_init();

stbl_port_t*
ctc_sai_port_get_port(
    _In_  sai_object_id_t port_id);

int32
ctc_sai_port_recover_vlan_ctl(uint32 gport);

int32
ctc_sai_port_openflow_set_vlan_filter(uint32 gport, bool enable);

int32
ctc_sai_port_openflow_set_openflow_enable(uint32 gport, bool enable);

int32_t
ctc_sai_port_alloc_timestamp (sai_object_id_t port_id, uint32_t *ptimestamp_nhid);

int32_t
ctc_sai_port_free_timestamp (sai_object_id_t port_id);

int32_t
ctc_sai_port_get_metadata (sai_object_id_t port_id);
#endif
