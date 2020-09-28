
#ifndef __TBL_TPOAM_EXEC_GLOBAL_DEFINE_H__
#define __TBL_TPOAM_EXEC_GLOBAL_DEFINE_H__

/* TBL_TPOAM_EXEC_GLOBAL field defines */
typedef enum
{
    TBL_TPOAM_EXEC_GLOBAL_FLD_TEST_TYPE            = 0 ,  /* RW */
    TBL_TPOAM_EXEC_GLOBAL_FLD_OAM_SESSION_ID       = 1 ,  /* RW */
    TBL_TPOAM_EXEC_GLOBAL_FLD_PID                  = 2 ,  /* RW */
    TBL_TPOAM_EXEC_GLOBAL_FLD_SOCK                 = 3 ,  /* RW */
    TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LM              = 4 ,  /* RW */
    TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_RMEP         = 5 ,  /* RW */
    TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_MIP          = 6 ,  /* RW */
    TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_LB_DISCOVERY    = 7 ,  /* RW */
    TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_DMM             = 8 ,  /* RW */
    TBL_TPOAM_EXEC_GLOBAL_FLD_EXEC_1DM             = 9 ,  /* RW */
    TBL_TPOAM_EXEC_GLOBAL_FLD_P_T_SEND_LB_MSG_TIMER = 10,  /* READ */
    TBL_TPOAM_EXEC_GLOBAL_FLD_MAX                  = 11
} tbl_tpoam_exec_global_field_id_t;

/* TBL_TPOAM_EXEC_GLOBAL defines */
typedef struct
{
    uint32               test_type;           /*lm/lb rmep/ mip/discovery/DM*/
    uint32               oam_session_id;
    uint32               pid;
    uint32               sock;
    tpoam_lmm_exec_t     exec_lm;             /*lm*/
    tpoam_lb_exec_t      exec_lb_rmep;        /*LB TPOAM_LB_MEP*/
    tpoam_lb_exec_t      exec_lb_mip;         /*LB TPOAM_LB_MIP*/
    tpoam_lb_exec_t      exec_lb_discovery;   /*LB TPOAM_LB_DISCOVERY*/
    tpoam_1dm_dmm_exec_t exec_dmm;            /*DM, TPOAM_DM_TYPE_DMM*/
    tpoam_1dm_dmm_exec_t exec_1dm;            /*1DM, TPOAM_DM_TYPE_1DM*/
    ctc_task_t           *p_t_send_lb_msg_timer;
} tbl_tpoam_exec_global_t;

#endif /* !__TBL_TPOAM_EXEC_GLOBAL_DEFINE_H__ */

