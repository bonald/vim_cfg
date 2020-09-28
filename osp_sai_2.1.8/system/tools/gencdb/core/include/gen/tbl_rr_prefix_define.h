
#ifndef __TBL_RR_PREFIX_DEFINE_H__
#define __TBL_RR_PREFIX_DEFINE_H__

/* TBL_RR_PREFIX field defines */
typedef enum
{
    TBL_RR_PREFIX_FLD_KEY                  = 0 ,  /* READ */
    TBL_RR_PREFIX_FLD_RR_PREFIX            = 1 ,  /* RW */
    TBL_RR_PREFIX_FLD_MAX                  = 2 
} tbl_rr_prefix_field_id_t;

/* TBL_RR_PREFIX defines */
typedef struct
{
    uint8                rr_prefix_index;
} tbl_rr_prefix_key_t;

typedef struct
{
    tbl_rr_prefix_key_t  key;
    prefix_t             rr_prefix;
} tbl_rr_prefix_t;

typedef struct
{
    tbl_rr_prefix_t      *rr_prefix_array[GLB_MAX_ECMP_ROUND_ROBIN+1];
} tbl_rr_prefix_master_t;

#endif /* !__TBL_RR_PREFIX_DEFINE_H__ */

