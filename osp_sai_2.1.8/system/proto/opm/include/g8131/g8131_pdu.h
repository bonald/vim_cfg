#ifndef _G8131_PDU_H_
#define _G8131_PDU_H_

struct g8131_aps_pdu_s
{
    uint32  req_state;
    uint32  A;
    uint32  B;
    uint32  D;
    uint32  R;
    uint32  req_signal;
    uint32  bridged_signal;
    uint32  T;
    uint32  reserved;
};
typedef struct g8131_aps_pdu_s g8131_aps_pdu_t;


#define G8131_ENCODE_PUT(P, L) \
do { \
    sal_memcpy((void*)(*ppnt), (void*)(P), (L)); \
    *ppnt += (L); \
    *size -= (L); \
} while (0)


#define G8131_ENCODE_PUTW(V) \
do { \
    *(*ppnt) = ((V) >> 8) & 0xFF; \
    *(*ppnt + 1) = (V) & 0xFF; \
    *ppnt += 2; \
    *size -= 2; \
} while (0)

#define G8131_ENCODE_PUTC(V) \
do { \
    *(*ppnt) = (V) & 0xFF; \
    (*ppnt)++; \
    (*size)--; \
} while (0)

#define G8131_ENCODE_PUT_EMPTY(L) \
do { \
    sal_memset((void*)(*ppnt), 0, (L)); \
    *ppnt += (L); \
    *size -= (L); \
} while (0)

#define G8131_DECODE_GETC(V) \
do { \
    (V) = **ppnt; \
    (*ppnt)++; \
    (*size)--; \
} while (0)

#define G8131_DECODE_GETW(V) \
do { \
    (V) = ((*(*ppnt)) << 8) \
        + (*(*ppnt + 1)); \
    *ppnt += 2; \
    *size -= 2; \
} while (0)

#define G8131_DECODE_GETL(V) \
do { \
    (V) = ((uint32_t)(*(*ppnt)) << 24) \
        + ((uint32_t)(*(*ppnt + 1)) << 16) \
        + ((uint32_t)(*(*ppnt + 2)) << 8) \
        + (uint32_t)(*(*ppnt + 3)); \
    *ppnt += 4; \
    *size -= 4; \
} while (0)

#define G8131_DECODE_GET(V, L) \
do { \
    sal_memcpy((void*)(V), (const void*)(*ppnt), (L)); \
    *ppnt += (L); \
    *size -= (L); \
} while (0)

#define G8131_DECODE_GET_EMPTY(L) \
do { \
    *ppnt += (L); \
    *size -= (L); \
} while (0)


void
g8131_pdu_aps_tx_timer_handler(void *p_data);

int32
g8131_pdu_aps_tx(tbl_aps_group_info_t * p_db_ag_info);

int32
g8131_pdu_aps_rx(tbl_oam_session_t * p_oam_session, tpoam_rx_pdu_t* pst_pdu);

void 
g8131_pdu_out_state_change_timer_handler (void *p_data);

void 
g8131_pdu_wtr_timer_handler (void *p_data);

void 
g8131_pdu_sdp_hold_off_timer_handler (void *p_data);

void 
g8131_pdu_sd_hold_off_timer_handler (void *p_data);

void 
g8131_pdu_sfp_hold_off_timer_handler (void *p_data);

void
g8131_pdu_sf_hold_off_timer_handler (void *p_data);

void 
g8131_dfop(tbl_aps_group_info_t * p_db_ag_info, uint32 defect);

#endif /* _G8131_PDU_H_ */
