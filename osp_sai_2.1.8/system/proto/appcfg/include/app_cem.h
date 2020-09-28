#ifndef __APP_CEM_H__
#define __APP_CEM_H__




int32
_app_event_manager_add_cfg(tbl_cem_t *p_event_manager);

int32
_app_event_manager_del_cfg(tbl_cem_key_t *p_event_manager_key);

int32
app_cem_set_event(tbl_cem_t *p_event_manager);

int32
app_cem_set_loadpath(tbl_cem_t *p_event_manager);

int32
app_cem_set_mail(tbl_cem_t *p_event_manager);

int32
app_cem_set_snmptrap(tbl_cem_t *p_event_manager);

int32
app_event_manager_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_cem_init();    
#endif /* !__APP_CEM_H__ */

