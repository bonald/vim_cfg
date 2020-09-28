#ifndef __SSM_PDU_H__
#define __SSM_PDU_H__

/* Size of PDUs transmitted/received (includes 14 byte frame header) */
#define SSMDU_FRAME_SIZE        64

#define SFLOW_PRO_ETHERTYPE     0x8809
#define SFLOW_PRO_SUBTYPE       0x0A
    
#define SSM_OUI_VALUE1          0x00
#define SSM_OUI_VALUE2          0x19
#define SSM_OUI_VALUE3          0xa7

#define SSM_ITU_SUBTYPE         0x0001

/*information pdu */
#define SSM_INFORMATION_PDU     0x10

/*event pdu */
#define SSM_EVENT_PDU           0x18

#define SSM_REVERT_VALUE1       0x00
#define SSM_REVERT_VALUE2       0x00
#define SSM_REVERT_VALUE3       0x00


/*TLV type */
#define SSM_TLV_TYPE            0x01

/*EXTENT TLV type */
#define SSM_TLV_EX_TYPE         0x02

/*TLV lenth */
#define SSM_TLV_LEN             0x0004

/*EXTENT TLV lenth */
#define SSM_TLV_EX_LEN          0x000c


#define SSM_ENCODE_PUT(P, L) \
do { \
    sal_memcpy((void*)(*ppnt), (void*)(P), (L)); \
    *ppnt += (L); \
    *size -= (L); \
} while (0)


#define SSM_ENCODE_PUTW(V) \
do { \
    *(*ppnt) = ((V) >> 8) & 0xFF; \
    *(*ppnt + 1) = (V) & 0xFF; \
    *ppnt += 2; \
    *size -= 2; \
} while (0)

#define SSM_ENCODE_PUTC(V) \
do { \
    *(*ppnt) = (V) & 0xFF; \
    (*ppnt)++; \
    (*size)--; \
} while (0)

#define SSM_ENCODE_PUT_EMPTY(L) \
do { \
    sal_memset((void*)(*ppnt), 0, (L)); \
    *ppnt += (L); \
    *size -= (L); \
} while (0)

#define SSM_DECODE_GETC(V) \
do { \
    (V) = **ppnt; \
    (*ppnt)++; \
    (*size)--; \
} while (0)

#define SSM_DECODE_GETW(V) \
do { \
    (V) = ((*(*ppnt)) << 8) \
        + (*(*ppnt + 1)); \
    *ppnt += 2; \
    *size -= 2; \
} while (0)

#define SSM_DECODE_GETL(V) \
do { \
    (V) = ((uint32_t)(*(*ppnt)) << 24) \
        + ((uint32_t)(*(*ppnt + 1)) << 16) \
        + ((uint32_t)(*(*ppnt + 2)) << 8) \
        + (uint32_t)(*(*ppnt + 3)); \
    *ppnt += 4; \
    *size -= 4; \
} while (0)

#define SSM_DECODE_GET(V, L) \
do { \
    sal_memcpy((void*)(V), (const void*)(*ppnt), (L)); \
    *ppnt += (L); \
    *size -= (L); \
} while (0)

#define SSM_DECODE_GET_EMPTY(L) \
do { \
    *ppnt += (L); \
    *size -= (L); \
} while (0)


void
ssm_pdu_information_send_trigger(void *p_data);

int32
ssm_pdu_event_send_trigger();

#endif /* !__SSM_PDU_H__ */
