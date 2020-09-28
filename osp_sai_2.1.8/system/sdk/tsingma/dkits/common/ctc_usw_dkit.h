#ifndef _CTC_GOLDENGATE_DKIT_H_
#define _CTC_GOLDENGATE_DKIT_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "sal.h"

#include "ctc_usw_dkit_discard.h"
#if 0
#include "ctc_usw_dkit_path.h"
#endif

 /*L2TYPE*/
#define L2TYPE_NONE                              0
#define L2TYPE_ETHV2                             1
#define L2TYPE_ETHSAP                            2
#define L2TYPE_ETHSNAP                           3

 /*L3TYPE*/
#define L3TYPE_NONE                              0x0
#define L3TYPE_IP                                0x1
#define L3TYPE_IPV4                              0x2
#define L3TYPE_IPV6                              0x3
#define L3TYPE_MPLS                              0x4
#define L3TYPE_MPLSUP                            0x5
#define L3TYPE_ARP                               0x6
#define L3TYPE_FCOE                              0x7
#define L3TYPE_TRILL                             0x8
#define L3TYPE_ETHEROAM                          0x9
#define L3TYPE_SLOWPROTO                         0xa
#define L3TYPE_CMAC                              0xb
#define L3TYPE_PTP                               0xc
#define L3TYPE_DOT1AE                            0xd
#define L3TYPE_SATPDU                            0xe
#define L3TYPE_FLEXIBLE                          0xf

 /*L4TYPE*/
#define L4TYPE_NONE                              0x0
#define L4TYPE_TCP                               0x1
#define L4TYPE_UDP                               0x2
#define L4TYPE_GRE                               0x3
#define L4TYPE_IPINIP                            0x4
#define L4TYPE_V6INIP                            0x5
#define L4TYPE_ICMP                              0x6
#define L4TYPE_IGMP                              0x7
#define L4TYPE_IPINV6                            0x8
#define L4TYPE_PBBITAGOAM                        0x9
#define L4TYPE_ACHOAM                            0xa
#define L4TYPE_V6INV6                            0xb
#define L4TYPE_RDP                               0xc
#define L4TYPE_SCTP                              0xd
#define L4TYPE_DCCP                              0xe
#define L4TYPE_FLEXIBLE                          0xf

 /*RXOAMTYPE*/
#define RXOAMTYPE_NONE                           0x0
#define RXOAMTYPE_ETHEROAM                       0x1
#define RXOAMTYPE_IPBFD                          0x2
#define RXOAMTYPE_PBTOAM                         0x3
#define RXOAMTYPE_PBBBSI                         0x4
#define RXOAMTYPE_PBBBV                          0x5
#define RXOAMTYPE_MPLSOAM                        0x6
#define RXOAMTYPE_MPLSBFD                        0x7
#define RXOAMTYPE_ACHOAM                         0x8
#define RXOAMTYPE_RSV                            0x9
#define RXOAMTYPE_TRILLBFD                       0xa

 /*OPERATIONTYPE*/
#define OPERATIONTYPE_NORMAL                     0
#define OPERATIONTYPE_LMTX                       1
#define OPERATIONTYPE_E2ILOOP                    2
#define OPERATIONTYPE_DMTX                       3
#define OPERATIONTYPE_C2C                        4
#define OPERATIONTYPE_PTP                        5
#define OPERATIONTYPE_NAT                        6
#define OPERATIONTYPE_OAM                        7

 /*SHARETYPE*/
#define SHARETYPE_NONE                           0
#define SHARETYPE_NAT                            1
#define SHARETYPE_LMTX                           2
#define SHARETYPE_PTP                            3
#define SHARETYPE_OAM                            4
#define SHARETYPE_DMTX                           5

 /*PAYLOADOPERATION*/
#define PAYLOADOPERATION_NONE                    0
#define PAYLOADOPERATION_ROUTE                   1
#define PAYLOADOPERATION_BRIDGE                  2
#define PAYLOADOPERATION_BRIDGE_VPLS             3
#define PAYLOADOPERATION_BRIDGE_INNER            4
#define PAYLOADOPERATION_MIRROR                  5
#define PAYLOADOPERATION_ROUTE_NOTTL             6
#define PAYLOADOPERATION_ROUTE_COMPACT           7

 /*NHPSHARETYPE*/
#define NHPSHARETYPE_L2EDIT_PLUS_L3EDIT_OP       0
#define NHPSHARETYPE_VLANTAG_OP                  1
#define NHPSHARETYPE_L2EDIT_PLUS_STAG_OP         2
#define NHPSHARETYPE_IS_SPAN_PACKET              3

 /*L2REWRITETYPE*/
#define L2REWRITETYPE_NONE                       0x0
#define L2REWRITETYPE_LOOPBACK                   0x1
#define L2REWRITETYPE_ETH4W                      0x2
#define L2REWRITETYPE_ETH8W                      0x3
#define L2REWRITETYPE_MAC_SWAP                   0x4
#define L2REWRITETYPE_L2FLEX                     0x5
#define L2REWRITETYPE_PBB4W                      0x6
#define L2REWRITETYPE_PBB8W                      0x7
#define L2REWRITETYPE_OF                         0x8
#define L2REWRITETYPE_INNER_SWAP                 0x9
#define L2REWRITETYPE_INNER_DS_LITE              0xa
#define L2REWRITETYPE_INNER_DS_LITE_8W           0xb

 /*L3REWRITETYPE*/
#define L3REWRITETYPE_NONE                       0x0
#define L3REWRITETYPE_MPLS4W                     0x1
#define L3REWRITETYPE_RESERVED                   0x2
#define L3REWRITETYPE_NAT4W                      0x3
#define L3REWRITETYPE_NAT8W                      0x4
#define L3REWRITETYPE_TUNNELV4                   0x5
#define L3REWRITETYPE_TUNNELV6                   0x6
#define L3REWRITETYPE_L3FLEX                     0x7
#define L3REWRITETYPE_OF8W                       0x8
#define L3REWRITETYPE_OF16W                      0x9
#define L3REWRITETYPE_LOOPBACK                   0xa
#define L3REWRITETYPE_TRILL                      0xb

 /*FIBDAKEYTYPE*/
#define FIBDAKEYTYPE_IPV4UCAST                   0
#define FIBDAKEYTYPE_IPV6UCAST                   1
#define FIBDAKEYTYPE_IPV4MCAST                   2
#define FIBDAKEYTYPE_IPV6MCAST                   3
#define FIBDAKEYTYPE_FCOE                        4
#define FIBDAKEYTYPE_TRILLUCAST                  5
#define FIBDAKEYTYPE_TRILLMCAST                  6
#define FIBDAKEYTYPE_RESERVED                    7

 /*FIBSAKEYTYPE*/
#define FIBSAKEYTYPE_IPV4RPF                     0
#define FIBSAKEYTYPE_IPV6RPF                     1
#define FIBSAKEYTYPE_IPV4PBR                     2
#define FIBSAKEYTYPE_IPV6PBR                     3
#define FIBSAKEYTYPE_IPV4NATSA                   4
#define FIBSAKEYTYPE_IPV6NATSA                   5
#define FIBSAKEYTYPE_FCOERPF                     6

 /*VTAGACTIONTYPE*/
#define VTAGACTIONTYPE_NONE                      0
#define VTAGACTIONTYPE_MODIFY                  1
#define VTAGACTIONTYPE_ADD                       2
#define VTAGACTIONTYPE_DELETE                    3

#define CTC_DKITS_IPV6_ADDR_STR_LEN           44          /**< IPv6 address string length */
#define CTC_DKITS_IPV6_ADDR_SORT(val)           \
    {                                          \
        uint32 t;                          \
        t = val[0];               \
        val[0] = val[3]; \
        val[3] = t;               \
                                  \
        t = val[1];               \
        val[1] = val[2]; \
        val[2] = t;               \
    }


#define CTC_DKIT_SET_USER_MAC(dest, src)     \
    {   \
        (dest)[0] = (src)[1] >> 8;          \
        (dest)[1] = (src)[1] & 0xFF;        \
        (dest)[2] = (src)[0] >> 24;         \
        (dest)[3] = ((src)[0] >> 16) & 0xFF;\
        (dest)[4] = ((src)[0] >> 8) & 0xFF; \
        (dest)[5] = (src)[0] & 0xFF;        \
    }

#define CTC_DKIT_SET_HW_MAC(dest, src)     \
    {   \
        (dest)[1]= (((src)[0] << 8)  | ((src)[1]));        \
        (dest)[0]= (((src)[2] << 24) | ((src)[3] << 16) | ((src)[4] << 8) | ((src)[5])); \
    }


#define CTC_DKIT_IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1 : 0)

/* chip name length*/
#define CTC_DKIT_CHIP_NAME_LEN              32

/*CTC port*/
#define CTC_DKIT_ONE_SLICE_CHANNEL_NUM       64
#define CTC_DKIT_ONE_SLICE_PORT_NUM          256
#define CTC_DKIT_GCHIP_LENGTH                7           /**< Gchip id length(unit: bit) */
#define CTC_DKIT_GCHIP_MASK                  0x7F        /**< Gchip id mask(unit: bit) */
#define CTC_DKIT_CTC_LPORT_LENGTH           8           /**< Local port id length(unit: bit) */
#define CTC_DKIT_EXT_PORT_LENGTH             3           /**< Local port id ext length(unit: bit) */
#define CTC_DKIT_EXT_PORT_MASK               7           /**< Local port ext bit mask */

#define CTC_DKIT_SYS_LPORT_LENGTH           9           /**< Local port id length(unit: bit) */
#define CTC_DKIT_SYS_LPORT_MASK             0x1FF        /**< Local port mask */
#define CTC_DKIT_DRV_LPORT_LENGTH           8           /**< Local port id length(unit: bit) */
#define CTC_DKIT_DRV_LPORT_MASK             0xFF        /**< Local port mask */
#define CTC_DKIT_DRV_LINKAGGID_MASK              0xFF        /**< Linkagg ID mask */
#define CTC_DKIT_LINKAGG_CHIPID              0x1F        /**< Linkagg port's global chip ID */
#define CTC_DKIT_LOCAL_PORT_LENGTH           8           /**< Local port id length(unit: bit) */
#define CTC_DKIT_LOCAL_PORT_MASK             0xFF        /**< Local port mask */

 /*DRV->CTC*/
#define CTC_DKIT_DRV_LPORT_TO_SYS_LPORT(lport) (lport)
#define CTC_DKIT_SYS_LPORT_TO_CTC_GPORT(gchip, lport)  (((((lport) >> CTC_DKIT_CTC_LPORT_LENGTH) & CTC_DKIT_EXT_PORT_MASK) \
                                              << (CTC_DKIT_CTC_LPORT_LENGTH + CTC_DKIT_GCHIP_LENGTH))           \
                                              | (((gchip) & CTC_DKIT_GCHIP_MASK) << CTC_DKIT_CTC_LPORT_LENGTH)    \
                                              | ((lport) & CTC_DKIT_DRV_LPORT_MASK))
#define CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(gchip, lport) CTC_DKIT_SYS_LPORT_TO_CTC_GPORT(gchip, CTC_DKIT_DRV_LPORT_TO_SYS_LPORT(lport))
#define CTC_DKIT_DRV_GPORT_TO_GCHIP(gport)         (((gport) >> CTC_DKIT_SYS_LPORT_LENGTH) & CTC_DKIT_GCHIP_MASK)
#define CTC_DKIT_DRV_GPORT_TO_DRV_LPORT(gport)        ((gport)& CTC_DKIT_SYS_LPORT_MASK)
#define CTC_DKIT_DRV_GPORT_IS_LINKAGG_PORT(gport)         (CTC_DKIT_DRV_GPORT_TO_GCHIP(gport) == CTC_DKIT_LINKAGG_CHIPID)
#define CTC_DKIT_DRV_GPORT_TO_LINKAGG_ID(gport) ((gport)& CTC_DKIT_DRV_LINKAGGID_MASK)
#define CTC_DKIT_DRV_GPORT_TO_CTC_GPORT(gport)   (CTC_DKIT_SYS_LPORT_TO_CTC_GPORT(CTC_DKIT_DRV_GPORT_TO_GCHIP(gport)\
        , CTC_DKIT_DRV_GPORT_IS_LINKAGG_PORT(gport)? \
        (CTC_DKIT_DRV_GPORT_TO_DRV_LPORT(gport)):(CTC_DKIT_DRV_LPORT_TO_SYS_LPORT(CTC_DKIT_DRV_GPORT_TO_DRV_LPORT(gport)))))

 /*CTC->DRV*/
#define CTC_DKIT_SYS_LPORT_TO_DRV_LPORT(lport)  (lport)
#define CTC_DKIT_CTC_GPORT_TO_SYS_LPORT(gport)         ((((gport) >> (CTC_DKIT_DRV_LPORT_LENGTH + CTC_DKIT_GCHIP_LENGTH)   \
                                               & CTC_DKIT_EXT_PORT_MASK) << CTC_DKIT_DRV_LPORT_LENGTH)          \
                                               | ((gport) & CTC_DKIT_DRV_LPORT_MASK))
#define CTC_DKIT_CTC_GPORT_TO_DRV_LPORT(gport) CTC_DKIT_SYS_LPORT_TO_DRV_LPORT(CTC_DKIT_CTC_GPORT_TO_SYS_LPORT((gport)))



#define CTC_DKIT_CTC_GPORT_TO_GCHIP(gport)         (((gport) >> CTC_DKIT_LOCAL_PORT_LENGTH) & CTC_DKIT_GCHIP_MASK)

#define CTC_DKITS_ENCODE_MCAST_IPE_DESTMAP(group) ((1<<18)|((group)&0xffff))
#define CTC_DKITS_ENCODE_DESTMAP( gchip,lport)        ((((gchip)&0x7F) << 9) | ((lport)&0x1FF))



#define CTC_DKIT_GET_GCHIP(lchip, gchip) \
        do { \
                 uint32 cmd_tmp = 0; \
                 uint8 i_tmp = 0; \
                 uint32 gport_tmp = 0; \
                 for (i_tmp = 0; i_tmp< 255; i_tmp++) \
                 { \
                     cmd_tmp = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_globalSrcPort_f); \
                     DRV_IOCTL(lchip, i_tmp, cmd_tmp, &gport_tmp); \
                     if (!CTC_DKIT_DRV_GPORT_IS_LINKAGG_PORT(gport_tmp)) {break;} \
                 } \
                 gchip =  CTC_DKIT_DRV_GPORT_TO_GCHIP(gport_tmp); \
    } while (0)


#define CTC_DKIT_PATH_PRINT(X, ...)  \
    do { \
            ctc_cli_out("    "); \
            ctc_cli_out(X, ##__VA_ARGS__);\
    } while (0)



enum ctc_dkit_module_e
{
    CTC_DKIT_IPE = 0,
    CTC_DKIT_BSR,
    CTC_DKIT_EPE,
    CTC_DKIT_MAX
} ;
typedef enum ctc_dkit_module_e ctc_dkit_module_t;

enum ctc_dkit_monitor_task_type_s
{
    CTC_DKIT_MONITOR_TASK_TEMPERATURE = 0,
    CTC_DKIT_MONITOR_TASK_PRBS = 1,
    CTC_DKIT_MONITOR_TASK_MAX
} ;
typedef enum ctc_dkit_monitor_task_type_s ctc_dkit_monitor_task_type_t;

struct ctc_dkit_monitor_task_s
{
    sal_task_t* monitor_task;
    void* para;
    uint8 task_end;
};
typedef struct ctc_dkit_monitor_task_s ctc_dkit_monitor_task_t;




#define CTC_DKIT_SERDES_HSS15G_LANE_NUM 8
#define CTC_DKIT_SERDES_HSS28G_LANE_NUM 4
#define CTC_DKIT_40 40
#define CTC_DKIT_SERDES_HSS28G_NUM_PER_SLICE (DRV_IS_DUET2(lchip) ? 4 : 2)
#define CTC_DKIT_SERDES_HSS15G_NUM_PER_SLICE 3

#define CTC_DKIT_SERDES_IS_HSS28G(serdes_id) (DRV_IS_DUET2(lchip) ? \
    (((serdes_id) >= 24) && ((serdes_id) <= 39)) : \
    (((serdes_id) >= 24) && ((serdes_id) <= 31)))

#define CTC_DKIT_MAP_SERDES_TO_HSSID(serdes_id) (DRV_IS_DUET2(lchip) ? \
    (CTC_DKIT_SERDES_IS_HSS28G(serdes_id) \
    ?((((serdes_id)% 40) - 24)/CTC_DKIT_SERDES_HSS28G_LANE_NUM + ((serdes_id)/40)*CTC_DKIT_SERDES_HSS28G_NUM_PER_SLICE) \
    :(((serdes_id)% 40)/CTC_DKIT_SERDES_HSS15G_LANE_NUM + ((serdes_id)/40)*CTC_DKIT_SERDES_HSS15G_NUM_PER_SLICE)) : \
    (CTC_DKIT_SERDES_IS_HSS28G(serdes_id) \
    ?((((serdes_id)% 32) - 24)/CTC_DKIT_SERDES_HSS28G_LANE_NUM + ((serdes_id)/32)*CTC_DKIT_SERDES_HSS28G_NUM_PER_SLICE) \
    :(((serdes_id)% 32)/CTC_DKIT_SERDES_HSS15G_LANE_NUM + ((serdes_id)/32)*CTC_DKIT_SERDES_HSS15G_LANE_NUM)))

#define CTC_DKIT_MAP_SERDES_TO_HSS_INDEX(serdes_id) (DRV_IS_DUET2(lchip) ? \
    (CTC_DKIT_SERDES_IS_HSS28G(serdes_id) \
    ?((((serdes_id)% 40) - 24)/CTC_DKIT_SERDES_HSS28G_LANE_NUM + (((serdes_id)/40)?3:3)) \
    :(((serdes_id)% 40)/CTC_DKIT_SERDES_HSS15G_LANE_NUM + ((serdes_id)/40)*7)) : \
    (CTC_DKIT_SERDES_IS_HSS28G(serdes_id) \
    ?((((serdes_id)% 32) - 24)/CTC_DKIT_SERDES_HSS28G_LANE_NUM + 3+((serdes_id)/32)*5) \
    :(((serdes_id)% 32)/CTC_DKIT_SERDES_HSS15G_LANE_NUM + ((serdes_id)/32)*5)))

#define CTC_DKIT_MAP_SERDES_TO_LANE_ID(serdes_id) (DRV_IS_DUET2(lchip) ? \
    (CTC_DKIT_SERDES_IS_HSS28G(serdes_id) \
    ?(((serdes_id% 40) - 24)%CTC_DKIT_SERDES_HSS28G_LANE_NUM) \
    :((serdes_id% 40)%CTC_DKIT_SERDES_HSS15G_LANE_NUM)) : \
    (CTC_DKIT_SERDES_IS_HSS28G(serdes_id) \
    ?(((serdes_id% 32) - 24)%CTC_DKIT_SERDES_HSS28G_LANE_NUM) \
    :((serdes_id% 32)%CTC_DKIT_SERDES_HSS15G_LANE_NUM)))

struct ctc_dkit_serdes_wr_s
{
    uint32 type;  /*ctc_dkit_serdes_mode_t*/

    uint16 serdes_id ;
    uint16 data;

    uint32 addr_offset;

    uint8 lchip;
    uint8 sub_type; /* only for TM CMU ID,0-2*/
    uint8 rsv[2];
};
typedef struct ctc_dkit_serdes_wr_s ctc_dkit_serdes_wr_t;




struct ctc_dkit_master_s
{
    void *p;
    ctc_dkit_discard_history_t discard_history;
    ctc_dkit_discard_stats_t discard_stats;
     /*-ctc_dkit_path_stats_t path_stats;*/
    ctc_dkit_monitor_task_t monitor_task[CTC_DKIT_MONITOR_TASK_MAX];
    void* discard_enable_bitmap;
};
typedef struct ctc_dkit_master_s ctc_dkit_master_t;



enum ctc_dkit_device_id_type_e
{
    CTC_DKIT_DEVICE_ID_DUET2_CTC7148,
    CTC_DKIT_DEVICE_ID_INVALID,
};
typedef enum ctc_dkit_device_id_type_e ctc_dkit_device_id_type_t;


extern int32
ctc_usw_dkit_get_chip_name(uint8 lchip, char* chip_name);

#ifdef __cplusplus
}
#endif


#endif



