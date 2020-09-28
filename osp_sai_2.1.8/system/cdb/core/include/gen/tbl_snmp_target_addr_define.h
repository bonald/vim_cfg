
#ifndef __TBL_SNMP_TARGET_ADDR_DEFINE_H__
#define __TBL_SNMP_TARGET_ADDR_DEFINE_H__

/* TBL_SNMP_TARGET_ADDR field defines */
typedef enum
{
    TBL_SNMP_TARGET_ADDR_FLD_KEY                  = 0 ,  /* RW */
    TBL_SNMP_TARGET_ADDR_FLD_FLAGS                = 1 ,  /* RW */
    TBL_SNMP_TARGET_ADDR_FLD_PARA_NAME            = 2 ,  /* RW */
    TBL_SNMP_TARGET_ADDR_FLD_IP_ADDR              = 3 ,  /* RW */
    TBL_SNMP_TARGET_ADDR_FLD_UPD_PORT             = 4 ,  /* RW */
    TBL_SNMP_TARGET_ADDR_FLD_TIMEOUT              = 5 ,  /* RW */
    TBL_SNMP_TARGET_ADDR_FLD_RETRIES              = 6 ,  /* RW */
    TBL_SNMP_TARGET_ADDR_FLD_MGMT_IF              = 7 ,  /* RW */
    TBL_SNMP_TARGET_ADDR_FLD_TAG_LIST             = 8 ,  /* RW */
    TBL_SNMP_TARGET_ADDR_FLD_MAX                  = 9 
} tbl_snmp_target_addr_field_id_t;

/* TBL_SNMP_TARGET_ADDR defines */
typedef struct
{
    char                 taraddr_name[SNMP_NAME_SIZE];
} tbl_snmp_target_addr_key_t;

typedef struct
{
    tbl_snmp_target_addr_key_t key;                 /* notify name */
    uint8                flags;               /* delete entry 0xFF */
    char                 para_name[SNMP_NAME_SIZE]; /* param name */
    addr_t               ip_addr;             /* target address ipv4 or ipv6 */
    uint32               upd_port;            /* udp_port, default is 162, 0-65535 */
    uint32               timeout;             /* timeout, default is 1500, 0-65535 */
    uint8                retries;             /* retries, default is 3, 0-255 */
    uint8                mgmt_if;             /* mgmt-if */
    char                 tag_list[SNMP_TAGLIST_MAX_LEN]; /* tag name list,default is tar-addr name max 255, max support 128 */
} tbl_snmp_target_addr_t;

typedef struct
{
    ctclib_hash_t        *snmp_target_addr_hash;
    ctclib_slist_t       *snmp_target_addr_list;
} tbl_snmp_target_addr_master_t;

#endif /* !__TBL_SNMP_TARGET_ADDR_DEFINE_H__ */

