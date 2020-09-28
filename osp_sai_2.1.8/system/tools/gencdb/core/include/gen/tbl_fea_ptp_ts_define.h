
#ifndef __TBL_FEA_PTP_TS_DEFINE_H__
#define __TBL_FEA_PTP_TS_DEFINE_H__

/* TBL_FEA_PTP_TS field defines */
typedef enum
{
    TBL_FEA_PTP_TS_FLD_KEY                  = 0 ,  /* READ */
    TBL_FEA_PTP_TS_FLD_INTR_IFINDEX         = 1 ,  /* RW */
    TBL_FEA_PTP_TS_FLD_NS                   = 2 ,  /* RW */
    TBL_FEA_PTP_TS_FLD_S_LSB                = 3 ,  /* RW */
    TBL_FEA_PTP_TS_FLD_S_MSB                = 4 ,  /* RW */
    TBL_FEA_PTP_TS_FLD_MAX                  = 5 
} tbl_fea_ptp_ts_field_id_t;

/* TBL_FEA_PTP_TS defines */
typedef struct
{
    uint16               intr_seq_id;
} tbl_fea_ptp_ts_key_t;

typedef struct
{
    tbl_fea_ptp_ts_key_t key;
    uint32               intr_ifindex;
    uint32               ns;
    uint32               s_lsb;
    uint32               s_msb;
} tbl_fea_ptp_ts_t;

typedef struct
{
    ctclib_hash_t        *fea_ptp_ts_hash;
    ctclib_slist_t       *fea_ptp_ts_list;
} tbl_fea_ptp_ts_master_t;

#endif /* !__TBL_FEA_PTP_TS_DEFINE_H__ */

