
#ifndef __TBL_SWITCH_DEBUG_DEFINE_H__
#define __TBL_SWITCH_DEBUG_DEFINE_H__

/* TBL_SWITCH_DEBUG field defines */
typedef enum
{
    TBL_SWITCH_DEBUG_FLD_STP_EVENT            = 0 ,  /* RW */
    TBL_SWITCH_DEBUG_FLD_STP_PACKET_TX        = 1 ,  /* RW */
    TBL_SWITCH_DEBUG_FLD_STP_PACKET_RX        = 2 ,  /* RW */
    TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_CIST    = 3 ,  /* RW */
    TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_MSTI    = 4 ,  /* RW */
    TBL_SWITCH_DEBUG_FLD_STP_TIMER            = 5 ,  /* RW */
    TBL_SWITCH_DEBUG_FLD_MLAG_EVENT           = 6 ,  /* RW */
    TBL_SWITCH_DEBUG_FLD_MLAG_PACKET          = 7 ,  /* RW */
    TBL_SWITCH_DEBUG_FLD_MLAG_PROTOCOL        = 8 ,  /* RW */
    TBL_SWITCH_DEBUG_FLD_MLAG_TIMER           = 9 ,  /* RW */
    TBL_SWITCH_DEBUG_FLD_LACP_EVENT           = 10,  /* RW */
    TBL_SWITCH_DEBUG_FLD_LACP_PACKET          = 11,  /* RW */
    TBL_SWITCH_DEBUG_FLD_LACP_PROTOCOL        = 12,  /* RW */
    TBL_SWITCH_DEBUG_FLD_LACP_TIMER           = 13,  /* RW */
    TBL_SWITCH_DEBUG_FLD_IGSP_EVENT           = 14,  /* RW */
    TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_TX       = 15,  /* RW */
    TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_RX       = 16,  /* RW */
    TBL_SWITCH_DEBUG_FLD_IGSP_PROTOCOL        = 17,  /* RW */
    TBL_SWITCH_DEBUG_FLD_IGSP_TIMER           = 18,  /* RW */
    TBL_SWITCH_DEBUG_FLD_IPSG_EVENT           = 19,  /* RW */
    TBL_SWITCH_DEBUG_FLD_IPSG_ERROR           = 20,  /* RW */
    TBL_SWITCH_DEBUG_FLD_LLDP_EVENT           = 21,  /* RW */
    TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_TX       = 22,  /* RW */
    TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_RX       = 23,  /* RW */
    TBL_SWITCH_DEBUG_FLD_LLDP_PROTOCOL        = 24,  /* RW */
    TBL_SWITCH_DEBUG_FLD_LLDP_TIMER           = 25,  /* RW */
    TBL_SWITCH_DEBUG_FLD_RPCAPI_REQUEST       = 26,  /* RW */
    TBL_SWITCH_DEBUG_FLD_RPCAPI_COMMAND       = 27,  /* RW */
    TBL_SWITCH_DEBUG_FLD_MAX                  = 28
} tbl_switch_debug_field_id_t;

/* TBL_SWITCH_DEBUG defines */
typedef struct
{
    uint32               stp;                 /* bitmap of SWTHDBG_FLAG_STP_ */
    uint32               mlag;                /* bitmap of SWTHDBG_FLAG_MLAG_ */
    uint32               lacp;                /* bitmap of SWTHDBG_FLAG_LACP_ */
    uint32               igsp;                /* bitmap of SWTHDBG_FLAG_IGSP_ */
    uint32               ipsg;                /* bitmap of SWTHDBG_FLAG_IPSG_ */
    uint32               lldp;                /* bitmap of SWTHDBG_FLAG_LLDP_ */
    uint32               rpcapi;              /* bitmap of SWTHDBG_FLAG_RPCAPI_ */
} tbl_switch_debug_t;

#endif /* !__TBL_SWITCH_DEBUG_DEFINE_H__ */

