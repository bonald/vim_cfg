#ifndef __TPOAM_API_H__
#define __TPOAM_API_H__

#include "tpoam_lm.h"



#define TPOAM_ASSERT sal_assert(0);


#define TPOAM_TEST_MODE_0_NULL_WO_CRC   0
#define TPOAM_TEST_MODE_1_NULL_WI_CRC   1
#define TPOAM_TEST_MODE_2_PRBS_WO_CRC   2
#define TPOAM_TEST_MODE_3_PRBS_WI_CRC   3



typedef void (*tpoam_api_service_show_cb)(FILE*, tbl_oam_session_t*, tpoam_lm_dual_enable_input_t*);


/* DM entry defines */
struct tpoam_dm_entry_s
{
    tpoam_interval_t    delay;      /* in ns */
    tpoam_interval_t    variation;
    sal_time_t          time;
    uint8               exp;
};
typedef struct tpoam_dm_entry_s tpoam_dm_entry_t;

#define TPOAM_TV_TO_SLICE(_tv_, _interval_)   \
    (((uint64)(_tv_)->tv_sec*1000000L + (uint64)(_tv_)->tv_usec) >> (_interval_))

#define L2_TIMER_SCALE_FACT 256
//#define TICS_TO_SECS(x)       ((x) / L2_TIMER_SCALE_FACT)
//#define SECS_TO_TICS(x)       ((x) * L2_TIMER_SCALE_FACT)

#define DELAY_STR_LEN           20

#define TPOAM_TIMEINTERVAL_TO_STR(_str_, _interval_)\
do {\
    sal_snprintf((_str_), DELAY_STR_LEN, "%14"PRId64, _interval_);\
} while (0)



#define TPOAM_API_EVT_PER_LINE   4
#define TPOAM_API_SHOW_EVT_BITMAP(_bitmap_, _flag_)\
do\
{\
    if (GLB_FLAG_ISSET((_bitmap_), (_flag_)))\
    {\
        if (!is_first)\
        {\
            sal_strcat(show_str, ", ");\
        }\
        is_first = FALSE;\
        if ((0 !=  flag_count) && (0 == flag_count % TPOAM_API_EVT_PER_LINE))\
        {\
            sal_strcat(show_str, "\n                  ");\
        }\
        flag_count++;\
        sal_strcat(show_str, tpoam_api_event_desc((_flag_)));\
    }\
} while (0)

int32
tpoam_api_clear_evt_or_1dm(uint32 type, tpoam_pw_type_t pw_type, 
    char* pch_service_name, uint32 clear_type);

int32
tpoam_api_clear_evt_or_1dm_session(uint32 session_id, uint32 clear_type);

int32
tpoam_api_service_get_status(tbl_oam_session_t* pst_oam, uint32 direction, uint32* pst_status);

ctc_task_t *
tpoam_util_add_timer_ms(void (*func) (void *), void *arg, int32 ms);

void
tpoam_util_cancel_timer(ctc_task_t **ppt);

char*
tpoam_api_exec_type_desc(uint32 exec_type);

char*
tpoam_api_pkt_type_desc(uint32 pkt_type);

int32
tpoam_api_exec_deinit(tbl_oam_session_t* pst_oam);

int32
tpoam_api_build_megid(char* pch_megstr, uint8* pch_megid);

int32
tpoam_api_stats_add_tx(tpoam_stats_tx_item_t* pst_tx_stats, uint8 opcode);

int32
tpoam_api_stats_add_rx(tpoam_stats_rx_item_t* pst_rx_stats, uint8 opcode);

int32
tpoam_api_stats_add_err(tpoam_stats_err_item_t* pst_err_stats, uint8 opcode);

int32
tpoam_get_prefix_item (char** argv , uint8 argc, char* prefix, uint8 prefix_len);

int32
tpoam_api_lm_interval_to_ms(uint8 interval, int32* p_ms);

int32
tpoam_api_dm_interval_to_ms(uint8 interval, int32* p_ms);

int32
tpoam_api_set_dm_global_enable(uint32 enable);

int32
tpoam_api_set_dm_calc_interval(uint32 interval);

int32
tpoam_api_set_dm_delay_threshold(uint32 threshold);

int32
tpoam_api_set_lm_global_enable(uint32 enable);

int32
tpoam_api_set_lm_local_threshold(uint32 threshold);

int32
tpoam_api_set_lm_remote_threshold(uint32 threshold);

int32
tpoam_api_set_lm_calc_interval(uint32 interval);

int32
tpoam_api_set_fast_aps_enable(uint32 fast_aps_en);

int32
tpoam_api_set_node_id(uint32 node_id);

int32
tpoam_api_set_channel_type(uint32 channel_type);

int32
tpoam_api_set_rdi_reason(uint32 rdi_reason_mask, uint32 to_hal);

int32
tpoam_api_set_sf_reason(uint32 sf_reason_mask);

int32
tpoam_api_set_sd_reason(uint32 sd_reason_mask);

int32
tpoam_api_set_event_rule_map(uint32 event_rule_map, tpoam_evt_dest_t dest);

char*
tpoam_api_event_desc(glb_tpoam_evt_t event);

int32
_tpoam_get_event_rule_value(uint32 argc, char** argv, uint32 para_index_base, uint32* p_evt_rule);

char*
tpoam_api_status_desc(uint8 status);

char*
tpoam_api_lb_testtype_desc(uint32 type);

char*
tpoam_api_lb_subtype_desc(uint32 type);

char*
tpoam_api_direction_desc(tpoam_direction_t direction);

char*
tpoam_api_event_action_desc(uint32 event_action);

char*
_tpoam_lb_sub_type_desc(uint8 lb_sub_type);

char*
tpoam_api_service_type_for_display_desc(uint32 oam_type, tpoam_pw_type_t pw_type, uint32 lsp_type);

char*
tpoam_api_cc_interval_desc(uint8 interval);

char*
tpoam_api_en_desc(uint8 is_short, uint8 enable);

int
_tpoam_api_show_pw_tpe_param2_interator(void *obj, tbl_iter_args_t *pargs);

int
_tpoam_api_show_lsp_p_param2_interator(void *obj, tbl_iter_args_t *pargs);

int
_tpoam_api_show_lsp_pe_param2_interator(void *obj, tbl_iter_args_t *pargs);

int
_tpoam_api_show_section_param2_interator(void *obj, tbl_iter_args_t *pargs);

int32
tpoam_api_get_lsp_p(char* pch_lsp_name, tbl_lsp_p_t** ppst_lsp_p);

int32
tpoam_api_get_lsp_pe(char* pch_lsp_name, tbl_lsp_pe_t** ppst_lsp_pe);

int32
tpoam_api_get_pw_tpe(char* pch_tpe_name, tbl_pw_tpe_t** ppst_tpe);

int32
tpoam_api_get_section(char* pch_tpe_name, tbl_section_oam_t** ppst_section);

int32
tpoam_api_get_section_new(uint32 ifindex, tbl_section_oam_t** ppst_section);

tbl_oam_session_t*
tbl_oam_session_get_oam_session_by_session_id(uint32 session_id);

tbl_lsp_pe_t*
tbl_lsp_pe_get_lsp_pe_by_name(char * name);

tbl_lsp_p_t*
tbl_lsp_p_get_lsp_p_by_name(char * name);

tbl_pw_tpe_t*
tbl_pw_tpe_get_pw_tpe_by_name(char * name);

char *
tpoam_api_get_ifname_ext_by_ifindex(uint32 ifindex);

tbl_section_oam_t*
tbl_section_oam_get_section_oam_by_ifindex(uint32 ifindex);

int32
tpoam_api_lookup_oam_by_megid(char* pch_megid, tbl_oam_session_t** ppst_oam);

void
tpoam_api_show_oam_lm_single(FILE *fp, tbl_oam_session_t* pst_oam, tpoam_single_lm_t* pst_lm_single);

int32
tpoam_api_show_lm_dual(FILE *fp, uint32 type, tpoam_pw_type_t pw_type, 
        char* pch_service_name, tpoam_lm_dual_enable_input_t* lm_dual_inputs);

int32
tpoam_api_show_lm_dual_session(FILE *fp, uint32 session_id, tpoam_lm_dual_enable_input_t* lm_dual_inputs, cfg_cmd_para_t *para);

int32
tpoam_api_show_lm_dual_upload_session(FILE *fp, uint32 session_id, tpoam_lm_dual_enable_input_t* lm_dual_inputs, cfg_cmd_para_t *para);

int32
tpoam_api_show_event_session(FILE *fp, uint32 session_id);

int32
tpoam_api_show_dmm_session(FILE *fp, uint32 session_id, cfg_cmd_para_t *para);

int32
tpoam_api_show_mp_service_session(FILE *fp, uint32 oam_type);

int32
tpoam_api_show_mp_status(FILE *fp, uint32 type, tpoam_pw_type_t pw_type);

int32
tpoam_api_show_mp_pw_tpe(FILE *fp, char* pch_tpe_name);

int32
tpoam_api_show_mp_lsp(FILE *fp, char* pch_lsp_name);

int32
tpoam_api_show_mp_section(FILE *fp, char* pch_section_name);

int32
tpoam_api_show_binding_flow_session(FILE *fp, uint32 session_id, cfg_cmd_para_t *para);

int32
tpoam_api_show_session(FILE *fp, uint32 session_id, uint32 oam_type, uint32 mp_type, cfg_cmd_para_t *para);

int32
tpoam_api_show_session_detail(FILE *fp, uint32 session_id, cfg_cmd_para_t *para);

int32
tpoam_api_show_statistics_session(FILE *fp, uint32 session_id, cfg_cmd_para_t *para);

int32
_tpoam_api_show_stats(FILE *fp, tpoam_stats_t* pst_stats, uint32 show_statistics, uint32 session_id, uint32 show_hardware_process);

int32
tpoam_api_show_mp_session(FILE *fp, uint32 session_id, cfg_cmd_para_t *para);

int32
tpoam_api_show_mp_status_session(FILE *fp, uint32 session_id, cfg_cmd_para_t *para);

int32
tpoam_api_show_1dm(FILE *fp, uint32 type, tpoam_pw_type_t pw_type, char* pch_service_name);

int32
tpoam_api_show_event(FILE *fp, uint32 type, tpoam_pw_type_t pw_type, char* pch_service_name);

int32
tpoam_api_set_lm_dual_en(tbl_oam_session_t* pst_oam);

int32
tpoam_api_lb_show_para(char *str_return, uint32* len, tpoam_lbm_para_t* pst_para);

#endif /* !__TPOAM_API_H__ */
