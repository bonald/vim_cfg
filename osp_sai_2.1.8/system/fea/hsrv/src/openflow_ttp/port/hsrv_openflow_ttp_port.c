
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_port.h"
#include "hsrv_openflow_ttp_port_db.h"

#define ____EXTERNAL_API_____

hsrv_openflow_ttp_port_info_t*
hsrv_openflow_port_get_port_info_by_ifindex(uint32 ifindex)
{
    hsrv_openflow_ttp_port_info_t* p_port = NULL;
    hsrv_openflow_portdb_lookup_port(ifindex, &p_port);

    return p_port;
}

hsrv_openflow_ttp_port_info_t*
hsrv_openflow_port_get_port_info_by_portid(uint32 gport)
{
    hsrv_openflow_ttp_port_info_t* p_port = NULL;
    uint32 ifindex = 0;
    hsrv_ifdb_get_ifindex_by_gportid(gport, &ifindex);
    hsrv_openflow_portdb_lookup_port(ifindex, &p_port);

    return p_port;
}

#define ____INTERNAL_API_____

static int32
hsrv_openflow_port_enable(uint32 ifindex)
{
    uint16 gport = 0;
    uint32 group_id = 0;
    uint32 class_id = 0;
    hsrv_openflow_ttp_port_info_t *p_port = NULL;
    
    hsrv_openflow_portdb_lookup_port(ifindex, &p_port);
    if (p_port)
    {
        return HSRV_E_NONE;
    }

    p_port = HSRV_OPENFLOW_PORT_MALLOC(sizeof(hsrv_openflow_ttp_port_info_t));
    HSRV_OPENFLOW_MEM_CHECK(p_port);
    sal_memset(p_port, 0, sizeof(hsrv_openflow_ttp_port_info_t));
    
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_label(ifindex, &class_id));
    group_id = _hsrv_openflow_get_group_id_by_ifindex_class_id(ifindex, class_id, CTC_INGRESS);
    p_port->ifindex = ifindex;
    p_port->gport = gport;
    p_port->group_id = group_id;
    p_port->class_id = class_id;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_portdb_add_port(p_port));

    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_port_enable(ifindex));
    
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_port_disable(uint32 ifindex)
{
    hsrv_openflow_ttp_port_info_t *p_port = NULL;
    
    hsrv_openflow_portdb_lookup_port(ifindex, &p_port);
    if (!p_port)
    {
        return HSRV_E_NONE;
    }

    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_port_disable(ifindex));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_portdb_del_port(ifindex));
    
    return HSRV_E_NONE;
}

#define _____HAL_APIs_____

int32 
fei_api_openflow_port_enable(fei_openflow_port_enable_t *pst_req)
{
    if (pst_req->enable)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_port_enable(pst_req->ifindex));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_port_disable(pst_req->ifindex));
    }
    
    return HSRV_E_NONE;
}


#define _____INIT_____
int32
hsrv_openflow_port_start(void)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_portdb_start());
    return HSRV_E_NONE;
}


