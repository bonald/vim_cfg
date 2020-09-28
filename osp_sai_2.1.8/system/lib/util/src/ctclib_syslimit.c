#if 0
/****************************************************************************
 * ctclib_syslimit.c
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       weij
 * Date         :       2012-04-13
 * Reason       :       First Create.
 ****************************************************************************/
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "sal.h"
#include "genlog.h"
#include "ctclib_syslimit.h"


#define MEMCHECK_THRESH_EXCEED_STR1 \
    "Exceed system memory threshold, all memories are exhausted"
#define MEMCHECK_THRESH_EXCEED_STR2 \
    "Exceed system memory threshold, no free memory"
#define MEMCHECK_THRESH_EXCEED_STR3 \
    "Exceed system memory threshold, free memory is less than [%d]M bytes"

static syslimit_memchk_state_t syslimit_memcheck_status = 0;
static int32 *syslimit_memcheck_current_status_ptr = (int*)&syslimit_memcheck_status;
static int32 ctc_memcheck_left_free = 0;

int32
syslimit_mem_get_cur_threshold()
{
    return *syslimit_memcheck_current_status_ptr;
}

/*********************************************************************
 * Name    : _syslimit_mem_threshold_check
 * Purpose : This function is used to check memery limits
 * Input   : memery threshold
 * Output  : none
 * Return  : -1:check fail(exceed threshokd) 0:check pass(still have free memery)
 * Note    : 
*********************************************************************/
static int32
_syslimit_mem_threshold_check(int32 threshold)
{
    switch (threshold)
    {
    case SYSLIMIT_MEMCHECK_STATE_CRITICAL:
        /*SYSLIMIT_MEMCHECK_STATE_CRITICAL,    (current memory < threshold0) */
        if (*syslimit_memcheck_current_status_ptr <= SYSLIMIT_MEMCHECK_STATE_CRITICAL )
        {
            return -1;
        }
        break;
       
    case SYSLIMIT_MEMCHECK_STATE_ALERT:
        /*SYSLIMIT_MEMCHECK_STATE_CRITICAL or SYSLIMIT_MEMCHECK_STATE_ALERT,    (current memory < threshold1) */
        if (*syslimit_memcheck_current_status_ptr <= SYSLIMIT_MEMCHECK_STATE_ALERT )
        {
            return -1;
        }
        break;
      
    case SYSLIMIT_MEMCHECK_STATE_WARNING:
        /*SYSLIMIT_MEMCHECK_STATE_CRITICAL or SYSLIMIT_MEMCHECK_STATE_ALERT or SYSLIMIT_MEMCHECK_STATE_WARNING */
        /* (current memory < threshold1) */
        if (*syslimit_memcheck_current_status_ptr <= SYSLIMIT_MEMCHECK_STATE_WARNING )
        {
            return -1;
        }
        break;

    case SYSLIMIT_MEMCHECK_STATE_NORMAL:
        /*SYSLIMIT_MEMCHECK_STATE_CRITICAL or SYSLIMIT_MEMCHECK_STATE_ALERT or SYSLIMIT_MEMCHECK_STATE_WARNING */
        /* (current memory < threshold1) */
        return 0;

    default:
        return -1;
    }
    
    return 0;
}

/*********************************************************************
 * Name    : syslimits_check
 * Purpose : This function is used to check all system limits
 * Input   : ctc_sys_limit_t
 * Output  : none
 * Return  : system limits flag
 * Note    : support memery and message limits check
*********************************************************************/
syslimit_flags_t
syslimits_check(ctc_sys_limit_t *pcsl)
{
    syslimit_flags_t ret = 0;
    if (!pcsl)
    {
        return 0;
    }
    
    /* need to check memery*/
    if ((pcsl->flags & SYSLIMIT_TYPE_MEM) == SYSLIMIT_TYPE_MEM)
    {
        if (_syslimit_mem_threshold_check(pcsl->mem_state))
        {
            ret = ret | SYSLIMIT_TYPE_MEM;
        }
    }
    /* need to check message*/
    if ((pcsl->flags & SYSLIMIT_TYPE_MSG) == SYSLIMIT_TYPE_MSG)
    {
        // TODO:
    }
    return ret;
}

syslimit_flags_t 
syslimit_mem_threshold_check(int threshold, uint32 eModule, char *descr)
{
    syslimit_flags_t ret = 0;
    ctc_sys_limit_t sys_limit;
    sal_memset(&sys_limit, 0, sizeof(ctc_sys_limit_t));
    sys_limit.flags = sys_limit.flags | SYSLIMIT_TYPE_MEM;
    sys_limit.mem_state = threshold;
    
    ret = syslimits_check(&sys_limit);
    if ((ret & SYSLIMIT_TYPE_MEM) == SYSLIMIT_TYPE_MEM)
    {
        switch(sys_limit.mem_state)
        {
        case SYSLIMIT_MEMCHECK_STATE_CRITICAL:
            if (descr)
            {
                log_sys(eModule, E_ERROR, "[%s] "MEMCHECK_THRESH_EXCEED_STR1, descr);
            }
            else
            {
                log_sys(eModule, E_ERROR, MEMCHECK_THRESH_EXCEED_STR1);
            }
            return -1;
   
        case SYSLIMIT_MEMCHECK_STATE_ALERT:
            if (descr)
            {
                log_sys(eModule, E_ERROR, "[%s] "MEMCHECK_THRESH_EXCEED_STR2, descr);
            }
            else
            {
                log_sys(eModule, E_ERROR, MEMCHECK_THRESH_EXCEED_STR2);
            }
            return -1;
  
        case SYSLIMIT_MEMCHECK_STATE_WARNING:
            if (descr)
            {
                log_sys(eModule, E_ERROR, "[%s] "MEMCHECK_THRESH_EXCEED_STR3, descr, ctc_memcheck_left_free);
            }
            else
            {
                log_sys(eModule, E_ERROR, MEMCHECK_THRESH_EXCEED_STR3, ctc_memcheck_left_free);
            }
            return -1;

        case SYSLIMIT_MEMCHECK_STATE_NORMAL:
            /*SYSLIMIT_MEMCHECK_STATE_CRITICAL or SYSLIMIT_MEMCHECK_STATE_ALERT or SYSLIMIT_MEMCHECK_STATE_WARNING */
            /* (current memory < threshold1) */
            return 0;
            
        default:
            return -1;
        }
    }
    
    return 0;
}
#endif

