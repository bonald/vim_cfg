#include "sal.h"
#include "glb_macro.h"
#include "glb_const.h"
#include "proto.h"
#include "glb_openflow_define.h"
#include "lib.h"

#include "fei.h"

#include "hsrv_msg.h"
#include "hsrv_msg_openflow.h"

#include "fei_openflow.h"

int32
fei_openflow_post_sys_startup()
{
    fei_openflow_post_sys_startup_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_post_sys_startup_t));

    return fei_talk(FEI_OPENFLOW_POST_SYS_STARTUP, &req, sizeof(req));
}

int32
fei_openflow_do_barrier()
{
    fei_openflow_do_barrier_t req;
    int32 rc = 0;
    
    sal_memset(&req, 0, sizeof(fei_openflow_do_barrier_t));

    /* Must use normal socket which is used by other message sent by ovs-vswitchd to fea, only in this case
       barrier is REAL barrier. */
    return fei_talk_sync_for_openflow_barrier(FEI_OPENFLOW_DO_BARRIER, &req, sizeof(req), &rc, sizeof(rc));
}

int32
fei_openflow_port_do_barrier_by_ovs(uint32 ifindex)
{
    fei_openflow_port_do_barrier_t req;
    int32 is_ready = 0;

    sal_memset(&req, 0, sizeof(fei_openflow_port_do_barrier_t));
    req.ifindex = ifindex;

    
    fei_talk_sync_for_openflow_barrier(FEI_OPENFLOW_DO_PORT_BARRIER_BY_OVS, &req, sizeof(req), &is_ready , sizeof(is_ready ));

    return is_ready;
}

int32
fei_openflow_port_do_barrier_by_switch(uint32 ifindex)
{
    fei_openflow_port_do_barrier_t req;
    int32 is_ready = 0;

    sal_memset(&req, 0, sizeof(fei_openflow_port_do_barrier_t));
    req.ifindex = ifindex;

    
    fei_talk_sync_for_openflow_barrier(FEI_OPENFLOW_DO_PORT_BARRIER_BY_SWITCH, &req, sizeof(req), &is_ready , sizeof(is_ready ));

    return is_ready;
}

