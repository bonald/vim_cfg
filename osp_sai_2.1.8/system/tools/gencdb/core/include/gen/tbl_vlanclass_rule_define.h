
#ifndef __TBL_VLANCLASS_RULE_DEFINE_H__
#define __TBL_VLANCLASS_RULE_DEFINE_H__

/* TBL_VLANCLASS_RULE field defines */
typedef enum
{
    TBL_VLANCLASS_RULE_FLD_KEY                  = 0 ,  /* READ */
    TBL_VLANCLASS_RULE_FLD_RULE_TYPE            = 1 ,  /* RW */
    TBL_VLANCLASS_RULE_FLD_VLAN_ID              = 2 ,  /* RW */
    TBL_VLANCLASS_RULE_FLD_MAC                  = 3 ,  /* RW */
    TBL_VLANCLASS_RULE_FLD_IP_ADDRESS           = 4 ,  /* RW */
    TBL_VLANCLASS_RULE_FLD_PROTOCOL_TYPE        = 5 ,  /* RW */
    TBL_VLANCLASS_RULE_FLD_PROTOCOL_ACTION      = 6 ,  /* RW */
    TBL_VLANCLASS_RULE_FLD_RULE_OID             = 7 ,  /* RW */
    TBL_VLANCLASS_RULE_FLD_MAX                  = 8 
} tbl_vlanclass_rule_field_id_t;

/* TBL_VLANCLASS_RULE defines */
typedef struct
{
    uint32               rule_id;
} tbl_vlanclass_rule_key_t;

typedef struct
{
    tbl_vlanclass_rule_key_t key;
    uint32               rule_type;           /*mac, protocol, subnet*/
    vid_t                vlan_id;             /*VLAN ID*/
    mac_addr_t           mac;                 /*mac address*/
    addr_t               ip_address;          /*ip address*/
    uint32               protocol_type;       /*vlanclss_rule_protocol_type_t*/
    uint32               protocol_action;     /*vlanclss_rule_protocol_action_t*/
    uint64               rule_oid;            /*rule object id*/
} tbl_vlanclass_rule_t;

typedef struct
{
    tbl_vlanclass_rule_t *vlanclass_rule_array[GLB_VLANCLASS_RULE_ID_MAX];
} tbl_vlanclass_rule_master_t;

#endif /* !__TBL_VLANCLASS_RULE_DEFINE_H__ */

