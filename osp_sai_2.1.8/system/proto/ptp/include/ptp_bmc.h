
#ifndef __PTP_BMC_H__
#define __PTP_BMC_H__

void
ptp_bmc_update_m1(tbl_ptp_port_t* p_port);

void
ptp_bmc_update_m3(tbl_ptp_port_t* p_port);

void
ptp_bmc_update_s1(tbl_ptp_port_t* p_port, ptp_common_message_header_t* hdr, ptp_msg_announce_t* msg);

void
ptp_bmc_execute(void);

int32
ptp_bmc_add_foreign(tbl_ptp_port_t* p_port, ptp_sk_buff_t* skb);

#define ptp_bmc_cleanup_foreign_db ptp_bmc_destroy_foreign_db

void
ptp_bmc_destroy_foreign_db(tbl_ptp_port_t *p_port);

#endif /*!__PTP_BMC_H__*/
