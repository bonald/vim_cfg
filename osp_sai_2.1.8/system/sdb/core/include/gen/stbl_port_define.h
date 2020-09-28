
#ifndef __STBL_PORT_DEFINE_H__
#define __STBL_PORT_DEFINE_H__

/* STBL_PORT field defines */
typedef enum
{
    STBL_PORT_FLD_KEY                  = 0 ,  /* READ */
    STBL_PORT_FLD_FLAG                 = 1 ,  /* READ */
    STBL_PORT_FLD_GPORT                = 2 ,  /* READ */
    STBL_PORT_FLD_LAG_ID               = 3 ,  /* READ */
    STBL_PORT_FLD_TIMESTAMP_COUNT      = 4 ,  /* READ */
    STBL_PORT_FLD_TIMESTAMP_NEXTHOPID  = 5 ,  /* READ */
    STBL_PORT_FLD_GROUP_ID_IGS         = 6 ,  /* READ */
    STBL_PORT_FLD_GROUP_ID_EGS         = 7 ,  /* READ */
    STBL_PORT_FLD_TTP_GROUP_ID_IGS     = 8 ,  /* READ */
    STBL_PORT_FLD_PORT_CLASS_ID        = 9 ,  /* READ */
    STBL_PORT_FLD_VLAN_BMP             = 10,  /* READ */
    STBL_PORT_FLD_VLANCLASS_GROUP_ID   = 11,  /* READ */
    STBL_PORT_FLD_VLANCLASS_DEFAULT_ACTION = 12,  /* READ */
    STBL_PORT_FLD_VLANCLASS_ACTIVE_TYPE = 13,  /* READ */
    STBL_PORT_FLD_ISOLATION_OID        = 14,  /* READ */
    STBL_PORT_FLD_ISOLATION_DIR        = 15,  /* READ */
    STBL_PORT_FLD_ISOLATION_TYPE       = 16,  /* READ */
    STBL_PORT_FLD_TAG_CTRL             = 17,  /* READ */
    STBL_PORT_FLD_DOT1Q_TYPE           = 18,  /* READ */
    STBL_PORT_FLD_TRANSMIT_EN          = 19,  /* READ */
    STBL_PORT_FLD_RECEIVE_EN           = 20,  /* READ */
    STBL_PORT_FLD_KEEP_VLAN_TAG        = 21,  /* READ */
    STBL_PORT_FLD_INGRESS_VLAN_FILTER  = 22,  /* READ */
    STBL_PORT_FLD_EGRESS_VLAN_FILTER   = 23,  /* READ */
    STBL_PORT_FLD_SRC_DISCARD          = 24,  /* READ */
    STBL_PORT_FLD_SRC_OUTER_IS_SVLAN   = 25,  /* READ */
    STBL_PORT_FLD_LAG_BIND_EN          = 26,  /* READ */
    STBL_PORT_FLD_FLOWCTL_DIR          = 27,  /* READ */
    STBL_PORT_FLD_FLOWCTL_EN           = 28,  /* READ */
    STBL_PORT_FLD_TYPE_IS_ACCESS       = 29,  /* READ */
    STBL_PORT_FLD_METADATA_EN          = 30,  /* READ */
    STBL_PORT_FLD_EGRESS_ACL_EN        = 31,  /* READ */
    STBL_PORT_FLD_REFLECTIVE_BRIDGE_EN = 32,  /* READ */
    STBL_PORT_FLD_DHCPRELAY_EXCEPTION  = 33,  /* READ */
    STBL_PORT_FLD_MAC_LIMIT_NUM        = 34,  /* READ */
    STBL_PORT_FLD_MAC_LIMIT_ACTION     = 35,  /* READ */
    STBL_PORT_FLD_PORT_STATE           = 36,  /* READ */
    STBL_PORT_FLD_RX_STATS_PTR         = 37,  /* READ */
    STBL_PORT_FLD_TX_STATS_PTR         = 38,  /* READ */
    STBL_PORT_FLD_MAX                  = 39
} stbl_port_field_id_t;

/* STBL_PORT defines */
typedef struct
{
    sai_object_id_t      port_oid;
} stbl_port_key_t;

typedef struct
{
    stbl_port_key_t      key;
    uint32               flag;
    uint32               gport;
    sai_object_id_t      lag_id;
    int32                timestamp_count;
    uint32               timestamp_nexthopid;
    uint32               group_id_igs;
    uint32               group_id_egs;
    uint32               ttp_group_id_igs;
    uint32               port_class_id;
    vlan_bmp_t           vlan_bmp;
    uint32               vlanclass_group_id;
    uint32               vlanclass_default_action;
    uint32               vlanclass_active_type; /* sai_port_vlanclass_active_type_t */
    sai_object_id_t      isolation_oid;
    uint32               isolation_dir;       /* sai_port_isolation_direction_t */
    uint32               isolation_type;      /* sai_port_isolation_type_t */
    uint32               tag_ctrl;            /* ctc_vlantag_ctl_t */
    uint32               dot1q_type;          /* ctc_dot1q_type_t */
    uint32               transmit_en;
    uint32               receive_en;
    uint32               keep_vlan_tag;
    uint32               ingress_vlan_filter;
    uint32               egress_vlan_filter;
    uint32               src_discard;
    uint32               src_outer_is_svlan;
    uint32               lag_bind_en;
    uint32               flowctl_dir;
    uint32               flowctl_en;
    uint32               type_is_access;
    uint32               metadata_en;
    uint32               egress_acl_en;
    uint32               reflective_bridge_en;
    uint32               dhcprelay_exception;
    uint32               mac_limit_num;
    uint32               mac_limit_action;
    uint32               port_state;          /* sai_port_oper_status_t */
    sdb_mac_rx_stats_ptr_t rx_stats_ptr;        /* pointer to ctc_stats_mac_rec_t */
    sdb_mac_tx_stats_ptr_t tx_stats_ptr;        /* pointer to ctc_stats_mac_snd_t */
} stbl_port_t;

typedef struct
{
    ctclib_hash_t        *port_hash;
} stbl_port_master_t;

#endif /* !__STBL_PORT_DEFINE_H__ */

