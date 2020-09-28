
#ifndef __TBL_G8032_RING_DEFINE_H__
#define __TBL_G8032_RING_DEFINE_H__

/* TBL_G8032_RING field defines */
typedef enum
{
    TBL_G8032_RING_FLD_KEY                  = 0 ,  /* READ */
    TBL_G8032_RING_FLD_RPL_OWNER            = 1 ,  /* RW */
    TBL_G8032_RING_FLD_RPL_NEIGHBOR         = 2 ,  /* RW */
    TBL_G8032_RING_FLD_RPL_NEXT_NEIGHBOR    = 3 ,  /* RW */
    TBL_G8032_RING_FLD_RING_LEVEL_ID        = 4 ,  /* RW */
    TBL_G8032_RING_FLD_PROTECTION_EN        = 5 ,  /* RW */
    TBL_G8032_RING_FLD_CONFIG_SYNC          = 6 ,  /* RW */
    TBL_G8032_RING_FLD_APS_PROTECTION_EN    = 7 ,  /* RW */
    TBL_G8032_RING_FLD_PROTECTION_TIME      = 8 ,  /* RW */
    TBL_G8032_RING_FLD_MODE_REVERTIVE       = 9 ,  /* RW */
    TBL_G8032_RING_FLD_VIRTUAL_CHANNEL      = 10,  /* RW */
    TBL_G8032_RING_FLD_G8032_MAJOR_RING_ID  = 11,  /* RW */
    TBL_G8032_RING_FLD_IS_SUB_RING          = 12,  /* RW */
    TBL_G8032_RING_FLD_INSTANCE_COUNT       = 13,  /* RW */
    TBL_G8032_RING_FLD_INSTANCE             = 14,  /* RW */
    TBL_G8032_RING_FLD_SUB_RING_ID          = 15,  /* RW */
    TBL_G8032_RING_FLD_IS_LOCAL_FS          = 16,  /* RW */
    TBL_G8032_RING_FLD_IS_LOCAL_MS          = 17,  /* RW */
    TBL_G8032_RING_FLD_EAST_PORT_IFINDEX    = 18,  /* RW */
    TBL_G8032_RING_FLD_EAST_PORT_IS_RPL     = 19,  /* RW */
    TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEIGHBOR = 20,  /* RW */
    TBL_G8032_RING_FLD_EAST_PORT_IS_RPL_NEXT_NEIGHBOR = 21,  /* RW */
    TBL_G8032_RING_FLD_EAST_PORT_IS_FORCE_SWITCH = 22,  /* RW */
    TBL_G8032_RING_FLD_EAST_PORT_IS_MANUAL_SWITCH = 23,  /* RW */
    TBL_G8032_RING_FLD_EAST_PORT_IS_BLOCKED = 24,  /* RW */
    TBL_G8032_RING_FLD_EAST_PORT_FLUSH_FDB  = 25,  /* RW */
    TBL_G8032_RING_FLD_EAST_PORT_IS_FAIL    = 26,  /* RW */
    TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID = 27,  /* RW */
    TBL_G8032_RING_FLD_EAST_PORT_REC_BPR    = 28,  /* RW */
    TBL_G8032_RING_FLD_WEST_PORT_IFINDEX    = 29,  /* RW */
    TBL_G8032_RING_FLD_WEST_PORT_IS_RPL     = 30,  /* RW */
    TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEIGHBOR = 31,  /* RW */
    TBL_G8032_RING_FLD_WEST_PORT_IS_RPL_NEXT_NEIGHBOR = 32,  /* RW */
    TBL_G8032_RING_FLD_WEST_PORT_IS_FORCE_SWITCH = 33,  /* RW */
    TBL_G8032_RING_FLD_WEST_PORT_IS_MANUAL_SWITCH = 34,  /* RW */
    TBL_G8032_RING_FLD_WEST_PORT_IS_BLOCKED = 35,  /* RW */
    TBL_G8032_RING_FLD_WEST_PORT_FLUSH_FDB  = 36,  /* RW */
    TBL_G8032_RING_FLD_WEST_PORT_IS_FAIL    = 37,  /* RW */
    TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID = 38,  /* RW */
    TBL_G8032_RING_FLD_WEST_PORT_REC_BPR    = 39,  /* RW */
    TBL_G8032_RING_FLD_CONTROL_VLAN         = 40,  /* RW */
    TBL_G8032_RING_FLD_IS_SUB_CONTROL_VLAN  = 41,  /* RW */
    TBL_G8032_RING_FLD_IS_FWDTOCPU          = 42,  /* RW */
    TBL_G8032_RING_FLD_RAPS_MEL             = 43,  /* RW */
    TBL_G8032_RING_FLD_RAPS_DNF             = 44,  /* RW */
    TBL_G8032_RING_FLD_PROTECTION_TRIGGER   = 45,  /* RW */
    TBL_G8032_RING_FLD_VLAN                 = 46,  /* RW */
    TBL_G8032_RING_FLD_PRIMARY_VLAN         = 47,  /* RW */
    TBL_G8032_RING_FLD_CURRENT_NODE_STATE   = 48,  /* RW */
    TBL_G8032_RING_FLD_PRIORITY_REQUEST     = 49,  /* RW */
    TBL_G8032_RING_FLD_HOLD_OFF_EXPIRED     = 50,  /* RW */
    TBL_G8032_RING_FLD_WTR_TIMEOUT_VAL      = 51,  /* RW */
    TBL_G8032_RING_FLD_WTB_TIMEOUT_VAL      = 52,  /* RW */
    TBL_G8032_RING_FLD_GUARD_TIMEOUT_VAL    = 53,  /* RW */
    TBL_G8032_RING_FLD_HOLD_OFF_TIMEOUT_VAL = 54,  /* RW */
    TBL_G8032_RING_FLD_FLUSH_FDB_TIMEOUT_VAL = 55,  /* RW */
    TBL_G8032_RING_FLD_CFM_MD_NAME          = 56,  /* RW */
    TBL_G8032_RING_FLD_CFM_MA_NAME          = 57,  /* RW */
    TBL_G8032_RING_FLD_LEVEL                = 58,  /* RW */
    TBL_G8032_RING_FLD_VLAN_ID              = 59,  /* RW */
    TBL_G8032_RING_FLD_EAST_INTERFACE_CFM_STATE = 60,  /* RW */
    TBL_G8032_RING_FLD_WEST_INTERFACE_CFM_STATE = 61,  /* RW */
    TBL_G8032_RING_FLD_P_RAPS_TX_WHILE      = 62,  /* READ */
    TBL_G8032_RING_FLD_P_WTR_WHILE          = 63,  /* READ */
    TBL_G8032_RING_FLD_P_WTB_WHILE          = 64,  /* READ */
    TBL_G8032_RING_FLD_P_HOLD_OFF_WHILE     = 65,  /* READ */
    TBL_G8032_RING_FLD_P_GUARD_WHILE        = 66,  /* READ */
    TBL_G8032_RING_FLD_P_FLUSH_FDB_WHILE    = 67,  /* READ */
    TBL_G8032_RING_FLD_P_TX_RAPS_NR_WHILE   = 68,  /* READ */
    TBL_G8032_RING_FLD_P_TX_RAPS_SF_WHILE   = 69,  /* READ */
    TBL_G8032_RING_FLD_P_TX_RAPS_FS_WHILE   = 70,  /* READ */
    TBL_G8032_RING_FLD_P_TX_RAPS_MS_WHILE   = 71,  /* READ */
    TBL_G8032_RING_FLD_P_TX_RAPS_NR_NB_WHILE = 72,  /* READ */
    TBL_G8032_RING_FLD_MAX                  = 73
} tbl_g8032_ring_field_id_t;

/* TBL_G8032_RING defines */
typedef struct
{
    uint32               ring_id;
} tbl_g8032_ring_key_t;

typedef struct
{
    tbl_g8032_ring_key_t key;
    uint8                rpl_owner;           /* RPL owner */
    uint8                rpl_neighbor;        /* RPL neighbor */
    uint8                rpl_next_neighbor;   /* RPL next neighbor */
    uint32               ring_level_id;       /* RPL owner */
    uint32               protection_en;       /* Ring level id */
    uint32               config_sync;         /* Ring level id */
    uint32               aps_protection_en;   /* Ring level id */
    uint32               protection_time;     /*Protection switch time */
    uint8                mode_revertive;      /*Revertive mode*/
    uint8                virtual_channel;     /*Virtual channel*/
    uint32               g8032_major_ring_id; /*If is a sub ring, the sub ring's  major ring */
    uint8                is_sub_ring;         /* Is sub-ring */
    uint32               instance_count;      /* Instance count */
    uint32               instance[G8032_MAX_INSTANCE]; /* Instance id arry */
    uint32               sub_ring_id[GLB_G8032_MAX_SUB_RINGS]; /*If not a sub ring, List of sub rings which the ring has*/
    uint8                is_local_fs;
    uint8                is_local_ms;
    uint32               east_port_ifindex;   /* East interface */
    uint8                east_port_is_rpl;    /* East interface is RPL */
    uint8                east_port_is_rpl_neighbor; /* East interface is RPL neighbor */
    uint8                east_port_is_rpl_next_neighbor; /* East interface is RPL next neighbor */
    uint8                east_port_is_force_switch; /* East interface is force switch */
    uint8                east_port_is_manual_switch; /* East interface is manual switch */
    uint8                east_port_is_blocked; /* Blocking east interface*/
    uint8                east_port_flush_fdb;
    uint8                east_port_is_fail;
    uint8                east_port_rec_node_id[6];
    uint8                east_port_rec_bpr;
    uint32               west_port_ifindex;   /* West interface */
    uint8                west_port_is_rpl;    /* West interface is RPL */
    uint8                west_port_is_rpl_neighbor; /* West interface is RPL neighbor */
    uint8                west_port_is_rpl_next_neighbor; /* West interface is RPL next neighbor */
    uint8                west_port_is_force_switch; /* West interface is force switch */
    uint8                west_port_is_manual_switch; /* West interface is manual switch */
    uint8                west_port_is_blocked; /* Blocking west interface*/
    uint8                west_port_flush_fdb;
    uint8                west_port_is_fail;
    uint8                west_port_rec_node_id[6];
    uint8                west_port_rec_bpr;
    uint32               control_vlan;        /* R-APS channel VLAN */
    uint8                is_sub_control_vlan; /* Is a subring R-APS channel VLAN */
    uint8                is_fwdtocpu;         /* Is R-APS message foward to cpu */
    uint8                raps_mel;            /* R-APS MEL value */
    uint8                raps_dnf;            /* R-APS Do not flush fdb */
    uint8                protection_trigger;  /* Protection switch trigger */
    vlan_bmp_t           vlan;                /* List of vlans which the ring has */
    vid_t                primary_vlan;        /* Primary Vlan of the ring, it's used by CFM */
    uint16               current_node_state;  /* Ring state node */
    uint32               priority_request;    /* Ring request message */
    uint8                hold_off_expired;    /* Indicates whether hold-off expires of not */
    uint32               wtr_timeout_val;     /* Wait-to-restore timeout value  */
    uint32               wtb_timeout_val;
    uint32               guard_timeout_val;   /* Gard timer timeout value  */
    uint32               hold_off_timeout_val; /* Hold off timer timeout value */
    uint32               flush_fdb_timeout_val; /* Flush fdb timer timeout value */
    char                 cfm_md_name[CFM_MD_NAME_MAX_LENGTH_8021AG+1]; /* CFM MD name */
    char                 cfm_ma_name[CFM_MA_NAME_MAX_LENGTH_8021AG+1]; /* CFM MA name */
    uint16               level;               /* CFM MD level */
    uint16               vlan_id;             /* CFM MA vlan id */
    uint16               east_interface_cfm_state; /* CFM cc state for east interface */
    uint16               west_interface_cfm_state; /* CFM cc state for west interface */
    ctc_task_t           *p_raps_tx_while;    /* Retransmission timer */
    ctc_task_t           *p_wtr_while;        /* Wait-to-restore timer */
    ctc_task_t           *p_wtb_while;        /* Wait-to-block timer */
    ctc_task_t           *p_hold_off_while;   /* Hold-off timer */
    ctc_task_t           *p_guard_while;      /* Guard-off timer */
    ctc_task_t           *p_flush_fdb_while;  /* Configuration complete */
    ctc_task_t           *p_tx_raps_nr_while; /*Timer to transmit raps_nr packets */
    ctc_task_t           *p_tx_raps_sf_while; /*Timer to transmit raps_sf packets */
    ctc_task_t           *p_tx_raps_fs_while; /*Timer to transmit raps_fs packets */
    ctc_task_t           *p_tx_raps_ms_while; /*Timer to transmit raps_ms packets */
    ctc_task_t           *p_tx_raps_nr_nb_while; /*Timer to transmit raps_nr_nb packets */
} tbl_g8032_ring_t;

typedef struct
{
    ctclib_hash_t        *g8032_ring_hash;
    ctclib_slist_t       *g8032_ring_list;
} tbl_g8032_ring_master_t;

#endif /* !__TBL_G8032_RING_DEFINE_H__ */

