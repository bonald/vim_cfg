#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_port_db.h"

hsrv_openflow_ttp_port_master_t    *g_pst_openflow_ttp_port_master = NULL;

static inline uint32
_hsrv_openflow_portdb_port_make(hsrv_openflow_ttp_port_info_t* p_port)
{
    return p_port->ifindex;
}

static inline bool
_hsrv_openflow_portdb_port_compare(hsrv_openflow_ttp_port_info_t* pva, hsrv_openflow_ttp_port_info_t* pvb)
{
    if (pva->ifindex != pvb->ifindex)
    {
        return FALSE;
    }

    return TRUE;
}

int32
hsrv_openflow_portdb_add_port(hsrv_openflow_ttp_port_info_t* p_port)
{
    ctclib_hash_get(HSRV_OPENFLOW_TTP_PORT_HASH, p_port, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_portdb_del_port(uint32 ifindex)
{
    hsrv_openflow_ttp_port_info_t* p_port_lkp = NULL;

    hsrv_openflow_portdb_lookup_port(ifindex, &p_port_lkp);
    if (p_port_lkp)
    {
        ctclib_hash_release(HSRV_OPENFLOW_TTP_PORT_HASH, p_port_lkp);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_portdb_lookup_port(uint32 ifindex, hsrv_openflow_ttp_port_info_t** pp_port)
{
    hsrv_openflow_ttp_port_info_t port_lkp;

    sal_memset(&port_lkp, 0, sizeof(hsrv_openflow_ttp_port_info_t));
    port_lkp.ifindex = ifindex;
 
    *pp_port = (hsrv_openflow_ttp_port_info_t*)ctclib_hash_lookup(HSRV_OPENFLOW_TTP_PORT_HASH, &port_lkp);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_portdb_start(void)
{
    if (NULL == g_pst_openflow_ttp_port_master)
    {
        g_pst_openflow_ttp_port_master = HSRV_OPENFLOW_PORT_MALLOC(sizeof(hsrv_openflow_ttp_port_master_t));
        HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_ttp_port_master);

        HSRV_OPENFLOW_TTP_PORT_HASH = ctclib_hash_create(_hsrv_openflow_portdb_port_make,
                                                      _hsrv_openflow_portdb_port_compare);
        HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TTP_PORT_HASH);
    }
    
    return HSRV_E_NONE;
}

