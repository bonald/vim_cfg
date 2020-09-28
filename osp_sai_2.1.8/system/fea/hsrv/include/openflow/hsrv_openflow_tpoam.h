#ifndef _HSRV_OPENFLOW_TPOAM_H_
#define _HSRV_OPENFLOW_TPOAM_H_

int32
hsrv_aps_group_offset_alloc(uint32* p_aps_offset);

int32
hsrv_aps_group_offset_release(uint32 offset);

int32
hsrv_openflow_show_tpoam(FILE* fp, char** argv, int32 argc);

int32
hsrv_tpoam_update_pw_mep_nh_by_groupid(uint32   lsp_aps_group_id);

#endif /* _HSRV_OPENFLOW_TPOAM_H_ */
