#ifndef __COLD_H__
#define __COLD_H__


#define COLD_MAX_ITEM_NAME_LEN         31
#define COLD_TIME_DESC                 32
#define COLD_MAX_BAK_LOG_NUM           255
#define COLD_NULL_CHECK(ptr) ((ptr) == NULL)
#define COLD_PTR_VALID_CHECK(ptr)\
if ( NULL == (ptr) )\
{\
    return PM_E_INVALID_PARAM;\
}

#define COLD_HM_CPU_THRESHOLD   75  /* percent */
#define COLD_HM_MEM_THRESHOLD   75  /* percent */

#define COLD_FLAG_SET(V, F) 	    ((V) = (V) | (F))
#define COLD_FLAG_UNSET(V, F)   	((V) = (V) & ~(F))
#define COLD_FLAG_CHECK(V, F)     ((V) & (F))

typedef struct cold_cfg_s
{
    uint8 module_enable; 
    uint8 log_bak_num; 
    uint8 last_bak_index; 
    uint8 reserved;
} cold_cfg_t;

int32 cold_item_sys_info_monitor(void);
bool cold_init(void);
#endif
