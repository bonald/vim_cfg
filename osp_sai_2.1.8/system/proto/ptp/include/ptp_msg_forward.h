
#ifndef __PTP_MSG_FORWARD_H__
#define __PTP_MSG_FORWARD_H__

void
ptp_residence_time_destroy_db(uint32 ifindex, uint32 destroy_all);

void
ptp_residence_time_age_db(void);

void
ptp_forward_register(tbl_ptp_port_t *p_port);

#endif /*!__PTP_MSG_FORWARD_H__*/
