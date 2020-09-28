
#ifndef __TBL_OPENFLOW_DEBUG_DEFINE_H__
#define __TBL_OPENFLOW_DEBUG_DEFINE_H__

/* TBL_OPENFLOW_DEBUG field defines */
typedef enum
{
    TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_VCONN       = 0 ,  /* RW */
    TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_RCONN       = 1 ,  /* RW */
    TBL_OPENFLOW_DEBUG_FLD_MAX                  = 2 
} tbl_openflow_debug_field_id_t;

/* TBL_OPENFLOW_DEBUG defines */
typedef struct
{
    uint32               openflow;            /* bitmap of OPENFLOWDBG_FLAG_ */
} tbl_openflow_debug_t;

#endif /* !__TBL_OPENFLOW_DEBUG_DEFINE_H__ */

