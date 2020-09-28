#ifndef __LCSH_LCM_TLK_H__
#define __LCSH_LCM_TLK_H__

#include "lcapi_lcm_lcsh_msg.h"

extern int32 
lcsh_lcm_msg_tx_test1(lcapi_lcm_lcsh_test1_msg_t* pst_test);

extern int32
lcsh_lcm_msg_tx_read_regs(read_regs_reg_t* pst_read_req, lcapi_lcm_lcsh_msg_t* pst_st_rcv);
int32
lcsh_lcm_msg_tx_write_regs(write_regs_reg_t* write_req, lcapi_lcm_lcsh_msg_t* st_rcv);
int32
lcsh_lcm_msg_tx_get_port_info(lcapi_lcm_lcsh_msg_t* st_req, showport_t* buf);
int32
lcsh_lcm_msg_tx_show_transceiver(uint8 detail);
int32
lcsh_lcm_msg_tx_write_soc_reg(write_soc_reg_t* write_req, lcapi_lcm_lcsh_msg_t* st_rcv);
int32
lcsh_lcm_msg_tx_read_soc_reg(read_soc_reg_t* read_req, lcapi_lcm_lcsh_msg_t* st_rcv);

#endif /*__LCSH_LCM_TLK_H__*/
