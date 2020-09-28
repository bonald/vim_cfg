#ifndef __HSRV_MSG_OPENFLOW_H__
#define __HSRV_MSG_OPENFLOW_H__

#include "glb_openflow_define.h"

enum fei_openflow_stats_res_type_s
{
    FEI_OPENFLOW_STATS_RES_TYPE_FLOW = 0,   /**< flow stats */
    FEI_OPENFLOW_STATS_RES_TYPE_METER,      /**< meter stats */
    FEI_OPENFLOW_STATS_RES_TYPE_GROUP,      /**< group stats */
    FEI_OPENFLOW_STATS_RES_TYPE_MAX,        /**< max stats, not used */
};
typedef enum fei_openflow_stats_res_type_s fei_openflow_stats_res_type_t;

enum fei_openflow_group_partial_type_e
{
    FEI_OPENFLOW_GROUP_PARTIAL_TYPE_HEAD = 0,
    FEI_OPENFLOW_GROUP_PARTIAL_TYPE_BUCKET,
    FEI_OPENFLOW_GROUP_PARTIAL_TYPE_TAIL,
    FEI_OPENFLOW_GROUP_PARTIAL_TYPE_MAX,
};
typedef enum fei_openflow_group_partial_type_e fei_openflow_group_partial_type_t;

typedef struct
{
    uint32 reserved; /*No parameter currently. */
} fei_openflow_post_sys_startup_t;

typedef struct
{
    uint32 reserved; /*No parameter currently. */
} fei_openflow_do_barrier_t;

typedef struct
{
    uint32 ifindex; /*No parameter currently. */
} fei_openflow_port_do_barrier_t;

typedef struct
{
    uint32 ifindex;
    uint32 link_up;
    bool   enable;
} fei_openflow_port_enable_t;

typedef struct
{
    uint32 ifindex;
    bool   enable;
    bool    vlan_filter;
    bool    ingress_add_native_vlan;
    bool    tunnel_mpls_disable;
    glb_openflow_tunnel_port_type_t type;
} fei_openflow_port_operation_agg_t;

typedef struct
{
    uint32 ifindex;
    uint32 vlan_id;
    bool   enable;
} fei_openflow_port_set_protected_vlan_t;

/* Added by weizj for tunnel bug 33724 */
/* Modifed by weizj for bug 35135 */
typedef struct
{
    uint8 is_group;
    uint8 is_drop;
} fei_openflow_set_drop_ingress;

/* Added by weizj for udf */
typedef struct
{
    uint8 enable;
    glb_openflow_udf_parser_t parser;
} fei_openflow_set_udf_parser;

typedef struct
{
    uint8 enable;
} fei_openflow_set_e2e_parser;

typedef struct
{
    int32  stpid;
} fei_openflow_set_stpid;

typedef struct
{
    int32  ctpid;
} fei_openflow_set_ctpid;

typedef struct
{
    uint32 bond_ifindex;
    bool   enable;
} fei_openflow_bond_create_t;

typedef struct
{
    uint32 bond_ifindex;
    uint32 slave_ifindex;
    bool   enable;
} fei_openflow_bond_update_slave_t;

typedef struct
{
    uint32 bond_ifindex;
    uint32 slave_ifindex;
} fei_openflow_bond_attach_slave_t;

typedef struct
{
    uint32 ifindex;
    bool   ignore_vlan_filter;
} fei_openflow_port_ignore_vlan_filter_t;

typedef struct
{
    uint32 ifindex;
    bool   tunnel_mpls_disable;
} fei_openflow_port_tunnel_mpls_disable_t;

typedef struct
{
    uint32 ifindex;
    uint32 ingress_add_native_vlan;
    uint32 egress_remove_native_vlan;
} fei_openflow_port_native_vlan_config_t;

typedef struct
{
    uint32  ifindex;
    uint32  action[128];
} fei_openflow_port_pkt_action_t;

typedef struct
{
    uint32 ifindex;
    uint32 enable;
} fei_openflow_port_notify_link_t;

typedef struct
{
    glb_openflow_flow_t flow;
} fei_openflow_add_flow_req_t;

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-19 */
typedef struct
{
    glb_openflow_tpoam_t flow;
} fei_openflow_add_tpoam_req_t;

typedef struct
{
    glb_openflow_tpoam_t flow;
} fei_openflow_del_tpoam_req_t;

typedef struct
{
    glb_openflow_tpoam_t flow;
} fei_openflow_update_tpoam_req_t;
/* liwh end */

typedef struct
{
    glb_openflow_packet_out_param_t packet_out;
} fei_openflow_packet_out_req_t;

typedef struct
{
    glb_openflow_flow_t flow;
} fei_openflow_mod_flow_req_t;

typedef struct
{
    uint32 flow_id;
} fei_openflow_del_flow_req_t;

typedef struct
{
    uint32 resvd;
} fei_openflow_reset_flow_req_t;

typedef struct
{
    fei_openflow_stats_res_type_t res_type;
    uint32 id;
} fei_openflow_get_stats_req_t;

typedef struct
{
    fei_openflow_stats_res_type_t res_type;
    uint32 id;
} fei_openflow_clear_stats_req_t;

typedef struct
{
    fei_openflow_group_partial_type_t partial_type;
    union {
        glb_openflow_partial_group_head_t head;
        glb_openflow_partial_group_bucket_t bucket;
        glb_openflow_partial_group_tail_t tail;
    } u;
} fei_openflow_add_group_req_t;

typedef struct
{
    fei_openflow_group_partial_type_t partial_type;
    union {
        glb_openflow_partial_group_head_t head;
        glb_openflow_partial_group_bucket_t bucket;
        glb_openflow_partial_group_tail_t tail;
    } u;
} fei_openflow_mod_group_req_t;

typedef struct
{
    uint32 group_id;
} fei_openflow_del_group_req_t;

/* Added by weizj for meter */
typedef struct
{
    glb_openflow_meter_t meter;
} fei_openflow_add_meter_req_t;

/* Added by weizj for meter */
typedef struct
{
    glb_openflow_queue_t queue;
} fei_openflow_add_queue_shape_req_t;

typedef struct
{
    glb_openflow_meter_t meter;
    uint8 old_stats_enable;
} fei_openflow_mod_meter_req_t;

typedef struct
{
    uint32 meter_id;
    uint8  stats_enable;
} fei_openflow_del_meter_req_t;

typedef struct
{
    uint8 enable;
} fei_openflow_mod_meter_ipg_req_t;
/* End by weizj for meter */

/* Added by weizj for inband */
typedef struct
{
    glb_openflow_inband_t config;
} fei_openflow_inband_set_config_req_t;

typedef struct
{
    char        name[HAL_NAME_LEN];
    uint32      ifindex;
    uint16      gport;
} fei_openflow_inband_vif_req_t;
/* End by weizj for inband */

typedef struct
{
    uint32 hash_key;
} fei_openflow_parser_set_ecmp_hash_key_req_t;
typedef struct
{
    uint32 hash_use;
    uint32 hash_key;
} fei_openflow_parser_set_linkagg_hash_key_req_t;

typedef struct
{
    uint8 enable;
} fei_openflow_parser_set_mpls_parser_req_t;

/* Added by weizj for tunnel */
typedef struct
{
    glb_openflow_tunnel_port_type_t type;
    uint32 ifindex;       /* bind port */
    uint32 enable;
} fei_openflow_port_decap_req_t;

typedef struct
{
    uint8 decap_mode;
    
} fei_openflow_tunnel_decap_mode_req_t;

typedef struct
{
    glb_openflow_tunnel_info_t tunnel_info;
} fei_openflow_add_tunnel_req_t;

typedef struct
{
    glb_openflow_tunnel_del_type_t tunnel_del_type; /* added by hansf for distinguish ovs del or running down del */
    uint32 ifindex;
} fei_openflow_del_tunnel_req_t;

/* Added by weizj for tunnel bug 33958 */
typedef struct
{
    uint32 ip_addr;
} fei_openflow_vtep_ip_req_t;
/* End by weizj for tunnel */

typedef struct
{
    uint32 ifindex;
    uint32 port_config;
} fei_openflow_port_set_config_t;

typedef struct
{
    uint32 ifindex;
    uint32 flag;
    bool   enable;
} fei_openflow_port_add_to_bridge_req_t;

typedef struct
{
    uint32 ifindex;
    bool   enable;
} fei_openflow_update_all_action_list_req_t;

typedef struct
{
    uint32 stub_ifindex;
    bool   enable;
} fei_openflow_stub_create_t;

typedef struct
{
    uint32 stub_ifindex;
    bool   enable;
} fei_openflow_stub_enable_t;

typedef struct
{
    uint32 bond_ifindex;
    uint8 mac[6];
} fei_openflow_bond_set_mac_t;

int32
hsrv_openflow_register_cb(void);

#endif /* !__HSRV_MSG_OPENFLOW_H__ */
