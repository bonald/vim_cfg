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
fei_openflow_stub_create(uint32 ifindex)
{
    fei_openflow_stub_create_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_stub_create_t));
    req.stub_ifindex = ifindex;
    req.enable = TRUE;
    
    return fei_talk(FEI_OPENFLOW_STUB_CREATE, &req, sizeof(req));
}

int32
fei_openflow_stub_destroy(uint32 ifindex)
{
    fei_openflow_stub_create_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_stub_create_t));
    req.stub_ifindex = ifindex;
    req.enable = FALSE;
    
    return fei_talk(FEI_OPENFLOW_STUB_CREATE, &req, sizeof(req));
}

int32
fei_openflow_stub_enable(uint32 ifindex, bool enable)
{
    fei_openflow_stub_enable_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_stub_enable_t));
    req.stub_ifindex = ifindex;
    req.enable = enable;
    
    return fei_talk(FEI_OPENFLOW_STUB_ENABLE, &req, sizeof(req));
}

