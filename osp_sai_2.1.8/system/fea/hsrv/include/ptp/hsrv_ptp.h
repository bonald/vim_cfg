#ifndef __HSRV_SYNC_PTP_H__
#define __HSRV_SYNC_PTP_H__

#define HSRV_PTP_PTR_CHECK(ptr) GLB_PTR_VALID_CHECK(ptr, HSRV_E_INVALID_PTR)
#define HSRV_PTP_E_RET HSRV_IF_ERROR_RETURN
#define HSRV_PTP_RETRY_TIMES 10
#define HSRV_PTP_RETRY_INTERVAL 50

int32
hsrv_ptp_sync_systime();

int32
hsrv_ptp_start();

int32
hsrv_ptp_tx_discard_cb(void);

int32
_hsrv_ptp_tx_ts_isr(uint8 gchip, void* p_data);

#endif /* !__HSRV_SYNC_PTP_H__ */
