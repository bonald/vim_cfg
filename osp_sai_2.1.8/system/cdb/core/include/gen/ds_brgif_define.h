
#ifndef __DS_BRGIF_DEFINE_H__
#define __DS_BRGIF_DEFINE_H__

/* DS_BRGIF field defines */
typedef enum
{
    DS_BRGIF_FLD_PVID                 = 0 ,  /* RW */
    DS_BRGIF_FLD_INNER_NATIVE_VID     = 1 ,  /* RW */
    DS_BRGIF_FLD_QINQ_TYPE            = 2 ,  /* RW */
    DS_BRGIF_FLD_QINQ_DEFAULT_FWD     = 3 ,  /* RW */
    DS_BRGIF_FLD_VLAN_TRANSLATION_EN  = 4 ,  /* RW */
    DS_BRGIF_FLD_VLAN_TRANS_DEFAULT_FWD = 5 ,  /* RW */
    DS_BRGIF_FLD_VLAN_MAPPING_TABLE   = 6 ,  /* RW */
    DS_BRGIF_FLD_DOT1Q_TAG_NATIVE_EN  = 7 ,  /* RW */
    DS_BRGIF_FLD_ALLOWED_VLAN         = 8 ,  /* RW */
    DS_BRGIF_FLD_INGRESS_FILTER       = 9 ,  /* RW */
    DS_BRGIF_FLD_EGRESS_FILTER        = 10,  /* READ */
    DS_BRGIF_FLD_TPID_TYPE            = 11,  /* RW */
    DS_BRGIF_FLD_MAC_LEARNING_EN      = 12,  /* RW */
    DS_BRGIF_FLD_OPER_MAC_LEARNING_EN = 13,  /* RW */
    DS_BRGIF_FLD_PORT_SECURITY_EN     = 14,  /* RW */
    DS_BRGIF_FLD_VIOLATE_MODE         = 15,  /* RW */
    DS_BRGIF_FLD_MLAG_IS_PEER_LINK    = 16,  /* READ */
    DS_BRGIF_FLD_MLAG_IS_PORT_BLOCK   = 17,  /* READ */
    DS_BRGIF_FLD_MLAG_IS_GROUP        = 18,  /* READ */
    DS_BRGIF_FLD_MAX_MAC              = 19,  /* RW */
    DS_BRGIF_FLD_UNTAGGED_VLAN        = 20,  /* RW */
    DS_BRGIF_FLD_MAX                  = 21
} ds_brgif_field_id_t;

/* DS_BRGIF defines */
typedef struct
{
    vid_t                pvid;                /* default is 1 */
    vid_t                inner_native_vid;    /* Inner Native Vlan for Qinq Ports */
    uint8                qinq_type;           /* glb_vlan_qinq_type_t */
    uint8                qinq_default_fwd;    /* QinQ default forwarding control */
    uint8                vlan_translation_en; /* VLAN-Translation enable control */
    uint8                vlan_trans_default_fwd; /* VLAN-Translation default forwarding control */
    char                 vlan_mapping_table[GLB_VLAN_MAPPING_TAB_NAME_MAX+1]; /* vlan mapping table name */
    uint8                dot1q_tag_native_en; /* Tagged packet when it is sent out with native vlan */
    brg_allowed_vlan_t   allowed_vlan;
    uint8                ingress_filter;
    uint8                egress_filter;
    uint8                tpid_type;           /* glb_tpid_type_t */
    uint8                mac_learning_en;
    uint8                oper_mac_learning_en; /* calcuted by mac_learning_en and mlag_is_peer_link */
    uint8                port_security_en;
    uint8                violate_mode;
    uint8                mlag_is_peer_link;   /* is MLAG peer link, if set should block MLAG isolation group ingress */
    uint8                mlag_is_port_block;  /* is MLAG peer port block, if set should block MLAG isolation group egress */
    uint8                mlag_is_group;       /* both MLAG port and MLAG peer-link has configured */
    uint32               max_mac;
    brg_allowed_vlan_t   untagged_vlan;       /* only valid for trunk port; if set, vlan is untagged, otherwise, tagged (default) */
} ds_brgif_t;

#endif /* !__DS_BRGIF_DEFINE_H__ */

