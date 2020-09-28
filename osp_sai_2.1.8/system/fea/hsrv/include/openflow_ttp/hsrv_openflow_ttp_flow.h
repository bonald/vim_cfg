#ifndef __HSRV_OPENFLOW_FLOW_H__
#define __HSRV_OPENFLOW_FLOW_H__

/* hsrv openflow memory flow related macros */
#define HSRV_OPENFLOW_FLOW_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_FLOW, size)
#define HSRV_OPENFLOW_FLOW_CALLOC(size) HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW_FLOW, size)
#define HSRV_OPENFLOW_FLOW_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_FLOW, ptr);  \
    ptr = NULL; \
}while(0)

#define HSRV_OPENFLOW_ACT_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_ACTION, size)
#define HSRV_OPENFLOW_ACT_CALLOC(size) HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW_ACTION, size)
#define HSRV_OPENFLOW_ACT_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_ACTION, ptr);  \
    ptr = NULL; \
}while(0)

extern uint32 g_pst_ttp_bridge_flow_table_miss_group_id;
extern uint32 g_pst_ttp_bridge_flow_table_miss_nhid;
extern uint32 g_pst_ttp_bridge_flow_table_miss_stats_id;

extern uint32 g_pst_ttp_unicast_flow_table_miss_group_id;
extern uint32 g_pst_ttp_unicast_flow_table_miss_nhid;
extern uint32 g_pst_ttp_unicast_flow_table_miss_stats_id;

extern uint32 g_pst_ttp_vlan_mapping_flow_table_miss_stats_id;

enum metadata_type_s
{
    METADATA_TYPE_FID,
    METADATA_TYPE_VRF,
    METADATA_TYPE_MAX
};
typedef enum metadata_type_s metadata_type_t;

struct metadata_mapping_s
{
    metadata_type_t type;
    uint64 metadata;
    uint32 ref_cnt;
    union
    {
        uint16 fid;
        uint16 vrf_id;
    }u;
};
typedef struct metadata_mapping_s metadata_mapping_t;

struct hsrv_vlan_flow_match_s
{
    uint32  inport;

    uint32  inport_mask;

    uint16  vlan_id;

    uint16  vlan_id_mask;
};
typedef struct hsrv_vlan_flow_match_s hsrv_vlan_flow_match_t;

struct hsrv_termination_mac_flow_match_s
{
  /** Must be 0x0800 */
  uint16    ether_type;

  uint64    metadata;   /*asic not support*/

  /** Destination MAC */
  uint8   dst_mac[MAC_ADDR_LEN];
};
typedef struct hsrv_termination_mac_flow_match_s hsrv_termination_mac_flow_match_t;

struct hsrv_bridge_flow_match_s
{
    uint32  vniid;  /*get from metadata*/

    uint16  fid;    /*get from metadata mapping*/

    uint64  metadata;
    
    /** Destination MAC */
    uint8   dst_mac[MAC_ADDR_LEN];
};
typedef struct hsrv_bridge_flow_match_s hsrv_bridge_flow_match_t;

struct hsrv_uincast_routing_flow_match_s
{
    /** Ethertype. Must be 0x0800 */
    uint16  ether_type;

    uint16  vrf;    /*get from metadata mapping*/

    uint64  metadata;

    /** Destination IPv4 address stored in host byte order.*/
    ip_addr_t   dst_ip;

    /** IP mask stored in host byte order. Must be prefix mask  */
    ip_addr_t   dst_ip_mask;
};
typedef struct hsrv_uincast_routing_flow_match_s hsrv_uincast_routing_flow_match_t;

struct hsrv_ingress_acl_flow_match_s
{
    /** OpenFlow Ingress Port number */
    uint32    inport;

    /** Field maskable only */
    uint32    inport_mask;

    /** Field maskable only */
    uint16    ether_type;

    uint16    ether_type_mask;

    uint64    metadata; /*not support now*/

    /** Source v4 IP stored in host byte order */
    ip_addr_t   src_ip;

    /** Source v4 IP mask stored in host byte order */
    ip_addr_t   src_ip_mask;

    /** Destination v4 IP stored in host byte order */
    ip_addr_t   dst_ip;

    /** Destination v4 IP mask stored in host byte order */
    ip_addr_t   dst_ip_mask;

    /** IP protocol. Only used if etherType == 0x0800 */
    uint8    ip_proto;

    /** IP protocol Mask, field maskable only */
    uint8    ip_proto_mask;

    /** Source L4 Port. Only used for TCP, UDP */
    uint16    src_port;

    /** Mask */
    uint16    src_port_mask;

    /** Destination L4 Port. Only used for TCP, UDP */
    uint16    dst_port;

    /** Mask */
    uint16    dst_port_mask;
};
typedef struct hsrv_ingress_acl_flow_match_s hsrv_ingress_acl_flow_match_t;




struct hsrv_vlan_flow_entry_s
{
    hsrv_vlan_flow_match_t  match_criteria;
    
    uint32  goto_table;

    uint64  metadata;   /*set fid*/
};
typedef struct hsrv_vlan_flow_entry_s hsrv_vlan_flow_entry_t;

struct hsrv_termination_mac_flow_entry_s
{
    /* Match Criteria  */
    hsrv_termination_mac_flow_match_t  match_criteria;

    uint32  goto_table;

    uint64  metadata;    /* set VRF */
};
typedef struct hsrv_termination_mac_flow_entry_s hsrv_termination_mac_flow_entry_t;

struct hsrv_bridge_flow_entry_s
{
    /* Match Criteria  */
    hsrv_bridge_flow_match_t  match_criteria;

    uint32  goto_table;

    /** Write-Action(s) instruction */
    /** data for GROUP action must be a L2 Interface, L2 Flood
    ** group entry as appropriate */
    uint32  group_id;   
};
typedef struct hsrv_bridge_flow_entry_s hsrv_bridge_flow_entry_t;

struct hsrv_uincast_routing_flow_entry_s
{
    /* Match Criteria  */
    hsrv_uincast_routing_flow_match_t  match_criteria;

    uint32  goto_table;

    /** Write-Action(s) instruction */
    /** Data for GROUP action, must be an L3 Unicast group or ECMP group entry */
    uint32  group_id;              

    bool    send_to_controller;
    
    bool    ttl_dec; /*default ttl dec*/    
};
typedef struct hsrv_uincast_routing_flow_entry_s hsrv_uincast_routing_flow_entry_t;

struct hsrv_ingress_acl_flow_entry_s
{
    /** match criteria */
    hsrv_ingress_acl_flow_match_t match_criteria;

    bool    send_to_controller;
    /** Clear-Action(s) instruction */
    /** if set, the action set is cleared */
    bool    clear_action;

    /** If non-zero, apply meter indicated */
    bool    meter_action;

    bool    redirect_action;

    bool    discard_action;
    
    bool    mirror_action;

    uint32  mirror_id;

    /** data for meter identifer */
    uint32    meter_id;

    /**  data for OUTPUT apply-action, restricted to CONTROLLER, set to 0 otherwise */
    uint32    output_port;
};
typedef struct hsrv_ingress_acl_flow_entry_s hsrv_ingress_acl_flow_entry_t;


struct hsrv_openflow_ttp_flow_entry_s
{
    /** Flow Table Id */
    uint32  table_id;  /* refer to OPENFLOW_TTP_FLOW_TABLE_ID_t */

    uint32  flow_id;

    /** Flow priority */
    uint32  priority;

    union
    {

        /** VLAN Port Table Flow Entry */
        hsrv_vlan_flow_entry_t             vlan_flow_entry;

        /** Termination MAC Table Flow Entry */
        hsrv_termination_mac_flow_entry_t   termination_mac_flow_entry;

        /** Bridging Table Flow Entry */
        hsrv_bridge_flow_entry_t         bridge_flow_entry;

        /** Unicast Routing Table Flow Entry */
        hsrv_uincast_routing_flow_entry_t   unicast_routing_flow_entry;

        /** Ingress Table Flow Entry */
        hsrv_ingress_acl_flow_entry_t       ingress_acl_flow_entry;

    } flow_data;

};
typedef struct hsrv_openflow_ttp_flow_entry_s hsrv_openflow_ttp_flow_entry_t;

struct hsrv_openflow_ttp_flow_s
{
    hsrv_openflow_ttp_flow_entry_t  flow;

     /*
     * This is a combo field, entry_id can be scl entry id, acl entry id,
     * mpls ilm label, and so on
     */
    uint32  entry_id;

    /* hsrv layer stats_id */
    uint32  stats_id;

    uint8   stats_supported;
};
typedef struct hsrv_openflow_ttp_flow_s hsrv_openflow_ttp_flow_t;

/*
int32
hsrv_openflow_show_flow_brief(FILE *fp);

int32
hsrv_openflow_show_flow(FILE *fp, uint32 flow_id);
*/

int32
hsrv_openflow_lookup_metadata_mapping(uint64 metadata, metadata_mapping_t** pp_metadata);

int32
hsrv_openflow_tunnel_get_info_by_metadata(uint32 meta_data, uint16* p_ofport, uint32* p_tun_id);

hsrv_openflow_ttp_flow_t*
hsrv_openflow_get_flow_info(uint32 flow_id);

int32
hsrv_openflow_flow_start(void);

#endif /* !__HSRV_OPENFLOW_FLOW_H__ */
