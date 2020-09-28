
#ifndef __TBL_FEA_PCAP_DEFINE_H__
#define __TBL_FEA_PCAP_DEFINE_H__

/* TBL_FEA_PCAP field defines */
typedef enum
{
    TBL_FEA_PCAP_FLD_TX_EN                = 0 ,  /* RW */
    TBL_FEA_PCAP_FLD_RX_EN                = 1 ,  /* RW */
    TBL_FEA_PCAP_FLD_IFINDEX              = 2 ,  /* RW */
    TBL_FEA_PCAP_FLD_MAX                  = 3 
} tbl_fea_pcap_field_id_t;

/* TBL_FEA_PCAP defines */
typedef struct
{
    uint32               tx_en;
    uint32               rx_en;
    uint32               ifindex;
} tbl_fea_pcap_t;

#endif /* !__TBL_FEA_PCAP_DEFINE_H__ */

