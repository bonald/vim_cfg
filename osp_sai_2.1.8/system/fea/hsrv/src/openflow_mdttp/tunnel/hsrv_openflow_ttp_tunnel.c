
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_tunnel_db.h"
#include "hsrv_openflow_ttp_tunnel.h"

extern hsrv_openflow_ttp_tunnel_master_t    *g_pst_openflow_ttp_tunnel_master;

struct hsrv_openflow_tunnel_debug_data_s
{
    FILE* fp;
    int32 i;
};
typedef struct hsrv_openflow_tunnel_debug_data_s hsrv_openflow_tunnel_debug_data_t;

#define ____TUNNEL_EXTERNAL_API____

int32
hsrv_openflow_tunnel_get_info_by_metadata(uint32 meta_data, uint16* p_ofport, uint32* p_tun_id)
{
    hsrv_openflow_ttp_tunnel_metadata_t *p_tnl_metadata = NULL;
    hsrv_openflow_tunneldb_lookup_metadata(meta_data, &p_tnl_metadata);
    if (p_tnl_metadata)
    {
        *p_ofport = p_tnl_metadata->ofport;
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunnel_get_metadata_info_by_ofport(uint16 ofport, hsrv_openflow_ttp_tunnel_metadata_t **p_tnl_metadata)
{
    /* metadata uqual with ofport */
    hsrv_openflow_tunneldb_lookup_metadata_ofport(ofport, p_tnl_metadata);
    return HSRV_E_NONE;
}

hsrv_openflow_ttp_tunnel_info_t*
hsrv_openflow_tunnel_get_tunnel_info(uint32 ifindex)
{
    hsrv_openflow_ttp_tunnel_info_t* p_tnl_info = NULL;

    hsrv_openflow_tunneldb_lookup_tunnel(ifindex, &p_tnl_info);

    return p_tnl_info;
}

#define ____TUNNEL_HAL_APIs____

int32
hsrv_openflow_tunnel_add_bind_port(hsrv_openflow_ttp_tunnel_info_t* p_tnl_info)
{
    hsrv_openflow_ttp_tunnel_bind_port_t* p_bind_port = NULL;
    uint16 gport = 0;
    
    HSRV_OPENFLOW_PTR_CHECK(p_tnl_info);

    hsrv_openflow_tunneldb_lookup_bind_port(p_tnl_info->tunnel_info.bond_port_ifindex, &p_bind_port);
    if (p_bind_port)
    {
        p_bind_port->ref_cnt ++;
        return HSRV_E_NONE;
    }
    
    p_bind_port = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_ttp_tunnel_bind_port_t));
    HSRV_OPENFLOW_MEM_CHECK(p_bind_port);
    sal_memset(p_bind_port, 0, sizeof(hsrv_openflow_ttp_tunnel_bind_port_t));

    /* metadata key hash */
    p_bind_port->bind_port_ifindex = p_tnl_info->tunnel_info.bond_port_ifindex;
    p_bind_port->ref_cnt ++;
    hsrv_openflow_tunneldb_add_bind_port(p_bind_port);

    
    /* set bind port acl and metadata attribute */
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_tnl_info->tunnel_info.bond_port_ifindex, &gport));
    HSRV_IF_ERROR_RETURN(ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, 0x1));
    HSRV_IF_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_METADATA_OVERWRITE_UDF, 0x1));
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunnel_del_bind_port(hsrv_openflow_ttp_tunnel_info_t* p_tnl_info)
{
    hsrv_openflow_ttp_tunnel_bind_port_t* p_bind_port = NULL;
    uint16 gport = 0;
    
    HSRV_OPENFLOW_PTR_CHECK(p_tnl_info);

    hsrv_openflow_tunneldb_lookup_bind_port(p_tnl_info->tunnel_info.bond_port_ifindex, &p_bind_port);
    if (!p_bind_port)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    p_bind_port->ref_cnt --;
    if (0 == p_bind_port->ref_cnt)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_tunneldb_del_bind_port(p_bind_port->bind_port_ifindex));
        
        /* set bind port acl and metadata attribute */
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_tnl_info->tunnel_info.bond_port_ifindex, &gport));
        HSRV_IF_ERROR_RETURN(ctc_port_set_direction_property(gport, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, 0x8));
        HSRV_IF_ERROR_RETURN(ctc_port_set_property(gport, CTC_PORT_PROP_METADATA_OVERWRITE_UDF, 0x0));

        /* reset bind port scl attr */
        ctc_ttp_drv_port_reset_scl_property(p_tnl_info->tunnel_info.bond_port_ifindex, 1);
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunnel_add_metadata(hsrv_openflow_ttp_tunnel_info_t* p_tnl_info)
{
    hsrv_openflow_ttp_tunnel_metadata_t* p_tnl_metadata = NULL;
    uint16 metadata = 0;
    
    HSRV_OPENFLOW_PTR_CHECK(p_tnl_info);

    hsrv_openflow_tunneldb_lookup_metadata_ofport(p_tnl_info->tunnel_info.ofport, &p_tnl_metadata);
    if (p_tnl_metadata)
    {
        return HSRV_E_ENTRY_EXIST;
    }
    
    p_tnl_metadata = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_ttp_tunnel_metadata_t));
    HSRV_OPENFLOW_MEM_CHECK(p_tnl_metadata);
    sal_memset(p_tnl_metadata, 0, sizeof(hsrv_openflow_ttp_tunnel_metadata_t));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_tunneldb_metadata_alloc(&metadata));

    /* metadata key hash */
    p_tnl_metadata->ofport = p_tnl_info->tunnel_info.ofport;
    p_tnl_metadata->metadata = metadata;
    hsrv_openflow_tunneldb_add_metadata(p_tnl_metadata);

    /* ofport key hash */
    p_tnl_metadata = NULL;
    p_tnl_metadata = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_ttp_tunnel_metadata_t));
    HSRV_OPENFLOW_MEM_CHECK(p_tnl_metadata);
    sal_memset(p_tnl_metadata, 0, sizeof(hsrv_openflow_ttp_tunnel_metadata_t));
    p_tnl_metadata->ofport = p_tnl_info->tunnel_info.ofport;
    p_tnl_metadata->metadata = metadata;
    hsrv_openflow_tunneldb_add_metadata_ofport(p_tnl_metadata); 
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tunnel_del_metadata(hsrv_openflow_ttp_tunnel_info_t* p_tnl_info)
{
    hsrv_openflow_ttp_tunnel_metadata_t* p_tnl_metadata = NULL;
    uint16 metadata = 0;
    
    HSRV_OPENFLOW_PTR_CHECK(p_tnl_info);

    hsrv_openflow_tunneldb_lookup_metadata_ofport(p_tnl_info->tunnel_info.ofport, &p_tnl_metadata);
    if (!p_tnl_metadata)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    metadata = p_tnl_metadata->metadata;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_tunneldb_del_metadata_ofport(p_tnl_info->tunnel_info.ofport));
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_tunneldb_del_metadata(metadata)); 

    HSRV_IF_ERROR_RETURN(hsrv_openflow_tunneldb_metadata_release(metadata));
    
    return HSRV_E_NONE;
}

int32
fei_api_openflow_tunnel_add_port(fei_openflow_add_tunnel_req_t* p_req)
{
    hsrv_openflow_ttp_tunnel_info_t* p_tnl_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_req);

    /* lookup tunnel */
    hsrv_openflow_tunneldb_lookup_tunnel(p_req->tunnel_info.tunnel_ifindex, &p_tnl_info);
    if (p_tnl_info)
    {
        /* del old bind port info */
        HSRV_IF_ERROR_RETURN(hsrv_openflow_tunnel_del_bind_port(p_tnl_info));

        /* if tunnel already exist, modify tunnel info */
        sal_memcpy(&p_tnl_info->tunnel_info, &p_req->tunnel_info, sizeof(glb_openflow_tunnel_info_t));

        /* add new bind port info */
        HSRV_IF_ERROR_RETURN(hsrv_openflow_tunnel_add_bind_port(p_tnl_info));
        
        return HSRV_E_NONE;
    }

    if (hsrv_openflow_tunnel_check_tunnel_port_count())
    {
        HSRV_OPENFLOW_LOG_WARN("Openflow tunnel port exceed the max size");
        return HSRV_E_OPENFLOW_RESOURCE_FULL;
    }

    /* alloc memery */
    p_tnl_info = HSRV_OPENFLOW_TUNNEL_MALLOC(sizeof(hsrv_openflow_ttp_tunnel_info_t));

    HSRV_OPENFLOW_MEM_CHECK(p_tnl_info);
    sal_memset(p_tnl_info, 0, sizeof(hsrv_openflow_ttp_tunnel_info_t));

    p_tnl_info->ifindex = p_req->tunnel_info.tunnel_ifindex;
    sal_memcpy(&p_tnl_info->tunnel_info, &p_req->tunnel_info, sizeof(glb_openflow_tunnel_info_t));

    /* add tunnel db */
    hsrv_openflow_tunneldb_add_tunnel(p_tnl_info);
    hsrv_openflow_tunnel_add_tunnel_port_count();

    /* add tunnel metadata mapping */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_tunnel_add_metadata(p_tnl_info));

    /* add bind port info */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_tunnel_add_bind_port(p_tnl_info));
    
    return HSRV_E_NONE; 
}


int32
fei_api_openflow_tunnel_delete_port(fei_openflow_del_tunnel_req_t* p_req)
{
    hsrv_openflow_ttp_tunnel_info_t* p_tnl_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_req);

    /* if del reason is running down, does not del hsrv tunnel port info */
    if (GLB_OPENFLOW_TUNNEL_RUNNING_DOWN == p_req->tunnel_del_type)
    {
        return HSRV_E_NONE;
    }
    
    /* lookup tunnel */
    hsrv_openflow_tunneldb_lookup_tunnel(p_req->ifindex, &p_tnl_info);
    if (NULL == p_tnl_info)
    {
        HSRV_OPENFLOW_LOG_WARN("Tunnel port:%d not in hsrv db", p_req->ifindex);
        return HSRV_E_NONE; 
    }

    /* release tunnel metadata mapping */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_tunnel_del_metadata(p_tnl_info));

    /* release bind port info */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_tunnel_del_bind_port(p_tnl_info));

    /* delete tunnel */
    hsrv_openflow_tunneldb_del_tunnel(p_req->ifindex);
    hsrv_openflow_tunnel_del_tunnel_port_count();


    /* free memery */
    HSRV_OPENFLOW_TUNNEL_FREE(p_tnl_info);

    p_tnl_info = NULL;

    return HSRV_E_NONE;
}

#define ____TUNNEL_DEBUG____

static void 
hsrv_openflow_ttp_show_tunnel_brief__(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_ttp_tunnel_info_t *p_tunnel = (hsrv_openflow_ttp_tunnel_info_t*)phb->data;
    hsrv_openflow_tunnel_debug_data_t* p_data = user_data;
    char ip[64] = {0};
   
    sal_fprintf(p_data->fp, "%6u", p_data->i);
    sal_fprintf(p_data->fp, "%15u", p_tunnel->ifindex);
    sal_fprintf(p_data->fp, "%16s", cdb_addr_ipv4_val2str(ip, sizeof(ip), (addr_ipv4_t*)(&p_tunnel->tunnel_info.local_ip)));
    sal_memset(ip, 0 , sizeof(ip));
    sal_fprintf(p_data->fp, "%16s\n", cdb_addr_ipv4_val2str(ip, sizeof(ip), (addr_ipv4_t*)(&p_tunnel->tunnel_info.remote_ip)));
    
    p_data->i ++;
} 

int32
hsrv_openflow_ttp_show_tunnel_brief(FILE *fp)
{
    hsrv_openflow_tunnel_debug_data_t data;
    
    sal_fprintf(fp, "--------------HSRV OPENFLOW TUNNEL---------------------\n");
    sal_fprintf(fp, "Tunnel hash count    : %d\n", g_pst_openflow_ttp_tunnel_master->p_tunnel_info_hash->count);
    sal_fprintf(fp, "-------------------------------------------------------\n");

    sal_fprintf(fp, "%6s %15s %16s %16s\n", "index", "tunnel_ifindex", "local-ip", "remote-ip");
    sal_fprintf(fp, "-------------------------------------------------------\n");

    data.fp = fp;
    data.i  = 0;
    
    ctclib_hash_iterate(HSRV_OPENFLOW_TTP_TUNNEL_INFO_HASH, hsrv_openflow_ttp_show_tunnel_brief__, &data);

    return HSRV_E_NONE;
}

int32
hsrv_openflow_ttp_show_tunnel(FILE *fp, uint32 tunnel_id)
{
    hsrv_openflow_ttp_tunnel_info_t *p_tunnel = NULL;
    char ip[64] = {0};
    char mac[64] = {0};

    sal_memset(ip, 0 , sizeof(ip));
    sal_memset(mac, 0 , sizeof(mac));
    hsrv_openflow_tunneldb_lookup_tunnel(tunnel_id, &p_tunnel);
    if (NULL == p_tunnel)
    {
        sal_fprintf(fp, "tunnel port %u not exist in db.\n", tunnel_id);
        return HSRV_E_NONE;
    }
    sal_fprintf(fp, "tunnel name : %s\n", p_tunnel->tunnel_info.tunnel_name);
    sal_fprintf(fp, "tunnel_ifindex : %u\n", tunnel_id);
    sal_fprintf(fp, "ofport : %hu\n", p_tunnel->tunnel_info.ofport);
    sal_fprintf(fp, "local-ip : %s\n", cdb_addr_ipv4_val2str(ip, sizeof(ip), (addr_ipv4_t*)(&p_tunnel->tunnel_info.local_ip)));
    sal_memset(ip, 0 , sizeof(ip));
    sal_fprintf(fp, "remote-ip : %s\n", cdb_addr_ipv4_val2str(ip, sizeof(ip), (addr_ipv4_t*)(&p_tunnel->tunnel_info.remote_ip)));
    sal_fprintf(fp, "bind_port_name : %s\n", p_tunnel->tunnel_info.bind_port_name);
    sal_fprintf(fp, "bind_port_ifindex : %u\n", p_tunnel->tunnel_info.bond_port_ifindex);
    sal_fprintf(fp, "nexthop_mac : %s\n", cdb_mac_addr_vtysh_val2str(mac, sizeof(mac), p_tunnel->tunnel_info.nexthop_mac));
    sal_fprintf(fp, "\n");
    return HSRV_E_NONE;
}

#define ____TUNNEL_INIT___

int32
hsrv_openflow_tunnel_start(void)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_tunneldb_start());
    return HSRV_E_NONE;
}


