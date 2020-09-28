
#ifndef __TBL_COPP_CFG_DEFINE_H__
#define __TBL_COPP_CFG_DEFINE_H__

/* TBL_COPP_CFG field defines */
typedef enum
{
    TBL_COPP_CFG_FLD_COPP_PNAME           = 0 ,  /* RW */
    TBL_COPP_CFG_FLD_COPP_IFIDX           = 1 ,  /* RW */
    TBL_COPP_CFG_FLD_COPP_TOTAL_RATE      = 2 ,  /* RW */
    TBL_COPP_CFG_FLD_MAX                  = 3 
} tbl_copp_cfg_field_id_t;

/* TBL_COPP_CFG defines */
typedef struct
{
    char                 copp_pname[POLICER_NAME_SIZE]; /* only support 1 copp-pmap-name global */
    uint32               copp_ifidx;          /* copp cpu interface index */
    uint32               copp_total_rate;     /* copp total rate value */
} tbl_copp_cfg_t;

#endif /* !__TBL_COPP_CFG_DEFINE_H__ */

