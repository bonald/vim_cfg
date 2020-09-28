#ifndef __LSRV_CARD_H__
#define __LSRV_CARD_H__



int32
lsrv_card_get_local_slot_no();

int32
lsrv_card_get_card(lai_card_info_t* card);

int32
lsrv_card_reboot();

void
lsrv_card_init(void* data);

void
lsrv_card_event_cb(lai_card_event_t  event,lai_card_info_t* p_card);

#endif /* !__LSRV_CARD_H__ */

