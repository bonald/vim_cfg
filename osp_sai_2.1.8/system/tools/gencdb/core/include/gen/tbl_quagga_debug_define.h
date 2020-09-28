
#ifndef __TBL_QUAGGA_DEBUG_DEFINE_H__
#define __TBL_QUAGGA_DEBUG_DEFINE_H__

/* TBL_QUAGGA_DEBUG field defines */
typedef enum
{
    TBL_QUAGGA_DEBUG_FLD_RIP_EVENT            = 0 ,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_RIP_PACKET           = 1 ,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_RIP_ZEBRA            = 2 ,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT           = 3 ,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_EVENTS      = 4 ,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_STATUS      = 5 ,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_TIMERS      = 6 ,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_FLOODING    = 7 ,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_GENERATE    = 8 ,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_INSTALL     = 9 ,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_REFRESH     = 10,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_EVENTS      = 11,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_STATUS      = 12,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_TIMERS      = 13,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_NSSA            = 14,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_DD       = 15,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_HELLO    = 16,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_ACK   = 17,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_REQUEST = 18,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_UPDATE = 19,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_INTERFACE = 20,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_REDISTRIBUTE = 21,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ABR       = 22,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_IA        = 23,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ROUTE     = 24,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_SPF       = 25,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_BGP_AS4_SEGMENT      = 26,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_BGP_EVENT            = 27,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_BGP_FILTERS          = 28,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_BGP_FSM              = 29,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_BGP_KEEPALIVES       = 30,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_IN       = 31,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_OUT      = 32,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_BGP_ZEBRA            = 33,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_IGMP_EVENT           = 34,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_IGMP_PACKET          = 35,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_IGMP_TRACE           = 36,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_MROUTE_ON            = 37,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_MROUTE_DETAIL        = 38,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_PIM_EVENT            = 39,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_PIM_PACKET           = 40,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_SEND  = 41,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_RECEIVE = 42,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_PIM_TRACE            = 43,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_PIM_ZEBRA            = 44,  /* RW */
    TBL_QUAGGA_DEBUG_FLD_MAX                  = 45
} tbl_quagga_debug_field_id_t;

/* TBL_QUAGGA_DEBUG defines */
typedef struct
{
    uint32               rip;                 /* bitmap of RIPDBG_FLAG_ */
    uint32               ospf;                /* bitmap of OSPFDBG_FLAG_ */
    uint32               bgp;                 /* bitmap of BGPDBG_FLAG_ */
    uint32               igmp;                /* bitmap of IGMPDBG_FLAG_ */
    uint32               mroute;              /* bitmap of MROUTEDBG_FLAG_ */
    uint32               pim;                 /* bitmap of PIMDBG_FLAG_ */
} tbl_quagga_debug_t;

#endif /* !__TBL_QUAGGA_DEBUG_DEFINE_H__ */

