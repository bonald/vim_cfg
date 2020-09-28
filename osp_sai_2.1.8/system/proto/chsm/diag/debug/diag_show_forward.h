/****************************************************************************
* 
*  Centec show forward related MACRO and ENUM, Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liul
* Date          : 2010-11-19 16:00
* Reason        : First Create.
****************************************************************************/

#ifndef _DIAG_SHOWL_FWD_H_
#define _DIAG_SHOWL_FWD_H_ 
/******************************************************************
 * include header files
******************************************************************/

#include "diag_show_forward_util_pkt.h"

/******************************************************************
 * defines constants
******************************************************************/
 
#define  SHOW_FORWARD_LOGGING_FILE "/tmp/show_forward.log"

#define SHOW_FORWARD_MTU                                ( 1500 +64 ) /* 64 bytes for forwarding headers */
#define PKT_LINE_LEN                 47 /* 16(hex) * 2 + 15 (blank) + 1 (reserve) */
#define PKT_LINE_MAX_BYTES     16 /* pkt every line max bytes is 16 */
#define ROOT_DIR_NAME              "/mnt/flash"

#define SHOW_FORWARD_IF_ERROR_RETURN(op) \
do { \
    int32 rv; \
    if ((rv = (op)) < 0) \
    {\
        cli_out(cli,"CHSM OP failed: rv = %d, %s:%d", rv, __FUNCTION__, __LINE__);\
        return(rv); \
    }\
}while(0)

struct diag_show_forward_common_header_s
{
    uint16 chip_id;
    uint16 chan_id;
};
typedef struct diag_show_forward_common_header_s diag_show_forward_common_header_t;

enum diag_show_forward_debug_switch_e
{
    DIAG_SHOW_FORWARD_MODULE,
    DIAG_SHOW_FORWARD_PKT,
    DIAG_SHOW_FORWARD_DEBUG
};

/* please modify the enum structure and make sure they are the in the same sequence in show_forward_debug_cmodel_sub_module_t */
enum diag_show_forward_debug_sub_module_e
{
   /*IPE module, 0*/
   DIAG_SHOW_FORWARD_IPE_HDR_ADJUST,
   DIAG_SHOW_FORWARD_IPE_PARSER,
   DIAG_SHOW_FORWARD_IPE_DECRYPTION,
   DIAG_SHOW_FORWARD_IPE_USERID,
   DIAG_SHOW_FORWARD_IPE_IF_MAPPER,
   DIAG_SHOW_FORWARD_IPE_LOOKUP,
   DIAG_SHOW_FORWARD_IPE_AGING,
   DIAG_SHOW_FORWARD_IPE_ACL_QOS,
   DIAG_SHOW_FORWARD_IPE_ROUTING,
   DIAG_SHOW_FORWARD_IPE_POLICING,

   /*10*/
   DIAG_SHOW_FORWARD_IPE_MPLS,                  /* 10 */
   DIAG_SHOW_FORWARD_IPE_BRIDGE,   
   DIAG_SHOW_FORWARD_IPE_STORM_CTRL,
   DIAG_SHOW_FORWARD_IPE_LEARNING,
   DIAG_SHOW_FORWARD_IPE_CLASSIFICATION,
   DIAG_SHOW_FORWARD_IPE_FORWARDING,
   DIAG_SHOW_FORWARD_IPE_MAX_MODULE,

   /*EPE module, */
   DIAG_SHOW_FORWARD_EPE_HDR_ADJUST,
   DIAG_SHOW_FORWARD_EPE_NEXT_HOP,
   DIAG_SHOW_FORWARD_EPE_PAYLOAD_PROCESS,
   DIAG_SHOW_FORWARD_EPE_L3_EDITING,        /*20*/
   DIAG_SHOW_FORWARD_EPE_L2_EDITING,
   DIAG_SHOW_FORWARD_EPE_ACL_QOS,
   DIAG_SHOW_FORWARD_EPE_CLASSIFICATION,
   DIAG_SHOW_FORWARD_EPE_HEADER_EDIT,
   DIAG_SHOW_FORWARD_EPE_MAX_MODULE,

   /*FWD module*/
   DIAG_SHOW_FORWARD_FWD_BUFF_STORE,    /*26*/
   DIAG_SHOW_FORWARD_FWD_GEN_ENQUE,
   DIAG_SHOW_FORWARD_FWD_GEN_QID,
   DIAG_SHOW_FORWARD_FWD_BUFF_RETRIEVE,
   DIAG_SHOW_FORWARD_FWD_MAX_MODULE,    /*30*/

   /*OAM module*/
   DIAG_SHOW_FORWARD_OAM_HDR_ADJUST,
   DIAG_SHOW_FORWARD_OAM_PARSER,
   DIAG_SHOW_FORWARD_OAM_LOOKUP,
   DIAG_SHOW_FORWARD_OAM_RX_PROCESS,
   DIAG_SHOW_FORWARD_OAM_UPDATE_FSM_PROCESS,
   DIAG_SHOW_FORWARD_OAM_HDR_EDIT_CTL,
   DIAG_SHOW_FORWARD_OAM_MAX_MODULE
};
typedef enum diag_show_forward_debug_sub_module_e diag_show_forward_debug_sub_module_t;

 enum diag_show_forward_debug_out_pkt_e
{
   DIAG_SHOW_FORWARD_INPUT_PKT,
   DIAG_SHOW_FORWARD_QMGT_PKT,
   DIAG_SHOW_FORWARD_OUTPUT_PKT,
   DIAG_SHOW_FORWARD_OAM_PKT,
   DIAG_SHOW_FORWARD_MAX_PKT
};
typedef enum diag_show_forward_debug_out_pkt_e diag_show_forward_out_pkt_t;

/******************************************************************
     * functions
 ******************************************************************/

extern int32
diag_show_forward_construct_pkt(FILE *fp, uint8 *line, diag_pkt_t *fpkt, diag_ctrl_var_t *ctrl_var, diag_config_t *config);
extern int32 
chsm_lcm_do_show_forward_ipe(tbl_card_t* lc,  diag_pkt_t *in_pkt);
extern int32 
chsm_lcm_do_show_forward_epe(tbl_card_t* lc, diag_pkt_t *in_pkt);
extern int32 
chsm_lcm_send_show_forward_flags(uint32_t module, uint64 flags);
extern int32
diag_show_forward_recv_ack(diag_pkt_t  * ipeout_pkt);

#endif /*_DIAG_SHOWL_FWD_H_*/

