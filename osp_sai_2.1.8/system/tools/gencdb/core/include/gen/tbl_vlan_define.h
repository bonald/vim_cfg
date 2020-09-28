
#ifndef __TBL_VLAN_DEFINE_H__
#define __TBL_VLAN_DEFINE_H__

/* TBL_VLAN field defines */
typedef enum
{
    TBL_VLAN_FLD_KEY                  = 0 ,  /* READ */
    TBL_VLAN_FLD_MAX_MAC_COUNT        = 1 ,  /* RW */
    TBL_VLAN_FLD_CUR_MAC_COUNT        = 2 ,  /* READ */
    TBL_VLAN_FLD_INSTANCE             = 3 ,  /* RW */
    TBL_VLAN_FLD_DHCSNOOPING          = 4 ,  /* RW */
    TBL_VLAN_FLD_ARPSNOOPING          = 5 ,  /* RW */
    TBL_VLAN_FLD_SECURITY_ACTION      = 6 ,  /* RW */
    TBL_VLAN_FLD_ENABLE               = 7 ,  /* RW */
    TBL_VLAN_FLD_MAC_LEARNING_EN      = 8 ,  /* RW */
    TBL_VLAN_FLD_OP_IFINDEX           = 9 ,  /* RW */
    TBL_VLAN_FLD_MEMBER_PORT          = 10,  /* READ */
    TBL_VLAN_FLD_NAME                 = 11,  /* RW */
    TBL_VLAN_FLD_ERPS_DOMAIN_ID       = 12,  /* RW */
    TBL_VLAN_FLD_DOT1X_GUEST_VLAN_NUM = 13,  /* RW */
    TBL_VLAN_FLD_STATS_EN             = 14,  /* RW */
    TBL_VLAN_FLD_G8032_RING_ID        = 15,  /* RW */
    TBL_VLAN_FLD_MAX                  = 16
} tbl_vlan_field_id_t;

/* TBL_VLAN defines */
typedef struct
{
    vid_t                vid;
} tbl_vlan_key_t;

typedef struct
{
    tbl_vlan_key_t       key;
    uint32               max_mac_count;
    uint32               cur_mac_count;
    uint32               instance;
    uint32               dhcsnooping;
    uint32               arpsnooping;
    uint32               security_action;
    uint16               enable;
    uint16               mac_learning_en;
    uint64               op_ifindex;
    vlan_memberport_t    member_port;
    char                 name[VLANNAME_SIZE+1];
    uint32               erps_domain_id;
    uint32               dot1x_guest_vlan_num;
    uint32               stats_en;
    uint32               g8032_ring_id;
} tbl_vlan_t;

typedef struct
{
    tbl_vlan_t           *vlan_array[GLB_MAX_VLAN_ID];
} tbl_vlan_master_t;

#endif /* !__TBL_VLAN_DEFINE_H__ */

