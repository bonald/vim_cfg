
#ifndef __TBL_MSTP_PORT_DEFINE_H__
#define __TBL_MSTP_PORT_DEFINE_H__

/* TBL_MSTP_PORT field defines */
typedef enum
{
    TBL_MSTP_PORT_FLD_KEY                  = 0 ,  /* READ */
    TBL_MSTP_PORT_FLD_IFINDEX              = 1 ,  /* READ */
    TBL_MSTP_PORT_FLD_ADMINPORTPATHCOST    = 2 ,  /* RW */
    TBL_MSTP_PORT_FLD_PORTPATHCOST         = 3 ,  /* READ */
    TBL_MSTP_PORT_FLD_FORWARD_TRANSITIONS  = 4 ,  /* READ */
    TBL_MSTP_PORT_FLD_TX_BPDU_COUNT        = 5 ,  /* READ */
    TBL_MSTP_PORT_FLD_TX_ERROR_COUNT       = 6 ,  /* READ */
    TBL_MSTP_PORT_FLD_RX_BPDU_COUNT        = 7 ,  /* READ */
    TBL_MSTP_PORT_FLD_RX_ERROR_COUNT       = 8 ,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_ENABLE          = 9 ,  /* RW */
    TBL_MSTP_PORT_FLD_BPDU_LOOP_COUNT      = 10,  /* READ */
    TBL_MSTP_PORT_FLD_RESTRICTED_ROLE      = 11,  /* RW */
    TBL_MSTP_PORT_FLD_RESTRICTED_TCN       = 12,  /* RW */
    TBL_MSTP_PORT_FLD_ADMIN_BPDUGUARD      = 13,  /* RW */
    TBL_MSTP_PORT_FLD_OPER_BPDUGUARD       = 14,  /* READ */
    TBL_MSTP_PORT_FLD_ADMIN_BPDUFILTER     = 15,  /* RW */
    TBL_MSTP_PORT_FLD_OPER_BPDUFILTER      = 16,  /* READ */
    TBL_MSTP_PORT_FLD_ADMIN_ROOTGUARD      = 17,  /* RW */
    TBL_MSTP_PORT_FLD_OPER_ROOTGUARD       = 18,  /* READ */
    TBL_MSTP_PORT_FLD_ADMIN_LOOPGUARD      = 19,  /* RW */
    TBL_MSTP_PORT_FLD_OPER_LOOPGUARD       = 20,  /* READ */
    TBL_MSTP_PORT_FLD_CISCO_CFG_FORMAT_ID  = 21,  /* READ */
    TBL_MSTP_PORT_FLD_ADMINP2PMAC          = 22,  /* RW */
    TBL_MSTP_PORT_FLD_OPERP2PMAC           = 23,  /* READ */
    TBL_MSTP_PORT_FLD_AGREE                = 24,  /* READ */
    TBL_MSTP_PORT_FLD_AGREED               = 25,  /* READ */
    TBL_MSTP_PORT_FLD_ADMINFORCEVERSION    = 26,  /* RW */
    TBL_MSTP_PORT_FLD_FORCEVERSION         = 27,  /* READ */
    TBL_MSTP_PORT_FLD_DESIGNATED_ROOTID    = 28,  /* READ */
    TBL_MSTP_PORT_FLD_DESIGNATED_ROOTPATHCOST = 29,  /* READ */
    TBL_MSTP_PORT_FLD_DESIGNATED_REGIONROOTID = 30,  /* READ */
    TBL_MSTP_PORT_FLD_DESIGNATED_INTROOTPATHCOST = 31,  /* READ */
    TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID = 32,  /* READ */
    TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDPORTID = 33,  /* READ */
    TBL_MSTP_PORT_FLD_DESIGNATED_BRIDGEPORTID = 34,  /* READ */
    TBL_MSTP_PORT_FLD_DESIGNATED_MESSAGEAGE = 35,  /* READ */
    TBL_MSTP_PORT_FLD_DESIGNATED_FWDDELAY  = 36,  /* READ */
    TBL_MSTP_PORT_FLD_DESIGNATED_HELLOTIME = 37,  /* READ */
    TBL_MSTP_PORT_FLD_DESIGNATED_MAXAGE    = 38,  /* READ */
    TBL_MSTP_PORT_FLD_DISPUTED             = 39,  /* READ */
    TBL_MSTP_PORT_FLD_FDBFLUSH             = 40,  /* READ */
    TBL_MSTP_PORT_FLD_FORWARD              = 41,  /* READ */
    TBL_MSTP_PORT_FLD_FORWARDING           = 42,  /* READ */
    TBL_MSTP_PORT_FLD_INFOIS               = 43,  /* READ */
    TBL_MSTP_PORT_FLD_LEARN                = 44,  /* READ */
    TBL_MSTP_PORT_FLD_LEARNING             = 45,  /* READ */
    TBL_MSTP_PORT_FLD_MCHECK               = 46,  /* RW */
    TBL_MSTP_PORT_FLD_NEWINFO              = 47,  /* READ */
    TBL_MSTP_PORT_FLD_NEWINFOMSTI          = 48,  /* READ */
    TBL_MSTP_PORT_FLD_ADMINEDGE            = 49,  /* RW */
    TBL_MSTP_PORT_FLD_OPEREDGE             = 50,  /* READ */
    TBL_MSTP_PORT_FLD_PORTENABLED          = 51,  /* READ */
    TBL_MSTP_PORT_FLD_PORTID               = 52,  /* READ */
    TBL_MSTP_PORT_FLD_PRIORITY             = 53,  /* RW */
    TBL_MSTP_PORT_FLD_PORT_ROOTID          = 54,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_ROOTPATHCOST    = 55,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_REGIONROOTID    = 56,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_INTROOTPATHCOST = 57,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_DESIGNATEDBRIDGEID = 58,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_DESIGNATEDPORTID = 59,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_BRIDGEPORTID    = 60,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_MESSAGEAGE      = 61,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_FWDDELAY        = 62,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_HELLOTIME       = 63,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_MAXAGE          = 64,  /* READ */
    TBL_MSTP_PORT_FLD_PROPOSED             = 65,  /* READ */
    TBL_MSTP_PORT_FLD_PROPOSING            = 66,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDBPDU             = 67,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDINFO             = 68,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDINFOUPDATETIMERS = 69,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDMSG              = 70,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDMSTP             = 71,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDRSTP             = 72,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDSTP              = 73,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDTC               = 74,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDTCACK            = 75,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDTCN              = 76,  /* READ */
    TBL_MSTP_PORT_FLD_REROOT               = 77,  /* READ */
    TBL_MSTP_PORT_FLD_RESELECT             = 78,  /* READ */
    TBL_MSTP_PORT_FLD_ROLE                 = 79,  /* READ */
    TBL_MSTP_PORT_FLD_SELECTED             = 80,  /* READ */
    TBL_MSTP_PORT_FLD_SELECTEDROLE         = 81,  /* READ */
    TBL_MSTP_PORT_FLD_SENDRSTP             = 82,  /* READ */
    TBL_MSTP_PORT_FLD_SYNC                 = 83,  /* READ */
    TBL_MSTP_PORT_FLD_SYNCED               = 84,  /* READ */
    TBL_MSTP_PORT_FLD_TCACK                = 85,  /* READ */
    TBL_MSTP_PORT_FLD_TCPROP               = 86,  /* READ */
    TBL_MSTP_PORT_FLD_TXCOUNT              = 87,  /* READ */
    TBL_MSTP_PORT_FLD_UPDTINFO             = 88,  /* READ */
    TBL_MSTP_PORT_FLD_REROOTED             = 89,  /* READ */
    TBL_MSTP_PORT_FLD_RSTPVERSION          = 90,  /* READ */
    TBL_MSTP_PORT_FLD_STPVERSION           = 91,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDINTERNAL         = 92,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDINTERNALCHANGE   = 93,  /* READ */
    TBL_MSTP_PORT_FLD_HOPCOUNT             = 94,  /* READ */
    TBL_MSTP_PORT_FLD_PRX_STATE            = 95,  /* READ */
    TBL_MSTP_PORT_FLD_PPM_STATE            = 96,  /* READ */
    TBL_MSTP_PORT_FLD_BDM_STATE            = 97,  /* READ */
    TBL_MSTP_PORT_FLD_PTX_STATE            = 98,  /* READ */
    TBL_MSTP_PORT_FLD_PIM_STATE            = 99,  /* READ */
    TBL_MSTP_PORT_FLD_PRT_STATE            = 100,  /* READ */
    TBL_MSTP_PORT_FLD_PST_STATE            = 101,  /* READ */
    TBL_MSTP_PORT_FLD_TCM_STATE            = 102,  /* READ */
    TBL_MSTP_PORT_FLD_FEA_STATE            = 103,  /* READ */
    TBL_MSTP_PORT_FLD_BPDU                 = 104,  /* READ */
    TBL_MSTP_PORT_FLD_PORT_TIMER           = 105,  /* READ */
    TBL_MSTP_PORT_FLD_EDGEDELAYWHILE_TIMER = 106,  /* READ */
    TBL_MSTP_PORT_FLD_FDWHILE_TIMER        = 107,  /* READ */
    TBL_MSTP_PORT_FLD_HELLOWHEN_TIMER      = 108,  /* READ */
    TBL_MSTP_PORT_FLD_MDELAYWHILE_TIMER    = 109,  /* READ */
    TBL_MSTP_PORT_FLD_RBWHILE_TIMER        = 110,  /* READ */
    TBL_MSTP_PORT_FLD_RCVDINFOWHILE_TIMER  = 111,  /* READ */
    TBL_MSTP_PORT_FLD_RRWHILE_TIMER        = 112,  /* READ */
    TBL_MSTP_PORT_FLD_TCWHILE_TIMER        = 113,  /* READ */
    TBL_MSTP_PORT_FLD_HOLD_TIMER           = 114,  /* READ */
    TBL_MSTP_PORT_FLD_MSTI_LIST            = 115,  /* READ */
    TBL_MSTP_PORT_FLD_MAX                  = 116
} tbl_mstp_port_field_id_t;

/* TBL_MSTP_PORT defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_mstp_port_key_t;

typedef struct
{
    tbl_mstp_port_key_t  key;
    uint32               ifindex;
    uint32               adminPortPathCost;   /* configured PortPathCost */
    uint32               portPathCost;        /* 17.13.11 PortPathCost, equal to adminPortPathCost or calc by bandwidth */
    uint32               forward_transitions;
    uint32               tx_bpdu_count;
    uint32               tx_error_count;
    uint32               rx_bpdu_count;
    uint32               rx_error_count;
    uint8                port_enable;
    uint8                bpdu_loop_count;
    uint8                restricted_role;
    uint8                restricted_tcn;
    uint8                admin_bpduguard;
    uint8                oper_bpduguard;
    uint8                admin_bpdufilter;
    uint8                oper_bpdufilter;
    uint8                admin_rootguard;
    uint8                oper_rootguard;
    uint8                admin_loopguard;
    uint8                oper_loopguard;
    uint8                cisco_cfg_format_id;
    uint8                adminP2PMac;         /* 6.4.3 Point-to-Point MAC parameters, adminPointToPointMAC, mstp_link_t */
    uint8                operP2PMac;          /* 6.4.3 Point-to-Point MAC parameters, operPointToPointMAC, TRUE/FALSE */
    uint8                agree;               /* 17.19.2 agree */
    uint8                agreed;              /* 17.19.3 agreed */
    uint8                adminForceVersion;   /* Configured forceVersion */
    uint8                forceVersion;        /* 17.13.4 Force Protocol Version */
    mstp_brg_id_t        designated_rootId;   /* 17.19.4/13.24.2 designatedPriority rootId */
    uint32               designated_rootPathCost; /* 17.19.4/13.24.2 designatedPriority rootPathCost */
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
    uint8                mcheck;              /* 17.19.13 mcheck */
    uint8                newInfo;             /* 17.19.16 newInfo */
    uint8                newInfoMsti;         /* 13.24.8 newInfoMsti */
    uint8                adminEdge;           /* 17.13.1 Admin Edge Port */
    uint8                operEdge;            /* 17.19.17 operEdge */
    uint8                portEnabled;         /* 17.19.18 portEnabled */
    uint16               portId;              /* 17.19.19 portId, Combo of ifindex and priority */
    uint16               priority;            /* port priority */
    mstp_brg_id_t        port_rootId;         /* 17.19.21/13.24.12 portPriority - rootId */
    uint32               port_rootPathCost;   /* 17.19.21/13.24.12 portPriority - rootPathCost */
    mstp_brg_id_t        port_regionRootId;   /* 17.19.21/13.24.12 portPriority - regionRootId */
    uint32               port_intRootPathCost; /* 17.19.21/13.24.12 portPriority - internalRootPathCost */
    mstp_brg_id_t        port_designatedBridgeId; /* 17.19.21/13.24.12 portPriority - designatedBridgeId */
    uint16               port_designatedPortId; /* 17.19.21/13.24.12 portPriority - designatedPortId */
    uint16               port_bridgePortId;   /* 17.19.21/13.24.12 portPriority - bridgePortId */
    uint8                port_messageAge;     /* 17.19.22 portTimes - messageAge */
    uint8                port_fwdDelay;       /* 17.19.22 portTimes - 17.20.6 FwdDelay */
    uint8                port_helloTime;      /* 17.19.22 portTimes - 17.20.7 HelloTime */
    uint8                port_maxAge;         /* 17.19.22 portTimes - 17.20.8 MaxAge */
    uint8                proposed;            /* 17.19.23 proposed */
    uint8                proposing;           /* 17.19.24 proposing */
    uint8                rcvdBpdu;            /* 17.19.25 rcvdBpdu */
    uint8                rcvdInfo;            /* 17.19.26 rcvdInfo, result of the rcvInfo() procedure (17.21.8). */
    uint8                rcvdInfoUpdateTimers; /* add for update timers */
    uint8                rcvdMsg;             /* 17.19.27 rcvdMsg */
    uint8                rcvdMSTP;
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
    uint8                sendRstp;            /* 17.19.38 sendRSTP */
    uint8                sync;                /* 17.19.39 sync */
    uint8                synced;              /* 17.19.40 synced */
    uint8                tcAck;               /* 17.19.41 tcAck */
    uint8                tcProp;              /* 17.19.42 tcProp */
    uint8                txCount;             /* 17.19.44 txCount */
    uint8                updtInfo;            /* 17.19.45 updtInfo */
    uint8                reRooted;            /* 17.20.10 reRooted */
    uint8                rstpVersion;         /* 17.20.11 rstpVersion */
    uint8                stpVersion;          /* 17.20.12 stpVersion */
    uint8                rcvdInternal;        /* 13.24.14 rcvdInternal */
    uint8                rcvdInternalChange;  /* store the port rcvdInternal is changed compare previous rcvdInternal  */
    uint8                hopCount;            /* 13.24.3 remaining Hops */
    uint8                prx_state;           /* 17.23 prx_state_t - Port Receive */
    uint8                ppm_state;           /* 17.24 ppm_state_t - Port Protocol Migration */
    uint8                bdm_state;           /* 17.25 bdm_state_t - Bridge Detection */
    uint8                ptx_state;           /* 17.26 ptx_state_t - Port Transmit */
    uint8                pim_state;           /* 17.27 pim_state_t - Port Information */
    uint8                prt_state;           /* 17.29 prt_state_t - Port Role Transition */
    uint8                pst_state;           /* 17.30 pst_state_t - Port State Transition */
    uint8                tcm_state;           /* 17.31 tcm_state_t - Topology Change */
    uint8                fea_state;           /* Port hw state, GLB_BR_PORT_STATE_XXX */
    void                 *bpdu;               /* bpdu_t */
    ctc_task_t           *port_timer;
    ctc_task_t           *edgeDelayWhile_timer; /* edgeDelayWhile timer Section 17.17.1 */
    ctc_task_t           *fdWhile_timer;      /* fdWhile timer Section 17.17.2 */
    ctc_task_t           *helloWhen_timer;    /* helloWhen timer Section 17.17.3 */
    ctc_task_t           *mdelayWhile_timer;  /* mdelayWhile Timer Section 17.17.4 */
    ctc_task_t           *rbWhile_timer;      /* rbWhile timer Section 17.17.5 */
    ctc_task_t           *rcvdInfoWhile_timer; /* rcvdInfoWhile Section 17.17.6 */
    ctc_task_t           *rrWhile_timer;      /* rrWhile timer Section 17.17.7 */
    ctc_task_t           *tcWhile_timer;      /* tcWhile timer Section 17.17.8 */
    ctc_task_t           *hold_timer;
    cdb_reference_list_t msti_list;           /* MSTI instance list */
    void                 *p_if;               /* refer to tbl_interface_t */
} tbl_mstp_port_t;

typedef struct
{
    ctclib_hash_t        *port_hash;
    ctclib_slist_t       *port_list;
} tbl_mstp_port_master_t;

#endif /* !__TBL_MSTP_PORT_DEFINE_H__ */

