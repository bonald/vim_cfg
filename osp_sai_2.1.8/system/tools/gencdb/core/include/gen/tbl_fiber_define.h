
#ifndef __TBL_FIBER_DEFINE_H__
#define __TBL_FIBER_DEFINE_H__

/* TBL_FIBER field defines */
typedef enum
{
    TBL_FIBER_FLD_KEY                  = 0 ,  /* READ */
    TBL_FIBER_FLD_INTERFACE_NAME       = 1 ,  /* READ */
    TBL_FIBER_FLD_FIBER_TYPE_NAME      = 2 ,  /* READ */
    TBL_FIBER_FLD_NAME                 = 3 ,  /* READ */
    TBL_FIBER_FLD_OUI                  = 4 ,  /* READ */
    TBL_FIBER_FLD_PN                   = 5 ,  /* READ */
    TBL_FIBER_FLD_REV                  = 6 ,  /* READ */
    TBL_FIBER_FLD_SN                   = 7 ,  /* READ */
    TBL_FIBER_FLD_COMPLIANCE_CODE      = 8 ,  /* READ */
    TBL_FIBER_FLD_SLOT                 = 9 ,  /* READ */
    TBL_FIBER_FLD_TMPR                 = 10,  /* RW */
    TBL_FIBER_FLD_VOLTAGE              = 11,  /* RW */
    TBL_FIBER_FLD_BIAS                 = 12,  /* RW */
    TBL_FIBER_FLD_TX_PWR               = 13,  /* RW */
    TBL_FIBER_FLD_RX_PWR               = 14,  /* RW */
    TBL_FIBER_FLD_TMPR2                = 15,  /* RW */
    TBL_FIBER_FLD_VOLTAGE2             = 16,  /* RW */
    TBL_FIBER_FLD_BIAS2                = 17,  /* RW */
    TBL_FIBER_FLD_TX_PWR2              = 18,  /* RW */
    TBL_FIBER_FLD_RX_PWR2              = 19,  /* RW */
    TBL_FIBER_FLD_ALARM_FLAG           = 20,  /* RW */
    TBL_FIBER_FLD_WARN_FLAG            = 21,  /* RW */
    TBL_FIBER_FLD_UPDATE_CNT           = 22,  /* RW */
    TBL_FIBER_FLD_CHANNEL_NUM          = 23,  /* RW */
    TBL_FIBER_FLD_CHANNEL_IDX          = 24,  /* RW */
    TBL_FIBER_FLD_IS_DETAIL            = 25,  /* RW */
    TBL_FIBER_FLD_FIBER_TYPE           = 26,  /* READ */
    TBL_FIBER_FLD_FIBER_FLAG           = 27,  /* READ */
    TBL_FIBER_FLD_IS_COPPER            = 28,  /* READ */
    TBL_FIBER_FLD_SPEED                = 29,  /* READ */
    TBL_FIBER_FLD_DDM_SUPPORT          = 30,  /* READ */
    TBL_FIBER_FLD_EXTERNALLY_CALIBRATED = 31,  /* READ */
    TBL_FIBER_FLD_WAVELENGTH           = 32,  /* RW */
    TBL_FIBER_FLD_LENGTH               = 33,  /* RW */
    TBL_FIBER_FLD_MAX                  = 34
} tbl_fiber_field_id_t;

/* TBL_FIBER defines */
typedef struct
{
    int32                port_id;
} tbl_fiber_key_t;

typedef struct
{
    tbl_fiber_key_t      key;
    char                 interface_name[IFNAME_SIZE];
    char                 fiber_type_name[FIBER_TRANS_TYPE_LEN];
    char                 name[FIBER_VENDOR_NAME_LEN+1]; /*SFP vendor name (ASCII)*/
    uint8                oui[FIBER_VENDOR_OUI_LEN+1]; /*SFP vendor IEEE company ID*/
    char                 pn[FIBER_VENDOR_PN_LEN+1]; /*Part number provided by SFP vendor (ASCII)*/
    char                 rev[FIBER_VENDOR_REV_LEN+1]; /*Revision level for part number provided by vendor (ASCII)*/
    char                 sn[FIBER_VENDOR_SN_LEN+1]; /*Serial number provided by vendor (ASCII)*/
    uint8                compliance_code[FIBER_COMPLIANCE_CODE_LEN]; /*complianceCode */
    int32                slot;
    double               tmpr[4];             /*high/low alarm , high/low warn*/
    double               voltage[4];
    double               bias[4];
    double               tx_pwr[4];
    double               rx_pwr[4];
    double               tmpr2[4];            /*real value, max 4*/
    double               voltage2[4];
    double               bias2[4];
    double               tx_pwr2[4];
    double               rx_pwr2[4];
    uint32               alarm_flag[4];       /*store alarm info*/
    uint32               warn_flag[4];        /*store warn info*/
    uint32               update_cnt;
    uint32               channel_num;
    uint32               channel_idx;
    uint32               is_detail;
    uint32               fiber_type;          /*glb_phy_type_t*/
    uint32               fiber_flag;
    uint32               is_copper;
    uint32               speed;               /*SFP speed = speed * 100Mbit/sec */
    uint32               ddm_support;         /*is support DDM?*/
    uint32               externally_calibrated; /*if support DDM, use internally calibrated or externally calibrated*/
    uint8                wavelength[2];       /*output wavelength*/
    uint8                length[5];           /*length */
} tbl_fiber_t;

typedef struct
{
    tbl_fiber_t          *fiber[GLB_CHSM_PORT_NUM_MAX];
} tbl_fiber_master_t;

#endif /* !__TBL_FIBER_DEFINE_H__ */

