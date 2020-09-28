
#ifndef __TBL_FEA_QOS_DOMAIN_DEFINE_H__
#define __TBL_FEA_QOS_DOMAIN_DEFINE_H__

/* TBL_FEA_QOS_DOMAIN field defines */
typedef enum
{
    TBL_FEA_QOS_DOMAIN_FLD_KEY                  = 0 ,  /* RW */
    TBL_FEA_QOS_DOMAIN_FLD_DOT1P_TO_TC_COLOR_MAP_ID = 1 ,  /* RW */
    TBL_FEA_QOS_DOMAIN_FLD_DSCP_TO_TC_COLOR_MAP_ID = 2 ,  /* RW */
    TBL_FEA_QOS_DOMAIN_FLD_EXP_TO_TC_COLOR_MAP_ID = 3 ,  /* RW */
    TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DOT1P_MAP_ID = 4 ,  /* RW */
    TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_DSCP_MAP_ID = 5 ,  /* RW */
    TBL_FEA_QOS_DOMAIN_FLD_TC_AND_COLOR_TO_EXP_MAP_ID = 6 ,  /* RW */
    TBL_FEA_QOS_DOMAIN_FLD_MAX                  = 7 
} tbl_fea_qos_domain_field_id_t;

/* TBL_FEA_QOS_DOMAIN defines */
typedef struct
{
    uint32               id;
} tbl_fea_qos_domain_key_t;

typedef struct
{
    tbl_fea_qos_domain_key_t key;
    uint64               dot1p_to_tc_color_map_id;
    uint64               dscp_to_tc_color_map_id;
    uint64               exp_to_tc_color_map_id;
    uint64               tc_and_color_to_dot1p_map_id;
    uint64               tc_and_color_to_dscp_map_id;
    uint64               tc_and_color_to_exp_map_id;
} tbl_fea_qos_domain_t;

typedef struct
{
    tbl_fea_qos_domain_t *fea_domain_array[GLB_QOS_DOMAIN_NUM];
} tbl_fea_qos_domain_master_t;

#endif /* !__TBL_FEA_QOS_DOMAIN_DEFINE_H__ */

