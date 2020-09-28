
#ifndef __PTP_ADPT_H__
#define __PTP_ADPT_H__

#include "hsrv_msg.h"

void
ptp_adpt_set_bits_clock_output(uint32 enable);

void
ptp_adpt_set_bits_clock_output_mode(uint32 enable);

void
ptp_adpt_set_sync_intf_output(uint32 enable);

void
ptp_adpt_set_ptp_acl_multicast(void);

void
ptp_adpt_adjust_offset(int64 offset);

int
ptp_nsm_unicast_arp_add(uint32 dest_ip, uint32 ifindex, uint32 is_add);

int32
ptp_adpt_set_tocpu_vlan(uint32 vid, uint32 add);

int32
ptp_adpt_set_global_enable(uint32 enable);

int32
ptp_adpt_switch_device_type(uint32 dev_type);

void
ptp_adpt_set_path_delay(tbl_ptp_port_t *p_db_ptp_port, int64 path_delay);

int32
ptp_adpt_get_tx_ts(uint32 ifindex, glb_ptp_timestamp_t* p_tx_ts,  uint16 seq_id, char *ifname);

int32
ptp_adpt_reset_tx_ts(void);

int32
ptp_adpt_update_system_time(int16 utc_offset);

int32
ptp_adpt_set_tod_pulse_delay_correction(int32 pulse_delay);

int32
ptp_adpt_get_hardware_second(int32* second, int32* nanoseconds);

#endif /*!__PTP_ADPT_H__*/

