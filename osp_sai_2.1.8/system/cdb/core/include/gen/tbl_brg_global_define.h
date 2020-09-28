
#ifndef __TBL_BRG_GLOBAL_DEFINE_H__
#define __TBL_BRG_GLOBAL_DEFINE_H__

/* TBL_BRG_GLOBAL field defines */
typedef enum
{
    TBL_BRG_GLOBAL_FLD_MAX_STATIC_FDB       = 0 ,  /* RW */
    TBL_BRG_GLOBAL_FLD_MAX_L2MCAST_FDB      = 1 ,  /* RW */
    TBL_BRG_GLOBAL_FLD_FDB_AGING_TIME       = 2 ,  /* RW */
    TBL_BRG_GLOBAL_FLD_OPER_FDB_AGING_TIME  = 3 ,  /* RW */
    TBL_BRG_GLOBAL_FLD_HW_LEARNING_ENABLE   = 4 ,  /* RW */
    TBL_BRG_GLOBAL_FLD_IPSG_MAX_PORT_BINDING = 5 ,  /* RW */
    TBL_BRG_GLOBAL_FLD_CUR_IPV4_SOURCE_GUARD = 6 ,  /* RW */
    TBL_BRG_GLOBAL_FLD_PORT_ISOLATE_MODE    = 7 ,  /* RW */
    TBL_BRG_GLOBAL_FLD_INSTANCE             = 8 ,  /* RW */
    TBL_BRG_GLOBAL_FLD_PVLAN_ISOLATE_ID     = 9 ,  /* RW */
    TBL_BRG_GLOBAL_FLD_MAX_INSTANCE         = 10,  /* READ */
    TBL_BRG_GLOBAL_FLD_MAX_PVLAN_ISOLATE_ID = 11,  /* READ */
    TBL_BRG_GLOBAL_FLD_FDB_LOOP_ERRDIS_EN   = 12,  /* RW */
    TBL_BRG_GLOBAL_FLD_FDB_LOOP_MAX_SIZE    = 13,  /* RW */
    TBL_BRG_GLOBAL_FLD_FDB_LOOP_ADD_RATE    = 14,  /* RW */
    TBL_BRG_GLOBAL_FLD_FDB_LOOP_DISC_CNT    = 15,  /* RW */
    TBL_BRG_GLOBAL_FLD_FDB_LOOP_CURR_SIZE   = 16,  /* RW */
    TBL_BRG_GLOBAL_FLD_TRAP_DELAY_UP        = 17,  /* RW */
    TBL_BRG_GLOBAL_FLD_TRAP_DELAY_DOWN      = 18,  /* RW */
    TBL_BRG_GLOBAL_FLD_FDB_ALL_FLUSHING     = 19,  /* RW */
    TBL_BRG_GLOBAL_FLD_MAC_BASED_VLAN_CLASS = 20,  /* RW */
    TBL_BRG_GLOBAL_FLD_IPV4_BASED_VLAN_CLASS = 21,  /* RW */
    TBL_BRG_GLOBAL_FLD_IPV6_BASED_VLAN_CLASS = 22,  /* RW */
    TBL_BRG_GLOBAL_FLD_MAX_MIRROR_MAC_ESCAPE = 23,  /* RW */
    TBL_BRG_GLOBAL_FLD_L2PROTOCOL_COS       = 24,  /* RW */
    TBL_BRG_GLOBAL_FLD_VLAN_MAPPING_ENTRY_CNT = 25,  /* RW */
    TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1      = 26,  /* RW */
    TBL_BRG_GLOBAL_FLD_TPID_TYPE1_REF_CNT   = 27,  /* RW */
    TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2      = 28,  /* RW */
    TBL_BRG_GLOBAL_FLD_TPID_TYPE2_REF_CNT   = 29,  /* RW */
    TBL_BRG_GLOBAL_FLD_MAX                  = 30
} tbl_brg_global_field_id_t;

/* TBL_BRG_GLOBAL defines */
typedef struct
{
    uint32               max_static_fdb;      /* static unicast FDB */
    uint32               max_l2mcast_fdb;     /* static multicast FDB */
    uint32               fdb_aging_time;      /* configured ageing-time */
    uint32               oper_fdb_aging_time; /* operate ageing-time, maybe changed for STP topo change */
    uint32               hw_learning_enable;
    uint32               ipsg_max_port_binding;
    uint32               cur_ipv4_source_guard;
    uint32               port_isolate_mode;
    bmp_64_t             instance;
    bmp_32_t             pvlan_isolate_id;
    uint32               max_instance;
    uint32               max_pvlan_isolate_id;
    uint32               fdb_loop_errdis_en;
    uint32               fdb_loop_max_size;
    uint32               fdb_loop_add_rate;
    uint32               fdb_loop_disc_cnt;
    uint32               fdb_loop_curr_size;
    uint32               trap_delay_up;
    uint32               trap_delay_down;
    uint32               fdb_all_flushing;
    uint32               mac_based_vlan_class;
    uint32               ipv4_based_vlan_class;
    uint32               ipv6_based_vlan_class;
    uint32               max_mirror_mac_escape;
    uint32               l2protocol_cos;
    uint32               vlan_mapping_entry_cnt;
    uint32               vlan_tpid_type1;
    uint32               tpid_type1_ref_cnt;
    uint32               vlan_tpid_type2;
    uint32               tpid_type2_ref_cnt;
} tbl_brg_global_t;

#endif /* !__TBL_BRG_GLOBAL_DEFINE_H__ */

