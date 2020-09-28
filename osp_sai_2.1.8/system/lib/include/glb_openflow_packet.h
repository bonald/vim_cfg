#ifndef __GLB_OPENFLOW_PACKET_DEFINE_H__
#define __GLB_OPENFLOW_PACKET_DEFINE_H__

/**************************  packet out ****************************************/
enum glb_openflow_packet_out_flag_e
{
    GLB_OPENFLOW_MPLS_NH_LABEL_IS_SET_VLAN = (1 << 0), 
};
typedef enum glb_openflow_packet_out_flag_e glb_openflow_packet_out_flag_t;

enum glb_openflow_packet_out_type_e
{
    GLB_OPENFLOW_PACKET_OUT_TYPE_NORMAL,
    GLB_OPENFLOW_PACKET_OUT_TYPE_TUNNEL,
    GLB_OPENFLOW_PACKET_OUT_TYPE_INPUT,

    GLB_OPENFLOW_PACKET_OUT_TYPE_MAX
};
typedef enum glb_openflow_packet_out_type_e glb_openflow_packet_out_type_t;

struct glb_openflow_packet_out_param_s
{
    glb_openflow_packet_out_type_t type;
    glb_openflow_packet_out_flag_t flag;
    uint32 ifindex;
    uint32 tunnel_id;
    int32 packet_len;
    uint32 vlan_id;
};
typedef struct glb_openflow_packet_out_param_s glb_openflow_packet_out_param_t;
#endif /*! __GLB_OPENFLOW__PACKET_DEFINE_H__ */

