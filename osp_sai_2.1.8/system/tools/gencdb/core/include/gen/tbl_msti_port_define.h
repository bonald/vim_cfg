
#ifndef __TBL_MSTI_PORT_DEFINE_H__
#define __TBL_MSTI_PORT_DEFINE_H__

/* TBL_MSTI_PORT field defines */
typedef enum
{
    TBL_MSTI_PORT_FLD_KEY                  = 0 ,  /* READ */
    TBL_MSTI_PORT_FLD_IFINDEX              = 1 ,  /* READ */
    TBL_MSTI_PORT_FLD_ADMINPORTPATHCOST    = 2 ,  /* RW */
    TBL_MSTI_PORT_FLD_PORTPATHCOST         = 3 ,  /* READ */
    TBL_MSTI_PORT_FLD_FORWARD_TRANSITIONS  = 4 ,  /* READ */
    TBL_MSTI_PORT_FLD_NEWINFOMSTI          = 5 ,  /* READ */
    TBL_MSTI_PORT_FLD_BPDU_LOOP_COUNT      = 6 ,  /* READ */
    TBL_MSTI_PORT_FLD_RESTRICTED_ROLE      = 7 ,  /* RW */
    TBL_MSTI_PORT_FLD_RESTRICTED_TCN       = 8 ,  /* RW */
    TBL_MSTI_PORT_FLD_OPER_ROOTGUARD       = 9 ,  /* READ */
    TBL_MSTI_PORT_FLD_OPER_LOOPGUARD       = 10,  /* READ */
    TBL_MSTI_PORT_FLD_AGREE                = 11,  /* READ */
    TBL_MSTI_PORT_FLD_AGREED               = 12,  /* READ */
    TBL_MSTI_PORT_FLD_ADMINFORCEVERSION    = 13,  /* RW */
    TBL_MSTI_PORT_FLD_DESIGNATED_REGIONROOTID = 14,  /* READ */
    TBL_MSTI_PORT_FLD_DESIGNATED_INTROOTPATHCOST = 15,  /* READ */
    TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID = 16,  /* READ */
    TBL_MSTI_PORT_FLD_DESIGNATED_DESIGNATEDPORTID = 17,  /* READ */
    TBL_MSTI_PORT_FLD_DESIGNATED_BRIDGEPORTID = 18,  /* READ */
    TBL_MSTI_PORT_FLD_DESIGNATED_MESSAGEAGE = 19,  /* READ */
    TBL_MSTI_PORT_FLD_DESIGNATED_FWDDELAY  = 20,  /* READ */
    TBL_MSTI_PORT_FLD_DESIGNATED_HELLOTIME = 21,  /* READ */
    TBL_MSTI_PORT_FLD_DESIGNATED_MAXAGE    = 22,  /* READ */
    TBL_MSTI_PORT_FLD_DISPUTED             = 23,  /* READ */
    TBL_MSTI_PORT_FLD_FDBFLUSH             = 24,  /* READ */
    TBL_MSTI_PORT_FLD_FORWARD              = 25,  /* READ */
    TBL_MSTI_PORT_FLD_FORWARDING           = 26,  /* READ */
    TBL_MSTI_PORT_FLD_INFOIS               = 27,  /* READ */
    TBL_MSTI_PORT_FLD_LEARN                = 28,  /* READ */
    TBL_MSTI_PORT_FLD_LEARNING             = 29,  /* READ */
    TBL_MSTI_PORT_FLD_NEWINFO              = 30,  /* READ */
    TBL_MSTI_PORT_FLD_PORTID               = 31,  /* READ */
    TBL_MSTI_PORT_FLD_PRIORITY             = 32,  /* RW */
    TBL_MSTI_PORT_FLD_PORT_ROOTID          = 33,  /* READ */
    TBL_MSTI_PORT_FLD_PORT_ROOTPATHCOST    = 34,  /* READ */
    TBL_MSTI_PORT_FLD_PORT_DESIGNATEDBRIDGEID = 35,  /* READ */
    TBL_MSTI_PORT_FLD_PORT_DESIGNATEDPORTID = 36,  /* READ */
    TBL_MSTI_PORT_FLD_PORT_BRIDGEPORTID    = 37,  /* READ */
    TBL_MSTI_PORT_FLD_PORT_MESSAGEAGE      = 38,  /* READ */
    TBL_MSTI_PORT_FLD_PORT_FWDDELAY        = 39,  /* READ */
    TBL_MSTI_PORT_FLD_PORT_HELLOTIME       = 40,  /* READ */
    TBL_MSTI_PORT_FLD_PORT_MAXAGE          = 41,  /* READ */
    TBL_MSTI_PORT_FLD_PROPOSED             = 42,  /* READ */
    TBL_MSTI_PORT_FLD_PROPOSING            = 43,  /* READ */
    TBL_MSTI_PORT_FLD_RCVDBPDU             = 44,  /* READ */
    TBL_MSTI_PORT_FLD_RCVDINFO             = 45,  /* READ */
    TBL_MSTI_PORT_FLD_RCVDMSG              = 46,  /* READ */
    TBL_MSTI_PORT_FLD_RCVDRSTP             = 47,  /* READ */
    TBL_MSTI_PORT_FLD_RCVDSTP              = 48,  /* READ */
    TBL_MSTI_PORT_FLD_RCVDTC               = 49,  /* READ */
    TBL_MSTI_PORT_FLD_RCVDTCACK            = 50,  /* READ */
    TBL_MSTI_PORT_FLD_RCVDTCN              = 51,  /* READ */
    TBL_MSTI_PORT_FLD_REROOT               = 52,  /* READ */
    TBL_MSTI_PORT_FLD_RESELECT             = 53,  /* READ */
    TBL_MSTI_PORT_FLD_ROLE                 = 54,  /* READ */
    TBL_MSTI_PORT_FLD_SELECTED             = 55,  /* READ */
    TBL_MSTI_PORT_FLD_SELECTEDROLE         = 56,  /* READ */
    TBL_MSTI_PORT_FLD_SYNC                 = 57,  /* READ */
    TBL_MSTI_PORT_FLD_SYNCED               = 58,  /* READ */
    TBL_MSTI_PORT_FLD_TCACK                = 59,  /* READ */
    TBL_MSTI_PORT_FLD_TCPROP               = 60,  /* READ */
    TBL_MSTI_PORT_FLD_TXCOUNT              = 61,  /* READ */
    TBL_MSTI_PORT_FLD_UPDTINFO             = 62,  /* READ */
    TBL_MSTI_PORT_FLD_REROOTED             = 63,  /* READ */
    TBL_MSTI_PORT_FLD_HOPCOUNT             = 64,  /* READ */
    TBL_MSTI_PORT_FLD_PPM_STATE            = 65,  /* READ */
    TBL_MSTI_PORT_FLD_PIM_STATE            = 66,  /* READ */
    TBL_MSTI_PORT_FLD_PRT_STATE            = 67,  /* READ */
    TBL_MSTI_PORT_FLD_PST_STATE            = 68,  /* READ */
    TBL_MSTI_PORT_FLD_TCM_STATE            = 69,  /* READ */
    TBL_MSTI_PORT_FLD_FEA_STATE            = 70,  /* READ */
    TBL_MSTI_PORT_FLD_BPDU                 = 71,  /* READ */
    TBL_MSTI_PORT_FLD_FDWHILE_TIMER        = 72,  /* READ */
    TBL_MSTI_PORT_FLD_MDELAYWHILE_TIMER    = 73,  /* READ */
    TBL_MSTI_PORT_FLD_RBWHILE_TIMER        = 74,  /* READ */
    TBL_MSTI_PORT_FLD_RCVDINFOWHILE_TIMER  = 75,  /* READ */
    TBL_MSTI_PORT_FLD_RRWHILE_TIMER        = 76,  /* READ */
    TBL_MSTI_PORT_FLD_TCWHILE_TIMER        = 77,  /* READ */
    TBL_MSTI_PORT_FLD_HOLD_TIMER           = 78,  /* READ */
    TBL_MSTI_PORT_FLD_MAX                  = 79
} tbl_msti_port_field_id_t;

/* TBL_MSTI_PORT defines */
typedef msti_port_key_t tbl_msti_port_key_t;

typedef struct
{
    tbl_msti_port_key_t  key;
    uint32               ifindex;
    uint32               adminPortPathCost;   /* configured PortPathCost */
    uint32               portPathCost;        /* 17.13.11 PortPathCost, equal to adminPortPathCost or calc by bandwidth */
    uint32               forward_transitions;
    uint8                newInfoMsti;
    uint8                bpdu_loop_count;
    uint8                restricted_role;
    uint8                restricted_tcn;
    uint8                oper_rootguard;
    uint8                oper_loopguard;
    uint8                agree;               /* 17.19.2 agree */
    uint8                agreed;              /* 17.19.3 agreed */
    uint8                adminForceVersion;   /* Configured forceVersion */
    mstp_brg_id_t        designated_regionRootId; /* 17.19.4/13.24.2 designatedPriority regionRootId */
    uint32               designated_intRootPathCost; /* 17.19.4/13.24.2 designatedPriority internalRootPathCost */
    mstp_brg_id_t        designated_designatedBridgeId; /* 17.19.4/13.24.2 designatedPriority designatedBridgeId */
    uint16               designated_designatedPortId; /* 17.19.4/13.24.2 designatedPriority designatedPortId */
    uint16               designated_bridgePortId; /* 17.19.4/13.24.2 designatedPriority bridgePortId */
    uint8                designated_messageAge; /* 17.19.5 designatedTimes - messageAge */
    uint8                designated_fwdDelay; /* 17.19.5 designatedTimes - 17.20.6 FwdDelay */
    uint8                designated_helloTime; /* 17.19.5 designatedTimes - 17.20.7 HelloTime */
    uint8                designated_maxAge;   /* 17.19.5 designatedTimes - 17.20.8 MaxAge */
    uint8                disputed;            /* 17.19.6 disputed */
    uint8                fdbFlush;            /* 17.19.7 fdbFlush */
    uint8                forward;             /* 17.19.8 forward */
    uint8                forwarding;          /* 17.19.9 forwarding */
    uint8                infoIs;              /* 17.19.10 infoIs */
    uint8                learn;               /* 17.19.11 learn */
    uint8                learning;            /* 17.19.12 learning */
    uint8                newInfo;             /* 17.19.16 newInfo */
    uint16               portId;              /* 17.19.19 portId, Combo of ifindex and priority */
    uint16               priority;            /* port priority */
    mstp_brg_id_t        port_rootId;         /* 17.19.21 portPriority - rootId */
    uint32               port_rootPathCost;   /* 17.19.21 portPriority - rootPathCost */
    mstp_brg_id_t        port_designatedBridgeId; /* 17.19.21 portPriority - designatedBridgeId */
    uint16               port_designatedPortId; /* 17.19.21 portPriority - designatedPortId */
    uint16               port_bridgePortId;   /* 17.19.21 portPriority - bridgePortId */
    uint8                port_messageAge;     /* 17.19.22 portTimes - messageAge */
    uint8                port_fwdDelay;       /* 17.19.22 portTimes - 17.20.6 FwdDelay */
    uint8                port_helloTime;      /* 17.19.22 portTimes - 17.20.7 HelloTime */
    uint8                port_maxAge;         /* 17.19.22 portTimes - 17.20.8 MaxAge */
    uint8                proposed;            /* 17.19.23 proposed */
    uint8                proposing;           /* 17.19.24 proposing */
    uint8                rcvdBpdu;            /* 17.19.25 rcvdBpdu */
    uint8                rcvdInfo;            /* 17.19.26 rcvdInfo, result of the rcvInfo() procedure (17.21.8). */
    uint8                rcvdMsg;             /* 17.19.27 rcvdMsg */
    uint8                rcvdRSTP;            /* 17.19.28 rcvdRSTP */
    uint8                rcvdSTP;             /* 17.19.29 rcvdSTP */
    uint8                rcvdTc;              /* 17.19.30 rcvdTc */
    uint8                rcvdTcAck;           /* 17.19.31 rcvdTcAck*/
    uint8                rcvdTcn;             /* 17.19.32 rcvdTcn */
    uint8                reRoot;              /* 17.19.33 reRoot */
    uint8                reselect;            /* 17.19.34 reselect */
    uint8                role;                /* 17.19.35 role, mstp_port_role_t */
    uint8                selected;            /* 17.19.36 selected */
    uint8                selectedRole;        /* 17.19.37 selectedRole, mstp_port_role_t */
    uint8                sync;                /* 17.19.39 sync */
    uint8                synced;              /* 17.19.40 synced */
    uint8                tcAck;               /* 17.19.41 tcAck */
    uint8                tcProp;              /* 17.19.42 tcProp */
    uint8                txCount;             /* 17.19.44 txCount */
    uint8                updtInfo;            /* 17.19.45 updtInfo */
    uint8                reRooted;            /* 17.20.10 reRooted */
    uint8                hopCount;            /* 13.24.3 remaining Hops */
    uint8                ppm_state;           /* 17.24 ppm_state_t - Port Protocol Migration */
    uint8                pim_state;           /* 17.27 pim_state_t - Port Information */
    uint8                prt_state;           /* 17.29 prt_state_t - Port Role Transition */
    uint8                pst_state;           /* 17.30 pst_state_t - Port State Transition */
    uint8                tcm_state;           /* 17.31 tcm_state_t - Topology Change */
    uint8                fea_state;           /* Port hw state, GLB_BR_PORT_STATE_XXX */
    void                 *bpdu;               /* msti_bpdu_t */
    ctc_task_t           *fdWhile_timer;      /* fdWhile timer Section 17.17.2 */
    ctc_task_t           *mdelayWhile_timer;  /* mdelayWhile Timer Section 17.17.4 */
    ctc_task_t           *rbWhile_timer;      /* rbWhile timer Section 17.17.5 */
    ctc_task_t           *rcvdInfoWhile_timer; /* rcvdInfoWhile Section 17.17.6 */
    ctc_task_t           *rrWhile_timer;      /* rrWhile timer Section 17.17.7 */
    ctc_task_t           *tcWhile_timer;      /* tcWhile timer Section 17.17.8 */
    ctc_task_t           *hold_timer;
    void                 *p_if;               /* refer to tbl_interface_t */
    void                 *p_port;             /* refer to tbl_mstp_port_t */
} tbl_msti_port_t;

typedef struct
{
    ctclib_hash_t        *port_msti_hash;
    ctclib_slist_t       *port_msti_list;
} tbl_msti_port_master_t;

#endif /* !__TBL_MSTI_PORT_DEFINE_H__ */

