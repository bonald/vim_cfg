#ifndef __ARP_SM_H__
#define __ARP_SM_H__


void
arp_main_aging_timeout(void* p_data);

int32
arp_sm(tbl_arp_t *ae_p, const arp_event_t event);

int32
arp_main_retry_timer_add(tbl_arp_t *ae_p);

int32
arp_main_get_retry_time(uint32 ifindex, uint32 *time);

#endif /* !__ARP_SM_H__ */
