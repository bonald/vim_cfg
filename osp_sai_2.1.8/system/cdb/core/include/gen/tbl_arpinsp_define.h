
#ifndef __TBL_ARPINSP_DEFINE_H__
#define __TBL_ARPINSP_DEFINE_H__

/* TBL_ARPINSP field defines */
typedef enum
{
    TBL_ARPINSP_FLD_KEY                  = 0 ,  /* READ */
    TBL_ARPINSP_FLD_ACLLOG_FILTER        = 1 ,  /* RW */
    TBL_ARPINSP_FLD_DHCPLOG_FILTER       = 2 ,  /* RW */
    TBL_ARPINSP_FLD_FILTER_STATE         = 3 ,  /* RW */
    TBL_ARPINSP_FLD_ENABLE_STATE         = 4 ,  /* RW */
    TBL_ARPINSP_FLD_FORWARDED            = 5 ,  /* READ */
    TBL_ARPINSP_FLD_DROPPED              = 6 ,  /* READ */
    TBL_ARPINSP_FLD_DHCP_DROPPED         = 7 ,  /* READ */
    TBL_ARPINSP_FLD_ACL_DROPPED          = 8 ,  /* READ */
    TBL_ARPINSP_FLD_DHCP_PERMIT          = 9 ,  /* READ */
    TBL_ARPINSP_FLD_ACL_PERMIT           = 10,  /* READ */
    TBL_ARPINSP_FLD_SRCMAC_FAILURE       = 11,  /* READ */
    TBL_ARPINSP_FLD_DSTMAC_FAILURE       = 12,  /* READ */
    TBL_ARPINSP_FLD_IP_FAILURE           = 13,  /* READ */
    TBL_ARPINSP_FLD_INVALID_PROTOCOL     = 14,  /* READ */
    TBL_ARPINSP_FLD_FILTER               = 15,  /* RW */
    TBL_ARPINSP_FLD_MAX                  = 16
} tbl_arpinsp_field_id_t;

/* TBL_ARPINSP defines */
typedef struct
{
    vid_t                vlan_id;
} tbl_arpinsp_key_t;

typedef struct
{
    tbl_arpinsp_key_t    key;
    uint32               acllog_filter;       /*filter acl-match*/
    uint32               dhcplog_filter;      /*filter dhcp binding*/
    uint32               filter_state;        /*filter state: static or dynamic*/
    uint32               enable_state;        /*DAI enable or not*/
    uint64               forwarded;           /* forwarded pkts */
    uint64               dropped;             /* drop pkts */
    uint64               dhcp_dropped;        /* drop pkts by DHCP */
    uint64               acl_dropped;         /* drop pkts by ACL*/
    uint64               dhcp_permit;         /* permit pkts by DHCP*/
    uint64               acl_permit;          /* permit pkts by ACL*/
    uint64               srcmac_failure;      /* drop pkts Source MAC check fail*/
    uint64               dstmac_failure;      /* drop pkts Dest MAC check fail*/
    uint64               ip_failure;          /* drop pkts IP check fail*/
    uint64               invalid_protocol;    /* drop pkts Invalid protocol data*/
    char                 filter[ARPINSP_FILTER_MAX_NUM+1]; /*filter name*/
} tbl_arpinsp_t;

typedef struct
{
    tbl_arpinsp_t        *arpinsp_array[GLB_MAX_VLAN_ID];
} tbl_arpinsp_master_t;

#endif /* !__TBL_ARPINSP_DEFINE_H__ */

