
#ifndef __HSRV_OPENFLOW_TTP_TUNNEL_DB_H__
#define __HSRV_OPENFLOW_TTP_TUNNEL_DB_H__

#define HSRV_TUNNEL_METADATA_MIN    1
#define HSRV_TUNNEL_METADATA_NUM    500

struct hsrv_openflow_ttp_tunnel_master_s
{
    ctclib_hash_t* p_tunnel_info_hash;         /* hsrv_openflow_ttp_tunnel_info_t */
    ctclib_hash_t* p_tunnel_metadata_hash;         /* hsrv_openflow_ttp_tunnel_metadata_t */
    ctclib_hash_t* p_tunnel_metadata_ofport_hash;         /* hsrv_openflow_ttp_tunnel_metadata_t */
    ctclib_opf_t*  p_tunnel_metadata_opf;
    ctclib_hash_t* p_tunnel_bind_port_hash;         /* hsrv_openflow_ttp_tunnel_bind_port_t */
    uint32 p_tunnel_port_num;
};
typedef struct hsrv_openflow_ttp_tunnel_master_s hsrv_openflow_ttp_tunnel_master_t;

extern hsrv_openflow_ttp_tunnel_master_t *g_pst_openflow_ttp_tunnel_master;

#define HSRV_OPENFLOW_TTP_TUNNEL_INFO_HASH               g_pst_openflow_ttp_tunnel_master->p_tunnel_info_hash
#define HSRV_OPENFLOW_TTP_TUNNEL_PORT_NUM                 g_pst_openflow_ttp_tunnel_master->p_tunnel_port_num
#define HSRV_OPENFLOW_TTP_TUNNEL_METADATA_HASH               g_pst_openflow_ttp_tunnel_master->p_tunnel_metadata_hash
#define HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OFPORT_HASH      g_pst_openflow_ttp_tunnel_master->p_tunnel_metadata_ofport_hash
#define HSRV_OPENFLOW_TTP_TUNNEL_METADATA_OPF               g_pst_openflow_ttp_tunnel_master->p_tunnel_metadata_opf
#define HSRV_OPENFLOW_TTP_TUNNEL_BIND_PORT_HASH      g_pst_openflow_ttp_tunnel_master->p_tunnel_bind_port_hash

int32
hsrv_openflow_tunneldb_add_tunnel(hsrv_openflow_ttp_tunnel_info_t* p_tunnel);

int32
hsrv_openflow_tunneldb_del_tunnel(uint32 ifindex);

int32
hsrv_openflow_tunneldb_lookup_tunnel(uint32 ifindex, hsrv_openflow_ttp_tunnel_info_t** pp_tunnel);

int32
hsrv_openflow_tunneldb_add_metadata(hsrv_openflow_ttp_tunnel_metadata_t* p_tunnel_metadata);

int32
hsrv_openflow_tunneldb_del_metadata(uint16 metadata);

int32
hsrv_openflow_tunneldb_lookup_metadata(uint16 metadata, hsrv_openflow_ttp_tunnel_metadata_t** pp_tunnel_metadata);

int32
hsrv_openflow_tunneldb_add_metadata_ofport(hsrv_openflow_ttp_tunnel_metadata_t* p_tunnel_metadata);

int32
hsrv_openflow_tunneldb_del_metadata_ofport(uint16 ofport);

int32
hsrv_openflow_tunneldb_lookup_metadata_ofport(uint16 ofport, hsrv_openflow_ttp_tunnel_metadata_t** pp_tunnel_metadata);

int32 
hsrv_openflow_tunnel_check_tunnel_port_count(void);

int32 
hsrv_openflow_tunnel_add_tunnel_port_count(void);

int32 
hsrv_openflow_tunnel_del_tunnel_port_count(void);

int32
hsrv_openflow_tunneldb_metadata_alloc(uint16 *metadata);

int32
hsrv_openflow_tunneldb_metadata_release(uint16 metadata);

int32
hsrv_openflow_tunneldb_add_bind_port(hsrv_openflow_ttp_tunnel_bind_port_t* p_tunnel_bind_port);


int32
hsrv_openflow_tunneldb_del_bind_port(uint32 ifindex);


int32
hsrv_openflow_tunneldb_lookup_bind_port(uint32 ifindex, hsrv_openflow_ttp_tunnel_bind_port_t** pp_tunnel_bind_port);

int32
hsrv_openflow_tunneldb_start(void);

#endif
