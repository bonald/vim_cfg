
#ifndef __TBL_SSM_DEBUG_DEFINE_H__
#define __TBL_SSM_DEBUG_DEFINE_H__

/* TBL_SSM_DEBUG field defines */
typedef enum
{
    TBL_SSM_DEBUG_FLD_SSM_EVENT            = 0 ,  /* RW */
    TBL_SSM_DEBUG_FLD_SSM_PACKET_TX        = 1 ,  /* RW */
    TBL_SSM_DEBUG_FLD_SSM_PACKET_RX        = 2 ,  /* RW */
    TBL_SSM_DEBUG_FLD_SSM_TIMER            = 3 ,  /* RW */
    TBL_SSM_DEBUG_FLD_MAX                  = 4 
} tbl_ssm_debug_field_id_t;

/* TBL_SSM_DEBUG defines */
typedef struct
{
    uint32               ssm;                 /* bitmap of SSM_DEBUG_FLAG_ */
} tbl_ssm_debug_t;

#endif /* !__TBL_SSM_DEBUG_DEFINE_H__ */

