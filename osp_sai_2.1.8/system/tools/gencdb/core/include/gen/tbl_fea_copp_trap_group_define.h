
#ifndef __TBL_FEA_COPP_TRAP_GROUP_DEFINE_H__
#define __TBL_FEA_COPP_TRAP_GROUP_DEFINE_H__

/* TBL_FEA_COPP_TRAP_GROUP field defines */
typedef enum
{
    TBL_FEA_COPP_TRAP_GROUP_FLD_KEY                  = 0 ,  /* READ */
    TBL_FEA_COPP_TRAP_GROUP_FLD_POLICER_OID          = 1 ,  /* RW */
    TBL_FEA_COPP_TRAP_GROUP_FLD_TRAP_OID             = 2 ,  /* RW */
    TBL_FEA_COPP_TRAP_GROUP_FLD_QUEUE_ID             = 3 ,  /* RW */
    TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_PACKET         = 4 ,  /* RW */
    TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_BYTE           = 5 ,  /* RW */
    TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_PACKET        = 6 ,  /* RW */
    TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_BYTE          = 7 ,  /* RW */
    TBL_FEA_COPP_TRAP_GROUP_FLD_RED_PACKET           = 8 ,  /* RW */
    TBL_FEA_COPP_TRAP_GROUP_FLD_RED_BYTE             = 9 ,  /* RW */
    TBL_FEA_COPP_TRAP_GROUP_FLD_MAX                  = 10
} tbl_fea_copp_trap_group_field_id_t;

/* TBL_FEA_COPP_TRAP_GROUP defines */
typedef struct
{
    uint32               copp_trap_reason_id;
} tbl_fea_copp_trap_group_key_t;

typedef struct
{
    tbl_fea_copp_trap_group_key_t key;
    uint64               policer_oid;
    uint64               trap_oid;
    uint32               queue_id;
    uint64               green_packet;
    uint64               green_byte;
    uint64               yellow_packet;
    uint64               yellow_byte;
    uint64               red_packet;
    uint64               red_byte;
} tbl_fea_copp_trap_group_t;

typedef struct
{
    ctclib_hash_t        *fea_copp_trap_group_hash;
    ctclib_slist_t       *fea_copp_trap_group_list;
} tbl_fea_copp_trap_group_master_t;

#endif /* !__TBL_FEA_COPP_TRAP_GROUP_DEFINE_H__ */

