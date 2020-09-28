#ifndef __HSRV_OPENFLOW_TTP_TUNNEL_H__
#define __HSRV_OPENFLOW_TTP_TUNNEL_H__

#define HSRV_OPENFLOW_TUNNEL_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_TUNNEL, size)
#define HSRV_OPENFLOW_TUNNEL_CALLOC(size) HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW_TUNNEL, size)
#define HSRV_OPENFLOW_TUNNEL_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_TUNNEL, ptr);  \
    ptr = NULL; \
}while(0)

#define HSRV_OPENFLOW_TUNNEL_PORT_MAX_SIZE 600

struct hsrv_openflow_ttp_tunnel_info_s
{
    uint32 ifindex;    /* tunnel ifindex */
    glb_openflow_tunnel_info_t tunnel_info;
};
typedef struct hsrv_openflow_ttp_tunnel_info_s hsrv_openflow_ttp_tunnel_info_t;

struct hsrv_openflow_ttp_tunnel_metadata_s
{
    uint16 ofport;
    uint32 tun_id;
    uint16 metadata;
    uint32 ref_cnt;
};
typedef struct hsrv_openflow_ttp_tunnel_metadata_s hsrv_openflow_ttp_tunnel_metadata_t;

/* bind port info for restore bind port scl and acl attribute */
struct hsrv_openflow_ttp_tunnel_bind_port_s
{
    uint32 bind_port_ifindex;
    uint32 ref_cnt; /* count tunnel refer the same physical port */
};
typedef struct hsrv_openflow_ttp_tunnel_bind_port_s hsrv_openflow_ttp_tunnel_bind_port_t;

int32
hsrv_openflow_tunnel_get_info_by_metadata(uint32 meta_data, uint16* p_ofport, uint32* p_tun_id);

int32
hsrv_openflow_tunnel_get_metadata_info_by_ofport(uint16 ofport, hsrv_openflow_ttp_tunnel_metadata_t **p_tnl_metadata);

hsrv_openflow_ttp_tunnel_info_t*
hsrv_openflow_tunnel_get_tunnel_info(uint32 ifindex);

int32
hsrv_openflow_ttp_show_tunnel_brief(FILE *fp);

int32
hsrv_openflow_ttp_show_tunnel(FILE *fp, uint32 tunnel_id);

int32
hsrv_openflow_tunnel_start(void);

#endif
