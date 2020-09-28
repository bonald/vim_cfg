#ifndef __TPOAM_SOCK_H__
#define __TPOAM_SOCK_H__


/* MACRO for TX OAM PDU thought mpls.ko */
#define TPOAM_SOCK_DIRECTION_PE     0
#define TPOAM_SOCK_LM_FLAG_SET      2
#define TPOAM_SOCK_DM_FLAG_SET      1
#define TPOAM_SOCK_DM_FLAG_UNSET    0
#define TPOAM_SOCK_TTL_DEF          TPOAM_TTL_DEF


#define GLB_MPLSTP_OAM_SECTION_1DM_DMM_OFFSET 12
#define GLB_MPLSTP_OAM_SECTION_DMR_OFFSET     28

#define GLB_MPLSTP_OAM_LSP_1DM_DMM_OFFSET     12
#define GLB_MPLSTP_OAM_LSP_DMR_OFFSET         28

#define GLB_MPLSTP_OAM_PW_1DM_DMM_OFFSET      8
#define GLB_MPLSTP_OAM_PW_DMR_OFFSET          24


struct mpls_label_s
{
    uint32 label:20;
    uint32 exp:3;
    uint32 s:1;
    uint32 ttl:8;    
};
typedef struct mpls_label_s mpls_label_t;

tpoam_sk_buff_t *
tpoam_alloc_skb(uint32 size);

void
tpoam_free_skb(tpoam_sk_buff_t** skb);

int32
tpoam_packet_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

int32
tpoam_sock_tx_new(tbl_oam_session_t* pst_oam, uint32 direction, uint32 pkt_size, tpoam_sk_buff_t* tx_skb, 
    uint32 exp, uint32 ttl, uint32 flag, uint32 is_dmr, uint32 is_bypass, uint32 pkt_type);

#endif /* !__TPOAM_SOCK_H__ */
