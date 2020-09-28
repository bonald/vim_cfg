/* Added by weizj for tunnel */
#ifndef __HSRV_OPENFLOW_TUNNEL_H__
#define __HSRV_OPENFLOW_TUNNEL_H__

#include "hsrv_openflow_nexthop.h"

/* hsrv openflow memory tunnel related macros */
#define HSRV_OPENFLOW_TUNNEL_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_TUNNEL, size)
#define HSRV_OPENFLOW_TUNNEL_CALLOC(size) HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW_TUNNEL, size)
#define HSRV_OPENFLOW_TUNNEL_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_TUNNEL, ptr);  \
    ptr = NULL; \
}while(0)

#define HSRV_OPENFLOW_TUNNEL_ZERO                                0
#define HSRV_OPENFLOW_TUNNEL_PORT_MAX_SIZE                       500
#define HSRV_OPENFLOW_TUNNEL_METADATA_BLOCK_SIZE                 256
#define HSRV_OPENFLOW_TUNNEL_DEFAULT_VID_FOR_ENCAP               4095
#define HSRV_OPENFLOW_TUNNEL_DEFAULT_MTU_SIZE                    1518
#define HSRV_OPENFLOW_TUNNEL_DEFAULT_SET_TTL                     64
#define HSRV_OPENFLOW_TUNNEL_DEFAULT_SET_TOS                     0
#define HSRV_OPENFLOW_TUNNEL_PROTOCOL_TYPE_TRANSPARENT_BRIDGING  0x6558

#define HSRV_OPENFLOW_TUNNEL_FID_MAX              0xffff
#define HSRV_OPENFLOW_TUNNEL_METADATA_MAX         0xffff

#define HSRV_OPENFLOW_TUNNEL_DECAP_MODE_1         0
#define HSRV_OPENFLOW_TUNNEL_DECAP_MODE_2         1

struct hsrv_openflow_tunnel_info_s
{
    uint32 ifindex;    /* tunnel ifindex */
    glb_openflow_tunnel_info_t tunnel_info;
};
typedef struct hsrv_openflow_tunnel_info_s hsrv_openflow_tunnel_info_t;

struct hsrv_openflow_dynamic_tunnel_nexthop_list_s
{
    ctclib_list_node_t node;                /**< node for nexthop_list in hsrv_openflow_dynamic_tunnel_info_t */

    uint32 tunnel_route_ip;
    uint32 mask_len;
    uint32 tunnel_nexthop_ip;  
    uint32 nexthop_id;
    //uint32 flow_id;
    uint32 tunnel_ifindex;
};
typedef struct hsrv_openflow_dynamic_tunnel_nexthop_list_s hsrv_openflow_dynamic_tunnel_nexthop_list_t;

struct hsrv_openflow_dynamic_tunnel_info_key_s
{
    //uint32 tunnel_route_ip;
    //uint32 mask_len;
    //uint32 tunnel_nexthop_ip;
    uint32 flow_id;
};
typedef struct hsrv_openflow_dynamic_tunnel_info_key_s hsrv_openflow_dynamic_tunnel_info_key_t;

struct hsrv_openflow_dynamic_tunnel_info_s
{
    hsrv_openflow_dynamic_tunnel_info_key_t dynamic_tunnel_info_key;
    ctclib_list_t nexthop_list;
};
typedef struct hsrv_openflow_dynamic_tunnel_info_s hsrv_openflow_dynamic_tunnel_info_t;


struct hsrv_openflow_tunnel_master_s
{
    ctclib_hash_t* openflow_tunnel_info_hash;         /* ifindex -> tunnel_info hash */
    ctclib_hash_t* openflow_dynamic_tunnel_info_hash;
    ctclib_hash_t* openflow_tunnel_fid_hash;          /* tun_id -> fid hash */
    ctclib_hash_t* openflow_tunnel_metadata_hash;     /* in_port + tun_id -> metadate hash */
    ctclib_hash_t* openflow_tunnel_decap_flow_hash;   /* metadata -> decap_flow hash */
    ctclib_hash_t* openflow_tunnel_l3if_hash;         /* gport -> l3if hash */
    ctclib_hash_t* openflow_tunnel_ofport_hash;       /* metadata -> ofport hash */

    ctclib_opf_t*  fid_opf;                           /* fid pool */
    ctclib_opf_t*  metadata_opf;                      /* metadat pool */

    uint16         port_cnt;                          /* current port count */
    uint16         metadata_cnt;                      /* current fid count */
    uint16         fid_cnt;                           /* current metadata count */
    uint16         flow_cnt;                          /* current decap flow count */

    uint16         port_max;                          /* tunnel port max count */
    uint8          decap_mode;                        /* decap mode */
    uint32         vtep_ip;                           /* local vtep ip */
    uint8          rsv;
};
typedef struct hsrv_openflow_tunnel_master_s hsrv_openflow_tunnel_master_t;


#define HSRV_OPENFLOW_TUNNEL_INFO_HASH               g_pst_openflow_tunnel_master->openflow_tunnel_info_hash
#define HSRV_OPENFLOW_DYNAMIC_TUNNEL_INFO_HASH  g_pst_openflow_tunnel_master->openflow_dynamic_tunnel_info_hash
#define HSRV_OPENFLOW_TUNNEL_FID_HASH                g_pst_openflow_tunnel_master->openflow_tunnel_fid_hash
#define HSRV_OPENFLOW_TUNNEL_METADATA_HASH           g_pst_openflow_tunnel_master->openflow_tunnel_metadata_hash
#define HSRV_OPENFLOW_TUNNEL_FLOW_HASH               g_pst_openflow_tunnel_master->openflow_tunnel_decap_flow_hash
// DEPRECATED. #define HSRV_OPENFLOW_TUNNEL_L3IF_HASH               g_pst_openflow_tunnel_master->openflow_tunnel_l3if_hash
#define HSRV_OPENFLOW_TUNNEL_OFPORT_HASH            g_pst_openflow_tunnel_master->openflow_tunnel_ofport_hash

#define HSRV_OPENFLOW_TUNNEL_FID_OPF                 g_pst_openflow_tunnel_master->fid_opf
#define HSRV_OPENFLOW_TUNNEL_METADATA_OPF            g_pst_openflow_tunnel_master->metadata_opf

#define HSRV_OPENFLOW_TUNNEL_DECAP_MODE              g_pst_openflow_tunnel_master->decap_mode
#define HSRV_OPENFLOW_TUNNEL_PORT_NUM                g_pst_openflow_tunnel_master->port_cnt
#define HSRV_OPENFLOW_TUNNEL_FID_NUM                 g_pst_openflow_tunnel_master->fid_cnt
#define HSRV_OPENFLOW_TUNNEL_METADATA_NUM            g_pst_openflow_tunnel_master->metadata_cnt
#define HSRV_OPENFLOW_TUNNEL_FLOW_NUM                g_pst_openflow_tunnel_master->flow_cnt
#define HSRV_OPENFLOW_TUNNEL_PORT_MAX                g_pst_openflow_tunnel_master->port_max
/* Added by weizj for tunnel bug 33958 */
#define HSRV_OPENFLOW_TUNNEL_VTEP_IP                 g_pst_openflow_tunnel_master->vtep_ip

/**
 * @brief Hsrv layer openflow tunnel add decap flow
 */
int32
hsrv_openflow_tunnel_add_decap_flow(glb_openflow_flow_t* p_flow);

/**
 * @brief Hsrv layer openflow tunnel remove decap flow
 */
int32
hsrv_openflow_tunnel_del_decap_flow(glb_openflow_flow_t* p_flow);

/**
 * @brief Hsrv layer openflow tunnel alloc tunnel nexthop
 */
int32
hsrv_openflow_tunnel_alloc_tunnel_nh(hsrv_ofnh_tunnel_param_t* p_param, hsrv_of_nh_info_t* p_mem_info);

/**
 * @brief Hsrv layer openflow tunnel release tunnel fid mapping
 */
int32
hsrv_openflow_tunnel_release_fid(uint32 tun_id);

/**
 * @brief Hsrv layer openflow tunnel get ofport and tun_id by metadata
 */
int32
hsrv_openflow_tunnel_get_info_by_metadata(uint32 meta_data, uint16* p_ofport, uint32* p_tun_id);

/**
 * @brief Hsrv layer openflow port is tunnel port
 */
bool
hsrv_openflow_port_is_tunnel(uint32 ifindex);

bool
hsrv_openflow_tunnel_get_link_status(uint32 ifindex);

int32
hsrv_openflow_tunnel_set_bond_slave_decap_en(uint32 ifindex, glb_openflow_tunnel_port_type_t type, uint32 enable);

int32
hsrv_openflow_tunnel_set_bond_slave_l3if_property(uint16 gport, uint32 enable);

/**
 * @brief Hsrv layer openflow tunnel show brief
 */
int32
hsrv_openflow_show_tunnel_brief(FILE *fp);

/**
 * @brief Hsrv layer openflow tunnel show port info all
 */
int32
hsrv_openflow_show_tunnel_port_all(FILE *fp);

/**
 * @brief Hsrv layer openflow tunnel show fid info all
 */
int32
hsrv_openflow_show_tunnel_fid_all(FILE *fp);

/**
 * @brief Hsrv layer openflow tunnel show metadata info all
 */
int32
hsrv_openflow_show_tunnel_metadata_all(FILE *fp);

/**
 * @brief Hsrv layer openflow tunnel start
 */
int32
hsrv_openflow_tunnel_start(void);

/**
 * @brief Hsrv layer openflow tunnel init
 */
int32
hsrv_openflow_tunnel_init(hsrv_system_spec_info_t* p_system_info);

int32
hsrv_openflow_tunnel_pacet_out_get_tunnel_type(glb_openflow_packet_out_param_t* p_param, glb_openflow_tunnel_port_type_t* type);

int32
hsrv_openflow_tunnel_packet_out_get_tunnel_nhid(glb_openflow_packet_out_param_t* p_param, uint32* p_nh_id, uint32* p_is_exist, uint16* p_phy_gport);

int32
hsrv_openflow_tunnel_packet_out_del_tunnel_nhid(uint32 nh_id);

#endif
