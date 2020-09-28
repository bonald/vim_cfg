#ifndef __GLB_RMT_H__
#define __GLB_RMT_H__

#define AF_RMT AF_X25 /* reuse AF_X25 */
#define PF_RMT AF_RMT

struct sockaddr_rmt {
    sa_family_t fam;
    uint8_t group;

    union {
        /* sender */
        struct {
            uint32_t txqlen;
            uint32_t ratelim;
            uint32_t burstlim;
        };

        /* receiver */
        struct {
            uint32_t rxqlen;
            uint32_t initseq;
        };
    };
};

struct rmt_dump {

    /* sk info */
    uint32_t sender;
    uint32_t group;
    uint32_t s_maxqlen;
    uint32_t s_ratelim;
    uint32_t s_burstlim;
    uint32_t s_nextseq;
    uint32_t r_maxqlen;
    uint32_t r_initseq;
    uint32_t r_lastseq;
    uint32_t r_nextseq;
    uint32_t r_nack_retries;
        
    /* RX */
    uint32_t rx_all;
    uint32_t rx_nack;
    uint32_t rx_data;
    uint32_t rx_idle;
    uint32_t rx_qlen;

    /* RX Error */    
    uint32_t nack_rx_group;
    uint32_t nack_rx_length;
    uint32_t data_rx_group;
    uint32_t data_rx_length;
    uint32_t data_rx_2old;
    uint32_t data_rx_overflow;
    uint32_t data_rx_queue_fail;
    uint32_t data_rx_overflow_r_queue;
    uint32_t data_rx_queue_fail_r_queue;
    uint32_t data_rx_duplicated;
    uint32_t idle_rx_group;
    uint32_t idle_rx_2old;
    uint32_t rcv_skb_share_check;
    uint32_t rcv_skb_may_pull;
    uint32_t rcv_skb_rmt_type;
};

/* DST_MAC: <FF:FF:FF:FF:FF:group> */
#define RMT_GROUP_MIN 1
#define RMT_GROUP_MAX 254

#define RMT_QLEN_MIN 0x1000

#define RMT_IOC_GETSEQ SIOCPROTOPRIVATE
#define RMT_IOC_DUMP (SIOCPROTOPRIVATE + 1)
#define RMT_IOC_S_START (SIOCPROTOPRIVATE + 2)
#ifdef __KERNEL__
#define ETH_P_RMT ETH_P_X25 /* reuse ETH_P_X25 */

struct rmt_hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
    __u8    reserved:4,
            ver:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
    __u8    ver:4,
            reserved:4;
#else
#error "Please fix <asm/byteorder.h>"
#endif

    __u8    type;
    __u16   len;
    __u32   seq;
};

#define RMTHDRLEN sizeof(struct rmt_hdr)

/* Protocol version */
#define RMT_VER 1

/* Payload type */
#define RMT_DATA 1
#define RMT_IDLE 0x80
#define RMT_NACK 0x81
#define RMT_ACK  0x82
#endif /* !__KERNEL__ */

#endif /* !__GLB_RMT_H__ */
