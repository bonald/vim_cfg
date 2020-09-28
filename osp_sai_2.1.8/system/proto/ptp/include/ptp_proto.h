
#ifndef __PTP_PROTO_H__
#define __PTP_PROTO_H__

void
ptp_announce_receipt_time_restart(tbl_ptp_port_t* p_port);

void
ptp_qualification_start(tbl_ptp_port_t* p_port);

void
ptp_proto_to_state(tbl_ptp_port_t* p_port);

#endif /*!__PTP_PROTO_H__*/
