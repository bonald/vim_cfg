#include "sal.h"
#include "glb_macro.h"
#include "proto.h"

#include "glb_const.h"
#include "glb_openflow_define.h"
#include "lib.h"

#include "fei.h"

#include "hsrv_msg.h"
#include "hsrv_msg_openflow.h"

int32
fei_openflow_bond_create(uint32 ifindex, bool enable)
{
    fei_openflow_bond_create_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_bond_create_t));
    req.bond_ifindex = ifindex;
    req.enable = enable;
    
    return fei_talk(FEI_OPENFLOW_BOND_CREATE, &req, sizeof(req));
}

int32
fei_openflow_bond_update_slave_state(uint32 bond_ifindex, uint32 slave_ifindex, bool enable)
{
    fei_openflow_bond_update_slave_t req;
    
    sal_memset(&req, 0, sizeof(fei_openflow_bond_update_slave_t));
    req.bond_ifindex = bond_ifindex;
    req.slave_ifindex = slave_ifindex;
    req.enable = enable;
    
    return fei_talk(FEI_OPENFLOW_BOND_UPDATE_STATE, &req, sizeof(req));
}

int32
fei_openflow_bond_attach_slave(uint32 bond_ifindex, uint32 slave_ifindex)
{
    fei_openflow_bond_attach_slave_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_bond_attach_slave_t));
    req.bond_ifindex = bond_ifindex;
    req.slave_ifindex = slave_ifindex;
        
    return fei_talk(FEI_OPENFLOW_BOND_ATTACH_SLAVE, &req, sizeof(req));
}

int32
fei_openflow_set_bond_mac(uint32 bond_ifindex, uint8* mac)
{
    fei_openflow_bond_set_mac_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_bond_set_mac_t));
    req.bond_ifindex = bond_ifindex;
    sal_memcpy(req.mac, mac, 6*sizeof(uint8));
        
    return fei_talk(FEI_OPENFLOW_BOND_SET_MAC, &req, sizeof(req));
}

