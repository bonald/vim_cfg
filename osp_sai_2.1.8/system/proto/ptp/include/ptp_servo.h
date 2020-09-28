
#ifndef __PTP_SERVO_H__
#define __PTP_SERVO_H__

void
ptp_servo_update_offset(tbl_ptp_port_t* p_port);

void
ptp_servo_update_delay(tbl_ptp_port_t* p_port);

void
ptp_servo_update_peerdelay(tbl_ptp_port_t* p_port);

void
ptp_servo_update_clock(tbl_ptp_port_t* p_port);

#endif /*!__PTP_SERVO_H__*/
