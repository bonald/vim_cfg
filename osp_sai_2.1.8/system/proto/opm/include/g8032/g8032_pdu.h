#ifndef _G8032_PDU_H_
#define _G8032_PDU_H_

typedef struct
{
   uint16_t pkt_type;
   uint8_t  mel_version;
   uint8_t  opcode;
   uint8_t  flags;   
   uint8_t  tlv_offset;
}raps_pdu_hdr_t;

typedef struct
{
   uint32_t     ring_id;
   uint8_t      not_flush_fdb;
}raps_pdu_tx_t;

typedef struct
{
   uint16_t pkt_type;
   
}raps_pdu_type_t;

typedef struct
{
   uint8_t  state_sub;
   uint8_t  status;
   uint8_t  node_id[6];   
}raps_info_t;



typedef struct
{
    uint8  dest_mac[MAC_ADDR_LEN];
    uint8  src_mac[MAC_ADDR_LEN];
    uint16 ether_type;
} raps_eth_hdr_t;

struct raps_vlan_hdr_s
{
    uint16 tag;
    uint16 proto;
};
typedef struct raps_vlan_hdr_s raps_vlan_hdr_t;

#define G8032_ENCODE_PUTC(V)                                                    \
    do {                                                                      \
      *(*pnt)     = (V) & 0xFF;                                               \
      (*pnt)++;                                                               \
      (*size)--;                                                              \
    } while (0)

#define G8032_ENCODE_PUT(P,L)                                                   \
    do {                                                                      \
      sal_memcpy ((void *)(*pnt), (void *) (P), (L));                        \
      *pnt += (L);                                                            \
      *size -= (L);                                                           \
    } while (0)

#define G8032_ENCODE_PUT_EMPTY(L)                                               \
    do {                                                                      \
      sal_memset ((void *) (*pnt), 0, (L));                                  \
      *pnt += (L);                                                            \
      *size -= (L);                                                           \
    } while (0)

#define G8032_RAPS_LENGTH          55
#define G8032_RAPS_DEST_MAC {0x01, 0x19, 0xA7, 0x00, 0x00, 0x01}
#define G8032_RAPS_OPCODE          0x28
#define G8032_INVALID_VAL             0
#define G8032_RAPS_FIRST_TLV_OFFSET   32
#define G8032_RESERVED_PDU        24 
#define G8032_RAPS_PDU_VERSION        0x01

int32
g8032_ring_pkt_fun(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

int32
g8032_ring_pkt_tx_raps_sf(raps_pdu_tx_t  *p_raps_pdu_tx);

int32
g8032_ring_pkt_tx_raps_fs(raps_pdu_tx_t  *p_raps_pdu_tx);

int32
g8032_ring_pkt_tx_raps_ms(raps_pdu_tx_t  *p_raps_pdu_tx);

int32
g8032_ring_pkt_tx_raps_nr(raps_pdu_tx_t  *p_raps_pdu_tx);

int32
g8032_ring_pkt_tx_raps_nr_rb(raps_pdu_tx_t  *p_raps_pdu_tx);

int32
g8032_ring_pkt_tx_raps_event(tbl_g8032_ring_t *p_g8032_ring);
#endif
