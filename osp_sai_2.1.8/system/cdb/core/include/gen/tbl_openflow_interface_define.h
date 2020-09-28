
#ifndef __TBL_OPENFLOW_INTERFACE_DEFINE_H__
#define __TBL_OPENFLOW_INTERFACE_DEFINE_H__

/* TBL_OPENFLOW_INTERFACE field defines */
typedef enum
{
    TBL_OPENFLOW_INTERFACE_FLD_KEY                  = 0 ,  /* READ */
    TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_ENABLE      = 1 ,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_OBEY_VLAN_FILTER     = 2 ,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_OPENFLOW_INSTANCE_ENABLE = 3 ,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_IS_ADD_TO_BR0        = 4 ,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN       = 5 ,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE = 6 ,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_PROTECTED_VLAN_CHANGE_IS_SET = 7 ,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_ACTION               = 8 ,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_INGRESS_ADD_NATIVE_VLAN_ENABLE = 9 ,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_EGRESS_REMOVE_NATIVE_VLAN_ENABLE = 10,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_IFINDEX              = 11,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_TYPE     = 12,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_BIND_TUNNEL_CNT      = 13,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_TUNNEL_MPLS_DISABLE  = 14,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_IS_BIND_TUNNEL       = 15,  /* RW */
    TBL_OPENFLOW_INTERFACE_FLD_MAX                  = 16
} tbl_openflow_interface_field_id_t;

/* TBL_OPENFLOW_INTERFACE defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_openflow_interface_key_t;

typedef struct
{
    tbl_openflow_interface_key_t key;
    uint8                openflow_enable;     /* openflow enable, default:0 */
    uint8                obey_vlan_filter;    /* obey vlan filter, default: 0 */
    uint8                openflow_instance_enable; /*ovs-db create and openflow enable*/
    uint8                is_add_to_br0;       /*add to bridge 0*/
    uint32               protected_vlan[OPENFLOW_PROTECTED_VLAN]; /*protected vlan*/
    uint32               protected_vlan_change; /*protected vlan change*/
    uint32               protected_vlan_change_is_set; /*protected vlan change is set or unset*/
    uint32               action[OPENFLOW_PACKET_ACTION]; /* openflow packet action*/
    uint32               ingress_add_native_vlan_enable; /*openflow enable native vlan*/
    uint32               egress_remove_native_vlan_enable; /*openflow enable native vlan*/
    uint32               ifindex;             /*openflow agg ifindex*/
    uint32               bind_tunnel_type;
    uint32               bind_tunnel_cnt;
    uint32               tunnel_mpls_disable; /* openflow tunnel-mpls disable, default:0 */
    uint32               is_bind_tunnel;      /* bind by tunnel, default is 0 */
} tbl_openflow_interface_t;

typedef struct
{
    ctclib_hash_t        *if_hash;
    ctclib_slist_t       *if_list;
} tbl_openflow_interface_master_t;

#endif /* !__TBL_OPENFLOW_INTERFACE_DEFINE_H__ */

