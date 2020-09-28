#include "sal.h"
#include "glb_macro.h"
#include "proto.h"

#include "glb_const.h"
#include "glb_openflow_define.h"
#include "lib.h"

#include "fei.h"

#include "hsrv_msg.h"
#include "hsrv_msg_openflow.h"


extern int32
fei_openflow_port_do_barrier_by_switch(uint32 ifindex);

#if 0
static int32 fei_openflow_port_wait_exist_by_switch(uint32 ifindex)
{
    uint32 cnt = 0;
    while(1)
    {
        if(fei_openflow_port_do_barrier_by_switch(ifindex))
        {
            return 1;
        }
        cnt ++;
        if(100 == cnt)
        {
            return 0;
        }
        usleep(1000000);
    }

    return 0;
}
#endif

int32
fei_openflow_port_notify_link(uint32 ifindex, uint32 enable)
{
    fei_openflow_port_notify_link_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_port_notify_link_t));
    req.ifindex = ifindex;
    req.enable = enable;
   
    return fei_talk(FEI_OPENFLOW_PORT_NOTIFY_LINK, &req, sizeof(req));
}

int32
fei_openflow_port_set_config(uint32 ifindex, uint32 port_config)
{
    fei_openflow_port_set_config_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_port_set_config_t));
    req.ifindex = ifindex;
    req.port_config = port_config;
    
    return fei_talk(FEI_OPENFLOW_PORT_SET_CONFIG, &req, sizeof(req));
}

int32
fei_openflow_add_port_to_bridge(uint32 ifindex, uint32 flag, bool enable)
{
    fei_openflow_port_add_to_bridge_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_update_all_action_list_req_t));
    req.ifindex = ifindex;
    req.flag = flag;
    req.enable = enable;
    
    return fei_talk(FEI_OPENFLOW_PORT_ADD_TO_BRIDGE, &req, sizeof(req));
}

