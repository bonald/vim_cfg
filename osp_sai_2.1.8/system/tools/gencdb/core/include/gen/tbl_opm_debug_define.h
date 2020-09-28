
#ifndef __TBL_OPM_DEBUG_DEFINE_H__
#define __TBL_OPM_DEBUG_DEFINE_H__

/* TBL_OPM_DEBUG field defines */
typedef enum
{
    TBL_OPM_DEBUG_FLD_ERPS_ALL             = 0 ,  /* RW */
    TBL_OPM_DEBUG_FLD_ERPS_PACKET          = 1 ,  /* RW */
    TBL_OPM_DEBUG_FLD_ERPS_TIMER           = 2 ,  /* RW */
    TBL_OPM_DEBUG_FLD_ERPS_PROTOCOL        = 3 ,  /* RW */
    TBL_OPM_DEBUG_FLD_ERPS_EVENT           = 4 ,  /* RW */
    TBL_OPM_DEBUG_FLD_G8131_TX             = 5 ,  /* RW */
    TBL_OPM_DEBUG_FLD_G8131_RX             = 6 ,  /* RW */
    TBL_OPM_DEBUG_FLD_G8131_EVENT          = 7 ,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_LB_LBM_DEBUG   = 8 ,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_LB_LBR_DEBUG   = 9 ,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_LM_COMM_DEBUG  = 10,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_LM_LMM_DEBUG   = 11,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_LM_LMR_DEBUG   = 12,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_DM_COMM_DEBUG  = 13,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_DM_DMM_DEBUG   = 14,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_DM_DMR_DEBUG   = 15,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMTX_DEBUG = 16,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMRX_DEBUG = 17,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_PACKET_RX_DEBUG = 18,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_PACKET_TX_DEBUG = 19,  /* RW */
    TBL_OPM_DEBUG_FLD_TPOAM_EVENT_DEBUG    = 20,  /* RW */
    TBL_OPM_DEBUG_FLD_G8032_ALL            = 21,  /* RW */
    TBL_OPM_DEBUG_FLD_G8032_PACKET_TX      = 22,  /* RW */
    TBL_OPM_DEBUG_FLD_G8032_PACKET_RX      = 23,  /* RW */
    TBL_OPM_DEBUG_FLD_G8032_EVENT          = 24,  /* RW */
    TBL_OPM_DEBUG_FLD_G8032_TIMER          = 25,  /* RW */
    TBL_OPM_DEBUG_FLD_CFM_ALL              = 26,  /* RW */
    TBL_OPM_DEBUG_FLD_CFM_PACKET           = 27,  /* RW */
    TBL_OPM_DEBUG_FLD_CFM_TIMER            = 28,  /* RW */
    TBL_OPM_DEBUG_FLD_CFM_EVENT            = 29,  /* RW */
    TBL_OPM_DEBUG_FLD_MAX                  = 30
} tbl_opm_debug_field_id_t;

/* TBL_OPM_DEBUG defines */
typedef struct
{
    uint32               erps;                /* bitmap of OPMDBG_FLAG_ERPS_ */
    uint32               g8131;               /* bitmap of OPMDBG_FLAG_G8131_ */
    uint32               tpoam;               /* bitmap of OPMDBG_FLAG_TPOAM_ */
    uint32               g8032;               /* bitmap of OPMDBG_FLAG_G802_ */
    uint32               cfm;                 /* bitmap of OPMDBG_FLAG_CFM_ */
} tbl_opm_debug_t;

#endif /* !__TBL_OPM_DEBUG_DEFINE_H__ */

