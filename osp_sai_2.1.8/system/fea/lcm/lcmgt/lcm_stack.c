/**************************************************************************
* lcm_stack.c   :   lcm stack api
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* 
* Revision  :        R0.1
* Author    :        tongzb
* Date      :        2019 - 09 - 28
* Reason    :        First Create.
**************************************************************************/
#include "sal_common.h"
#include "glb_distribute_system_define.h"
#if 0
#include "stack.h"   
#else
uint8 __attribute__((weak)) stack_is_enable()
{
    static int enable = -1;
    struct stat stat_buf;

    if (enable != -1)
    {
        return enable;
    }

    enable = !stat(GLB_STACK_ENABLE_FILE, &stat_buf);
    return enable;
}

uint16 __attribute__((weak)) stack_self_slotid()
{
    FILE *fp;
    char buf[BUFSIZ];
    static int slot = -1;

    if (slot != -1)
    {
        return slot;
    }

    slot = 1;
    fp = sal_fopen(GLB_SLOT_NO_FILE, "r");
    if (fp)
    {
        fgets (buf, BUFSIZ, fp);
        slot = atoi(buf);
        sal_fclose(fp);
    }    

    return slot; 
}

uint16 __attribute__((weak)) stack_master_slotid()
{
    FILE *fp;
    char buf[BUFSIZ];
    static int slot = -1;

    if (slot != -1)
    {
        return slot;
    }

    slot = 1;
    fp = sal_fopen(GLB_STACK_MASTER_SLOT_FILE, "r");
    if (fp)
    {
        fgets (buf, BUFSIZ, fp);
        slot = atoi(buf);
        sal_fclose(fp);
    }    

    return slot;
}
#endif

int32
lcm_stack_is_enable()
{
    return stack_is_enable();
}

int32
lcm_stack_self_slotid()
{
    return stack_self_slotid();
}

int32
lcm_stack_master_slotid()
{
    return stack_self_slotid();
}


