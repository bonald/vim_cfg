
#include "proto.h"
#include "lib_tblinc.h"
#include <sys/mman.h>
#include <sys/dir.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "sal.h"
#include "genlog.h"
#include "ctc_task.h"
#include "cdb_const.h"
#include "lib_syslimit.h"
#include "glb_if_define.h"
#include "glb_ip_define.h"
#include "glb_tempfile_define.h"
#include "cfg_cmd.h"
#include "proto_error.h"

int32
syslimit_memory_threshold_check(syslimit_memchk_state_t memchk_state)
{
    tbl_sys_global_t *p_sys_glb = NULL;
    p_sys_glb = tbl_sys_global_get_sys_global();
    if (p_sys_glb == NULL)
    {
        return -1;
    }
    switch (memchk_state)
    {
    case SYSLIMIT_MEMCHECK_STATE_CRITICAL:
        if (p_sys_glb->system_memchk_state >= SYSLIMIT_MEMCHECK_STATE_CRITICAL)
        {
            return -1;
        }
        else
        {
            return 0;
        }

    case SYSLIMIT_MEMCHECK_STATE_ALERT:
        if (p_sys_glb->system_memchk_state >= SYSLIMIT_MEMCHECK_STATE_ALERT)
        {
            return -1;
        }
        else
        {
            return 0;
        }

    case SYSLIMIT_MEMCHECK_STATE_WARNING:
        if (p_sys_glb->system_memchk_state >= SYSLIMIT_MEMCHECK_STATE_WARNING)
        {
            return -1;
        }
        else
        {
            return 0;
        }

    case SYSLIMIT_MEMCHECK_STATE_NORMAL:
        if (p_sys_glb->system_memchk_state <= SYSLIMIT_MEMCHECK_STATE_NORMAL)
        {
            return 0;
        }
        
    default:
        return -1;
    }
}

