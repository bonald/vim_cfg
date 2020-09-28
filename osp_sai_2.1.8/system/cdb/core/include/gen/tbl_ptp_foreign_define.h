
#ifndef __TBL_PTP_FOREIGN_DEFINE_H__
#define __TBL_PTP_FOREIGN_DEFINE_H__

#include "gen/tbl_ptp_port_define.h"
/* TBL_PTP_FOREIGN field defines */
typedef enum
{
    TBL_PTP_FOREIGN_FLD_KEY                  = 0 ,  /* READ */
    TBL_PTP_FOREIGN_FLD_FOREIGN_MASTER_ANNOUNCE_MESSAGES = 1 ,  /* RW */
    TBL_PTP_FOREIGN_FLD_HEADER               = 2 ,  /* RW */
    TBL_PTP_FOREIGN_FLD_ANNOUNCE             = 3 ,  /* RW */
    TBL_PTP_FOREIGN_FLD_PTM_FM_QUALIFICATION_TIMER = 4 ,  /* RW */
    TBL_PTP_FOREIGN_FLD_PTM_FM_ANNOUNCE_RECEIPT_TIMER = 5 ,  /* RW */
    TBL_PTP_FOREIGN_FLD_QUALIFIED            = 6 ,  /* RW */
    TBL_PTP_FOREIGN_FLD_PORT_FOREIGN_NUM     = 7 ,  /* RW */
    TBL_PTP_FOREIGN_FLD_MAX                  = 8 
} tbl_ptp_foreign_field_id_t;

/* TBL_PTP_FOREIGN defines */
typedef ptp_foreign_master_t tbl_ptp_foreign_key_t;

typedef struct
{
    tbl_ptp_foreign_key_t key;
    uint16               foreign_master_announce_messages; /*recv announce msg count*/
    ptp_common_message_header_t header;              /*pkt header*/
    ptp_msg_announce_t   announce;            /*pkt*/
    ctc_task_t           *ptm_fm_qualification_timer; /*is foreign master stablelldp_msap_id_t*/
    ctc_task_t           *ptm_fm_announce_receipt_timer; /*announce msg counter, equal to rev timeout*/
    uint32               qualified;
    tbl_ptp_port_t       *p_port_belong_to;
    uint32               port_foreign_num;    /*volume at least 5, maybe use it to limit the size*/
} tbl_ptp_foreign_t;

typedef struct
{
    ctclib_hash_t        *ptp_foreign_hash;
    ctclib_slist_t       *ptp_foreign_list;
} tbl_ptp_foreign_master_t;

#endif /* !__TBL_PTP_FOREIGN_DEFINE_H__ */

