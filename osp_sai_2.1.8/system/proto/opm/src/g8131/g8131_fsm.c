#include "proto.h"
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"
#include "gen/tbl_lsp_aps_group_define.h"
#include "gen/tbl_lsp_aps_group.h"
#include "gen/tbl_pw_aps_group_define.h"
#include "gen/tbl_pw_aps_group.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"
#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "glb_g8131_define.h"
#include "glb_tp_oam.h"
#include "tpoam_sock.h"
#include "tpoam_pdu.h"
#include "g8131_api.h"
#include "g8131_fsm.h"
#include "g8131_pdu.h"
#include "fei.h"
#include "fei_g8131.h"

struct
{
   int32 (* func)();
   int32 next_state;
} G8131FSM[G8131_FSM_STATE_MAX][G8131_FSM_EVENT_MAX] =
{
    /* G8131_FSM_STATE_NR_W - 0 */
    {
      /* Local Events */
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_LO},    /*Lockout*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_FS},    /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},  /*Signal Fail on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_W},  /*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},  /*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_W},  /*Protection recovers from SF*/
      {g8131_fsm_local_handle_sd_w,        G8131_FSM_STATE_SD_W},  /*Signal Degrade on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_W},  /*Working recovers from SD*/
      {g8131_fsm_local_handle_sd_p,        G8131_FSM_STATE_SD_P},  /*Signal Degrade on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_W},  /*Protection recovers from SD*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_MS_P},  /*Manual Switch to protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_MS_W},  /*Manual Switch to working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_W},  /*Clear*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_EXER_W},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_W},  /*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_nochange,                 G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Force switch*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Signal Fail on working*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Signal Degrade on Working*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_NR_W},/*Signal Degrade on Protection*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Manual Switch to protection*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_NR_W},/*Manual Switch to Working*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Wait-to-Restore*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_RR_W},/*Exercise on working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_W},/*Exercise on protection*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_NR_W},/*Reverse Request on working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_W},/*Reverse Request on protection*/
      {g8131_fsm_far_nr_w_handle_nr_null,  G8131_FSM_STATE_NR_W},/*No Request, r/b = null*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_NR_W},/*No Request, r/b = normal*/
      {g8131_fsm_far_nr_w_handle_dnr,      G8131_FSM_STATE_NR_P},/*Do Not Revert*/
    },

    /* G8131_FSM_STATE_NR_P -1 */
    {
      /* Local Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_FS},  /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},/*Signal Fail on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_NR_P},/*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},/*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_P},/*Protection recovers from SF*/
      {g8131_fsm_local_handle_sd_w,        G8131_FSM_STATE_SD_W},/*Signal Degrade on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_NR_P},/*Working recovers from SD*/
      {g8131_fsm_local_handle_sd_p,        G8131_FSM_STATE_SD_P},/*Signal Degrade on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_P},/*Protection recovers from SD*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_MS_P},/*Manual Switch to protection*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_MS_W},/*Manual Switch to working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_P},/*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_NR_P},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_P},/*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_NR_P},/*Force switch*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_NR_P},/*Signal Fail on working*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_NR_P},/*Signal Degrade on Working*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Degrade on Protection*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_NR_P},/*Manual Switch to protection*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Manual Switch to Working*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_NR_P},/*Wait-to-Restore*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_P},/*Exercise on working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_P},/*Exercise on protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_P},/*Reverse Request on working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_NR_P},/*Reverse Request on protection*/
      {g8131_fsm_far_nr_p_handle_nr_null,  G8131_FSM_STATE_NR_W},/*No Request, r/b = null*/
      {g8131_fsm_far_nr_p_handle_nr_normal,G8131_FSM_STATE_NR_W},/*No Request, r/b = normal*/
      {g8131_fsm_far_nr_p_handle_dnr,      G8131_FSM_STATE_NR_P},/*Do Not Revert*/
    },

    /* G8131_FSM_STATE_LO -2 */
    {
      /* Local Events */
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Forced Switch*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Signal Fail on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Working recovers from SF*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Signal Fail on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Protection recovers from SF*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Signal Degrade on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Working recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Protection recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Manual Switch to working*/
      {g8131_fsm_local_lo_handle_clear,    G8131_FSM_STATE_NR_W},/*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_LO},  /*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_nochange,                 G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Signal Fail on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Force switch*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Signal Fail on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Signal Degrade on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Manual Switch to Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Wait-to-Restore*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Exercise on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Exercise on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Reverse Request on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Reverse Request on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*No Request, r/b = null*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*No Request, r/b = normal*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_LO},  /*Do Not Revert*/
    },

    /* G8131_FSM_STATE_FS -3 */
    {
      /* Local Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Forced Switch*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Signal Fail on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},/*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_FS},  /*Protection recovers from SF*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Signal Degrade on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Working recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Protection recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Manual Switch to working*/
      {g8131_fsm_local_fs_handle_clear,    G8131_FSM_STATE_NR_W},/*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_FS},  /*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_FS},  /*Force switch*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Signal Fail on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Signal Degrade on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Manual Switch to Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Wait-to-Restore*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Exercise on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Exercise on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Reverse Request on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Reverse Request on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*No Request, r/b = null*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*No Request, r/b = normal*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_FS},  /*Do Not Revert*/
    },

    /* G8131_FSM_STATE_SF_W -4 */
    {
      /* Local Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_FS},  /*Forced Switch*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SF_W},/*Signal Fail on Working*/
      {g8131_fsm_local_sf_w_handle_wrsf,   G8131_FSM_STATE_WTR}, /*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},/*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SF_W},/*Protection recovers from SF*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Signal Degrade on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Working recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Protection recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Manual Switch to working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SF_W},/*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SF_W},/*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Force switch*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_SF_W},/*Signal Fail on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Signal Degrade on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Manual Switch to Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Wait-to-Restore*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Exercise on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Exercise on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Reverse Request on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Reverse Request on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*No Request, r/b = null*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*No Request, r/b = normal*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_W},/*Do Not Revert*/
    },

    /* G8131_FSM_STATE_SF_P -5 */
    {
      /* Local Events */
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Forced Switch*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Signal Fail on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Working recovers from SF*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SF_P},/*Signal Fail on Protection*/
      {g8131_fsm_local_sf_p_handle_prsf,   G8131_FSM_STATE_NR_W},/*Protection recovers from SF*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Signal Degrade on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Working recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Protection recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Manual Switch to working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SF_P},/*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SF_P},/*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_SF_P},/*Signal Fail on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Force switch*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Signal Fail on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Signal Degrade on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Manual Switch to Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Wait-to-Restore*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Exercise on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Exercise on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Reverse Request on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Reverse Request on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*No Request, r/b = null*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*No Request, r/b = normal*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SF_P},/*Do Not Revert*/
    },

    /* G8131_FSM_STATE_SD_W -6 */
    {
      /* Local Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_FS},  /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},/*Signal Fail on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SD_W},/*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},/*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SD_W},/*Protection recovers from SF*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SD_W},/*Signal Degrade on Working*/
      {g8131_fsm_local_sd_w_handle_wrsd,   G8131_FSM_STATE_WTR}, /*Working recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Protection recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Manual Switch to working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SD_W},/*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SD_W},/*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Force switch*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Signal Fail on working*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_SD_W},/*Signal Degrade on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Manual Switch to Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Wait-to-Restore*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Exercise on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Exercise on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Reverse Request on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Reverse Request on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*No Request, r/b = null*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*No Request, r/b = normal*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_W},/*Do Not Revert*/
    },

    /* G8131_FSM_STATE_SD_P -7 */
    {
      /* Local Events */
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_FS},  /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},/*Signal Fail on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SD_P},/*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},/*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SD_P},/*Protection recovers from SF*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Signal Degrade on Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Working recovers from SD*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SD_P},/*Signal Degrade on Protection*/
      {g8131_fsm_local_sd_p_handle_prsd,   G8131_FSM_STATE_NR_W},/*Protection recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Manual Switch to working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SD_P},/*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_SD_P},/*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Force switch*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Signal Fail on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Signal Degrade on Working*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_SD_P},/*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Manual Switch to Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Wait-to-Restore*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Exercise on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Exercise on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Reverse Request on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Reverse Request on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*No Request, r/b = null*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*No Request, r/b = normal*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_SD_P},/*Do Not Revert*/
    },

    /* G8131_FSM_STATE_MS_P -8 */
    {
      /* Local Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_FS},  /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},/*Signal Fail on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_MS_P},/*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},/*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_MS_P},/*Protection recovers from SF*/
      {g8131_fsm_local_handle_sd_w,        G8131_FSM_STATE_SD_W},/*Signal Degrade on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_MS_P},/*Working recovers from SD*/
      {g8131_fsm_local_handle_sd_p,        G8131_FSM_STATE_SD_P},/*Signal Degrade on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_MS_P},/*Protection recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_P},/*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_P},/*Manual Switch to working*/
      {g8131_fsm_local_ms_p_handle_clear,  G8131_FSM_STATE_NR_W},/*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_P},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_MS_P},/*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Force switch*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Signal Fail on working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Signal Degrade on Working*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Degrade on Protection*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_MS_P},/*Manual Switch to protection*/
      {g8131_fsm_far_ms_p_handle_ms_w,     G8131_FSM_STATE_MS_P},/*Manual Switch to Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_P},/*Wait-to-Restore*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_P},/*Exercise on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_P},/*Exercise on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_P},/*Reverse Request on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_P},/*Reverse Request on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_P},/*No Request, r/b = null*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_P},/*No Request, r/b = normal*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_P},/*Do Not Revert*/
    },

    /* G8131_FSM_STATE_MS_W -9 */
    {
      /* Local Events */
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_FS},  /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},/*Signal Fail on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_MS_W},/*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},/*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_MS_W},/*Protection recovers from SF*/
      {g8131_fsm_local_handle_sd_w,        G8131_FSM_STATE_SD_W},/*Signal Degrade on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_MS_W},/*Working recovers from SD*/
      {g8131_fsm_local_handle_sd_p,        G8131_FSM_STATE_SD_P},/*Signal Degrade on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_MS_W},/*Protection recovers from SD*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*Manual Switch to protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*Manual Switch to working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_MS_W},/*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Force switch*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Signal Fail on working*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Signal Degrade on Working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Signal Degrade on Protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*Manual Switch to protection*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_MS_W},/*Manual Switch to Working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*Wait-to-Restore*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*Exercise on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*Exercise on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*Reverse Request on working*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*Reverse Request on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*No Request, r/b = null*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*No Request, r/b = normal*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_MS_W},/*Do Not Revert*/
    },

    /* G8131_FSM_STATE_WTR -10 */
    {
      /* Local Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_FS},  /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},/*Signal Fail on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_WTR}, /*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},/*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_WTR}, /*Protection recovers from SF*/
      {g8131_fsm_local_handle_sd_w,        G8131_FSM_STATE_SD_W},/*Signal Degrade on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_WTR}, /*Working recovers from SD*/
      {g8131_fsm_local_handle_sd_p,        G8131_FSM_STATE_SD_P},/*Signal Degrade on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_WTR}, /*Protection recovers from SD*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_MS_P},/*Manual Switch to protection*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_MS_W},/*Manual Switch to working*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_WTR}, /*Exercise*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Force switch*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Signal Fail on working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Signal Degrade on Working*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Degrade on Protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Manual Switch to protection*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Manual Switch to Working*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_WTR}, /*Wait-to-Restore*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_WTR}, /*Exercise on working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_WTR}, /*Exercise on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_WTR}, /*Reverse Request on working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_WTR}, /*Reverse Request on protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_WTR}, /*No Request, r/b = null*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_WTR}, /*No Request, r/b = normal*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_WTR}, /*Do Not Revert*/
    },

    /* G8131_FSM_STATE_DNR -11 */
    {
      /* Local Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_LO},  /*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_FS},  /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},/*Signal Fail on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_DNR}, /*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},/*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_DNR}, /*Protection recovers from SF*/
      {g8131_fsm_local_handle_sd_w,        G8131_FSM_STATE_SD_W},/*Signal Degrade on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_DNR}, /*Working recovers from SD*/
      {g8131_fsm_local_handle_sd_p,        G8131_FSM_STATE_SD_P},/*Signal Degrade on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_DNR}, /*Protection recovers from SD*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_MS_P},/*Manual Switch to protection*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_MS_W},/*Manual Switch to working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_DNR}, /*Clear*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_EXER_P},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_DNR}, /*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Force switch*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Signal Fail on working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Signal Degrade on Working*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Degrade on Protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Manual Switch to protection*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Manual Switch to Working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Wait-to-Restore*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_DNR}, /*Exercise on working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_RR_P},/*Exercise on protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_DNR}, /*Reverse Request on working*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_DNR}, /*Reverse Request on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_DNR}, /*No Request, r/b = null*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_DNR}, /*No Request, r/b = normal*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_DNR}, /*Do Not Revert*/
    },

    /* G8131_FSM_STATE_EXER_W -12 */
    {
      /* Local Events */
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_LO},    /*Lockout*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_FS},    /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},  /*Signal Fail on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_W},/*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},  /*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_W},/*Protection recovers from SF*/
      {g8131_fsm_local_handle_sd_w,        G8131_FSM_STATE_SD_W},  /*Signal Degrade on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_W},/*Working recovers from SD*/
      {g8131_fsm_local_handle_sd_p,        G8131_FSM_STATE_SD_P},  /*Signal Degrade on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_W},/*Protection recovers from SD*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_MS_P},  /*Manual Switch to protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_MS_W},  /*Manual Switch to working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},  /*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_EXER_W},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_W},/*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},  /*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},  /*Signal Fail on Protection*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},  /*Force switch*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},  /*Signal Fail on working*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},  /*Signal Degrade on Working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},  /*Signal Degrade on Protection*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},  /*Manual Switch to protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},  /*Manual Switch to Working*/
      {g8131_fsm_far_exer_w_handle_wtr,    G8131_FSM_STATE_EXER_W},/*Wait-to-Restore*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_EXER_W},/*Exercise on working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_W},/*Exercise on protection*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_EXER_W},/*Reverse Request on working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_W},/*Reverse Request on protection*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_EXER_W},/*No Request, r/b = null*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_W},/*No Request, r/b = normal*/
      {g8131_fsm_far_exer_w_handle_dnr,    G8131_FSM_STATE_EXER_W},/*Do Not Revert*/
    },

    /* G8131_FSM_STATE_EXER_P -13 */
    {
      /* Local Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_LO},    /*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_FS},    /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},  /*Signal Fail on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_P},/*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},  /*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_P},/*Protection recovers from SF*/
      {g8131_fsm_local_handle_sd_w,        G8131_FSM_STATE_SD_W},  /*Signal Degrade on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_P},/*Working recovers from SD*/
      {g8131_fsm_local_handle_sd_p,        G8131_FSM_STATE_SD_P},  /*Signal Degrade on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_P},/*Protection recovers from SD*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_MS_P},  /*Manual Switch to protection*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_MS_W},  /*Manual Switch to working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_DNR},   /*Clear*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_EXER_P},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_P},/*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},  /*Lockout*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},  /*Signal Fail on Protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},  /*Force switch*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},  /*Signal Fail on working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},  /*Signal Degrade on Working*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},  /*Signal Degrade on Protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},  /*Manual Switch to protection*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},  /*Manual Switch to Working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},  /*Wait-to-Restore*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_P},/*Exercise on working*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_EXER_P},/*Exercise on protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_P},/*Reverse Request on working*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_EXER_P},/*Reverse Request on protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_EXER_P},/*No Request, r/b = null*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_EXER_P},/*No Request, r/b = normal*/
      {g8131_fsm_overrule,                 G8131_FSM_STATE_EXER_P},/*Do Not Revert*/
    },

    /* G8131_FSM_STATE_RR_W -14 */
    {
      /* Local Events */
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_LO},    /*Lockout*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_FS},    /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},  /*Signal Fail on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_W},  /*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},  /*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_W},  /*Protection recovers from SF*/
      {g8131_fsm_local_handle_sd_w,        G8131_FSM_STATE_SD_W},  /*Signal Degrade on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_W},  /*Working recovers from SD*/
      {g8131_fsm_local_handle_sd_p,        G8131_FSM_STATE_SD_P},  /*Signal Degrade on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_W},  /*Protection recovers from SD*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_MS_P},  /*Manual Switch to protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_MS_W},  /*Manual Switch to working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_W},  /*Clear*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_EXER_W},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_W},  /*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Force switch*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Signal Fail on working*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Signal Degrade on Working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Signal Degrade on Protection*/
      {g8131_fsm_update_state_switch_to_p, G8131_FSM_STATE_NR_P},/*Manual Switch to protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Manual Switch to Working*/
      {g8131_fsm_far_rr_w_handle_wtr,      G8131_FSM_STATE_RR_W},/*Wait-to-Restore*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_RR_W},/*Exercise on working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_W},/*Exercise on protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*Reverse Request on working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_W},/*Reverse Request on protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_W},/*No Request, r/b = null*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_W},/*No Request, r/b = normal*/
      {g8131_fsm_far_rr_w_handle_dnr,      G8131_FSM_STATE_RR_W},/*Do Not Revert*/
    },

    /* G8131_FSM_STATE_RR_P -15 */
    {
      /* Local Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_LO},    /*Lockout*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_FS},    /*Forced Switch*/
      {g8131_fsm_local_handle_sf_w,        G8131_FSM_STATE_SF_W},  /*Signal Fail on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_P},  /*Working recovers from SF*/
      {g8131_fsm_local_handle_sf_p,        G8131_FSM_STATE_SF_P},  /*Signal Fail on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_P},  /*Protection recovers from SF*/
      {g8131_fsm_local_handle_sd_w,        G8131_FSM_STATE_SD_W},  /*Signal Degrade on Working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_P},  /*Working recovers from SD*/
      {g8131_fsm_local_handle_sd_p,        G8131_FSM_STATE_SD_P},  /*Signal Degrade on Protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_P},  /*Protection recovers from SD*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_MS_P},  /*Manual Switch to protection*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_MS_W},  /*Manual Switch to working*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_P},  /*Clear*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_EXER_P},/*Exercise*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_P},  /*WTR timer expires*/

      /* Remote Events */
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Lockout*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Fail on Protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Force switch*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Signal Fail on working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Signal Degrade on Working*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Signal Degrade on Protection*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Manual Switch to protection*/
      {g8131_fsm_update_state_switch_to_w, G8131_FSM_STATE_NR_W},/*Manual Switch to Working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_NR_P},/*Wait-to-Restore*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_P},/*Exercise on working*/
      {g8131_fsm_nochange,                 G8131_FSM_STATE_RR_P},/*Exercise on protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_P},/*Reverse Request on working*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_DNR}, /*Reverse Request on protection*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_P},/*No Request, r/b = null*/
      {g8131_fsm_ignore,                   G8131_FSM_STATE_RR_P},/*No Request, r/b = normal*/
      {g8131_fsm_update_regular_state,     G8131_FSM_STATE_DNR}, /*Do Not Revert*/
    }
};


int32
g8131_fsm_ignore(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    G8131_EVENT_DEBUG("APS group %u: Event received is NOT EXPECT to happen for the State, ignore this event. State: %s  Event: %s\n", 
        p_db_ag_info->key.aps_group_id, G8131_FSM_STATE_STR(p_db_ag_info->current_state), G8131_FSM_EVENT_STR(event));
    return PM_E_NONE;
}

int32
g8131_fsm_nochange(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    G8131_EVENT_DEBUG("APS group %u: Event received is NOCHANGE in current state. State: %s  Event: %s\n", 
        p_db_ag_info->key.aps_group_id, G8131_FSM_STATE_STR(p_db_ag_info->current_state), G8131_FSM_EVENT_STR(event));
    return PM_E_NONE;
}

int32
g8131_fsm_overrule(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    G8131_EVENT_DEBUG("APS group %u: Event received is OVERRULED by the existing condition. State: %s  Event: %s\n", 
        p_db_ag_info->key.aps_group_id, G8131_FSM_STATE_STR(p_db_ag_info->current_state), G8131_FSM_EVENT_STR(event));
    return PM_E_NONE;
}

int32
g8131_fsm_tbd(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    G8131_EVENT_DEBUG("APS group %u: Event received is TBD in current state. State: %s  Event: %s\n", 
        p_db_ag_info->key.aps_group_id, G8131_FSM_STATE_STR(p_db_ag_info->current_state), G8131_FSM_EVENT_STR(event));
    return PM_E_NONE;
}

int32
g8131_fsm_set_request_signal(tbl_aps_group_info_t* p_db_ag_info, uint32 new_state)
{
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    
    switch (new_state)
    {
    case G8131_FSM_STATE_NR_W:
        p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NULL;
        break;
    case G8131_FSM_STATE_NR_P:
        p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NORMAL;
        break;
    case G8131_FSM_STATE_LO:
        p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NULL;
        break;
    case G8131_FSM_STATE_FS:
        p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NORMAL;
        break;
    case G8131_FSM_STATE_SF_W:
        p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NORMAL;
        break;
    case G8131_FSM_STATE_SF_P:
        p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NULL;
        break;
    case G8131_FSM_STATE_SD_W:
        p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NORMAL;
        break;
    case G8131_FSM_STATE_SD_P:
        p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NULL;
        break;
    case G8131_FSM_STATE_MS_P:
        p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NORMAL;
        break;
    case G8131_FSM_STATE_MS_W:
         p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NULL;
        break;
    case G8131_FSM_STATE_WTR:
         p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NORMAL;
        break;
    case G8131_FSM_STATE_DNR:
         p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NORMAL;
        break;
    case G8131_FSM_STATE_EXER_W:
         p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NULL;
        break;
    case G8131_FSM_STATE_EXER_P:
         p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NORMAL;
        break;       
    case G8131_FSM_STATE_RR_W:
         p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NULL;
        break;
    case G8131_FSM_STATE_RR_P:
        p_db_ag_info->requested_signal = G8131_REQUEST_SIGNAL_NORMAL;
        break;
    default:
          break;
    }
    return PM_E_NONE;
}

int32
g8131_fsm_change_state(tbl_aps_group_info_t* p_db_ag_info, uint32 new_state)
{
    uint32 cur_state = 0;
    int32 rc = PM_E_NONE;

    CTC_TASK_STOP_TIMER(p_db_ag_info->wtr_timer);

    cur_state = G8131_FSM_GET_CUR_STATE (p_db_ag_info);
    G8131_FSM_SET_LAST_STATE(p_db_ag_info, cur_state);
    G8131_FSM_SET_CUR_STATE(p_db_ag_info, new_state);
    PM_E_RETURN(g8131_fsm_set_request_signal(p_db_ag_info, new_state));

    /*Debug*/
    G8131_EVENT_DEBUG("APS group %u: State change: last state is %s, current event is: %s  ---> current state is %s",
                      p_db_ag_info->key.aps_group_id,
                      G8131_FSM_STATE_STR(p_db_ag_info->last_state),
                      G8131_FSM_EVENT_STR(p_db_ag_info->current_event),
                      G8131_FSM_STATE_STR(p_db_ag_info->current_state));
    
    rc = g8131_pdu_aps_tx(p_db_ag_info);
    if (PM_E_NONE == rc)
    {
        p_db_ag_info->sc_aps_out_pdu_count ++;

        /*start timer*/
        p_db_ag_info->sc_out_aps_timer = p_db_ag_info->sc_out_aps_timer ? p_db_ag_info->sc_out_aps_timer:
        ctc_task_add_timer(TASK_PRI_NORMAL, g8131_pdu_out_state_change_timer_handler, p_db_ag_info, 0);
    }
    return rc;
}

int32
g8131_fsm_update_regular_state(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    uint32 cur_state = 0;
    uint32 next_state = 0;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    cur_state = G8131_FSM_GET_CUR_STATE(p_db_ag_info);
    next_state = G8131FSM[cur_state][event].next_state;
    PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, next_state));

    return rc;
}

int32
g8131_fsm_do_aps_switch(tbl_aps_group_info_t* p_db_ag_info, uint32 is_working_active)
{
    int32 rc = PM_E_NONE;
    uint32 protection_en = 0;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    if (is_working_active)
    {
        p_db_ag_info->active_path = G8131_ACTIVE_WORKING;
        PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_db_ag_info, TBL_APS_GROUP_INFO_FLD_ACTIVE_PATH));
        protection_en = FALSE;
    }
    else
    {
        p_db_ag_info->active_path = G8131_ACTIVE_PROTECTING;
        PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_db_ag_info, TBL_APS_GROUP_INFO_FLD_ACTIVE_PATH));
        protection_en = TRUE;
    }
    PM_E_RETURN(g8131_aps_group_protection_en(p_db_ag_info, protection_en));
    return rc;
}

int32
g8131_fsm_update_state_switch_to_p(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
    PM_E_RETURN(g8131_fsm_update_regular_state(p_db_ag_info, event));

    return rc;
}

int32
g8131_fsm_update_state_switch_to_w(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, TRUE));
    PM_E_RETURN(g8131_fsm_update_regular_state(p_db_ag_info, event));
    
    return rc;
}

int32
g8131_fsm_local_handle_sf_w(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    if ((p_db_ag_info->hold_off_time) && (p_db_ag_info->active_path == G8131_ACTIVE_WORKING)&&
        (!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SF_HOLD_OFF_STATE)))
    {
        /*set sf_hold_off_timer */
        p_db_ag_info->sf_hold_off_timer = p_db_ag_info->sf_hold_off_timer ? p_db_ag_info->sf_hold_off_timer:
        ctc_task_add_timer_msec(TASK_PRI_NORMAL, g8131_pdu_sf_hold_off_timer_handler, p_db_ag_info, p_db_ag_info->hold_off_time * 100);

        GLB_SET_FLAG(p_db_ag_info->hold_off_expired, G8131_IN_SF_HOLD_OFF_STATE);
    }
    else if ((p_db_ag_info->hold_off_time) && (p_db_ag_info->sf_hold_off_timer) &&
             (GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SF_HOLD_OFF_STATE)))
    {
        return PM_E_NONE;
    }
    else
    {
        if (p_db_ag_info->active_path == G8131_ACTIVE_WORKING)
        {
            PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        }
        PM_E_RETURN(g8131_fsm_update_regular_state(p_db_ag_info, event));
    }
    
    return rc;
}

int32
g8131_fsm_local_handle_sf_p(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    if ((p_db_ag_info->hold_off_time) && (p_db_ag_info->active_path == G8131_ACTIVE_PROTECTING)&&
        (!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SFP_HOLD_OFF_STATE)))
    {
        /*set sfp_hold_off_timer */
        p_db_ag_info->sfp_hold_off_timer = p_db_ag_info->sfp_hold_off_timer ? p_db_ag_info->sfp_hold_off_timer:
        ctc_task_add_timer_msec(TASK_PRI_NORMAL, g8131_pdu_sfp_hold_off_timer_handler, p_db_ag_info, p_db_ag_info->hold_off_time * 100);

        GLB_SET_FLAG(p_db_ag_info->hold_off_expired, G8131_IN_SFP_HOLD_OFF_STATE);
    }
    else if ((p_db_ag_info->hold_off_time) && (p_db_ag_info->sfp_hold_off_timer) &&
             (GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SFP_HOLD_OFF_STATE)))
    {
        return PM_E_NONE;
    }
    else
    {
        if (p_db_ag_info->active_path == G8131_ACTIVE_PROTECTING)
        {
            PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, TRUE));
        }
        PM_E_RETURN(g8131_fsm_update_regular_state(p_db_ag_info, event));
    }
    
    return rc;
}   

int32
g8131_fsm_local_handle_sd_w(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    if ((p_db_ag_info->hold_off_time) && (p_db_ag_info->active_path == G8131_ACTIVE_WORKING) &&
        (!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SD_HOLD_OFF_STATE)))
    {
        /*set sd_hold_off_timer */
        p_db_ag_info->sd_hold_off_timer = p_db_ag_info->sd_hold_off_timer ? p_db_ag_info->sd_hold_off_timer:
        ctc_task_add_timer_msec(TASK_PRI_NORMAL, g8131_pdu_sd_hold_off_timer_handler, p_db_ag_info, p_db_ag_info->hold_off_time * 100);
        
        GLB_SET_FLAG(p_db_ag_info->hold_off_expired, G8131_IN_SD_HOLD_OFF_STATE);
    }
    else if ((p_db_ag_info->hold_off_time) && (p_db_ag_info->sd_hold_off_timer) &&
             (GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SD_HOLD_OFF_STATE)))
    {
        return PM_E_NONE;
    }
    else
    {
        if (p_db_ag_info->active_path == G8131_ACTIVE_WORKING)
        {
            PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        }
        PM_E_RETURN(g8131_fsm_update_regular_state(p_db_ag_info, event));
    }
    
    return rc;
}

int32
g8131_fsm_local_handle_sd_p(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    if ((p_db_ag_info->hold_off_time) && (p_db_ag_info->active_path == G8131_ACTIVE_PROTECTING) &&
        (!GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SDP_HOLD_OFF_STATE)))
    {
        /*set sdp_hold_off_timer */
        p_db_ag_info->sdp_hold_off_timer = p_db_ag_info->sdp_hold_off_timer ? p_db_ag_info->sdp_hold_off_timer:
        ctc_task_add_timer_msec(TASK_PRI_NORMAL, g8131_pdu_sdp_hold_off_timer_handler, p_db_ag_info, p_db_ag_info->hold_off_time * 100);

        GLB_SET_FLAG(p_db_ag_info->hold_off_expired, G8131_IN_SDP_HOLD_OFF_STATE);
    }
    else if ((p_db_ag_info->hold_off_time) && (p_db_ag_info->sdp_hold_off_timer) &&
             (GLB_FLAG_ISSET(p_db_ag_info->hold_off_expired, G8131_IN_SDP_HOLD_OFF_STATE)))
    {
        return PM_E_NONE;
    }
    else
    {
        if (p_db_ag_info->active_path == G8131_ACTIVE_PROTECTING)
        {
            PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, TRUE));
        }
        PM_E_RETURN(g8131_fsm_update_regular_state(p_db_ag_info, event));
    }
    
    return rc;
}

int32 
g8131_fsm_far_nr_w_handle_nr_null(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    if (g8131_is_sfp_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SF_P));
        return PM_E_NONE;
    }

    if (g8131_is_sf_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SF_W));
        return PM_E_NONE;
    }

    if (g8131_is_sdp_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_P));
        return PM_E_NONE;
    }

    if (g8131_is_sd_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_W));
        return PM_E_NONE;
    }
    PM_E_RETURN(g8131_fsm_nochange(p_db_ag_info, event));
    
    return rc;
}

int32
g8131_fsm_far_nr_w_handle_dnr(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    
    if (TRUE == p_db_ag_info->is_revertive)
    {
        PM_E_RETURN(g8131_fsm_update_state_switch_to_p(p_db_ag_info, event));
    }
    else
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_DNR));
    }
    return rc;
}

int32
g8131_fsm_far_nr_p_handle_nr_null(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    
    if (g8131_is_sf_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SF_W));
        return PM_E_NONE;
    }
    if (g8131_is_sd_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_W));
        return PM_E_NONE;
    }
    PM_E_RETURN(g8131_fsm_update_state_switch_to_w(p_db_ag_info, event));
    
    return rc;
}

int32
g8131_fsm_far_nr_p_handle_nr_normal(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    
    if (TRUE == p_db_ag_info->is_revertive)
    {
        if(G8131_FSM_STATE_SF_W == p_db_ag_info->last_state ||
           G8131_FSM_STATE_SD_W == p_db_ag_info->last_state)
        {
             PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_WTR));

             p_db_ag_info->wtr_timer = p_db_ag_info->wtr_timer ? p_db_ag_info->wtr_timer:
             ctc_task_add_timer(TASK_PRI_NORMAL, g8131_pdu_wtr_timer_handler, p_db_ag_info, p_db_ag_info->wait_to_restore_time);
        }
        else
        {
            PM_E_RETURN(g8131_fsm_update_state_switch_to_w(p_db_ag_info, event));
        }
    }
    else
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_DNR));
    }
    
    return rc;
}

int32
g8131_fsm_far_nr_p_handle_dnr(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    if (TRUE == p_db_ag_info->is_revertive)
    {
        PM_E_RETURN(g8131_fsm_nochange(p_db_ag_info, event));
    }
    else
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_DNR));
    }
    return rc;
}

int32
g8131_fsm_local_lo_handle_clear(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    if (g8131_is_sfp_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SF_P));
        return PM_E_NONE;
    }

    if (g8131_is_sf_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SF_W));
        return PM_E_NONE;
    }

    if (g8131_is_sdp_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_P));
        return PM_E_NONE;
    }

    if (g8131_is_sd_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_W));
        return PM_E_NONE;
    }
    PM_E_RETURN(g8131_fsm_update_regular_state(p_db_ag_info, event));
    
    return rc;
}

int32
g8131_fsm_local_fs_handle_clear(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    if(g8131_is_sf_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SF_W));
        return PM_E_NONE;
    }

    if(g8131_is_sdp_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, TRUE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_P));
        return PM_E_NONE;
    }

    if(g8131_is_sd_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_W));
        return PM_E_NONE;
    }

    if (TRUE == p_db_ag_info->is_revertive)
    {
        PM_E_RETURN(g8131_fsm_update_state_switch_to_w(p_db_ag_info, event));
    }
    else
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_DNR));
    }
    return rc;
}

int32
g8131_fsm_local_ms_p_handle_clear(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    if (TRUE == p_db_ag_info->is_revertive)
    {
        PM_E_RETURN(g8131_fsm_update_state_switch_to_w(p_db_ag_info, event));
    }
    else
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_DNR));
    }
    return rc;
}

int32
g8131_fsm_local_sf_w_handle_wrsf(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    if (g8131_is_sdp_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, TRUE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_P));
        return PM_E_NONE;
    }
    
    if (g8131_is_sd_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_W));
        return PM_E_NONE;
    }
    
    if (TRUE == p_db_ag_info->is_revertive)
     {
         PM_E_RETURN(g8131_fsm_update_regular_state(p_db_ag_info, event));
         
         p_db_ag_info->wtr_timer = p_db_ag_info->wtr_timer ? p_db_ag_info->wtr_timer:
         ctc_task_add_timer(TASK_PRI_NORMAL, g8131_pdu_wtr_timer_handler, p_db_ag_info, p_db_ag_info->wait_to_restore_time);
    }
    else
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_DNR));
    }
    
    return PM_E_NONE;
}

int32
g8131_fsm_local_sf_p_handle_prsf(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }

    if (g8131_is_sf_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SF_W));
        return PM_E_NONE;
    }

    if (g8131_is_sdp_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_P));
        return PM_E_NONE;
    }
    
    if (g8131_is_sd_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_W));
        return PM_E_NONE;
    }
    PM_E_RETURN(g8131_fsm_update_regular_state(p_db_ag_info, event));
    
    return rc;
}

int32
g8131_fsm_local_sd_w_handle_wrsd(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    if (g8131_is_sdp_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, TRUE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_P));
        return PM_E_NONE;
    }

    if (TRUE == p_db_ag_info->is_revertive)
    {
        PM_E_RETURN(g8131_fsm_update_regular_state(p_db_ag_info, event));

        p_db_ag_info->wtr_timer = p_db_ag_info->wtr_timer ? p_db_ag_info->wtr_timer:
             ctc_task_add_timer(TASK_PRI_NORMAL, g8131_pdu_wtr_timer_handler, p_db_ag_info, p_db_ag_info->wait_to_restore_time);

    }
    else
    {
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_DNR));
    }
    return rc;
}

int32
g8131_fsm_local_sd_p_handle_prsd(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }
    if (g8131_is_sd_reasserted(p_db_ag_info))
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_SD_W));
        return PM_E_NONE;
    }
    PM_E_RETURN(g8131_fsm_update_regular_state(p_db_ag_info, event));
    
    return rc;
}

int32
g8131_fsm_far_ms_p_handle_ms_w(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }    
    /*Not understand when should it remained in (->G) state, so always implement to ->A state instead.*/
    PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, TRUE));
    PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_NR_W));
    return rc;
}

int32
g8131_fsm_far_exer_w_handle_wtr(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE; 
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }   
    if (p_db_ag_info->is_revertive)
    {
        PM_E_RETURN(g8131_fsm_ignore(p_db_ag_info, event));
    }
    else
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_NR_P));
    }
    return rc;
}

int32
g8131_fsm_far_exer_w_handle_dnr(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }   
    if (p_db_ag_info->is_revertive)
    {
        PM_E_RETURN(g8131_fsm_overrule(p_db_ag_info, event));
    }
    else
    {
        PM_E_RETURN(g8131_fsm_ignore(p_db_ag_info, event));
    }
    return rc;
}

int32
g8131_fsm_far_rr_w_handle_wtr(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }   
    if (p_db_ag_info->is_revertive)
    {
        PM_E_RETURN(g8131_fsm_overrule(p_db_ag_info, event));
    }
    else
    {
        PM_E_RETURN(g8131_fsm_do_aps_switch(p_db_ag_info, FALSE));
        PM_E_RETURN(g8131_fsm_change_state(p_db_ag_info, G8131_FSM_STATE_NR_P));
    }
    return rc;
}

int32
g8131_fsm_far_rr_w_handle_dnr(tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }   
    if (p_db_ag_info->is_revertive)
    {
        PM_E_RETURN(g8131_fsm_overrule(p_db_ag_info, event));
    }
    else
    {
        PM_E_RETURN(g8131_fsm_ignore(p_db_ag_info, event));
    }
    return rc;
}

int32
g8131_fsm_execute (tbl_aps_group_info_t* p_db_ag_info, uint32 event)
{
    int32 rc = PM_E_NONE;
    int32 cur_state = 0;

    if (NULL == p_db_ag_info)
    {
        return PM_E_NOT_EXIST;
    }   

    if (G8131_ACTIVE_NONE == p_db_ag_info->active_path)
    {
        return PM_E_NONE;
    }

    if (((event < G8131_FSM_EVENT_MAX) && (event >= G8131_FSM_R_EVENT_LO)) && (p_db_ag_info->far_end_last_event != event))
    {
        if (G8131_FSM_R_EVENT_LO == event)
        {
            G8131_EVENT_DEBUG("FSM EXECUTE: %s aps group %u fast aps false", 
                               g8131_get_aps_type(p_db_ag_info), p_db_ag_info->key.aps_group_id);
            p_db_ag_info->fast_aps_en = FALSE;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_db_ag_info, TBL_APS_GROUP_INFO_FLD_FAST_APS_EN));
            PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, FALSE));
        }
        else if ((G8131_FSM_R_EVENT_LO == p_db_ag_info->far_end_last_event) && (0 == p_db_ag_info->hold_off_time))
        {
            G8131_EVENT_DEBUG("FSM EXECUTE: %s aps group %u fast aps true", 
                               g8131_get_aps_type(p_db_ag_info), p_db_ag_info->key.aps_group_id);
            p_db_ag_info->fast_aps_en = TRUE;
            PM_E_RETURN(tbl_aps_group_info_set_aps_group_info_field(p_db_ag_info, TBL_APS_GROUP_INFO_FLD_FAST_APS_EN));
            PM_E_RETURN(fei_g8131_mpls_fast_aps_en(p_db_ag_info->aps_group_type, p_db_ag_info->key.aps_group_id, TRUE));
        } 
        p_db_ag_info->far_end_last_event = event;
    }
    p_db_ag_info->current_event = event;
    
    cur_state = G8131_FSM_GET_CUR_STATE(p_db_ag_info);
    /* Invoke registered function. */
    (*(G8131FSM [cur_state][event].func)) (p_db_ag_info, event);

    return rc;
}

