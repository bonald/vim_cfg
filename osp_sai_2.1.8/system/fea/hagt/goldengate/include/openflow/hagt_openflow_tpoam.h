#ifndef __HAGT_OPENFLOW_TPOAM_H__
#define __HAGT_OPENFLOW_TPOAM_H__


#define HAGT_TPOAM_LM_DUAL_UPDATE_TIME         10    /* Query all lm dual node in 10 times */
#define HAGT_TPOAM_LM_DUAL_UPDATE_PER_TIME_MIN 1   /* process node one time */
#define HAGT_TPOAM_LM_DUAL_HASH_SIZE           1024  /* The lm dual hash size */

#define HAGT_TPOAM_LM_SD_TIMER_INTERVAL         50    /* 50ms */
#define HAGT_TPOAM_LM_SD_HASH_SIZE              50

#define HAGT_TPOAM_FAST_APS_MAX_APS_NUM         4096
#define HAGT_TPOAM_FAST_APS_MAX_MEP_NUM         8192





struct hagt_tpoam_s
{
#ifdef _GLB_UML_SYSTEM_
    bool    oam_en;
#endif /* _GLB_UML_SYSTEM_ */
    ctc_task_t* pst_oam_timer;
    ctc_task_t* pst_lm_timer;
    uint16                      lm_dual_hash_index; 
    uint16                      lm_dual_hash_count_per_time; /* hash count / 10 */
    uint16                      lm_dual_time;   /* if time equals 10, must process all left nodes 
                                                   then, time is changed to 0 */
    uint16                      lm_dual_hash_total_update_count; 

    ctclib_hash_t*              pst_lm_dual_hash;
    ctclib_hash_t*              pst_lm_sd_hash;

    uint32 sf_reason;
    uint32 fast_aps_enable;
    uint32  pw_not_standard_mode;
    ctclib_vector_t* p_mep;
    ctclib_vector_t* p_aps;
    uint32 hagt_tpoam_debug;
};
typedef struct hagt_tpoam_s hagt_tpoam_t;


struct hagt_tpoam_lm_dual_s
{
    uint8   oam_type;
    uint32  key_id;
    uint16  mep_index;
    uint8   lm_enable    :1;
    uint8   lm_type      :2;
    uint8   lm_cos_type  :4;
    uint8   lm_cos_value;
    uint8   stats_interval;
    uint8   timer_counter;  /* counter = stats_interval / 100ms */
    uint8   reserved;
};
typedef struct hagt_tpoam_lm_dual_s hagt_tpoam_lm_dual_t;


struct hagt_tpoam_lm_sd_s
{
    uint8   oam_type;
    uint32  key_id;
    uint16  mep_index;
    uint8   lm_type;
    uint8   lm_cos_type;
    uint8   lm_cos_value;
    uint16  ratio_limit;

    ctc_oam_lm_info_t lm_info[CTC_OAM_STATS_COS_NUM]; 
    uint32  lm_far_end_frame_loss_ratio[CTC_OAM_STATS_COS_NUM];
    uint32  lm_near_end_frame_loss_ratio[CTC_OAM_STATS_COS_NUM];
};
typedef struct hagt_tpoam_lm_sd_s hagt_tpoam_lm_sd_t;


struct glb_tpoam_lm_dual_counter_cache_s
{
    uint8_t num;
    glb_tpoam_lm_dual_counter_t entry[GLB_TPOAM_MAX_LM_DUAL_COUNTER_CACHE_NUM];
};
typedef struct glb_tpoam_lm_dual_counter_cache_s glb_tpoam_lm_dual_counter_cache_t;


int32
hagt_openflow_tpoam_register_callback(void);

int32
hagt_tpoam_init();

uint32
hagt_tpoam_get_pw_mode(void);

#endif /* !__HAGT_OPENFLOW_TPOAM_H__ */
