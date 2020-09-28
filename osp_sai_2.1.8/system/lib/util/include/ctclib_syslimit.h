#if 0

#ifndef _CTCLIB_SYSLIMIT_H
#define _CTCLIB_SYSLIMIT_H


enum syslimit_memchk_state_e
{
    SYSLIMIT_MEMCHECK_STATE_NORMAL,    /* > threshold2 */
    SYSLIMIT_MEMCHECK_STATE_WARNING,    /* > threshold1 & < threshold2 */
    SYSLIMIT_MEMCHECK_STATE_ALERT,    /* > threshold0 & < threshold1 */
    SYSLIMIT_MEMCHECK_STATE_CRITICAL,    /* < threshold0 */
};
typedef enum syslimit_memchk_state_e syslimit_memchk_state_t;

enum syslimit_msgchk_state_e
{
    SYSLIMIT_MSGCHK_STATE_CRITICAL, /* queue length too long */
    SYSLIMIT_MSGCHK_STATE_ALERT, /* buffer overflow */
    SYSLIMIT_MSGCHK_STATE_WARNING, 
    SYSLIMIT_MSGCHK_STATE_NORMAL,
    SYSLIMIT_MSGCHK_STATE_INVALID   /* Invalid state */
};
typedef enum syslimit_msgchk_state_e syslimit_msgchk_state_t;

typedef u_int32_t syslimit_flags_t;
#define SYSLIMIT_TYPE_MEM      0x01
#define SYSLIMIT_TYPE_MSG      0x02
typedef struct 
{
    syslimit_flags_t flags;
    syslimit_memchk_state_t mem_state;
    syslimit_msgchk_state_t msg_state;
} ctc_sys_limit_t;

#define SYSLIMIT_MEM_DEFAULT_THRESH1 (10)
#define SYSLIMIT_MEM_DEFAULT_THRESH2 (20)
#define SYSLIMIT_MEM_DEFAULT_THRESH3 (30)

#define SYSLIMIT_MEMCHK_SHM_DEV_NAME    "/dev/ctc_memchk_one_page_shm"

int32 
syslimit_mem_get_cur_threshold();

syslimit_flags_t 
syslimit_mem_threshold_check(int threshold, uint32 eModule, char *descr);


#endif /*_CTCLIB_SYSLIMIT_H*/


#endif
