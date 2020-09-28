#ifndef __LCM_SPECIFIC__
#define __LCM_SPECIFIC__

/* include files difference */
#ifdef CNOS_OS
#include "glb_if_define.h"
#include "glb_phy_define.h"
#include "ctclib_memory.h"
#include "ctclib_if.h"
#include "ctc_task.h"
#include "ctc_sock.h"
#include <linux/if.h>
#else
#include "memory.h"
#include "memmgr.h"
#include "ctclib_thread.h"
#endif 

/* MEM define difference */
#ifdef CNOS_OS
#define CTCLIB_MEM_ALLOC_MODULE         MEM_FEA_LCM_MODULE
#define CTCLIB_MEM_LCM_MODULE           MEM_FEA_LCM_MODULE
#define CTCLIB_MEM_LCM_PORT             MEM_LCM_PORT
#define MTYPE_BUFFER_DATA               MEM_TEMP
#ifdef GOLDENGATE
#define _GOLDENGATE_
#endif
#define sal_sock_accept sal_accept
#define sal_sock_bind sal_bind
#define sal_sock_listen sal_listen
#endif 

/* Timer difference : in ctc_task.h and ctclib_thread.h e.g. CTC_TASK_ADD_TIME */
#ifndef CNOS_OS
ctclib_thread_master_t* lc_get_thread_master(void);   
#define _GOLDENGATE_
#endif

#ifdef CNOS_OS
#define LCM_DECLARE_TASK_ENCAP(_FUNC_)
#define LCM_DEFINE_TASK_ENCAP(_FUNC_)
#define LCM_TASK_FUNC(_FUNC_)       \
void _FUNC_(void * p_data)
#define LCM_TASK_RETURN(err)    return
#else
#define LCM_DECLARE_TASK_ENCAP(_FUNC_) \
int32 _FUNC_##_encap(thread_t* p_thread);

#define LCM_DEFINE_TASK_ENCAP(_FUNC_) \
int32 _FUNC_##_encap(thread_t* p_thread) \
{ \
    _FUNC_(p_thread->arg); \
    return 0; \
}
#define LCM_TASK_FUNC(_FUNC_)       \
int32 _FUNC_(thread_t * p_data)

#define LCM_TASK_RETURN(err)    return err
#endif

#endif /* !__LCM_SPECIFIC__ */
