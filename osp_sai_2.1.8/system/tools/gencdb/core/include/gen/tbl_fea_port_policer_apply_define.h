
#ifndef __TBL_FEA_PORT_POLICER_APPLY_DEFINE_H__
#define __TBL_FEA_PORT_POLICER_APPLY_DEFINE_H__

/* TBL_FEA_PORT_POLICER_APPLY field defines */
typedef enum
{
    TBL_FEA_PORT_POLICER_APPLY_FLD_KEY                  = 0 ,  /* READ */
    TBL_FEA_PORT_POLICER_APPLY_FLD_IN_POLICER_ID        = 1 ,  /* RW */
    TBL_FEA_PORT_POLICER_APPLY_FLD_OUT_POLICER_ID       = 2 ,  /* RW */
    TBL_FEA_PORT_POLICER_APPLY_FLD_MAX                  = 3 
} tbl_fea_port_policer_apply_field_id_t;

/* TBL_FEA_PORT_POLICER_APPLY defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_fea_port_policer_apply_key_t;

typedef struct
{
    tbl_fea_port_policer_apply_key_t key;
    uint64               in_policer_id;
    uint64               out_policer_id;
} tbl_fea_port_policer_apply_t;

typedef struct
{
    ctclib_hash_t        *if_hash;
    ctclib_slist_t       *if_list;
} tbl_fea_port_policer_apply_master_t;

#endif /* !__TBL_FEA_PORT_POLICER_APPLY_DEFINE_H__ */

