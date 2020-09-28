/**
 @file sys_usw_common.h

 @date 2010-3-10

 @version v2.0

*/

#ifndef _SYS_USW_COMMON_H
#define _SYS_USW_COMMON_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_const.h"
#include "sys_usw_chip.h"
#include "sys_usw_mchip.h"

#define SYS_COMMON_USELESS_MAC  0xffff
#define SYS_MAX_LOCAL_SLICE_NUM       1
#define SYS_PHY_PORT_NUM_PER_SLICE   64  /*+ max local phyport per slice*/

#define SYS_USW_MAX_PHY_PORT              64

#define SYS_USW_LB_HASH_CHK_OFFSET(lb_hash_offset) \
    do {\
        if (lb_hash_offset != CTC_LB_HASH_OFFSET_DISABLE)\
        {\
            CTC_MAX_VALUE_CHECK(lb_hash_offset, 127);\
            if (lb_hash_offset % 16) {\
                CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);\
            }\
        }\
    }while(0)

#define SYS_DUMP_DB_LOG(p_file, fmt, arg...) \
  do {                                    \
    if(NULL == p_file)                    \
    {                                     \
        sal_printf(fmt, ##arg);          \
    }                                     \
    else                                  \
    {                                     \
        sal_fprintf(p_file, fmt, ##arg); \
    }                                     \
  } while (0)
/*--------------------------------------------------------------------------------
 *               channel allocation
 *--------------------------------------------------------------------------------*/

/* define network port chanid */
#define GMAC_MIN_CHANID              0


/*--------------------------------------------------------------------------------
 *               localphy port allocation
 *--------------------------------------------------------------------------------
 * network port: 0~63,  internal port: 64~231, rsv port: 232~255,
 */

enum sys_local_port_alloc_e
{
	/*netwrok  port*/
    SYS_NETWORK_PORT_START           = 0,
    SYS_NETWORK_PORT_END             = 63,

	/*internal  port*/
    SYS_INTERNAL_PORT_START          = 64,
    SYS_INTERNAL_PORT_END            = 231,

	/*reserve port*/
    SYS_RSV_PORT_START               = 232 ,
    SYS_RSV_PORT_WLAN_ENCAP          = SYS_RSV_PORT_START,
    SYS_RSV_PORT_WLAN_E2ILOOP,       /*233*/
    SYS_RSV_PORT_CAWAP_DECRYPT0_ID,  /*234*/
    SYS_RSV_PORT_CAWAP_DECRYPT1_ID,  /*235*/
    SYS_RSV_PORT_CAWAP_DECRYPT2_ID,  /*236*/
    SYS_RSV_PORT_CAWAP_DECRYPT3_ID,  /*237*/
    SYS_RSV_PORT_CAWAP_ENCRYPT0_ID,  /*238*/
    SYS_RSV_PORT_CAWAP_ENCRYPT1_ID,  /*239*/
    SYS_RSV_PORT_CAWAP_ENCRYPT2_ID,  /*240*/
    SYS_RSV_PORT_CAWAP_ENCRYPT3_ID,  /*241*/
    SYS_RSV_PORT_CAWAP_REASSEMBLE_ID,/*242*/
    SYS_RSV_PORT_MAC_ENCRYPT_ID,     /*243*/
    SYS_RSV_PORT_MAC_DECRYPT_ID,     /*244*/

    SYS_RSV_PORT_DROP_ID             = 248 ,
    SYS_RSV_PORT_OAM_CPU_ID,         /*249*/
    SYS_RSV_PORT_ELOOP_ID,           /*250*/
    SYS_RSV_PORT_IP_TUNNEL           = SYS_RSV_PORT_ELOOP_ID,
    SYS_RSV_PORT_MIRROR,             /*251*/
    SYS_RSV_PORT_ILOOP_ID,           /*252*/
    SYS_RSV_PORT_SPINE_LEAF_PORT     = 255,
    SYS_RSV_PORT_END                 = SYS_RSV_PORT_SPINE_LEAF_PORT
};
typedef enum sys_local_port_alloc_e sys_local_port_alloc_t;





/*************************DestMap Encode*************************/
/*ucast:mc(1bit)+useProfile(1)+tocpu(1)+dest_chipid(7)+destid(9)*/
/*mcast:mc(1bit)+rsv(2)+mc_grpid(16)*/

#define SYS_ENCODE_DESTMAP( gchip,lport)        ((((gchip)&0x7F) << 9) | ((lport)&0x1FF))
#define SYS_DECODE_DESTMAP_GCHIP(destmap)       (((destmap)&0xFFFF)>>9)
#define SYS_ENCODE_ARP_DESTMAP( destmap_profile)   ((0<<18)|(1<<17)|((destmap_profile)&0x1FF))

#define SYS_ENCODE_APS_DESTMAP( gchip,lport)        ((((gchip)&0x7F) << 12) | ((lport)&0xFFF))

#define SYS_CHIPID_RESERVED_ECMP (0X7f)
#define SYS_ENCODE_ECMP_DESTMAP( group_id)       (((group_id)>>9 & 0x01)<<16 | SYS_CHIPID_RESERVED_ECMP<<9 | ((group_id)&0x1FF))

#define SYS_DECODE_ECMP_DESTMAP(destmap)         (((destmap) >> 16 & 0x01 << 9) | ((destmap) & 0x1FF) )
#define SYS_IS_ECMP_DESTMAP(destmap)             ((((destmap) >> 9 & 0x7F) ==  SYS_CHIPID_RESERVED_ECMP)? 1 : 0)
/*to lcpu(DMA)*/
#define SYS_ENCODE_EXCP_DESTMAP_MET(gchip, queueId) ((1 <<10) |((queueId)&0x1FF))
#define SYS_ENCODE_EXCP_DESTMAP(gchip, queueId) ((1 <<16) | (((gchip)&0x7F) << 9) | ((queueId)&0x1FF))

/*DestId(7) mean to cpu based on prio, destId(3,0) mean reason grp*/
#define SYS_ENCODE_EXCP_DESTMAP_GRP(gchip, grpId) ((1 <<16) | (((gchip)&0x7F) << 9) | (1 <<7)| ((grpId)&0xF))
#define SYS_ENCODE_EXCP_DESTMAP_MET_GRP(gchip, grpId) ((1 <<10) | (1 <<7) | ((grpId)&0xF))

#define SYS_DESTMAP_IS_CPU(destmap) ((((destmap) >> 16)&0x1) == 1)
/*to lcpu(ETH)*/

/*mirror to network port*/
#define SYS_ENCODE_MIRROR_DESTMAP(gchip, lport) ((((gchip)&0x7F) << 9) | ((lport)&0x1FF))

#define SYS_ENCODE_MCAST_IPE_DESTMAP(group) ((1<<18)|((group)&0xffff))

#define SYS_IS_DROP_PORT(gport)   (CTC_MAP_GPORT_TO_LPORT(gport) == SYS_RSV_PORT_DROP_ID)

/*qos domain/prio max value*/

enum sys_ad_union_g_type_e
{
    SYS_AD_UNION_G_NA   = 0,
    SYS_AD_UNION_G_0   = 1,
    SYS_AD_UNION_G_1   = 2,
    SYS_AD_UNION_G_2   = 3,
    SYS_AD_UNION_G_3   = 4,
    SYS_AD_UNION_G_4   = 5,
    SYS_AD_UNION_G_5   = 6,
    SYS_AD_UNION_G_6   = 7
};




#define FEATURE_SUPPORT_CHECK(feature) \
{\
    uint8 work_status = 0; \
        extern int32 sys_usw_ftm_get_working_status(uint8 lchip, uint8* p_status);\
    if (CTC_FEATURE_MAX <= feature) \
        return CTC_E_INVALID_PARAM; \
    if (FALSE == sys_usw_common_check_feature_support(feature)) \
        return CTC_E_NOT_SUPPORT;\
    if (lchip && sys_usw_chip_get_rchain_en() && !CTC_BMP_ISSET(g_rchain_en, feature) && g_ctcs_api_en){              \
            return CTC_E_NONE; }            \
    if((feature != CTC_FEATURE_REGISTER && feature != CTC_FEATURE_CHIP) && (0 == sys_usw_ftm_get_working_status(lchip, &work_status)) && (work_status > 0)){\
            return CTC_E_NOT_READY;}\
}

#define SYS_GLOBAL_CHIPID_CHECK(gchip)      \
    if ((gchip == 0x1F) || (gchip > MCHIP_CAP(SYS_CAP_GCHIP_CHIP_ID)))          \
    {                                       \
        return CTC_E_INVALID_CHIP_ID; \
    }

#define SYS_GLOBAL_PORT_CHECK(gport)                                                          \
{    \
    uint8 temp_lchip = 0;   \
     if ((CTC_IS_LINKAGG_PORT(gport) && ((gport & CTC_LOCAL_PORT_MASK) > SYS_USW_MAX_LINKAGG_ID))  \
        || (!CTC_IS_LINKAGG_PORT(gport) && (!CTC_IS_CPU_PORT(gport)) \
        && ((!sys_usw_get_local_chip_id(CTC_MAP_GPORT_TO_GCHIP(gport), &temp_lchip) && (CTC_MAP_GPORT_TO_LPORT(gport) >= MCHIP_CAP(SYS_CAP_PORT_NUM_PER_CHIP)))  \
        || (sys_usw_get_local_chip_id(CTC_MAP_GPORT_TO_GCHIP(gport), &temp_lchip) && (CTC_MAP_GPORT_TO_LPORT(gport) >= MCHIP_CAP(SYS_CAP_PORT_NUM_GLOBAL))))))  \
    {                                                                                         \
        return CTC_E_INVALID_PORT;                                                     \
    }   \
}

#define SYS_GPORT_CHECK_WITH_UNLOCK(gport, lock)                                                          \
{    \
    uint8 temp_lchip = 0;   \
     if ((CTC_IS_LINKAGG_PORT(gport) && ((gport & CTC_LOCAL_PORT_MASK) > SYS_USW_MAX_LINKAGG_ID))  \
        || (!CTC_IS_LINKAGG_PORT(gport) && (!CTC_IS_CPU_PORT(gport)) \
        && ((!sys_usw_get_local_chip_id(CTC_MAP_GPORT_TO_GCHIP(gport), &temp_lchip) && (CTC_MAP_GPORT_TO_LPORT(gport) >= MCHIP_CAP(SYS_CAP_PORT_NUM_PER_CHIP)))  \
        || (sys_usw_get_local_chip_id(CTC_MAP_GPORT_TO_GCHIP(gport), &temp_lchip) && (CTC_MAP_GPORT_TO_LPORT(gport) >= MCHIP_CAP(SYS_CAP_PORT_NUM_GLOBAL))))))  \
    {                                                                                         \
        sal_mutex_unlock(lock);    \
        return CTC_E_INVALID_PORT;                                                     \
    }   \
}

#define SYS_USW_CID_CHECK(lchip,cid)  \
     if ( ((cid) == CTC_ACL_UNKNOWN_CID) || ((cid) > CTC_MAX_ACL_CID)) \
    { return CTC_E_BADID; }

#define SYS_USW_DESTMAP_TO_DRV_GPORT(destmap) \
    ((((destmap >> 9) & 0x7F) << 9) | (destmap & 0x1FF)); \

#define SYS_UINT64_SET(val_64, value)  do{\
    val_64 = value[1];\
    val_64 <<= 32;\
    val_64 |= value[0];\
    }while(0)

#define SYS_USW_SET_HW_MAC(dest, src)     \
    {   \
        (dest)[1]= (((src)[0] << 8)  | ((src)[1]));        \
        (dest)[0]= (((src)[2] << 24) | ((src)[3] << 16) | ((src)[4] << 8) | ((src)[5])); \
    }

#define SYS_USW_SET_HW_UDF(lchip, dest, src)     \
    if(DRV_IS_DUET2(lchip) || DRV_IS_TSINGMA(lchip))\
    {   \
        (dest)[3] = ((src)[12] << 24 | src[13]<<16| src[14]<<8|src[15]);                    \
        (dest)[2] = ((src)[8] << 24 | src[9]<<16| src[10]<<8|src[11]);                       \
        (dest)[1] = ((src)[4] << 24 | src[5]<<16| src[6]<<8|src[7]);                        \
        (dest)[0] = ((src)[0] << 24 | src[1]<<16| src[2]<<8|src[3]);                        \
    }

#define SYS_USW_REVERT_SATPDU_BYTE(dest, src)     \
    {   uint8* p_src = (uint8*)src; \
        uint32* p_dest = (uint32*)dest; \
        (p_dest)[0] = ((p_src)[4] << 24 | p_src[5]<<16| p_src[6]<<8|p_src[7]);                        \
        (p_dest)[1] = ((p_src)[0] << 24 | p_src[1]<<16| p_src[2]<<8|p_src[3]);     \
    }
#define SYS_USW_REVERT_SCI_BYTE(dest, src)     \
    {   uint8* p_src = (uint8*)src; \
        uint32* p_dest = (uint32*)dest; \
        (p_dest)[0] = ((p_src)[4] << 24 | p_src[5]<<16| p_src[6]<<8|p_src[7]);                        \
        (p_dest)[1] = ((p_src)[0] << 24 | p_src[1]<<16| p_src[2]<<8|p_src[3]);     \
    }

#define SYS_USW_REVERT_BYTE(dest, src)     \
    {   \
        (dest)[1]= (src)[0]; \
        (dest)[0]= (src)[1]; \
    }

#define SYS_USW_SET_USER_MAC(dest, src)     \
    {   \
        (dest)[0] = (src)[1] >> 8;          \
        (dest)[1] = (src)[1] & 0xFF;        \
        (dest)[2] = (src)[0] >> 24;         \
        (dest)[3] = ((src)[0] >> 16) & 0xFF;\
        (dest)[4] = ((src)[0] >> 8) & 0xFF; \
        (dest)[5] = (src)[0] & 0xFF;        \
    }
#define SYS_USW_SET_USER_MAC_PREFIX(dest, src)     \
    {   \
        (dest)[0] = (src)[1] & 0xFF;          \
        (dest)[1] = (src)[0] >> 24;        \
        (dest)[2] = ((src)[0] >> 16) & 0xFF;         \
        (dest)[3] = ((src)[0] >> 8) & 0xFF;\
        (dest)[4] = (src)[0] & 0xFF; \
    }

#define SYS_USW_REVERT_IP6(dest, src)     \
    {                                            \
        (dest)[3] = (src)[0];                        \
        (dest)[2] = (src)[1];                        \
        (dest)[1] = (src)[2];                        \
        (dest)[0] = (src)[3];                        \
    }

#define SYS_CHECK_UNION_BITMAP(gtype,gtype_value)  if(gtype!=0 && gtype != gtype_value ) return CTC_E_PARAM_CONFLICT;

#define SYS_SET_UNION_TYPE(gtype,gtype_value,T,un_id,ds, is_add) \
 { \
    uint32 fld_data[4] = {0};\
    if((gtype)!=0 && (gtype) != (gtype_value)) CTC_ERROR_RETURN(CTC_E_PARAM_CONFLICT);\
    if(is_add)  {\
        (gtype) = (gtype_value);\
    }else {\
        Get##T(A, u##un_id##_data_f, ds, fld_data);\
        (gtype) = (!fld_data[0] && !fld_data[1] && !fld_data[2] && !fld_data[3])? 0: (gtype);\
    }\
 }

#define SYS_LOGIC_PORT_CHECK(logic_port)           \
    if ((logic_port) >= sys_usw_common_get_vport_num(lchip)) \
    { return CTC_E_INVALID_PORT; }

#define SYS_LOGIC_PORT_CHECK_WITH_UNLOCK(logic_port, lock)           \
    if ((logic_port) >= sys_usw_common_get_vport_num(lchip)) \
    {    \
        sal_mutex_unlock(lock);    \
        return CTC_E_INVALID_PORT;  \
    }

#define SYS_USW_GLOBAL_ACL_PROP_CHK(prop) \
{   \
    CTC_MAX_VALUE_CHECK(prop->acl_en, 1);     \
    CTC_MAX_VALUE_CHECK(prop->direction, CTC_EGRESS); \
    if (prop->direction == CTC_INGRESS)   \
    {   \
        CTC_MAX_VALUE_CHECK(prop->acl_priority, MCHIP_CAP(SYS_CAP_ACL_INGRESS_ACL_LKUP)-1);   \
    }   \
    else    \
    {   \
        CTC_MAX_VALUE_CHECK(prop->acl_priority, MCHIP_CAP(SYS_CAP_ACL_EGRESS_LKUP)-1);   \
    }   \
    CTC_MAX_VALUE_CHECK(prop->tcam_lkup_type,CTC_ACL_TCAM_LKUP_TYPE_MAX-1);  \
    CTC_MAX_VALUE_CHECK(prop->hash_lkup_type, CTC_ACL_HASH_LKUP_TYPE_MAX-1);    \
    CTC_MAX_VALUE_CHECK(prop->hash_field_sel_id, 0xF);    \
}

#define SYS_ACL_PROPERTY_CHECK(prop)    \
{   \
    SYS_USW_GLOBAL_ACL_PROP_CHK(prop);\
    if (CTC_ACL_TCAM_LKUP_TYPE_COPP == prop->tcam_lkup_type)\
    {\
        return CTC_E_NOT_SUPPORT;\
    }\
}

#define SYS_L3IFID_VAILD_CHECK(l3if_id)         \
if (l3if_id == 0 || l3if_id > (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1))  \
{                                               \
	return CTC_E_BADID;\
}

#define    SYS_MAX_VALUE_CHECK_WITH_UNLOCK(var, max_value, mutex) \
    { \
        if ((var) > (max_value)) \
        { \
            sal_mutex_unlock(mutex); \
            return CTC_E_INVALID_PARAM; \
        } \
    }


#define SYS_CHIP_SUPPORT_CHECK(lchip, chip_type) \
do{\
    if(drv_is_chip(lchip, chip_type))\
    {\
        return CTC_E_NOT_SUPPORT;\
    }\
}while(0)


#define SYS_L4_PROTOCOL_IPV4_ICMP                        1
#define SYS_L4_PROTOCOL_IPV4_IGMP                        2
#define SYS_L4_PROTOCOL_TCP                              6
#define SYS_L4_PROTOCOL_UDP                              17
#define SYS_L4_PROTOCOL_RDP                              27
#define SYS_L4_PROTOCOL_IPV6_ICMP                        58
#define SYS_L4_PROTOCOL_GRE                              47
#define SYS_L4_PROTOCOL_IPV4                             4
#define SYS_L4_PROTOCOL_IPV6                             41
#define SYS_L4_PROTOCOL_DCCP                             33
#define SYS_L4_PROTOCOL_SCTP                             132
#define SYS_MTU_MAX_SIZE     0x3FFF


#define SYS_ACL_TCAM_LKUP_TYPE_NONE                0
#define SYS_ACL_TCAM_LKUP_TYPE_L2                  1
#define SYS_ACL_TCAM_LKUP_TYPE_L2_L3               2
#define SYS_ACL_TCAM_LKUP_TYPE_L3                  3
#define SYS_ACL_TCAM_LKUP_TYPE_VLAN                4
#define SYS_ACL_TCAM_LKUP_TYPE_L3_EXT              5
#define SYS_ACL_TCAM_LKUP_TYPE_L2_L3_EXT           6
#define SYS_ACL_TCAM_LKUP_TYPE_CID                 7
#define SYS_ACL_TCAM_LKUP_TYPE_INTERFACE           8
#define SYS_ACL_TCAM_LKUP_TYPE_FORWARD             9
#define SYS_ACL_TCAM_LKUP_TYPE_FORWARD_EXT         10
#define SYS_ACL_TCAM_LKUP_TYPE_COPP                11
#define SYS_ACL_TCAM_LKUP_TYPE_UDF                  12
#define SYS_ACL_TCAM_LKUP_TYPE_MAX                 13




enum sys_bpe_igs_muxtype_e
{
    BPE_IGS_MUXTYPE_NOMUX = 0,
    BPE_IGS_MUXTYPE_MUXDEMUX = 1,
    BPE_IGS_MUXTYPE_EVB = 2,
    BPE_IGS_MUXTYPE_CB_DOWNLINK = 3,
    BPE_IGS_MUXTYPE_PE_DOWNLINK_WITH_CASCADE_PORT = 4,
    BPE_IGS_MUXTYPE_PE_UPLINK = 5,
    EPE_IGS_MUXTYPE_GEM_PORT = 6,
    BPE_IGS_MUXTYPE_NUM
};
typedef enum sys_bpe_igs_muxtype_e bpe_igs_muxtype_t;

enum sys_bpe_egs_muxtype_e
{
    BPE_EGS_MUXTYPE_NOMUX,
    BPE_EGS_MUXTYPE_MUXDEMUX = 2,
    BPE_EGS_MUXTYPE_LOOPBACK_ENCODE,
    BPE_EGS_MUXTYPE_EVB,
    BPE_EGS_MUXTYPE_CB_CASCADE,
    BPE_EGS_MUXTYPE_UPSTREAM,
    BPE_EGS_MUXTYPE_CASCADE,
    BPE_EGS_MUXTYPE_NUM
};
typedef enum sys_bpe_egs_muxtype_e bpe_egs_muxtype_t;

enum sys_rpf_chk_type_e
{
    SYS_RPF_CHK_TYPE_STRICK,
    SYS_RPF_CHK_TYPE_LOOSE,
    SYS_RPF_CHK_TYPE_NUM
};
typedef enum sys_rpf_chk_type_e sys_rpf_chk_type_t;

#ifdef EMULATION_ENV
#define DOWN_FRE_RATE 2000
#else
#define DOWN_FRE_RATE 1
#endif

typedef uint32 ds_t[32];

struct sys_traverse_s
{
    void* data;
    void* data1;
    uint32 value1;
    uint32 value2;
    uint32 value3;
    uint32 value4;
};
typedef struct sys_traverse_s  sys_traverse_t;
/*****************************************************************************************/

extern char*
sys_output_mac(mac_addr_t in_mac);

extern bool
sys_usw_common_check_feature_support(uint8 feature);

extern uint8
sys_usw_map_acl_tcam_lkup_type(uint8 lchip, uint8 ctc_lkup_t);

extern uint8
sys_usw_unmap_acl_tcam_lkup_type(uint8 lchip, uint8 sys_lkup_t);

extern int32
sys_usw_common_get_compress_near_division_value(uint8 lchip, uint32 value,uint8 division_wide,uint8 shift_bit_wide,
                                                                 uint16* division, uint16* shift_bits, uint8 is_negative);
extern uint32
sys_usw_common_get_vport_num(uint8 lchip);

const char*
sys_usw_get_pkt_type_desc(uint8 pkt_type);
extern int32
sys_usw_dword_reverse_copy(uint32* dest, uint32* src, uint32 len);
extern int32
sys_usw_byte_reverse_copy(uint8* dest, uint8* src, uint32 len);
extern int32
sys_usw_swap32(uint32* data, int32 len, uint32 hton);

int32
sys_usw_ma_add_to_asic(uint8 lchip, uint32 ma_index, void* dsma);

int32
sys_usw_ma_clear_write_cache(uint8 lchip);

extern int32
sys_usw_task_create(uint8 lchip,
                sal_task_t** ptask,
                char* name,
                size_t stack_size,
                int prio,
                sal_task_type_t task_type,
                unsigned long long cpu_mask,
                void (* start)(void*),
                void* arg);

#ifdef __cplusplus
}
#endif

#endif

