#ifndef __LSRV_SYNCE_H__
#define __LSRV_SYNCE_H__
 
struct lsrv_synce_s
{
    bool global_enable;
    bool is_recovering;
    uint16 recovery_port;
};
typedef struct lsrv_synce_s lsrv_synce_t;

int32
lsrv_synce_start();

void
lsrv_synce_dpll_state_event_cb(lai_synce_event_t event, lai_synce_info_t* p_synce_info);

#endif /* !__LSRV_SYNCE_H_ */
