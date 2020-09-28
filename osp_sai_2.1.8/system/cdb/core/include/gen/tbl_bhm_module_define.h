
#ifndef __TBL_BHM_MODULE_DEFINE_H__
#define __TBL_BHM_MODULE_DEFINE_H__

/* TBL_BHM_MODULE field defines */
typedef enum
{
    TBL_BHM_MODULE_FLD_KEY                  = 0 ,  /* READ */
    TBL_BHM_MODULE_FLD_RUNNING              = 1 ,  /* READ */
    TBL_BHM_MODULE_FLD_PID                  = 2 ,  /* READ */
    TBL_BHM_MODULE_FLD_DEATH_COUNT          = 3 ,  /* READ */
    TBL_BHM_MODULE_FLD_MAX                  = 4 
} tbl_bhm_module_field_id_t;

/* TBL_BHM_MODULE defines */
typedef struct
{
    uint32               pm_id;
} tbl_bhm_module_key_t;

typedef struct
{
    tbl_bhm_module_key_t key;
    uint32               running;             /* is running */
    uint32               pid;                 /* process ID */
    uint32               death_count;         /* losed keepalive count */
} tbl_bhm_module_t;

typedef struct
{
    tbl_bhm_module_t     *pm_array[PM_ID_EXT_MAX];
} tbl_bhm_module_master_t;

#endif /* !__TBL_BHM_MODULE_DEFINE_H__ */

