
#ifndef __TBL_IPSLA_TEST_STATS_DEFINE_H__
#define __TBL_IPSLA_TEST_STATS_DEFINE_H__

/* TBL_IPSLA_TEST_STATS field defines */
typedef enum
{
    TBL_IPSLA_TEST_STATS_FLD_KEY                  = 0 ,  /* READ */
    TBL_IPSLA_TEST_STATS_FLD_LAST_SEND_TIME       = 1 ,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_TESTID               = 2 ,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_SEND_CNT             = 3 ,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_RECEIVE_CNT          = 4 ,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_OVERTHRESHOLD_CNT    = 5 ,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_SENDFAIL_CNT         = 6 ,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_RTT_MIN              = 7 ,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_RTT_MAX              = 8 ,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_RTT_SUM              = 9 ,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_SUCCESS              = 10,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_FAIL_PERCENT         = 11,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_TIMEOUT              = 12,  /* RW */
    TBL_IPSLA_TEST_STATS_FLD_MAX                  = 13
} tbl_ipsla_test_stats_field_id_t;

/* TBL_IPSLA_TEST_STATS defines */
typedef ipsla_test_stats_key_t tbl_ipsla_test_stats_key_t;

typedef struct
{
    tbl_ipsla_test_stats_key_t key;
    uint64               last_send_time;
    uint32               testid;
    uint8                send_cnt;
    uint8                receive_cnt;
    uint8                overthreshold_cnt;
    uint8                sendfail_cnt;
    uint32               rtt_min;
    uint32               rtt_max;
    uint32               rtt_sum;
    uint8                success;
    uint8                fail_percent;
    uint16               timeout;
} tbl_ipsla_test_stats_t;

typedef struct
{
    ctclib_hash_t        *ipsla_test_stats_hash;
    ctclib_slist_t       *ipsla_test_stats_list;
} tbl_ipsla_test_stats_master_t;

#endif /* !__TBL_IPSLA_TEST_STATS_DEFINE_H__ */

