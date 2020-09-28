/* Added by weizj for tunnel */
#ifndef __HSRV_OPENFLOW_TUNNEL_PRIV_H__
#define __HSRV_OPENFLOW_TUNNEL_PRIV_H__


struct hsrv_openflow_tunnel_decap_flow_s
{
    uint16 metadata;
    glb_openflow_tnl_key_t tun_key;

    uint32 ref_cnt;
};
typedef struct hsrv_openflow_tunnel_decap_flow_s hsrv_openflow_tunnel_decap_flow_t;

struct hsrv_openflow_tunnel_metadata_s
{
    uint16 ofport;
    uint32 tun_id;
    uint16 metadata;

    uint32 ref_cnt;
};
typedef struct hsrv_openflow_tunnel_metadata_s hsrv_openflow_tunnel_metadata_t;

struct hsrv_openflow_tunnel_fid_s
{
    uint32 tun_id;
    uint16 fid;

    uint32 ref_cnt;
};
typedef struct hsrv_openflow_tunnel_fid_s hsrv_openflow_tunnel_fid_t;

// DEPRECATED.
#if 0
struct hsrv_openflow_tunnel_l3if_s
{
    uint16 gport;

    uint32 ref_cnt;
};
typedef struct hsrv_openflow_tunnel_l3if_s hsrv_openflow_tunnel_l3if_t;
#endif

struct hsrv_openflow_tunnel_debug_data_s
{
    FILE* fp;
    int32 i;

    /* Added by weizj for tunnel bug 33662 */
    uint32 ifindex[HSRV_OPENFLOW_TUNNEL_PORT_MAX_SIZE];
};
typedef struct hsrv_openflow_tunnel_debug_data_s hsrv_openflow_tunnel_debug_data_t;

#endif
