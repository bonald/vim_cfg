#ifndef _LIB_SYSLIMIT_H
#define _LIB_SYSLIMIT_H

enum syslimit_memchk_state_e
{
    SYSLIMIT_MEMCHECK_STATE_NORMAL,    /* > threshold2 */
    SYSLIMIT_MEMCHECK_STATE_WARNING,    /* > threshold1 & < threshold2 */
    SYSLIMIT_MEMCHECK_STATE_ALERT,    /* > threshold0 & < threshold1 */
    SYSLIMIT_MEMCHECK_STATE_CRITICAL,    /* < threshold0 */
};
typedef enum syslimit_memchk_state_e syslimit_memchk_state_t;

#define SYSTEM_MEM_DEFAULT_PECENT_THRESH1 (4)
#define SYSTEM_MEM_DEFAULT_PECENT_THRESH2 (6)
#define SYSTEM_MEM_DEFAULT_PECENT_THRESH3 (8)

int32
syslimit_memory_threshold_check(syslimit_memchk_state_t memchk_state);

#endif /*_LIB_SYSLIMIT_H*/
