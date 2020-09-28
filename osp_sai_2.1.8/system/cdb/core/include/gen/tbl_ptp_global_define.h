
#ifndef __TBL_PTP_GLOBAL_DEFINE_H__
#define __TBL_PTP_GLOBAL_DEFINE_H__

/* TBL_PTP_GLOBAL field defines */
typedef enum
{
    TBL_PTP_GLOBAL_FLD_DEFAULT_DS           = 0 ,  /* RW */
    TBL_PTP_GLOBAL_FLD_CURRENT_DS           = 1 ,  /* RW */
    TBL_PTP_GLOBAL_FLD_PARENT_DS            = 2 ,  /* RW */
    TBL_PTP_GLOBAL_FLD_TIME_PROP_DS         = 3 ,  /* RW */
    TBL_PTP_GLOBAL_FLD_TIME_PROP_INTERN     = 4 ,  /* RW */
    TBL_PTP_GLOBAL_FLD_DEVICE_TYPE          = 5 ,  /* RW */
    TBL_PTP_GLOBAL_FLD_GLOBAL_ENABLE        = 6 ,  /* RW */
    TBL_PTP_GLOBAL_FLD_MASTER_MIN_DELAY_REQ_INTERVAL = 7 ,  /* RW */
    TBL_PTP_GLOBAL_FLD_C                    = 8 ,  /* RW */
    TBL_PTP_GLOBAL_FLD_P_INTERNAL_TIMER     = 9 ,  /* READ */
    TBL_PTP_GLOBAL_FLD_CURRENT_SLAVE_PORT   = 10,  /* RW */
    TBL_PTP_GLOBAL_FLD_SET_SYSTEM_TIME      = 11,  /* RW */
    TBL_PTP_GLOBAL_FLD_MASTERCHANGETRAP     = 12,  /* RW */
    TBL_PTP_GLOBAL_FLD_STATECHANGETRAP      = 13,  /* RW */
    TBL_PTP_GLOBAL_FLD_ACCURACYCHANGE       = 14,  /* RW */
    TBL_PTP_GLOBAL_FLD_GLB_ENABLE_HW_MSEC_TIME = 15,  /* RW */
    TBL_PTP_GLOBAL_FLD_GLB_ENABLE_PF_MSEC_TIME = 16,  /* RW */
    TBL_PTP_GLOBAL_FLD_TOTAL_OFFSET         = 17,  /* RW */
    TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_OFFSET   = 18,  /* RW */
    TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_FFO      = 19,  /* RW */
    TBL_PTP_GLOBAL_FLD_HSRV_UPDATE_SYSTEM_TIME = 20,  /* RW */
    TBL_PTP_GLOBAL_FLD_HSRV_DEVICE_TYPE     = 21,  /* RW */
    TBL_PTP_GLOBAL_FLD_MAX                  = 22
} tbl_ptp_global_field_id_t;

/* TBL_PTP_GLOBAL defines */
typedef struct
{
    ptp_default_t        default_ds;
    ptp_current_t        current_ds;
    ptp_parent_t         parent_ds;
    ptp_time_properties_t time_prop_ds;
    ptp_time_properties_t time_prop_intern;    /*non-standard, for cmd cfg*/
    uint8                device_type;         /*ptp device-type (bc|oc|e2etc|p2ptc)*/
    uint32               global_enable;       /*ptp global-enable*/
    int8                 master_min_delay_req_interval; /*from msg log_message_interval, for delay_req_interval calc*/
    ptp_global_c_t       c;                   /*servo*/
    ctc_task_t           *p_internal_timer;   /*BMC cal*/
    char                 current_slave_port[IFNAME_SIZE]; /*only one slave port*/
    uint32               set_system_time;     /*ptp set-systime*/
    uint32               masterChangeTrap;    /*snmp-server trap enable ptp master-change*/
    uint32               stateChangeTrap;     /*snmp-server trap enable ptp state-change*/
    uint32               accuracyChange;      /*snmp-server trap enable ptp accuracy-change*/
    uint64               glb_enable_hw_msec_time; /*global enable hw msec time*/
    uint64               glb_enable_pf_msec_time; /*global enable platform msec time*/
    int64                total_offset;        /*total offset*/
    int64                hsrv_adjust_offset;  /*for hsrv adjust offset*/
    int64                hsrv_adjust_ffo;     /*for hsrv adjust ffo*/
    int16                hsrv_update_system_time; /*ptp update systime*/
    uint8                hsrv_device_type;    /*ptp device-type (bc|oc|e2etc|p2ptc)*/
} tbl_ptp_global_t;

#endif /* !__TBL_PTP_GLOBAL_DEFINE_H__ */

