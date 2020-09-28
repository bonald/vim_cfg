
#ifndef __TBL_OAM_SESSION_UPLOAD_DEFINE_H__
#define __TBL_OAM_SESSION_UPLOAD_DEFINE_H__

/* TBL_OAM_SESSION_UPLOAD field defines */
typedef enum
{
    TBL_OAM_SESSION_UPLOAD_FLD_KEY                  = 0 ,  /* READ */
    TBL_OAM_SESSION_UPLOAD_FLD_BIND_OAM_FLOW_ID     = 1 ,  /* RW */
    TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM            = 2 ,  /* RW */
    TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_DM_EVENT      = 3 ,  /* RW */
    TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM            = 4 ,  /* RW */
    TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_LM_EVENT      = 5 ,  /* RW */
    TBL_OAM_SESSION_UPLOAD_FLD_UPLOAD_CCM           = 6 ,  /* RW */
    TBL_OAM_SESSION_UPLOAD_FLD_MAX                  = 7 
} tbl_oam_session_upload_field_id_t;

/* TBL_OAM_SESSION_UPLOAD defines */
typedef struct
{
    uint32               session_id;
} tbl_oam_session_upload_key_t;

typedef struct
{
    tbl_oam_session_upload_key_t key;                 /*also for event*/
    uint32               bind_oam_flow_id;    /*is dm/lm/event/ccm ?*//*also for event*/
    upload_tpoam_dm_t    upload_dm;
    upload_tpoam_dm_t    upload_dm_event;
    upload_tpoam_lm_t    upload_lm;
    upload_tpoam_lm_t    upload_lm_event;
    upload_tpoam_ccm_t   upload_ccm;
} tbl_oam_session_upload_t;

typedef struct
{
    ctclib_hash_t        *oam_session_hash;
    ctclib_slist_t       *oam_session_list;
} tbl_oam_session_upload_master_t;

#endif /* !__TBL_OAM_SESSION_UPLOAD_DEFINE_H__ */

