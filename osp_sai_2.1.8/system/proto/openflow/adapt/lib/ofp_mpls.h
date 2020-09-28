#ifndef __OFP_MPLS_H__
#define __OFP_MPLS_H__

#define OFP_MPLS_LABEL_MASK 0xfffff
#define OFP_MPLS_TC_MASK 0x7

enum ofp_mpls_type_e
{
    OPF_MPLS_TYPE_NONE, 
    OPF_MPLS_TYPE_VPWS,
    OPF_MPLS_TYPE_VPLS,
    OPF_MPLS_TYPE_NORMAL,
    OPF_MPLS_TYPE_POP,
    OPF_MPLS_TYPE_SWAP,
    OPF_MPLS_TYPE_L3VPN,
    OPF_ACTION_TYPE_MAX /**< Type Max */
};
typedef enum ofp_mpls_type_e ofp_mpls_type_t;

struct ofp_mpls_info_s
{
    /* match field related */
    uint32_t        mpls_label;

    /* mpls action related */
#if 0 /* should be removed. */
    bool            is_mpls;
    bool            is_mpls_pop;
    ofp_mpls_type_t mpls_service_type;
#endif
    glb_openflow_mpls_output_action_seq_type_t   mpls_output_action_seq_type;
    uint8_t         mpls_output_count;
    uint16_t mpls_output[GLB_OPENFLOW_MAX_OUTPUT_PER_FLOW];
};
typedef struct ofp_mpls_info_s ofp_mpls_info_t;

#endif /* !__OFP_MPLS_H__ */
