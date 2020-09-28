
#ifndef __HSRV_WB__H__
#define __HSRV_WB__H__

#define LRANGE_MIN_INDEX    0
#define LRANGE_MAX_INDEX    -1

int32
hsrv_wb_sdb_restore_done();

int32
hsrv_wb_do_reboot();

int32
hsrv_wb_init();

int32
hsrv_wb_cmd_init();

extern uint32
ctc_sai_get_wb_restoring();

#endif /* __HSRV_WB__H__ */

