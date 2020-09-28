
#ifndef __TBL_QOS_DOMAIN_DEFINE_H__
#define __TBL_QOS_DOMAIN_DEFINE_H__

/* TBL_QOS_DOMAIN field defines */
typedef enum
{
    TBL_QOS_DOMAIN_FLD_KEY                  = 0 ,  /* RW */
    TBL_QOS_DOMAIN_FLD_PHB_ENABLE           = 1 ,  /* READ */
    TBL_QOS_DOMAIN_FLD_CFI_ENABLE           = 2 ,  /* RW */
    TBL_QOS_DOMAIN_FLD_COS2PRI              = 3 ,  /* RW */
    TBL_QOS_DOMAIN_FLD_COS2COLOR            = 4 ,  /* RW */
    TBL_QOS_DOMAIN_FLD_COS2PHB              = 5 ,  /* RW */
    TBL_QOS_DOMAIN_FLD_DSCP2PRI             = 6 ,  /* RW */
    TBL_QOS_DOMAIN_FLD_DSCP2COLOR           = 7 ,  /* RW */
    TBL_QOS_DOMAIN_FLD_DSCP2PHB             = 8 ,  /* RW */
    TBL_QOS_DOMAIN_FLD_PREC2PRI             = 9 ,  /* RW */
    TBL_QOS_DOMAIN_FLD_PREC2COLOR           = 10,  /* RW */
    TBL_QOS_DOMAIN_FLD_PREC2PHB             = 11,  /* RW */
    TBL_QOS_DOMAIN_FLD_EXP2PRI              = 12,  /* RW */
    TBL_QOS_DOMAIN_FLD_EXP2COLOR            = 13,  /* RW */
    TBL_QOS_DOMAIN_FLD_EXP2PHB              = 14,  /* RW */
    TBL_QOS_DOMAIN_FLD_PRI_COLOR2COS        = 15,  /* RW */
    TBL_QOS_DOMAIN_FLD_PRI_COLOR2DSCP       = 16,  /* RW */
    TBL_QOS_DOMAIN_FLD_PRI_COLOR2EXP        = 17,  /* RW */
    TBL_QOS_DOMAIN_FLD_PHB2COS              = 18,  /* RW */
    TBL_QOS_DOMAIN_FLD_PHB2DSCP             = 19,  /* RW */
    TBL_QOS_DOMAIN_FLD_PHB2EXP              = 20,  /* RW */
    TBL_QOS_DOMAIN_FLD_COS2TC               = 21,  /* RW */
    TBL_QOS_DOMAIN_FLD_DSCP2TC              = 22,  /* RW */
    TBL_QOS_DOMAIN_FLD_EXP2TC               = 23,  /* RW */
    TBL_QOS_DOMAIN_FLD_TC_COLOR2COS         = 24,  /* RW */
    TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP        = 25,  /* RW */
    TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP         = 26,  /* RW */
    TBL_QOS_DOMAIN_FLD_MAX                  = 27
} tbl_qos_domain_field_id_t;

/* TBL_QOS_DOMAIN defines */
typedef struct
{
    uint32               id;
} tbl_qos_domain_key_t;

typedef struct
{
    tbl_qos_domain_key_t key;
    uint32               phb_enable;          /* used for dump */
    uint32               cfi_enable;
    uint32               cos2pri[GLB_QOS_COS_NUM];
    uint32               cos2color[GLB_QOS_COS_NUM];
    uint32               cos2phb[GLB_QOS_COS_NUM];
    uint32               dscp2pri[GLB_QOS_DSCP_NUM];
    uint32               dscp2color[GLB_QOS_DSCP_NUM];
    uint32               dscp2phb[GLB_QOS_DSCP_NUM];
    uint32               prec2pri[GLB_QOS_IP_PREC_NUM];
    uint32               prec2color[GLB_QOS_IP_PREC_NUM];
    uint32               prec2phb[GLB_QOS_IP_PREC_NUM];
    uint32               exp2pri[GLB_QOS_EXP_NUM];
    uint32               exp2color[GLB_QOS_EXP_NUM];
    uint32               exp2phb[GLB_QOS_EXP_NUM];
    uint32               pri_color2cos[GLB_QOS_PRI_COLOR_TABLE_SIZE]; /* priority << 3 + color == > cos */
    uint32               pri_color2dscp[GLB_QOS_PRI_COLOR_TABLE_SIZE];
    uint32               pri_color2exp[GLB_QOS_PRI_COLOR_TABLE_SIZE];
    uint32               phb2cos[GLB_QOS_PHB_MAX]; /* PHB ==> COS */
    uint32               phb2dscp[GLB_QOS_PHB_MAX];
    uint32               phb2exp[GLB_QOS_PHB_MAX];
    uint32               cos2tc[GLB_QOS_COS_NUM];
    uint32               dscp2tc[GLB_QOS_DSCP_NUM];
    uint32               exp2tc[GLB_QOS_EXP_NUM];
    uint32               tc_color2cos[GLB_QOS_TC_COLOR_TABLE_SIZE]; /* tc << 2 + color == > cos */
    uint32               tc_color2dscp[GLB_QOS_TC_COLOR_TABLE_SIZE];
    uint32               tc_color2exp[GLB_QOS_TC_COLOR_TABLE_SIZE];
} tbl_qos_domain_t;

typedef struct
{
    tbl_qos_domain_t     *domain_array[GLB_QOS_DOMAIN_NUM];
    tbl_qos_domain_t     *default_domain;     /* store default mapping info */
} tbl_qos_domain_master_t;

#endif /* !__TBL_QOS_DOMAIN_DEFINE_H__ */

