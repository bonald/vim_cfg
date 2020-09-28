#ifndef __TPOAM_PDU_H__
#define __TPOAM_PDU_H__

#define GLB_MPLS_TP_ALERT_LABEL          13

#define GLB_MPLS_OAM_PW_MAC_WD_TYPE       0x0028

/* Version */
#define TPOAM_DEF_VERSION           0

/* MACRO for LABEL parser */
#define TPOAM_MPLS_LABEL_BYTE_WIDTH         4

#define TPOAM_Y1731_PDU_MEL_SHIFT           0x05
#define TPOAM_Y1731_PDU_MEL_MASK            0x07
#define TPOAM_Y1731_PDU_VER                 0x00
#define TPOAM_Y1731_PDU_VER_MASK            0x1F
#define TPOAM_Y1731_PDU_RDI_SHIFT           0x07
#define TPOAM_Y1731_PDU_RDI_MASK            0x01
#define TPOAM_Y1731_PDU_FLAG_NONE           0x00

/* First TLV Offset defines */
#define TPOAM_Y1731_CCM_FIRST_TLV_OFFSET        70
#define TPOAM_Y1731_LBM_LBR_FIRST_TLV_OFFSET    4
#define TPOAM_Y1731_AIS_FIRST_TLV_OFFSET        0
#define TPOAM_Y1731_LCK_FIRST_TLV_OFFSET        0
#define TPOAM_Y1731_CSF_FIRST_TLV_OFFSET        0
#define TPOAM_Y1731_1DM_FIRST_TLV_OFFSET        16
#define TPOAM_Y1731_DMM_DMR_FIRST_TLV_OFFSET    32
#define TPOAM_Y1731_LMM_LMR_FIRST_TLV_OFFSET    12
#define TPOAM_Y1731_APS_FIRST_TLV_OFFSET        4

#define TPOAM_TEST_MODE_0_NULL_WO_CRC   0
#define TPOAM_TEST_MODE_1_NULL_WI_CRC   1
#define TPOAM_TEST_MODE_2_PRBS_WO_CRC   2
#define TPOAM_TEST_MODE_3_PRBS_WI_CRC   3

#define TPOAM_PDU_LB_INDICATION_LBM     0
#define TPOAM_PDU_LB_INDICATION_LBR     1

/* ITU-T Y.1731 Table 9-2 */
/* ITU-T G.8113.1/G.tpoam Table 8-3 */
enum tpoam_y1731_pdu_tlv_e
{
    TPOAM_Y1731_PDU_END_TLV                     = 0,
    TPOAM_Y1731_PDU_DATA_TLV                    = 3,
    TPOAM_Y1731_PDU_REPLY_INGRESS_TLV           = 5,
    TPOAM_Y1731_PDU_REPLY_EGRESS_TLV            = 6,
    TPOAM_Y1731_PDU_LTM_EGRESS_IDENTIFIER_TLV   = 7,
    TPOAM_Y1731_PDU_LTR_EGRESS_IDENTIFIER_TLV   = 8,
    TPOAM_Y1731_PDU_TEST_TLV                    = 32,
    TPOAM_Y1731_PDU_TARGET_MEP_MIP_ID_TLV       = 33,
    TPOAM_Y1731_PDU_REPLY_MEP_MIP_ID_TLV        = 34,
    TPOAM_Y1731_PDU_REQUEST_MEP_ID_TLV          = 35,
};
typedef enum tpoam_y1731_pdu_tlv_e tpoam_y1731_pdu_tlv_t;

/* ITU-T Y.1731 Table 9-1 */
enum tpoam_y1731_pdu_opcode_e
{
    TPOAM_Y1731_PDU_CCM     = 1,
    TPOAM_Y1731_PDU_LBR     = 2,
    TPOAM_Y1731_PDU_LBM     = 3,
    TPOAM_Y1731_PDU_LTR     = 4,
    TPOAM_Y1731_PDU_LTM     = 5,
    TPOAM_Y1731_PDU_AIS     = 33,
    TPOAM_Y1731_PDU_LCK     = 35,
    TPOAM_Y1731_PDU_TST     = 37,
    TPOAM_Y1731_PDU_APS     = 39,
    TPOAM_Y1731_PDU_G8032   = 40,
    TPOAM_Y1731_PDU_MCC     = 41,
    TPOAM_Y1731_PDU_LMR     = 42,
    TPOAM_Y1731_PDU_LMM     = 43,
    TPOAM_Y1731_PDU_1DM     = 45,
    TPOAM_Y1731_PDU_DMR     = 46,
    TPOAM_Y1731_PDU_DMM     = 47,
    TPOAM_Y1731_PDU_EXR     = 48,
    TPOAM_Y1731_PDU_EXM     = 49,
    TPOAM_Y1731_PDU_VSR     = 50,
    TPOAM_Y1731_PDU_VSM     = 51,
    TPOAM_Y1731_PDU_CSF     = 52,
    TPOAM_PW_MAC_WITHDRAW   = 0xFF    /* added by huwx for PW MAC-WD, 
                                         must diff value with Y.1731 Table define*/
};
typedef enum tpoam_y1731_pdu_opcode_e tpoam_y1731_pdu_opcode_t;

enum tpoam_y1731_pdu_lb_sub_type_e
{
    TPOAM_Y1731_PDU_LB_SUB_TYPE_DISCOVERY_INGRESS_NODE = TPOAM_LB_DISCOVERY,
    TPOAM_Y1731_PDU_LB_SUB_TYPE_DISCOVERY_EGRESS = TPOAM_LB_DISCOVERY_EGRESS,
    TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MEP_ID = TPOAM_LB_MEP,
    TPOAM_Y1731_PDU_LB_SUB_TYPE_ICC_BASED_MIP_ID = TPOAM_LB_MIP
};
typedef enum tpoam_y1731_pdu_lb_sub_type_e tpoam_y1731_pdu_lb_sub_type_t;

#define TPOAM_GET_LABEL_ID(_label_) \
(((_label_) >> 12) & 0xFFFFF)

#define TPOAM_GET_LABEL_EXP(_label_) \
(((_label_) >> 9) & 0x7)

#define TPOAM_GET_LABEL_S(_label_) \
(((_label_) >> 8) & 0x1)

#define TPOAM_GET_LABEL_TTL(_label_) \
((_label_) & 0xFF)


#define TLV_ENCODE_PUTW(V)                                                    \
    do {                                                                      \
      *(*pnt)     = ((V) >> 8) & 0xFF;                                        \
      *(*pnt + 1) = (V) & 0xFF;                                               \
      *pnt += 2;                                                              \
      *size -= 2;                                                             \
    } while (0)


#define TLV_ENCODE_PUTC(V)                                                    \
    do {                                                                      \
      *(*pnt)     = (V) & 0xFF;                                               \
      (*pnt)++;                                                               \
      (*size)--;                                                              \
    } while (0)


#define TLV_ENCODE_PUT_EMPTY(L)                                               \
    do {                                                                      \
      sal_memset ((void *) (*pnt), 0, (L));                                  \
      *pnt += (L);                                                            \
      *size -= (L);                                                           \
    } while (0)

#define TLV_DECODE_GET(P,L)                                                   \
    do {                                                                      \
      sal_memcpy ((void *) (P), *pnt, (L));                                  \
      *pnt += (L);                                                            \
      *size -= (L);                                                           \
    } while (0)

#define TLV_DECODE_GETW(V)                                                    \
    do {                                                                      \
      (V) = ((*(*pnt))    << 8)                                               \
          +  (*(*pnt + 1));                                                   \
      *pnt += 2;                                                              \
      *size -= 2;                                                             \
    } while (0)


#define DECODE_GET_EMPTY(LEN) \
        do {                  \
        *pnt += (LEN); \
        *size -= (LEN); \
        } while (0)
        
#define TLV_DECODE_GETC(V)                                                    \
    do {                                                                      \
      (V) = **pnt;                                                            \
      (*pnt)++;                                                               \
      (*size)--;                                                              \
    } while (0)

#define TLV_DECODE_GETL(V)                                                    \
    do {                                                                      \
      (V) = ((uint32_t)(*(*pnt))     << 24)                                             \
          + ((uint32_t)(*(*pnt + 1)) << 16)                                             \
          + ((uint32_t)(*(*pnt + 2)) << 8)                                              \
          +  (uint32_t)(*(*pnt + 3));                                                   \
      *pnt += 4;                                                              \
      *size -= 4;                                                             \
    } while (0)

#define TLV_DECODE_GETLL(V)                                                    \
        do {                                                                      \
          (V) = ((uint64_t)(*(*pnt))     << 56)                                             \
              + ((uint64_t)(*(*pnt + 1)) << 48)                                             \
              + ((uint64_t)(*(*pnt + 2)) << 40)                                              \
              + ((uint64_t)(*(*pnt + 3)) << 32)                                             \
              + ((uint64_t)(*(*pnt + 4)) << 24)                                              \
              + ((uint64_t)(*(*pnt + 5)) << 16)                                             \
              + ((uint64_t)(*(*pnt + 6)) << 8)                                              \
              +  (uint64_t)(*(*pnt + 7));                                                   \
          *pnt += 8;                                                              \
          *size -= 8;                                                             \
        } while (0)

        
#define TLV_DECODE_SKIP(L)                                                    \
    do {                                                                      \
      *pnt += (L);                                                            \
      *size -= (L);                                                           \
    } while (0)

#define TLV_DECODE_REWIND(L)                                                 \
    do {                                                                      \
      *pnt -= (L);                                                            \
      *size += (L);                                                           \
    } while (0)

#define TLV_ENCODE_PUT(P,L)                                                   \
    do {                                                                      \
      sal_memcpy ((void *)(*pnt), (void *) (P), (L));                        \
      *pnt += (L);                                                            \
      *size -= (L);                                                           \
    } while (0)

    
#define TLV_ENCODE_PUTL(V)                                                    \
    do {                                                                      \
      *(*pnt)     = ((V) >> 24) & 0xFF;                                       \
      *(*pnt + 1) = ((V) >> 16) & 0xFF;                                       \
      *(*pnt + 2) = ((V) >> 8) & 0xFF;                                        \
      *(*pnt + 3) = (V) & 0xFF;                                               \
      *pnt += 4;                                                              \
      *size -= 4;                                                             \
    } while (0)


/* MPLS-TP OAM Y.1731 Header decode defines */
struct tpoam_y1731_hdr_s
{
    uint32 mel:3;
    uint32 ver:5;
    uint32 opcode:8;
    uint32 flags:8;
    uint32 tlv_offset:8;    
};
typedef struct tpoam_y1731_hdr_s tpoam_y1731_hdr_t;

struct mac_wd_tlv_header_bits
{
    uint16 u_bit :1;
    uint16 f_bit :1;
    uint16 type :14;
};

struct mac_wd_tlv_header
{
    struct mac_wd_tlv_header_bits bits;
    uint16 length;
    uint8 flags;
    uint8* p_data;
};

struct tpoam_pw_mac_wd_hdr_s
{
    uint32 reserved:16;
    uint32 left_len:8;
    uint32 ack:1;
    uint32 flags:7;
    uint32 u_b:1;
    uint32 f_b:1;
    uint32 seq_num_tlv:14;
    uint32 seq_num_len:16;
    uint32 seq_num:32;
    struct mac_wd_tlv_header  mac_list_tlv;
    struct mac_wd_tlv_header  mac_flush_para_tlv;
};
typedef struct tpoam_pw_mac_wd_hdr_s tpoam_pw_mac_wd_hdr_t;

/* MPLS-TP OAM Y.1731 PDU decode defines */
struct tpoam_rx_pdu_s
{
    uint32  service_type;
    mpls_label_t label;
    mpls_label_t outer_label;
    uint32          label_num;
    uint32          ifindex;
    uint16          vid;
    uint16          channel_type;
    uint32          len;
    uint32          y1731_len;
    int32           left_len;
    tpoam_y1731_hdr_t   hdr;
    tpoam_pw_mac_wd_hdr_t   mac_wd_hdr; /*added by huwx for pw oam mac wd, 2012-9-10 */
    uint8           lb_sub_type;
    uint8           direction;      /* used for LSP-P etc., is meaningless for LSP-PE etc. */
    uint32          lb_trans_id;    /* LBM of LBR */
    uint8*          p_head;
    uint8*          p_y1731_head;
    uint8*          p_pw_mac_wd_head; /*added by huwx for pw oam mac wd, 2012-9-10 */
    uint8*          p_data;
    tpoam_interval_t    delay;
    uint32          TxFcf;
    uint32          RxFcf;
    uint32          TxFcb;
    uint32          RxFcl;
    uint32          priority;
};
typedef struct tpoam_rx_pdu_s tpoam_rx_pdu_t;

int32 
tpoam_pdu_rx_pdu (tpoam_rx_pdu_t* pst_pdu, uint32 had_paser_label, uint32 oam_session_id, uint32 pw_type, uint32 lsp_type);

#endif /* !__TPOAM_PDU_H__ */
