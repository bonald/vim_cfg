
#include "proto.h"
#include "snmpagt_define.h"
#include "snmpagt.h"

struct counter64
snmpagt_u64_to_c64(uint64 u64)
{
    struct counter64 c64;

    c64.high = ((u64 >> 32) & 0xFFFFFFFF);    
    c64.low  = (u64 & 0xFFFFFFFF);
    return c64;
}

int32
snmpagt_init(uint32 debug)
{
    int32 rc = PM_E_NONE;

    /* 1. init lib and task */
    lib_init(PM_ID_CTL);
    ctc_task_init();

    snmpagt_agent_init();

    snmpagt_sub_init(1);
    ctc_task_main();
    exit(rc);
}

