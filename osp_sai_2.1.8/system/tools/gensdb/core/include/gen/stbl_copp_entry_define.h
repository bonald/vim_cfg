
#ifndef __STBL_COPP_ENTRY_DEFINE_H__
#define __STBL_COPP_ENTRY_DEFINE_H__

/* STBL_COPP_ENTRY field defines */
typedef enum
{
    STBL_COPP_ENTRY_FLD_KEY                  = 0 ,  /* READ */
    STBL_COPP_ENTRY_FLD_ENTRY_OID            = 1 ,  /* READ */
    STBL_COPP_ENTRY_FLD_COUNTER_OID          = 2 ,  /* READ */
    STBL_COPP_ENTRY_FLD_NH_OID               = 3 ,  /* READ */
    STBL_COPP_ENTRY_FLD_TYPE                 = 4 ,  /* READ */
    STBL_COPP_ENTRY_FLD_IP_TYPE              = 5 ,  /* READ */
    STBL_COPP_ENTRY_FLD_PROTOCOL_NUM         = 6 ,  /* READ */
    STBL_COPP_ENTRY_FLD_DST_PORT             = 7 ,  /* READ */
    STBL_COPP_ENTRY_FLD_MAX                  = 8 
} stbl_copp_entry_field_id_t;

/* STBL_COPP_ENTRY defines */
typedef struct
{
    uint32               copp_entry_id;
} stbl_copp_entry_key_t;

typedef struct
{
    stbl_copp_entry_key_t key;
    sai_object_id_t      entry_oid;
    sai_object_id_t      counter_oid;
    sai_object_id_t      nh_oid;
    uint8                type;
    uint8                ip_type;
    uint16               protocol_num;
    uint16               dst_port;
} stbl_copp_entry_t;

typedef struct
{
    stbl_copp_entry_t    *array[GLB_COPP_ENTRY_MAX];
} stbl_copp_entry_master_t;

#endif /* !__STBL_COPP_ENTRY_DEFINE_H__ */

