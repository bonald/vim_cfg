#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_tunnel_db.h"

hsrv_openflow_ttp_tunnel_master_t    *g_pst_openflow_ttp_tunnel_master = NULL;

static inline uint32
_hsrv_openflow_tunneldb_tunnel_make(hsrv_openflow_ttp_tunnel_info_t* p_tunnel)
{
    return p_tunnel->ifindex;
}

static inline bool
_hsrv_openflow_tunneldb_tunnel_compare(hsrv_openflow_ttp_tunnel_info_t* pva, hsrv_openflow_ttp_tunnel_info_t* pvb)
{
    if (pva->ifindex != pvb->ifindex)
    {
        return FALSE;
    }

    return TRUE;
}

static inline uint32
_hsrv_openflow_tunneldb_metadata_make(hsrv_openflow_ttp_tunnel_metadata_t* p_tunnel_metadata)
{
    return p_tunnel_metadata->metadata;
}

static inline bool
_hsrv_openflow_tunneldb_metadata_compare(hsrv_openflow_ttp_tunnel_metadata_t* pva, hsrv_openflow_ttp_tunnel_metadata_t* pvb)
{
    if (pva->metadata != pvb->metadata)
    {
        return FALSE;
    }

    return TRUE;
}

static inline uint32
_hsrv_openflow_tunneldb_metadata_ofport_make(hsrv_openflow_ttp_tunnel_metadata_t* p_tunnel_metadata)
{
    return p_tunnel_metadata->ofport;
}

static inline bool
_hsrv_openflow_tunneldb_metadata_ofport_compare(hsrv_openflow_ttp_tunnel_metadata_t* pva, hsrv_openflow_ttp_tunnel_metadata_t* pvb)
{
    if (pva->ofport != pvb->ofport)
    {
        return FALSE;
    }

    return TRUE;
}

static inline uint32
_hsrv_openflow_tunneldb_bind_port_make(hsrv_openflow_ttp_tunnel_bind_port_t* p_tunnel_bind_port)
{
    return p_tunnel_bind_port->bind_port_ifindex;
}

static inline bool
_hsrv_openflow_tunneldb_bind_port_compare(hsrv_openflow_ttp_tunnel_bind_port_t* pva, hsrv_openflow_ttp_tunnel_bind_port_t* pvb)
{
    if (pva->bind_port_ifindex != pvb->bind_port_ifindex)
    {
        return FALSE;
    }

    return TRUE;
}

int32
hsrv_openflow_tunneldb_add_bind_port(hsrv_openflow_ttp_tunnel_bind_port_t* p_tunnel_bind_port)
{
    ctclib_hash_get(HSRV_OPENFLOW_TTP_TUNNEL_BIND_PORT_HASH, p_tunnel_bind_port, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_del_bind_port(uint32 ifindex)
{
    hsrv_openflow_ttp_tunnel_bind_port_t* p_bind_port_lkp = NULL;

    hsrv_openflow_tunneldb_lookup_bind_port(ifindex, &p_bind_port_lkp);
    if (p_bind_port_lkp)
    {
        ctclib_hash_release(HSRV_OPENFLOW_TTP_TUNNEL_BIND_PORT_HASH, p_bind_port_lkp);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_lookup_bind_port(uint32 ifindex, hsrv_openflow_ttp_tunnel_bind_port_t** pp_tunnel_bind_port)
{
    hsrv_openflow_ttp_tunnel_bind_port_t p_bind_port_lkp;

    sal_memset(&p_bind_port_lkp, 0, sizeof(hsrv_openflow_ttp_tunnel_bind_port_t));
    p_bind_port_lkp.bind_port_ifindex = ifindex;
 
    *pp_tunnel_bind_port = (hsrv_openflow_ttp_tunnel_bind_port_t*)ctclib_hash_lookup(HSRV_OPENFLOW_TTP_TUNNEL_BIND_PORT_HASH, &p_bind_port_lkp);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_add_metadata(hsrv_openflow_ttp_tunnel_metadata_t* p_tunnel_metadata)
{
    ctclib_hash_get(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_HASH, p_tunnel_metadata, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_del_metadata(uint16 metadata)
{
    hsrv_openflow_ttp_tunnel_metadata_t* p_metadata_lkp = NULL;

    hsrv_openflow_tunneldb_lookup_metadata(metadata, &p_metadata_lkp);
    if (p_metadata_lkp)
    {
        ctclib_hash_release(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_HASH, p_metadata_lkp);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_lookup_metadata(uint16 metadata, hsrv_openflow_ttp_tunnel_metadata_t** pp_tunnel_metadata)
{
    hsrv_openflow_ttp_tunnel_metadata_t tunnel_metadata_lkp;

    sal_memset(&tunnel_metadata_lkp, 0, sizeof(hsrv_openflow_ttp_tunnel_metadata_t));
    tunnel_metadata_lkp.metadata = metadata;
 
    *pp_tunnel_metadata = (hsrv_openflow_ttp_tunnel_metadata_t*)ctclib_hash_lookup(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_HASH, &tunnel_metadata_lkp);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_add_metadata_ofport(hsrv_openflow_ttp_tunnel_metadata_t* p_tunnel_metadata)
{
    ctclib_hash_get(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OFPORT_HASH, p_tunnel_metadata, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_del_metadata_ofport(uint16 ofport)
{
    hsrv_openflow_ttp_tunnel_metadata_t* p_metadata_lkp = NULL;

    hsrv_openflow_tunneldb_lookup_metadata_ofport(ofport, &p_metadata_lkp);
    if (p_metadata_lkp)
    {
        ctclib_hash_release(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OFPORT_HASH, p_metadata_lkp);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_lookup_metadata_ofport(uint16 ofport, hsrv_openflow_ttp_tunnel_metadata_t** pp_tunnel_metadata)
{
    hsrv_openflow_ttp_tunnel_metadata_t tunnel_metadata_lkp;

    sal_memset(&tunnel_metadata_lkp, 0, sizeof(hsrv_openflow_ttp_tunnel_metadata_t));
    tunnel_metadata_lkp.ofport = ofport;
 
    *pp_tunnel_metadata = (hsrv_openflow_ttp_tunnel_metadata_t*)ctclib_hash_lookup(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OFPORT_HASH, &tunnel_metadata_lkp);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_add_tunnel(hsrv_openflow_ttp_tunnel_info_t* p_tunnel)
{
    ctclib_hash_get(HSRV_OPENFLOW_TTP_TUNNEL_INFO_HASH, p_tunnel, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_del_tunnel(uint32 ifindex)
{
    hsrv_openflow_ttp_tunnel_info_t* p_tunnel_lkp = NULL;

    hsrv_openflow_tunneldb_lookup_tunnel(ifindex, &p_tunnel_lkp);
    if (p_tunnel_lkp)
    {
        ctclib_hash_release(HSRV_OPENFLOW_TTP_TUNNEL_INFO_HASH, p_tunnel_lkp);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_lookup_tunnel(uint32 ifindex, hsrv_openflow_ttp_tunnel_info_t** pp_tunnel)
{
    hsrv_openflow_ttp_tunnel_info_t tunnel_lkp;

    sal_memset(&tunnel_lkp, 0, sizeof(hsrv_openflow_ttp_tunnel_info_t));
    tunnel_lkp.ifindex = ifindex;
 
    *pp_tunnel = (hsrv_openflow_ttp_tunnel_info_t*)ctclib_hash_lookup(HSRV_OPENFLOW_TTP_TUNNEL_INFO_HASH, &tunnel_lkp);
    
    return HSRV_E_NONE;
}

int32 
hsrv_openflow_tunnel_check_tunnel_port_count(void)
{
    if (HSRV_OPENFLOW_TTP_TUNNEL_PORT_NUM + 1 > HSRV_OPENFLOW_TUNNEL_PORT_MAX_SIZE)
    {
        return HSRV_E_OPENFLOW_RESOURCE_FULL;
    }
    return HSRV_E_NONE;
}

int32 
hsrv_openflow_tunnel_add_tunnel_port_count(void)
{
    HSRV_OPENFLOW_TTP_TUNNEL_PORT_NUM ++;
    return HSRV_E_NONE;
}

int32 
hsrv_openflow_tunnel_del_tunnel_port_count(void)
{
    if (HSRV_OPENFLOW_TTP_TUNNEL_PORT_NUM > 0)
    {
        HSRV_OPENFLOW_TTP_TUNNEL_PORT_NUM --;
    }
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_metadata_alloc(uint16 *metadata)
{
    uint16 metadata_tmp = 0;
    metadata_tmp = ctclib_opf_alloc_offset(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OPF, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == metadata_tmp)
    {
        return HSRV_E_RESOURCE_FULL;
    }
    *metadata = metadata_tmp;

    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunneldb_metadata_release(uint16 metadata)
{
    return ctclib_opf_free_offset(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OPF, 0, 1, metadata);
}

int32
hsrv_openflow_tunneldb_start(void)
{
    if (NULL == g_pst_openflow_ttp_tunnel_master)
    {
        g_pst_openflow_ttp_tunnel_master = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_ttp_tunnel_master_t));
        HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_ttp_tunnel_master);

        HSRV_OPENFLOW_TTP_TUNNEL_INFO_HASH = ctclib_hash_create(_hsrv_openflow_tunneldb_tunnel_make,
                                                      _hsrv_openflow_tunneldb_tunnel_compare);
        HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TTP_TUNNEL_INFO_HASH);
        HSRV_OPENFLOW_TTP_TUNNEL_PORT_NUM = 0;

        HSRV_OPENFLOW_TTP_TUNNEL_METADATA_HASH = ctclib_hash_create(_hsrv_openflow_tunneldb_metadata_make,
                                                      _hsrv_openflow_tunneldb_metadata_compare);
        HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_HASH);

        HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OFPORT_HASH = ctclib_hash_create(_hsrv_openflow_tunneldb_metadata_ofport_make,
                                                      _hsrv_openflow_tunneldb_metadata_ofport_compare);
        HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OFPORT_HASH);

        HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OPF = ctclib_opf_init(1);
        HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OPF);
        ctclib_opf_init_offset(HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OPF, 0, HSRV_TUNNEL_METADATA_MIN, HSRV_TUNNEL_METADATA_NUM);

        HSRV_OPENFLOW_TTP_TUNNEL_BIND_PORT_HASH = ctclib_hash_create(_hsrv_openflow_tunneldb_metadata_ofport_make,
                                                      _hsrv_openflow_tunneldb_metadata_ofport_compare);
        HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TTP_TUNNEL_BIND_PORT_HASH);
    }
    
    return HSRV_E_NONE;
}

