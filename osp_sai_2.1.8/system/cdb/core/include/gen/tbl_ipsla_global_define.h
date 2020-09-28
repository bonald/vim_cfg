
#ifndef __TBL_IPSLA_GLOBAL_DEFINE_H__
#define __TBL_IPSLA_GLOBAL_DEFINE_H__

/* TBL_IPSLA_GLOBAL field defines */
typedef enum
{
    TBL_IPSLA_GLOBAL_FLD_IPSLA_ENTRY_NUM      = 0 ,  /* READ */
    TBL_IPSLA_GLOBAL_FLD_T_IPSLA_TIMER        = 1 ,  /* READ */
    TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_FAILURE    = 2 ,  /* RW */
    TBL_IPSLA_GLOBAL_FLD_TRAP_TEST_SUCCESS    = 3 ,  /* RW */
    TBL_IPSLA_GLOBAL_FLD_TRAP_PKT_TIMEOUT     = 4 ,  /* RW */
    TBL_IPSLA_GLOBAL_FLD_TRAP_OVERTHRESHOLD   = 5 ,  /* RW */
    TBL_IPSLA_GLOBAL_FLD_MAX                  = 6 
} tbl_ipsla_global_field_id_t;

/* TBL_IPSLA_GLOBAL defines */
typedef struct
{
    uint16               ipsla_entry_num;     /* configured ipsla entry number */
    ctc_task_t           *t_ipsla_timer;      /* IPSLA update timer */
    uint8                trap_test_failure;   /* trap_test_failure enable */
    uint8                trap_test_success;   /* trap_test_success enable */
    uint8                trap_pkt_timeout;    /* trap_pkt_timeout enable */
    uint8                trap_overthreshold;  /* trap_overthreshold enable */
} tbl_ipsla_global_t;

#endif /* !__TBL_IPSLA_GLOBAL_DEFINE_H__ */

