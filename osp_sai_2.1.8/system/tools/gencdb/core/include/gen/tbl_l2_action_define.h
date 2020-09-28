
#ifndef __TBL_L2_ACTION_DEFINE_H__
#define __TBL_L2_ACTION_DEFINE_H__

#include "gen/ds_flush_fdb_define.h"

/* TBL_L2_ACTION field defines */
typedef enum
{
    TBL_L2_ACTION_FLD_FLUSH_FDB            = 0 ,  /* SUB */
    TBL_L2_ACTION_FLD_MAX                  = 1 
} tbl_l2_action_field_id_t;

/* TBL_L2_ACTION defines */
typedef struct
{
    ds_flush_fdb_t       *flush_fdb;
} tbl_l2_action_t;

#endif /* !__TBL_L2_ACTION_DEFINE_H__ */

