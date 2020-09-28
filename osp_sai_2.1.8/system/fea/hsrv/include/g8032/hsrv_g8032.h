#ifndef __HSRV_G8032_H__
#define __HSRV_G8032_H__
#include "glb_tempfile_define.h"
#include "hsrv_inc.h"

typedef struct hsrv_g8032_group_key_s
{
    uint32 aps_group_id;
    uint32 nhid;
    uint32 nhoffset;
} hsrv_g8032_group_key_t;

typedef struct hsrv_g8032_ring_if_s
{
    ctclib_list_node_t node;
    uint32 ifindex;
    hsrv_g8032_group_key_t wg;

    uint8 pg_is_switching;
    uint8 pad;
    uint16 protecting_time;
} hsrv_g8032_ring_if_t;

enum hsrv_g8032_nh_type_e
{
    HSRV_G8032_NH_TYPE_NH_NONE,
    HSRV_G8032_NH_TYPE_NH_MCAST,
    HSRV_G8032_NH_TYPE_NH_ECMP,
    HSRV_G8032_NH_TYPE_NH_FLEX,
    HSRV_G8032_NH_TYPE_NH_FLEX_CPU,
    HSRV_G8032_NH_TYPE_NH_IPUC,
    HSRV_G8032_NH_TYPE_NH_IP_TUNNEL,
    HSRV_G8032_NH_TYPE_NH_IP_TUNNEL_WITH_TUN_ID,
    HSRV_G8032_NH_TYPE_NH_MPLS,
    HSRV_G8032_NH_TYPE_NH_XLATE,
    HSRV_G8032_NH_TYPE_NH_ILOOP,
    HSRV_G8032_NH_TYPE_NH_DROP,
    HSRV_G8032_NH_TYPE_MAX /**< Type Max */
};
typedef enum hsrv_g8032_nh_type_e hsrv_g8032_nh_type_t;

struct hsrv_g8032_nh_offset_s
{
    uint32 nhid;
    /*for reserved nh id, offset and nh_type will not be used*/
    uint32 offset;
    hsrv_g8032_nh_type_t nh_type;
        
    bool port_check_discard; /* If set 1, discard the packet if it's destination port is same with ingress port */
    bool redirect_to_ingress_port; /* If set 1, redirect the packet to the ingress port
                                      and the port_check_discard flag is nonsens in this case. */
    bool stats_en;
    bool is_resvd;
    uint32 stats_id;
};
typedef struct hsrv_g8032_nh_offset_s hsrv_g8032_nh_offset_t;

int32
hsrv_g8032_ring_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id,  void *p_tbl, void *p_ds);

#endif

