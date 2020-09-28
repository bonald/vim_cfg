
#ifndef __PROTO_DEBUG_H__
#define __PROTO_DEBUG_H__

#include "gen/tbl_switch_debug_define.h"
#include "gen/tbl_chsm_debug_define.h"

extern tbl_switch_debug_t *_g_p_tbl_switch_debug;
extern tbl_chsm_debug_t *_g_p_tbl_chsm_debug;

/* debug chsm judgment*/
#define CHSM_DBG_IS_ON(module, typeenum) \
    (_g_p_tbl_chsm_debug->module & CHSMDBG_FLAG_##typeenum)

#define CHSM_DEBUG_PRINT(level, fmt, args...)   \
    log_sys(M_MOD_##level, E_DEBUG, fmt, ##args);

#define CHSM_DEBUG_OUT_INFO(level, mod, typeenum, fmt, args...)  \
do                                                      \
{                                                       \
    if (CHSM_DBG_IS_ON(mod, typeenum))                  \
    {                                                   \
        CHSM_DEBUG_PRINT(level, fmt, ##args);           \
    }                                                   \
}                                                       \
while (0)    

/* debug switch judgment*/
#define PM_DBG_IS_ON(module, typeenum) \
    (_g_p_tbl_switch_debug->module & SWTHDBG_FLAG_##typeenum)

#define PM_DEBUG_PRINT(level, fmt, args...)   \
    log_sys(M_MOD_##level, E_DEBUG, fmt, ##args);

#define PM_DEBUG_OUT_INFO(level, mod, typeenum, fmt, args...)  \
do                                                      \
{                                                       \
    if (PM_DBG_IS_ON(mod, typeenum))                    \
    {                                                   \
        PM_DEBUG_PRINT(level, fmt, ##args);             \
    }                                                   \
}                                                       \
while (0)
    
#if 0
/* debug switch judgment*/
#define CTCLIB_IS_DEBUG_ON(level, mod, sub, typeenum)              \
((typeenum < typeenum_##level##_##mod##_##sub##_max)              \
    && (g_st_##level##_##mod##_##sub##_debug_handle.flags & (1 << (typeenum))) == (1 << (typeenum)))

#endif
#endif /* !__PROTO_DEBUG_H__ */

