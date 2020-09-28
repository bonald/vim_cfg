#ifndef _G8131_API_H_
#define _G8131_API_H_

#include "bsd_queue.h"
#include "genlog.h"
#include "proto_debug.h"
#include "gen/tbl_opm_debug_define.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"

/*G8131 DEBUG*/
extern tbl_opm_debug_t *_g_p_tbl_opm_debug;
#define G8131_DBG_IS_ON(module, typeenum) \
    (_g_p_tbl_opm_debug->module & OPMDBG_FLAG_##typeenum)

#define G8131_DEBUG_PRINT(fmt, args...) \
    log_sys(M_MOD_G8131, E_DEBUG, fmt, ##args);

#define G8131_LOG_DEBUG(module, typeenum, fmt, args...) \
do { \
    if (G8131_DBG_IS_ON(module, typeenum)) \
        G8131_DEBUG_PRINT(fmt, ##args); \
} while(0)

#define G8131_TX_DEBUG(fmt, args...) \
do { \
    G8131_LOG_DEBUG(g8131, G8131_TX, fmt, ##args); \
} while(0)

#define G8131_RX_DEBUG(fmt, args...) \
do { \
    G8131_LOG_DEBUG(g8131, G8131_RX, fmt, ##args); \
} while(0)

#define G8131_EVENT_DEBUG(fmt, args...) \
do { \
    G8131_LOG_DEBUG(g8131, G8131_EVENT, fmt, ##args); \
} while(0)

/*G8131 DEBUG end*/


int32
g8131_start(void);

int32
g8131_aps_group_info_set_current_event(tbl_aps_group_info_t* p_aps_group_info);

int32
g8131_debug_aps_group_info_set_state_change(tbl_aps_group_info_t* p_aps_group_info, uint32 working_active);

int32
g8131_debug_aps_group_info_set_event_change(tbl_aps_group_info_t* p_aps_group_info);

int32
g8131_aps_group_info_initialize(tbl_aps_group_info_t* p_aps_group_info, uint32 revertive, uint32 bridge_mode, uint32 direction);

int32
g8131_reset_aps_group_info(tbl_aps_group_info_t* p_aps_group_info);

int32
g8131_is_sfp_reasserted(tbl_aps_group_info_t* p_db_ag_info);

int32
g8131_is_sf_reasserted(tbl_aps_group_info_t* p_db_ag_info);

int32
g8131_is_sdp_reasserted(tbl_aps_group_info_t* p_db_ag_info);

int32
g8131_is_sd_reasserted(tbl_aps_group_info_t* p_db_ag_info);

int32
g8131_is_sd_or_sdp_reasserted_by_oam(char* name, uint32 type);

int32
g8131_is_sf_or_sfp_reasserted_by_oam(char* name, uint32 type);

int32
g8131_get_link_state (char* name, uint32 type);

tbl_oam_session_t*
g8131_get_oam_by_aps_group(char* name, uint32 type);

tbl_aps_group_info_t*
g8131_get_aps_group_by_oam(tbl_oam_session_t* p_oam_session);

int32
g8131_tpoam_notify_status(tbl_oam_session_t* p_oam_session, uint32 status, uint32 old_status);

int32
g8131_aps_group_protection_en(tbl_aps_group_info_t* p_db_ag_info, uint32 protection_en);

int32
g8131_lsp_aps_group_protection_en(tbl_aps_group_info_t* p_db_ag_info, uint32 protection_en);

int32
g8131_pw_aps_group_protection_en(tbl_aps_group_info_t* p_db_ag_info, uint32 protection_en);

int32
g8131_pw_aps_group_update_protection(tbl_aps_group_info_t* p_db_ag_info);

int32
g8131_lsp_aps_group_update_protection(tbl_aps_group_info_t* p_db_ag_info);

char *
g8131_get_aps_type(tbl_aps_group_info_t* p_db_ag_info);

#endif /* _g8131_API_H_ */