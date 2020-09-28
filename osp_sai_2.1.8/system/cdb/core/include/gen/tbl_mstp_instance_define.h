
#ifndef __TBL_MSTP_INSTANCE_DEFINE_H__
#define __TBL_MSTP_INSTANCE_DEFINE_H__

#include "gen/tbl_msti_port_define.h"
/* TBL_MSTP_INSTANCE field defines */
typedef enum
{
    TBL_MSTP_INSTANCE_FLD_KEY                  = 0 ,  /* READ */
    TBL_MSTP_INSTANCE_FLD_INSTANCE_OID         = 1 ,  /* READ */
    TBL_MSTP_INSTANCE_FLD_VLAN                 = 2 ,  /* RW */
    TBL_MSTP_INSTANCE_FLD_BRIDGE_PRIORITY      = 3 ,  /* RW */
    TBL_MSTP_INSTANCE_FLD_BRIDGE_ID            = 4 ,  /* READ */
    TBL_MSTP_INSTANCE_FLD_ROOT_PORTID          = 5 ,  /* READ */
    TBL_MSTP_INSTANCE_FLD_ROOT_ROOTID          = 6 ,  /* READ */
    TBL_MSTP_INSTANCE_FLD_ROOT_ROOTPATHCOST    = 7 ,  /* READ */
    TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDBRIDGEID = 8 ,  /* READ */
    TBL_MSTP_INSTANCE_FLD_ROOT_DESIGNATEDPORTID = 9 ,  /* READ */
    TBL_MSTP_INSTANCE_FLD_ROOT_BRIDGEPORTID    = 10,  /* READ */
    TBL_MSTP_INSTANCE_FLD_ROOT_MESSAGEAGE      = 11,  /* READ */
    TBL_MSTP_INSTANCE_FLD_ROOT_FWDDELAY        = 12,  /* READ */
    TBL_MSTP_INSTANCE_FLD_ROOT_HELLOTIME       = 13,  /* READ */
    TBL_MSTP_INSTANCE_FLD_ROOT_MAXAGE          = 14,  /* READ */
    TBL_MSTP_INSTANCE_FLD_PRS_STATE            = 15,  /* READ */
    TBL_MSTP_INSTANCE_FLD_HOPCOUNT             = 16,  /* READ */
    TBL_MSTP_INSTANCE_FLD_G8032_RING_ID        = 17,  /* RW */
    TBL_MSTP_INSTANCE_FLD_PORT_LIST            = 18,  /* READ */
    TBL_MSTP_INSTANCE_FLD_MAX                  = 19
} tbl_mstp_instance_field_id_t;

/* TBL_MSTP_INSTANCE defines */
typedef struct
{
    uint32               instance;
} tbl_mstp_instance_key_t;

typedef struct
{
    tbl_mstp_instance_key_t key;
    uint64               instance_oid;        /* OID alloced by SAI */
    vlan_bmp_t           vlan;
    uint32               bridge_priority;     /* MSTI instance priority */
    mstp_brg_id_t        bridge_id;           /* 17.18.2 BridgeIdentifier */
    uint32               root_portId;         /* 17.18.5 rootPortId */
    mstp_brg_id_t        root_rootId;         /* 17.18.6/13.23.6 rootPriority - rootId */
    uint32               root_rootPathCost;   /* 17.18.6/13.23.6 rootPriority - rootPathCost */
    mstp_brg_id_t        root_designatedBridgeId; /* 17.18.6/13.23.6 rootPriority - designatedBridgeId */
    uint16               root_designatedPortId; /* 17.18.6/13.23.6 rootPriority - designatedPortId */
    uint16               root_bridgePortId;   /* 17.18.6/13.23.6 rootPriority - bridgePortId */
    uint32               root_messageAge;     /* 17.18.7 rootTimes - messageAge */
    uint32               root_fwdDelay;       /* 17.18.7 rootTimes - 17.20.6 fwdDelay */
    uint32               root_helloTime;      /* 17.18.7 rootTimes - 17.20.7 helloTime */
    uint32               root_maxAge;         /* 17.18.7 rootTimes - 17.20.8 maxAge */
    uint8                prs_state;           /* prs_state_t - Port Role Selection */
    uint8                hopCount;
    tbl_msti_port_t      *root_port;
    uint32               g8032_ring_id;
    cdb_reference_list_t port_list;           /* MSTI port list */
} tbl_mstp_instance_t;

typedef struct
{
    ctclib_slist_t       *inst_list;
} tbl_mstp_instance_master_t;

#endif /* !__TBL_MSTP_INSTANCE_DEFINE_H__ */

