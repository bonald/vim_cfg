#ifndef __FEI_OPENFLOW_TPOAM_H__
#define __FEI_OPENFLOW_TPOAM_H__

int32
fei_flow_add_tpoam_flow(glb_openflow_tpoam_t* p_flow);

int32
fei_flow_del_tpoam_flow(glb_openflow_tpoam_t* p_flow);

int32
fei_flow_update_tpoam_flow(glb_openflow_tpoam_t* p_flow);

int32
tpoam_api_get_oam_flow(glb_openflow_tpoam_t  *oam_flow, tbl_oam_session_t* p_db_oam_session);

int32
tpoam_api_get_fei_key(tbl_oam_session_t* p_oam_session, glb_openflow_tpoam_t  *p_oam_flow);

int32
fei_set_pw_mode(uint32* p_value);
#endif /* !__FEI_OPENFLOW_TPOAM_H__ */
