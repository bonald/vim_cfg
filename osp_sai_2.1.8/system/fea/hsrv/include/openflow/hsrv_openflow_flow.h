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


#define HSRV_OPENFLOW_HSRVACT_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_HSRV_ACTION, size)
#define HSRV_OPENFLOW_HSRVACT_CALLOC(size) HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW_HSRV_ACTION, size)
#define HSRV_OPENFLOW_HSRVACT_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_HSRV_ACTION, ptr);  \
    ptr = NULL; \
}while(0)


struct hsrv_openflow_label_s
{
    glb_openflow_label_type_t type;

    bool   enable;
    uint32 label;
};
typedef struct hsrv_openflow_label_s hsrv_openflow_label_t;

struct hsrv_openflow_map_output_data_s
{
    uint32 flow_id;
    uint16 in_port;
    uint32 ifindex;
    uint16 eth_type;
    uint8  ip_proto;
    uint8  is_group;  /* Added by weizj for bug 35135 */
    uint8  flow_tunnel_type; /* added by hansf for v4/v6 transform , glb_openflow_flow_tnl_type_e */
    uint8  payload_packet_type; /* added by hansf for v4/v6 transform, glb_openflow_flow_payload_packet_type_e */

    glb_openflow_flow_action_list_t* p_actlist;
};
typedef struct hsrv_openflow_map_output_data_s hsrv_openflow_map_output_data_t;

#define CTC_LINKAGG_CHIPID              0x1F        /**< Linkagg port's global chip ID */
#define CTC_LOCAL_PORT_LENGTH           8           /**< Local port id length(unit: bit) */
#define CTC_MAP_TID_TO_GPORT(tid)       ((tid) | (CTC_LINKAGG_CHIPID << CTC_LOCAL_PORT_LENGTH))


struct hsrv_openflow_action_list_match_data_s
{
    glb_openflow_inport_t  in_port;
    uint8  table_id;
    uint8  is_table_mis;
    uint16 eth_type;
    uint8  ip_proto;
};
typedef struct hsrv_openflow_action_list_match_data_s hsrv_openflow_actlist_match_data_t;

struct hsrv_openflow_action_list_info_s
{
    hsrv_openflow_actlist_match_data_t mdata;
    glb_openflow_flow_action_list_t actlist;
    
    hsrv_openflow_nexthop_info_t nh_info;
    uint32  ref_cnt;
};
typedef struct hsrv_openflow_action_list_info_s hsrv_actlist_info_t;

struct hsrv_openflow_dft_const_info_s
{
    uint32  label_id;
    uint32  entry_id;
    uint32  entry_id_extra;
    uint32  stats_id;
    bool    entry_extra_used;
};
typedef struct hsrv_openflow_dft_const_info_s hsrv_openflow_dft_const_info_t;

struct hsrv_openflow_dft_variable_info_s
{
    bool    normal;
    uint32  old_truncate_len;
    uint32  truncate_len;
    uint16  truncate_output_port;
    uint32  output_count;
    uint32  group_vlan_type;
    uint32  group_vlanid;
    uint32  group_vlan_cos;
    uint32  strip_len;
    
    uint32  pw_label;    /* aps */
    uint32  is_aps_flow;
    
    hsrv_openflow_nexthop_info_t nh_info;
    hsrv_actlist_info_t* actlist;
    
    hsrv_openflow_bound_group_t group_info;
    hsrv_openflow_meter_info_t meter_info;
    hsrv_openflow_queue_info_t queue_info;
};
typedef struct hsrv_openflow_dft_variable_info_s hsrv_openflow_dft_variable_info_t;

struct hsrv_openflow_dft_info_s
{
    hsrv_openflow_dft_const_info_t    cnst; /* values may not change when modify flow*/
    hsrv_openflow_dft_variable_info_t var;  /* values may change when modify flow*/
};
typedef struct hsrv_openflow_dft_info_s hsrv_openflow_dft_info_t;

union hsrv_openflow_flow_info_u
{
    hsrv_openflow_dft_info_t dft;
    /* more */
};
typedef union hsrv_openflow_flow_info_u hsrv_openflow_flow_info_t;

struct hsrv_openflow_flow_s
{
    glb_openflow_flow_t flow;
    
    hsrv_openflow_flow_info_t u;
};
typedef struct hsrv_openflow_flow_s hsrv_openflow_flow_t;

int32
hsrv_openflow_show_flow_brief(FILE *fp);

int32
hsrv_openflow_show_flow(FILE *fp, uint32 flow_id);

int32
hsrv_openflow_show_flow_actlist_brief(FILE *fp);

int32
hsrv_openflow_show_flow_actlist_ptr(FILE *fp, uint32 actlist_ptr);

int32
hsrv_openflow_msg_op_label(hsrv_openflow_label_t* p_label);

/* Added by weizj for udf */
int32
hsrv_openflow_msg_set_udf_parser(glb_openflow_udf_parser_t* p_parser);

int32
hsrv_openflow_map_output_action_to_mcast_members(hsrv_openflow_map_output_data_t* p_data,
          hsrv_of_nh_info_t* p_member_nh_array,
          uint32* p_member_cnt);

int32
hsrv_openflow_map_combo_action(glb_openflow_flow_action_t *p_action, hsrv_openflow_action_combo_t *p_action_combo);

void
hsrv_openflow_show_flow_actlist_actions__(FILE *fp, glb_openflow_flow_action_list_t* p_actlist, int prefix);

int32
hsrv_openflow_nexthop_add_port_to_all_action_list(uint16 gport, hsrv_of_nh_info_t *p_member_nh_array, 
                                    uint32 *member_no_use_idx);

int32
hsrv_openflow_nexthop_del_port_from_all_action_list(uint16 gport, hsrv_of_nh_info_t *p_member_nh_array, 
                                    int32 *member_all_idx);

bool
hsrv_openflow_flowdb_get_drop_ingress(void);

int32
hsrv_openflow_flow_start(void);

int32
hsrv_openflow_flow_init(hsrv_system_spec_info_t* p_system_info);

#endif /* !__HSRV_OPENFLOW_FLOW_H__ */
