
#ifndef __TBL_ACL_CONFIG_DEFINE_H__
#define __TBL_ACL_CONFIG_DEFINE_H__

/* TBL_ACL_CONFIG field defines */
typedef enum
{
    TBL_ACL_CONFIG_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACL_CONFIG_FLD_ACE_REF              = 1 ,  /* RW */
    TBL_ACL_CONFIG_FLD_ACEV6_REF            = 2 ,  /* RW */
    TBL_ACL_CONFIG_FLD_ACEMAC_REF           = 3 ,  /* RW */
    TBL_ACL_CONFIG_FLD_INTF_REF_IN          = 4 ,  /* RW */
    TBL_ACL_CONFIG_FLD_INTF_REF_OUT         = 5 ,  /* RW */
    TBL_ACL_CONFIG_FLD_L4_PORT_REF          = 6 ,  /* RW */
    TBL_ACL_CONFIG_FLD_TCP_FLAGS_REF        = 7 ,  /* RW */
    TBL_ACL_CONFIG_FLD_REMARK               = 8 ,  /* RW */
    TBL_ACL_CONFIG_FLD_SEQ_REF              = 9 ,  /* RW */
    TBL_ACL_CONFIG_FLD_ETHER_REF            = 10,  /* RW */
    TBL_ACL_CONFIG_FLD_ARP_REF              = 11,  /* RW */
    TBL_ACL_CONFIG_FLD_TAP_REF              = 12,  /* RW */
    TBL_ACL_CONFIG_FLD_TYPE_IDENTIFYING     = 13,  /* RW */
    TBL_ACL_CONFIG_FLD_ACE_TRUNCATION_REF_CNT = 14,  /* RW */
    TBL_ACL_CONFIG_FLD_DECAP_EN             = 15,  /* RW */
    TBL_ACL_CONFIG_FLD_DECAP_INNER_MATCH_SEQ_MAX = 16,  /* RW */
    TBL_ACL_CONFIG_FLD_DECAP_NORMAL_SEQ_MIN = 17,  /* RW */
    TBL_ACL_CONFIG_FLD_SERVICE_IPT_ATTACHED = 18,  /* RW */
    TBL_ACL_CONFIG_FLD_MAX                  = 19
} tbl_acl_config_field_id_t;

/* TBL_ACL_CONFIG defines */
typedef struct
{
    char                 name[ACL_NAME_FULL_NAME_SIZE+1];
} tbl_acl_config_key_t;

typedef struct
{
    tbl_acl_config_key_t key;
    uint32               ace_ref;
    uint32               acev6_ref;
    uint32               acemac_ref;
    uint32               intf_ref_in;
    uint32               intf_ref_out;
    uint32               l4_port_ref;
    uint32               tcp_flags_ref;
    char                 remark[ACL_REMARK_SIZE+1];
    uint32               seq_ref;
    uint32               ether_ref;
    uint32               arp_ref;
    uint32               tap_ref;             /* tap ingress group flow used count */
    uint32               type_identifying;    /* identifying how  to use this acl flow */
    uint32               ace_truncation_ref_cnt;
    uint32               decap_en;            /* decap en */
    uint32               decap_inner_match_seq_max; /* decap type use */
    uint32               decap_normal_seq_min; /* decap type use */
    uint32               service_ipt_attached; /* attached by system service iptables rules based on acl */
} tbl_acl_config_t;

typedef struct
{
    ctclib_hash_t        *acl_config_hash;
    ctclib_slist_t       *acl_config_list;
} tbl_acl_config_master_t;

#endif /* !__TBL_ACL_CONFIG_DEFINE_H__ */

