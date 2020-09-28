
#ifndef __TBL_POE_DEFINE_H__
#define __TBL_POE_DEFINE_H__

/* TBL_POE field defines */
typedef enum
{
    TBL_POE_FLD_KEY                  = 0 ,  /* READ */
    TBL_POE_FLD_SLOT                 = 1 ,  /* READ */
    TBL_POE_FLD_GLB_ENABLE           = 2 ,  /* RW */
    TBL_POE_FLD_GLB_POWER_LIMIT      = 3 ,  /* RW */
    TBL_POE_FLD_GLB_STATUS           = 4 ,  /* RW */
    TBL_POE_FLD_GLB_CUR_CONSUMP      = 5 ,  /* RW */
    TBL_POE_FLD_GLB_AVER_CONSUMP     = 6 ,  /* RW */
    TBL_POE_FLD_GLB_PEAK_CONSUMP     = 7 ,  /* RW */
    TBL_POE_FLD_PSE_CHANNEL_NUM      = 8 ,  /* RW */
    TBL_POE_FLD_CONST_POWER_LIMIT    = 9 ,  /* RW */
    TBL_POE_FLD_PSE_CHANNEL_ID       = 10,  /* RW */
    TBL_POE_FLD_PANEL_SLOT_NO        = 11,  /* RW */
    TBL_POE_FLD_PANEL_PORT_NO        = 12,  /* RW */
    TBL_POE_FLD_PANEL_SUB_PORT_NO    = 13,  /* RW */
    TBL_POE_FLD_ENABLE               = 14,  /* RW */
    TBL_POE_FLD_FORCE_POWER          = 15,  /* RW */
    TBL_POE_FLD_MODE                 = 16,  /* RW */
    TBL_POE_FLD_PRIORITY             = 17,  /* RW */
    TBL_POE_FLD_POWER_LIMIT          = 18,  /* RW */
    TBL_POE_FLD_CLASS_LEVEL          = 19,  /* RW */
    TBL_POE_FLD_STATUS               = 20,  /* RW */
    TBL_POE_FLD_CUR_CONSUMP          = 21,  /* RW */
    TBL_POE_FLD_AVER_CONSUMP         = 22,  /* RW */
    TBL_POE_FLD_PEAK_CONSUMP         = 23,  /* RW */
    TBL_POE_FLD_CURRENT              = 24,  /* RW */
    TBL_POE_FLD_VOLTAGE              = 25,  /* RW */
    TBL_POE_FLD_TEMPER               = 26,  /* RW */
    TBL_POE_FLD_MAX                  = 27
} tbl_poe_field_id_t;

/* TBL_POE defines */
typedef struct
{
    int32                id;
} tbl_poe_key_t;

typedef struct
{
    tbl_poe_key_t        key;
    int32                slot;
    uint8                glb_enable;
    uint32               glb_power_limit;
    uint32               glb_status;
    uint32               glb_cur_consump;
    uint32               glb_aver_consump;
    uint32               glb_peak_consump;
    uint32               pse_channel_num;
    uint32               const_power_limit;
    uint32               pse_channel_id[MAX_POE_PORT_NUM];
    uint32               panel_slot_no[MAX_POE_PORT_NUM];
    uint32               panel_port_no[MAX_POE_PORT_NUM];
    uint32               panel_sub_port_no[MAX_POE_PORT_NUM];
    uint32               enable[MAX_POE_PORT_NUM];
    uint32               force_power[MAX_POE_PORT_NUM];
    uint32               mode[MAX_POE_PORT_NUM];
    uint32               priority[MAX_POE_PORT_NUM];
    uint32               power_limit[MAX_POE_PORT_NUM];
    uint32               class_level[MAX_POE_PORT_NUM];
    uint32               status[MAX_POE_PORT_NUM];
    uint32               cur_consump[MAX_POE_PORT_NUM];
    uint32               aver_consump[MAX_POE_PORT_NUM];
    uint32               peak_consump[MAX_POE_PORT_NUM];
    uint32               current[MAX_POE_PORT_NUM];
    uint32               voltage[MAX_POE_PORT_NUM];
    uint32               temper[MAX_POE_PORT_NUM];
} tbl_poe_t;

typedef struct
{
    ctclib_hash_t        *poe_hash;
    ctclib_slist_t       *poe_list;
} tbl_poe_master_t;

#endif /* !__TBL_POE_DEFINE_H__ */

