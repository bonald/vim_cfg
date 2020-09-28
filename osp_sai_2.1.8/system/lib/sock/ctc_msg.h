
#ifndef __CTC_MSG_H__
#define __CTC_MSG_H__

#define BIT_IS_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1 : 0)
#define BIT_SET(flag, bit)      ((flag) = (flag) | (1 << (bit)))
#define BIT_UNSET(flag, bit)    ((flag) = (flag) & (~(1 << (bit))))

#define MSG_ENCODE_PUTC(V) \
do { \
    *(*ppnt) = (V) & 0xFF; \
    (*ppnt)++; \
    (*size)--; \
} while (0)

#define MSG_ENCODE_PUTW(V) \
do { \
    *(*ppnt) = ((V) >> 8) & 0xFF; \
    *(*ppnt + 1) = (V) & 0xFF; \
    *ppnt += 2; \
    *size -= 2; \
} while (0)

#define MSG_ENCODE_PUTL(V) \
do { \
    *(*ppnt) = ((V) >> 24) & 0xFF; \
    *(*ppnt + 1) = ((V) >> 16) & 0xFF; \
    *(*ppnt + 2) = ((V) >> 8) & 0xFF; \
    *(*ppnt + 3) = (V) & 0xFF; \
    *ppnt += 4; \
    *size -= 4; \
} while (0)

#define MSG_ENCODE_PUT_EMPTY(L) \
do { \
    sal_memset((void*)(*ppnt), 0, (L)); \
    *ppnt += (L); \
    *size -= (L); \
} while (0)

#define MSG_ENCODE_PUT(P, L) \
do { \
    sal_memcpy((void*)(*ppnt), (void*)(P), (L)); \
    *ppnt += (L); \
    *size -= (L); \
} while (0)

#define MSG_DECODE_GETC(V) \
do { \
    (V) = **ppnt; \
    (*ppnt)++; \
    (*size)--; \
} while (0)

#define MSG_DECODE_GETW(V) \
do { \
    (V) = ((*(*ppnt)) << 8) \
        + (*(*ppnt + 1)); \
    *ppnt += 2; \
    *size -= 2; \
} while (0)

#define MSG_DECODE_GETL(V) \
do { \
    (V) = ((uint32_t)(*(*ppnt)) << 24) \
        + ((uint32_t)(*(*ppnt + 1)) << 16) \
        + ((uint32_t)(*(*ppnt + 2)) << 8) \
        + (uint32_t)(*(*ppnt + 3)); \
    *ppnt += 4; \
    *size -= 4; \
} while (0)

#define MSG_DECODE_GET(V, L) \
do { \
    sal_memcpy((void*)(V), (const void*)(*ppnt), (L)); \
    *ppnt += (L); \
    *size -= (L); \
} while (0)

#define MSG_DECODE_GET_EMPTY(L) \
do { \
    *ppnt += (L); \
    *size -= (L); \
} while (0)

#define MAX_FLD_NUM         64
#define MAX_FLD_DWORD_SZ    (MAX_FLD_NUM/32)

#define MSG_VER                 (0)
#define MSG_SIZE                (10240)
#define MSG_HDR_SIZE            (8)
#define MSG_HDR_CONNECT_SIZE    (4)
#define MSG_HDR_SYNC_SIZE       (8)
#define MSG_HDR_CFG_SIZE        (12)
#define MSG_HDR_DATA_SIZE       (16)
#define MSG_HDR_ACT_SIZE        (4)
#define MSG_HDR_FEI_SIZE        (8)
#define MSG_HDR_PKT_SIZE        (16)
#define MSG_HDR_LCM_SIZE        (4)
#define MSG_HDR_SUBSCRIBE_SIZE  (16)
#define MSG_HDR_SUB_REQ_SIZE    (0)
#define MSG_HDR_SUB_RESP_SIZE   (0)
#define MSG_HDR_BHM_SIZE        (4)

#define MSG_HAL_FLAGS_SYNC      1

#define MSG_BUILD_HDR(_msg_, _oper_, _datalen_) \
do { \
    (_msg_)->hdr.version = MSG_VER; \
    (_msg_)->hdr.operation = (_oper_); \
    (_msg_)->hdr.length = MSG_HDR_SIZE + ctc_msg_get_uhdr_len((_oper_)) + (_datalen_); \
} while(0)

#define MSG_BUILD1_HDR(_msg_, _oper_, _datalen_, _data1len_) \
do { \
    (_msg_)->hdr.version = MSG_VER; \
    (_msg_)->hdr.operation = (_oper_); \
    (_msg_)->hdr.length = MSG_HDR_SIZE + ctc_msg_get_uhdr_len((_oper_)) + (_datalen_) + (_data1len_); \
} while(0)

#define MSG_BUILD2_HDR(_msg_, _oper_, _datalen_, _data1len_, _data2len_) \
do { \
    (_msg_)->hdr.version = MSG_VER; \
    (_msg_)->hdr.operation = (_oper_); \
    (_msg_)->hdr.length = MSG_HDR_SIZE + ctc_msg_get_uhdr_len((_oper_)) + (_datalen_) + (_data1len_) + (_data2len_); \
} while(0)

extern const char *g_ipc_dest_desc[];

typedef enum msg_oper_e
{
    MSG_OPER_CONNECT = 0,   /* for build connection message */
    MSG_OPER_SYNC,          /* for CDB sync control message */
    MSG_OPER_CFG,           /* for configuration message */
    MSG_OPER_DATA,          /* for PM's IPC message to sync data */
    MSG_OPER_FEI,           /* for FEI message to FEA */
    MSG_OPER_PKT,           /* for PDU message to FEA's PETH */
    MSG_OPER_LCM,           /* for message between chsm and lcm */
    MSG_OPER_SUBSCRIBE,     /* for subscribe TBLs's changes */
    MSG_OPER_SUB_REQ,       /* for subscribe request */
    MSG_OPER_SUB_RESP,      /* for subscribe response TBLs's changes */
    MSG_OPER_BHM            /* for BHM */
} msg_oper_t;

typedef struct
{
    uint8       version;    /* version */
    uint8       operation;  /* msg_oper_t */
    uint16      length;
    uint32      seq;        /* sequence number */
} msg_hdr_t;

typedef struct
{
    uint16      src;        /* pm_id_t */
    uint16      pid;
} msg_connect_hdr_t;

typedef struct
{
    uint16      oper;       /* cdb_sync_t */
    uint8       dest;       /* pm_id_t */
    uint8       src;        /* pm_id_t */
    uint32      seq;        /* sequence number */
} msg_sync_hdr_t;

typedef struct
{
    uint8       dest;       /* pm_id_t */
    uint8       src;        /* pm_id_t */
    uint16      pid;
    uint8       oper;       /* cdb_oper_t */
    uint8       type;       /* cdb_node_type_t */
    uint16      id;         /* cdb_dir_t / cdb_tbl_t / cdb_ds_t */
    uint32      cfg;        /* cdb_cfg_t */
} msg_cfg_hdr_t;

typedef struct
{
    uint8       oper;                       /* cdb_oper_t */
    uint8       type;                       /* cdb_node_type_t */
    uint16      tbl_id;                     /* cdb_tbl_t */
    uint16      ds_id;                      /* cdb_ds_t */
    uint16      ds2_id;                     /* cdb_ds_t */
    uint32      fields[MAX_FLD_DWORD_SZ];   /* now uses fields[0] as field_id, later will change to fields bitmap */
} msg_data_hdr_t;

typedef struct
{
    uint32      type;
    uint32      flags;
} msg_fei_hdr_t;

typedef struct
{
    uint16      ifindex;
    uint16      vid;
    uint32      type;       /* glb_pkt_type_t */
    uint32      sub_type;
    uint32      flags;      /* GLB_PKT_FLAGS_XXX */
} msg_pkt_hdr_t;

typedef struct
{
    uint32      slot;
} msg_lcm_hdr_t;

typedef struct
{
    uint8       type;                     /* cdb_subscribe_type_t */
    uint8       node_type;                /* cdb_node_type_t for sub cdb, or sync for sub tables */
    uint16      tbl_id;                   /* cdb_tbl_t */
    uint16      ds_id;                    /* cdb_ds_t */
    uint16      tbl_num;
    uint32      field;
    uint16      pid;
    uint16      src;
} msg_subscribe_hdr_t;

typedef struct
{
    uint16      src;        /* pm_id_t */
    uint16      pid;
} msg_bhm_hdr_t;

typedef union
{
    msg_connect_hdr_t   connect;
    msg_sync_hdr_t      sync;
    msg_cfg_hdr_t       cfg;
    msg_data_hdr_t      data;
    msg_fei_hdr_t       fei;
    msg_pkt_hdr_t       pkt;
    msg_lcm_hdr_t       lcm;
    msg_subscribe_hdr_t sub;
    msg_bhm_hdr_t       bhm;
} msg_union_hdr_t;

typedef struct
{
    msg_hdr_t           hdr;
    msg_union_hdr_t     u_hdr;
    uint32              data_len;
    void               *data;
    uint32              data1_len;
    void               *data1;
    uint32              data2_len;
    void               *data2;
    uint32              peer;
} ctc_msg_t;

typedef enum ctc_dec_rc_e
{
    MSG_DEC_OK,            /* Decoded successfully */
    MSG_DEC_MORE,          /* More data expected, call again */
    MSG_DEC_FAIL           /* Failure to decode data */
} msg_dec_rc_t;

typedef struct
{
    uint32      code;           /* enum msg_dec_rc_t */
    uint32      consumed;       /* Number of bytes consumed */
} msg_dec_ret_t;

/* PM_ID_MAX */
#define IPC_DEST_DESC(_dest_)   \
    ((_dest_) < 23) ? g_ipc_dest_desc[(_dest_)] : "invalid"

extern const char *g_ipc_dest_desc[];

typedef int32 (*SOCK_RX_FN)(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

uint32
ctc_msg_get_uhdr_len(msg_oper_t oper);

const char*
ctc_msg_oper_desc(msg_oper_t oper);

msg_dec_ret_t
ctc_msg_decode_hdr(uint8* data, uint32 length, ctc_msg_t* p_msg);

int32
ctc_msg_decode(void* p_sess, int32 read_len);

int32
ctc_msg_dump(ctc_msg_t *p_msg, uint32 send);

int32
ctc_msg_free(ctc_msg_t *p_msg);

#endif /* !__CTC_MSG_H__ */

