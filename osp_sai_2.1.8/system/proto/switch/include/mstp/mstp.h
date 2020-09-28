
#ifndef __MSTP_H__
#define __MSTP_H__

int32
mstp_init();

int32
mstp_start(switch_master_t *master);

int32
mstp_stop(switch_master_t *master);

#endif /* !__MSTP_H__ */

