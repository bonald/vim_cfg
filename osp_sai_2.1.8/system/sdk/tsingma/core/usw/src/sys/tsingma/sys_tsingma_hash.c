/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_macro.h"
#include "ctc_register.h"
#include "sys_usw_parser.h"
#include "sys_usw_common.h"
#include "drv_api.h"

#define SYS_LB_HASH_SEL_STR \
"l2",\
"ipv4",\
"ipv4-tcp-udp",\
"ipv4-tcp-udp-ports-equal",\
"ipv4-vxlan",\
"ipv4-gre",\
"ipv4-nvgre",\
"ipv6",\
"ipv6-tcp-udp",\
"ipv6-tcp-udp-ports-equal",\
"ipv6-vxlan",\
"ipv6-gre",\
"ipv6-nvgre",\
"nvgre-inner-l2",\
"nvgre-inner-ipv4",\
"nvgre-inner-ipv6",\
"vxlan-inner-l2",\
"vxlan-inner-ipv4",\
"vxlan-inner-ipv6",\
"mpls",\
"mpls-inner-ipv4",\
"mpls-inner-ipv6",\
"mpls-vpws-raw",\
"mpls-l2vpn-inner-l2",\
"mpls-l2vpn-inner-ipv4",\
"mpls-l2vpn-inner-ipv6",\
"mpls-l3vpn-inner-ipv4",\
"mpls-l3vpn-inner-ipv6",\
"trill-outer-l2-only",\
"trill",\
"trill-inner-l2",\
"trill-inner-ipv4",\
"trill-inner-ipv6",\
"trill-decap-inner-l2",\
"trill-decap-inner-ipv4",\
"trill-decap-inner-ipv6",\
"fcoe",\
"flex-tnl-inner-l2",\
"flex-tnl-inner-ipv4",\
"flex-tnl-inner-ipv6",\
"udf",\
"max"

#define SYS_LB_HASH_CTL_STR \
"l2-only",\
"disable-ipv4",\
"disable-ipv6",\
"disable-mpls",\
"disable-fcoe",\
"disable-trill",\
"disable-tunnel-ipv4-in4",\
"disable-tunnel-ipv6-in4",\
"disable-tunnel-gre-in4",\
"disable-tunnel-nvgre-in4",\
"disable-tunnel-vxlan-in4",\
"disable-tunnel-ipv4-in6",\
"disable-tunnel-ipv6-in6",\
"disable-tunnel-gre-in6",\
"disable-tunnel-nvgre-in6",\
"disable-tunnel-vxlan-in6",\
"mpls-exclude-rsv",\
"ipv6-addr-collapse",\
"ipv6-use-flow-label",\
"nvgre-aware",\
"fcoe-symmetric-en",\
"udf-select-mode",\
"udf-data-byte-mask",\
"use-mapped-vlan",\
"ipv4-symmetric-en",\
"ipv6-symmetric-en",\
"l2vpn-use-inner-l2",\
"vpws-use-inner",\
"trill-decap-use-inner-l2",\
"trill-transmit-mode",\
"mpls-entropy-en",\
"mpls-label2-en",\
"vxlan-use-inner-l2",\
"nvgre-use-inner-l2",\
"flex-tnl-use-inner-l2",\
"hash-seed",\
"hash-type-a",\
"hash-type-b",\
"max"

#define SYS_LB_HASH_DBG_OUT(level, FMT, ...)\
    do\
    {\
        CTC_DEBUG_OUT(linkagg, linkagg, LINKAGG_SYS, level, FMT, ##__VA_ARGS__);\
    } while (0)

enum sys_hash_poly_e
    {
        SYS_LB_HASHPOLY_CRC8_POLY1,
        SYS_LB_HASHPOLY_CRC8_POLY2,
        SYS_LB_HASHPOLY_XOR8,
        SYS_LB_HASHPOLY_CRC10_POLY1,
        SYS_LB_HASHPOLY_CRC10_POLY2,
        SYS_LB_HASHPOLY_XOR16,
        SYS_LB_HASHPOLY_CRC16_POLY1,
        SYS_LB_HASHPOLY_CRC16_POLY2,
        SYS_LB_HASHPOLY_CRC16_POLY3,
        SYS_LB_HASHPOLY_CRC16_POLY4,
        SYS_LB_HASHPOLY_CRC9_POLY1,
        SYS_LB_HASHPOLY_CRC9_POLY2,
        SYS_LB_HASHPOLY_CRC9_POLY3,
        SYS_LB_HASHPOLY_CRC9_POLY4,
        SYS_LB_HASHPOLY_CRC8_POLY3,
        SYS_LB_HASHPOLY_CRC8_POLY4,
        SYS_LB_HASHPOLY_POLY_MAX
    };
    typedef enum sys_hash_poly_e sys_hash_poly_t;

#define SYS_LB_HASH_MPLS_SET_INNER_MAC(value, hash_ctl)       \
do                                                            \
{                                                             \
    if (CTC_FLAG_ISSET(value,CTC_LB_HASH_FIELD_MACDA_LO) &&   \
        CTC_FLAG_ISSET(value,CTC_LB_HASH_FIELD_MACDA_MI) &&   \
        CTC_FLAG_ISSET(value,CTC_LB_HASH_FIELD_MACDA_HI))     \
    {                                                         \
        SetParserMplsCtl(V,mplsMacDaHashEn_f,&hash_ctl,1);    \
    }                                                         \
    else                                                      \
    {                                                         \
        SetParserMplsCtl(V,mplsMacDaHashEn_f,&hash_ctl,0);    \
    }                                                         \
    if (CTC_FLAG_ISSET(value,CTC_LB_HASH_FIELD_MACSA_LO) &&   \
        CTC_FLAG_ISSET(value,CTC_LB_HASH_FIELD_MACSA_MI) &&   \
        CTC_FLAG_ISSET(value,CTC_LB_HASH_FIELD_MACSA_HI))     \
    {                                                         \
        SetParserMplsCtl(V,mplsMacSaHashEn_f,&hash_ctl,1);    \
    }                                                         \
    else                                                      \
    {                                                         \
        SetParserMplsCtl(V,mplsMacSaHashEn_f,&hash_ctl,0);    \
    }                                                         \
}while(0)

#define SYS_LB_HASH_MPLS_GET_INNER_MAC(flag,value,hash_ctl) \
do\
{\
    uint32 lable_en=0;\
    uint32 inner_macda_en=0;\
    uint32 inner_macsa_en=0;\
    lable_en = GetIpeHashCtl0(V, hashTemplate3Bulk13IsMplsLabel_f,&hash_ctl);\
    cmd = DRV_IOR(ParserMplsCtl_t, ParserMplsCtl_mplsMacDaHashEn_f);\
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd,&inner_macda_en));\
    cmd = DRV_IOR(ParserMplsCtl_t, ParserMplsCtl_mplsMacSaHashEn_f);\
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd,&inner_macsa_en));\
    if(CTC_FLAG_ISSET(flag, 0x2000))\
    {\
        if(lable_en)\
        {\
            CTC_SET_FLAG(value,CTC_LB_HASH_FIELD_LABEL2_HI);\
        }\
        else\
        {\
            CTC_UNSET_FLAG(value,CTC_LB_HASH_FIELD_LABEL2_HI);\
            if(inner_macsa_en)\
            {\
                CTC_SET_FLAG(value,  CTC_LB_HASH_FIELD_MACSA_LO);\
                CTC_SET_FLAG(value,  CTC_LB_HASH_FIELD_MACSA_MI);\
                CTC_SET_FLAG(value,  CTC_LB_HASH_FIELD_MACSA_HI);\
            }\
            if(inner_macda_en)\
            {\
                CTC_SET_FLAG(value,  CTC_LB_HASH_FIELD_MACDA_LO);\
                CTC_SET_FLAG(value,  CTC_LB_HASH_FIELD_MACDA_MI);\
                CTC_SET_FLAG(value,  CTC_LB_HASH_FIELD_MACDA_HI);\
            }\
        }\
    }\
}while(0)

#define SYS_LB_HASH_CTL_NUM 4
#define SYS_LB_HASH_SCL_PROFILE_NUM 8
#define HASH_KEY_TEMPLATE0(flag, value, mode)\
do{\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_PORT))\
    {\
        CTC_BIT_SET(flag, 0);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID))\
    {\
        CTC_BIT_SET(flag, 1);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_PORT))\
    {\
        CTC_BIT_SET(flag, 2);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_CHIP_ID))\
    {\
        CTC_BIT_SET(flag, 3);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACSA_LO))\
    {\
        CTC_BIT_SET(flag, 4);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACSA_MI))\
    {\
        CTC_BIT_SET(flag, 5);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACSA_HI))\
    {\
        CTC_BIT_SET(flag, 6);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACDA_LO))\
    {\
        CTC_BIT_SET(flag, 7);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACDA_MI))\
    {\
        CTC_BIT_SET(flag, 8);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACDA_HI))\
    {\
        CTC_BIT_SET(flag, 9);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_ETHER_TYPE))\
    {\
        CTC_BIT_SET(flag, 10);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_VNI_LO) && 1 == mode)\
    {\
        CTC_BIT_SET(flag, 11);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SVLAN) && 0 == mode)\
    {\
        CTC_BIT_SET(flag, 11);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_VNI_HI)&& 1 == mode)\
    {\
        CTC_BIT_SET(flag, 12);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_CVLAN)&& 0 == mode)\
    {\
        CTC_BIT_SET(flag, 12);\
    }\
}while(0);\

#define GET_HASH_KEY_TEMPLATE0(flag, value, mode)\
do{\
    if (CTC_FLAG_ISSET(flag, 0x1))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x4))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x8))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x10))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACSA_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x20))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACSA_MI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x40))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACSA_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x80))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACDA_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x100))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACDA_MI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x200))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACDA_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x400))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_ETHER_TYPE);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x800) && 1 == mode)\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_VNI_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x800) && 0 == mode)\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SVLAN);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x1000) && 1 == mode)\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_VNI_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x1000) && 0 == mode)\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_CVLAN);\
    }\
}while(0);\

#define HASH_KEY_TEMPLATE1(flag, value)\
do{\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_PORT))\
    {\
        CTC_BIT_SET(flag, 0);\
    }\
	if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID))\
    {\
        CTC_BIT_SET(flag, 1);\
    }\
	if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_PORT))\
    {\
        CTC_BIT_SET(flag, 2);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_CHIP_ID))\
    {\
        CTC_BIT_SET(flag, 3);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACSA_LO))\
    {\
        CTC_BIT_SET(flag, 4);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACSA_MI))\
    {\
        CTC_BIT_SET(flag, 5);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACSA_HI))\
    {\
        CTC_BIT_SET(flag, 6);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACDA_LO))\
    {\
        CTC_BIT_SET(flag, 7);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACDA_MI))\
    {\
        CTC_BIT_SET(flag, 8);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACDA_HI))\
    {\
        CTC_BIT_SET(flag, 9);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_ETHER_TYPE))\
    {\
        CTC_BIT_SET(flag, 10);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SVLAN))\
    {\
        CTC_BIT_SET(flag, 11);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_INGRESS_NICKNAME))\
    {\
        CTC_BIT_SET(flag, 12);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_EGRESS_NICKNAME))\
    {\
        CTC_BIT_SET(flag, 13);\
    }\
}while(0);\

#define GET_HASH_KEY_TEMPLATE1(flag, value)\
do{\
    if (CTC_FLAG_ISSET(flag, 0x1))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x4))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x8))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x10))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACSA_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x20))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACSA_MI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x40))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACSA_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x80))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACDA_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x100))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACDA_MI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x200))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_MACDA_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x400))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_ETHER_TYPE);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x800))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SVLAN);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x1000))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_INGRESS_NICKNAME);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2000))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_EGRESS_NICKNAME);\
    }\
}while(0);\



#define HASH_KEY_TEMPLATE2(flag, value)\
do{\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_PORT))\
    {\
        CTC_BIT_SET(flag, 0);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID))\
    {\
        CTC_BIT_SET(flag, 1);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_PORT))\
    {\
        CTC_BIT_SET(flag, 2);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_CHIP_ID))\
    {\
        CTC_BIT_SET(flag, 3);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_FCOE_SID_LO))\
    {\
        CTC_BIT_SET(flag, 4);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_FCOE_SID_HI))\
    {\
        CTC_BIT_SET(flag, 5);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_FCOE_DID_LO))\
    {\
        CTC_BIT_SET(flag, 6);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_FCOE_DID_HI))\
    {\
        CTC_BIT_SET(flag, 7);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_UDF_LO))\
    {\
        CTC_BIT_SET(flag, 8);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_UDF_HI))\
    {\
        CTC_BIT_SET(flag, 9);\
    }\
}while (0);\

#define GET_HASH_KEY_TEMPLATE2(flag,value )\
do{\
    if (CTC_FLAG_ISSET(flag, 0x1))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x4))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x8))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x10))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_FCOE_SID_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x20))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_FCOE_SID_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x40))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_FCOE_DID_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x80))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_FCOE_DID_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x100))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_UDF_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x200))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_UDF_HI);\
    }\
}while(0);\

#define HASH_KEY_TEMPLATE3(flag, value) \
	do {\
        if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_PORT))\
        {\
            CTC_BIT_SET(flag, 0);\
        }\
        if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID))\
        {\
            CTC_BIT_SET(flag, 1);\
        }\
        if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_PORT))\
        {\
            CTC_BIT_SET(flag, 2);\
        }\
        if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_CHIP_ID))\
        {\
            CTC_BIT_SET(flag, 3);\
        }\
	    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_LABEL0_LO))\
	    {\
	        CTC_BIT_SET(flag, 4);\
	    }\
	    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_LABEL1_LO))\
	    {\
	        CTC_BIT_SET(flag, 5);\
	    }\
	    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_LABEL2_LO))\
	    {\
	        CTC_BIT_SET(flag, 6);\
	    }\
	    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_LABEL0_HI))\
	    {\
	        CTC_BIT_SET(flag, 7);\
	    }\
	    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_SA_LO))\
	    {\
	        CTC_BIT_SET(flag, 8);\
	    }\
	    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_SA_HI))\
	    {\
	        CTC_BIT_SET(flag, 9);\
	    }\
	    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_DA_LO))\
	    {\
	        CTC_BIT_SET(flag, 10);\
	    }\
	    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_DA_HI))\
	    {\
	        CTC_BIT_SET(flag, 11);\
	    }\
	    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_LABEL1_HI))\
	    {\
	        CTC_BIT_SET(flag, 12);\
	    }\
	    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_LABEL2_HI))\
	    {\
	        CTC_BIT_SET(flag, 13);\
	    }\
	    if ((CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACDA_LO) && CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACDA_MI) && CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACDA_HI))\
            || (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACSA_LO) && CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACSA_MI) && CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_MACSA_HI)))\
        {\
            CTC_BIT_SET(flag, 13);\
        }\
	}while (0);\

#define GET_HASH_KEY_TEMPLATE3(flag, value)\
do{\
    if (CTC_FLAG_ISSET(flag, 0x1))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x4))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x8))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x10))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_LABEL0_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x20))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_LABEL1_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x40))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_LABEL2_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x80))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_LABEL0_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x100))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_SA_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x200))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_SA_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x400))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_DA_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x800))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_DA_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x1000))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_LABEL1_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2000))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_LABEL2_HI);\
    }\
}while(0);\

#define HASH_KEY_TEMPLATE4(flag, value, mode)\
do{\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_PORT))\
    {\
        CTC_BIT_SET(flag, 0);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID))\
    {\
        CTC_BIT_SET(flag, 1);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_PORT))\
    {\
        CTC_BIT_SET(flag, 2);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_CHIP_ID))\
    {\
        CTC_BIT_SET(flag, 3);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_SA_LO))\
    {\
        CTC_BIT_SET(flag, 4);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_SA_HI))\
    {\
        CTC_BIT_SET(flag, 5);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_DA_LO))\
    {\
        CTC_BIT_SET(flag, 6);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_DA_HI))\
    {\
        CTC_BIT_SET(flag, 7);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_PROTOCOL))\
    {\
        CTC_BIT_SET(flag, 8);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_L4_SRCPORT) && !CTC_IS_BIT_SET(mode, 3))\
    {\
        CTC_BIT_SET(flag, 9);\
    }\
	if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_VNI_LO) && CTC_IS_BIT_SET(mode, 3))\
    {\
        CTC_BIT_SET(flag, 9);\
    }\
	if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_L4_DSTPORT)&& !CTC_IS_BIT_SET(mode, 3))\
    {\
        CTC_BIT_SET(flag, 10);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_VNI_HI) &&  CTC_IS_BIT_SET(mode, 3))\
    {\
        CTC_BIT_SET(flag, 10);\
    }\
	if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SVLAN))\
    {\
        CTC_BIT_SET(flag, 11);\
    }\
	if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_VNI_LO) && 1 == (mode&0x3))\
    {\
        CTC_BIT_SET(flag, 12);\
    }\
	if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_UDF_LO) && 0 == (mode&0x3))\
    {\
        CTC_BIT_SET(flag, 12);\
    }\
	if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_FLOWLABEL_LO) && 2 == (mode&0x3))\
    {\
        CTC_BIT_SET(flag, 12);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_VNI_HI) && 1 == (mode&0x3))\
    {\
        CTC_BIT_SET(flag, 13);\
    }\
	if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_UDF_HI) && 0 == (mode&0x3))\
    {\
        CTC_BIT_SET(flag, 13);\
    }\
	if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_FLOWLABEL_HI) && 2 == (mode&0x3))\
    {\
        CTC_BIT_SET(flag, 13);\
    }\
}\
while(0);

#define GET_HASH_KEY_TEMPLATE4(flag, value, mode)\
do{\
    if (CTC_FLAG_ISSET(flag, 0x1))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x4))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x8))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x10))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_SA_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x20))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_SA_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x40))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_DA_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x80))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_DA_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x100))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_PROTOCOL);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x200)&& !CTC_IS_BIT_SET(mode, 3))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_L4_SRCPORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x200)&& CTC_IS_BIT_SET(mode, 3))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_VNI_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x400)&& !CTC_IS_BIT_SET(mode, 3))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_L4_DSTPORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x400) && CTC_IS_BIT_SET(mode, 3))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_VNI_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x800))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SVLAN);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x1000) && 1 == (mode&0x3))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_VNI_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x1000) && 0 == (mode&0x3))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_UDF_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x1000) && 2 == (mode&0x3))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_FLOWLABEL_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2000) && 1 == (mode&0x3))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_VNI_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2000) && 0 == (mode&0x3))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_UDF_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2000) && 2 == (mode&0x3))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_FLOWLABEL_HI);\
    }\
}while(0);\

#define HASH_KEY_TEMPLATE5(flag, value, mode)\
do{\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_PORT))\
    {\
        CTC_BIT_SET(flag, 0);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID))\
    {\
        CTC_BIT_SET(flag, 1);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_PORT))\
    {\
        CTC_BIT_SET(flag, 2);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_DST_CHIP_ID))\
    {\
        CTC_BIT_SET(flag, 3);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_SA_LO))\
    {\
        CTC_BIT_SET(flag, 4);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_SA_HI))\
    {\
        CTC_BIT_SET(flag, 5);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_DA_LO))\
    {\
        CTC_BIT_SET(flag, 6);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_IP_DA_HI))\
    {\
        CTC_BIT_SET(flag, 7);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_GRE_PROTOCOL))\
    {\
        CTC_BIT_SET(flag, 8);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_VNI_LO) && 1 == (mode&0x1))\
    {\
        CTC_BIT_SET(flag, 9);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_GRE_KEY_LO) && 0 == (mode&0x1))\
    {\
        CTC_BIT_SET(flag, 9);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_VNI_HI) && 1 == (mode&0x1))\
    {\
        CTC_BIT_SET(flag, 10);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_GRE_KEY_HI) && 0 == (mode&0x1))\
    {\
        CTC_BIT_SET(flag, 10);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_SVLAN))\
    {\
        CTC_BIT_SET(flag, 11);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_UDF_LO)&& 0 == (mode&0x2))\
    {\
        CTC_BIT_SET(flag, 12);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_FLOWLABEL_LO)&& 0x2 == (mode&0x2))\
    {\
        CTC_BIT_SET(flag, 12);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_UDF_HI)&& 0 == (mode&0x2))\
    {\
        CTC_BIT_SET(flag, 13);\
    }\
    if (CTC_FLAG_ISSET(value, CTC_LB_HASH_FIELD_FLOWLABEL_HI)&& 0x2 == (mode&0x2))\
    {\
        CTC_BIT_SET(flag, 13);\
    }\
}while (0);\

#define GET_HASH_KEY_TEMPLATE5(flag, value, mode)\
do{\
    if (CTC_FLAG_ISSET(flag, 0x1))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SRC_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x4))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_PORT);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x8))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_DST_CHIP_ID);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x10))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_SA_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x20))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_SA_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x40))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_DA_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x80))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_IP_DA_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x100))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_GRE_PROTOCOL);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x200) && 1 == (mode&0x1))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_VNI_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x200) && 0 == (mode&0x1))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_GRE_KEY_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x400) && 1 == (mode&0x1))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_VNI_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x400) && 0 == (mode&0x1))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_GRE_KEY_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x800))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_SVLAN);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x1000) && 0 == (mode&0x2))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_UDF_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x1000) && 0x2 == (mode&0x2))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_FLOWLABEL_LO);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2000) && 0 == (mode&0x2))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_UDF_HI);\
    }\
    if (CTC_FLAG_ISSET(flag, 0x2000) && 0x2 == (mode&0x2))\
    {\
        CTC_SET_FLAG(value, CTC_LB_HASH_FIELD_FLOWLABEL_HI);\
    }\
}while(0);\


STATIC int32
_sys_tsingma_hash_config_set_udf_select(uint8 lchip, ctc_lb_hash_config_t* p_hash_cfg)
{
    IpeSclFlowHashBulkBitmapCtl_m hash_ctl;
    uint32 cmd = 0;
	uint8 step = 0;


	sal_memset(&hash_ctl, 0, sizeof(hash_ctl));
    cmd = DRV_IOR(IpeSclFlowHashBulkBitmapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    step = (IpeSclFlowHashBulkBitmapCtl_array_1_userHashSelector0BulkBitmap_f -
		    IpeSclFlowHashBulkBitmapCtl_array_0_userHashSelector0BulkBitmap_f)*p_hash_cfg->sel_id;

    SetIpeSclFlowHashBulkBitmapCtl(V, array_0_userHashSelector0BulkMaskValid_f + step, &hash_ctl, 1);
    SetIpeSclFlowHashBulkBitmapCtl(V, array_0_userHashSelector0ForceUdfKey_f + step, &hash_ctl, 1);
    SetIpeSclFlowHashBulkBitmapCtl(V, array_0_userHashSelector0BulkBitmap_f + step, &hash_ctl, p_hash_cfg->value);

    cmd = DRV_IOW(IpeSclFlowHashBulkBitmapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_hash_config_get_udf_select(uint8 lchip, ctc_lb_hash_config_t* p_hash_cfg)
{
    IpeSclFlowHashBulkBitmapCtl_m hash_ctl;
    uint32 cmd = 0;
	uint8 step = 0;

	sal_memset(&hash_ctl, 0, sizeof(hash_ctl));
    cmd = DRV_IOR(IpeSclFlowHashBulkBitmapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    step = (IpeSclFlowHashBulkBitmapCtl_array_1_userHashSelector0BulkBitmap_f -
		    IpeSclFlowHashBulkBitmapCtl_array_0_userHashSelector0BulkBitmap_f)*p_hash_cfg->sel_id;

    p_hash_cfg->value = GetIpeSclFlowHashBulkBitmapCtl(V, array_0_userHashSelector0BulkBitmap_f + step, &hash_ctl);

    return CTC_E_NONE;
}



STATIC int32
_sys_tsingma_hash_config_set_hash_select(uint8 lchip, ctc_lb_hash_config_t* p_hash_cfg)
{
    IpeHashCtl0_m hash_ctl;
    ParserMplsCtl_m parser_mpls_ctl;
    uint32 cmd = 0;
	uint32 flag = 0;
	uint8 mode = 0;
	uint8 flow_label_en = 0;


	sal_memset(&hash_ctl, 0, sizeof(hash_ctl));
    cmd = DRV_IOR(IpeHashCtl0_t + p_hash_cfg->sel_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    sal_memset(&parser_mpls_ctl, 0, sizeof(parser_mpls_ctl));
    cmd = DRV_IOR(ParserMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_mpls_ctl));

    flow_label_en = GetIpeHashCtl0(V, ipv4PayloadIsV6UseFlowLabel_f, &hash_ctl);


    switch (p_hash_cfg->hash_select)
    {
            /*L2 header hash */
        case CTC_LB_HASH_SELECT_L2:
			HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, outerL2BulkBitmap_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, ipv4InnerL2BulkBitmap_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, ipv6InnerL2BulkBitmap_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, defaultBulkBitmap_f, &hash_ctl, flag);

            break;

            /*ipv4 header hash */
        case CTC_LB_HASH_SELECT_IPV4:
            HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, ipv4InnerL3BulkBitmap0_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, ipv4OuterL3BulkBitmap_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, ipv6InnerL3BulkBitmap0_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_IPV4_TCP_UDP:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, ipv4TcpUdpBulkBitmap1_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, ipv6TcpUdpBulkBitmap3_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_IPV4_TCP_UDP_PORTS_EQUAL:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, ipv4TcpUdpBulkBitmap0_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, ipv6TcpUdpBulkBitmap2_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_IPV4_VXLAN:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 1);
            SetIpeHashCtl0(V, ipv4VxlanBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_IPV4_GRE:
			HASH_KEY_TEMPLATE5(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, ipv4GreBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_IPV4_NVGRE:
			HASH_KEY_TEMPLATE5(flag, p_hash_cfg->value, 1);
            SetIpeHashCtl0(V, ipv4NvgreBulkBitmap_f, &hash_ctl, flag);
            break;

            /*ipv6 header hash */
        case CTC_LB_HASH_SELECT_IPV6:
			mode = flow_label_en ? 2:0;
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            SetIpeHashCtl0(V, ipv4InnerL3BulkBitmap1_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, ipv6InnerL3BulkBitmap1_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, ipv6OuterL3BulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_IPV6_TCP_UDP:
		    mode = flow_label_en ? 2:0;
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            SetIpeHashCtl0(V, ipv4TcpUdpBulkBitmap3_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, ipv6TcpUdpBulkBitmap1_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_IPV6_TCP_UDP_PORTS_EQUAL:
			mode = flow_label_en ? 2:0;
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            SetIpeHashCtl0(V, ipv4TcpUdpBulkBitmap2_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, ipv6TcpUdpBulkBitmap0_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_IPV6_VXLAN:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 1);
            SetIpeHashCtl0(V, ipv6VxlanBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_IPV6_GRE:
		    mode = flow_label_en ? 2:0;
			HASH_KEY_TEMPLATE5(flag, p_hash_cfg->value, mode);
            SetIpeHashCtl0(V, ipv6GreBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_IPV6_NVGRE:
			mode = flow_label_en ? 3:1;
			HASH_KEY_TEMPLATE5(flag, p_hash_cfg->value, mode);
            SetIpeHashCtl0(V, ipv6NvgreBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_NVGRE_INNER_L2:
			HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 1);
            SetIpeHashCtl0(V, nvgreInnerL2BulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_NVGRE_INNER_IPV4:
			CTC_BIT_SET(mode, 3);
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            SetIpeHashCtl0(V, nvgreInnerL3BulkBitmap0_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_NVGRE_INNER_IPV6:
		    mode = flow_label_en ? 2:0;
		    CTC_BIT_SET(mode, 3);
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            SetIpeHashCtl0(V, nvgreInnerL3BulkBitmap1_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_VXLAN_INNER_L2:
			HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 1);
            SetIpeHashCtl0(V, vxlanInnerL2BulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_VXLAN_INNER_IPV4:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 1);
            SetIpeHashCtl0(V, vxlanInnerL3BulkBitmap0_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_VXLAN_INNER_IPV6:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 1);
            SetIpeHashCtl0(V, vxlanInnerL3BulkBitmap1_f, &hash_ctl, flag);
            break;

            /*mpls header hash */
        case CTC_LB_HASH_SELECT_MPLS:
			HASH_KEY_TEMPLATE3(flag, p_hash_cfg->value);
            SYS_LB_HASH_MPLS_SET_INNER_MAC(p_hash_cfg->value, parser_mpls_ctl);  
            SetIpeHashCtl0(V, mplsTransitInnerL3IpBulkBitmap_f, &hash_ctl, flag);
            SetIpeHashCtl0(V, mplsTransitInnerOthersBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_MPLS_INNER_IPV4:
			HASH_KEY_TEMPLATE3(flag, p_hash_cfg->value);
            SYS_LB_HASH_MPLS_SET_INNER_MAC(p_hash_cfg->value,parser_mpls_ctl); 
            SetIpeHashCtl0(V, mplsTransitInnerL3Ipv4BulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_MPLS_INNER_IPV6:
		    HASH_KEY_TEMPLATE3(flag, p_hash_cfg->value);
            SYS_LB_HASH_MPLS_SET_INNER_MAC(p_hash_cfg->value,parser_mpls_ctl); 
            SetIpeHashCtl0(V, mplsTransitInnerL3Ipv6BulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_MPLS_VPWS_RAW:
			HASH_KEY_TEMPLATE3(flag, p_hash_cfg->value);
            SYS_LB_HASH_MPLS_SET_INNER_MAC(p_hash_cfg->value,parser_mpls_ctl); 
            SetIpeHashCtl0(V, mplsL2VpnDecapsBypassInnerParserBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_L2:
			HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, mplsL2VpnDecapsInnerL2BulkBitmap_f, &hash_ctl, flag);
			break;
        case CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV4:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, mplsL2VpnDecapsInnerL3Ipv4BulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV6:
			mode = flow_label_en ? 2:0;
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            SetIpeHashCtl0(V, mplsL2VpnDecapsInnerL3Ipv6BulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV4:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, mplsL3VpnDecapsInnerL3Ipv4BulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV6:
			mode = flow_label_en ? 2:0;
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            SetIpeHashCtl0(V, mplsL3VpnDecapsInnerL3Ipv6BulkBitmap_f, &hash_ctl, flag);
            break;

            /*trill header hash */
        case CTC_LB_HASH_SELECT_TRILL_OUTER_L2_ONLY:
			HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, trillTransitOuterL2HashBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_TRILL:
			HASH_KEY_TEMPLATE1(flag, p_hash_cfg->value);
            SetIpeHashCtl0(V, trillTransitTunnelHashBulkBitmap1_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_TRILL_INNER_L2:
			HASH_KEY_TEMPLATE1(flag, p_hash_cfg->value);
            SetIpeHashCtl0(V, trillTransitTunnelHashBulkBitmap0_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_TRILL_INNER_IPV4:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, trillTransitInnerL3Ipv4HashBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_TRILL_INNER_IPV6:
			mode = flow_label_en ? 2:0;
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            SetIpeHashCtl0(V, trillTransitInnerL3Ipv6HashBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_L2:
			HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, trillTerminateInnerL2HashBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_IPV4:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, trillTerminateInnerL3Ipv4HashBulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_IPV6:
			mode = flow_label_en ? 2:0;
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            SetIpeHashCtl0(V, trillTerminateInnerL3Ipv6HashBulkBitmap_f, &hash_ctl, flag);
            break;

            /*fcoe heaer hash */
        case CTC_LB_HASH_SELECT_FCOE:
			HASH_KEY_TEMPLATE2(flag, p_hash_cfg->value);
            SetIpeHashCtl0(V, fcoeBulkBitmap_f, &hash_ctl, flag);
            break;


            /*flex tunnel heaer hash */
        case CTC_LB_HASH_SELECT_FLEX_TNL_INNER_L2:
			HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, unknownInnerL2BulkBitmap_f, &hash_ctl, flag);
            break;
        case CTC_LB_HASH_SELECT_FLEX_TNL_INNER_IPV4:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, unknownInnerIpBulkBitmap_f, &hash_ctl, flag);
            break;

        case CTC_LB_HASH_SELECT_FLEX_TNL_INNER_IPV6:
			HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            SetIpeHashCtl0(V, unknownInnerIpv6BulkBitmap_f, &hash_ctl, flag);
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOW(IpeHashCtl0_t + p_hash_cfg->sel_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    cmd = DRV_IOW(ParserMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_mpls_ctl));

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_hash_config_get_hash_select(uint8 lchip, ctc_lb_hash_config_t* p_hash_cfg)
{
    IpeHashCtl0_m hash_ctl;
    uint32 cmd = 0;
	uint32 flag = 0;
	uint8 mode = 0;
	uint8 flow_label_en = 0;

	sal_memset(&hash_ctl, 0, sizeof(hash_ctl));
    cmd = DRV_IOR(IpeHashCtl0_t + p_hash_cfg->sel_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));
	p_hash_cfg->value = 0;
    flow_label_en = GetIpeHashCtl0(V, ipv4PayloadIsV6UseFlowLabel_f, &hash_ctl);

    switch (p_hash_cfg->hash_select)
    {
            /*L2 header hash */
        case CTC_LB_HASH_SELECT_L2:
            flag = GetIpeHashCtl0(V, outerL2BulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 0);
            break;

            /*ipv4 header hash */
        case CTC_LB_HASH_SELECT_IPV4:
            flag = GetIpeHashCtl0(V, ipv4InnerL3BulkBitmap0_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            break;
        case CTC_LB_HASH_SELECT_IPV4_TCP_UDP:
            flag = GetIpeHashCtl0(V, ipv4TcpUdpBulkBitmap1_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            break;
        case CTC_LB_HASH_SELECT_IPV4_TCP_UDP_PORTS_EQUAL:
            flag = GetIpeHashCtl0(V, ipv4TcpUdpBulkBitmap0_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            break;
        case CTC_LB_HASH_SELECT_IPV4_VXLAN:
            flag = GetIpeHashCtl0(V, ipv4VxlanBulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 1);
            break;
        case CTC_LB_HASH_SELECT_IPV4_GRE:
            flag = GetIpeHashCtl0(V, ipv4GreBulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE5(flag, p_hash_cfg->value, 0);
            break;
        case CTC_LB_HASH_SELECT_IPV4_NVGRE:
            flag = GetIpeHashCtl0(V, ipv4NvgreBulkBitmap_f, &hash_ctl);
		    GET_HASH_KEY_TEMPLATE5(flag, p_hash_cfg->value, 1);
            break;

            /*ipv6 header hash */
        case CTC_LB_HASH_SELECT_IPV6:
            flag = GetIpeHashCtl0(V, ipv4InnerL3BulkBitmap1_f, &hash_ctl);
			mode = flow_label_en ? 2:0;
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            break;
        case CTC_LB_HASH_SELECT_IPV6_TCP_UDP:
            flag = GetIpeHashCtl0(V, ipv4TcpUdpBulkBitmap3_f, &hash_ctl);
			mode = flow_label_en ? 2:0;
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            break;
        case CTC_LB_HASH_SELECT_IPV6_TCP_UDP_PORTS_EQUAL:
            flag = GetIpeHashCtl0(V, ipv4TcpUdpBulkBitmap2_f, &hash_ctl);
			mode = flow_label_en ? 2:0;
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            break;
        case CTC_LB_HASH_SELECT_IPV6_VXLAN:
            flag = GetIpeHashCtl0(V, ipv6VxlanBulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 1);
            break;
        case CTC_LB_HASH_SELECT_IPV6_GRE:
            flag = GetIpeHashCtl0(V, ipv6GreBulkBitmap_f, &hash_ctl);
		    mode = flow_label_en ? 2:0;
			GET_HASH_KEY_TEMPLATE5(flag, p_hash_cfg->value, mode);
            break;
        case CTC_LB_HASH_SELECT_IPV6_NVGRE:
            flag = GetIpeHashCtl0(V, ipv6NvgreBulkBitmap_f, &hash_ctl);
		    mode = flow_label_en ? 3:1;
			GET_HASH_KEY_TEMPLATE5(flag, p_hash_cfg->value, mode);
            break;
        case CTC_LB_HASH_SELECT_NVGRE_INNER_L2:
            flag = GetIpeHashCtl0(V, nvgreInnerL2BulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 1);
            break;
        case CTC_LB_HASH_SELECT_NVGRE_INNER_IPV4:
            flag = GetIpeHashCtl0(V, nvgreInnerL3BulkBitmap0_f, &hash_ctl);
		    CTC_BIT_SET(mode, 3);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            break;
        case CTC_LB_HASH_SELECT_NVGRE_INNER_IPV6:
            flag = GetIpeHashCtl0(V, nvgreInnerL3BulkBitmap1_f, &hash_ctl);
		    mode = flow_label_en ? 2:0;
		    CTC_BIT_SET(mode, 3);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            break;
        case CTC_LB_HASH_SELECT_VXLAN_INNER_L2:
            flag = GetIpeHashCtl0(V, vxlanInnerL2BulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 1);
            break;
        case CTC_LB_HASH_SELECT_VXLAN_INNER_IPV4:
            flag = GetIpeHashCtl0(V, vxlanInnerL3BulkBitmap0_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 1);
            break;
        case CTC_LB_HASH_SELECT_VXLAN_INNER_IPV6:
            flag = GetIpeHashCtl0(V, vxlanInnerL3BulkBitmap1_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 1);
            break;

            /*mpls header hash */
        case CTC_LB_HASH_SELECT_MPLS:
            flag = GetIpeHashCtl0(V, mplsTransitInnerL3IpBulkBitmap_f, &hash_ctl);
            flag = GetIpeHashCtl0(V, mplsTransitInnerOthersBulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE3(flag, p_hash_cfg->value);
            SYS_LB_HASH_MPLS_GET_INNER_MAC(flag, p_hash_cfg->value, hash_ctl);
            break;
        case CTC_LB_HASH_SELECT_MPLS_INNER_IPV4:
            flag = GetIpeHashCtl0(V, mplsTransitInnerL3Ipv4BulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE3(flag, p_hash_cfg->value);
            SYS_LB_HASH_MPLS_GET_INNER_MAC(flag, p_hash_cfg->value, hash_ctl);
            break;
        case CTC_LB_HASH_SELECT_MPLS_INNER_IPV6:
            flag = GetIpeHashCtl0(V, mplsTransitInnerL3Ipv6BulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE3(flag, p_hash_cfg->value);
            SYS_LB_HASH_MPLS_GET_INNER_MAC(flag, p_hash_cfg->value, hash_ctl);
            break;
        case CTC_LB_HASH_SELECT_MPLS_VPWS_RAW:
            flag = GetIpeHashCtl0(V, mplsL2VpnDecapsBypassInnerParserBulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE3(flag, p_hash_cfg->value);
            SYS_LB_HASH_MPLS_GET_INNER_MAC(flag, p_hash_cfg->value, hash_ctl);
            break;
        case CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_L2:
            flag = GetIpeHashCtl0(V, mplsL2VpnDecapsInnerL2BulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 0);
			break;
        case CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV4:
            flag = GetIpeHashCtl0(V, mplsL2VpnDecapsInnerL3Ipv4BulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            break;
        case CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV6:
            flag = GetIpeHashCtl0(V, mplsL2VpnDecapsInnerL3Ipv6BulkBitmap_f, &hash_ctl);
			mode = flow_label_en ? 2:0;
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            break;
        case CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV4:
            flag = GetIpeHashCtl0(V, mplsL3VpnDecapsInnerL3Ipv4BulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            break;
        case CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV6:
            flag = GetIpeHashCtl0(V, mplsL3VpnDecapsInnerL3Ipv6BulkBitmap_f, &hash_ctl);
			mode = flow_label_en ? 2:0;
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            break;

            /*trill header hash */
        case CTC_LB_HASH_SELECT_TRILL_OUTER_L2_ONLY:
            flag = GetIpeHashCtl0(V, trillTransitOuterL2HashBulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 0);
            break;
        case CTC_LB_HASH_SELECT_TRILL:
            flag = GetIpeHashCtl0(V, trillTransitTunnelHashBulkBitmap1_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE1(flag, p_hash_cfg->value);
            break;
        case CTC_LB_HASH_SELECT_TRILL_INNER_L2:
            flag = GetIpeHashCtl0(V, trillTransitTunnelHashBulkBitmap0_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE1(flag, p_hash_cfg->value);
            break;
        case CTC_LB_HASH_SELECT_TRILL_INNER_IPV4:
            flag = GetIpeHashCtl0(V, trillTransitInnerL3Ipv4HashBulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            break;
        case CTC_LB_HASH_SELECT_TRILL_INNER_IPV6:
            flag = GetIpeHashCtl0(V, trillTransitInnerL3Ipv6HashBulkBitmap_f, &hash_ctl);
			mode = flow_label_en ? 2:0;
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            break;
        case CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_L2:
            flag = GetIpeHashCtl0(V, trillTerminateInnerL2HashBulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 0);
            break;
        case CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_IPV4:
            flag = GetIpeHashCtl0(V, trillTerminateInnerL3Ipv4HashBulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            break;
        case CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_IPV6:
            flag = GetIpeHashCtl0(V, trillTerminateInnerL3Ipv6HashBulkBitmap_f, &hash_ctl);
			mode = flow_label_en ? 2:0;
			GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, mode);
            break;

            /*fcoe heaer hash */
        case CTC_LB_HASH_SELECT_FCOE:
            flag = GetIpeHashCtl0(V, fcoeBulkBitmap_f, &hash_ctl);
			GET_HASH_KEY_TEMPLATE2(flag, p_hash_cfg->value);
            break;

            /*flex tunnel heaer hash */
        case CTC_LB_HASH_SELECT_FLEX_TNL_INNER_L2:
            flag = GetIpeHashCtl0(V, unknownInnerL2BulkBitmap_f, &hash_ctl);
            GET_HASH_KEY_TEMPLATE0(flag, p_hash_cfg->value, 0);
            break;

        case CTC_LB_HASH_SELECT_FLEX_TNL_INNER_IPV4:
            flag = GetIpeHashCtl0(V, unknownInnerIpBulkBitmap_f, &hash_ctl);
            GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            break;

        case CTC_LB_HASH_SELECT_FLEX_TNL_INNER_IPV6:
            flag = GetIpeHashCtl0(V, unknownInnerIpv6BulkBitmap_f, &hash_ctl);
            GET_HASH_KEY_TEMPLATE4(flag, p_hash_cfg->value, 0);
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_hash_config_set_hash_control(uint8 lchip, ctc_lb_hash_config_t* p_hash_cfg)
{
    IpeHashCtl0_m hash_ctl;
    uint32 cmd = 0;
	uint32 value = 0;
    uint32 step  = 0;
    sal_memset(&hash_ctl, 0, sizeof(hash_ctl));

    cmd = DRV_IOR(IpeHashCtl0_t + p_hash_cfg->sel_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    switch (p_hash_cfg->hash_control)
    {
        case CTC_LB_HASH_CONTROL_L2_ONLY:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, disableIpv4Hash_f, &hash_ctl, value);
			SetIpeHashCtl0(V, disableIpv6Hash_f, &hash_ctl, value);
			SetIpeHashCtl0(V, disableMplsHash_f, &hash_ctl, value);
			SetIpeHashCtl0(V, disableFcoeHash_f, &hash_ctl, value);
			SetIpeHashCtl0(V, disableTrillHash_f, &hash_ctl, value);
			break;
        case CTC_LB_HASH_CONTROL_DISABLE_IPV4:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, disableIpv4Hash_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_IPV6:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, disableIpv6Hash_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_MPLS:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, disableMplsHash_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_FCOE:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, disableFcoeHash_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TRILL:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, disableTrillHash_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV4_IN4:
			value = GetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl);
			p_hash_cfg->value ? CTC_BIT_UNSET(value, 2) : CTC_BIT_SET(value, 2);
            SetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV6_IN4:
			value = GetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl);
			p_hash_cfg->value ? CTC_BIT_UNSET(value, 1) : CTC_BIT_SET(value, 1);
            SetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_GRE_IN4:
			value = GetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl);
			p_hash_cfg->value ? CTC_BIT_UNSET(value, 3) : CTC_BIT_SET(value, 3);
            SetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN4:
			value = GetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl);
			p_hash_cfg->value ? CTC_BIT_UNSET(value, 4) : CTC_BIT_SET(value, 4);
	        SetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN4:
			value = GetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl);
			p_hash_cfg->value ? CTC_BIT_UNSET(value, 5) : CTC_BIT_SET(value, 5);
            SetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV4_IN6:
			value = GetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl);
			p_hash_cfg->value ? CTC_BIT_UNSET(value, 2) : CTC_BIT_SET(value, 2);
	        SetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV6_IN6:
			value = GetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl);
			p_hash_cfg->value ? CTC_BIT_UNSET(value, 1) : CTC_BIT_SET(value, 1);
	        SetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_GRE_IN6:
			value = GetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl);
			p_hash_cfg->value ? CTC_BIT_UNSET(value, 3) : CTC_BIT_SET(value, 3);
	        SetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN6:
			value = GetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl);
			p_hash_cfg->value ? CTC_BIT_UNSET(value, 4) : CTC_BIT_SET(value, 4);
	        SetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN6:
			value = GetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl);
			p_hash_cfg->value ? CTC_BIT_UNSET(value, 5) : CTC_BIT_SET(value, 5);
	        SetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_MPLS_EXCLUDE_RSV:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, mplsHashSkipReservedLabel_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_IPV6_ADDR_COLLAPSE:
            value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, ipv6AddrCompressMode_f, &hash_ctl,value);
            break;
        case CTC_LB_HASH_CONTROL_IPV6_USE_FLOW_LABEL:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, ipv4PayloadIsV6UseFlowLabel_f, &hash_ctl, value);
            SetIpeHashCtl0(V, bulk12And13Ipv6UseFlowLabel0_f, &hash_ctl, value);
            SetIpeHashCtl0(V, bulk12And13Ipv6UseFlowLabel1_f, &hash_ctl, value);
            SetIpeHashCtl0(V, bulk12And13Ipv6UseFlowLabel2_f, &hash_ctl, value);
            SetIpeHashCtl0(V, bulk12And13Ipv6UseFlowLabel3_f, &hash_ctl, value);
            SetIpeHashCtl0(V, bulk12And13Ipv6UseFlowLabel4_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_NVGRE_AWARE:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, awaredNvgre_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_FCOE_SYMMETRIC_EN:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, fcoeSymmetricHashEn_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_UDF_SELECT_MODE:
			CTC_MAX_VALUE_CHECK(p_hash_cfg->value,7);
            SetIpeHashCtl0(V, udfDataSel_f, &hash_ctl, p_hash_cfg->value);
            break;
        case CTC_LB_HASH_CONTROL_USE_MAPPED_VLAN:
            value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, vidDataSel_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_IPV4_SYMMETRIC_EN:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, outerIpv4SymmetricHashEn_f, &hash_ctl, value);
            SetIpeHashCtl0(V, innerIpv4SymmetricHashEn_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_IPV6_SYMMETRIC_EN:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, outerIpv6SymmetricHashEn_f, &hash_ctl, value);
            SetIpeHashCtl0(V, innerIpv6SymmetricHashEn_f, &hash_ctl, value);
            break;

        case CTC_LB_HASH_CONTROL_L2VPN_USE_INNER_L2:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, mplsL2VpnDecapsUseInnerL2Hash_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_VPWS_USE_INNER:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, vpwsDecapsHashMode_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_TRILL_DECAP_USE_INNER_L2:
			value = p_hash_cfg->value ? 0 : 1;
            SetIpeHashCtl0(V, trillTerminateNodeHashMode_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_TRILL_TRANSMIT_MODE:
			CTC_MAX_VALUE_CHECK(p_hash_cfg->value,3);
            SetIpeHashCtl0(V, trillTransitNodeHashMode_f, &hash_ctl, p_hash_cfg->value);
            break;
        case CTC_LB_HASH_CONTROL_MPLS_ENTROPY_EN:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, mplsL2VpnDecapsVpwsUseGlobalInnerHdrHashA_f, &hash_ctl, value);
            SetIpeHashCtl0(V, mplsL2VpnDecapsVpwsUseGlobalInnerHdrHashB_f, &hash_ctl, value);
            SetIpeHashCtl0(V, mplsTransitUseGlobalInnerHdrHashA_f, &hash_ctl, value);
            SetIpeHashCtl0(V, mplsTransitUseGlobalInnerHdrHashB_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_MPLS_LABEL2_EN:
			value = p_hash_cfg->value ? 3 : 0;
            SetIpeHashCtl0(V, hashTemplate3Bulk13IsMplsLabel_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_VXLAN_USE_INNER_L2:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, vxlanInnerL2HashEn_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_NVGRE_USE_INNER_L2:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, nvGreInnerL2HashEn_f, &hash_ctl, value);
            break;
        case CTC_LB_HASH_CONTROL_FLEX_TNL_USE_INNER_L2:
			value = p_hash_cfg->value ? 1 : 0;
            SetIpeHashCtl0(V, forceInnerL2HashEn_f, &hash_ctl, value);
            break;

        case CTC_LB_HASH_CONTROL_HASH_SEED:
			CTC_MAX_VALUE_CHECK(p_hash_cfg->value,0xFFFFFFFF);
            SetIpeHashCtl0(V, hashSeed_f, &hash_ctl, p_hash_cfg->value);
            break;
        case CTC_LB_HASH_CONTROL_HASH_TYPE_A:
        case CTC_LB_HASH_CONTROL_HASH_TYPE_B:
        {
            switch(p_hash_cfg->value)
            {
            case CTC_LB_HASH_TYPE_CRC8_POLY1:
                value = SYS_LB_HASHPOLY_CRC8_POLY1;
                break;
            case CTC_LB_HASH_TYPE_CRC8_POLY2:
                value = SYS_LB_HASHPOLY_CRC8_POLY2;
                break;
            case CTC_LB_HASH_TYPE_XOR8:
                value = SYS_LB_HASHPOLY_XOR8;
                break;
            case CTC_LB_HASH_TYPE_CRC10_POLY1:
                value = SYS_LB_HASHPOLY_CRC10_POLY1;
                break;
            case CTC_LB_HASH_TYPE_CRC10_POLY2:
                value = SYS_LB_HASHPOLY_CRC10_POLY2;
                break;
            case CTC_LB_HASH_TYPE_XOR16:
                value = SYS_LB_HASHPOLY_XOR16;
                break;
            case CTC_LB_HASH_TYPE_CRC16_POLY1:
                value = SYS_LB_HASHPOLY_CRC16_POLY1;
                break;
            case CTC_LB_HASH_TYPE_CRC16_POLY2:
                value = SYS_LB_HASHPOLY_CRC16_POLY2;
                break;
            case CTC_LB_HASH_TYPE_CRC16_POLY3:
                value = SYS_LB_HASHPOLY_CRC16_POLY3;
                break;
            case CTC_LB_HASH_TYPE_CRC16_POLY4:
                value = SYS_LB_HASHPOLY_CRC16_POLY4;
                break;
            default:
                return CTC_E_INVALID_PARAM;
            }
            (p_hash_cfg->hash_control == CTC_LB_HASH_CONTROL_HASH_TYPE_A)?
            (SetIpeHashCtl0(V, hashPolySelectA_f, &hash_ctl, value)):
            (SetIpeHashCtl0(V, hashPolySelectB_f, &hash_ctl, value));
        }
            break;
        case CTC_LB_HASH_CONTROL_HASH_TYPE_UDF_DATA_BYTE_MASK:
            value = p_hash_cfg->value;
            step =  IpeSclFlowHashBulkBitmapCtl_array_1_flexHashUdfByteMask_f - IpeSclFlowHashBulkBitmapCtl_array_0_flexHashUdfByteMask_f;
            cmd = DRV_IOW(IpeSclFlowHashBulkBitmapCtl_t, IpeSclFlowHashBulkBitmapCtl_array_0_flexHashUdfByteMask_f + step * p_hash_cfg->sel_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOW(IpeHashCtl0_t + p_hash_cfg->sel_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_hash_config_get_hash_control(uint8 lchip, ctc_lb_hash_config_t* p_hash_cfg)
{
    IpeHashCtl0_m hash_ctl;
    uint32 cmd = 0;
    uint32 value = 0;
    uint8  step  = 0;
    sal_memset(&hash_ctl, 0, sizeof(hash_ctl));

    cmd = DRV_IOR(IpeHashCtl0_t + p_hash_cfg->sel_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    switch (p_hash_cfg->hash_control)
    {
        case CTC_LB_HASH_CONTROL_L2_ONLY:
            value = GetIpeHashCtl0(V, disableIpv4Hash_f, &hash_ctl);
            value &= GetIpeHashCtl0(V, disableIpv6Hash_f, &hash_ctl);
            value &= GetIpeHashCtl0(V, disableMplsHash_f, &hash_ctl);
            value &= GetIpeHashCtl0(V, disableFcoeHash_f, &hash_ctl);
            value &= GetIpeHashCtl0(V, disableTrillHash_f, &hash_ctl);
            p_hash_cfg->value = value;
            break;

        case CTC_LB_HASH_CONTROL_DISABLE_IPV4:
            p_hash_cfg->value = GetIpeHashCtl0(V, disableIpv4Hash_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_IPV6:
            p_hash_cfg->value = GetIpeHashCtl0(V, disableIpv6Hash_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_MPLS:
            p_hash_cfg->value = GetIpeHashCtl0(V, disableMplsHash_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_FCOE:
            p_hash_cfg->value = GetIpeHashCtl0(V, disableFcoeHash_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TRILL:
            p_hash_cfg->value = GetIpeHashCtl0(V, disableTrillHash_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV4_IN4:
            value = GetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl);
            p_hash_cfg->value = !CTC_IS_BIT_SET(value, 2);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV6_IN4:
            value = GetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl);
            p_hash_cfg->value = !CTC_IS_BIT_SET(value, 1);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_GRE_IN4:
            value = GetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl);
            p_hash_cfg->value = !CTC_IS_BIT_SET(value, 3);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN4:
            value = GetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl);
            p_hash_cfg->value = !CTC_IS_BIT_SET(value, 4);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN4:
            value = GetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl);
            p_hash_cfg->value = !CTC_IS_BIT_SET(value, 5);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV4_IN6:
            value = GetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl);
            p_hash_cfg->value = !CTC_IS_BIT_SET(value, 2);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_IPV6_IN6:
            value = GetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl);
            p_hash_cfg->value = !CTC_IS_BIT_SET(value, 1);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_GRE_IN6:
            value = GetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl);
            p_hash_cfg->value = !CTC_IS_BIT_SET(value, 3);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_NVGRE_IN6:
            value = GetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl);
            p_hash_cfg->value = !CTC_IS_BIT_SET(value, 4);
            break;
        case CTC_LB_HASH_CONTROL_DISABLE_TUNNEL_VXLAN_IN6:
            value = GetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl);
            p_hash_cfg->value = !CTC_IS_BIT_SET(value, 5);
            break;
        case CTC_LB_HASH_CONTROL_MPLS_EXCLUDE_RSV:
            p_hash_cfg->value = GetIpeHashCtl0(V, mplsHashSkipReservedLabel_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_IPV6_ADDR_COLLAPSE:
            p_hash_cfg->value = GetIpeHashCtl0(V, ipv6AddrCompressMode_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_IPV6_USE_FLOW_LABEL:
            p_hash_cfg->value = GetIpeHashCtl0(V, ipv4PayloadIsV6UseFlowLabel_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_NVGRE_AWARE:
            p_hash_cfg->value = GetIpeHashCtl0(V, awaredNvgre_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_FCOE_SYMMETRIC_EN:
            p_hash_cfg->value = GetIpeHashCtl0(V, fcoeSymmetricHashEn_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_UDF_SELECT_MODE:
            p_hash_cfg->value = GetIpeHashCtl0(V, udfDataSel_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_USE_MAPPED_VLAN:
            p_hash_cfg->value = GetIpeHashCtl0(V, vidDataSel_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_IPV4_SYMMETRIC_EN:
            p_hash_cfg->value = GetIpeHashCtl0(V, outerIpv4SymmetricHashEn_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_IPV6_SYMMETRIC_EN:
            p_hash_cfg->value = GetIpeHashCtl0(V, outerIpv6SymmetricHashEn_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_L2VPN_USE_INNER_L2:
            p_hash_cfg->value = GetIpeHashCtl0(V, mplsL2VpnDecapsUseInnerL2Hash_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_VPWS_USE_INNER:
            p_hash_cfg->value = GetIpeHashCtl0(V, vpwsDecapsHashMode_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_TRILL_DECAP_USE_INNER_L2:
            p_hash_cfg->value = GetIpeHashCtl0(V, trillTerminateNodeHashMode_f, &hash_ctl)?0:1;
            break;
        case CTC_LB_HASH_CONTROL_TRILL_TRANSMIT_MODE:
            p_hash_cfg->value = GetIpeHashCtl0(V, trillTransitNodeHashMode_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_MPLS_ENTROPY_EN:
            p_hash_cfg->value = GetIpeHashCtl0(V, mplsL2VpnDecapsVpwsUseGlobalInnerHdrHashA_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_MPLS_LABEL2_EN:
            value = GetIpeHashCtl0(V, hashTemplate3Bulk13IsMplsLabel_f, &hash_ctl);
            p_hash_cfg->value = value ? 1 : 0;
            break;
        case CTC_LB_HASH_CONTROL_VXLAN_USE_INNER_L2:
            p_hash_cfg->value =  GetIpeHashCtl0(V, vxlanInnerL2HashEn_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_NVGRE_USE_INNER_L2:
            p_hash_cfg->value = GetIpeHashCtl0(V, nvGreInnerL2HashEn_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_FLEX_TNL_USE_INNER_L2:
            p_hash_cfg->value = GetIpeHashCtl0(V, forceInnerL2HashEn_f, &hash_ctl);
            break;
        case CTC_LB_HASH_CONTROL_HASH_SEED:
            p_hash_cfg->value = GetIpeHashCtl0(V, hashSeed_f, &hash_ctl);
            break;

        case CTC_LB_HASH_CONTROL_HASH_TYPE_A:
        case CTC_LB_HASH_CONTROL_HASH_TYPE_B:
            {
                value = (p_hash_cfg->hash_control == CTC_LB_HASH_CONTROL_HASH_TYPE_A)?
                GetIpeHashCtl0(V, hashPolySelectA_f, &hash_ctl):
                GetIpeHashCtl0(V, hashPolySelectB_f, &hash_ctl);
                switch(value)
                {
                case SYS_LB_HASHPOLY_CRC8_POLY1:
                    p_hash_cfg->value = CTC_LB_HASH_TYPE_CRC8_POLY1;
                    break;
                case SYS_LB_HASHPOLY_CRC8_POLY2:
                    p_hash_cfg->value = CTC_LB_HASH_TYPE_CRC8_POLY2;
                    break;
                case SYS_LB_HASHPOLY_XOR8:
                    p_hash_cfg->value = CTC_LB_HASH_TYPE_XOR8;
                    break;
                case SYS_LB_HASHPOLY_CRC10_POLY1:
                    p_hash_cfg->value = CTC_LB_HASH_TYPE_CRC10_POLY1;
                    break;
                case SYS_LB_HASHPOLY_CRC10_POLY2:
                    p_hash_cfg->value = CTC_LB_HASH_TYPE_CRC10_POLY2;
                    break;
                case SYS_LB_HASHPOLY_XOR16:
                    p_hash_cfg->value = CTC_LB_HASH_TYPE_XOR16;
                    break;
                case SYS_LB_HASHPOLY_CRC16_POLY1:
                    p_hash_cfg->value = CTC_LB_HASH_TYPE_CRC16_POLY1;
                    break;
                case SYS_LB_HASHPOLY_CRC16_POLY2:
                    p_hash_cfg->value = CTC_LB_HASH_TYPE_CRC16_POLY2;
                    break;
                case SYS_LB_HASHPOLY_CRC16_POLY3:
                    p_hash_cfg->value = CTC_LB_HASH_TYPE_CRC16_POLY3;
                    break;
                case SYS_LB_HASHPOLY_CRC16_POLY4:
                    p_hash_cfg->value = CTC_LB_HASH_TYPE_CRC16_POLY4;
                    break;
                default:
                    return CTC_E_INVALID_PARAM;
                }
            }
            break;
       case CTC_LB_HASH_CONTROL_HASH_TYPE_UDF_DATA_BYTE_MASK:
            step =  IpeSclFlowHashBulkBitmapCtl_array_1_flexHashUdfByteMask_f - IpeSclFlowHashBulkBitmapCtl_array_0_flexHashUdfByteMask_f;
            cmd = DRV_IOR(IpeSclFlowHashBulkBitmapCtl_t, IpeSclFlowHashBulkBitmapCtl_array_0_flexHashUdfByteMask_f + step * p_hash_cfg->sel_id);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            p_hash_cfg->value = value;
            break;
        default:
               return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}



int32
sys_tsingma_hash_set_config(uint8 lchip, void* p_hash)
{
	ctc_lb_hash_config_t* p_hash_cfg = 0;
	p_hash_cfg = (ctc_lb_hash_config_t*) p_hash;
    CTC_PTR_VALID_CHECK(p_hash_cfg);

	 /*CTC_MAX_VALUE_CHECK(p_hash_cfg->cfg_type,1);*/
	CTC_MAX_VALUE_CHECK(p_hash_cfg->hash_select, CTC_LB_HASH_SELECT_MAX-1);
	CTC_MAX_VALUE_CHECK(p_hash_cfg->hash_control, CTC_LB_HASH_CONTROL_MAX-1);
	CTC_MAX_VALUE_CHECK(p_hash_cfg->value, 0xFFFFFFFF);


    if (p_hash_cfg->cfg_type == CTC_LB_HASH_CFG_HASH_SELECT)
    {
        if (p_hash_cfg->hash_select == CTC_LB_HASH_SELECT_UDF)
        {
            CTC_MAX_VALUE_CHECK(p_hash_cfg->sel_id, SYS_LB_HASH_SCL_PROFILE_NUM -1);
            CTC_MIN_VALUE_CHECK(p_hash_cfg->sel_id, 1);
            CTC_ERROR_RETURN(_sys_tsingma_hash_config_set_udf_select(lchip, p_hash_cfg));
        }
        else
        {
        	CTC_MAX_VALUE_CHECK(p_hash_cfg->sel_id,SYS_LB_HASH_CTL_NUM-1);
            CTC_ERROR_RETURN(_sys_tsingma_hash_config_set_hash_select(lchip, p_hash_cfg));
        }
    }
    else if (p_hash_cfg->cfg_type == CTC_LB_HASH_CFG_HASH_CONTROL)
    {
        if (p_hash_cfg->hash_control != CTC_LB_HASH_CONTROL_HASH_TYPE_UDF_DATA_BYTE_MASK)
        {
            CTC_MAX_VALUE_CHECK(p_hash_cfg->sel_id, SYS_LB_HASH_CTL_NUM-1);
        }
        else
        {
            CTC_MAX_VALUE_CHECK(p_hash_cfg->sel_id, SYS_LB_HASH_SCL_PROFILE_NUM-1);
            CTC_MIN_VALUE_CHECK(p_hash_cfg->sel_id, 1);
        }
        CTC_ERROR_RETURN(_sys_tsingma_hash_config_set_hash_control(lchip, p_hash_cfg));
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_hash_get_config(uint8 lchip, void* p_hash)
{

    ctc_lb_hash_config_t* p_hash_cfg = 0;
    p_hash_cfg = (ctc_lb_hash_config_t*) p_hash;
    CTC_PTR_VALID_CHECK(p_hash_cfg);

    if (p_hash_cfg->cfg_type == CTC_LB_HASH_CFG_HASH_SELECT)
    {
        if (p_hash_cfg->hash_select == CTC_LB_HASH_SELECT_UDF)
        {
            CTC_MAX_VALUE_CHECK(p_hash_cfg->sel_id, SYS_LB_HASH_SCL_PROFILE_NUM - 1);
            CTC_ERROR_RETURN(_sys_tsingma_hash_config_get_udf_select(lchip, p_hash_cfg));
        }
        else
        {
            CTC_MAX_VALUE_CHECK(p_hash_cfg->sel_id, SYS_LB_HASH_CTL_NUM - 1);
            CTC_ERROR_RETURN(_sys_tsingma_hash_config_get_hash_select(lchip, p_hash_cfg));
        }
    }
    else if (p_hash_cfg->cfg_type == CTC_LB_HASH_CFG_HASH_CONTROL)
    {
        if (p_hash_cfg->hash_control != CTC_LB_HASH_CONTROL_HASH_TYPE_UDF_DATA_BYTE_MASK)
        {
            CTC_MAX_VALUE_CHECK(p_hash_cfg->sel_id, SYS_LB_HASH_CTL_NUM-1);
        }
        else
        {
            CTC_MAX_VALUE_CHECK(p_hash_cfg->sel_id, SYS_LB_HASH_SCL_PROFILE_NUM-1);
        }
        CTC_ERROR_RETURN(_sys_tsingma_hash_config_get_hash_control(lchip, p_hash_cfg));
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_hash_set_lag_offset(uint8 lchip, ctc_lb_hash_offset_t* p_hash_offset)
{
    uint8 offset = 0;
    uint8 select = 0;
	uint32 cmd0 = 0;
    uint8 fld_offset = 0;
    uint8 offset_valid = (CTC_LB_HASH_OFFSET_DISABLE == p_hash_offset->offset)? 0: 1;
    DsPortBasedHashProfile1_m DsPortBasedHashProfile1;

	sal_memset(&DsPortBasedHashProfile1, 0, sizeof(DsPortBasedHashProfile1));


    cmd0 = DRV_IOR(DsPortBasedHashProfile1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_hash_offset->profile_id), cmd0, &DsPortBasedHashProfile1));
    if (p_hash_offset->use_packet_head_hash == 1)
    {
        select = 8;
        offset = 0;
    }
    else
    {
        select = (p_hash_offset->offset) / 16;
        offset = (p_hash_offset->offset) % 16;
    }

    if (p_hash_offset->hash_module == CTC_LB_HASH_MODULE_STACKING)
    {
        fld_offset = (DsPortBasedHashProfile1_gLag_1_slbValid_f - DsPortBasedHashProfile1_gLag_0_slbValid_f);
    }
    else if(p_hash_offset->hash_module == CTC_LB_HASH_MODULE_LINKAGG)
    {
        fld_offset = 0;
    }

    switch (p_hash_offset->hash_mode)
    {
        case CTC_LB_HASH_MODE_STATIC:
	        SetDsPortBasedHashProfile1(V, gLag_0_slbValid_f + fld_offset, &DsPortBasedHashProfile1, offset_valid);
	        SetDsPortBasedHashProfile1(V, gLag_0_hashSelectForSlb_f + fld_offset, &DsPortBasedHashProfile1, select);
	        SetDsPortBasedHashProfile1(V, gLag_0_hashOffsetForSlb_f + fld_offset, &DsPortBasedHashProfile1, offset);
            break;

        case CTC_LB_HASH_MODE_DLB_FLOW_SET:
            SetDsPortBasedHashProfile1(V, gLag_0_dlbValid_f + fld_offset, &DsPortBasedHashProfile1, offset_valid);
            SetDsPortBasedHashProfile1(V, gLag_0_hashSelectForDlb_f + fld_offset, &DsPortBasedHashProfile1, select);
            SetDsPortBasedHashProfile1(V, gLag_0_hashOffsetForDlb_f + fld_offset, &DsPortBasedHashProfile1, offset);
            break;

        case CTC_LB_HASH_MODE_RESILIENT:
            SetDsPortBasedHashProfile1(V, gLag_0_rhValid_f + fld_offset, &DsPortBasedHashProfile1, offset_valid);
            SetDsPortBasedHashProfile1(V, gLag_0_hashSelectForRh_f + fld_offset, &DsPortBasedHashProfile1, select);
            SetDsPortBasedHashProfile1(V, gLag_0_hashOffsetForRh_f + fld_offset, &DsPortBasedHashProfile1, offset);
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    cmd0 = DRV_IOW(DsPortBasedHashProfile1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_hash_offset->profile_id), cmd0, &DsPortBasedHashProfile1));


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_hash_get_lag_offset(uint8 lchip, ctc_lb_hash_offset_t* p_hash_offset)
{
    uint8 offset = 0;
    uint8 select = 0;
	uint32 cmd0 = 0;
    uint8 fld_offset = 0;
    DsPortBasedHashProfile1_m DsPortBasedHashProfile1;

	sal_memset(&DsPortBasedHashProfile1, 0, sizeof(DsPortBasedHashProfile1));


    cmd0 = DRV_IOR(DsPortBasedHashProfile1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_hash_offset->profile_id), cmd0, &DsPortBasedHashProfile1));


    if (p_hash_offset->hash_module == CTC_LB_HASH_MODULE_STACKING)
    {
        fld_offset = (DsPortBasedHashProfile1_gLag_1_slbValid_f - DsPortBasedHashProfile1_gLag_0_slbValid_f);
    }
    else if(p_hash_offset->hash_module == CTC_LB_HASH_MODULE_LINKAGG)
    {
        fld_offset = 0;
    }

    switch (p_hash_offset->hash_mode)
    {
        case CTC_LB_HASH_MODE_STATIC:
	        select = GetDsPortBasedHashProfile1(V, gLag_0_hashSelectForSlb_f + fld_offset, &DsPortBasedHashProfile1);
	        offset = GetDsPortBasedHashProfile1(V, gLag_0_hashOffsetForSlb_f + fld_offset, &DsPortBasedHashProfile1);
            break;

        case CTC_LB_HASH_MODE_DLB_FLOW_SET:
            select = GetDsPortBasedHashProfile1(V, gLag_0_hashSelectForDlb_f + fld_offset, &DsPortBasedHashProfile1);
            offset = GetDsPortBasedHashProfile1(V, gLag_0_hashOffsetForDlb_f + fld_offset, &DsPortBasedHashProfile1);
            break;

        case CTC_LB_HASH_MODE_RESILIENT:
            select = GetDsPortBasedHashProfile1(V, gLag_0_hashSelectForRh_f + fld_offset, &DsPortBasedHashProfile1);
            offset = GetDsPortBasedHashProfile1(V, gLag_0_hashOffsetForRh_f + fld_offset, &DsPortBasedHashProfile1);
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    if (select == 8 && offset == 0)
    {
        p_hash_offset->use_packet_head_hash = 1;
    }
    else
    {
        p_hash_offset->offset = select*16 + offset;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_hash_set_ecmp_offset(uint8 lchip, ctc_lb_hash_offset_t* p_hash_offset)
{
    uint8 offset = 0;
    uint8 select = 0;
    uint32 cmd0 = 0;
    uint8 use_entropy = 0;
    uint8 offset_valid = (CTC_LB_HASH_OFFSET_DISABLE == p_hash_offset->offset)? 0: 1;
    DsPortBasedHashProfile0_m DsPortBasedHashProfile0;
    IpeEcmpCtl_m IpeEcmpCtl;

    if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
    {
        sal_memset(&IpeEcmpCtl, 0, sizeof(IpeEcmpCtl));
        cmd0 = DRV_IOR(IpeEcmpCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd0, &IpeEcmpCtl));
    }
    else
    {
        sal_memset(&DsPortBasedHashProfile0, 0, sizeof(DsPortBasedHashProfile0));
        cmd0 = DRV_IOR(DsPortBasedHashProfile0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_hash_offset->profile_id, cmd0, &DsPortBasedHashProfile0));
    }

    if (p_hash_offset->use_packet_head_hash)
    {
        select = 8;
        offset = 0;
    }
    else
    {
        select = (p_hash_offset->offset) / 16;
        offset = (p_hash_offset->offset) % 16;
    }

    use_entropy = p_hash_offset->use_entropy_hash?1:0;


    switch (p_hash_offset->hash_mode)
    {
    case CTC_LB_HASH_MODE_STATIC:
        if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_VXLAN)
        {
            if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
            {
                /*entropy */
                SetIpeEcmpCtl(V, vxlanEcmpUseEntropy_f, &IpeEcmpCtl, use_entropy);

                SetIpeEcmpCtl(V, hashOffsetForVxlanEcmp_f, &IpeEcmpCtl, offset);
                SetIpeEcmpCtl(V, hashSelectForVxlanEcmp_f, &IpeEcmpCtl, select);
            }
            else
            {
                /*entropy */
                SetDsPortBasedHashProfile0(V, gEcmp_vxlanEcmpUseEntropy_f, &DsPortBasedHashProfile0, use_entropy);

                SetDsPortBasedHashProfile0(V, gEcmp_slbVxlanValid_f, &DsPortBasedHashProfile0, offset_valid);
                SetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForSlbVxlan_f, &DsPortBasedHashProfile0, offset);
                SetDsPortBasedHashProfile0(V, gEcmp_hashSelectForSlbVxlan_f, &DsPortBasedHashProfile0, select);
            }
        }
        else if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_NVGRE)
        {
            if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
            {
                /*entropy */
                SetIpeEcmpCtl(V, nvgreEcmpUseEntropy_f, &IpeEcmpCtl, use_entropy);

                SetIpeEcmpCtl(V, hashOffsetForNvgreEcmp_f, &IpeEcmpCtl, offset);
                SetIpeEcmpCtl(V, hashSelectForNvgreEcmp_f, &IpeEcmpCtl, select);
            }
            else
            {
                /*entropy */
                SetDsPortBasedHashProfile0(V, gEcmp_nvgreEcmpUseEntropy_f, &DsPortBasedHashProfile0, use_entropy);

                SetDsPortBasedHashProfile0(V, gEcmp_slbNvgreValid_f, &DsPortBasedHashProfile0, offset_valid);
                SetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForSlbNvgre_f, &DsPortBasedHashProfile0, offset);
                SetDsPortBasedHashProfile0(V, gEcmp_hashSelectForSlbNvgre_f, &DsPortBasedHashProfile0, select);
            }
        }
        else if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_MPLS)
        {
            if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
            {
                /*entropy */
                SetIpeEcmpCtl(V, mplsEcmpUseEntropy_f, &IpeEcmpCtl, use_entropy);

                SetIpeEcmpCtl(V, hashOffsetForMplsEcmp_f, &IpeEcmpCtl, offset);
                SetIpeEcmpCtl(V, hashSelectForMplsEcmp_f, &IpeEcmpCtl, select);
            }
            else
            {
                /*entropy */
                SetDsPortBasedHashProfile0(V, gEcmp_mplsEcmpUseEntropy_f, &DsPortBasedHashProfile0, use_entropy);

                SetDsPortBasedHashProfile0(V, gEcmp_slbMplsValid_f, &DsPortBasedHashProfile0, offset_valid);
                SetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForSlbMpls_f, &DsPortBasedHashProfile0, offset);
                SetDsPortBasedHashProfile0(V, gEcmp_hashSelectForSlbMpls_f, &DsPortBasedHashProfile0, select);
            }
        }

        else if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_IP)
        {
            if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
            {
                /*entropy */
                SetIpeEcmpCtl(V, l3PktEcmpUseEntropy_f, &IpeEcmpCtl, use_entropy);

                SetIpeEcmpCtl(V, hashOffsetForL3Ecmp_f, &IpeEcmpCtl, offset);
                SetIpeEcmpCtl(V, hashSelectForL3Ecmp_f, &IpeEcmpCtl, select);
            }
            else
            {
                /*entropy */
                SetDsPortBasedHashProfile0(V, gEcmp_l3PktEcmpUseEntropy_f, &DsPortBasedHashProfile0, use_entropy);

                SetDsPortBasedHashProfile0(V, gEcmp_slbL3Valid_f, &DsPortBasedHashProfile0, offset_valid);
                SetDsPortBasedHashProfile0(V, gEcmp_hashSelectForSlbL3_f, &DsPortBasedHashProfile0, select);
                SetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForSlbL3_f, &DsPortBasedHashProfile0, offset);
            }
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
        break;

    case CTC_LB_HASH_MODE_DLB_FLOW_SET:
        if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
        {
            SetIpeEcmpCtl(V, hashOffsetForFlowIdDlbEcmp_f, &IpeEcmpCtl, offset);
            SetIpeEcmpCtl(V, hashSelectForFlowIdDlbEcmp_f, &IpeEcmpCtl, select);
        }
        else
        {
            SetDsPortBasedHashProfile0(V, gEcmp_dlbValid_f, &DsPortBasedHashProfile0, offset_valid);
            SetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForDlbFlowId_f, &DsPortBasedHashProfile0, offset);
            SetDsPortBasedHashProfile0(V, gEcmp_hashSelectForDlbFlowId_f, &DsPortBasedHashProfile0, select);
        }
        break;


    case CTC_LB_HASH_MODE_DLB_MEMBER:
        if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
        {
            SetIpeEcmpCtl(V, hashOffsetForHashValueDlbEcmp_f, &IpeEcmpCtl, offset);
            SetIpeEcmpCtl(V, hashSelectForHashValueDlbEcmp_f, &IpeEcmpCtl, select);
        }
        else
        {
            SetDsPortBasedHashProfile0(V, gEcmp_dlbValid_f, &DsPortBasedHashProfile0, offset_valid);
            SetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForDlbHash_f, &DsPortBasedHashProfile0, offset);
            SetDsPortBasedHashProfile0(V, gEcmp_hashSelectForDlbHash_f, &DsPortBasedHashProfile0, select);
        }
        break;


    case CTC_LB_HASH_MODE_RESILIENT:
        if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
        {
            SetIpeEcmpCtl(V, hashOffsetForRhEcmp_f, &IpeEcmpCtl, offset);
            SetIpeEcmpCtl(V, hashSelectForRhEcmp_f, &IpeEcmpCtl, select);
        }
        else
        {
            SetDsPortBasedHashProfile0(V, gEcmp_rhValid_f, &DsPortBasedHashProfile0, offset_valid);
            SetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForRh_f, &DsPortBasedHashProfile0, offset);
            SetDsPortBasedHashProfile0(V, gEcmp_hashSelectForRh_f, &DsPortBasedHashProfile0, select);
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
    {
        cmd0 = DRV_IOW(IpeEcmpCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd0, &IpeEcmpCtl));
    }
    else
    {
        cmd0 = DRV_IOW(DsPortBasedHashProfile0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_hash_offset->profile_id, cmd0, &DsPortBasedHashProfile0));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_hash_get_ecmp_offset(uint8 lchip, ctc_lb_hash_offset_t* p_hash_offset)
{
    uint8 offset = 0;
    uint8 select = 0;
    uint32 cmd0 = 0;
    uint8 use_entropy = 0;
    DsPortBasedHashProfile0_m DsPortBasedHashProfile0;
    IpeEcmpCtl_m IpeEcmpCtl;

    if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
    {
        sal_memset(&IpeEcmpCtl, 0, sizeof(IpeEcmpCtl));
        cmd0 = DRV_IOR(IpeEcmpCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd0, &IpeEcmpCtl));
    }
    else
    {
        sal_memset(&DsPortBasedHashProfile0, 0, sizeof(DsPortBasedHashProfile0));
        cmd0 = DRV_IOR(DsPortBasedHashProfile0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_hash_offset->profile_id), cmd0, &DsPortBasedHashProfile0));
    }


    switch (p_hash_offset->hash_mode)
    {
    case CTC_LB_HASH_MODE_STATIC:
        if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_VXLAN)
        {
            if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
            {
                /*entropy */
                use_entropy = GetIpeEcmpCtl(V, vxlanEcmpUseEntropy_f, &IpeEcmpCtl);

                offset = GetIpeEcmpCtl(V, hashOffsetForVxlanEcmp_f, &IpeEcmpCtl);
                select = GetIpeEcmpCtl(V, hashSelectForVxlanEcmp_f, &IpeEcmpCtl);
            }
            else
            {
                /*entropy */
                use_entropy = GetDsPortBasedHashProfile0(V, gEcmp_vxlanEcmpUseEntropy_f, &DsPortBasedHashProfile0);

                offset = GetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForSlbVxlan_f, &DsPortBasedHashProfile0);
                select = GetDsPortBasedHashProfile0(V, gEcmp_hashSelectForSlbVxlan_f, &DsPortBasedHashProfile0);
            }
        }
        else if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_NVGRE)
        {
            if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
            {
                /*entropy */
                use_entropy = GetIpeEcmpCtl(V, nvgreEcmpUseEntropy_f, &IpeEcmpCtl);

                offset = GetIpeEcmpCtl(V, hashOffsetForNvgreEcmp_f, &IpeEcmpCtl);
                select = GetIpeEcmpCtl(V, hashSelectForNvgreEcmp_f, &IpeEcmpCtl);
            }
            else
            {
                /*entropy */
                use_entropy = GetDsPortBasedHashProfile0(V, gEcmp_nvgreEcmpUseEntropy_f, &DsPortBasedHashProfile0);

                offset = GetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForSlbNvgre_f, &DsPortBasedHashProfile0);
                select = GetDsPortBasedHashProfile0(V, gEcmp_hashSelectForSlbNvgre_f, &DsPortBasedHashProfile0);
            }
        }
        else if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_MPLS)
        {
            if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
            {
                /*entropy */
                use_entropy = GetIpeEcmpCtl(V, mplsEcmpUseEntropy_f, &IpeEcmpCtl);

                offset = GetIpeEcmpCtl(V, hashOffsetForMplsEcmp_f, &IpeEcmpCtl);
                select = GetIpeEcmpCtl(V, hashSelectForMplsEcmp_f, &IpeEcmpCtl);
            }
            else
            {
                /*entropy */
                use_entropy = GetDsPortBasedHashProfile0(V, gEcmp_mplsEcmpUseEntropy_f, &DsPortBasedHashProfile0);

                offset = GetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForSlbMpls_f, &DsPortBasedHashProfile0);
                select = GetDsPortBasedHashProfile0(V, gEcmp_hashSelectForSlbMpls_f, &DsPortBasedHashProfile0);
            }
        }

        else if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_IP)
        {
            if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
            {
                /*entropy */
                use_entropy = GetIpeEcmpCtl(V, l3PktEcmpUseEntropy_f, &IpeEcmpCtl);

                offset = GetIpeEcmpCtl(V, hashOffsetForL3Ecmp_f, &IpeEcmpCtl);
                select = GetIpeEcmpCtl(V, hashSelectForL3Ecmp_f, &IpeEcmpCtl);
            }
            else
            {
                /*entropy */
                use_entropy = GetDsPortBasedHashProfile0(V, gEcmp_l3PktEcmpUseEntropy_f, &DsPortBasedHashProfile0);

                offset = GetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForSlbL3_f, &DsPortBasedHashProfile0);
                select = GetDsPortBasedHashProfile0(V, gEcmp_hashSelectForSlbL3_f, &DsPortBasedHashProfile0);
            }
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
        break;

    case CTC_LB_HASH_MODE_DLB_FLOW_SET:
        if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
        {
            offset = GetIpeEcmpCtl(V, hashOffsetForFlowIdDlbEcmp_f, &IpeEcmpCtl);
            select = GetIpeEcmpCtl(V, hashSelectForFlowIdDlbEcmp_f, &IpeEcmpCtl);
        }
        else
        {
            offset = GetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForDlbFlowId_f, &DsPortBasedHashProfile0);
            select = GetDsPortBasedHashProfile0(V, gEcmp_hashSelectForDlbFlowId_f, &DsPortBasedHashProfile0);
        }
        break;


    case CTC_LB_HASH_MODE_DLB_MEMBER:
        if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
        {
            offset = GetIpeEcmpCtl(V, hashOffsetForHashValueDlbEcmp_f, &IpeEcmpCtl);
            select = GetIpeEcmpCtl(V, hashSelectForHashValueDlbEcmp_f, &IpeEcmpCtl);
        }
        else
        {
            offset = GetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForDlbHash_f, &DsPortBasedHashProfile0);
            select = GetDsPortBasedHashProfile0(V, gEcmp_hashSelectForDlbHash_f, &DsPortBasedHashProfile0);
        }
        break;


    case CTC_LB_HASH_MODE_RESILIENT:
        if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
        {
            offset = GetIpeEcmpCtl(V, hashOffsetForRhEcmp_f, &IpeEcmpCtl);
            select = GetIpeEcmpCtl(V, hashSelectForRhEcmp_f, &IpeEcmpCtl);
        }
        else
        {
            offset = GetDsPortBasedHashProfile0(V, gEcmp_hashOffsetForRh_f, &DsPortBasedHashProfile0);
            select = GetDsPortBasedHashProfile0(V, gEcmp_hashSelectForRh_f, &DsPortBasedHashProfile0);
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }


    if (select == 8 && offset == 0)
    {
        p_hash_offset->use_packet_head_hash = 1;
    }
    else
    {
        p_hash_offset->offset = select*16 + offset;
    }

    p_hash_offset->use_entropy_hash = use_entropy;

  return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_hash_set_packet_head_offset(uint8 lchip, ctc_lb_hash_offset_t* p_hash_offset)
{
    uint8 offset = 0;
    uint8 select = 0;
    uint8 offset_valid = (CTC_LB_HASH_OFFSET_DISABLE == p_hash_offset->offset)? 0: 1;
    uint32 cmd0 = 0;
    uint32 cmd1 = 0;
    DsPortBasedHashProfile0_m DsPortBasedHashProfile0;
    IpeFwdCtl_m IpeFwdCtl;

    if ((p_hash_offset->profile_id == 0) || (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE))
    {
        sal_memset(&IpeFwdCtl, 0, sizeof(IpeFwdCtl));
        cmd1 = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd1, &IpeFwdCtl));
    }
    else
    {
        sal_memset(&DsPortBasedHashProfile0, 0, sizeof(DsPortBasedHashProfile0));
        cmd0 = DRV_IOR(DsPortBasedHashProfile0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_hash_offset->profile_id, cmd0, &DsPortBasedHashProfile0));
    }

    if (p_hash_offset->use_packet_head_hash)
    {
        select = 8;
        offset = 0;
    }
    else
    {
        select = (p_hash_offset->offset) / 16;
        offset = (p_hash_offset->offset) % 16;
    }

    if ((!offset_valid) && (!p_hash_offset->profile_id))
    {
        CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);
    }

    switch (p_hash_offset->hash_module)
    {
        case CTC_LB_HASH_MODULE_HEAD_LAG:
            if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_UC)
            {
                if (p_hash_offset->profile_id == 0)
                {
                    SetIpeFwdCtl(V, lbIdUcHashSelect_f, &IpeFwdCtl, select);
                    SetIpeFwdCtl(V, lbIdUcHashOffset_f, &IpeFwdCtl, offset);
				}
                else
                {
                    SetDsPortBasedHashProfile0(V, gLbid_ucValid_f, &DsPortBasedHashProfile0, offset_valid);
                    SetDsPortBasedHashProfile0(V, gLbid_hashOffsetForUc_f, &DsPortBasedHashProfile0, offset);
                    SetDsPortBasedHashProfile0(V, gLbid_hashSelectForUc_f, &DsPortBasedHashProfile0, select);
                }
            }
            else if(p_hash_offset->fwd_type == CTC_LB_HASH_FWD_NON_UC)
            {
                if (p_hash_offset->profile_id == 0)
                {
                    SetIpeFwdCtl(V, lbIdNonUcHashSelect_f, &IpeFwdCtl, select);
                    SetIpeFwdCtl(V, lbIdNonUcHashOffset_f, &IpeFwdCtl, offset);
                }
                else
                {
                    SetDsPortBasedHashProfile0(V, gLbid_nonUcValid_f, &DsPortBasedHashProfile0, offset_valid);
                    SetDsPortBasedHashProfile0(V, gLbid_hashOffsetForNonUc_f, &DsPortBasedHashProfile0, offset);
                    SetDsPortBasedHashProfile0(V, gLbid_hashSelectForNonUc_f, &DsPortBasedHashProfile0, select);
                }
            }
            else
            {
                return CTC_E_INVALID_PARAM;
            }

            break;
        case CTC_LB_HASH_MODULE_HEAD_ECMP:
            if (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE)
            {
                SetIpeFwdCtl(V, entropyHashSelect_f, &IpeFwdCtl, select);
                SetIpeFwdCtl(V, entropyHashOffset_f, &IpeFwdCtl, offset);
            }
            else
            {
                SetDsPortBasedHashProfile0(V, gEntropy_entropyValid_f, &DsPortBasedHashProfile0, offset_valid);
                SetDsPortBasedHashProfile0(V, gEntropy_hashOffsetForEntropy_f, &DsPortBasedHashProfile0, offset);
                SetDsPortBasedHashProfile0(V, gEntropy_hashSelectForEntropy_f, &DsPortBasedHashProfile0, select);
            }
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }


    if ((p_hash_offset->profile_id == 0) || (p_hash_offset->profile_id == CTC_LB_HASH_ECMP_GLOBAL_PROFILE))
    {
        cmd1 = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd1, &IpeFwdCtl));
    }
    else
    {
        cmd0 = DRV_IOW(DsPortBasedHashProfile0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_hash_offset->profile_id), cmd0, &DsPortBasedHashProfile0));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_hash_get_packet_head_offset(uint8 lchip, ctc_lb_hash_offset_t* p_hash_offset)
{
    uint8 offset = 0;
    uint8 select = 0;
    uint32 cmd0 = 0;
    uint32 cmd1 = 0;
    DsPortBasedHashProfile0_m DsPortBasedHashProfile0;
    IpeFwdCtl_m IpeFwdCtl;

    if (p_hash_offset->profile_id == 0)
    {
        sal_memset(&IpeFwdCtl, 0, sizeof(IpeFwdCtl));
        cmd1 = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd1, &IpeFwdCtl));
    }
    else
    {
        sal_memset(&DsPortBasedHashProfile0, 0, sizeof(DsPortBasedHashProfile0));
        cmd0 = DRV_IOR(DsPortBasedHashProfile0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_hash_offset->profile_id, cmd0, &DsPortBasedHashProfile0));
    }


    switch (p_hash_offset->hash_module)
    {
        case CTC_LB_HASH_MODULE_HEAD_LAG:
            if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_UC)
            {
                if (p_hash_offset->profile_id == 0)
                {
                    select = GetIpeFwdCtl(V, lbIdUcHashSelect_f, &IpeFwdCtl);
                    offset = GetIpeFwdCtl(V, lbIdUcHashOffset_f, &IpeFwdCtl);
				}
                else
                {
                    offset = GetDsPortBasedHashProfile0(V, gLbid_hashOffsetForUc_f, &DsPortBasedHashProfile0);
                    select = GetDsPortBasedHashProfile0(V, gLbid_hashSelectForUc_f, &DsPortBasedHashProfile0);
                }
            }
            else if(p_hash_offset->fwd_type == CTC_LB_HASH_FWD_NON_UC)
            {
                if (p_hash_offset->profile_id == 0)
                {
                    select = GetIpeFwdCtl(V, lbIdNonUcHashSelect_f, &IpeFwdCtl);
                    offset = GetIpeFwdCtl(V, lbIdNonUcHashOffset_f, &IpeFwdCtl);
                }
                else
                {
                    offset = GetDsPortBasedHashProfile0(V, gLbid_hashOffsetForNonUc_f, &DsPortBasedHashProfile0);
                    select = GetDsPortBasedHashProfile0(V, gLbid_hashSelectForNonUc_f, &DsPortBasedHashProfile0);
                }
            }
            else
            {
                return CTC_E_INVALID_PARAM;
            }

            break;
        case CTC_LB_HASH_MODULE_HEAD_ECMP:
            if (p_hash_offset->profile_id == 0)
            {
                select = GetIpeFwdCtl(V, entropyHashSelect_f, &IpeFwdCtl);
                offset = GetIpeFwdCtl(V, entropyHashOffset_f, &IpeFwdCtl);
            }
            else
            {
                offset = GetDsPortBasedHashProfile0(V, gEntropy_hashOffsetForEntropy_f, &DsPortBasedHashProfile0);
                select = GetDsPortBasedHashProfile0(V, gEntropy_hashSelectForEntropy_f, &DsPortBasedHashProfile0);
            }

            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    if (select == 8 && offset == 0)
    {
        p_hash_offset->use_packet_head_hash = 1;
    }
    else
    {
        p_hash_offset->offset = select*16 + offset;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_hash_set_stacking_non_uc_offset(uint8 lchip, ctc_lb_hash_offset_t* p_hash_offset)
{
    uint8 offset = 0;
    uint8 select = 0;
	uint32 cmd = 0;
    IpeFwdCflexUplinkLagHashCtl_m hash_ctl;

    if (CTC_LB_HASH_OFFSET_DISABLE == p_hash_offset->offset)
    {
        CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);
    }

    if (p_hash_offset->use_packet_head_hash)
    {
        select = 8;
        offset = 0;
    }
    else
    {
        select = (p_hash_offset->offset) / 16;
        offset = (p_hash_offset->offset) % 16;
    }

    sal_memset(&hash_ctl, 0, sizeof(hash_ctl));
    cmd = DRV_IOR(IpeFwdCflexUplinkLagHashCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    SetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_2_hashOffsetCfg_f, &hash_ctl, offset);
    SetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_2_hashSelectCfg_f, &hash_ctl, select);

    cmd = DRV_IOW(IpeFwdCflexUplinkLagHashCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_hash_get_stacking_non_uc_offset(uint8 lchip, ctc_lb_hash_offset_t* p_hash_offset)
{
    uint8 offset = 0;
    uint8 select = 0;
	uint32 cmd = 0;
    IpeFwdCflexUplinkLagHashCtl_m hash_ctl;

    sal_memset(&hash_ctl, 0, sizeof(hash_ctl));
    cmd = DRV_IOR(IpeFwdCflexUplinkLagHashCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    offset = GetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_2_hashOffsetCfg_f, &hash_ctl);
    select = GetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_2_hashSelectCfg_f, &hash_ctl);

    if (select == 8 && offset == 0)
    {
        p_hash_offset->use_packet_head_hash = 1;
    }
    else
    {
        p_hash_offset->offset = select*16 + offset;
    }


    return CTC_E_NONE;
}



int32
sys_tsingma_hash_set_offset(uint8 lchip, ctc_lb_hash_offset_t* p_hash_offset_param)
{
      ctc_lb_hash_offset_t* p_hash_offset = 0;

      p_hash_offset = (ctc_lb_hash_offset_t*) p_hash_offset_param;

      CTC_PTR_VALID_CHECK(p_hash_offset);
      if ((p_hash_offset->hash_module == CTC_LB_HASH_MODULE_ECMP) || (p_hash_offset->hash_module == CTC_LB_HASH_MODULE_HEAD_ECMP))
      {
          if ((p_hash_offset->profile_id > 63) && (p_hash_offset->profile_id != CTC_LB_HASH_ECMP_GLOBAL_PROFILE))
          {
              return CTC_E_INVALID_PARAM;
          }
      }
      else
      {
          if (p_hash_offset->profile_id > 63)
          {
              return CTC_E_INVALID_PARAM;
          }
      }
      SYS_USW_LB_HASH_CHK_OFFSET(p_hash_offset->offset);

      if (p_hash_offset->use_entropy_hash &&
          !(p_hash_offset->hash_module == CTC_LB_HASH_MODULE_ECMP &&
      p_hash_offset->hash_mode == CTC_LB_HASH_MODE_STATIC))
      {
          return CTC_E_FEATURE_NOT_SUPPORT;
      }

      switch (p_hash_offset->hash_module)
      {
          case CTC_LB_HASH_MODULE_LINKAGG:
              CTC_ERROR_RETURN(_sys_tsingma_hash_set_lag_offset(lchip, p_hash_offset));
              break;

          case CTC_LB_HASH_MODULE_STACKING:
          {
              if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_NON_UC)
              {
                  CTC_ERROR_RETURN(_sys_tsingma_hash_set_stacking_non_uc_offset(lchip, p_hash_offset));
              }
              else
              {
                  CTC_ERROR_RETURN(_sys_tsingma_hash_set_lag_offset(lchip, p_hash_offset));
              }
          }
              break;

          case CTC_LB_HASH_MODULE_ECMP:
              CTC_ERROR_RETURN(_sys_tsingma_hash_set_ecmp_offset(lchip, p_hash_offset));
              break;

          case CTC_LB_HASH_MODULE_HEAD_LAG:
          case CTC_LB_HASH_MODULE_HEAD_ECMP:
              CTC_ERROR_RETURN(_sys_tsingma_hash_set_packet_head_offset(lchip, p_hash_offset));
              break;

          default:
              return CTC_E_INVALID_PARAM;
      }

      return CTC_E_NONE;
}

int32
sys_tsingma_hash_get_offset(uint8 lchip, void* p_hash_offset_param)
{
      ctc_lb_hash_offset_t* p_hash_offset = 0;

      p_hash_offset = (ctc_lb_hash_offset_t*) p_hash_offset_param;

      CTC_PTR_VALID_CHECK(p_hash_offset);
      if (((p_hash_offset->hash_module != CTC_LB_HASH_MODULE_ECMP) && (p_hash_offset->profile_id > 63))
         || ((p_hash_offset->hash_module == CTC_LB_HASH_MODULE_ECMP) && ((p_hash_offset->profile_id > 63) && (p_hash_offset->profile_id != CTC_LB_HASH_ECMP_GLOBAL_PROFILE))))
      {
         return CTC_E_INVALID_PARAM;
      }
      CTC_MAX_VALUE_CHECK(p_hash_offset->offset, 127);

      switch (p_hash_offset->hash_module)
      {
          case CTC_LB_HASH_MODULE_LINKAGG:
              CTC_ERROR_RETURN(_sys_tsingma_hash_get_lag_offset(lchip, p_hash_offset));
			  break;

          case CTC_LB_HASH_MODULE_STACKING:
          {
              if (p_hash_offset->fwd_type == CTC_LB_HASH_FWD_NON_UC)
              {
                  CTC_ERROR_RETURN(_sys_tsingma_hash_get_stacking_non_uc_offset(lchip, p_hash_offset));
              }
              else
              {
                  CTC_ERROR_RETURN(_sys_tsingma_hash_get_lag_offset(lchip, p_hash_offset));
              }
          }
              break;

          case CTC_LB_HASH_MODULE_ECMP:
              CTC_ERROR_RETURN(_sys_tsingma_hash_get_ecmp_offset(lchip, p_hash_offset));
              break;

          case CTC_LB_HASH_MODULE_HEAD_LAG:
          case CTC_LB_HASH_MODULE_HEAD_ECMP:
              CTC_ERROR_RETURN(_sys_tsingma_hash_get_packet_head_offset(lchip, p_hash_offset));
              break;

          default:
              return CTC_E_INVALID_PARAM;
      }

      return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_parser_set_layer2_hash(uint8 lchip,
                                    ctc_parser_hash_ctl_t* p_psc,
                                    sys_parser_hash_usage_t hash_usage)
{
    uint32 value = 0;
	ctc_lb_hash_config_t hash_cfg;

    sal_memset(&hash_cfg, 0, sizeof(hash_cfg));

    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
    hash_cfg.sel_id = hash_usage;

    value = CTC_FLAG_ISSET(p_psc->l2_flag, CTC_PARSER_L2_HASH_FLAGS_STAG_VID);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_SVLAN : 0));

	value = CTC_FLAG_ISSET(p_psc->l2_flag, CTC_PARSER_L2_HASH_FLAGS_CTAG_VID);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_CVLAN : 0));

    value = CTC_FLAG_ISSET(p_psc->l2_flag , CTC_PARSER_L2_HASH_FLAGS_ETHERTYPE);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_ETHER_TYPE : 0));

    value = CTC_FLAG_ISSET(p_psc->l2_flag, CTC_PARSER_L2_HASH_FLAGS_MACSA);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_MACSA_LO : 0));
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_MACSA_MI : 0));
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_MACSA_HI : 0));

    value = CTC_FLAG_ISSET(p_psc->l2_flag,  CTC_PARSER_L2_HASH_FLAGS_MACDA);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_MACDA_LO : 0));
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_MACDA_MI : 0));
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_MACDA_HI : 0));

    value = CTC_FLAG_ISSET(p_psc->l2_flag,  CTC_PARSER_L2_HASH_FLAGS_PORT);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_SRC_PORT : 0));


    hash_cfg.hash_select = CTC_LB_HASH_SELECT_L2;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
    hash_cfg.hash_select = CTC_LB_HASH_SELECT_NVGRE_INNER_L2;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
	hash_cfg.hash_select = CTC_LB_HASH_SELECT_VXLAN_INNER_L2;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
    hash_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_L2;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
	hash_cfg.hash_select = CTC_LB_HASH_SELECT_TRILL_OUTER_L2_ONLY;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
    hash_cfg.hash_select = CTC_LB_HASH_SELECT_TRILL;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
	hash_cfg.hash_select = CTC_LB_HASH_SELECT_TRILL_INNER_L2;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
    hash_cfg.hash_select = CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_L2;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_parser_get_layer2_hash(uint8 lchip,
                                    ctc_parser_hash_ctl_t* p_psc,
                                    sys_parser_hash_usage_t hash_usage)
{
    uint32 value = 0;
	ctc_lb_hash_config_t hash_cfg;

    sal_memset(&hash_cfg, 0, sizeof(hash_cfg));
	hash_cfg.sel_id = hash_usage;

    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;

    hash_cfg.hash_select = CTC_LB_HASH_SELECT_L2;
    CTC_ERROR_RETURN(sys_tsingma_hash_get_config(lchip, &hash_cfg));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_SVLAN);
    CTC_SET_FLAG(p_psc->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_STAG_VID : 0));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_CVLAN);
    CTC_SET_FLAG(p_psc->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_CTAG_VID : 0));

    value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_ETHER_TYPE);
    CTC_SET_FLAG(p_psc->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_ETHERTYPE : 0));

    value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_MACSA_LO);
	value &= CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_MACSA_MI);
	value &= CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_MACSA_HI);
    CTC_SET_FLAG(p_psc->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_MACSA : 0));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_MACDA_LO);
	value &= CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_MACDA_MI);
	value &= CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_MACDA_HI);
    CTC_SET_FLAG(p_psc->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_MACDA : 0));

    value = CTC_FLAG_ISSET(hash_cfg.value,  CTC_LB_HASH_FIELD_SRC_PORT);
    CTC_SET_FLAG(p_psc->l2_flag, (value ? CTC_PARSER_L2_HASH_FLAGS_PORT : 0));

    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_parser_set_l3_l4_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_psc,
                                sys_parser_hash_usage_t hash_usage)
{
    uint32 value = 0;
    ctc_lb_hash_config_t hash_cfg;
    uint8 use_template_gre = 0;
    uint32 template_ip[] = {
        CTC_LB_HASH_SELECT_IPV4,
        CTC_LB_HASH_SELECT_IPV4_TCP_UDP,
        CTC_LB_HASH_SELECT_IPV4_TCP_UDP_PORTS_EQUAL,
        CTC_LB_HASH_SELECT_IPV4_VXLAN,
        CTC_LB_HASH_SELECT_IPV6,
        CTC_LB_HASH_SELECT_IPV6_TCP_UDP,
        CTC_LB_HASH_SELECT_IPV6_TCP_UDP_PORTS_EQUAL,
        CTC_LB_HASH_SELECT_IPV6_VXLAN,
        CTC_LB_HASH_SELECT_NVGRE_INNER_IPV4,
        CTC_LB_HASH_SELECT_NVGRE_INNER_IPV6,
        CTC_LB_HASH_SELECT_VXLAN_INNER_IPV4,
        CTC_LB_HASH_SELECT_VXLAN_INNER_IPV6,
        CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV4,
        CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV6,
        CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV4,
        CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV6,
        CTC_LB_HASH_SELECT_TRILL_INNER_IPV4,
        CTC_LB_HASH_SELECT_TRILL_INNER_IPV6,
        CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_IPV4,
        CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_IPV6
        };

    sal_memset(&hash_cfg, 0, sizeof(hash_cfg));

    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
	hash_cfg.sel_id = hash_usage;

    if (CTC_FLAG_ISSET(p_psc->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_IP))
    {
        value = CTC_FLAG_ISSET(p_psc->ip_flag, CTC_PARSER_IP_HASH_FLAGS_PROTOCOL);
        CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_PROTOCOL : 0));

        value = CTC_FLAG_ISSET(p_psc->ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPV6_FLOW_LABEL);
        CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_FLOWLABEL_LO : 0));
        CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_FLOWLABEL_HI : 0));

        value = CTC_FLAG_ISSET(p_psc->ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPSA);
        CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_IP_SA_LO : 0));
        CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_IP_SA_HI : 0));

        value = CTC_FLAG_ISSET(p_psc->ip_flag , CTC_PARSER_IP_HASH_FLAGS_IPDA);
        CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_IP_DA_LO : 0));
        CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_IP_DA_HI : 0));
    }

    if (CTC_FLAG_ISSET(p_psc->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L4))
    {
        if (CTC_FLAG_ISSET(p_psc->l4_flag, CTC_PARSER_L4_HASH_FLAGS_GRE_KEY) ||
            CTC_FLAG_ISSET(p_psc->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_VSID))
        {
            value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_PARSER_L4_HASH_FLAGS_GRE_KEY);
            CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_GRE_KEY_LO : 0));
            CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_GRE_KEY_HI : 0));

            value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_VSID);
            CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_SVLAN : 0));

            use_template_gre = 1;
        }
        else
        {
            value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_PARSER_L4_HASH_FLAGS_SRC_PORT);
            CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_L4_SRCPORT : 0));

            value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_PARSER_L4_HASH_FLAGS_DST_PORT);
            CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_L4_DSTPORT : 0));

            value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_SRC_PORT);
            CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_L4_SRCPORT : 0));

            value = CTC_FLAG_ISSET(p_psc->l4_flag, CTC_PARSER_L4_HASH_FLAGS_L4_VXLAN_VNI);
            CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_SVLAN : 0));
        }
    }


    if (use_template_gre)
    {
        hash_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4_GRE;
        CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
        hash_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4_NVGRE;
        CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));

        hash_cfg.hash_select = CTC_LB_HASH_SELECT_IPV6_GRE;
        CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
        hash_cfg.hash_select = CTC_LB_HASH_SELECT_IPV6_NVGRE;
        CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));

    }
    else
    {
       uint8 i = 0;

        for (i = 0; i < sizeof(template_ip) / 4; i++)
        {
            hash_cfg.hash_select = i;
            CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));

        }
    }

	 return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_parser_get_l3_l4_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_psc,
                                sys_parser_hash_usage_t hash_usage)
{
    uint32 value = 0;
    ctc_lb_hash_config_t hash_cfg;

    uint32 bitmap = 0;
    sal_memset(&hash_cfg, 0, sizeof(hash_cfg));

    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
	hash_cfg.sel_id = hash_usage;


    hash_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4;
    CTC_ERROR_RETURN(sys_tsingma_hash_get_config(lchip, &hash_cfg));
    bitmap = hash_cfg.value;

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_L4_SRCPORT);
    CTC_SET_FLAG(p_psc->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_SRC_PORT : 0));

    value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_L4_DSTPORT);
    CTC_SET_FLAG(p_psc->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_DST_PORT : 0));

    value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_L4_SRCPORT);
    CTC_SET_FLAG(p_psc->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_VXLAN_L4_SRC_PORT : 0));

    value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_SVLAN);
    CTC_SET_FLAG(p_psc->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_VXLAN_VNI : 0));


    /**<gre,nvgre packets*/
    hash_cfg.hash_select = CTC_LB_HASH_SELECT_IPV4_GRE;
    CTC_ERROR_RETURN(sys_tsingma_hash_get_config(lchip, &hash_cfg));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_GRE_KEY_LO);
	value &= CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_GRE_KEY_HI);
    CTC_SET_FLAG(p_psc->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_GRE_KEY : 0));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_SVLAN);
    CTC_SET_FLAG(p_psc->l4_flag, (value ? CTC_PARSER_L4_HASH_FLAGS_L4_NVGRE_VSID : 0));

   /* IPv4/IPv6*/
    hash_cfg.value |= bitmap;

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_PROTOCOL);
    CTC_SET_FLAG(p_psc->ip_flag, (value ? CTC_PARSER_IP_HASH_FLAGS_PROTOCOL : 0));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_FLOWLABEL_LO);
    CTC_SET_FLAG(p_psc->ip_flag, (value ? CTC_PARSER_IP_HASH_FLAGS_IPV6_FLOW_LABEL : 0));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
    CTC_SET_FLAG(p_psc->ip_flag, (value ? CTC_PARSER_IP_HASH_FLAGS_IPSA : 0));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
    CTC_SET_FLAG(p_psc->ip_flag, (value ? CTC_PARSER_IP_HASH_FLAGS_IPDA : 0));

	return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_parser_set_mpls_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_psc, sys_parser_hash_usage_t hash_usage)
{
    uint32 value = 0;
    ctc_lb_hash_config_t hash_cfg;

    sal_memset(&hash_cfg, 0, sizeof(hash_cfg));
    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
	hash_cfg.sel_id = hash_usage;
    /**<mpls packets*/

    value = CTC_FLAG_ISSET(p_psc->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_PROTOCOL);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_PROTOCOL : 0));

	value = CTC_FLAG_ISSET(p_psc->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPV6_FLOW_LABEL);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_FLOWLABEL_LO : 0));
	CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_FLOWLABEL_HI : 0));

    value = CTC_FLAG_ISSET(p_psc->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPSA);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_IP_SA_LO : 0));
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_IP_SA_HI : 0));

    value = CTC_FLAG_ISSET(p_psc->mpls_flag, CTC_PARSER_MPLS_HASH_FLAGS_IPDA);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_IP_DA_LO : 0));
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_IP_DA_HI : 0));

    hash_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
    hash_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_INNER_IPV4;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));

	hash_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_INNER_IPV6;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
    hash_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_VPWS_RAW;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
    hash_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV4;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));

	hash_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV6;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
    hash_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV4;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
	hash_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV6;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_parser_get_mpls_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_psc, sys_parser_hash_usage_t hash_usage)
{
    uint32 value = 0;
    ctc_lb_hash_config_t hash_cfg;

    sal_memset(&hash_cfg, 0, sizeof(hash_cfg));
    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
	hash_cfg.sel_id = hash_usage;
    /**<mpls packets*/

	hash_cfg.hash_select = CTC_LB_HASH_SELECT_MPLS;
    CTC_ERROR_RETURN(sys_tsingma_hash_get_config(lchip, &hash_cfg));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_PROTOCOL);
    CTC_SET_FLAG(p_psc->mpls_flag, (value ? CTC_PARSER_MPLS_HASH_FLAGS_PROTOCOL : 0));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_FLOWLABEL_LO);
	value &= CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_FLOWLABEL_HI);
    CTC_SET_FLAG(p_psc->mpls_flag, (value ? CTC_PARSER_MPLS_HASH_FLAGS_IPV6_FLOW_LABEL : 0));

    value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_IP_SA_LO);
    value &= CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_IP_SA_HI);
    CTC_SET_FLAG(p_psc->mpls_flag, (value ? CTC_PARSER_MPLS_HASH_FLAGS_IPSA : 0));

    value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_IP_DA_LO);
	value &= CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_IP_DA_HI);
    CTC_SET_FLAG(p_psc->mpls_flag, (value ? CTC_PARSER_MPLS_HASH_FLAGS_IPDA : 0));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_parser_set_trill_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_psc, sys_parser_hash_usage_t hash_usage)
{
    uint32 value = 0;
    ctc_lb_hash_config_t hash_cfg;

    sal_memset(&hash_cfg, 0, sizeof(hash_cfg));
    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
	hash_cfg.sel_id = hash_usage;
    /**<TRILL packets*/

    value = CTC_FLAG_ISSET(p_psc->trill_flag, CTC_PARSER_TRILL_HASH_FLAGS_INNER_VLAN_ID);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_SVLAN : 0));

    value = CTC_FLAG_ISSET(p_psc->trill_flag, CTC_PARSER_TRILL_HASH_FLAGS_INGRESS_NICKNAME);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_INGRESS_NICKNAME : 0));

    value = CTC_FLAG_ISSET(p_psc->trill_flag, CTC_PARSER_TRILL_HASH_FLAGS_EGRESS_NICKNAME);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_EGRESS_NICKNAME : 0));


    hash_cfg.hash_select = CTC_LB_HASH_SELECT_TRILL;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));

	hash_cfg.hash_select = CTC_LB_HASH_SELECT_TRILL_INNER_L2;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));
	 return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_parser_get_trill_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_psc, sys_parser_hash_usage_t hash_usage)
{
    uint32 value = 0;
    ctc_lb_hash_config_t hash_cfg;

    sal_memset(&hash_cfg, 0, sizeof(hash_cfg));
    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
	hash_cfg.sel_id = hash_usage;
    /**<TRILL packets*/

	hash_cfg.hash_select = CTC_LB_HASH_SELECT_TRILL;
    CTC_ERROR_RETURN(sys_tsingma_hash_get_config(lchip, &hash_cfg));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_SVLAN);
    CTC_SET_FLAG(p_psc->trill_flag, (value ? CTC_PARSER_TRILL_HASH_FLAGS_INNER_VLAN_ID : 0));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_INGRESS_NICKNAME);
    CTC_SET_FLAG(p_psc->trill_flag, (value ? CTC_PARSER_TRILL_HASH_FLAGS_INGRESS_NICKNAME : 0));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_EGRESS_NICKNAME);
    CTC_SET_FLAG(p_psc->trill_flag, (value ? CTC_PARSER_TRILL_HASH_FLAGS_EGRESS_NICKNAME : 0));
	 return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_parser_set_fcoe_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_psc,
                                  sys_parser_hash_usage_t hash_usage)
{
    uint32 value = 0;
    ctc_lb_hash_config_t hash_cfg;

    sal_memset(&hash_cfg, 0, sizeof(hash_cfg));

    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
	hash_cfg.sel_id = hash_usage;
    /**<FCOE packets*/

    value = CTC_FLAG_ISSET(p_psc->fcoe_flag, CTC_PARSER_FCOE_HASH_FLAGS_SID);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_FCOE_SID_LO : 0));
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_FCOE_SID_HI : 0));


    value = CTC_FLAG_ISSET(p_psc->fcoe_flag, CTC_PARSER_FCOE_HASH_FLAGS_DID);
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_FCOE_DID_LO : 0));
    CTC_SET_FLAG(hash_cfg.value, (value ? CTC_LB_HASH_FIELD_FCOE_DID_HI : 0));

    hash_cfg.hash_select = CTC_LB_HASH_SELECT_FCOE;
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_parser_get_fcoe_hash(uint8 lchip, ctc_parser_hash_ctl_t* p_psc,
                                  sys_parser_hash_usage_t hash_usage)
{
    uint32 value = 0;
    ctc_lb_hash_config_t hash_cfg;

    sal_memset(&hash_cfg, 0, sizeof(hash_cfg));

    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
	hash_cfg.sel_id = hash_usage;
    /**<FCOE packets*/

    hash_cfg.hash_select = CTC_LB_HASH_SELECT_FCOE;
    CTC_ERROR_RETURN(sys_tsingma_hash_get_config(lchip, &hash_cfg));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_FCOE_SID_LO);
    value &= CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_FCOE_SID_HI);
    CTC_SET_FLAG(p_psc->fcoe_flag, (value ? CTC_PARSER_FCOE_HASH_FLAGS_SID : 0));

	value = CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_FCOE_DID_LO);
    value &= CTC_FLAG_ISSET(hash_cfg.value, CTC_LB_HASH_FIELD_FCOE_DID_HI);
    CTC_SET_FLAG(p_psc->fcoe_flag, (value ? CTC_PARSER_FCOE_HASH_FLAGS_DID : 0));
    return CTC_E_NONE;
}




int32
sys_tsingma_parser_set_hash_field(uint8 lchip,
                                  ctc_parser_hash_ctl_t* p_hash_ctl,
                                  sys_parser_hash_usage_t hash_usage)
{
    ctc_lb_hash_config_t hash_cfg;
    sal_memset(&hash_cfg, 0, sizeof(hash_cfg));

    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:%d\n", p_hash_ctl->hash_type_bitmap);

    CTC_PTR_VALID_CHECK(p_hash_ctl);

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L2))
    {
        CTC_ERROR_RETURN(_sys_tsingma_parser_set_layer2_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_IP) ||
        CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L4) )
    {
        CTC_ERROR_RETURN(_sys_tsingma_parser_set_l3_l4_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_MPLS))
    {
        CTC_ERROR_RETURN(_sys_tsingma_parser_set_mpls_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_TRILL))
    {
        CTC_ERROR_RETURN(_sys_tsingma_parser_set_trill_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_FCOE))
    {
        CTC_ERROR_RETURN(_sys_tsingma_parser_set_fcoe_hash(lchip, p_hash_ctl, hash_usage));
    }

    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    hash_cfg.sel_id = hash_usage;
    hash_cfg.hash_control = CTC_LB_HASH_CONTROL_L2_ONLY;


    if (p_hash_ctl->hash_type_bitmap == CTC_PARSER_HASH_TYPE_FLAGS_L2 ||
        p_hash_ctl->hash_type_bitmap == (CTC_PARSER_HASH_TYPE_FLAGS_L2 | CTC_PARSER_HASH_TYPE_FLAGS_INNER) ||
    p_hash_ctl->hash_type_bitmap == (CTC_PARSER_HASH_TYPE_FLAGS_L2 | CTC_PARSER_HASH_TYPE_FLAGS_DLB_EFD) ||
    p_hash_ctl->hash_type_bitmap == (CTC_PARSER_HASH_TYPE_FLAGS_L2 | CTC_PARSER_HASH_TYPE_FLAGS_DLB))
    {
        hash_cfg.value = 1;
    }
    else
    {
        hash_cfg.value = 0;
    }
    CTC_ERROR_RETURN(sys_tsingma_hash_set_config(lchip, &hash_cfg));


    return CTC_E_NONE;
}

int32
sys_tsingma_parser_get_hash_field(uint8 lchip,
                                  ctc_parser_hash_ctl_t* p_hash_ctl,
                                  sys_parser_hash_usage_t hash_usage)
{
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PARSER_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "hash_type_bitmap:%d\n", p_hash_ctl->hash_type_bitmap);

    CTC_PTR_VALID_CHECK(p_hash_ctl);

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L2))
    {
        CTC_ERROR_RETURN(_sys_tsingma_parser_get_layer2_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_IP) ||
        CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_L4) )
    {
        CTC_ERROR_RETURN(_sys_tsingma_parser_get_l3_l4_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_MPLS))
    {
        CTC_ERROR_RETURN(_sys_tsingma_parser_get_mpls_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_TRILL))
    {
        CTC_ERROR_RETURN(_sys_tsingma_parser_get_trill_hash(lchip, p_hash_ctl, hash_usage));
    }

    if (CTC_FLAG_ISSET(p_hash_ctl->hash_type_bitmap, CTC_PARSER_HASH_TYPE_FLAGS_FCOE))
    {
        CTC_ERROR_RETURN(_sys_tsingma_parser_get_fcoe_hash(lchip, p_hash_ctl, hash_usage));
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_lb_hash_show_hash_cfg(uint8 lchip,ctc_lb_hash_config_t hash_cfg)
{
    uint16 hash_select = 0;
	uint16 hash_control = 0;
    uint8 tmplate = 0;
	ctc_lb_hash_config_t hash_cfg1;
    char* hash_sel_str[] = {SYS_LB_HASH_SEL_STR};
	char* hash_ctl_str[] = {SYS_LB_HASH_CTL_STR};
	sal_memset(&hash_cfg1,0,sizeof(ctc_lb_hash_config_t));
	hash_cfg1.sel_id = hash_cfg.sel_id;
	hash_cfg1.cfg_type= hash_cfg.cfg_type;

     /*if (DRV_IS_DUET2(lchip))*/
    {
      /*   return CTC_E_NOT_SUPPORT;*/
    }

	/*show hash select tempmlate*/

	/*show hash select value*/
	if (hash_cfg1.cfg_type == CTC_LB_HASH_CFG_HASH_SELECT)
    {
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nNo. %-30s%-15s%-20s\n", "hash-select", "value", "template");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s\n", "======================================================");
        for (hash_select = CTC_LB_HASH_SELECT_L2; hash_select < CTC_LB_HASH_SELECT_MAX; hash_select++)
        {
            switch(hash_select)
            {
            case CTC_LB_HASH_SELECT_L2:
            case CTC_LB_HASH_SELECT_NVGRE_INNER_L2:
            case CTC_LB_HASH_SELECT_VXLAN_INNER_L2:
            case CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_L2:
            case CTC_LB_HASH_SELECT_TRILL_OUTER_L2_ONLY:
            case CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_L2:
            case CTC_LB_HASH_SELECT_FLEX_TNL_INNER_L2:
                tmplate = 0;
                break;
            case CTC_LB_HASH_SELECT_TRILL:
            case CTC_LB_HASH_SELECT_TRILL_INNER_L2:
                tmplate = 1;
                break;
            case CTC_LB_HASH_SELECT_FCOE:
                tmplate = 2;
                break;
            case CTC_LB_HASH_SELECT_MPLS:
            case CTC_LB_HASH_SELECT_MPLS_INNER_IPV4:
            case CTC_LB_HASH_SELECT_MPLS_INNER_IPV6:
            case CTC_LB_HASH_SELECT_MPLS_VPWS_RAW:
                tmplate = 3;
                break;
            case CTC_LB_HASH_SELECT_IPV4:
            case CTC_LB_HASH_SELECT_IPV4_TCP_UDP:
            case CTC_LB_HASH_SELECT_IPV4_TCP_UDP_PORTS_EQUAL:
            case CTC_LB_HASH_SELECT_IPV4_VXLAN:
            case CTC_LB_HASH_SELECT_IPV6:
            case CTC_LB_HASH_SELECT_IPV6_TCP_UDP:
            case CTC_LB_HASH_SELECT_IPV6_TCP_UDP_PORTS_EQUAL:
            case CTC_LB_HASH_SELECT_IPV6_VXLAN:
            case CTC_LB_HASH_SELECT_NVGRE_INNER_IPV4:
            case CTC_LB_HASH_SELECT_NVGRE_INNER_IPV6:
            case CTC_LB_HASH_SELECT_VXLAN_INNER_IPV4:
            case CTC_LB_HASH_SELECT_VXLAN_INNER_IPV6:
            case CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV4:
            case CTC_LB_HASH_SELECT_MPLS_L2VPN_INNER_IPV6:
            case CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV4:
            case CTC_LB_HASH_SELECT_MPLS_L3VPN_INNER_IPV6:
            case CTC_LB_HASH_SELECT_TRILL_INNER_IPV4:
            case CTC_LB_HASH_SELECT_TRILL_INNER_IPV6:
            case CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_IPV4:
            case CTC_LB_HASH_SELECT_TRILL_DECAP_INNER_IPV6:
            case CTC_LB_HASH_SELECT_FLEX_TNL_INNER_IPV4:
            case CTC_LB_HASH_SELECT_FLEX_TNL_INNER_IPV6:
                tmplate = 4;
                break;

            case CTC_LB_HASH_SELECT_IPV4_GRE:
            case CTC_LB_HASH_SELECT_IPV4_NVGRE:
            case CTC_LB_HASH_SELECT_IPV6_GRE:
            case CTC_LB_HASH_SELECT_IPV6_NVGRE:
                tmplate = 5;
                break;

            case CTC_LB_HASH_SELECT_UDF:
                tmplate = 6;
                break;

            }
            hash_cfg1.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;
            hash_cfg1.hash_select = hash_select;
            CTC_ERROR_RETURN(sys_tsingma_hash_get_config(lchip, &hash_cfg1));
            SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-3d %-30s 0x%04x          %d\n", hash_select, hash_sel_str[hash_select], hash_cfg1.value, tmplate);
        }
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s\n", "======================================================");
    }
    /*show hash control value*/
    if (hash_cfg1.cfg_type == CTC_LB_HASH_CFG_HASH_CONTROL)
    {
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nNo. %-28s %-20s\n", "hash-control", "value");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s\n", "========================================");
        for (hash_select = CTC_LB_HASH_CONTROL_L2_ONLY; hash_control < CTC_LB_HASH_CONTROL_MAX; hash_control++)
        {
            hash_cfg1.hash_control = hash_control;
            CTC_ERROR_RETURN(sys_tsingma_hash_get_config(lchip, &hash_cfg1));
            SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-3d %-30s:%u\n", hash_control, hash_ctl_str[hash_control], hash_cfg1.value);
        }
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s\n", "========================================");
    }
return 0;
}

int32
sys_tsingma_lb_hash_show_template(uint8 lchip,uint8 template_id)
{

    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }

    switch(template_id)
    {
    case 0:
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s:\n", "templet0");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "", "", "", "Cvid");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "Svid", "EtherType", "MacDaHig", "MacDamid");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "MacDaLow", "MacSaHig", "MacSamid", "MacSaLow");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "DstChipId", "Dstlocalport", "Srcchipid", "Srclocalport");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        break;
    case 1:
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s:\n", "templet1");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "", "", "EgressNickname", "IngressNickname");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "Svid", "EtherType", "MacDaHig", "MacDamid");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "MacDaLow", "MacSaHig", "MacSamid", "MacSaLow");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "DstChipId", "Dstlocalport", "Srcchipid", "Srclocalport");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        break;
    case 2:

        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s:\n", "templet2");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "", "", "", "");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "", "", "UdfHigh", "UdfLow");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "FcoeDidHigh", "FcoeDidLow", "FcoeSidHigh", "FcoeSidLow");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "DstChipId", "Dstlocalport", "Srcchipid", "Srclocalport");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        break;
    case 3:
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s:\n", "templet3");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "", "", "MplsLabel2High", "MplsLabel1High");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "IpDaHigh", "IpDaLow", "IpSaHigh", "IpSaLow");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "MplsLabel0High", "MplsLabel2Low", "MplsLabel1Low", "MplsLabel0Low");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "DstChipId", "Dstlocalport", "Srcchipid", "Srclocalport");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        break;
    case 4:
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s:\n", "templet4");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "", "", "Udf/Ipv6FlowLabel", "Udf/Ipv6Label");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "Svid", "L4Info2", "L4Info1", "L4Protocol");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "IpDaHigh", "LpDaLow", "IpSaHig", "IpSaLow");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "DstChipId", "Dstlocalport", "Srcchipid", "Srclocalport");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        break;
    case 5:
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s:\n", "templet5");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "", "", "UdfHigh", "UdfLow");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "Svid", "GreKeyHigh", "GreKeyLow", "GreProType");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "IpDaHigh", "IpDaLow", "IpSaHigh", "IpSaLow");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "DstChipId", "Dstlocalport", "Srcchipid", "Srclocalport");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        break;

    case 6:
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s:\n", "templet6");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "", "", "", "");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "Udf[127:112]", "Udf[111:96]", "Udf[95:80]", "Udf[79:64]");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "Udf[63:48]", "Udf[47:32]", "Udf[31:16]", "Udf[15:0]");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "|%15s|%15s|%15s|%15s|\n", "DstChipId", "Dstlocalport", "Srcchipid", "Srclocalport");
        SYS_LB_HASH_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\n");
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}

int32
_sys_tsingma_hash_init(uint8 lchip)
{
    IpeHashCtl0_m hash_ctl;
    uint32 cmd = 0;
    uint8 i = 0;

    for (i = 0; i < 4; i++)
    {
        sal_memset(&hash_ctl, 0, sizeof(hash_ctl));
        cmd = DRV_IOR(IpeHashCtl0_t + i, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

        SetIpeHashCtl0(V, forceL2VpnMplsHashEn_f, &hash_ctl, 0);
        SetIpeHashCtl0(V, forceL3VpnMplsHashEn_f, &hash_ctl, 0);
        SetIpeHashCtl0(V, forceMplsHashEn_f, &hash_ctl, 1);
        SetIpeHashCtl0(V, hashSeedMask_f, &hash_ctl, 0x3);
        SetIpeHashCtl0(V, trillTerminateNodeHashMode_f, &hash_ctl, 1);
        SetIpeHashCtl0(V, trillTransitNodeHashMode_f, &hash_ctl, 3);
        SetIpeHashCtl0(V, ipv4EnableInnerHashTunnelBitmap_f, &hash_ctl, 0x3F);
        SetIpeHashCtl0(V, ipv6EnableInnerHashTunnelBitmap_f, &hash_ctl, 0x3F);
        SetIpeHashCtl0(V, transitNodeV4TunnelFormatPktHashMode_f, &hash_ctl, 0x3F);
        SetIpeHashCtl0(V, transitNodeV6TunnelFormatPktHashMode_f, &hash_ctl, 0x3F);
        SetIpeHashCtl0(V, ipv4NvgreInnerL3Bulk9And10UseVsiId_f, &hash_ctl, 1);
        SetIpeHashCtl0(V, ipv4VxlanInnerL3Bulk12Andl3UseVni_f, &hash_ctl, 1);
        SetIpeHashCtl0(V, ipv6NvgreInnerL3Bulk9And10UseVsiId_f, &hash_ctl, 1);
        SetIpeHashCtl0(V, ipv6VxlanInnerL3Bulk12Andl3UseVni_f, &hash_ctl, 1);
        SetIpeHashCtl0(V, mplsReservedLabel_f, &hash_ctl, 15);
        SetIpeHashCtl0(V, symmetricHashIpv4VxlanMaskL4Info_f, &hash_ctl, 1);
        SetIpeHashCtl0(V, symmetricHashIpv6VxlanMaskL4Info_f, &hash_ctl, 1);
        SetIpeHashCtl0(V, v4VxlanOuterL3HashUdfFieldUseVni_f, &hash_ctl, 1);
        SetIpeHashCtl0(V, v6VxlanOuterL3HashUdfFieldUseVni_f, &hash_ctl, 1);
        SetIpeHashCtl0(V, vxlanInnerL2HashUseVni_f, &hash_ctl, 1);
        SetIpeHashCtl0(V, nvgreInnerL2HashUseVsid_f, &hash_ctl, 1);
        cmd = DRV_IOW(IpeHashCtl0_t + i, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_ctl));

    }

    return CTC_E_NONE;
}

int32
sys_tsingma_parser_hash_init(uint8 lchip)
{
    uint8 hash_usage = 0;
	ctc_parser_hash_ctl_t hash_ctl;
	ctc_lb_hash_config_t hash_cfg;
	ctc_lb_hash_offset_t p_hash_offset;

    CTC_ERROR_RETURN(_sys_tsingma_hash_init(lchip));

    sal_memset(&hash_ctl, 0, sizeof(ctc_parser_hash_ctl_t));
	sal_memset(&p_hash_offset, 0, sizeof(ctc_lb_hash_offset_t));
    CTC_ERROR_RETURN(_sys_tsingma_parser_set_layer2_hash(lchip, &hash_ctl, hash_usage));
	CTC_ERROR_RETURN(_sys_tsingma_parser_set_l3_l4_hash(lchip, &hash_ctl, hash_usage));
	CTC_ERROR_RETURN(_sys_tsingma_parser_set_mpls_hash(lchip, &hash_ctl, hash_usage));
	CTC_ERROR_RETURN(_sys_tsingma_parser_set_trill_hash(lchip, &hash_ctl, hash_usage));
	CTC_ERROR_RETURN(_sys_tsingma_parser_set_fcoe_hash(lchip, &hash_ctl, hash_usage));

	CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPSA);
    CTC_SET_FLAG(hash_ctl.ip_flag, CTC_PARSER_IP_HASH_FLAGS_IPDA);
	CTC_ERROR_RETURN(_sys_tsingma_parser_set_l3_l4_hash(lchip, &hash_ctl, hash_usage));

    sal_memset(&hash_cfg, 0, sizeof(ctc_lb_hash_config_t));
	hash_cfg.sel_id = 1;
    hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;
    hash_cfg.hash_control = CTC_LB_HASH_CONTROL_HASH_TYPE_A;
    hash_cfg.value = SYS_LB_HASHPOLY_XOR8;
	CTC_ERROR_RETURN(_sys_tsingma_hash_config_set_hash_control(lchip,&hash_cfg));
	hash_cfg.sel_id = 0;
	CTC_ERROR_RETURN(_sys_tsingma_hash_config_set_hash_control(lchip,&hash_cfg));

    /*static linkagg*/
	p_hash_offset.hash_module = CTC_LB_HASH_MODULE_LINKAGG;
	p_hash_offset.hash_mode = CTC_LB_HASH_MODE_STATIC;
	p_hash_offset.offset = 32;
	CTC_ERROR_RETURN(_sys_tsingma_hash_set_lag_offset(lchip, &p_hash_offset));

    /*dlb linkagg*/
	p_hash_offset.hash_module = CTC_LB_HASH_MODULE_LINKAGG;
	p_hash_offset.hash_mode = CTC_LB_HASH_MODE_DLB_FLOW_SET;
	p_hash_offset.offset = 32;
	CTC_ERROR_RETURN(_sys_tsingma_hash_set_lag_offset(lchip, &p_hash_offset));

    /*resilent linkagg*/
	p_hash_offset.hash_module = CTC_LB_HASH_MODULE_LINKAGG;
	p_hash_offset.hash_mode = CTC_LB_HASH_MODE_RESILIENT;
	p_hash_offset.offset = 32;
	CTC_ERROR_RETURN(_sys_tsingma_hash_set_lag_offset(lchip, &p_hash_offset));

    /*static stacking*/
	p_hash_offset.hash_module = CTC_LB_HASH_MODULE_STACKING;
	p_hash_offset.hash_mode = CTC_LB_HASH_MODE_STATIC;
	p_hash_offset.offset = 32;
	CTC_ERROR_RETURN(_sys_tsingma_hash_set_lag_offset(lchip, &p_hash_offset));

    /*dlb stacking*/
	p_hash_offset.hash_module = CTC_LB_HASH_MODULE_STACKING;
	p_hash_offset.hash_mode = CTC_LB_HASH_MODE_DLB_FLOW_SET;
	p_hash_offset.offset = 32;
	CTC_ERROR_RETURN(_sys_tsingma_hash_set_lag_offset(lchip, &p_hash_offset));

    /*resilent stacking*/
	p_hash_offset.hash_module = CTC_LB_HASH_MODULE_STACKING;
	p_hash_offset.hash_mode = CTC_LB_HASH_MODE_RESILIENT;
	p_hash_offset.offset = 32;
	CTC_ERROR_RETURN(_sys_tsingma_hash_set_lag_offset(lchip, &p_hash_offset));

    /*UC for header*/
	p_hash_offset.hash_module = CTC_LB_HASH_MODULE_HEAD_LAG;
	p_hash_offset.fwd_type = CTC_LB_HASH_FWD_UC;
	p_hash_offset.offset = 32;
	CTC_ERROR_RETURN(_sys_tsingma_hash_set_packet_head_offset(lchip, &p_hash_offset));

	/*Non-UC for header*/
    p_hash_offset.hash_module = CTC_LB_HASH_MODULE_HEAD_LAG;
	p_hash_offset.fwd_type = CTC_LB_HASH_FWD_NON_UC;
	p_hash_offset.offset = 32;
	CTC_ERROR_RETURN(_sys_tsingma_hash_set_packet_head_offset(lchip, &p_hash_offset));
    return CTC_E_NONE;

}



