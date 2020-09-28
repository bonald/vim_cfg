#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"
#include "ctc_usw_dkit.h"
#include "ctc_usw_dkit_drv.h"
#include "ctc_usw_dkit_discard.h"
#include "ctc_usw_dkit_discard_type.h"
#include "ctc_usw_dkit_path.h"
#include "ctc_usw_dkit_captured_info.h"

#define DRV_TCAM_IPEACL_3_BASE        (DRV_TCAM_KEY0_MAX_ENTRY_NUM)
#define DRV_TCAM_IPEACL_2_BASE        (DRV_TCAM_IPEACL_3_BASE + DRV_TCAM_KEY1_MAX_ENTRY_NUM)
#define DRV_TCAM_USERID_0_BASE        (DRV_TCAM_IPEACL_2_BASE + DRV_TCAM_KEY2_MAX_ENTRY_NUM)
#define DRV_TCAM_EPEACL_0_BASE        (DRV_TCAM_USERID_0_BASE + DRV_TCAM_KEY3_MAX_ENTRY_NUM)
#define DRV_TCAM_IPEACL_1_BASE        (DRV_TCAM_EPEACL_0_BASE + DRV_TCAM_KEY4_MAX_ENTRY_NUM)
#define DRV_TCAM_IPEACL_0_BASE        (DRV_TCAM_IPEACL_1_BASE + DRV_TCAM_KEY5_MAX_ENTRY_NUM)

#define Uints_80bits_Per_Block        2048      /*Tcam entry(80 bits) num per block*/

#define CTC_DKIT_IPV6_ADDR_STR_LEN    44          /**< IPv6 address string length */

#define CTC_DKIT_CAPTURED_HASH_HIT "HASH lookup hit"
#define CTC_DKIT_CAPTURED_TCAM_HIT "TCAM lookup hit"
#define CTC_DKIT_CAPTURED_NOT_HIT "Not hit any entry!!!"
#define CTC_DKIT_CAPTURED_CONFLICT "HASH lookup conflict"
#define CTC_DKIT_CAPTURED_DEFAULT "HASH lookup hit default entry"

#define CTC_DKITS_LPM_TCAM_REAL_ENTRY_NUM        1024

#define CTC_DKITS_LPM_TCAM_MAX_ENTRY_NUM (DRV_IS_DUET2(lchip) ? 1024:2048)

#define CTC_DKITS_LPM_TCAM_LOCAL_BIT_LEN (DRV_IS_DUET2(lchip) ? 12:13)
struct ctc_dkit_captured_path_info_s
{
      uint8 slice_ipe;
      uint8 slice_epe;
      uint8 is_ipe_captured;
      uint8 is_bsr_captured;
      uint8 is_epe_captured;
      uint8 oam_rx_captured;
      uint8 oam_tx_captured;
      uint8 discard;
      uint8 exception;
      uint8 detail;
      uint32 discard_type;
      uint32 exception_index;
      uint32 exception_sub_index;
      uint8 rx_oam;
      uint8 lchip;
      uint8 path_sel;
};
typedef struct ctc_dkit_captured_path_info_s ctc_dkit_captured_path_info_t;
STATIC int32
_ctc_usw_dkit_captured_path_oam_rx(ctc_dkit_captured_path_info_t* p_info);
STATIC int32
_ctc_usw_dkit_captured_path_oam_tx(ctc_dkit_captured_path_info_t* p_info);

STATIC int32
_ctc_usw_dkit_ipuc_get_table_id(uint8 lchip, uint32 pri_valid, uint32 tcam_group_index, uint32 key_type, uint32* key_id, uint32* ad_id, uint32* p_cmd, uint32* p_cmd1)
{
    uint32 cmd = 0;
    LpmTcamCtl_m lpm_tcam_ctl;

    cmd = DRV_IOR(LpmTcamCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &lpm_tcam_ctl);

    if (CTC_DKITS_L3DA_TYPE_IPV4UCAST == key_type)
    {
        *key_id = pri_valid ? DsLpmTcamIpv4HalfKey_t : DsLpmTcamIpv4DaPubHalfKey_t;
        *ad_id = pri_valid ? DsLpmTcamIpv4HalfKeyAd_t : DsLpmTcamIpv4DaPubHalfKeyAd_t;
        *p_cmd = pri_valid ? DRV_IOR(DsLpmTcamIpv4HalfKeyAd_t, DsLpmTcamIpv4HalfKeyAd_lpmPipelineValid_f):\
            DRV_IOR(DsLpmTcamIpv4DaPubHalfKeyAd_t, DsLpmTcamIpv4DaPubHalfKeyAd_lpmPipelineValid_f);
        *p_cmd1 = pri_valid ? DRV_IOR(DsLpmTcamIpv4HalfKeyAd_t, DsLpmTcamIpv4HalfKeyAd_lpmStartByte_f):\
            DRV_IOR(DsLpmTcamIpv4DaPubHalfKeyAd_t, DsLpmTcamIpv4DaPubHalfKeyAd_lpmStartByte_f);
    }
    else if (CTC_DKITS_L3DA_TYPE_IPV6UCAST == key_type && !GetLpmTcamCtl(V, tcamGroupConfig_0_ipv6Lookup64Prefix_f + tcam_group_index, &lpm_tcam_ctl))
    {
        *key_id = pri_valid ? DsLpmTcamIpv6DoubleKey0_t : DsLpmTcamIpv6DaPubDoubleKey0_t;
        *ad_id = pri_valid ? DsLpmTcamIpv6DoubleKey0Ad_t : DsLpmTcamIpv6DaPubDoubleKey0Ad_t;
        *p_cmd = pri_valid ? DRV_IOR(DsLpmTcamIpv6DoubleKey0Ad_t, DsLpmTcamIpv6DoubleKey0Ad_lpmPipelineValid_f):\
            DRV_IOR(DsLpmTcamIpv6DaPubDoubleKey0Ad_t, DsLpmTcamIpv6DaPubDoubleKey0Ad_lpmPipelineValid_f);
        *p_cmd1 = pri_valid ? DRV_IOR(DsLpmTcamIpv6DoubleKey0Ad_t, DsLpmTcamIpv6DoubleKey0Ad_lpmStartByte_f):\
            DRV_IOR(DsLpmTcamIpv6DaPubDoubleKey0Ad_t, DsLpmTcamIpv6DoubleKey0Ad_lpmStartByte_f);
    }
    else if (CTC_DKITS_L3DA_TYPE_IPV6UCAST == key_type && GetLpmTcamCtl(V, tcamGroupConfig_0_ipv6Lookup64Prefix_f + tcam_group_index, &lpm_tcam_ctl))
    {
        *key_id = pri_valid ? DsLpmTcamIpv6SingleKey_t : DsLpmTcamIpv6DaPubSingleKey_t;
        *ad_id = pri_valid ? DsLpmTcamIpv6SingleKeyAd_t : DsLpmTcamIpv6DaPubSingleKeyAd_t;
        *p_cmd = pri_valid ? DRV_IOR(DsLpmTcamIpv6SingleKeyAd_t, DsLpmTcamIpv6SingleKeyAd_lpmPipelineValid_f):\
            DRV_IOR(DsLpmTcamIpv6DaPubSingleKeyAd_t, DsLpmTcamIpv6DaPubSingleKeyAd_lpmPipelineValid_f);
        *p_cmd1 = pri_valid ? DRV_IOR(DsLpmTcamIpv6SingleKeyAd_t, DsLpmTcamIpv6SingleKeyAd_lpmStartByte_f):\
            DRV_IOR(DsLpmTcamIpv6DaPubSingleKeyAd_t, DsLpmTcamIpv6DaPubSingleKeyAd_lpmStartByte_f);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_ipuc_map_key_index(uint8 lchip, uint32 tbl_id, uint32 key_index, uint32* mapped_key_idx, uint32* mapped_ad_idx)
{
    uint8 block_num = 0;
    uint8 blk_id = 0;
    uint16 blk_offset = 0;
    uint32 local_index = 0;
    uint32 block_size = 0;

    block_num = (key_index >> CTC_DKITS_LPM_TCAM_LOCAL_BIT_LEN)&0x3 ;
    block_size = ((block_num <= 1) ? (CTC_DKITS_LPM_TCAM_MAX_ENTRY_NUM/2): (CTC_DKITS_LPM_TCAM_MAX_ENTRY_NUM));
    local_index = key_index & ((1<<CTC_DKITS_LPM_TCAM_LOCAL_BIT_LEN )-1);/* use 50 bit as unit */

    blk_id = local_index / block_size + block_num * 4;
    blk_offset = local_index - local_index / block_size * block_size;
    *mapped_key_idx = (TCAM_START_INDEX(lchip, tbl_id, blk_id) +  blk_offset) / (TCAM_KEY_SIZE(lchip, tbl_id) / DRV_LPM_KEY_BYTES_PER_ENTRY);
    *mapped_ad_idx = TCAM_START_INDEX(lchip, tbl_id, blk_id) +  blk_offset;

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_map_tcam_key_index(uint8 lchip, uint32 dbg_index, uint8 is_acl, uint8 lkup_level, uint32* o_key_index)
{
    uint8  tcam_blk_id = 0;
    uint16 local_index = 0;
    uint32 key_index = 0xFFFFFFFF;
    uint32 entry_num = 0;
    uint8  hit_blk_max_default = 0;
    uint32 acl_tbl_id[] = {DsAclQosMacKey160Ing0_t, DsAclQosMacKey160Ing1_t, DsAclQosMacKey160Ing2_t, DsAclQosMacKey160Ing3_t,
                           DsAclQosMacKey160Ing4_t, DsAclQosMacKey160Ing5_t, DsAclQosMacKey160Ing6_t, DsAclQosMacKey160Ing7_t};
    uint32 scl_tbl_id[] = {DsScl0L3Key160_t, DsScl1L3Key160_t,DsScl2L3Key160_t, DsScl3L3Key160_t};

    tcam_blk_id = dbg_index >> 11;
    local_index = dbg_index & 0x7FF;

    /*D2:USERID 2,IPE_ACL 8,EPE_ACL 3*/
    /*TM:USERID 4,IPE_ACL 8,EPE_ACL 3*/
    hit_blk_max_default = DRV_IS_DUET2(lchip) ? 13:15;
    if(tcam_blk_id < hit_blk_max_default)
    {
        key_index = local_index;
    }
    else  /*used share tcam*/
    {
        drv_usw_ftm_get_entry_num(lchip, is_acl ?acl_tbl_id[lkup_level]:scl_tbl_id[lkup_level], &entry_num);
        key_index = 2*entry_num - Uints_80bits_Per_Block + local_index;
    }

    if(o_key_index)
    {
        *o_key_index = key_index;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_discard_process(ctc_dkit_captured_path_info_t* p_info,
                                        uint32 discard, uint32 discard_type, uint32 module)
{
    if (1 == discard)
    {
        p_info->discard = 1;
        p_info->discard_type = discard_type;

        if (CTC_DKIT_IPE == module)/* ipe */
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Drop!!!", ctc_usw_dkit_get_reason_desc(discard_type + IPE_FEATURE_START));
        }
        else if (CTC_DKIT_EPE == module)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Drop!!!", ctc_usw_dkit_get_reason_desc(discard_type + EPE_FEATURE_START));
        }
    }
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_exception_process(ctc_dkit_captured_path_info_t* p_info,
                                          uint32 exception, uint32 exception_index, uint32 exception_sub_index)
{
    char* exception_desc[8] =
    {
    "Reserved",    /* IPEEXCEPTIONINDEX_RESERVED               CBit(3,'h',"0", 1)*/
    "Bridge exception", /* IPEEXCEPTIONINDEX_BRIDGE_EXCEPTION       CBit(3,'h',"1", 1)*/
    "Route exception", /* IPEEXCEPTIONINDEX_ROUTE_EXCEPTION        CBit(3,'h',"2", 1)*/
    "Other exception" /* IPEEXCEPTIONINDEX_OTHER_EXCEPTION        CBit(3,'h',"3", 1)*/

    };

    if (1 == exception)
    {
        p_info->exception = 1;
        p_info->exception_index = exception_index;
        p_info->exception_sub_index = exception_sub_index;

        CTC_DKIT_PATH_PRINT("%-15s:%s!!!\n", "Exception",exception_desc[exception_index]);
        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Excep index", exception_index);
        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Excep sub index", exception_sub_index);
    }
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_ipe_exception_check(ctc_dkit_captured_path_info_t* p_info)
{
    uint8 slice = p_info->slice_ipe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;

    /*DbgIpeFwdProcessInfo*/
    uint32 fwd_exceptionEn = 0;
    uint32 fwd_exceptionIndex = 0;
    uint32 fwd_exceptionSubIndex = 0;
    DbgIpeFwdProcessInfo_m dbg_ipe_fwd_process_info;


    /*DbgIpeFwdProcessInfo*/
    sal_memset(&dbg_ipe_fwd_process_info, 0, sizeof(dbg_ipe_fwd_process_info));
    cmd = DRV_IOR(DbgIpeFwdProcessInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_ipe_fwd_process_info);

    GetDbgIpeFwdProcessInfo(A, exceptionEn_f, &dbg_ipe_fwd_process_info, &fwd_exceptionEn);
    GetDbgIpeFwdProcessInfo(A, exceptionIndex_f, &dbg_ipe_fwd_process_info, &fwd_exceptionIndex);
    GetDbgIpeFwdProcessInfo(A, exceptionSubIndex_f, &dbg_ipe_fwd_process_info, &fwd_exceptionSubIndex);

    _ctc_usw_dkit_captured_path_exception_process(p_info, fwd_exceptionEn, fwd_exceptionIndex, fwd_exceptionSubIndex);
    return CLI_SUCCESS;
}

STATIC char*
_ctc_usw_dkit_captured_get_igr_scl_key_desc(uint8 lchip, uint32 key_type, uint8 lk_level)
{
    lk_level = !lk_level;
    switch (key_type)
    {
        case USERIDHASHTYPE_DISABLE:
            return "";
        case USERIDHASHTYPE_DOUBLEVLANPORT:
            return lk_level ? "DsUserIdDoubleVlanPortHashKey" :"DsUserId1DoubleVlanPortHashKey";
        case USERIDHASHTYPE_SVLANPORT:
            return lk_level ? "DsUserIdSvlanPortHashKey" : "DsUserId1SvlanPortHashKey";
        case USERIDHASHTYPE_CVLANPORT:
            return lk_level ? "DsUserIdCvlanPortHashKey" : "DsUserId1CvlanPortHashKey";
        case USERIDHASHTYPE_SVLANCOSPORT:
            return lk_level ? "DsUserIdSvlanCosPortHashKey" : "DsUserId1SvlanCosPortHashKey";
        case USERIDHASHTYPE_CVLANCOSPORT:
            return lk_level ? "DsUserIdCvlanCosPortHashKey" : "DsUserId1CvlanCosPortHashKey";
        case USERIDHASHTYPE_MACPORT:
            return lk_level ? "DsUserIdMacPortHashKey" : "DsUserId1MacPortHashKey";
        case USERIDHASHTYPE_IPV4PORT:
            return lk_level ? "DsUserIdIpv4PortHashKey" : "DsUserId1Ipv4PortHashKey";
        case USERIDHASHTYPE_MAC:
            return lk_level ? "DsUserIdMacHashKey" : "DsUserId1MacHashKey";
        case USERIDHASHTYPE_IPV4SA:
            return lk_level ? "DsUserIdIpv4SaHashKey" : "DsUserId1Ipv4SaHashKey";
        case USERIDHASHTYPE_PORT:
            return lk_level ? "DsUserIdPortHashKey" : "DsUserId1PortHashKey";
        case USERIDHASHTYPE_SVLANMACSA:
            return lk_level ? "DsUserIdSvlanMacSaHashKey" : "DsUserId1SvlanMacSaHashKey";
        case USERIDHASHTYPE_SVLAN:
            return lk_level ? "DsUserIdSvlanHashKey" : "DsUserId1SvlanHashKey";
        case USERIDHASHTYPE_ECIDNAMESPACE:
            return lk_level ? "DsUserIdEcidNameSpaceHashKey" : "DsUserId1EcidNameSpaceHashKey";
        case USERIDHASHTYPE_INGECIDNAMESPACE:
            return lk_level ? "DsUserIdIngEcidNameSpaceHashKey" : "DsUserId1IngEcidNameSpaceHashKey";
        case USERIDHASHTYPE_IPV6SA:
            return lk_level ? "DsUserIdIpv6SaHashKey" : "DsUserId1Ipv6SaHashKey";
        case USERIDHASHTYPE_IPV6PORT:
            return lk_level ? "DsUserIdIpv6PortHashKey" : "DsUserId1Ipv6PortHashKey";
        case USERIDHASHTYPE_CAPWAPSTASTATUS:
            return lk_level ? "DsUserIdCapwapStaStatusHashKey" : "DsUserId1CapwapStaStatusHashKey";
        case USERIDHASHTYPE_CAPWAPSTASTATUSMC:
            return lk_level ? "DsUserIdCapwapStaStatusMcHashKey" : "DsUserId1CapwapStaStatusMcHashKey";
        case USERIDHASHTYPE_CAPWAPMACDAFORWARD:
            return lk_level ? "DsUserIdCapwapMacDaForwardHashKey" : "DsUserId1CapwapMacDaForwardHashKey";
        case USERIDHASHTYPE_CAPWAPVLANFORWARD:
            return lk_level ? "DsUserIdCapwapVlanForwardHashKey" : "DsUserId1CapwapVlanForwardHashKey";
        case USERIDHASHTYPE_TUNNELIPV4:
            return lk_level ? "DsUserIdTunnelIpv4HashKey" : "DsUserId1TunnelIpv4HashKey";
        case USERIDHASHTYPE_TUNNELIPV4GREKEY:
            return lk_level ? "DsUserIdTunnelIpv4GreKeyHashKey" : "DsUserId1TunnelIpv4GreKeyHashKey";
        case USERIDHASHTYPE_TUNNELIPV4UDP:
            return lk_level ? "DsUserIdTunnelIpv4UdpHashKey" : "DsUserId1TunnelIpv4UdpHashKey";
        case USERIDHASHTYPE_TUNNELPBB:
            return "";
        case USERIDHASHTYPE_TUNNELTRILLUCRPF:
            return lk_level ? "DsUserIdTunnelTrillUcRpfHashKey" : "DsUserId1TunnelTrillUcRpfHashKey";
        case USERIDHASHTYPE_TUNNELTRILLUCDECAP:
            return lk_level ? "DsUserIdTunnelTrillUcDecapHashKey" : "DsUserId1TunnelTrillUcDecapHashKey";
        case USERIDHASHTYPE_TUNNELTRILLMCRPF:
            return lk_level ? "DsUserIdTunnelTrillMcRpfHashKey" : "DsUserId1TunnelTrillMcRpfHashKey";
        case USERIDHASHTYPE_TUNNELTRILLMCDECAP:
            return lk_level ? "DsUserIdTunnelTrillMcDecapHashKey" : "DsUserId1TunnelTrillMcDecapHashKey";
        case USERIDHASHTYPE_TUNNELTRILLMCADJ:
            return lk_level ? "DsUserIdTunnelTrillMcAdjHashKey" : "DsUserId1TunnelTrillMcAdjHashKey";
        case USERIDHASHTYPE_TUNNELIPV4RPF:
            return lk_level ? "DsUserIdTunnelIpv4RpfHashKey" : "DsUserId1TunnelIpv4RpfHashKey";
        case USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE0:
            return lk_level ? "DsUserIdTunnelIpv4UcVxlanMode0HashKey" : "DsUserId1TunnelIpv4UcVxlanMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE1:
            return lk_level ? "DsUserIdTunnelIpv4UcVxlanMode1HashKey" : "DsUserId1TunnelIpv4UcVxlanMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0:
            return lk_level ? "DsUserIdTunnelIpv6UcVxlanMode0HashKey" : "DsUserId1TunnelIpv6UcVxlanMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1:
            return lk_level ? "DsUserIdTunnelIpv6UcVxlanMode1HashKey" : "DsUserId1TunnelIpv6UcVxlanMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE0:
            return lk_level ? "DsUserIdTunnelIpv4UcNvgreMode0HashKey" : "DsUserId1TunnelIpv4UcNvgreMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE1:
            return lk_level ? "DsUserIdTunnelIpv4UcNvgreMode1HashKey" : "DsUserId1TunnelIpv4UcNvgreMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0:
            return lk_level ? "DsUserIdTunnelIpv6UcNvgreMode0HashKey" : "DsUserId1TunnelIpv6UcNvgreMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1:
            return lk_level ? "DsUserIdTunnelIpv6UcNvgreMode1HashKey" : "DsUserId1TunnelIpv6UcNvgreMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV4MCVXLANMODE0:
            return lk_level ? "DsUserIdTunnelIpv4McVxlanMode0HashKey" : "DsUserId1TunnelIpv4McVxlanMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV4VXLANMODE1:
            return lk_level ? "DsUserIdTunnelIpv4VxlanMode1HashKey" : "DsUserId1TunnelIpv4VxlanMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0:
            return lk_level ? "DsUserIdTunnelIpv6McVxlanMode0HashKey" : "DsUserId1TunnelIpv6McVxlanMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1:
            return lk_level ? "DsUserIdTunnelIpv6McVxlanMode1HashKey" : "DsUserId1TunnelIpv6McVxlanMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV4MCNVGREMODE0:
            return lk_level ? "DsUserIdTunnelIpv4McNvgreMode0HashKey" : "DsUserId1TunnelIpv4McNvgreMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV4NVGREMODE1:
            return lk_level ? "DsUserIdTunnelIpv4NvgreMode1HashKey" : "DsUserId1TunnelIpv4NvgreMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0:
            return lk_level ? "DsUserIdTunnelIpv6McNvgreMode0HashKey" : "DsUserId1TunnelIpv6McNvgreMode0HashKey";
        case USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1:
            return lk_level ? "DsUserIdTunnelIpv6McNvgreMode1HashKey" : "DsUserId1TunnelIpv6McNvgreMode1HashKey";
        case USERIDHASHTYPE_TUNNELIPV4CAPWAP:
            return lk_level ? "DsUserIdTunnelIpv4CapwapHashKey" : "DsUserId1TunnelIpv4CapwapHashKey";
        case USERIDHASHTYPE_TUNNELIPV6CAPWAP:
            return lk_level ? "DsUserIdTunnelIpv6CapwapHashKey" : "DsUserId1TunnelIpv6CapwapHashKey";
        case USERIDHASHTYPE_TUNNELCAPWAPRMAC:
            return lk_level ? "DsUserIdTunnelCapwapRmacHashKey" : "DsUserId1TunnelCapwapRmacHashKey";
        case USERIDHASHTYPE_TUNNELCAPWAPRMACRID:
            return lk_level ? "DsUserIdTunnelCapwapRmacRidHashKey" : "DsUserId1TunnelCapwapRmacRidHashKey";
        case USERIDHASHTYPE_TUNNELMPLS:
            return lk_level ? "DsUserIdTunnelMplsHashKey" : "";
        default:
            if(key_type == DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA))
            {
                return lk_level ? "DsUserIdTunnelIpv4DaHashKey" : "DsUserId1TunnelIpv4DaHashKey";
            }
            else if(key_type == DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2))
            {
                return lk_level ? "DsUserIdSclFlowL2HashKey" :"DsUserId1SclFlowL2HashKey";
            }
            return "";
    }
}

STATIC int32
_ctc_usw_dkit_captured_is_hit(ctc_dkit_captured_path_info_t* path_info)
{
    uint32 cmd = 0;
    uint32 value = 0;

    ctc_dkit_captured_path_info_t* p_info = (ctc_dkit_captured_path_info_t*)path_info;
    uint8 lchip = path_info->lchip;

    if (NULL == p_info)
    {
        return CLI_ERROR;
    }

    /*1. IPE*/
    cmd = DRV_IOR(DbgIpeLkpMgrInfo_t, DbgIpeLkpMgrInfo_valid_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    if (value)
    {
        p_info->slice_ipe = 0;
        p_info->is_ipe_captured = 1;
    }

    if (p_info->is_ipe_captured)
    {

        cmd = DRV_IOR(DbgIpeOamInfo_t, DbgIpeOamInfo_tempRxOamType_f);
        DRV_IOCTL(lchip, p_info->slice_ipe, cmd, &value);
        p_info->rx_oam = 1;
    }

    /*2. EPE*/
    cmd = DRV_IOR(DbgEpeHdrAdjInfo_t, DbgEpeHdrAdjInfo_valid_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    if (value)
    {
        p_info->slice_epe = 0;
        p_info->is_epe_captured = 1;
    }

    /*3. BSR*/
    cmd = DRV_IOR(DbgFwdBufStoreInfo_t, DbgFwdBufStoreInfo_valid_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    if (value)
    {
        p_info->is_bsr_captured = 1;
    }

    /*3. OAMEngine*/
    cmd = DRV_IOR(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjValid_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    if (value)
    {
        p_info->oam_rx_captured = 1;
    }
    cmd = DRV_IOR(DbgOamTxProc_t, DbgOamTxProc_oamTxProcValid_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    if (value)
    {
        p_info->oam_tx_captured = 1;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_parser(ctc_dkit_captured_path_info_t* p_info, uint8 type)
{
    uint32 cmd = 0;
    uint8 slice = p_info->slice_ipe;
    uint8 lchip = p_info->lchip;
    uint8 idx1  = 0;
    char* str_parser[] = {"1st", "2nd", "epe"};
    char* str_l3_type[] = {"NONE", "IP", "IPV4", "IPV6", "MPLS", "MPLSUP", "ARP", "FCOE", "TRILL",
                           "ETHEROAM", "SLOWPROTO", "CMAC", "PTP", "DOT1AE", "SATPDU", "FLEXIBLE"};
    char buf[CTC_DKIT_IPV6_ADDR_STR_LEN] = {0};
    uint32 temp_data = 0;
    ipv6_addr_t ip6_address = {0};
    mac_addr_t    mac_address = {0};
    hw_mac_addr_t hw_mac_da = {0};
    hw_mac_addr_t hw_mac_sa = {0};
    uint32      svlan_valid = 0;
    uint32      svlan_id    = 0;
    uint32      stag_cfi    = 0;
    uint32      stag_cos    = 0;
    uint32      cvlan_valid = 0;
    uint32      cvlan_id    = 0;
    uint32      ctag_cfi    = 0;
    uint32      ctag_cos    = 0;
    uint32      ether_type  = 0;
    uint32      gre_flags   = 0;
    uint32      gre_proto_type = 0;
    uint32      layer3_type = 0;
    uint32      layer4_type = 0;
    uint32      l4_src_port = 0;
    uint32      l4_dst_port = 0;
    ipv6_addr_t hw_ip6_da   = {0};
    ipv6_addr_t hw_ip6_sa   = {0};
    uint32      ip4_da      = 0;
    uint32      ip4_sa      = 0;
    uint32      ip_proto    = 0;
    uint32      mpls_label[8] = {0};
    uint32      mpls_label_num = 0;
    uint32      arp_op_code  = 0;
    uint32      arp_proto_type = 0;
    uint32      arp_target_ip = 0;
    hw_mac_addr_t arp_target_mac = {0};
    uint32      arp_sender_ip = 0;
    hw_mac_addr_t arp_sender_mac = {0};
    uint32      fcoe_did    = 0;
    uint32      fcoe_sid    = 0;
    uint32      trill_egress_nickname = 0;
    uint32      trill_ingress_nickname = 0;
    uint32      trill_is_esadi = 0;
    uint32      trill_is_bfd_echo = 0;
    uint32      trill_is_bfd = 0;
    uint32      trill_is_channel = 0;
    uint32      trill_bfd_my_discriminator = 0;
    uint32      trill_inner_vlan_id = 0;
    uint32      trill_inner_vlan_valid = 0;
    uint32      trill_length = 0;
    uint32      trill_multi_hop = 0;
    uint32      trill_multi_cast = 0;
    uint32      trill_version = 0;
    uint32      ether_oam_op_code = 0;
    uint32      ether_oam_level = 0;
    uint32      ether_oam_version = 0;
    uint32      slow_proto_code = 0;
    uint32      slow_proto_flags = 0;
    uint32      slow_proto_subtype = 0;
    uint32      ptp_msg = 0;
    uint32      ptp_version = 0;
    uint32      dot1ae_an = 0;
    uint32      dot1ae_es = 0;
    uint32      dot1ae_pn = 0;
    uint32      dot1ae_sc = 0;
    uint32      dot1ae_sci[2] = {0};
    uint32      dot1ae_sl = 0;
    uint32      dot1ae_unknow = 0;
    uint32      satpdu_mef_oui = 0;
    uint32      satpdu_oui_subtype = 0;
    uint32      satpdu_pdu_byte[2] = {0};
    uint32      flex_data[2] = {0};
    uint32      udf_valid = 0;
    uint32      udf_hit_index = 0;
    uint32      parser_valid = 0;
    uint32      second_parser_en = 0;

    DbgParserFromIpeHdrAdjInfo_m parser_result;
    sal_memset(&parser_result, 0, sizeof(DbgParserFromIpeHdrAdjInfo_m));
    if(0 == type)       /*ipe 1st parser*/
    {
        cmd = DRV_IOR(DbgParserFromIpeHdrAdjInfo_t, DRV_ENTRY_FLAG);
    }
    else if(1 == type)  /*ipe 2nd parser*/
    {
        cmd = DRV_IOR(DbgIpeMplsDecapInfo_t, DbgIpeMplsDecapInfo_secondParserEn_f);
        DRV_IOCTL(lchip, slice, cmd, &second_parser_en);
        if(!second_parser_en)
        {
            return CLI_SUCCESS;
        }
        cmd = DRV_IOR(DbgParserFromIpeIntfInfo_t, DRV_ENTRY_FLAG);
    }
    else if(2 == type)  /*epe parser*/
    {
        cmd = DRV_IOR(DbgParserFromEpeHdrAdjInfo_t, DRV_ENTRY_FLAG);
    }
    DRV_IOCTL(lchip, slice, cmd, &parser_result);

    GetDbgParserFromIpeHdrAdjInfo(A, valid_f, &parser_result, &parser_valid);
    if(!parser_valid)
    {
        return CLI_SUCCESS;
    }

    CTC_DKIT_PRINT("%s Parser Result:\n", str_parser[type]);

    if (DRV_IS_DUET2(lchip))
    {
        GetDbgParserFromIpeHdrAdjInfo(A, macDa_f,        &parser_result, &hw_mac_da);
        GetDbgParserFromIpeHdrAdjInfo(A, macSa_f,        &parser_result, &hw_mac_sa);
        GetDbgParserFromIpeHdrAdjInfo(A, svlanIdValid_f, &parser_result, &svlan_valid);
        GetDbgParserFromIpeHdrAdjInfo(A, svlanId_f,      &parser_result, &svlan_id);
        GetDbgParserFromIpeHdrAdjInfo(A, stagCos_f,      &parser_result, &stag_cos);
        GetDbgParserFromIpeHdrAdjInfo(A, stagCfi_f,      &parser_result, &stag_cfi);
        GetDbgParserFromIpeHdrAdjInfo(A, cvlanIdValid_f, &parser_result, &cvlan_valid);
        GetDbgParserFromIpeHdrAdjInfo(A, cvlanId_f,      &parser_result, &cvlan_id);
        GetDbgParserFromIpeHdrAdjInfo(A, ctagCos_f,      &parser_result, &ctag_cos);
        GetDbgParserFromIpeHdrAdjInfo(A, ctagCfi_f,      &parser_result, &ctag_cfi);
        GetDbgParserFromIpeHdrAdjInfo(A, etherType_f,                    &parser_result, &ether_type);
        GetDbgParserFromIpeHdrAdjInfo(A, layer3Type_f,                   &parser_result, &layer3_type);
        GetDbgParserFromIpeHdrAdjInfo(A, layer4Type_f,                   &parser_result, &layer4_type);
        GetDbgParserFromIpeHdrAdjInfo(A, layer3HeaderProtocol_f,         &parser_result, &ip_proto);
        GetDbgParserFromIpeHdrAdjInfo(A, uL4Source_gGre_greFlags_f,      &parser_result, &gre_flags);
        GetDbgParserFromIpeHdrAdjInfo(A, uL4Dest_gGre_greProtocolType_f, &parser_result, &gre_proto_type);
        GetDbgParserFromIpeHdrAdjInfo(A, uL4Dest_gPort_l4DestPort_f,     &parser_result, &l4_dst_port);
        GetDbgParserFromIpeHdrAdjInfo(A, uL4Source_gPort_l4SourcePort_f, &parser_result, &l4_src_port);
        /*L3TYPE_IPV4*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gIpv4_ipDa_f,   &parser_result, &ip4_da);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Source_gIpv4_ipSa_f, &parser_result, &ip4_sa);
        /*L3TYPE_IPV6*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gIpv6_ipDa_f,   &parser_result, &hw_ip6_da);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Source_gIpv6_ipSa_f, &parser_result, &hw_ip6_sa);
        /*L3TYPE_MPLS*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gMpls_mplsLabel0_f,   &parser_result, &mpls_label[0]);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gMpls_mplsLabel1_f,   &parser_result, &mpls_label[1]);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gMpls_mplsLabel2_f,   &parser_result, &mpls_label[2]);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gMpls_mplsLabel3_f,   &parser_result, &mpls_label[3]);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Source_gMpls_mplsLabel4_f, &parser_result, &mpls_label[4]);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Source_gMpls_mplsLabel5_f, &parser_result, &mpls_label[5]);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Source_gMpls_mplsLabel6_f, &parser_result, &mpls_label[6]);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Source_gMpls_mplsLabel7_f, &parser_result, &mpls_label[7]);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Tos_gMpls_labelNum_f,      &parser_result, &mpls_label_num);
        /*L3TYPE_MPLSUP*/

        /*L3TYPE_ARP*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gArp_arpOpCode_f,      &parser_result, &arp_op_code);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Source_gArp_protocolType_f, &parser_result, &arp_proto_type);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gArp_targetIp_f,       &parser_result, &arp_target_ip);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gArp_targetMac_f,      &parser_result, &arp_target_mac);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Source_gArp_senderIp_f,     &parser_result, &arp_sender_ip);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Source_gArp_senderMac_f,    &parser_result, &arp_sender_mac);
        /*L3TYPE_FCOE*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gFcoe_fcoeDid_f,    &parser_result, &fcoe_did);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Source_gFcoe_fcoeSid_f,  &parser_result, &fcoe_sid);
        /*L3TYPE_TRILL*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_egressNickname_f         , &parser_result, &trill_egress_nickname);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Source_gTrill_ingressNickname_f      , &parser_result, &trill_ingress_nickname);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_isEsadi_f                , &parser_result, &trill_is_esadi);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_isTrillBfdEcho_f         , &parser_result, &trill_is_bfd_echo);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_isTrillBfd_f             , &parser_result, &trill_is_bfd);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_isTrillChannel_f         , &parser_result, &trill_is_channel);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_trillBfdMyDiscriminator_f, &parser_result, &trill_bfd_my_discriminator);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_trillInnerVlanId_f       , &parser_result, &trill_inner_vlan_id);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_trillInnerVlanValid_f    , &parser_result, &trill_inner_vlan_valid);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_trillLength_f            , &parser_result, &trill_length);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_trillMultiHop_f          , &parser_result, &trill_multi_hop);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_trillMulticast_f         , &parser_result, &trill_multi_cast);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gTrill_trillVersion_f           , &parser_result, &trill_version);
        /*L3TYPE_ETHEROAM*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gEtherOam_etherOamLevel_f,   &parser_result, &ether_oam_level);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gEtherOam_etherOamOpCode_f,  &parser_result, &ether_oam_op_code);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gEtherOam_etherOamVersion_f, &parser_result, &ether_oam_version);
        /*L3TYPE_SLOWPROTO*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gSlowProto_slowProtocolCode_f,    &parser_result, &slow_proto_code);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gSlowProto_slowProtocolFlags_f,   &parser_result, &slow_proto_flags);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gSlowProto_slowProtocolSubType_f, &parser_result, &slow_proto_subtype);
        /*L3TYPE_CMAC*/

        /*L3TYPE_PTP*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gPtp_ptpMessageType_f, &parser_result, &ptp_msg);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gPtp_ptpVersion_f,     &parser_result, &ptp_version);
        /*L3TYPE_DOT1AE*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gDot1Ae_secTagAn_f,            &parser_result, &dot1ae_an);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gDot1Ae_secTagEs_f,            &parser_result, &dot1ae_es);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gDot1Ae_secTagPn_f,            &parser_result, &dot1ae_pn);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gDot1Ae_secTagSc_f,            &parser_result, &dot1ae_sc);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gDot1Ae_secTagSci_f,           &parser_result, &dot1ae_sci);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gDot1Ae_secTagSl_f,            &parser_result, &dot1ae_sl);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gDot1Ae_unknownDot1AePacket_f, &parser_result, &dot1ae_unknow);
        /*L3TYPE_SATPDU*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gSatPdu_mefOui_f,     &parser_result, &satpdu_mef_oui);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gSatPdu_ouiSubType_f, &parser_result, &satpdu_oui_subtype);
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gSatPdu_pduByte_f,    &parser_result, &satpdu_pdu_byte);
        /*L3TYPE_FLEXIBLE*/
        GetDbgParserFromIpeHdrAdjInfo(A, uL3Dest_gFlexL3_flexData_f,    &parser_result, &flex_data);
        /*UDF*/
        GetDbgParserFromIpeHdrAdjInfo(A, udfValid_f,    &parser_result, &udf_valid);
        GetDbgParserFromIpeHdrAdjInfo(A, udfHitIndex_f, &parser_result, &udf_hit_index);
    }
    else
    {
        ParserResult_m parser_result_tmp;
        GetDbgParserFromIpeHdrAdjInfo(A, data_f, &parser_result, &parser_result_tmp);

        GetParserResult(A, macDa_f,        &parser_result_tmp, &hw_mac_da);
        GetParserResult(A, macSa_f,        &parser_result_tmp, &hw_mac_sa);
        GetParserResult(A, svlanIdValid_f, &parser_result_tmp, &svlan_valid);
        GetParserResult(A, svlanId_f,      &parser_result_tmp, &svlan_id);
        GetParserResult(A, stagCos_f,      &parser_result_tmp, &stag_cos);
        GetParserResult(A, stagCfi_f,      &parser_result_tmp, &stag_cfi);
        GetParserResult(A, cvlanIdValid_f, &parser_result_tmp, &cvlan_valid);
        GetParserResult(A, cvlanId_f,      &parser_result_tmp, &cvlan_id);
        GetParserResult(A, ctagCos_f,      &parser_result_tmp, &ctag_cos);
        GetParserResult(A, ctagCfi_f,      &parser_result_tmp, &ctag_cfi);
        GetParserResult(A, etherType_f,                    &parser_result_tmp, &ether_type);
        GetParserResult(A, layer3Type_f,                   &parser_result_tmp, &layer3_type);
        GetParserResult(A, layer4Type_f,                   &parser_result_tmp, &layer4_type);
        GetParserResult(A, layer3HeaderProtocol_f,         &parser_result_tmp, &ip_proto);
        GetParserResult(A, uL4Source_gGre_greFlags_f,      &parser_result_tmp, &gre_flags);
        GetParserResult(A, uL4Dest_gGre_greProtocolType_f, &parser_result_tmp, &gre_proto_type);
        GetParserResult(A, uL4Dest_gPort_l4DestPort_f,     &parser_result_tmp, &l4_dst_port);
        GetParserResult(A, uL4Source_gPort_l4SourcePort_f, &parser_result_tmp, &l4_src_port);
        /*L3TYPE_IPV4*/
        GetParserResult(A, uL3Dest_gIpv4_ipDa_f,   &parser_result_tmp, &ip4_da);
        GetParserResult(A, uL3Source_gIpv4_ipSa_f, &parser_result_tmp, &ip4_sa);
        /*L3TYPE_IPV6*/
        GetParserResult(A, uL3Dest_gIpv6_ipDa_f,   &parser_result_tmp, &hw_ip6_da);
        GetParserResult(A, uL3Source_gIpv6_ipSa_f, &parser_result_tmp, &hw_ip6_sa);
        /*L3TYPE_MPLS*/
        GetParserResult(A, uL3Dest_gMpls_mplsLabel0_f,   &parser_result_tmp, &mpls_label[0]);
        GetParserResult(A, uL3Dest_gMpls_mplsLabel1_f,   &parser_result_tmp, &mpls_label[1]);
        GetParserResult(A, uL3Dest_gMpls_mplsLabel2_f,   &parser_result_tmp, &mpls_label[2]);
        GetParserResult(A, uL3Dest_gMpls_mplsLabel3_f,   &parser_result_tmp, &mpls_label[3]);
        GetParserResult(A, uL3Source_gMpls_mplsLabel4_f, &parser_result_tmp, &mpls_label[4]);
        GetParserResult(A, uL3Source_gMpls_mplsLabel5_f, &parser_result_tmp, &mpls_label[5]);
        GetParserResult(A, uL3Source_gMpls_mplsLabel6_f, &parser_result_tmp, &mpls_label[6]);
        GetParserResult(A, uL3Source_gMpls_mplsLabel7_f, &parser_result_tmp, &mpls_label[7]);
        GetParserResult(A, uL3Tos_gMpls_labelNum_f,      &parser_result_tmp, &mpls_label_num);
        /*L3TYPE_MPLSUP*/

        /*L3TYPE_ARP*/
        GetParserResult(A, uL3Dest_gArp_arpOpCode_f,      &parser_result_tmp, &arp_op_code);
        GetParserResult(A, uL3Source_gArp_protocolType_f, &parser_result_tmp, &arp_proto_type);
        GetParserResult(A, uL3Dest_gArp_targetIp_f,       &parser_result_tmp, &arp_target_ip);
        GetParserResult(A, uL3Dest_gArp_targetMac_f,      &parser_result_tmp, &arp_target_mac);
        GetParserResult(A, uL3Source_gArp_senderIp_f,     &parser_result_tmp, &arp_sender_ip);
        GetParserResult(A, uL3Source_gArp_senderMac_f,    &parser_result_tmp, &arp_sender_mac);
        /*L3TYPE_FCOE*/
        GetParserResult(A, uL3Dest_gFcoe_fcoeDid_f,    &parser_result_tmp, &fcoe_did);
        GetParserResult(A, uL3Source_gFcoe_fcoeSid_f,  &parser_result_tmp, &fcoe_sid);
        /*L3TYPE_TRILL*/
        GetParserResult(A, uL3Dest_gTrill_egressNickname_f         , &parser_result_tmp, &trill_egress_nickname);
        GetParserResult(A, uL3Source_gTrill_ingressNickname_f      , &parser_result_tmp, &trill_ingress_nickname);
        GetParserResult(A, uL3Dest_gTrill_isEsadi_f                , &parser_result_tmp, &trill_is_esadi);
        GetParserResult(A, uL3Dest_gTrill_isTrillBfdEcho_f         , &parser_result_tmp, &trill_is_bfd_echo);
        GetParserResult(A, uL3Dest_gTrill_isTrillBfd_f             , &parser_result_tmp, &trill_is_bfd);
        GetParserResult(A, uL3Dest_gTrill_isTrillChannel_f         , &parser_result_tmp, &trill_is_channel);
        GetParserResult(A, uL3Dest_gTrill_trillBfdMyDiscriminator_f, &parser_result_tmp, &trill_bfd_my_discriminator);
        GetParserResult(A, uL3Dest_gTrill_trillInnerVlanId_f       , &parser_result_tmp, &trill_inner_vlan_id);
        GetParserResult(A, uL3Dest_gTrill_trillInnerVlanValid_f    , &parser_result_tmp, &trill_inner_vlan_valid);
        GetParserResult(A, uL3Dest_gTrill_trillLength_f            , &parser_result_tmp, &trill_length);
        GetParserResult(A, uL3Dest_gTrill_trillMultiHop_f          , &parser_result_tmp, &trill_multi_hop);
        GetParserResult(A, uL3Dest_gTrill_trillMulticast_f         , &parser_result_tmp, &trill_multi_cast);
        GetParserResult(A, uL3Dest_gTrill_trillVersion_f           , &parser_result_tmp, &trill_version);
        /*L3TYPE_ETHEROAM*/
        GetParserResult(A, uL3Dest_gEtherOam_etherOamLevel_f,   &parser_result_tmp, &ether_oam_level);
        GetParserResult(A, uL3Dest_gEtherOam_etherOamOpCode_f,  &parser_result_tmp, &ether_oam_op_code);
        GetParserResult(A, uL3Dest_gEtherOam_etherOamVersion_f, &parser_result_tmp, &ether_oam_version);
        /*L3TYPE_SLOWPROTO*/
        GetParserResult(A, uL3Dest_gSlowProto_slowProtocolCode_f,    &parser_result_tmp, &slow_proto_code);
        GetParserResult(A, uL3Dest_gSlowProto_slowProtocolFlags_f,   &parser_result_tmp, &slow_proto_flags);
        GetParserResult(A, uL3Dest_gSlowProto_slowProtocolSubType_f, &parser_result_tmp, &slow_proto_subtype);
        /*L3TYPE_CMAC*/

        /*L3TYPE_PTP*/
        GetParserResult(A, uL3Dest_gPtp_ptpMessageType_f, &parser_result_tmp, &ptp_msg);
        GetParserResult(A, uL3Dest_gPtp_ptpVersion_f,     &parser_result_tmp, &ptp_version);
        /*L3TYPE_DOT1AE*/
        GetParserResult(A, uL3Dest_gDot1Ae_secTagAn_f,            &parser_result_tmp, &dot1ae_an);
        GetParserResult(A, uL3Dest_gDot1Ae_secTagEs_f,            &parser_result_tmp, &dot1ae_es);
        GetParserResult(A, uL3Dest_gDot1Ae_secTagPn_f,            &parser_result_tmp, &dot1ae_pn);
        GetParserResult(A, uL3Dest_gDot1Ae_secTagSc_f,            &parser_result_tmp, &dot1ae_sc);
        GetParserResult(A, uL3Dest_gDot1Ae_secTagSci_f,           &parser_result_tmp, &dot1ae_sci);
        GetParserResult(A, uL3Dest_gDot1Ae_secTagSl_f,            &parser_result_tmp, &dot1ae_sl);
        GetParserResult(A, uL3Dest_gDot1Ae_unknownDot1AePacket_f, &parser_result_tmp, &dot1ae_unknow);
        /*L3TYPE_SATPDU*/
        GetParserResult(A, uL3Dest_gSatPdu_mefOui_f,     &parser_result_tmp, &satpdu_mef_oui);
        GetParserResult(A, uL3Dest_gSatPdu_ouiSubType_f, &parser_result_tmp, &satpdu_oui_subtype);
        GetParserResult(A, uL3Dest_gSatPdu_pduByte_f,    &parser_result_tmp, &satpdu_pdu_byte);
        /*L3TYPE_FLEXIBLE*/
        GetParserResult(A, uL3Dest_gFlexL3_flexData_f,    &parser_result_tmp, &flex_data);
        /*UDF*/
        GetParserResult(A, udfValid_f,    &parser_result_tmp, &udf_valid);
        GetParserResult(A, udfHitIndex_f, &parser_result_tmp, &udf_hit_index);
    }

    CTC_DKIT_SET_USER_MAC(mac_address, hw_mac_da);
    CTC_DKIT_PATH_PRINT("mac-da:           %.4x.%.4x.%.4x\n", sal_ntohs(*(unsigned short*)&mac_address[0]),
                            sal_ntohs(*(unsigned short*)&mac_address[2]),
                            sal_ntohs(*(unsigned short*)&mac_address[4]));
    CTC_DKIT_SET_USER_MAC(mac_address, hw_mac_sa);
    CTC_DKIT_PATH_PRINT("mac-sa:           %.4x.%.4x.%.4x\n", sal_ntohs(*(unsigned short*)&mac_address[0]),
                            sal_ntohs(*(unsigned short*)&mac_address[2]),
                            sal_ntohs(*(unsigned short*)&mac_address[4]));
    if(svlan_valid)
    {
        CTC_DKIT_PATH_PRINT("svlan id:         %d\n", svlan_id);
        CTC_DKIT_PATH_PRINT("stag-cos:         %d\n", stag_cos);
    }
    if(cvlan_valid)
    {
        CTC_DKIT_PATH_PRINT("cvlan id:         %d\n", cvlan_id);
        CTC_DKIT_PATH_PRINT("ctag-cos:         %d\n", ctag_cos);
    }
    CTC_DKIT_PATH_PRINT("ether-type:       0x%04X\n", ether_type);
    CTC_DKIT_PATH_PRINT("layer3-type:      L3TYPE_%s\n", str_l3_type[layer3_type]);
    switch(layer3_type)
    {
    case L3TYPE_IPV4:
        temp_data = sal_ntohl(ip4_da);
        sal_inet_ntop(AF_INET, &temp_data, buf, CTC_DKIT_IPV6_ADDR_STR_LEN);
        CTC_DKIT_PATH_PRINT("ip-da:            %-30s\n", buf);
        temp_data = sal_ntohl(ip4_sa);
        sal_inet_ntop(AF_INET, &temp_data, buf, CTC_DKIT_IPV6_ADDR_STR_LEN);
        CTC_DKIT_PATH_PRINT("ip-sa:            %-30s\n", buf);
        CTC_DKIT_PATH_PRINT("ip-protocol:      %u\n", ip_proto);
        break;
    case L3TYPE_IPV6:
        ip6_address[0] = sal_ntohl(hw_ip6_da[3]);
        ip6_address[1] = sal_ntohl(hw_ip6_da[2]);
        ip6_address[2] = sal_ntohl(hw_ip6_da[1]);
        ip6_address[3] = sal_ntohl(hw_ip6_da[0]);
        sal_inet_ntop(AF_INET6, ip6_address, buf, CTC_DKIT_IPV6_ADDR_STR_LEN);
        CTC_DKIT_PATH_PRINT("ip6-da:           %-44s\n", buf);
        ip6_address[0] = sal_ntohl(hw_ip6_sa[3]);
        ip6_address[1] = sal_ntohl(hw_ip6_sa[2]);
        ip6_address[2] = sal_ntohl(hw_ip6_sa[1]);
        ip6_address[3] = sal_ntohl(hw_ip6_sa[0]);
        sal_inet_ntop(AF_INET6, ip6_address, buf, CTC_DKIT_IPV6_ADDR_STR_LEN);
        CTC_DKIT_PATH_PRINT("ip6-sa:           %-44s\n", buf);
        CTC_DKIT_PATH_PRINT("ip-protocol:      %u\n", ip_proto);
        break;
    case L3TYPE_MPLS:
        CTC_DKIT_PATH_PRINT("label-num:        %d\n", mpls_label_num);
        for(idx1 = 0; idx1 < mpls_label_num; idx1++)
        {
            CTC_DKIT_PATH_PRINT("mpls-label%d:      0x%08X (label:%u)\n", idx1, mpls_label[idx1], (mpls_label[idx1] >> 12));
        }
        break;
    case L3TYPE_MPLSUP:
        break;
    case L3TYPE_ARP:
        CTC_DKIT_PATH_PRINT("arp-op-code:      0x%04x\n", arp_op_code);
        CTC_DKIT_PATH_PRINT("arp-proto-type:   0x%04x\n", arp_proto_type);
        temp_data = sal_ntohl(arp_target_ip);
        sal_inet_ntop(AF_INET, &temp_data, buf, CTC_DKIT_IPV6_ADDR_STR_LEN);
        CTC_DKIT_PATH_PRINT("arp-target-ip:    %-30s\n", buf);
        temp_data = sal_ntohl(arp_sender_ip);
        sal_inet_ntop(AF_INET, &temp_data, buf, CTC_DKIT_IPV6_ADDR_STR_LEN);
        CTC_DKIT_PATH_PRINT("arp-sender-ip:    %-30s\n", buf);
        CTC_DKIT_SET_USER_MAC(mac_address, arp_target_mac);
        CTC_DKIT_PATH_PRINT("arp-target-mac:   %.4x.%.4x.%.4x\n", sal_ntohs(*(unsigned short*)&mac_address[0]),
                                sal_ntohs(*(unsigned short*)&mac_address[2]),
                                sal_ntohs(*(unsigned short*)&mac_address[4]));
        CTC_DKIT_SET_USER_MAC(mac_address, arp_sender_mac);
        CTC_DKIT_PATH_PRINT("arp-sender-mac:   %.4x.%.4x.%.4x\n", sal_ntohs(*(unsigned short*)&mac_address[0]),
                                sal_ntohs(*(unsigned short*)&mac_address[2]),
                                sal_ntohs(*(unsigned short*)&mac_address[4]));
        break;
    case L3TYPE_FCOE:
        CTC_DKIT_PATH_PRINT("fcoe-did:         %u\n", fcoe_did);
        CTC_DKIT_PATH_PRINT("fcoe-sid:         %u\n", fcoe_sid);
        break;
    case L3TYPE_TRILL:
        CTC_DKIT_PATH_PRINT("ingress-nickname: %u\n", trill_ingress_nickname);
        CTC_DKIT_PATH_PRINT("egress-nickname:  %u\n", trill_egress_nickname);
        CTC_DKIT_PATH_PRINT("is-bfd-echo:      %u\n", trill_is_bfd_echo);
        CTC_DKIT_PATH_PRINT("trill-isesadi:    %u\n", trill_is_esadi);
        CTC_DKIT_PATH_PRINT("is-bfd-echo:      %u\n", trill_is_bfd_echo);
        CTC_DKIT_PATH_PRINT("is-bfd:           %u\n", trill_is_bfd);
        CTC_DKIT_PATH_PRINT("is-channel:       %u\n", trill_is_channel);
        CTC_DKIT_PATH_PRINT("bfd-my-discrim:   %u\n", trill_bfd_my_discriminator);
        CTC_DKIT_PATH_PRINT("inner-vlan-id:    %u\n", trill_inner_vlan_id);
        CTC_DKIT_PATH_PRINT("inner-vlan-valid: %u\n", trill_inner_vlan_valid);
        CTC_DKIT_PATH_PRINT("trill-length:     %u\n", trill_length);
        CTC_DKIT_PATH_PRINT("multi-hop:        %u\n", trill_multi_hop);
        CTC_DKIT_PATH_PRINT("trill-multicast:  %u\n", trill_multi_cast);
        CTC_DKIT_PATH_PRINT("trill-version:    %u\n", trill_version);
        break;
    case L3TYPE_ETHEROAM:
        CTC_DKIT_PATH_PRINT("ether-oam-level:  %u\n", ether_oam_level);
        CTC_DKIT_PATH_PRINT("ether-oam-op-code:%u\n", ether_oam_op_code);
        CTC_DKIT_PATH_PRINT("ether-oam-version:%u\n", ether_oam_version);
        break;
    case L3TYPE_SLOWPROTO:
        CTC_DKIT_PATH_PRINT("slow-proto-code:  %u\n", slow_proto_code);
        CTC_DKIT_PATH_PRINT("slow-proto-flags: %u\n", slow_proto_flags);
        CTC_DKIT_PATH_PRINT("slow-proto-subtype:%u\n", slow_proto_subtype);
        break;
    case L3TYPE_CMAC:
        break;
    case L3TYPE_PTP:
        CTC_DKIT_PATH_PRINT("ptp-message:      %u\n", ptp_msg);
        CTC_DKIT_PATH_PRINT("ptp-version:      %u\n", ptp_version);
        break;
    case L3TYPE_DOT1AE:
        CTC_DKIT_PATH_PRINT("dot1ae-an:        %u\n", dot1ae_an);
        CTC_DKIT_PATH_PRINT("dot1ae-es:        %u\n", dot1ae_es);
        CTC_DKIT_PATH_PRINT("dot1ae-pn:        %u\n", dot1ae_pn);
        CTC_DKIT_PATH_PRINT("dot1ae-sc:        %u\n", dot1ae_sc);
        CTC_DKIT_PATH_PRINT("dot1ae-sci:       0x%08x%08x\n", dot1ae_sci[1], dot1ae_sci[0]);
        CTC_DKIT_PATH_PRINT("dot1ae-sl:        %u\n", dot1ae_sl);
        CTC_DKIT_PATH_PRINT("dot1ae-unknow:    %u\n", dot1ae_unknow);
        break;
    case L3TYPE_SATPDU:
        CTC_DKIT_PATH_PRINT("satpdu-mef-oui:   %u\n", satpdu_mef_oui);
        CTC_DKIT_PATH_PRINT("satpdu-oui-subtype:%u\n", satpdu_oui_subtype);
        CTC_DKIT_PATH_PRINT("satpdu-pdu-byte:  0x%08x%08x\n", satpdu_pdu_byte[1], satpdu_pdu_byte[0]);
        break;
    case L3TYPE_FLEXIBLE:
        CTC_DKIT_PATH_PRINT("flex-data:        0x%08x%08x\n", satpdu_pdu_byte[1], satpdu_pdu_byte[0]);
        break;
    default:
        break;
    }
    if(L4TYPE_GRE == layer4_type)
    {
        CTC_DKIT_PATH_PRINT("gre-flags:        0x%x\n", gre_flags);
        CTC_DKIT_PATH_PRINT("gre-proto-type:   0x%x\n", gre_proto_type);
    }
    else if((layer4_type == L4TYPE_TCP) || (layer4_type == L4TYPE_UDP) || (layer4_type == L4TYPE_RDP)
                || (layer4_type == L4TYPE_SCTP) || (layer4_type == L4TYPE_DCCP))
    {
        CTC_DKIT_PATH_PRINT("l4-src-port:      %u\n", l4_src_port);
        CTC_DKIT_PATH_PRINT("l4-dst-port:      %u\n", l4_dst_port);
    }

    CTC_DKIT_PATH_PRINT("\nudf-valid:      %u\n", udf_valid);
    if (udf_valid)
    {
        CTC_DKIT_PATH_PRINT("udf-hit-index:   %u\n", udf_hit_index);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_capture_path_ipe_bridge(ctc_dkit_captured_path_info_t* p_info)
{
    uint32 cmd = 0;
    uint32 discard = 0;
    uint32 discard_type = 0;
    uint32 exceptionEn = 0;
    uint32 exceptionIndex = 0;
    uint32 exceptionSubIndex = 0;

    /*DbgFibLkpEngineMacDaHashInfo*/
    uint32 macda_blackHoleMacDaResultValid = 0;
    uint32 macda_blackHoleHitMacDaKeyIndex = 0;
    uint32 macda_macDaHashResultValid = 0;
    uint32 macda_macDaHashLookupConflict = 0;
    uint32 macda_hitHashKeyIndexMacDa = 0;
    uint32 macda_valid = 0;
    DbgFibLkpEngineMacDaHashInfo_m dbg_fib_lkp_engine_mac_da_hash_info;

    /*DbgFibLkpEngineMacSaHashInfo*/
    uint32 macsa_blackHoleMacSaResultValid = 0;
    uint32 macsa_blackHoleHitMacSaKeyIndex = 0;
    uint32 macsa_macSaHashResultValid = 0;
    uint32 macsa_macSaHashLookupConflict = 0;
    uint32 macsa_hitHashKeyIndexMacSa = 0;
    uint32 macsa_valid = 0;
    DbgFibLkpEngineMacSaHashInfo_m dbg_fib_lkp_engine_mac_sa_hash_info;

    /*DbgIpeMacBridgingInfo*/
    uint32 bridge_macDaResultValid = 0;
    uint32 bridge_macDaDefaultEntryValid = 0;
    uint32 bridge_macDaIsPortMac = 0;
    uint32 bridge_discard = 0;
    uint32 bridge_discardType = 0;
    uint32 bridge_exceptionEn = 0;
    uint32 bridge_exceptionIndex = 0;
    uint32 bridge_exceptionSubIndex = 0;
    uint32 bridge_bridgePacket = 0;
    uint32 bridge_bridgeEscape = 0;
    uint32 bridge_stormCtlEn = 0;
    uint32 bridge_valid = 0;
    DbgIpeMacBridgingInfo_m dbg_ipe_mac_bridging_info;

    /*DbgIpeMacLearningInfo*/
    uint32 lrn_macSaResultPending = 0;
    uint32 lrn_macSaDefaultEntryValid = 0;
    uint32 lrn_macSaHashConflict = 0;
    uint32 lrn_macSaResultValid = 0;
    uint32 lrn_isInnerMacSaLookup = 0;
    uint32 lrn_fid = 0;
    uint32 lrn_learningSrcPort = 0;
    uint32 lrn_isGlobalSrcPort = 0;
    uint32 lrn_macSecurityDiscard = 0;
    uint32 lrn_learningEn = 0;
    uint32 lrn_discard = 0;
    uint32 lrn_discardType = 0;
    uint32 lrn_valid = 0;
    uint32 lrn_exceptionEn = 0;
    uint32 lrn_exceptionIndex = 0;
    uint32 lrn_exceptionSubIndex = 0;
    DbgIpeMacLearningInfo_m dbg_ipe_mac_learning_info;

    uint32 lkp_macDaLookupEn = 0;
    uint32 fid = 0;
    uint32 sa_fid = 0;
    uint32 ad_index = 0;
    uint32 defaultEntryBase = 0;
    uint32 lkp_macSaLookupEn = 0;
    uint32 lrn_type = 0;
    uint32 aging_ptr = 0;

    drv_acc_in_t  in;
    drv_acc_out_t out;

    uint8 lchip = p_info->lchip;

    sal_memset(&in, 0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));

    /*DbgFibLkpEngineMacDaHashInfo*/
    sal_memset(&dbg_fib_lkp_engine_mac_da_hash_info, 0, sizeof(dbg_fib_lkp_engine_mac_da_hash_info));
    cmd = DRV_IOR(DbgFibLkpEngineMacDaHashInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_fib_lkp_engine_mac_da_hash_info);
    GetDbgFibLkpEngineMacDaHashInfo(A, blackHoleMacDaResultValid_f, &dbg_fib_lkp_engine_mac_da_hash_info, &macda_blackHoleMacDaResultValid);
    GetDbgFibLkpEngineMacDaHashInfo(A, blackHoleHitMacDaKeyIndex_f, &dbg_fib_lkp_engine_mac_da_hash_info, &macda_blackHoleHitMacDaKeyIndex);
    GetDbgFibLkpEngineMacDaHashInfo(A, gMacDaLkpResult_macDaResultValid_f, &dbg_fib_lkp_engine_mac_da_hash_info, &macda_macDaHashResultValid);
    GetDbgFibLkpEngineMacDaHashInfo(A, gMacDaLkpResult_macDaHashConflict_f, &dbg_fib_lkp_engine_mac_da_hash_info, &macda_macDaHashLookupConflict);
    GetDbgFibLkpEngineMacDaHashInfo(A, gMacDaLkpResult_macDaHashKeyHitIndex_f, &dbg_fib_lkp_engine_mac_da_hash_info, &macda_hitHashKeyIndexMacDa);
    GetDbgFibLkpEngineMacDaHashInfo(A, gMacDaLkp_macDaLookupEn_f, &dbg_fib_lkp_engine_mac_da_hash_info, &lkp_macDaLookupEn);
    GetDbgFibLkpEngineMacDaHashInfo(A, valid_f, &dbg_fib_lkp_engine_mac_da_hash_info, &macda_valid);
    GetDbgFibLkpEngineMacDaHashInfo(A, gMacDaLkp_vsiId_f, &dbg_fib_lkp_engine_mac_da_hash_info, &fid);

    /*DbgFibLkpEngineMacSaHashInfo*/
    sal_memset(&dbg_fib_lkp_engine_mac_sa_hash_info, 0, sizeof(dbg_fib_lkp_engine_mac_sa_hash_info));
    cmd = DRV_IOR(DbgFibLkpEngineMacSaHashInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_fib_lkp_engine_mac_sa_hash_info);
    GetDbgFibLkpEngineMacSaHashInfo(A, blackHoleMacSaResultValid_f, &dbg_fib_lkp_engine_mac_sa_hash_info, &macsa_blackHoleMacSaResultValid);
    GetDbgFibLkpEngineMacSaHashInfo(A, blackHoleHitMacSaKeyIndex_f, &dbg_fib_lkp_engine_mac_sa_hash_info, &macsa_blackHoleHitMacSaKeyIndex);
    GetDbgFibLkpEngineMacSaHashInfo(A, gMacSaLkpResult_macSaResultValid_f, &dbg_fib_lkp_engine_mac_sa_hash_info, &macsa_macSaHashResultValid);
    GetDbgFibLkpEngineMacSaHashInfo(A, gMacSaLkpResult_macSaHashConflict_f, &dbg_fib_lkp_engine_mac_sa_hash_info, &macsa_macSaHashLookupConflict);
    GetDbgFibLkpEngineMacSaHashInfo(A, gMacSaLkpResult_macSaHashKeyHitIndex_f, &dbg_fib_lkp_engine_mac_sa_hash_info, &macsa_hitHashKeyIndexMacSa);
    GetDbgFibLkpEngineMacSaHashInfo(A, gMacSaLkp_macSaLookupEn_f, &dbg_fib_lkp_engine_mac_sa_hash_info, &lkp_macSaLookupEn);
    GetDbgFibLkpEngineMacSaHashInfo(A, valid_f, &dbg_fib_lkp_engine_mac_sa_hash_info, &macsa_valid);
    GetDbgFibLkpEngineMacSaHashInfo(A, gMacSaLkp_vsiId_f, &dbg_fib_lkp_engine_mac_sa_hash_info, &sa_fid);

    /*DbgIpeMacBridgingInfo*/
    sal_memset(&dbg_ipe_mac_bridging_info, 0, sizeof(dbg_ipe_mac_bridging_info));
    cmd = DRV_IOR(DbgIpeMacBridgingInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_ipe_mac_bridging_info);
    GetDbgIpeMacBridgingInfo(A, macDaResultValid_f, &dbg_ipe_mac_bridging_info, &bridge_macDaResultValid);
    GetDbgIpeMacBridgingInfo(A, macDaDefaultEntryValid_f, &dbg_ipe_mac_bridging_info, &bridge_macDaDefaultEntryValid);
    GetDbgIpeMacBridgingInfo(A, macDaIsPortMac_f, &dbg_ipe_mac_bridging_info, &bridge_macDaIsPortMac);
    GetDbgIpeMacBridgingInfo(A, discard_f, &dbg_ipe_mac_bridging_info, &bridge_discard);
    GetDbgIpeMacBridgingInfo(A, discardType_f, &dbg_ipe_mac_bridging_info, &bridge_discardType);
    GetDbgIpeMacBridgingInfo(A, exceptionEn_f, &dbg_ipe_mac_bridging_info, &bridge_exceptionEn);
    GetDbgIpeMacBridgingInfo(A, exceptionIndex_f, &dbg_ipe_mac_bridging_info, &bridge_exceptionIndex);
    GetDbgIpeMacBridgingInfo(A, exceptionSubIndex_f, &dbg_ipe_mac_bridging_info, &bridge_exceptionSubIndex);
    GetDbgIpeMacBridgingInfo(A, bridgePacket_f, &dbg_ipe_mac_bridging_info, &bridge_bridgePacket);
    GetDbgIpeMacBridgingInfo(A, bridgeEscape_f, &dbg_ipe_mac_bridging_info, &bridge_bridgeEscape);
    GetDbgIpeMacBridgingInfo(A, stormCtlEn_f, &dbg_ipe_mac_bridging_info, &bridge_stormCtlEn);
    GetDbgIpeMacBridgingInfo(A, valid_f, &dbg_ipe_mac_bridging_info, &bridge_valid);

    /*DbgIpeMacLearningInfo*/
    sal_memset(&dbg_ipe_mac_learning_info, 0, sizeof(dbg_ipe_mac_learning_info));
    cmd = DRV_IOR(DbgIpeMacLearningInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_ipe_mac_learning_info);
    GetDbgIpeMacLearningInfo(A, macSaResultPending_f, &dbg_ipe_mac_learning_info, &lrn_macSaResultPending);
    GetDbgIpeMacLearningInfo(A, macSaDefaultEntryValid_f, &dbg_ipe_mac_learning_info, &lrn_macSaDefaultEntryValid);
    GetDbgIpeMacLearningInfo(A, macSaHashConflict_f, &dbg_ipe_mac_learning_info, &lrn_macSaHashConflict);
    GetDbgIpeMacLearningInfo(A, macSaResultValid_f, &dbg_ipe_mac_learning_info, &lrn_macSaResultValid);
    GetDbgIpeMacLearningInfo(A, isInnerMacSaLookup_f, &dbg_ipe_mac_learning_info, &lrn_isInnerMacSaLookup);
    GetDbgIpeMacLearningInfo(A, learningFid_f, &dbg_ipe_mac_learning_info, &lrn_fid);
    GetDbgIpeMacLearningInfo(A, learningSrcPort_f, &dbg_ipe_mac_learning_info, &lrn_learningSrcPort);
    GetDbgIpeMacLearningInfo(A, learningUseGlobalSrcPort_f, &dbg_ipe_mac_learning_info, &lrn_isGlobalSrcPort);
    GetDbgIpeMacLearningInfo(A, macSecurityDiscard_f, &dbg_ipe_mac_learning_info, &lrn_macSecurityDiscard);
    GetDbgIpeMacLearningInfo(A, learningEn_f, &dbg_ipe_mac_learning_info, &lrn_learningEn);
    GetDbgIpeMacLearningInfo(A, valid_f, &dbg_ipe_mac_learning_info, &lrn_valid);
    GetDbgIpeMacLearningInfo(A, learningType_f, &dbg_ipe_mac_learning_info, &lrn_type);
    GetDbgIpeMacLearningInfo(A, agingIndex_f, &dbg_ipe_mac_learning_info, &aging_ptr);
    GetDbgIpeMacLearningInfo(A, discardType_f, &dbg_ipe_mac_learning_info, &lrn_discardType);
    GetDbgIpeMacLearningInfo(A, discard_f, &dbg_ipe_mac_learning_info, &lrn_discard);
    GetDbgIpeMacLearningInfo(A, exceptionEn_f, &dbg_ipe_mac_learning_info, &lrn_exceptionEn);
    GetDbgIpeMacLearningInfo(A, exceptionIndex_f, &dbg_ipe_mac_learning_info, &lrn_exceptionIndex);
    GetDbgIpeMacLearningInfo(A, exceptionSubIndex_f, &dbg_ipe_mac_learning_info, &lrn_exceptionSubIndex);

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    /*bridge process*/
    if (bridge_valid)
    {
        if (lkp_macDaLookupEn && bridge_bridgePacket)
        {
            CTC_DKIT_PRINT("Bridge Process:\n");
            CTC_DKIT_PATH_PRINT("%-15s\n", "MAC DA lookup--->");
            if (bridge_macDaResultValid)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%d\n", "FID", fid);
                if (macda_blackHoleMacDaResultValid)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", "hit black hole entry");
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", macda_blackHoleHitMacDaKeyIndex);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", "DsFibMacBlackHoleHashKey");
                    cmd = DRV_IOR(DsFibMacBlackHoleHashKey_t, DsFibMacBlackHoleHashKey_dsAdIndex_f);
                    DRV_IOCTL(lchip, macda_blackHoleHitMacDaKeyIndex, cmd, &ad_index);
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "AD index", ad_index);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", "DsMac");
                }
                else if (bridge_macDaDefaultEntryValid)
                {
                    cmd = DRV_IOR(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gMacDaLookupResultCtl_defaultEntryBase_f);
                    DRV_IOCTL(lchip, 0, cmd, &defaultEntryBase);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_DEFAULT);
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "AD index", (fid + (defaultEntryBase << 8)));
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", "DsMac");
                }
                else if (macda_valid && macda_macDaHashResultValid)
                {

                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", macda_hitHashKeyIndexMacDa);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", "DsFibHost0MacHashKey");

                    in.type = DRV_ACC_TYPE_LOOKUP;
                    in.tbl_id = DsFibHost0MacHashKey_t;
                    in.index = macda_hitHashKeyIndexMacDa;
                    in.op_type = DRV_ACC_OP_BY_INDEX;
                    drv_acc_api(lchip, &in, &out);
                    drv_get_field(lchip, DsFibHost0MacHashKey_t, DsFibHost0MacHashKey_dsAdIndex_f, out.data, &ad_index);
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "AD index", ad_index);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", "DsMac");
                }
                else
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
                }
            }
            else
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
            }
        }

        if (lrn_valid && lkp_macSaLookupEn&&bridge_bridgePacket)
        {
            CTC_DKIT_PATH_PRINT("%-15s\n", "MAC SA lookup--->");
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "FID", sa_fid);
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Aging ptr", aging_ptr);

            if (lrn_macSaDefaultEntryValid)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_DEFAULT);
            }
            else if (lrn_macSaHashConflict)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_CONFLICT);
            }
            else if (lrn_macSaResultValid)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
            }
            else
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
            }
            if (lrn_learningEn)
            {
                CTC_DKIT_PATH_PRINT("%-15s\n", "MAC SA learning--->");
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Learning Mode", lrn_type?"Hw Learning":"Sw Leaning");
                if (lrn_isInnerMacSaLookup)
                {
                    CTC_DKIT_PATH_PRINT("%-15s\n", "Inner MAC SA learning--->");
                }

                CTC_DKIT_PATH_PRINT("%-15s:%d\n", "FID", lrn_fid);
                if (!lrn_isGlobalSrcPort)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "logic port", lrn_learningSrcPort);
                }
                else
                {
                    CTC_DKIT_PATH_PRINT("%-15s:0x%04x\n", "source gport", CTC_DKIT_DRV_GPORT_TO_CTC_GPORT(lrn_learningSrcPort));
                }
            }
        }

    }

    if (bridge_discard)
    {
        discard = 1;
        discard_type = bridge_discardType;
    }
    else if (lrn_discard)
    {
        discard = 1;
        discard_type = lrn_discardType;
    }

    if (bridge_exceptionEn)
    {
        exceptionEn = 1;
        exceptionIndex = bridge_exceptionIndex;
        exceptionSubIndex = bridge_exceptionSubIndex;
    }
    else if (lrn_exceptionEn)
    {
        exceptionEn = 1;
        exceptionIndex = lrn_exceptionIndex;
        exceptionSubIndex = lrn_exceptionSubIndex;
    }

Discard:
    _ctc_usw_dkit_captured_path_discard_process(p_info, discard, discard_type, CTC_DKIT_IPE);
    _ctc_usw_dkit_captured_path_exception_process(p_info, exceptionEn, exceptionIndex, exceptionSubIndex);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_capture_path_ipe_route(ctc_dkit_captured_path_info_t* p_info)
{
    uint32 discard = 0;
    uint32 discard_type = 0;
    uint32 exceptionEn = 0;
    uint32 exceptionIndex = 0;
    uint32 exceptionSubIndex = 0;
    uint32 cmd = 0;
    uint32 cmd1 = 0;
    uint32 key_index = 0;
    uint32 mapped_key_index = 0;
    uint32 mapped_ad_index = 0;
    uint32 route_l3DaLookupEn = 0;
    uint32 route_l3SaLookupEn = 0;
    uint32 route_l3DaResultValid = 0;
    uint32 route_l3SaResultValid = 0;
    uint32 route_flowForceRouteUseIpSa = 0;
    uint32 route_ipMartianAddress = 0;
    uint32 route_ipLinkScopeAddress = 0;
    uint32 route_packetTtl = 0;
    uint32 route_shareType = 0;
    uint32 route_layer3Exception = 0;
    uint32 route_discard = 0;
    uint32 route_discardType = 0;
    uint32 route_exceptionEn = 0;
    uint32 route_exceptionIndex = 0;
    uint32 route_exceptionSubIndex = 0;
    uint32 route_fatalExceptionValid = 0;
    uint32 route_fatalException = 0;
    uint32 route_payloadPacketType = 0;
    uint32 ip_headerError = 0;
    uint32 route_escape = 0;
    uint32 route_valid = 0;
    DbgIpeIpRoutingInfo_m dbg_ipe_ip_routing_info;
    uint32 key_table_id = 0;
    uint32 ad_table_id = 0;

    /*DbgFibLkpEnginel3DaHashInfo*/
    uint32 l3Da_first_hash_ResultValid = 0;
    uint32 l3Da_second_hash_ResultValid = 0;
    uint32 l3Da_first_hitHashKeyIndexl3Da = 0;
    uint32 l3Da_second_hitHashKeyIndexl3Da = 0;
    uint32 l3Da_hashvalid = 0;
    uint32 lpmtcam_lkp1_pri_valid = 0;
    uint32 lpmtcam_lkp1_pub_valid = 0;
    uint32 lpmtcam_lkp2_pri_valid = 0;
    uint32 lpmtcam_lkp2_pub_valid = 0;
    DbgFibLkpEnginel3DaHashInfo_m dbg_fib_lkp_enginel3_da_hash_info;

    /*DbgFibLkpEnginel3SaHashInfo*/
    uint32 l3Sa_l3SaHashResultValid = 0;
    uint32 l3Sa_hitHashKeyIndexl3Sa = 0;
    uint32 l3Sa_hitUrpfHashKeyIndexl3Sa = 0;
    uint32 l3Sa_hitTcamKeyIndexl3Sa = 0;
    uint32 l3Sa_valid = 0;
    uint32 l3Sa_key_type = 0;
    uint32 l3Sa_nat_tcam = 0;
    uint32 l3Sa_rpf_tcam = 0;

    DbgFibLkpEnginel3SaHashInfo_m dbg_fib_lkp_enginel3_sa_hash_info;
    DbgFibLkpEngineHostUrpfHashInfo_m dbg_fib_lkp_enginel3_host_urpf_hash_info;
    char* key_name = NULL;
    char* result = NULL;

    uint8 lchip = p_info->lchip;
    uint32 key_type = 0;
    uint32 ad_index = 0;
    uint32 vrf_id = 0;
    uint32 index = 0;
    char* ad_name = NULL;

    /*DbgLpmTcamEngineResult0Info*/
    DbgLpmTcamEngineResult0Info_m dbg_lpm_tcam_res0;
    uint32 grp0_valid[6] = {0};
    uint32 grp0_index[6] = {0};

    /*DbgLpmTcamEngineResult1Info*/
    DbgLpmTcamEngineResult1Info_m dbg_lpm_tcam_res1;
    uint32 grp1_valid[6] = {0};
    uint32 grp1_index[6] = {0};

    uint32 pipeline_valid = 0;
    uint32 lpm_type0 = 0;
    uint32 lpm_type1 = 0;
    uint32 entry_type;
    /*DbgLpmPipelineLookupResultInfo*/
    DbgLpmPipelineLookupResultInfo_m pipeline_info;


    /*DbgFibLkpEnginel3DaHashInfo*/
    sal_memset(&dbg_fib_lkp_enginel3_da_hash_info, 0, sizeof(dbg_fib_lkp_enginel3_da_hash_info));
    cmd = DRV_IOR(DbgFibLkpEnginel3DaHashInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_fib_lkp_enginel3_da_hash_info);
    GetDbgFibLkpEnginel3DaHashInfo(A, hostHashFirstL3DaResultValid_f,
        &dbg_fib_lkp_enginel3_da_hash_info, &l3Da_first_hash_ResultValid);
    GetDbgFibLkpEnginel3DaHashInfo(A, hostHashSecondDaResultValid_f,
        &dbg_fib_lkp_enginel3_da_hash_info, &l3Da_second_hash_ResultValid);
    GetDbgFibLkpEnginel3DaHashInfo(A, l3DaLookupEn_f, &dbg_fib_lkp_enginel3_da_hash_info, &route_l3DaLookupEn);
    GetDbgFibLkpEnginel3DaHashInfo(A, hostHashFirstL3DaKeyIndex_f,
        &dbg_fib_lkp_enginel3_da_hash_info, &l3Da_first_hitHashKeyIndexl3Da);
    GetDbgFibLkpEnginel3DaHashInfo(A, hostHashSecondDaKeyIndex_f,
        &dbg_fib_lkp_enginel3_da_hash_info, &l3Da_second_hitHashKeyIndexl3Da);
    GetDbgFibLkpEnginel3DaHashInfo(A, valid_f, &dbg_fib_lkp_enginel3_da_hash_info, &l3Da_hashvalid);
    GetDbgFibLkpEnginel3DaHashInfo(A, l3DaKeyType_f, &dbg_fib_lkp_enginel3_da_hash_info, &key_type);
    GetDbgFibLkpEnginel3DaHashInfo(A, vrfId_f, &dbg_fib_lkp_enginel3_da_hash_info, &vrf_id);
    GetDbgFibLkpEnginel3DaHashInfo(A, l3DaAdindexForDebug_f, &dbg_fib_lkp_enginel3_da_hash_info, &ad_index);

    /*lpm info*/
    GetDbgFibLkpEnginel3DaHashInfo(A, l3DaTcamLookup1PrivateResultValid_f,
            &dbg_fib_lkp_enginel3_da_hash_info, &lpmtcam_lkp1_pri_valid);
    GetDbgFibLkpEnginel3DaHashInfo(A, l3DaTcamLookup1PublicResultValid_f,
            &dbg_fib_lkp_enginel3_da_hash_info, &lpmtcam_lkp1_pub_valid);
    GetDbgFibLkpEnginel3DaHashInfo(A, l3DaTcamLookup2PrivateResultValid_f,
            &dbg_fib_lkp_enginel3_da_hash_info, &lpmtcam_lkp2_pri_valid);
    GetDbgFibLkpEnginel3DaHashInfo(A, l3DaTcamLookup2PublicResultValid_f,
            &dbg_fib_lkp_enginel3_da_hash_info, &lpmtcam_lkp2_pub_valid);

    /*DbgFibLkpEnginel3SaHashInfo*/
    sal_memset(&dbg_fib_lkp_enginel3_sa_hash_info, 0, sizeof(dbg_fib_lkp_enginel3_sa_hash_info));
    cmd = DRV_IOR(DbgFibLkpEnginel3SaHashInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_fib_lkp_enginel3_sa_hash_info);
    GetDbgFibLkpEnginel3SaHashInfo(A, l3SaResultValid_f, &dbg_fib_lkp_enginel3_sa_hash_info, &l3Sa_l3SaHashResultValid);
    GetDbgFibLkpEnginel3SaHashInfo(A, l3SaLookupEn_f, &dbg_fib_lkp_enginel3_sa_hash_info, &route_l3SaLookupEn);
    GetDbgFibLkpEnginel3SaHashInfo(A, valid_f, &dbg_fib_lkp_enginel3_sa_hash_info, &l3Sa_valid);
    GetDbgFibLkpEnginel3SaHashInfo(A, l3SaKeyType_f, &dbg_fib_lkp_enginel3_sa_hash_info, &l3Sa_key_type);
    GetDbgFibLkpEnginel3SaHashInfo(A, hostHashFirstSaNatTcamResultValid_f,
        &dbg_fib_lkp_enginel3_sa_hash_info, &l3Sa_nat_tcam);
    GetDbgFibLkpEnginel3SaHashInfo(A, hostHashFirstSaRpfTcamResultValid_f,
        &dbg_fib_lkp_enginel3_sa_hash_info, &l3Sa_rpf_tcam);
    GetDbgFibLkpEnginel3SaHashInfo(A, hostHashSecondSaKeyIndex_f,
        &dbg_fib_lkp_enginel3_sa_hash_info, &l3Sa_hitHashKeyIndexl3Sa);
    GetDbgFibLkpEnginel3SaHashInfo(A, hostHashFirstSaTcamHitIndex_f,
        &dbg_fib_lkp_enginel3_sa_hash_info, &l3Sa_hitTcamKeyIndexl3Sa);

    /*DbgFibLkpEngineHostUrpfHashInfo*/
    sal_memset(&dbg_fib_lkp_enginel3_host_urpf_hash_info, 0, sizeof(dbg_fib_lkp_enginel3_host_urpf_hash_info));
    cmd = DRV_IOR(DbgFibLkpEngineHostUrpfHashInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_fib_lkp_enginel3_host_urpf_hash_info);
    GetDbgFibLkpEngineHostUrpfHashInfo(A, hostUrpfHashHitIndex_f,
        &dbg_fib_lkp_enginel3_host_urpf_hash_info, &l3Sa_hitUrpfHashKeyIndexl3Sa);

    /*DbgIpeIpRoutingInfo*/
    sal_memset(&dbg_ipe_ip_routing_info, 0, sizeof(dbg_ipe_ip_routing_info));
    cmd = DRV_IOR(DbgIpeIpRoutingInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_ipe_ip_routing_info);
    GetDbgIpeIpRoutingInfo(A, ipDaResultValid_f, &dbg_ipe_ip_routing_info, &route_l3DaResultValid);
    GetDbgIpeIpRoutingInfo(A, ipSaResultValid_f, &dbg_ipe_ip_routing_info, &route_l3SaResultValid);

    GetDbgIpeIpRoutingInfo(A, flowForceRouteUseIpSa_f, &dbg_ipe_ip_routing_info, &route_flowForceRouteUseIpSa);
    GetDbgIpeIpRoutingInfo(A, ipMartianAddress_f, &dbg_ipe_ip_routing_info, &route_ipMartianAddress);
    GetDbgIpeIpRoutingInfo(A, ipLinkScopeAddress_f, &dbg_ipe_ip_routing_info, &route_ipLinkScopeAddress);
    GetDbgIpeIpRoutingInfo(A, packetTtl_f, &dbg_ipe_ip_routing_info, &route_packetTtl);
    GetDbgIpeIpRoutingInfo(A, shareType_f, &dbg_ipe_ip_routing_info, &route_shareType);
    GetDbgIpeIpRoutingInfo(A, layer3Exception_f, &dbg_ipe_ip_routing_info, &route_layer3Exception);
    GetDbgIpeIpRoutingInfo(A, discard_f, &dbg_ipe_ip_routing_info, &route_discard);
    GetDbgIpeIpRoutingInfo(A, discardType_f, &dbg_ipe_ip_routing_info, &route_discardType);
    GetDbgIpeIpRoutingInfo(A, exceptionEn_f, &dbg_ipe_ip_routing_info, &route_exceptionEn);
    GetDbgIpeIpRoutingInfo(A, exceptionIndex_f, &dbg_ipe_ip_routing_info, &route_exceptionIndex);
    GetDbgIpeIpRoutingInfo(A, exceptionSubIndex_f, &dbg_ipe_ip_routing_info, &route_exceptionSubIndex);
    GetDbgIpeIpRoutingInfo(A, fatalExceptionValid_f, &dbg_ipe_ip_routing_info, &route_fatalExceptionValid);
    GetDbgIpeIpRoutingInfo(A, fatalException_f, &dbg_ipe_ip_routing_info, &route_fatalException);
    GetDbgIpeIpRoutingInfo(A, payloadPacketType_f, &dbg_ipe_ip_routing_info, &route_payloadPacketType);
    GetDbgIpeIpRoutingInfo(A, ipHeaderError_f, &dbg_ipe_ip_routing_info, &ip_headerError);
    GetDbgIpeIpRoutingInfo(A, routingEscape_f, &dbg_ipe_ip_routing_info, &route_escape);
    GetDbgIpeIpRoutingInfo(A, valid_f, &dbg_ipe_ip_routing_info, &route_valid);

    /*DbgLpmTcamEngineResult0Info*/
    sal_memset(&dbg_lpm_tcam_res0, 0, sizeof(dbg_lpm_tcam_res0));
    cmd = DRV_IOR(DbgLpmTcamEngineResult0Info_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_lpm_tcam_res0);
    for (index = 0; index < 6; index++)
    {
        DRV_IOR_FIELD(lchip, DbgLpmTcamEngineResult0Info_t, DbgLpmTcamEngineResult0Info_gTcamGroup_0_lookupResultValid_f+index*2,
            &grp0_valid[index], &dbg_lpm_tcam_res0);

        DRV_IOR_FIELD(lchip, DbgLpmTcamEngineResult0Info_t, DbgLpmTcamEngineResult0Info_gTcamGroup_0_tcamHitIndex_f+index*2,
            &grp0_index[index], &dbg_lpm_tcam_res0);
    }

    /*DbgLpmTcamEngineResult1Info*/
    sal_memset(&dbg_lpm_tcam_res1, 0, sizeof(dbg_lpm_tcam_res1));
    cmd = DRV_IOR(DbgLpmTcamEngineResult1Info_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_lpm_tcam_res1);
    for (index = 0; index < 6; index++)
    {
        DRV_IOR_FIELD(lchip, DbgLpmTcamEngineResult1Info_t, DbgLpmTcamEngineResult1Info_gTcamGroup_1_lookupResultValid_f+index*2,
            &grp1_valid[index], &dbg_lpm_tcam_res1);

        DRV_IOR_FIELD(lchip, DbgLpmTcamEngineResult0Info_t, DbgLpmTcamEngineResult1Info_gTcamGroup_1_tcamHitIndex_f+index*2,
            &grp1_index[index], &dbg_lpm_tcam_res1);
    }

    cmd = DRV_IOR(DbgLpmPipelineLookupResultInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &pipeline_info);

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    if (route_valid)
    {
        if (route_l3DaLookupEn)
        {
            CTC_DKIT_PRINT("\n");
            CTC_DKIT_PRINT("Route Process:\n");
            CTC_DKIT_PATH_PRINT("%-15s\n", "IP DA lookup--->");

            if (ip_headerError)
            {
                CTC_DKIT_PATH_PRINT("%-15s\n", "IP Header Error!!");
            }
            else if (route_l3DaResultValid)
            {
                if (l3Da_first_hash_ResultValid || l3Da_second_hash_ResultValid)/*hash result*/
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
                    if (l3Da_first_hash_ResultValid)
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", l3Da_first_hitHashKeyIndexl3Da);
                    }
                    else
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", l3Da_second_hitHashKeyIndexl3Da);
                    }
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "Vrf id", vrf_id);

                    if ((CTC_DKITS_L3DA_TYPE_IPV4UCAST == key_type) ||
                        (CTC_DKITS_L3DA_TYPE_IPV4MCAST == key_type))/* FIBDAKEYTYPE_IPV4UCAST or FIBDAKEYTYPE_IPV4MCAST */
                    {
                        key_name = "DsFibHost0Ipv4HashKey";
                    }
                    else if (CTC_DKITS_L3DA_TYPE_IPV6UCAST == key_type)/* FIBDAKEYTYPE_IPV6UCAST */
                    {
                        key_name = "DsFibHost0Ipv6UcastHashKey";
                    }
                    else if (CTC_DKITS_L3DA_TYPE_IPV6MCAST == key_type)/* FIBDAKEYTYPE_IPV6MCAST */
                    {
                        key_name = "DsFibHost0MacIpv6McastHashKey";
                    }
                    else
                    {
                        /* TODO, PBR... */
                        key_name = "Unkown";
                    }
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name);

                }
                else if (lpmtcam_lkp1_pri_valid || lpmtcam_lkp1_pub_valid || lpmtcam_lkp2_pri_valid || lpmtcam_lkp2_pub_valid)/*lpm result*/
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "LPM tcam result", "LPM tcam lookup hit");

                    if (lpmtcam_lkp1_pri_valid)
                    {
                        for (index = 0; index < 6; index++)
                        {
                            if (grp0_valid[index])
                            {
                                CTC_DKIT_PATH_PRINT("%-15s:%s%d\n", "tcam key:", "Lookup1 private block ",index);
                                key_index = grp0_index[index];
                                break;
                            }
                        }
                    }

                    if (lpmtcam_lkp1_pub_valid)
                    {
                        for (index = 0; index < 6; index++)
                        {
                            if (grp0_valid[index])
                            {
                                CTC_DKIT_PATH_PRINT("%-15s:%s%d\n", "tcam key:", "Lookup1 public block ", index);
                                key_index = grp0_index[index];
                                break;
                            }
                        }
                    }
#if 0
                    if (lpmtcam_lkp2_pri_valid)
                    {
                        for (index = 0; index < 6; index++)
                        {
                            if (grp1_valid[index])
                            {
                                CTC_DKIT_PATH_PRINT("%-15s:%s%d\n", "tcam key:", "Lookup2 private block ", index);
                                key_index = grp1_index[index];
                                break;
                            }
                        }
                    }

                    if (lpmtcam_lkp2_pub_valid)
                    {
                        for (index = 0; index < 6; index++)
                        {
                            if (grp1_valid[index])
                            {
                                CTC_DKIT_PATH_PRINT("%-15s:%s%d\n", "tcam key:", "Lookup2 public block ", index);
                                key_index = grp1_index[index];
                                break;
                            }
                        }
                    }
#endif

                    _ctc_usw_dkit_ipuc_get_table_id(lchip, lpmtcam_lkp1_pri_valid, index, key_type, &key_table_id, &ad_table_id, &cmd,&cmd1);

                    _ctc_usw_dkit_ipuc_map_key_index(lchip, key_table_id, key_index, &mapped_key_index, &mapped_ad_index);
                    key_name = TABLE_NAME(lchip, key_table_id);
                    ad_name = TABLE_NAME(lchip, ad_table_id);


                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "Spec tcam index", key_index);
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", mapped_key_index);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name);
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "tcam AD index", mapped_ad_index);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "tcam AD name", ad_name);

                    DRV_IOCTL(lchip, mapped_ad_index, cmd, &pipeline_valid);
                    DRV_IOCTL(lchip, mapped_ad_index, cmd, &entry_type);
                    if (pipeline_valid)
                    {
                        if(DRV_IS_DUET2(lchip))
                        {
                            CTC_DKIT_PATH_PRINT("\n");
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Pipeline0", "LPM pipeline0 lookup process");
                            key_index = GetDbgLpmPipelineLookupResultInfo(V, pipeline1Index0_f, &pipeline_info);

                            cmd = DRV_IOR(DsLpmLookupKey_t, DsLpmLookupKey_type0_f);
                            DRV_IOCTL(lchip, key_index, cmd, &lpm_type0);
                            cmd = DRV_IOR(DsLpmLookupKey_t, DsLpmLookupKey_type1_f);
                            DRV_IOCTL(lchip, key_index, cmd, &lpm_type1);

                            if ((lpm_type0 == 2) || (lpm_type1 == 2))
                            {
                                pipeline_valid = 1;
                            }
                            else
                            {
                                pipeline_valid = 0;
                            }

                            key_name = "DsLpmLookupKey";
                            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", key_index);
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name);

                            if(pipeline_valid)
                            {
                                CTC_DKIT_PATH_PRINT("\n");
                                key_index = GetDbgLpmPipelineLookupResultInfo(V, pipeline2Index0_f, &pipeline_info);
                                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Pipeline1", "LPM pipeline1 lookup process");

                                key_name = "DsLpmLookupKey";
                                CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", key_index);
                                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name);
                            }
                        }
                        else
                        {
                            uint8 offset = GetDbgLpmPipelineLookupResultInfo(V, pipeline2Index0_f, &pipeline_info);
                            uint8 is_snake1 = ((offset >> 4) > 1) ? 1:0;
                            if(entry_type)
                            {
                                if(entry_type == 1)
                                {
                                    key_name = is_snake1 ? "DsNeoLpmIpv4Bit32Snake1" : "DsNeoLpmIpv4Bit32Snake";
                                }
                                else if(entry_type == 2)
                                {
                                    key_name = is_snake1 ? "DsNeoLpmIpv6Bit64Snake1" : "DsNeoLpmIpv6Bit64Snake";
                                }
                                else
                                {
                                    key_name = is_snake1 ? "DsNeoLpmIpv6Bit128Snake1" : "DsNeoLpmIpv6Bit128Snake";
                                }
                            }
                            key_index = GetDbgLpmPipelineLookupResultInfo(V, pipeline1Index0_f, &pipeline_info);
                            CTC_DKIT_PATH_PRINT("\n");
                            CTC_DKIT_PATH_PRINT("%-15s\n", "LPM pipeline lookup process");
                            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "Snake table index", key_index);
                            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "Offset", offset&0xF);
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Table name", key_name);
                        }
                    }
                }

                CTC_DKIT_PATH_PRINT("\n");
                CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "AD index", ad_index);
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", "DsIpDa\n");
            }
            else
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
            }
        }

        if(route_l3SaLookupEn)
        {
            CTC_DKIT_PATH_PRINT("%-15s\n", "IP SA lookup--->");
            if (route_l3SaResultValid)
            {
                if (l3Sa_key_type == CTC_DKITS_L3SA_TYPE_IPV4NATSA)
                {
                    if (l3Sa_nat_tcam)
                    {
                        result = "IPv4 Nat LPM Tcam lookup hit";
                        key_name = "DsLpmTcamIpv4NatDoubleKey";
                        key_index = (l3Sa_hitTcamKeyIndexl3Sa >= 0x2000) ? ((l3Sa_hitTcamKeyIndexl3Sa-0x2000)/4) : l3Sa_hitTcamKeyIndexl3Sa;
                    }
                    else if (l3Sa_l3SaHashResultValid)
                    {
                        result = "IPv4 Nat HASH lookup hit";
                        key_name = "DsFibHost1Ipv4NatSaPortHashKey";
                        key_index = l3Sa_hitHashKeyIndexl3Sa;
                    }
                }
                else if (l3Sa_key_type == CTC_DKITS_L3SA_TYPE_IPV6NATSA)
                {
                    if (l3Sa_nat_tcam)
                    {
                        result = "IPv6 Nat LPM Tcam lookup hit";
                        key_name = "DsLpmTcamIpv6DoubleKey0";
                        key_index = l3Sa_hitTcamKeyIndexl3Sa;
                    }
                    else if (l3Sa_l3SaHashResultValid)
                    {
                        result = "IPv6 Nat HASH lookup hit";
                        key_name = "DsFibHost1Ipv6NatSaPortHashKey";
                        key_index = l3Sa_hitHashKeyIndexl3Sa;
                    }
                }
                else if (l3Sa_key_type == CTC_DKITS_L3SA_TYPE_IPV4RPF)
                {
                    if (l3Sa_rpf_tcam)
                    {
                        result = "IPv4 RPF LPM Tcam lookup hit";
                        key_name = "DsLpmTcamIpv4HalfKey";
                        key_index = l3Sa_hitTcamKeyIndexl3Sa;
                    }
                    else if (l3Sa_l3SaHashResultValid)
                    {
                        result = "IPv4 RPF HASH lookup hit";
                        key_name = "DsFibHost0Ipv4HashKey";
                        key_index = l3Sa_hitUrpfHashKeyIndexl3Sa;
                    }
                }
                else if (l3Sa_key_type == CTC_DKITS_L3SA_TYPE_IPV6RPF)
                {
                    if (l3Sa_rpf_tcam)
                    {
                        result = "IPv6 RPF LPM Tcam lookup hit";
                        key_name = "DsLpmTcamIpv6DoubleKey0";
                        key_index = l3Sa_hitTcamKeyIndexl3Sa;
                    }
                    else if (l3Sa_l3SaHashResultValid)
                    {
                        result = "IPv6 RPF HASH lookup hit";
                        key_name = "DsFibHost0Ipv6UcastHashKey";
                        key_index = l3Sa_hitHashKeyIndexl3Sa;
                    }
                }

                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", result);
                CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", key_index);
                CTC_DKIT_PATH_PRINT("%-15s:%s\n\n", "key name", key_name);

                /*pbr TBD*/
            }
            else
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
            }

        }
        if (route_escape)
        {
            CTC_DKIT_PATH_PRINT("%-15s\n", "Routing Escape!!");
        }

    }

    if (route_discard)
    {
        discard = 1;
        discard_type = route_discardType;
    }

    if (route_exceptionEn)
    {
        exceptionEn = 1;
        exceptionIndex = route_exceptionIndex;
        exceptionSubIndex = route_exceptionSubIndex;
    }
Discard:
    _ctc_usw_dkit_captured_path_discard_process(p_info, discard, discard_type, CTC_DKIT_IPE);
    _ctc_usw_dkit_captured_path_exception_process(p_info, exceptionEn, exceptionIndex, exceptionSubIndex);

    return 0;
}

STATIC int32
_ctc_usw_dkit_capture_path_ipe_linkagg(ctc_dkit_captured_path_info_t* p_info)
{
    uint8  lchip = p_info->lchip;
    uint32 cmd = 0;
    LagEngineInput_m   lag_engine_input;
    IpeToLagEngineInput_m ipe_lag_engine_input;
    BsrToLagEngineInput_m bsr_lag_engine_input;
    LagEngineOutput_m  lag_engine_ouput;
    uint32 valid =0;
    uint32 bsr_valid =0;
    DbgLagEngineInfoFromIpeFwd_m dbg_lag_engine_info;
    DbgLagEngineInfoFromBsrEnqueue_m dbg_bsr_lag_engine_info;
    uint32 cFlexFwdSgGroupSel = 0;
    uint32 cFlexLagHash = 0;
    uint32 destMap = 0;
    uint32 destSgmacGroupId_in = 0;
    uint32 headerHash = 0;
    uint32 headerHashValid = 0;
    uint32 hashValue[4] = {0};
    uint32 hashValueValid = 0;
    uint32 u1_gFromEnq_destSelect = 0;
    uint32 u1_gFromEnq_fromDmaOrOamChannel = 0;
    uint32 u1_gFromEnq_iLoopToAggUpMep = 0;
    uint32 u1_gFromEnq_mcastIdValid = 0;
    uint32 u1_gFromEnq_replicatedMet = 0;

    uint32 destChannelIdValid = 0;
    uint32 destChannelId = 0;
    uint32 destChipIdMatch = 0;
    uint32 destSgmacGroupId_out = 0;
    uint32 iloopUseLocalPhyPort = 0;
    uint32 peerChipId = 0;
    uint32 portLagEn = 0;
    uint32 sgmacLagEnable = 0;
    uint32 stackingDiscard = 0;
    uint32 updateDestMap = 0;

    sal_memset(&dbg_lag_engine_info, 0, sizeof(DbgLagEngineInfoFromIpeFwd_m));
    cmd = DRV_IOR(DbgLagEngineInfoFromIpeFwd_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_lag_engine_info);
    sal_memset(&dbg_bsr_lag_engine_info, 0, sizeof(DbgLagEngineInfoFromBsrEnqueue_m));
    cmd = DRV_IOR(DbgLagEngineInfoFromBsrEnqueue_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_bsr_lag_engine_info);
    if (DRV_IS_DUET2(lchip))
    {
        GetDbgLagEngineInfoFromIpeFwd(A, lagEngineInput_f, &dbg_lag_engine_info, &lag_engine_input);
    }
    else
    {
        GetDbgLagEngineInfoFromIpeFwd(A, lagEngineInput_f, &dbg_lag_engine_info, &ipe_lag_engine_input);
        GetDbgLagEngineInfoFromBsrEnqueue(A, lagEngineInput_f, &dbg_bsr_lag_engine_info, &bsr_lag_engine_input);
        GetDbgLagEngineInfoFromBsrEnqueue(A, valid_f, &dbg_bsr_lag_engine_info, &bsr_valid);
    }
    GetDbgLagEngineInfoFromIpeFwd(A, lagEngineOutput_f, &dbg_lag_engine_info, &lag_engine_ouput);
    GetDbgLagEngineInfoFromIpeFwd(A, valid_f, &dbg_lag_engine_info, &valid);

    if (valid)
    {
        if (DRV_IS_DUET2(lchip))
        {
            GetLagEngineInput(A, cFlexFwdSgGroupSel_f, &lag_engine_input, &cFlexFwdSgGroupSel);
            GetLagEngineInput(A, cFlexLagHash_f, &lag_engine_input, &cFlexLagHash);
            GetLagEngineInput(A, destMap_f, &lag_engine_input, &destMap);
            GetLagEngineInput(A, destSgmacGroupId_f, &lag_engine_input, &destSgmacGroupId_in);
            GetLagEngineInput(A, headerHash_f, &lag_engine_input, &headerHash);
            GetLagEngineInput(A, u1_gFromEnq_destSelect_f, &lag_engine_input, &u1_gFromEnq_destSelect);
            GetLagEngineInput(A, u1_gFromEnq_fromDmaOrOamChannel_f, &lag_engine_input, &u1_gFromEnq_fromDmaOrOamChannel);
            GetLagEngineInput(A, u1_gFromEnq_iLoopToAggUpMep_f, &lag_engine_input, &u1_gFromEnq_iLoopToAggUpMep);
            GetLagEngineInput(A, u1_gFromEnq_mcastIdValid_f, &lag_engine_input, &u1_gFromEnq_mcastIdValid);
            GetLagEngineInput(A, u1_gFromEnq_replicatedMet_f, &lag_engine_input, &u1_gFromEnq_replicatedMet);
        }
        else
        {
            GetIpeToLagEngineInput(A, cFlexFwdSgGroupSel_f, &ipe_lag_engine_input, &cFlexFwdSgGroupSel);
            GetIpeToLagEngineInput(A, cFlexLagHash_f, &ipe_lag_engine_input, &cFlexLagHash);
            GetIpeToLagEngineInput(A, destMap_f, &ipe_lag_engine_input, &destMap);
            GetIpeToLagEngineInput(A, headerHash_f, &ipe_lag_engine_input, &headerHash);
            GetIpeToLagEngineInput(A, headerHashValid_f, &ipe_lag_engine_input, &headerHashValid);
            GetIpeToLagEngineInput(A, hashValue_f, &ipe_lag_engine_input, hashValue);
            GetIpeToLagEngineInput(A, hashValueValid_f, &ipe_lag_engine_input, &hashValueValid);
            if (bsr_valid)
            {
                GetBsrToLagEngineInput(A, destSgmacGroupId_f, &bsr_lag_engine_input, &destSgmacGroupId_in);
                GetBsrToLagEngineInput(A, destSelect_f, &bsr_lag_engine_input, &u1_gFromEnq_destSelect);
                GetBsrToLagEngineInput(A, fromDmaOrOamChannel_f, &bsr_lag_engine_input, &u1_gFromEnq_fromDmaOrOamChannel);
                GetBsrToLagEngineInput(A, iLoopToAggUpMep_f, &bsr_lag_engine_input, &u1_gFromEnq_iLoopToAggUpMep);
                GetBsrToLagEngineInput(A, mcastIdValid_f, &bsr_lag_engine_input, &u1_gFromEnq_mcastIdValid);
                GetBsrToLagEngineInput(A, replicatedMet_f, &bsr_lag_engine_input, &u1_gFromEnq_replicatedMet);
            }
        }

        GetLagEngineOutput(A, destChannelIdValid_f, &lag_engine_ouput, &destChannelIdValid);
        GetLagEngineOutput(A, destChannelId_f, &lag_engine_ouput, &destChannelId);
        GetLagEngineOutput(A, destChipIdMatch_f, &lag_engine_ouput, &destChipIdMatch);
        GetLagEngineOutput(A, destSgmacGroupId_f, &lag_engine_ouput, &destSgmacGroupId_out);
        GetLagEngineOutput(A, iloopUseLocalPhyPort_f, &lag_engine_ouput, &iloopUseLocalPhyPort);
        GetLagEngineOutput(A, peerChipId_f, &lag_engine_ouput, &peerChipId);
        GetLagEngineOutput(A, portLagEn_f, &lag_engine_ouput, &portLagEn);
        GetLagEngineOutput(A, sgmacLagEnable_f, &lag_engine_ouput, &sgmacLagEnable);
        GetLagEngineOutput(A, stackingDiscard_f, &lag_engine_ouput, &stackingDiscard);
        GetLagEngineOutput(A, updateDestMap_f, &lag_engine_ouput, &updateDestMap);

    }

    if (sgmacLagEnable || portLagEn)
    {
        if (sgmacLagEnable)
        {
            CTC_DKIT_PRINT("SgmacLinkagg Process:\n");
            CTC_DKIT_PATH_PRINT("%-15s:%u\n", "cFlexFwdSgGroupSel", cFlexFwdSgGroupSel);
            CTC_DKIT_PATH_PRINT("%-15s:%u\n", "cFlexLagHash", cFlexLagHash);
            CTC_DKIT_PATH_PRINT("%-15s:%u\n", "stackingDiscard", stackingDiscard);
            CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Trunk ID", destSgmacGroupId_out);
        }
        if (portLagEn)
        {
            CTC_DKIT_PRINT("PortLinkagg Process:\n");
            CTC_DKIT_PATH_PRINT("%-15s:%u\n", "LinkAgg ID", (destMap & 0xff));
            CTC_DKIT_PATH_PRINT("%-15s:%u\n", "headerHash", headerHash);
            if (!DRV_IS_DUET2(lchip))
            {
                CTC_DKIT_PATH_PRINT("%-15s:%u\n", "headerHashValid", headerHashValid);
                CTC_DKIT_PATH_PRINT("%-15s:0x%.8x%.8x%.8x%.8x\n", "hashValue", hashValue[3],hashValue[2],hashValue[1],hashValue[0]);
                CTC_DKIT_PATH_PRINT("%-15s:%u\n", "hashValueValid", hashValueValid);
            }
        }
        if (destChannelIdValid)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%u\n", "destChannelId", destChannelId);
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_ipe_parser(ctc_dkit_captured_path_info_t* p_info)
{
    _ctc_usw_dkit_captured_path_parser(p_info, 0);
    _ctc_usw_dkit_captured_path_parser(p_info, 1);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_ipe_scl(ctc_dkit_captured_path_info_t* p_info)
{
    uint8 i = 0;
    uint8 slice = p_info->slice_ipe;
    uint8 lchip = p_info->lchip;
    uint8 lkp_level = 0;
    uint32 ad_ds_type[4] = {0};
    uint32 ad_proc_valid[4] = {0};
    uint32 buf_ad_ds_type[4] = {0};
    uint32 buf_ad_proc_valid[4] = {0};
    uint8  buf_cnt[4] = {0};
    uint32 cmd = 0;
    uint32 key_index = 0;
    uint32 is_half = 0;
    char* str_scl_key_type[] = {"MACKEY160", "L3KEY160", "IPV6KEY320", "MACL3KEY320", "MACIPV6KEY640", "USERIDKEY", "UDFKEY160", "UDFKEY320"};
    char* str_ad_ds_type[] = {"NONE","USERID","SCL","TUNNELID"};
    /*DbgIpeUserIdInfo*/
    uint32 scl_sclFlowHashEn = 0;
    uint32 scl_exceptionEn = 0;
    uint32 scl_exceptionIndex = 0;
    uint32 scl_exceptionSubIndex = 0;
    uint32 scl_localPhyPort = 0;
    uint32 scl_discard = 0;
    uint32 scl_discardType = 0;
    uint32 scl_bypassAll = 0;
    uint32 scl_dsFwdPtrValid = 0;
    uint32 scl_dsFwdPtr = 0;
    uint32 scl_userIdTcamEn[4] = {0};
    uint32 scl_userIdTcamType[4] = {0};
    uint32 scl_valid = 0;
    DbgIpeUserIdInfo_m dbg_ipe_user_id_info;
    /*DbgUserIdHashEngineForUserId0Info*/
    uint32 hash0_lookupResultValid = 0;
    uint32 hash0_defaultEntryValid = 0;
    uint32 hash0_keyIndex = 0;
    uint32 hash0_adIndex = 0;
    uint32 hash0_hashConflict = 0;
    uint32 hash0_keyType = 0;
    uint32 hash0_valid = 0;
    DbgUserIdHashEngineForUserId0Info_m dbg_user_id_hash_engine_for_user_id0_info;
    /*DbgUserIdHashEngineForUserId1Info*/
    uint32 hash1_lookupResultValid = 0;
    uint32 hash1_defaultEntryValid = 0;
    uint32 hash1_keyIndex = 0;
    uint32 hash1_adIndex = 0;
    uint32 hash1_hashConflict = 0;
    uint32 hash1_keyType = 0;
    uint32 hash1_valid = 0;
    DbgUserIdHashEngineForUserId1Info_m dbg_user_id_hash_engine_for_user_id1_info;
    /*DbgFlowTcamEngineUserIdInfo*/
    uint32 tcam_userIdTcamResultValid[4] = {0};
    uint32 tcam_userIdTcamIndex[4] = {0};
    uint32 tcam_userIdValid[4] = {0};
    DbgFlowTcamEngineUserIdInfo0_m dbg_flow_tcam_engine_user_id_info;
    uint32 scl_key_size = 0;
    DbgFlowTcamEngineUserIdKeyInfo0_m dbg_scl_key_info;
    uint8  userIdResultPriorityMode = 0;
    uint32 scl_key_info_key_id[4] = {DbgFlowTcamEngineUserIdKeyInfo0_t, DbgFlowTcamEngineUserIdKeyInfo1_t,
                             DbgFlowTcamEngineUserIdKeyInfo2_t,DbgFlowTcamEngineUserIdKeyInfo3_t};
    uint32 scl_key_info_keysizefld[4] = {DbgFlowTcamEngineUserIdKeyInfo0_keySize_f, DbgFlowTcamEngineUserIdKeyInfo1_keySize_f,
                             DbgFlowTcamEngineUserIdKeyInfo2_keySize_f,DbgFlowTcamEngineUserIdKeyInfo3_keySize_f};
    sal_memset(&dbg_scl_key_info, 0, sizeof(dbg_scl_key_info));

    /*DbgIpeUserIdInfo*/
    if (!DRV_IS_DUET2(lchip))
    {
        str_scl_key_type[4] = "UDFKEY320";
        str_scl_key_type[7] = "MACIPV6KEY640";
    }

    sal_memset(&dbg_ipe_user_id_info, 0, sizeof(dbg_ipe_user_id_info));
    cmd = DRV_IOR(DbgIpeUserIdInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_ipe_user_id_info);
    GetDbgIpeUserIdInfo(A, localPhyPort_f, &dbg_ipe_user_id_info, &scl_localPhyPort);
    GetDbgIpeUserIdInfo(A, exceptionEn_f, &dbg_ipe_user_id_info, &scl_exceptionEn);
    GetDbgIpeUserIdInfo(A, exceptionIndex_f, &dbg_ipe_user_id_info, &scl_exceptionIndex);
    GetDbgIpeUserIdInfo(A, exceptionSubIndex_f, &dbg_ipe_user_id_info, &scl_exceptionSubIndex);
    GetDbgIpeUserIdInfo(A, discard_f, &dbg_ipe_user_id_info, &scl_discard);
    GetDbgIpeUserIdInfo(A, discardType_f, &dbg_ipe_user_id_info, &scl_discardType);
    GetDbgIpeUserIdInfo(A, bypassAll_f, &dbg_ipe_user_id_info, &scl_bypassAll);
    GetDbgIpeUserIdInfo(A, dsFwdPtrValid_f, &dbg_ipe_user_id_info, &scl_dsFwdPtrValid);
    GetDbgIpeUserIdInfo(A, dsFwdPtr_f, &dbg_ipe_user_id_info, &scl_dsFwdPtr);
    GetDbgIpeUserIdInfo(A, userIdTcamLookupEn0_f, &dbg_ipe_user_id_info, &scl_userIdTcamEn[0]);
    GetDbgIpeUserIdInfo(A, userIdTcamLookupEn1_f, &dbg_ipe_user_id_info, &scl_userIdTcamEn[1]);
    GetDbgIpeUserIdInfo(A, userIdTcamLookupEn2_f, &dbg_ipe_user_id_info, &scl_userIdTcamEn[2]);
    GetDbgIpeUserIdInfo(A, userIdTcamLookupEn3_f, &dbg_ipe_user_id_info, &scl_userIdTcamEn[3]);
    GetDbgIpeUserIdInfo(A, userIdTcamKeyType0_f, &dbg_ipe_user_id_info, &scl_userIdTcamType[0]);
    GetDbgIpeUserIdInfo(A, userIdTcamKeyType1_f, &dbg_ipe_user_id_info, &scl_userIdTcamType[1]);
    GetDbgIpeUserIdInfo(A, userIdTcamKeyType2_f, &dbg_ipe_user_id_info, &scl_userIdTcamType[2]);
    GetDbgIpeUserIdInfo(A, userIdTcamKeyType3_f, &dbg_ipe_user_id_info, &scl_userIdTcamType[3]);
    GetDbgIpeUserIdInfo(A, valid_f, &dbg_ipe_user_id_info, &scl_valid);
    GetDbgIpeUserIdInfo(A, process0AdDsType_f, &dbg_ipe_user_id_info, &buf_ad_ds_type[0]);
    GetDbgIpeUserIdInfo(A, process1AdDsType_f, &dbg_ipe_user_id_info, &buf_ad_ds_type[1]);
    GetDbgIpeUserIdInfo(A, process2AdDsType_f, &dbg_ipe_user_id_info, &buf_ad_ds_type[2]);
    GetDbgIpeUserIdInfo(A, process3AdDsType_f, &dbg_ipe_user_id_info, &buf_ad_ds_type[3]);
    GetDbgIpeUserIdInfo(A, adProcessValid0_f, &dbg_ipe_user_id_info, &buf_ad_proc_valid[0]);
    GetDbgIpeUserIdInfo(A, adProcessValid1_f, &dbg_ipe_user_id_info, &buf_ad_proc_valid[1]);
    GetDbgIpeUserIdInfo(A, adProcessValid2_f, &dbg_ipe_user_id_info, &buf_ad_proc_valid[2]);
    GetDbgIpeUserIdInfo(A, adProcessValid3_f, &dbg_ipe_user_id_info, &buf_ad_proc_valid[3]);

    cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_userIdResultPriorityMode_f);
    DRV_IOCTL(lchip, slice, cmd, &userIdResultPriorityMode);
    switch (userIdResultPriorityMode)
    {
        case 0:
            buf_cnt[0] = 0;
            buf_cnt[1] = 1;
            buf_cnt[2] = 2;
            buf_cnt[3] = 3;
            break;
        case 1:
            buf_cnt[2] = 0;
            buf_cnt[0] = 1;
            buf_cnt[1] = 2;
            buf_cnt[3] = 3;
            break;
        case 2:
            buf_cnt[2] = 0;
            buf_cnt[3] = 1;
            buf_cnt[0] = 2;
            buf_cnt[1] = 3;
            break;
        case 3:
            buf_cnt[0] = 0;
            buf_cnt[2] = 1;
            buf_cnt[1] = 2;
            buf_cnt[3] = 3;
            break;
        case 4:
            buf_cnt[2] = 0;
            buf_cnt[0] = 1;
            buf_cnt[3] = 2;
            buf_cnt[1] = 3;
            break;
        default:
            break;
    }
    for (i = 0 ;i <4 ;i++)
    {
       ad_ds_type[i] = buf_ad_ds_type[buf_cnt[i]];
       ad_proc_valid[i] = buf_ad_proc_valid[buf_cnt[i]];
    }

    cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_sclFlowHashEn_f);
    DRV_IOCTL(lchip, scl_localPhyPort, cmd, &scl_sclFlowHashEn);

    /*DbgUserIdHashEngineForUserId0Info*/
    sal_memset(&dbg_user_id_hash_engine_for_user_id0_info, 0, sizeof(dbg_user_id_hash_engine_for_user_id0_info));
    cmd = DRV_IOR(DbgUserIdHashEngineForUserId0Info_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_user_id_hash_engine_for_user_id0_info);
    GetDbgUserIdHashEngineForUserId0Info(A, lookupResultValid_f, &dbg_user_id_hash_engine_for_user_id0_info, &hash0_lookupResultValid);
    GetDbgUserIdHashEngineForUserId0Info(A, defaultEntryValid_f, &dbg_user_id_hash_engine_for_user_id0_info, &hash0_defaultEntryValid);
    GetDbgUserIdHashEngineForUserId0Info(A, keyIndex_f, &dbg_user_id_hash_engine_for_user_id0_info, &hash0_keyIndex);
    GetDbgUserIdHashEngineForUserId0Info(A, adIndex_f, &dbg_user_id_hash_engine_for_user_id0_info, &hash0_adIndex);
    GetDbgUserIdHashEngineForUserId0Info(A, hashConflict_f, &dbg_user_id_hash_engine_for_user_id0_info, &hash0_hashConflict);
    GetDbgUserIdHashEngineForUserId0Info(A, userIdKeyType_f, &dbg_user_id_hash_engine_for_user_id0_info, &hash0_keyType);
    GetDbgUserIdHashEngineForUserId0Info(A, valid_f, &dbg_user_id_hash_engine_for_user_id0_info, &hash0_valid);

    /*DbgUserIdHashEngineForUserId1Info*/
    sal_memset(&dbg_user_id_hash_engine_for_user_id1_info, 0, sizeof(dbg_user_id_hash_engine_for_user_id1_info));
    cmd = DRV_IOR(DbgUserIdHashEngineForUserId1Info_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_user_id_hash_engine_for_user_id1_info);
    GetDbgUserIdHashEngineForUserId1Info(A, lookupResultValid_f, &dbg_user_id_hash_engine_for_user_id1_info, &hash1_lookupResultValid);
    GetDbgUserIdHashEngineForUserId1Info(A, defaultEntryValid_f, &dbg_user_id_hash_engine_for_user_id1_info, &hash1_defaultEntryValid);
    GetDbgUserIdHashEngineForUserId1Info(A, keyIndex_f, &dbg_user_id_hash_engine_for_user_id1_info, &hash1_keyIndex);
    GetDbgUserIdHashEngineForUserId1Info(A, adIndex_f, &dbg_user_id_hash_engine_for_user_id1_info, &hash1_adIndex);
    GetDbgUserIdHashEngineForUserId1Info(A, hashConflict_f, &dbg_user_id_hash_engine_for_user_id1_info, &hash1_hashConflict);
    GetDbgUserIdHashEngineForUserId1Info(A, userIdKeyType_f, &dbg_user_id_hash_engine_for_user_id1_info, &hash1_keyType);
    GetDbgUserIdHashEngineForUserId1Info(A, valid_f, &dbg_user_id_hash_engine_for_user_id1_info, &hash1_valid);

    /*DbgFlowTcamEngineUserIdInfo0*/
    sal_memset(&dbg_flow_tcam_engine_user_id_info, 0, sizeof(dbg_flow_tcam_engine_user_id_info));
    cmd = DRV_IOR(DbgFlowTcamEngineUserIdInfo0_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_user_id_info);
    GetDbgFlowTcamEngineUserIdInfo0(A, tcamResultValid0_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdTcamResultValid[0]);
    GetDbgFlowTcamEngineUserIdInfo0(A, tcamIndex0_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdTcamIndex[0]);
    GetDbgFlowTcamEngineUserIdInfo0(A, valid_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdValid[0]);
    /*DbgFlowTcamEngineUserIdInfo1*/
    cmd = DRV_IOR(DbgFlowTcamEngineUserIdInfo1_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_user_id_info);
    GetDbgFlowTcamEngineUserIdInfo1(A, tcamResultValid1_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdTcamResultValid[1]);
    GetDbgFlowTcamEngineUserIdInfo1(A, tcamIndex1_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdTcamIndex[1]);
    GetDbgFlowTcamEngineUserIdInfo1(A, valid_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdValid[1]);
    /*DbgFlowTcamEngineUserIdInfo2*/
    cmd = DRV_IOR(DbgFlowTcamEngineUserIdInfo2_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_user_id_info);
    GetDbgFlowTcamEngineUserIdInfo2(A, tcamResultValid2_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdTcamResultValid[2]);
    GetDbgFlowTcamEngineUserIdInfo2(A, tcamIndex2_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdTcamIndex[2]);
    GetDbgFlowTcamEngineUserIdInfo2(A, valid_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdValid[2]);
    /*DbgFlowTcamEngineUserIdInfo3*/
    cmd = DRV_IOR(DbgFlowTcamEngineUserIdInfo3_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_user_id_info);
    GetDbgFlowTcamEngineUserIdInfo3(A, tcamResultValid3_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdTcamResultValid[3]);
    GetDbgFlowTcamEngineUserIdInfo3(A, tcamIndex3_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdTcamIndex[3]);
    GetDbgFlowTcamEngineUserIdInfo3(A, valid_f, &dbg_flow_tcam_engine_user_id_info, &tcam_userIdValid[3]);

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    if(scl_valid)
    {
        if ((hash0_keyType && hash0_valid)|| (hash1_keyType && hash1_valid) || scl_userIdTcamEn[0] ||
            scl_userIdTcamEn[1] ||scl_userIdTcamEn[2] || scl_userIdTcamEn[3])
        {
            CTC_DKIT_PRINT("SCL Process:\n");

            for (lkp_level = 0; lkp_level < 4; lkp_level += 1)
            {
                if (ad_proc_valid[lkp_level])
                {
                    CTC_DKIT_PATH_PRINT("%s%-5d:%s\n", "AD-ds-type", lkp_level, str_ad_ds_type[ad_ds_type[lkp_level]]);
                }
            }

            /*hash0*/
            if (hash0_keyType && hash0_valid)
            {
                CTC_DKIT_PATH_PRINT("%-15s\n", "HASH0 lookup--->");
                if (hash0_hashConflict)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_CONFLICT);
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", hash0_keyIndex);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", _ctc_usw_dkit_captured_get_igr_scl_key_desc(lchip, hash0_keyType, 0));
                }
                else if (hash0_defaultEntryValid)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_DEFAULT);
                }
                else if (hash0_lookupResultValid)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
                }
                else
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
                }
                if (hash0_defaultEntryValid || hash0_lookupResultValid)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", hash0_keyIndex);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", _ctc_usw_dkit_captured_get_igr_scl_key_desc(lchip, hash0_keyType, 0));

                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "AD index", hash0_adIndex);

                    if (scl_sclFlowHashEn)
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", "DsSclFlow");
                    }
                    else if (hash0_keyType && (hash0_keyType < USERIDHASHTYPE_TUNNELIPV4))
                    {
                        cmd = DRV_IOR(DsUserIdHalf_t, DsUserIdHalf_isHalf_f);
                        DRV_IOCTL(lchip, hash0_adIndex, cmd, &is_half);
                        if (is_half)
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", "DsUserIdHalf");
                        }
                        else
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", "DsUserId");
                        }
                    }
                    else if (hash0_keyType >= USERIDHASHTYPE_TUNNELIPV4)
                    {
                        cmd = DRV_IOR(DsTunnelIdHalf_t, DsTunnelIdHalf_isHalf_f);
                        DRV_IOCTL(lchip, hash0_adIndex, cmd, &is_half);
                        if (is_half)
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", "DsTunnelIdHalf");
                        }
                        else
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", "DsTunnelId");
                        }
                    }
                }
            }
            /*hash1*/
            if (hash1_keyType && hash1_valid)
            {
                lkp_level = DRV_IS_DUET2(lchip) ? 0:1;
                CTC_DKIT_PATH_PRINT("%-15s\n", "HASH1 lookup--->");
                if (hash1_hashConflict)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_CONFLICT);
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", hash1_keyIndex);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", _ctc_usw_dkit_captured_get_igr_scl_key_desc(lchip, hash1_keyType, lkp_level));
                }
                else if (hash1_defaultEntryValid)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_DEFAULT);
                }
                else if (hash1_lookupResultValid)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
                }
                else
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
                }
                if (hash1_defaultEntryValid || hash1_lookupResultValid)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", hash1_keyIndex);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", _ctc_usw_dkit_captured_get_igr_scl_key_desc(lchip, hash1_keyType, lkp_level));
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "AD index", hash1_adIndex);
                    if (hash1_keyType && (hash1_keyType < USERIDHASHTYPE_TUNNELIPV4))
                    {
                        cmd = DRV_IS_DUET2(lchip) ? DRV_IOR(DsUserIdHalf_t, DsUserIdHalf_isHalf_f):DRV_IOR(DsUserIdHalf1_t, DsUserIdHalf1_isHalf_f);
                        DRV_IOCTL(lchip, hash1_adIndex, cmd, &is_half);
                        if (is_half)
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", DRV_IS_DUET2(lchip) ? "DsUserIdHalf":"DsUserIdHalf1");
                        }
                        else
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", DRV_IS_DUET2(lchip) ? "DsUserId":"DsUserId1" );
                        }
                    }
                    else if (hash1_keyType >= USERIDHASHTYPE_TUNNELIPV4)
                    {
                        cmd = DRV_IS_DUET2(lchip) ? DRV_IOR(DsTunnelIdHalf_t, DsTunnelIdHalf_isHalf_f) : DRV_IOR(DsTunnelIdHalf1_t, DsTunnelIdHalf1_isHalf_f);
                        DRV_IOCTL(lchip, hash0_adIndex, cmd, &is_half);
                        if (is_half)
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", DRV_IS_DUET2(lchip) ? "DsTunnelIdHalf":"DsTunnelIdHalf1");
                        }
                        else
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", DRV_IS_DUET2(lchip) ? "DsTunnelId":"DsTunnelId1");
                        }
                    }
                }
            }
            /*tcam0~3*/
            for (i = 0; i < 4; i++)
            {
                if (scl_userIdTcamEn[i] && tcam_userIdValid[i])
                {
                    CTC_DKIT_PATH_PRINT("TCAM%d lookup---> \n", i);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key type", str_scl_key_type[scl_userIdTcamType[i]]);
                    if (tcam_userIdTcamResultValid[i])
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_TCAM_HIT);
                        _ctc_usw_dkit_map_tcam_key_index(lchip, tcam_userIdTcamIndex[i], 0, i, &key_index);
                        cmd = DRV_IOR(scl_key_info_key_id[i], DRV_ENTRY_FLAG);
                        DRV_IOCTL(lchip, 0, cmd, &dbg_scl_key_info);
                        DRV_GET_FIELD_A(lchip, scl_key_info_key_id[i], scl_key_info_keysizefld[i], &dbg_scl_key_info, &scl_key_size);
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x (%dbit)\n", "key index", key_index / (1 << scl_key_size), 80*(1 << scl_key_size));
                    }
                    else
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
                    }
                }
            }
        }
    }

Discard:
    _ctc_usw_dkit_captured_path_discard_process(p_info, scl_discard, scl_discardType, CTC_DKIT_IPE);
    _ctc_usw_dkit_captured_path_exception_process(p_info, scl_exceptionEn, scl_exceptionIndex, scl_exceptionSubIndex);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_ipe_interface(ctc_dkit_captured_path_info_t* p_info)
{
    uint8 slice = p_info->slice_ipe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;
    uint16 gport = 0;
    uint8 gchip = 0;
    uint32 value = 0;
    uint32 bridge_en = 0;
    uint32 route_en = 0;
    uint32 mpls_en = 0;
    uint32 port_cross_en = 0;
    char* vlan_action[4] = {"None", "Replace", "Add","Delete"};


    /* DbgIpeUserIdInfo */
    uint32 localPhyPort = 0;
    uint32 globalSrcPort = 0;
    /* DbgIpeIntfMapperInfo */
    uint32 vlanId = 0;
    uint32 isPortMac = 0;
    uint32 svlanTagOperationValid = 0;
    uint32 cvlanTagOperationValid = 0;
    uint32 vlanPtr = 0;
    uint32 interfaceId = 0;
    uint32 isRouterMac = 0;
    uint32 discard = 0;
    uint32 discardType = 0;
    uint32 bypassAll = 0;
    uint32 exceptionEn = 0;
    uint32 exceptionIndex = 0;
    uint32 exceptionSubIndex = 0;
    uint32 dsFwdPtrValid = 0;
    uint32 dsFwdPtr = 0;
    uint32 valid = 0;
    DbgIpeUserIdInfo_m dbg_ipe_user_id_info;
    DbgIpeIntfMapperInfo_m dbg_ipe_intf_mapper_info;

    CTC_DKIT_PRINT("Interface Process:\n");
    sal_memset(&dbg_ipe_user_id_info, 0, sizeof(dbg_ipe_user_id_info));
    cmd = DRV_IOR(DbgIpeUserIdInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_ipe_user_id_info);
    GetDbgIpeUserIdInfo(A, localPhyPort_f, &dbg_ipe_user_id_info, &localPhyPort);
     /*GetDbgIpeUserIdInfo(A, globalSrcPort_f, &dbg_ipe_user_id_info, &globalSrcPort);*/
     /*GetDbgIpeUserIdInfo(A, outerVlanIsCVlan_f, &dbg_ipe_user_id_info, &outerVlanIsCVlan);*/
    GetDbgIpeUserIdInfo(A, discard_f, &dbg_ipe_user_id_info, &discard);
    GetDbgIpeUserIdInfo(A, discardType_f, &dbg_ipe_user_id_info, &discardType);
    GetDbgIpeUserIdInfo(A, exceptionEn_f, &dbg_ipe_user_id_info, &exceptionEn);
    GetDbgIpeUserIdInfo(A, exceptionIndex_f, &dbg_ipe_user_id_info, &exceptionIndex);
    GetDbgIpeUserIdInfo(A, exceptionSubIndex_f, &dbg_ipe_user_id_info, &exceptionSubIndex);
    GetDbgIpeUserIdInfo(A, valid_f, &dbg_ipe_user_id_info, &valid);

    if (valid)
    {
        if (CTC_DKIT_DRV_GPORT_IS_LINKAGG_PORT(globalSrcPort))
        {

            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "linkagg group", CTC_DKIT_DRV_GPORT_TO_LINKAGG_ID(globalSrcPort));
        }
        CTC_DKIT_GET_GCHIP(lchip, gchip);
        gport = CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(gchip, (localPhyPort + slice*CTC_DKIT_ONE_SLICE_PORT_NUM));
        CTC_DKIT_PATH_PRINT("%-15s:0x%04x\n", "source gport", gport);
    }

    sal_memset(&dbg_ipe_intf_mapper_info, 0, sizeof(dbg_ipe_intf_mapper_info));
    cmd = DRV_IOR(DbgIpeIntfMapperInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_ipe_intf_mapper_info);
    GetDbgIpeIntfMapperInfo(A, vlanId_f, &dbg_ipe_intf_mapper_info, &vlanId);
     /*GetDbgIpeIntfMapperInfo(A, isLoop_f, &dbg_ipe_intf_mapper_info, &isLoop);*/
    GetDbgIpeIntfMapperInfo(A, isPortMac_f, &dbg_ipe_intf_mapper_info, &isPortMac);
     /*GetDbgIpeIntfMapperInfo(A, sourceCos_f, &dbg_ipe_intf_mapper_info, &sourceCos);*/
     /*GetDbgIpeIntfMapperInfo(A, sourceCfi_f, &dbg_ipe_intf_mapper_info, &sourceCfi);*/
     /*GetDbgIpeIntfMapperInfo(A, ctagCos_f, &dbg_ipe_intf_mapper_info, &ctagCos);*/
     /*GetDbgIpeIntfMapperInfo(A, ctagCfi_f, &dbg_ipe_intf_mapper_info, &ctagCfi);*/
    GetDbgIpeIntfMapperInfo(A, svlanTagOperationValid_f, &dbg_ipe_intf_mapper_info, &svlanTagOperationValid);
    GetDbgIpeIntfMapperInfo(A, cvlanTagOperationValid_f, &dbg_ipe_intf_mapper_info, &cvlanTagOperationValid);
     /*GetDbgIpeIntfMapperInfo(A, classifyStagCos_f, &dbg_ipe_intf_mapper_info, &classifyStagCos);*/
     /*GetDbgIpeIntfMapperInfo(A, classifyStagCfi_f, &dbg_ipe_intf_mapper_info, &classifyStagCfi);*/
     /*GetDbgIpeIntfMapperInfo(A, classifyCtagCos_f, &dbg_ipe_intf_mapper_info, &classifyCtagCos);*/
     /*GetDbgIpeIntfMapperInfo(A, classifyCtagCfi_f, &dbg_ipe_intf_mapper_info, &classifyCtagCfi);*/
    GetDbgIpeIntfMapperInfo(A, vlanPtr_f, &dbg_ipe_intf_mapper_info, &vlanPtr);
    GetDbgIpeIntfMapperInfo(A, interfaceId_f, &dbg_ipe_intf_mapper_info, &interfaceId);
    GetDbgIpeIntfMapperInfo(A, isRouterMac_f, &dbg_ipe_intf_mapper_info, &isRouterMac);
    GetDbgIpeIntfMapperInfo(A, discard_f, &dbg_ipe_intf_mapper_info, &discard);
    GetDbgIpeIntfMapperInfo(A, discardType_f, &dbg_ipe_intf_mapper_info, &discardType);
    GetDbgIpeIntfMapperInfo(A, bypassAll_f, &dbg_ipe_intf_mapper_info, &bypassAll);
    GetDbgIpeIntfMapperInfo(A, exceptionEn_f, &dbg_ipe_intf_mapper_info, &exceptionEn);
    GetDbgIpeIntfMapperInfo(A, exceptionIndex_f, &dbg_ipe_intf_mapper_info, &exceptionIndex);
    GetDbgIpeIntfMapperInfo(A, exceptionSubIndex_f, &dbg_ipe_intf_mapper_info, &exceptionSubIndex);
    GetDbgIpeIntfMapperInfo(A, dsFwdPtrValid_f, &dbg_ipe_intf_mapper_info, &dsFwdPtrValid);
    GetDbgIpeIntfMapperInfo(A, dsFwdPtr_f, &dbg_ipe_intf_mapper_info, &dsFwdPtr);
     /*GetDbgIpeIntfMapperInfo(A, protocolVlanEn_f, &dbg_ipe_intf_mapper_info, &protocolVlanEn);*/
     /*GetDbgIpeIntfMapperInfo(A, tempBypassAll_f, &dbg_ipe_intf_mapper_info, &tempBypassAll);*/
     /*GetDbgIpeIntfMapperInfo(A, routeAllPacket_f, &dbg_ipe_intf_mapper_info, &routeAllPacket);*/
    GetDbgIpeIntfMapperInfo(A, valid_f, &dbg_ipe_intf_mapper_info, &valid);
    if (valid)
    {
        if (vlanPtr < 4096)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "vlan ptr", vlanPtr);
             /*-TBD CTC_DKIT_PATH_PRINT("%-15s:%s\n", "vlan domain", outerVlanIsCVlan?"cvlan domain":"svlan domain");*/
        }
        if (interfaceId)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "l3 intf id", interfaceId);
            cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_vrfId_f);
            DRV_IOCTL(lchip, interfaceId, cmd, &value);
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "intf vrfid", value);
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "route mac", isRouterMac?"YES":"NO");
        }
    }
    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portCrossConnect_f);
    DRV_IOCTL(lchip, (localPhyPort + slice*CTC_DKIT_ONE_SLICE_PORT_NUM), cmd, &port_cross_en);
    if (port_cross_en)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "cross connect", "YES");
    }
    else
    {
    /*bridge en*/
    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_bridgeEn_f);
    DRV_IOCTL(lchip, (localPhyPort + slice*CTC_DKIT_ONE_SLICE_PORT_NUM), cmd, &bridge_en);
    if (vlanPtr < 4096)
    {
        cmd = DRV_IOR(DsSrcVlan_t, DsSrcVlan_bridgeDisable_f);
        DRV_IOCTL(lchip, vlanPtr, cmd, &value);
    }
    bridge_en = bridge_en && (!value);
    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "bridge port", bridge_en?"YES":"NO");
    /*route en*/
    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_routedPort_f);
    DRV_IOCTL(lchip, (localPhyPort + slice*CTC_DKIT_ONE_SLICE_PORT_NUM), cmd, &route_en);
    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "route port", route_en?"YES":"NO");
    }
    /*mpls en*/
    if (interfaceId)
    {
        cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_mplsEn_f);
        DRV_IOCTL(lchip, interfaceId, cmd, &mpls_en);
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "MPLS port", mpls_en? "YES" : "NO");
    }

    if(p_info->is_bsr_captured)
    {
        /*DbgFwdBufStoreInfo*/
        uint32 valid = 0;
        uint32 operationType = 0;
        uint32 svlanTagOperationValid = 0;
        uint32 sTagAction = 0;
        uint32 sourceCos = 0;
        uint32 sourceCfi = 0;
        uint32 svlanTpidIndex = 0;
        uint32 srcVlanId = 0;
        uint32 cvlanTagOperationValid = 0;
        uint32 cTagAction = 0;
        uint32 srcCtagCos = 0;
        uint32 srcCtagCfi = 0;
        uint32 srcCvlanId = 0;

        uint32 svlanTpid = 0;
        uint32 cvlanTagTpid = 0;

        DbgFwdBufStoreInfo_m dbg_fwd_buf_store_info;
        sal_memset(&dbg_fwd_buf_store_info, 0, sizeof(dbg_fwd_buf_store_info));
        cmd = DRV_IOR(DbgFwdBufStoreInfo_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &dbg_fwd_buf_store_info);
        GetDbgFwdBufStoreInfo(A, valid_f, &dbg_fwd_buf_store_info, &valid);
#if 0
        GetDbgFwdBufStoreInfo(A, operationType_f, &dbg_fwd_buf_store_info, &operationType);
        GetDbgFwdBufStoreInfo(A, svlanTagOperationValid_f, &dbg_fwd_buf_store_info, &svlanTagOperationValid);
        GetDbgFwdBufStoreInfo(A, stagAction_f, &dbg_fwd_buf_store_info, &sTagAction);
        GetDbgFwdBufStoreInfo(A, sourceCos_f, &dbg_fwd_buf_store_info, &sourceCos);
        GetDbgFwdBufStoreInfo(A, sourceCfi_f, &dbg_fwd_buf_store_info, &sourceCfi);
        GetDbgFwdBufStoreInfo(A, srcVlanId_f, &dbg_fwd_buf_store_info, &srcVlanId);
#endif
         /*GetDbgFwdBufStoreInfo(A, svlanTpidIndex_f, &dbg_fwd_buf_store_info, &svlanTpidIndex);*/

         /*GetDbgFwdBufStoreInfo(A, u1_normal_cvlanTagOperationValid_f, &dbg_fwd_buf_store_info, &cvlanTagOperationValid);*/
         /*GetDbgFwdBufStoreInfo(A, u1_normal_cTagAction_f, &dbg_fwd_buf_store_info, &cTagAction);*/
         /*GetDbgFwdBufStoreInfo(A, u1_normal_srcCtagCos_f, &dbg_fwd_buf_store_info, &srcCtagCos);*/
         /*GetDbgFwdBufStoreInfo(A, u1_normal_srcCtagCfi_f, &dbg_fwd_buf_store_info, &srcCtagCfi);*/
         /*GetDbgFwdBufStoreInfo(A, u1_normal_srcCvlanId_f, &dbg_fwd_buf_store_info, &srcCvlanId);*/

        if (valid)
        {

             if ((svlanTagOperationValid && sTagAction) || (cvlanTagOperationValid && cTagAction&&(OPERATIONTYPE_NORMAL == operationType)))
             {
                 CTC_DKIT_PATH_PRINT("%-15s\n", "vlan edit--->");
             }
            if (svlanTagOperationValid)
            {
                uint32 field_id[4] = {EpeHdrAdjustCtl_array_0_svlanTagTpid_f,EpeHdrAdjustCtl_array_1_svlanTagTpid_f,
                                                       EpeHdrAdjustCtl_array_2_svlanTagTpid_f,EpeHdrAdjustCtl_array_3_svlanTagTpid_f};
                cmd = DRV_IOR(EpeHdrAdjustCtl_t, field_id[svlanTpidIndex]);
                DRV_IOCTL(lchip, 0, cmd, &svlanTpid);
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "svlan edit", vlan_action[sTagAction]);
                if ((VTAGACTIONTYPE_MODIFY == sTagAction) || (VTAGACTIONTYPE_ADD == sTagAction))
                {
                    CTC_DKIT_PATH_PRINT("%-15s:0x%04x\n", "new TPID", svlanTpid);
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new COS", sourceCos);
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new CFI", sourceCfi);
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new vlan id", srcVlanId);
                }
            }
            if((cvlanTagOperationValid)&&(OPERATIONTYPE_NORMAL == operationType))
            {
                cmd = DRV_IOR(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_cvlanTagTpid_f);
                DRV_IOCTL(lchip, 0, cmd, &cvlanTagTpid);
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "cvlan edit", vlan_action[cTagAction]);
                if ((VTAGACTIONTYPE_MODIFY == cTagAction) || (VTAGACTIONTYPE_ADD == cTagAction))
                {
                    CTC_DKIT_PATH_PRINT("%-15s:0x%04x\n", "new TPID", cvlanTagTpid);
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new COS", srcCtagCos);
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new CFI", srcCtagCfi);
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new vlan id", srcCvlanId);
                }
            }
        }
    }

    _ctc_usw_dkit_captured_path_discard_process(p_info, discard, discardType, CTC_DKIT_IPE);
    _ctc_usw_dkit_captured_path_exception_process(p_info, exceptionEn, exceptionIndex, exceptionSubIndex);
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_ipe_tunnel(ctc_dkit_captured_path_info_t* p_info)
{
    uint8 slice = p_info->slice_ipe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;
    uint32 is_half = 0;
    /*DbgUserIdHashEngineForMpls0Info*/
    uint32 mpls0_lookupResultValid = 0;
    uint32 mpls0_defaultEntryValid = 0;
    uint32 mpls0_keyIndex = 0;
    uint32 mpls0_adIndex = 0;
    uint32 mpls0_hashConflict = 0;
    uint32 mpls0_valid = 0;
    DbgUserIdHashEngineForMpls0Info_m dbg_user_id_hash_engine_for_mpls0_info;
    DbgMplsHashEngineForLabel0Info_m  dbg_mpls_hash_engine_for_label0_info;
    /*DbgUserIdHashEngineForMpls1Info*/
    uint32 mpls1_lookupResultValid = 0;
    uint32 mpls1_defaultEntryValid = 0;
    uint32 mpls1_keyIndex = 0;
    uint32 mpls1_adIndex = 0;
    uint32 mpls1_hashConflict = 0;
    uint32 mpls1_valid = 0;
    DbgUserIdHashEngineForMpls1Info_m dbg_user_id_hash_engine_for_mpls1_info;
    DbgMplsHashEngineForLabel1Info_m  dbg_mpls_hash_engine_for_label1_info;
    /*DbgUserIdHashEngineForMpls2Info*/
    uint32 mpls2_lookupResultValid = 0;
    uint32 mpls2_defaultEntryValid = 0;
    uint32 mpls2_keyIndex = 0;
    uint32 mpls2_adIndex = 0;
    uint32 mpls2_hashConflict = 0;
    uint32 mpls2_valid = 0;
    DbgUserIdHashEngineForMpls2Info_m dbg_user_id_hash_engine_for_mpls2_info;
    DbgMplsHashEngineForLabel2Info_m  dbg_mpls_hash_engine_for_label2_info;
    /*DbgIpeMplsDecapInfo*/
    uint32 tunnel_tunnelDecap1 = 0;
    uint32 tunnel_tunnelDecap2 = 0;
    uint32 tunnel_ipMartianAddress = 0;
    uint32 tunnel_isMplsSwitched = 0;
    uint32 tunnel_innerPacketLookup = 0;
    uint32 tunnel_forceParser = 0;
    uint32 tunnel_secondParserForEcmp = 0;
    uint32 tunnel_isDecap = 0;
    uint32 tunnel_mplsLabelOutOfRange = 0;
    uint32 tunnel_isLeaf = 0;
    uint32 tunnel_logicPortType = 0;
    uint32 tunnel_payloadPacketType = 0;
    uint32 tunnel_dsFwdPtrValid = 0;
    uint32 tunnel_nextHopPtrValid = 0;
    uint32 tunnel_ptpDeepParser = 0;
    uint32 tunnel_ptpExtraLabelNum = 0;
    uint32 tunnel_dsFwdPtr = 0;
    uint32 tunnel_discard = 0;
    uint32 tunnel_discardType = 0;
    uint32 tunnel_bypassAll = 0;
    uint32 tunnel_exceptionEn = 0;
    uint32 tunnel_exceptionIndex = 0;
    uint32 tunnel_exceptionSubIndex = 0;
    uint32 tunnel_fatalExceptionValid = 0;
    uint32 tunnel_fatalException = 0;
    uint32 tunnel_innerParserOffset = 0;
    uint32 tunnel_innerParserPacketType = 0;
    uint32 tunnel_mplsLmEn0 = 0;
    uint32 tunnel_mplsLmEn1 = 0;
    uint32 tunnel_mplsLmEn2 = 0;
    uint32 tunnel_valid = 0;
    char   ad_name[3][16];
    DbgIpeMplsDecapInfo_m dbg_ipe_mpls_decap_info;

    if (DRV_IS_DUET2(lchip))
    {
        /*DbgUserIdHashEngineForMpls0Info*/
        sal_memset(&dbg_user_id_hash_engine_for_mpls0_info, 0, sizeof(dbg_user_id_hash_engine_for_mpls0_info));
        cmd = DRV_IOR(DbgUserIdHashEngineForMpls0Info_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, slice, cmd, &dbg_user_id_hash_engine_for_mpls0_info);
        GetDbgUserIdHashEngineForMpls0Info(A, lookupResultValid_f, &dbg_user_id_hash_engine_for_mpls0_info, &mpls0_lookupResultValid );
         /*- GetDbgUserIdHashEngineForMpls0Info(A, defaultEntryValid_f, &dbg_user_id_hash_engine_for_mpls0_info, &mpls0_defaultEntryValid );*/
        GetDbgUserIdHashEngineForMpls0Info(A, keyIndex_f, &dbg_user_id_hash_engine_for_mpls0_info, &mpls0_keyIndex);
        GetDbgUserIdHashEngineForMpls0Info(A, adIndex_f, &dbg_user_id_hash_engine_for_mpls0_info, &mpls0_adIndex);
         /*- GetDbgUserIdHashEngineForMpls0Info(A, hashConflict_f, &dbg_user_id_hash_engine_for_mpls0_info, &mpls0_hashConflict);*/
         /*- GetDbgUserIdHashEngineForMpls0Info(A, camLookupResultValid_f, &dbg_user_id_hash_engine_for_mpls0_info, &mpls0_CAMlookupResultValid);*/
         /*- GetDbgUserIdHashEngineForMpls0Info(A, keyIndexCam_f, &dbg_user_id_hash_engine_for_mpls0_info, &mpls0_keyIndexCam);*/
        GetDbgUserIdHashEngineForMpls0Info(A, valid_f, &dbg_user_id_hash_engine_for_mpls0_info, &mpls0_valid);

        /*DbgUserIdHashEngineForMpls1Info*/
        sal_memset(&dbg_user_id_hash_engine_for_mpls1_info, 0, sizeof(dbg_user_id_hash_engine_for_mpls1_info));
        cmd = DRV_IOR(DbgUserIdHashEngineForMpls1Info_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, slice, cmd, &dbg_user_id_hash_engine_for_mpls1_info);
        GetDbgUserIdHashEngineForMpls1Info(A, lookupResultValid_f, &dbg_user_id_hash_engine_for_mpls1_info, &mpls1_lookupResultValid );
         /*- GetDbgUserIdHashEngineForMpls1Info(A, defaultEntryValid_f, &dbg_user_id_hash_engine_for_mpls1_info, &mpls1_defaultEntryValid );*/
        GetDbgUserIdHashEngineForMpls1Info(A, keyIndex_f, &dbg_user_id_hash_engine_for_mpls1_info, &mpls1_keyIndex);
        GetDbgUserIdHashEngineForMpls1Info(A, adIndex_f, &dbg_user_id_hash_engine_for_mpls1_info, &mpls1_adIndex);
         /*- GetDbgUserIdHashEngineForMpls1Info(A, hashConflict_f, &dbg_user_id_hash_engine_for_mpls1_info, &mpls1_hashConflict);*/
         /*- GetDbgUserIdHashEngineForMpls1Info(A, camLookupResultValid_f, &dbg_user_id_hash_engine_for_mpls1_info, &mpls1_CAMlookupResultValid);*/
         /*- GetDbgUserIdHashEngineForMpls1Info(A, keyIndexCam_f, &dbg_user_id_hash_engine_for_mpls1_info, &mpls1_keyIndexCam);*/
        GetDbgUserIdHashEngineForMpls1Info(A, valid_f, &dbg_user_id_hash_engine_for_mpls1_info, &mpls1_valid);

        /*DbgUserIdHashEngineForMpls2Info*/
        sal_memset(&dbg_user_id_hash_engine_for_mpls2_info, 0, sizeof(dbg_user_id_hash_engine_for_mpls2_info));
        cmd = DRV_IOR(DbgUserIdHashEngineForMpls2Info_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, slice, cmd, &dbg_user_id_hash_engine_for_mpls2_info);
        GetDbgUserIdHashEngineForMpls2Info(A, lookupResultValid_f, &dbg_user_id_hash_engine_for_mpls2_info, &mpls2_lookupResultValid );
         /*- GetDbgUserIdHashEngineForMpls2Info(A, defaultEntryValid_f, &dbg_user_id_hash_engine_for_mpls2_info, &mpls2_defaultEntryValid );*/
        GetDbgUserIdHashEngineForMpls2Info(A, keyIndex_f, &dbg_user_id_hash_engine_for_mpls2_info, &mpls2_keyIndex);
        GetDbgUserIdHashEngineForMpls2Info(A, adIndex_f, &dbg_user_id_hash_engine_for_mpls2_info, &mpls2_adIndex);
         /*- GetDbgUserIdHashEngineForMpls2Info(A, hashConflict_f, &dbg_user_id_hash_engine_for_mpls2_info, &mpls2_hashConflict);*/
         /*- GetDbgUserIdHashEngineForMpls2Info(A, camLookupResultValid_f, &dbg_user_id_hash_engine_for_mpls2_info, &mpls2_CAMlookupResultValid);*/
         /*- GetDbgUserIdHashEngineForMpls2Info(A, keyIndexCam_f, &dbg_user_id_hash_engine_for_mpls2_info, &mpls2_keyIndexCam);*/
        GetDbgUserIdHashEngineForMpls2Info(A, valid_f, &dbg_user_id_hash_engine_for_mpls2_info, &mpls2_valid);
    }
    else
    {
        /*DbgMplsHashEngineForLabel0Info*/
        sal_memset(&dbg_mpls_hash_engine_for_label0_info, 0, sizeof(dbg_mpls_hash_engine_for_label0_info));
        cmd = DRV_IOR(DbgMplsHashEngineForLabel0Info_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, slice, cmd, &dbg_mpls_hash_engine_for_label0_info);
        GetDbgMplsHashEngineForLabel0Info(A, lookupResultValid_f, &dbg_mpls_hash_engine_for_label0_info, &mpls0_lookupResultValid );
        GetDbgMplsHashEngineForLabel0Info(A, keyIndex_f, &dbg_mpls_hash_engine_for_label0_info, &mpls0_keyIndex);
        GetDbgMplsHashEngineForLabel0Info(A, mplsHashConflict_f, &dbg_mpls_hash_engine_for_label0_info, &mpls0_hashConflict);
        GetDbgMplsHashEngineForLabel0Info(A, valid_f, &dbg_mpls_hash_engine_for_label0_info, &mpls0_valid);

        /*DbgMplsHashEngineForLabel1Info*/
        sal_memset(&dbg_mpls_hash_engine_for_label1_info, 0, sizeof(dbg_mpls_hash_engine_for_label1_info));
        cmd = DRV_IOR(DbgMplsHashEngineForLabel1Info_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, slice, cmd, &dbg_mpls_hash_engine_for_label1_info);
        GetDbgMplsHashEngineForLabel1Info(A, lookupResultValid_f, &dbg_mpls_hash_engine_for_label1_info, &mpls1_lookupResultValid );
        GetDbgMplsHashEngineForLabel1Info(A, keyIndex_f, &dbg_mpls_hash_engine_for_label1_info, &mpls1_keyIndex);
        GetDbgMplsHashEngineForLabel1Info(A, mplsHashConflict_f, &dbg_mpls_hash_engine_for_label0_info, &mpls1_hashConflict);
        GetDbgMplsHashEngineForLabel1Info(A, valid_f, &dbg_mpls_hash_engine_for_label1_info, &mpls1_valid);

        /*DbgMplsHashEngineForLabel2Info*/
        sal_memset(&dbg_mpls_hash_engine_for_label2_info, 0, sizeof(dbg_mpls_hash_engine_for_label2_info));
        cmd = DRV_IOR(DbgMplsHashEngineForLabel2Info_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, slice, cmd, &dbg_mpls_hash_engine_for_label2_info);
        GetDbgMplsHashEngineForLabel2Info(A, lookupResultValid_f, &dbg_mpls_hash_engine_for_label2_info, &mpls2_lookupResultValid );
        GetDbgMplsHashEngineForLabel2Info(A, keyIndex_f, &dbg_mpls_hash_engine_for_label2_info, &mpls2_keyIndex);
        GetDbgMplsHashEngineForLabel2Info(A, mplsHashConflict_f, &dbg_mpls_hash_engine_for_label0_info, &mpls2_hashConflict);
        GetDbgMplsHashEngineForLabel2Info(A, valid_f, &dbg_mpls_hash_engine_for_label2_info, &mpls2_valid);
    }
    /*DbgIpeMplsDecapInfo*/
    sal_memset(&dbg_ipe_mpls_decap_info, 0, sizeof(dbg_ipe_mpls_decap_info));
    cmd = DRV_IOR(DbgIpeMplsDecapInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_ipe_mpls_decap_info);
    if (DRV_IS_DUET2(lchip))
    {
        GetDbgIpeMplsDecapInfo(A, tunnelDecap1_f, &dbg_ipe_mpls_decap_info, &tunnel_tunnelDecap1);
        GetDbgIpeMplsDecapInfo(A, tunnelDecap2_f, &dbg_ipe_mpls_decap_info, &tunnel_tunnelDecap2);
    }
    else
    {
        GetDbgIpeMplsDecapInfo(A, tunnelDecap0_f, &dbg_ipe_mpls_decap_info, &tunnel_tunnelDecap1);
        GetDbgIpeMplsDecapInfo(A, tunnelDecap1_f, &dbg_ipe_mpls_decap_info, &tunnel_tunnelDecap2);
    }
    GetDbgIpeMplsDecapInfo(A, ipMartianAddress_f, &dbg_ipe_mpls_decap_info, &tunnel_ipMartianAddress);
     /*- GetDbgIpeMplsDecapInfo(A, labelNum_f, &dbg_ipe_mpls_decap_info, &tunnel_labelNum);*/
    GetDbgIpeMplsDecapInfo(A, isMplsSwitched_f, &dbg_ipe_mpls_decap_info, &tunnel_isMplsSwitched);
     /*- GetDbgIpeMplsDecapInfo(A, contextLabelExist_f, &dbg_ipe_mpls_decap_info, &tunnel_contextLabelExist);*/
    GetDbgIpeMplsDecapInfo(A, innerPacketLookup_f, &dbg_ipe_mpls_decap_info, &tunnel_innerPacketLookup);
    GetDbgIpeMplsDecapInfo(A, forceParser_f, &dbg_ipe_mpls_decap_info, &tunnel_forceParser);
    GetDbgIpeMplsDecapInfo(A, secondParserForEcmp_f, &dbg_ipe_mpls_decap_info, &tunnel_secondParserForEcmp);
    GetDbgIpeMplsDecapInfo(A, isDecap_f, &dbg_ipe_mpls_decap_info, &tunnel_isDecap);
     /*- GetDbgIpeMplsDecapInfo(A, srcDscp_f, &dbg_ipe_mpls_decap_info, &tunnel_srcDscp);*/
    GetDbgIpeMplsDecapInfo(A, mplsLabelOutOfRange_f, &dbg_ipe_mpls_decap_info, &tunnel_mplsLabelOutOfRange);
     /*- GetDbgIpeMplsDecapInfo(A, useEntropyLabelHash_f, &dbg_ipe_mpls_decap_info, &tunnel_useEntropyLabelHash);*/
     /*- GetDbgIpeMplsDecapInfo(A, rxOamType_f, &dbg_ipe_mpls_decap_info, &tunnel_rxOamType);*/
     /*- GetDbgIpeMplsDecapInfo(A, packetTtl_f, &dbg_ipe_mpls_decap_info, &tunnel_packetTtl);*/
    GetDbgIpeMplsDecapInfo(A, isLeaf_f, &dbg_ipe_mpls_decap_info, &tunnel_isLeaf);
    GetDbgIpeMplsDecapInfo(A, logicPortType_f, &dbg_ipe_mpls_decap_info, &tunnel_logicPortType);
     /*- GetDbgIpeMplsDecapInfo(A, serviceAclQosEn_f, &dbg_ipe_mpls_decap_info, &tunnel_serviceAclQosEn);*/
    GetDbgIpeMplsDecapInfo(A, payloadPacketType_f, &dbg_ipe_mpls_decap_info, &tunnel_payloadPacketType);
    GetDbgIpeMplsDecapInfo(A, dsFwdPtrValid_f, &dbg_ipe_mpls_decap_info, &tunnel_dsFwdPtrValid);
    GetDbgIpeMplsDecapInfo(A, nextHopPtrValid_f, &dbg_ipe_mpls_decap_info, &tunnel_nextHopPtrValid);
     /*- GetDbgIpeMplsDecapInfo(A, isPtpTunnel_f, &dbg_ipe_mpls_decap_info, &tunnel_isPtpTunnel);*/
    GetDbgIpeMplsDecapInfo(A, ptpDeepParser_f, &dbg_ipe_mpls_decap_info, &tunnel_ptpDeepParser);
    GetDbgIpeMplsDecapInfo(A, ptpExtraLabelNum_f, &dbg_ipe_mpls_decap_info, &tunnel_ptpExtraLabelNum);
    GetDbgIpeMplsDecapInfo(A, dsFwdPtr_f, &dbg_ipe_mpls_decap_info, &tunnel_dsFwdPtr);
    GetDbgIpeMplsDecapInfo(A, discard_f, &dbg_ipe_mpls_decap_info, &tunnel_discard);
    GetDbgIpeMplsDecapInfo(A, discardType_f, &dbg_ipe_mpls_decap_info, &tunnel_discardType);
    GetDbgIpeMplsDecapInfo(A, bypassAll_f, &dbg_ipe_mpls_decap_info, &tunnel_bypassAll);
    GetDbgIpeMplsDecapInfo(A, exceptionEn_f, &dbg_ipe_mpls_decap_info, &tunnel_exceptionEn);
    GetDbgIpeMplsDecapInfo(A, exceptionIndex_f, &dbg_ipe_mpls_decap_info, &tunnel_exceptionIndex);
    GetDbgIpeMplsDecapInfo(A, exceptionSubIndex_f, &dbg_ipe_mpls_decap_info, &tunnel_exceptionSubIndex);
    GetDbgIpeMplsDecapInfo(A, fatalExceptionValid_f, &dbg_ipe_mpls_decap_info, &tunnel_fatalExceptionValid);
    GetDbgIpeMplsDecapInfo(A, fatalException_f, &dbg_ipe_mpls_decap_info, &tunnel_fatalException);
    GetDbgIpeMplsDecapInfo(A, innerParserOffset_f, &dbg_ipe_mpls_decap_info, &tunnel_innerParserOffset);
    GetDbgIpeMplsDecapInfo(A, innerParserPacketType_f, &dbg_ipe_mpls_decap_info, &tunnel_innerParserPacketType);
    GetDbgIpeMplsDecapInfo(A, mplsLmEn0_f, &dbg_ipe_mpls_decap_info, &tunnel_mplsLmEn0);
    GetDbgIpeMplsDecapInfo(A, mplsLmEn1_f, &dbg_ipe_mpls_decap_info, &tunnel_mplsLmEn1);
    GetDbgIpeMplsDecapInfo(A, mplsLmEn2_f, &dbg_ipe_mpls_decap_info, &tunnel_mplsLmEn2);
     /*- GetDbgIpeMplsDecapInfo(A, firstLabel_f, &dbg_ipe_mpls_decap_info, &tunnel_firstLabel);*/
    GetDbgIpeMplsDecapInfo(A, valid_f, &dbg_ipe_mpls_decap_info, &tunnel_valid);

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    if (tunnel_valid)
    {
        if (tunnel_tunnelDecap1 || tunnel_tunnelDecap2)
        {
            CTC_DKIT_PRINT("Tunnel Process:\n");
            if (tunnel_innerPacketLookup)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "forward by", "inner packet");
            }
            else
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "forward by", "tunnel");
            }
        }
        else if (tunnel_isMplsSwitched)
        {
            uint8 i = 0;
            uint32 mpls_valid[3] = {mpls0_valid, mpls1_valid, mpls2_valid};
            uint32 mpls_key_index[3] = {mpls0_keyIndex, mpls1_keyIndex, mpls2_keyIndex};
            uint32 mpls_ad_index[3] = {mpls0_adIndex, mpls1_adIndex, mpls2_adIndex};
            uint32 mpls_conflict[3] = {mpls0_hashConflict, mpls1_hashConflict, mpls2_hashConflict};
            uint32 mpls_default[3] = {mpls0_defaultEntryValid, mpls1_defaultEntryValid, mpls2_defaultEntryValid};
            uint32 mpls_result_valid[3] = {mpls0_lookupResultValid, mpls1_lookupResultValid, mpls2_lookupResultValid};
            CTC_DKIT_PRINT("MPLS Process:\n");
            for (i = 0; i < 3; i++)
            {
                if (mpls_valid[i])
                {
                    if (DRV_IS_DUET2(lchip))
                    {
                        cmd = DRV_IOR(DsMplsHalf_t, DsMplsHalf_isHalf_f);
                        DRV_IOCTL(lchip, mpls_ad_index[i], cmd, &is_half);
                        sal_sprintf(ad_name[i], is_half ? "DsMplsHalf" : "DsMpls");
                    }
                    else
                    {
                        uint16 cam_num = 0;
                        cam_num = TABLE_MAX_INDEX(lchip, DsMplsHashCam_t);
                        mpls_ad_index[i] = (mpls_key_index[i] >= cam_num) ? (mpls_key_index[i] - cam_num):(mpls_key_index[i]);
                        sal_sprintf(ad_name[i], (mpls_key_index[i] < cam_num) ? "DsMplsHashCamAd" : "DsMpls");
                    }
                    if (0 == i)
                    {
                        CTC_DKIT_PATH_PRINT("%-15s\n", "1st label lookup--->");
                    }
                    else if (1 == i)
                    {
                        CTC_DKIT_PATH_PRINT("%-15s\n", "2nd label lookup--->");
                    }
                    else if (2 == i)
                    {
                        CTC_DKIT_PATH_PRINT("%-15s\n", "3th label lookup--->");
                    }
                    if (mpls_conflict[i])
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_CONFLICT);
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", mpls_key_index[i]);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", DRV_IS_DUET2(lchip) ? "DsUserIdTunnelMplsHashKey":"DsMplsLabelHashKey");
                    }
                    else if (mpls_default[i])
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_DEFAULT);
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", mpls_key_index[i]);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name",  DRV_IS_DUET2(lchip) ? "DsUserIdTunnelMplsHashKey":"DsMplsLabelHashKey");
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "AD index", mpls_ad_index[i]);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", ad_name[i]);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "pop lable", "YES");
                    }
                    else if (mpls_result_valid[i])
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", mpls_key_index[i]);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name",  DRV_IS_DUET2(lchip) ? "DsUserIdTunnelMplsHashKey":"DsMplsLabelHashKey");
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "AD index", mpls_ad_index[i]);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", ad_name[i]);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "pop lable", "YES");
                    }
                    else
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
                    }
                }
            }
            if(tunnel_innerPacketLookup)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "forward by", "inner packet");
            }
            else
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "forward by", "MPLS label");
            }
        }
    }

Discard:
    _ctc_usw_dkit_captured_path_discard_process(p_info, tunnel_discard, tunnel_discardType, CTC_DKIT_IPE);
    _ctc_usw_dkit_captured_path_exception_process(p_info, tunnel_exceptionEn, tunnel_exceptionIndex, tunnel_exceptionSubIndex);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_ipe_flow_hash(ctc_dkit_captured_path_info_t* p_info)
{
    uint8  slice = p_info->slice_ipe;
    uint8  lchip = p_info->lchip;
    uint32 cmd = 0;

    char*  str_hash_key_name[] = {"Invalid", "DsFlowL2HashKey", "DsFlowL2L3HashKey", "DsFlowL3Ipv4HashKey",
                                    "DsFlowL3Ipv6HashKey", "DsFlowL3MplsHashKey"};
    char*  str_field_sel_name[] = {"Invalid", "FlowL2HashFieldSelect", "FlowL2L3HashFieldSelect", "FlowL3Ipv4HashFieldSelect",
                                    "FlowL3Ipv6HashFieldSelect", "FlowL3MplsHashFieldSelect"};

    /*DbgFibLkpEngineFlowHashInfo*/
    uint32 hashKeyType = 0;
    uint32 hashLookupEn = 0;
    uint32 flowFieldSel = 0;
    uint32 flowL2KeyUseCvlan = 0;
    uint32 lookupResultValid = 0;
    uint32 hashConflict = 0;
    uint32 keyIndex = 0;
    uint32 dsAdIndex = 0;
    uint32 hash_valid = 0;
    DbgFibLkpEngineFlowHashInfo_m dbg_fib_lkp_engine_flow_hash_info;


    /*DbgFibLkpEngineFlowHashInfo*/
    sal_memset(&dbg_fib_lkp_engine_flow_hash_info, 0, sizeof(dbg_fib_lkp_engine_flow_hash_info));
    cmd = DRV_IOR(DbgFibLkpEngineFlowHashInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_fib_lkp_engine_flow_hash_info);
    GetDbgFibLkpEngineFlowHashInfo(A, gFlowHashLkp_hashKeyType_f, &dbg_fib_lkp_engine_flow_hash_info, &hashKeyType);
    GetDbgFibLkpEngineFlowHashInfo(A, gFlowHashLkp_flowHashLookupEn_f, &dbg_fib_lkp_engine_flow_hash_info, &hashLookupEn);
    GetDbgFibLkpEngineFlowHashInfo(A, gFlowHashLkp_flowFieldSel_f, &dbg_fib_lkp_engine_flow_hash_info, &flowFieldSel);
    GetDbgFibLkpEngineFlowHashInfo(A, gFlowHashLkp_flowL2KeyUseCvlan_f, &dbg_fib_lkp_engine_flow_hash_info, &flowL2KeyUseCvlan);
    GetDbgFibLkpEngineFlowHashInfo(A, gFlowHashResult_lookupResultValid_f, &dbg_fib_lkp_engine_flow_hash_info, &lookupResultValid);
    GetDbgFibLkpEngineFlowHashInfo(A, gFlowHashResult_hashConflict_f, &dbg_fib_lkp_engine_flow_hash_info, &hashConflict);
    GetDbgFibLkpEngineFlowHashInfo(A, gFlowHashResult_keyIndex_f, &dbg_fib_lkp_engine_flow_hash_info, &keyIndex);
    GetDbgFibLkpEngineFlowHashInfo(A, gFlowHashResult_dsAdIndex_f, &dbg_fib_lkp_engine_flow_hash_info, &dsAdIndex);
    GetDbgFibLkpEngineFlowHashInfo(A, valid_f, &dbg_fib_lkp_engine_flow_hash_info, &hash_valid);

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    /*hash*/
    if (hash_valid && hashLookupEn)
    {
        CTC_DKIT_PRINT("Flow HASH lookup--->");
        if (hashConflict)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_CONFLICT);
            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", keyIndex);
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", str_hash_key_name[hashKeyType]);
        }
        else if (lookupResultValid)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", keyIndex);
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", str_hash_key_name[hashKeyType]);
            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "field sel", flowFieldSel);
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "field sel name", str_field_sel_name[hashKeyType]);
            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "AD index", dsAdIndex);
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "AD name", "DsFlow");
        }
        else
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
        }

    }

Discard:

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_ipe_acl(ctc_dkit_captured_path_info_t* p_info)
{
    uint8 slice = p_info->slice_ipe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;
    uint8 idx = 0;
    uint32 key_index = 0;
    IpePreLookupAclCtl_m ipe_pre_acl_ctl;
    char* acl_key_type_desc[] = {"TCAML2KEY", "TCAMl3KEY","TCAML3EXTKEY","TCAMIPV6KEY","TCAMIPV6EXTKEY","TCAML2L3KEY","TCAML2L3KEYEXT",
                                 "TCAML2IPV6KEY","TCAMCIDKEY", "TCAMINTFKEY", "TCAMFWDKEY", "TCAMFWDEXT","TCAMCOPPKEY","TCAMCOPPEXTKEY", "TCAMUDFKEY"};
    /*DbgFlowTcamEngineIpeAclInfo*/
    uint32 acl0TcamResultValid = 0;
    uint32 acl0TcamIndex = 0;
    uint32 acl0TcamValid = 0;
    uint32 acl1TcamResultValid = 0;
    uint32 acl1TcamIndex = 0;
    uint32 acl1TcamValid = 0;
    uint32 acl2TcamResultValid = 0;
    uint32 acl2TcamIndex = 0;
    uint32 acl2TcamValid = 0;
    uint32 acl3TcamResultValid = 0;
    uint32 acl3TcamIndex = 0;
    uint32 acl3TcamValid = 0;
    uint32 acl4TcamResultValid = 0;
    uint32 acl4TcamIndex = 0;
    uint32 acl4TcamValid = 0;
    uint32 acl5TcamResultValid = 0;
    uint32 acl5TcamIndex = 0;
    uint32 acl5TcamValid = 0;
    uint32 acl6TcamResultValid = 0;
    uint32 acl6TcamIndex = 0;
    uint32 acl6TcamValid = 0;
    uint32 acl7TcamResultValid = 0;
    uint32 acl7TcamIndex = 0;
    uint32 acl7TcamValid = 0;
    DbgFlowTcamEngineIpeAclInfo0_m dbg_flow_tcam_engine_ipe_acl_info;

    /*DbgFlowTcamEngineIpeAclKeyInfo0-7*/
    uint32 acl_key_info[10] = {0};
    uint32 acl_key_valid = 0;
    uint32 acl_key_size = 0;  /* 0:80bit; 1:160bit; 2:320bit; 3:640bit */
    DbgFlowTcamEngineIpeAclKeyInfo0_m dbg_acl_key_info;
    uint8  step = DbgFlowTcamEngineIpeAclKeyInfo1_key_f - DbgFlowTcamEngineIpeAclKeyInfo0_key_f;
    uint32 acl_key_info_key_id[8] = {DbgFlowTcamEngineIpeAclKeyInfo0_t, DbgFlowTcamEngineIpeAclKeyInfo1_t,
                                     DbgFlowTcamEngineIpeAclKeyInfo2_t, DbgFlowTcamEngineIpeAclKeyInfo3_t,
                                     DbgFlowTcamEngineIpeAclKeyInfo4_t, DbgFlowTcamEngineIpeAclKeyInfo5_t,
                                     DbgFlowTcamEngineIpeAclKeyInfo6_t, DbgFlowTcamEngineIpeAclKeyInfo7_t};

    uint8 step2 =IpePreLookupAclCtl_gGlbAcl_1_aclLookupType_f - IpePreLookupAclCtl_gGlbAcl_0_aclLookupType_f ;
    uint32 acl_key_type = 0;

    uint32 acl_lkup_en_bitmap = 0;

    /*DbgIpeAclProcInfo*/
    DbgIpeAclProcInfo_m dbg_ipe_acl_proc_info;

    sal_memset(&ipe_pre_acl_ctl, 0, sizeof(ipe_pre_acl_ctl));
    /*DbgFlowTcamEngineIpeAclInfo0*/
    sal_memset(&dbg_flow_tcam_engine_ipe_acl_info, 0, sizeof(dbg_flow_tcam_engine_ipe_acl_info));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo0_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_ipe_acl_info);
    GetDbgFlowTcamEngineIpeAclInfo0(A, ingrTcamResultValid0_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl0TcamResultValid);
    GetDbgFlowTcamEngineIpeAclInfo0(A, ingrTcamIndex0_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl0TcamIndex);
    GetDbgFlowTcamEngineIpeAclInfo0(A, valid_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl0TcamValid);

    /*DbgFlowTcamEngineIpeAclInfo1*/
    sal_memset(&dbg_flow_tcam_engine_ipe_acl_info, 0, sizeof(dbg_flow_tcam_engine_ipe_acl_info));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo1_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_ipe_acl_info);
    GetDbgFlowTcamEngineIpeAclInfo1(A, ingrTcamResultValid1_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl1TcamResultValid);
    GetDbgFlowTcamEngineIpeAclInfo1(A, ingrTcamIndex1_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl1TcamIndex);
    GetDbgFlowTcamEngineIpeAclInfo1(A, valid_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl1TcamValid);

    /*DbgFlowTcamEngineIpeAclInfo2*/
    sal_memset(&dbg_flow_tcam_engine_ipe_acl_info, 0, sizeof(dbg_flow_tcam_engine_ipe_acl_info));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo2_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_ipe_acl_info);
    GetDbgFlowTcamEngineIpeAclInfo2(A, ingrTcamResultValid2_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl2TcamResultValid);
    GetDbgFlowTcamEngineIpeAclInfo2(A, ingrTcamIndex2_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl2TcamIndex);
    GetDbgFlowTcamEngineIpeAclInfo2(A, valid_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl2TcamValid);

    /*DbgFlowTcamEngineIpeAclInfo3*/
    sal_memset(&dbg_flow_tcam_engine_ipe_acl_info, 0, sizeof(dbg_flow_tcam_engine_ipe_acl_info));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo3_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_ipe_acl_info);
    GetDbgFlowTcamEngineIpeAclInfo3(A, ingrTcamResultValid3_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl3TcamResultValid);
    GetDbgFlowTcamEngineIpeAclInfo3(A, ingrTcamIndex3_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl3TcamIndex);
    GetDbgFlowTcamEngineIpeAclInfo3(A, valid_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl3TcamValid);

    /*DbgFlowTcamEngineIpeAclInfo4*/
    sal_memset(&dbg_flow_tcam_engine_ipe_acl_info, 0, sizeof(dbg_flow_tcam_engine_ipe_acl_info));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo4_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_ipe_acl_info);
    GetDbgFlowTcamEngineIpeAclInfo4(A, ingrTcamResultValid4_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl4TcamResultValid);
    GetDbgFlowTcamEngineIpeAclInfo4(A, ingrTcamIndex4_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl4TcamIndex);
    GetDbgFlowTcamEngineIpeAclInfo4(A, valid_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl4TcamValid);

    /*DbgFlowTcamEngineIpeAclInfo5*/
    sal_memset(&dbg_flow_tcam_engine_ipe_acl_info, 0, sizeof(dbg_flow_tcam_engine_ipe_acl_info));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo5_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_ipe_acl_info);
    GetDbgFlowTcamEngineIpeAclInfo5(A, ingrTcamResultValid5_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl5TcamResultValid);
    GetDbgFlowTcamEngineIpeAclInfo5(A, ingrTcamIndex5_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl5TcamIndex);
    GetDbgFlowTcamEngineIpeAclInfo5(A, valid_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl5TcamValid);

    /*DbgFlowTcamEngineIpeAclInfo6*/
    sal_memset(&dbg_flow_tcam_engine_ipe_acl_info, 0, sizeof(dbg_flow_tcam_engine_ipe_acl_info));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo6_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_ipe_acl_info);
    GetDbgFlowTcamEngineIpeAclInfo6(A, ingrTcamResultValid6_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl6TcamResultValid);
    GetDbgFlowTcamEngineIpeAclInfo6(A, ingrTcamIndex6_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl6TcamIndex);
    GetDbgFlowTcamEngineIpeAclInfo6(A, valid_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl6TcamValid);

    /*DbgFlowTcamEngineIpeAclInfo7*/
    sal_memset(&dbg_flow_tcam_engine_ipe_acl_info, 0, sizeof(dbg_flow_tcam_engine_ipe_acl_info));
    cmd = DRV_IOR(DbgFlowTcamEngineIpeAclInfo7_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_ipe_acl_info);
    GetDbgFlowTcamEngineIpeAclInfo7(A, ingrTcamResultValid7_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl7TcamResultValid);
    GetDbgFlowTcamEngineIpeAclInfo7(A, ingrTcamIndex7_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl7TcamIndex);
    GetDbgFlowTcamEngineIpeAclInfo7(A, valid_f, &dbg_flow_tcam_engine_ipe_acl_info, &acl7TcamValid);

    /*IpePreLookupAclCtl*/
    cmd = DRV_IOR(IpePreLookupAclCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ipe_pre_acl_ctl);

    /*DbgIpeFwdProcessInfo*/
    if (DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOR(DbgIpeFwdProcessInfo_t, DbgIpeFwdProcessInfo_aclEnBitMap_f);
        DRV_IOCTL(lchip, slice, cmd, &acl_lkup_en_bitmap);
    }
    else
    {
        cmd = DRV_IOR(DbgIpeAclProcInfo0_t, DbgIpeAclProcInfo0_aclEnBitMap_f);
        DRV_IOCTL(lchip, slice, cmd, &acl_lkup_en_bitmap);
    }
    /*DbgIpeAclProcInfo*/
    sal_memset(&dbg_ipe_acl_proc_info, 0, sizeof(dbg_ipe_acl_proc_info));
    cmd = DRV_IOR(DbgIpeAclProcInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_ipe_acl_proc_info);

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    if (acl_lkup_en_bitmap)
    {
        CTC_DKIT_PRINT("ACL Process:\n");

        /*tcam*/
        if (acl0TcamValid || acl1TcamValid || acl2TcamValid || acl3TcamValid ||
            acl4TcamValid || acl5TcamValid || acl6TcamValid || acl7TcamValid)
        {
            char* lookup_result[8] = {"TCAM0 lookup--->", "TCAM1 lookup--->", "TCAM2 lookup--->", "TCAM3 lookup--->",
                                      "TCAM4 lookup--->", "TCAM5 lookup--->", "TCAM6 lookup--->", "TCAM7 lookup--->"};
            uint32 result_valid[8] = {acl0TcamResultValid, acl1TcamResultValid, acl2TcamResultValid, acl3TcamResultValid,
                                      acl4TcamResultValid, acl5TcamResultValid, acl6TcamResultValid, acl7TcamResultValid};
            uint32 tcam_index[8] = {acl0TcamIndex, acl1TcamIndex, acl2TcamIndex, acl3TcamIndex,
                                    acl4TcamIndex, acl5TcamIndex, acl6TcamIndex, acl7TcamIndex};

            for (idx = 0; idx < 8 ; idx++)
            {
                if ((acl_lkup_en_bitmap >> idx) & 0x1)
                {
                    CTC_DKIT_PATH_PRINT("%-15s\n", lookup_result[idx]);

                    cmd = DRV_IOR(acl_key_info_key_id[idx], DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, 0, cmd, &dbg_acl_key_info);
                    DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineIpeAclKeyInfo0_key_f + step * idx,
                                    &dbg_acl_key_info, &acl_key_info);
                    DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineIpeAclKeyInfo0_valid_f + step * idx,
                                    &dbg_acl_key_info, &acl_key_valid);
                    DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineIpeAclKeyInfo0_keySize_f + step * idx,
                                    &dbg_acl_key_info, &acl_key_size);
                    /*get key type*/
                    if(acl_key_valid)
                    {
                        if(11 == GetIpePreLookupAclCtl(V, gGlbAcl_0_aclLookupType_f + step2 * idx, &ipe_pre_acl_ctl))   /*copp*/
                        {
                            GetDsAclQosCoppKey320Ing0(A, keyType_f, acl_key_info, &acl_key_type);
                            if(0 == acl_key_type)
                            {
                                acl_key_type = 12;
                            }
                            else
                            {
                                acl_key_type = 13;
                            }
                        }
                        else
                        {
                            GetDsAclQosL3Key320Ing0(A, aclQosKeyType0_f, acl_key_info, &acl_key_type);
                        }
                        if (acl_key_type<=14)
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key type", acl_key_type_desc[acl_key_type]);
                        }
                    }
                    if (result_valid[idx])
                    {
                        cmd = DRV_IOR(acl_key_info_key_id[idx], DRV_ENTRY_FLAG);
                        DRV_IOCTL(lchip, 0, cmd, &dbg_acl_key_info);
                        DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineIpeAclKeyInfo0_key_f + step * idx,
                        &dbg_acl_key_info, &acl_key_info);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_TCAM_HIT);
                        _ctc_usw_dkit_map_tcam_key_index(lchip, tcam_index[idx], 1, idx, &key_index);
                        CTC_DKIT_PATH_PRINT("%-15s:%u (%dbit)\n", "key index", key_index/(1<<acl_key_size),80*(1<<acl_key_size));
                    }
                    else
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
                    }
                }
            }
        }

    }

Discard:

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_capture_path_ipe_cid_pair(ctc_dkit_captured_path_info_t* p_info)
{
    uint8  slice = p_info->slice_ipe;
    uint8  lchip = p_info->lchip;
    uint32 cmd = 0;

    uint32  cid_pair_en = 0;
    char*  str_hash_key_name[] = {"DsCategoryIdPairHashRightKey", "DsCategoryIdPairHashLeftKey"};
    char*  str_hash_ad_name[] = {"DsCategoryIdPairHashRightAd", "DsCategoryIdPairHashLeftAd"};

    uint32 cidPairHashAdIndex = 0;
    uint32 cidPairHashEntryOffset = 0;
    uint32 cidPairHashHit = 0;
    uint32 cidPairHashIsLeft = 0;
    uint32 cidPairHashKeyIndex = 0;
    uint32 cidPairTcamHitIndex = 0;
    uint32 cidPairTcamResultValid = 0;
    uint32 destCategoryIdClassfied = 0;
    uint32 destCategoryId = 0;
    uint32 srcCategoryIdClassfied = 0;
    uint32 srcCategoryId = 0;
    DbgIpeFwdProcessInfo_m dbg_fwd_proc_info;
    DbgIpeAclProcInfo0_m   dbg_ipeacl_pro_info;

    uint32 aclLookupLevel = 0;
    uint32 operationMode = 0;
    uint32 aclLabel = 0;
    uint32 aclUseGlobalPortType = 0;
    uint32 aclUsePIVlan = 0;
    uint32 keepAclLabel = 0;
    uint32 keepAclUseGlobalPortType = 0;
    uint32 keepAclUsePIVlan = 0;
    uint32 lookupType = 0;
    uint32 deny = 0;
    uint32 permit = 0;
    DsCategoryIdPair_m     ds_cid_pair;

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    sal_memset(&dbg_fwd_proc_info, 0, sizeof(DbgIpeFwdProcessInfo_m));
    sal_memset(&ds_cid_pair,       0, sizeof(DsCategoryIdPair_m));
    sal_memset(&dbg_ipeacl_pro_info, 0, sizeof(DbgIpeAclProcInfo0_m));

    cmd = DRV_IOR(IpeFwdCategoryCtl_t, IpeFwdCategoryCtl_categoryIdPairLookupEn_f);
    DRV_IOCTL(lchip, 0, cmd, &cid_pair_en);

    if (DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOR(DbgIpeFwdProcessInfo_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, slice, cmd, &dbg_fwd_proc_info);
        GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairHashAdIndex_f     , &dbg_fwd_proc_info, &cidPairHashAdIndex);
        GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairHashEntryOffset_f , &dbg_fwd_proc_info, &cidPairHashEntryOffset);
        GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairHashHit_f         , &dbg_fwd_proc_info, &cidPairHashHit);
        GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairHashIsLeft_f      , &dbg_fwd_proc_info, &cidPairHashIsLeft);
        GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairHashKeyIndex_f    , &dbg_fwd_proc_info, &cidPairHashKeyIndex);
        GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairTcamHitIndex_f    , &dbg_fwd_proc_info, &cidPairTcamHitIndex);
        GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairTcamResultValid_f , &dbg_fwd_proc_info, &cidPairTcamResultValid);
        GetDbgIpeFwdProcessInfo(A, gCidLkp_destCategoryIdClassfied_f, &dbg_fwd_proc_info, &destCategoryIdClassfied);
        GetDbgIpeFwdProcessInfo(A, gCidLkp_destCategoryId_f         , &dbg_fwd_proc_info, &destCategoryId);
        GetDbgIpeFwdProcessInfo(A, gCidLkp_srcCategoryIdClassfied_f , &dbg_fwd_proc_info, &srcCategoryIdClassfied);
        GetDbgIpeFwdProcessInfo(A, gCidLkp_srcCategoryId_f          , &dbg_fwd_proc_info, &srcCategoryId);
    }
    else
    {
        cmd = DRV_IOR(DbgIpeAclProcInfo0_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, slice, cmd, &dbg_ipeacl_pro_info);
        GetDbgIpeAclProcInfo0(A, gCidLkp_cidPairHashAdIndex_f     , &dbg_ipeacl_pro_info, &cidPairHashAdIndex);
        GetDbgIpeAclProcInfo0(A, gCidLkp_cidPairHashEntryOffset_f , &dbg_ipeacl_pro_info, &cidPairHashEntryOffset);
        GetDbgIpeAclProcInfo0(A, gCidLkp_cidPairHashHit_f         , &dbg_ipeacl_pro_info, &cidPairHashHit);
        GetDbgIpeAclProcInfo0(A, gCidLkp_cidPairHashIsLeft_f      , &dbg_ipeacl_pro_info, &cidPairHashIsLeft);
        GetDbgIpeAclProcInfo0(A, gCidLkp_cidPairHashKeyIndex_f    , &dbg_ipeacl_pro_info, &cidPairHashKeyIndex);
        GetDbgIpeAclProcInfo0(A, gCidLkp_cidPairTcamHitIndex_f    , &dbg_ipeacl_pro_info, &cidPairTcamHitIndex);
        GetDbgIpeAclProcInfo0(A, gCidLkp_cidPairTcamResultValid_f , &dbg_ipeacl_pro_info, &cidPairTcamResultValid);
        GetDbgIpeAclProcInfo0(A, gCidLkp_destCategoryIdClassfied_f, &dbg_ipeacl_pro_info, &destCategoryIdClassfied);
        GetDbgIpeAclProcInfo0(A, gCidLkp_destCategoryId_f         , &dbg_ipeacl_pro_info, &destCategoryId);
        GetDbgIpeAclProcInfo0(A, gCidLkp_srcCategoryIdClassfied_f , &dbg_ipeacl_pro_info, &srcCategoryIdClassfied);
        GetDbgIpeAclProcInfo0(A, gCidLkp_srcCategoryId_f          , &dbg_ipeacl_pro_info, &srcCategoryId);
    }

    if(cid_pair_en)
    {
        CTC_DKIT_PRINT("Cid Pair Process:\n");
        if(srcCategoryIdClassfied)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%u\n", "src category id", srcCategoryId);
        }
        else
        {
            CTC_DKIT_PATH_PRINT("src category id not valid!\n");
        }
        if(destCategoryIdClassfied)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%u\n", "dst category id", destCategoryId);
        }
        else
        {
            CTC_DKIT_PATH_PRINT("dest category id not valid!\n");
        }
        CTC_DKIT_PATH_PRINT("Cid Hash Lookup--->\n");
        if (cidPairHashHit)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "key index", cidPairHashKeyIndex);
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "array offset", cidPairHashEntryOffset);
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", str_hash_key_name[cidPairHashIsLeft]);
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Ad index", cidPairHashAdIndex);
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Ad name", str_hash_ad_name[cidPairHashIsLeft]);
            if(cidPairHashIsLeft)
            {
                cmd = DRV_IOR(DsCategoryIdPairHashRightAd_t, DRV_ENTRY_FLAG);
            }
            else
            {
                cmd = DRV_IOR(DsCategoryIdPairHashLeftAd_t, DRV_ENTRY_FLAG);
            }
            DRV_IOCTL(lchip, cidPairHashAdIndex, cmd, &ds_cid_pair);
        }
        else
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
        }
        CTC_DKIT_PATH_PRINT("Cid Tcam Lookup--->\n");
        if (cidPairTcamResultValid)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_TCAM_HIT);
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "key index", cidPairTcamHitIndex);
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", "DsCategoryIdPairTcamKey");
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Ad index", cidPairTcamHitIndex);
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Ad name", "DsCategoryIdPairTcamAd");
            cmd = DRV_IOR(DsCategoryIdPairTcamAd_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, cidPairTcamHitIndex, cmd, &ds_cid_pair);
        }
        else
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
        }
        if(cidPairHashHit || cidPairTcamResultValid)
        {
            GetDsCategoryIdPair(A, aclLookupLevel_f                , &ds_cid_pair, &aclLookupLevel);
            GetDsCategoryIdPair(A, operationMode_f                 , &ds_cid_pair, &operationMode);
            GetDsCategoryIdPair(A, u1_g1_aclLabel_f                , &ds_cid_pair, &aclLabel);
            GetDsCategoryIdPair(A, u1_g1_aclUseGlobalPortType_f    , &ds_cid_pair, &aclUseGlobalPortType);
            GetDsCategoryIdPair(A, u1_g1_aclUsePIVlan_f            , &ds_cid_pair, &aclUsePIVlan);
            GetDsCategoryIdPair(A, u1_g1_keepAclLabel_f            , &ds_cid_pair, &keepAclLabel);
            GetDsCategoryIdPair(A, u1_g1_keepAclUseGlobalPortType_f, &ds_cid_pair, &keepAclUseGlobalPortType);
            GetDsCategoryIdPair(A, u1_g1_keepAclUsePIVlan_f        , &ds_cid_pair, &keepAclUsePIVlan);
            GetDsCategoryIdPair(A, u1_g1_lookupType_f              , &ds_cid_pair, &lookupType);
            GetDsCategoryIdPair(A, u1_g2_deny_f                    , &ds_cid_pair, &deny);
            GetDsCategoryIdPair(A, u1_g2_permit_f                  , &ds_cid_pair, &permit);
            if(operationMode)
            {
                if(permit)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Action", "Permit");
                }
                else if(deny)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Action", "Deny");
                }
            }
            else
            {
                CTC_DKIT_PATH_PRINT("%-15s:Overwrite Acl %d\n", "Action", aclLookupLevel);
                CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Acl lkup type", lookupType);
                if(!keepAclLabel)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Acl label", aclLabel);
                }
                if(!keepAclUsePIVlan)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "use mapped vlan", aclUsePIVlan);
                }

            }

        }
    }

Discard:

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_ipe_forward(ctc_dkit_captured_path_info_t* p_info)
{
    uint8  lchip = p_info->lchip;
    uint32 cmd = 0;

    uint32 phbInfoClassifiedCfi   = 0;
    uint32 phbInfoClassifiedCos   = 0;
    uint32 phbInfoClassifiedDscp  = 0;
    uint32 phbInfoClassifiedTc    = 0;
    uint32 phbInfoColor           = 0;
    uint32 phbInfoPrio            = 0;
    uint32 phbInfoSrcDscpValid    = 0;
    uint32 phbInfoSrcDscp         = 0;
    uint32 phbInfoValid           = 0;
    DbgIpePerHopBehaviorInfo_m dbg_phb_info;

    char*  str_color[] = {"None", "Red", "Yellow", "Green"};

    /*bridge process*/
    _ctc_usw_dkit_capture_path_ipe_bridge(p_info);

    /*route process*/
    _ctc_usw_dkit_capture_path_ipe_route(p_info);

    /*phb process*/
    sal_memset(&dbg_phb_info, 0, sizeof(DbgIpePerHopBehaviorInfo_m));
    cmd = DRV_IOR(DbgIpePerHopBehaviorInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_phb_info);
    GetDbgIpePerHopBehaviorInfo(A, classifiedCfi_f    , &dbg_phb_info, &phbInfoClassifiedCfi);
    GetDbgIpePerHopBehaviorInfo(A, classifiedCos_f    , &dbg_phb_info, &phbInfoClassifiedCos);
    GetDbgIpePerHopBehaviorInfo(A, classifiedDscp_f   , &dbg_phb_info, &phbInfoClassifiedDscp);
    GetDbgIpePerHopBehaviorInfo(A, classifiedTc_f     , &dbg_phb_info, &phbInfoClassifiedTc);
    GetDbgIpePerHopBehaviorInfo(A, color_f            , &dbg_phb_info, &phbInfoColor);
    GetDbgIpePerHopBehaviorInfo(A, prio_f             , &dbg_phb_info, &phbInfoPrio);
    GetDbgIpePerHopBehaviorInfo(A, srcDscpValid_f     , &dbg_phb_info, &phbInfoSrcDscpValid);
    GetDbgIpePerHopBehaviorInfo(A, srcDscp_f          , &dbg_phb_info, &phbInfoSrcDscp);
    GetDbgIpePerHopBehaviorInfo(A, valid_f            , &dbg_phb_info, &phbInfoValid);

    /*policer*/
    if(phbInfoValid)
    {
        CTC_DKIT_PRINT("Phb Process:\n");
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Phb Color", str_color[phbInfoColor]);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Phb Prio", phbInfoPrio);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Phb ClassifiedCfi", phbInfoClassifiedCfi);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Phb ClassifiedCos", phbInfoClassifiedCos);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Phb ClassifiedDscp", phbInfoClassifiedDscp);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Phb ClassifiedTc", phbInfoClassifiedTc);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Phb SrcDscpValid", phbInfoSrcDscpValid);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Phb SrcDscp", phbInfoSrcDscp);
    }

    /*linkagg process*/
    _ctc_usw_dkit_capture_path_ipe_linkagg(p_info);


    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_ipe_oam(ctc_dkit_captured_path_info_t* p_info)
{
    uint8 slice = p_info->slice_ipe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;
    uint8 i = 0;
    char* lookup_result[4] = {"ETHER lookup--->", "BFD lookup--->", "TP lookup--->", "Section lookup--->"};
    char* key_name[4] = {"DsEgressXcOamEthHashKey", "DsEgressXcOamBfdHashKey",
                                             "DsEgressXcOamMplsLabelHashKey", "DsEgressXcOamMplsSectionHashKey"};
    /*DbgEgrXcOamHashEngineFromIpeOam0Info*/
    uint32 hash0_resultValid = 0;
    uint32 hash0_egressXcOamHashConflict = 0;
    uint32 hash0_keyIndex = 0;
    uint32 hash0_valid = 0;
    DbgEgrXcOamHashEngineFromIpeOam0Info_m dbg_egr_xc_oam_hash_engine_from_ipe_oam0_info;
    /*DbgEgrXcOamHashEngineFromIpeOam1Info*/
    uint32 hash1_resultValid = 0;
    uint32 hash1_egressXcOamHashConflict = 0;
    uint32 hash1_keyIndex = 0;
    uint32 hash1_valid = 0;
    DbgEgrXcOamHashEngineFromIpeOam1Info_m dbg_egr_xc_oam_hash_engine_from_ipe_oam1_info;
    /*DbgEgrXcOamHashEngineFromIpeOam2Info*/
    uint32 hash2_resultValid = 0;
    uint32 hash2_egressXcOamHashConflict = 0;
    uint32 hash2_keyIndex = 0;
    uint32 hash2_valid = 0;
    DbgEgrXcOamHashEngineFromIpeOam2Info_m dbg_egr_xc_oam_hash_engine_from_ipe_oam2_info;
    /*DbgIpeLkpMgrInfo*/
    uint32 lkp_oamUseFid = 0;
    uint32 lkp_mplsLmValid = 0;
    uint32 lkp_mplsSectionLmValid = 0;
    uint32 lkp_ingressOamKeyType0 = 0;
    uint32 lkp_ingressOamKeyType1 = 0;
    uint32 lkp_ingressOamKeyType2 = 0;
    uint32 lkp_valid = 0;
    DbgIpeLkpMgrInfo_m dbg_ipe_lkp_mgr_info;
    /*DbgIpeOamInfo*/
    uint32 oam_oamLookupEn = 0;
    uint32 oam_lmStatsEn0 = 0;
    uint32 oam_lmStatsEn1 = 0;
    uint32 oam_lmStatsEn2 = 0;
    uint32 oam_lmStatsIndex = 0;
    uint32 oam_lmStatsPtr0 = 0;
    uint32 oam_lmStatsPtr1 = 0;
    uint32 oam_lmStatsPtr2 = 0;
    uint32 oam_etherLmValid = 0;
    uint32 oam_mplsLmValid = 0;
    uint32 oam_oamDestChipId = 0;
    uint32 oam_mepIndex = 0;
    uint32 oam_mepEn = 0;
    uint32 oam_mipEn = 0;
    uint32 oam_tempRxOamType = 0;
    uint32 oam_valid = 0;
    DbgIpeOamInfo_m dbg_ipe_oam_info;
    uint8 key_type[3] = {lkp_ingressOamKeyType0, lkp_ingressOamKeyType1, lkp_ingressOamKeyType2};
    uint8 hash_valid[3] = {hash0_valid, hash1_valid, hash2_valid};
    uint8 confict[3] = {hash0_egressXcOamHashConflict, hash1_egressXcOamHashConflict, hash2_egressXcOamHashConflict};
    uint32 key_index[3] = {hash0_keyIndex, hash1_keyIndex, hash2_keyIndex};
    uint8 hash_resultValid[3] = {hash0_resultValid,hash1_resultValid,hash2_resultValid};

    /*DbgEgrXcOamHashEngineFromIpeOam0Info*/
    sal_memset(&dbg_egr_xc_oam_hash_engine_from_ipe_oam0_info, 0, sizeof(dbg_egr_xc_oam_hash_engine_from_ipe_oam0_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromIpeOam0Info_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_egr_xc_oam_hash_engine_from_ipe_oam0_info);
    GetDbgEgrXcOamHashEngineFromIpeOam0Info(A, resultValid_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam0_info, &hash0_resultValid);
    GetDbgEgrXcOamHashEngineFromIpeOam0Info(A, hashConflict_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam0_info, &hash0_egressXcOamHashConflict);
    GetDbgEgrXcOamHashEngineFromIpeOam0Info(A, keyIndex_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam0_info, &hash0_keyIndex);
    GetDbgEgrXcOamHashEngineFromIpeOam0Info(A, valid_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam0_info, &hash0_valid);
    /*DbgEgrXcOamHashEngineFromIpeOam1Info*/
    sal_memset(&dbg_egr_xc_oam_hash_engine_from_ipe_oam1_info, 0, sizeof(dbg_egr_xc_oam_hash_engine_from_ipe_oam1_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromIpeOam1Info_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_egr_xc_oam_hash_engine_from_ipe_oam1_info);
    GetDbgEgrXcOamHashEngineFromIpeOam1Info(A, resultValid_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam1_info, &hash1_resultValid);
    GetDbgEgrXcOamHashEngineFromIpeOam1Info(A, hashConflict_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam1_info, &hash1_egressXcOamHashConflict);
    GetDbgEgrXcOamHashEngineFromIpeOam1Info(A, keyIndex_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam1_info, &hash1_keyIndex);
    GetDbgEgrXcOamHashEngineFromIpeOam1Info(A, valid_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam1_info, &hash1_valid);
    /*DbgEgrXcOamHashEngineFromIpeOam2Info*/
    sal_memset(&dbg_egr_xc_oam_hash_engine_from_ipe_oam2_info, 0, sizeof(dbg_egr_xc_oam_hash_engine_from_ipe_oam2_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromIpeOam2Info_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_egr_xc_oam_hash_engine_from_ipe_oam2_info);
    GetDbgEgrXcOamHashEngineFromIpeOam2Info(A, resultValid_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam2_info, &hash2_resultValid);
    GetDbgEgrXcOamHashEngineFromIpeOam2Info(A, hashConflict_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam2_info, &hash2_egressXcOamHashConflict);
    GetDbgEgrXcOamHashEngineFromIpeOam2Info(A, keyIndex_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam2_info, &hash2_keyIndex);
    GetDbgEgrXcOamHashEngineFromIpeOam2Info(A, valid_f, &dbg_egr_xc_oam_hash_engine_from_ipe_oam2_info, &hash2_valid);
    /*DbgIpeLkpMgrInfo*/
    sal_memset(&dbg_ipe_lkp_mgr_info, 0, sizeof(dbg_ipe_lkp_mgr_info));
    cmd = DRV_IOR(DbgIpeLkpMgrInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_ipe_lkp_mgr_info);
    GetDbgIpeLkpMgrInfo(A, oamUseFid_f, &dbg_ipe_lkp_mgr_info, &lkp_oamUseFid);
    GetDbgIpeLkpMgrInfo(A, mplsLmValid_f, &dbg_ipe_lkp_mgr_info, &lkp_mplsLmValid);
    GetDbgIpeLkpMgrInfo(A, mplsSectionLmValid_f, &dbg_ipe_lkp_mgr_info, &lkp_mplsSectionLmValid);
    GetDbgIpeLkpMgrInfo(A, ingressOamKeyType0_f, &dbg_ipe_lkp_mgr_info, &lkp_ingressOamKeyType0);
    GetDbgIpeLkpMgrInfo(A, ingressOamKeyType1_f, &dbg_ipe_lkp_mgr_info, &lkp_ingressOamKeyType1);
    GetDbgIpeLkpMgrInfo(A, ingressOamKeyType2_f, &dbg_ipe_lkp_mgr_info, &lkp_ingressOamKeyType2);
    GetDbgIpeLkpMgrInfo(A, valid_f, &dbg_ipe_lkp_mgr_info, &lkp_valid);
    /*DbgIpeOamInfo*/
    sal_memset(&dbg_ipe_oam_info, 0, sizeof(dbg_ipe_oam_info));
    cmd = DRV_IOR(DbgIpeOamInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_ipe_oam_info);
    GetDbgIpeOamInfo(A, oamLookupEn_f, &dbg_ipe_oam_info, &oam_oamLookupEn);
    GetDbgIpeOamInfo(A, lmStatsEn0_f, &dbg_ipe_oam_info, &oam_lmStatsEn0);
    GetDbgIpeOamInfo(A, lmStatsEn1_f, &dbg_ipe_oam_info, &oam_lmStatsEn1);
    GetDbgIpeOamInfo(A, lmStatsEn2_f, &dbg_ipe_oam_info, &oam_lmStatsEn2);
    GetDbgIpeOamInfo(A, lmStatsIndex_f, &dbg_ipe_oam_info, &oam_lmStatsIndex);
    GetDbgIpeOamInfo(A, lmStatsPtr0_f, &dbg_ipe_oam_info, &oam_lmStatsPtr0);
    GetDbgIpeOamInfo(A, lmStatsPtr1_f, &dbg_ipe_oam_info, &oam_lmStatsPtr1);
    GetDbgIpeOamInfo(A, lmStatsPtr2_f, &dbg_ipe_oam_info, &oam_lmStatsPtr2);
    GetDbgIpeOamInfo(A, etherLmValid_f, &dbg_ipe_oam_info, &oam_etherLmValid);
    GetDbgIpeOamInfo(A, mplsLmValid_f, &dbg_ipe_oam_info, &oam_mplsLmValid);
    GetDbgIpeOamInfo(A, oamDestChipId_f, &dbg_ipe_oam_info, &oam_oamDestChipId);
    GetDbgIpeOamInfo(A, mepIndex_f, &dbg_ipe_oam_info, &oam_mepIndex);
    GetDbgIpeOamInfo(A, mepEn_f, &dbg_ipe_oam_info, &oam_mepEn);
    GetDbgIpeOamInfo(A, mipEn_f, &dbg_ipe_oam_info, &oam_mipEn);
    GetDbgIpeOamInfo(A, tempRxOamType_f, &dbg_ipe_oam_info, &oam_tempRxOamType);
    GetDbgIpeOamInfo(A, valid_f, &dbg_ipe_oam_info, &oam_valid);

    key_type[0] = lkp_ingressOamKeyType0;
    key_type[1] = lkp_ingressOamKeyType1;
    key_type[2] = lkp_ingressOamKeyType2;

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    if (lkp_ingressOamKeyType0 || lkp_ingressOamKeyType1 || lkp_ingressOamKeyType2)
    {

        CTC_DKIT_PRINT("OAM Process:\n");

        for (i = 0; i < 3; i++)
        {
            if (key_type[i])
            {
                CTC_DKIT_PATH_PRINT("%s%d %s\n", "hash",i,lookup_result[key_type[i] - EGRESSXCOAMHASHTYPE_ETH]);
                if (hash_valid[i])
                {
                    if (confict[i])
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_CONFLICT);
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", key_index[i]);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name[key_type[i] - EGRESSXCOAMHASHTYPE_ETH]);
                    }
                    else if (hash_resultValid[i])
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", key_index[i]);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name[key_type[i] - EGRESSXCOAMHASHTYPE_ETH]);
                    }
                    else
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
                    }
                }
            }
        }
    }

    if (oam_valid)
    {
        if (hash0_resultValid || hash1_resultValid || hash2_resultValid)
        {
            if (oam_mepEn)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "hit MEP", "YES");
                CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "mep index", oam_mepIndex);
                CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Master chip id", oam_oamDestChipId);
            }
            if (oam_mipEn)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "hit MIP", "YES");
                CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Master chip id", oam_oamDestChipId);
            }
            if (oam_tempRxOamType)
            {
                char* oam_type[RXOAMTYPE_TRILLBFD] =
                {
                    "ETHER OAM", "IP BFD", "PBT OAM", "PBBBSI", "PBBBV",
                    "MPLS OAM", "MPLS BFD", "ACH OAM", "RSV", "TRILL BFD"
                };
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "OAM type", oam_type[oam_tempRxOamType - 1]);
            }

            if (oam_mplsLmValid || oam_etherLmValid)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s 0x%x\n", "read LM from", "DsOamLmStats", oam_lmStatsIndex);
            }
            if (oam_lmStatsEn0)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s 0x%x\n", "write LM to", "DsOamLmStats", oam_lmStatsPtr0);
            }
            if (oam_lmStatsEn1)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s 0x%x\n", "write LM to", "DsOamLmStats", oam_lmStatsPtr1);
            }
            if (oam_lmStatsEn2)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s 0x%x\n", "write LM to", "DsOamLmStats", oam_lmStatsPtr2);
            }
        }
    }

Discard:

    return CLI_SUCCESS;
}

STATIC int32
 _ctc_usw_dkit_captured_path_ipe_destination(ctc_dkit_captured_path_info_t* p_info)
 {
    uint8 slice = p_info->slice_ipe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;
    uint8 is_mcast = 0;
    uint8 is_link_agg = 0;
    uint16 dest_port = 0;
    uint32 rx_oam_type = 0;
    uint8 dest_chip = 0;
    uint16 gport = 0;
    uint32 dsNextHopInternalBase = 0;
    uint8 fwd_to_cpu = 0;
    /*DbgIpeEcmpProcInfo*/
    uint32 ecmp_ecmpGroupId = 0;
    uint32 ecmp_ecmpEn = 0;
    uint32 ecmp_valid = 0;
    DbgIpeEcmpProcInfo_m dbg_ipe_ecmp_proc_info;
    /*DbgIpeForwardingInfo*/

    uint32 fwd_discardType = 0;
    uint32 fwd_exceptionEn = 0;
    uint32 fwd_exceptionIndex = 0;
    uint32 fwd_exceptionSubIndex = 0;
    uint32 fwd_fatalExceptionValid = 0;
    uint32 fwd_fatalException = 0;
    uint32 fwd_apsSelectValid0 = 0;
    uint32 fwd_apsSelectGroupId0 = 0;
    uint32 fwd_apsSelectValid1 = 0;
    uint32 fwd_apsSelectGroupId1 = 0;
    uint32 fwd_discard = 0;
    uint32 fwd_destMap = 0;
    uint32 fwd_apsBridgeEn0 = 0;
    uint32 fwd_apsBridgeEn1 = 0;
    uint32 fwd_apsGroup0 = 0;
    uint32 fwd_apsGroup1 = 0;
    uint32 fwd_nextHopExt = 0;
    uint32 fwd_nextHopIndex = 0;
    uint32 fwd_valid = 0;
    uint32 fwd_acl_en = 0;
    uint32 fwd_acl_fwd_valid = 0;
    uint32 fwd_bypass_lag = 0;
    uint32 fwd_bypass_policer = 0;
    uint32 fwd_bypass_stormctl = 0;
    uint32 fwd_learn = 0;
    uint32 fwd_ptr_valid1 = 0;
    uint32 fwd_ptr1 = 0;
    uint32 fwd_ptr_valid0 = 0;
    uint32 fwd_ptr0 = 0;
    uint32 fwd_nexthop_valid0 = 0;
    uint32 fwd_nexthop_valid1 = 0;
    uint32 fwd_ipfix_en = 0;
    uint32 fwd_share_type = 0;

    DbgIpeFwdProcessInfo_m dbg_ipe_forwarding_info;
    DbgIpeAclProcInfo0_m   dbg_ipe_acl_proc_info;

    /*DbgIpeEcmpProcInfo*/
    sal_memset(&dbg_ipe_ecmp_proc_info, 0, sizeof(dbg_ipe_ecmp_proc_info));
    cmd = DRV_IOR(DbgIpeEcmpProcInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_ipe_ecmp_proc_info);
    GetDbgIpeEcmpProcInfo(A, ecmpGroupId_f, &dbg_ipe_ecmp_proc_info, &ecmp_ecmpGroupId);
    GetDbgIpeEcmpProcInfo(A, ecmpEn_f, &dbg_ipe_ecmp_proc_info, &ecmp_ecmpEn);
    GetDbgIpeEcmpProcInfo(A, valid_f, &dbg_ipe_ecmp_proc_info, &ecmp_valid);
    /*DbgIpeForwardingInfo*/
    sal_memset(&dbg_ipe_forwarding_info, 0, sizeof(dbg_ipe_forwarding_info));
    cmd = DRV_IOR(DbgIpeFwdProcessInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_ipe_forwarding_info);

    sal_memset(&dbg_ipe_acl_proc_info, 0, sizeof(dbg_ipe_acl_proc_info));
    cmd = DRV_IOR(DbgIpeAclProcInfo0_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_ipe_acl_proc_info);

    GetDbgIpeFwdProcessInfo(A, aclDsFwdPtrValid_f, &dbg_ipe_forwarding_info, &fwd_acl_fwd_valid);
    GetDbgIpeFwdProcessInfo(A, aclEnBitMap_f, &dbg_ipe_forwarding_info, &fwd_acl_en);
    GetDbgIpeFwdProcessInfo(A, bypassAccessLagEngine_f,  &dbg_ipe_forwarding_info, &fwd_bypass_lag);
    GetDbgIpeFwdProcessInfo(A, bypassPolicingOp_f,  &dbg_ipe_forwarding_info, &fwd_bypass_policer);
    GetDbgIpeFwdProcessInfo(A, bypassStormCtlOp_f,  &dbg_ipe_forwarding_info, &fwd_bypass_stormctl);
    GetDbgIpeFwdProcessInfo(A, destMap_f,  &dbg_ipe_forwarding_info, &fwd_destMap);
    GetDbgIpeFwdProcessInfo(A, discardType_f,  &dbg_ipe_forwarding_info, &fwd_discardType);
    GetDbgIpeFwdProcessInfo(A, discard_f,  &dbg_ipe_forwarding_info, &fwd_discard);
    GetDbgIpeFwdProcessInfo(A, doLearningOperation_f,  &dbg_ipe_forwarding_info, &fwd_learn);
    GetDbgIpeFwdProcessInfo(A, secondDsFwdPtrValid_f,  &dbg_ipe_forwarding_info, &fwd_ptr_valid1);
    GetDbgIpeFwdProcessInfo(A, secondDsFwdPtr_f,  &dbg_ipe_forwarding_info, &fwd_ptr1);
    GetDbgIpeFwdProcessInfo(A, secondNextHopPtrValid_f,  &dbg_ipe_forwarding_info, &fwd_nexthop_valid1);
    if (DRV_IS_DUET2(lchip))
    {
        GetDbgIpeFwdProcessInfo(A, apsBridgeIndex0_f, &dbg_ipe_forwarding_info, &fwd_apsGroup0);
        GetDbgIpeFwdProcessInfo(A, apsBridgeIndex1_f, &dbg_ipe_forwarding_info, &fwd_apsGroup1);
        GetDbgIpeFwdProcessInfo(A, apsBridgeValid0_f, &dbg_ipe_forwarding_info, &fwd_apsBridgeEn0);
        GetDbgIpeFwdProcessInfo(A, apsBridgeValid1_f,  &dbg_ipe_forwarding_info, &fwd_apsBridgeEn1);
        GetDbgIpeFwdProcessInfo(A, apsSelectGroupId0_f, &dbg_ipe_forwarding_info, &fwd_apsSelectGroupId0);
        GetDbgIpeFwdProcessInfo(A, apsSelectGroupId1_f,  &dbg_ipe_forwarding_info, &fwd_apsSelectGroupId1);
        GetDbgIpeFwdProcessInfo(A, apsSelectValid0_f,  &dbg_ipe_forwarding_info, &fwd_apsSelectValid0);
        GetDbgIpeFwdProcessInfo(A, apsSelectValid1_f,  &dbg_ipe_forwarding_info, &fwd_apsSelectValid1);
        GetDbgIpeFwdProcessInfo(A, firstDsFwdPtrValid_f,  &dbg_ipe_forwarding_info, &fwd_ptr_valid0);
        GetDbgIpeFwdProcessInfo(A, firstDsFwdPtr_f,  &dbg_ipe_forwarding_info, &fwd_ptr0);
        GetDbgIpeFwdProcessInfo(A, firstNextHopPtrValid_f,  &dbg_ipe_forwarding_info, &fwd_nexthop_valid0);
    }
    else
    {
        GetDbgIpeAclProcInfo0(A, apsBridgeIndex0_f, &dbg_ipe_acl_proc_info, &fwd_apsGroup0);
        GetDbgIpeAclProcInfo0(A, apsBridgeIndex1_f, &dbg_ipe_acl_proc_info, &fwd_apsGroup1);
        GetDbgIpeAclProcInfo0(A, apsBridgeValid0_f, &dbg_ipe_acl_proc_info, &fwd_apsBridgeEn0);
        GetDbgIpeAclProcInfo0(A, apsBridgeValid1_f,  &dbg_ipe_acl_proc_info, &fwd_apsBridgeEn1);
        GetDbgIpeAclProcInfo0(A, apsSelectGroupId0_f, &dbg_ipe_acl_proc_info, &fwd_apsSelectGroupId0);
        GetDbgIpeAclProcInfo0(A, apsSelectGroupId1_f,  &dbg_ipe_acl_proc_info, &fwd_apsSelectGroupId1);
        GetDbgIpeAclProcInfo0(A, apsSelectValid0_f,  &dbg_ipe_acl_proc_info, &fwd_apsSelectValid0);
        GetDbgIpeAclProcInfo0(A, apsSelectValid1_f,  &dbg_ipe_acl_proc_info, &fwd_apsSelectValid1);
        GetDbgIpeAclProcInfo0(A, firstDsFwdPtrValid_f,  &dbg_ipe_acl_proc_info, &fwd_ptr_valid0);
        GetDbgIpeAclProcInfo0(A, firstDsFwdPtr_f,  &dbg_ipe_acl_proc_info, &fwd_ptr0);
        GetDbgIpeAclProcInfo0(A, firstNextHopPtrValid_f,  &dbg_ipe_acl_proc_info, &fwd_nexthop_valid0);
    }

#if 0
/*temply not support cid*/
    GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairHashAdIndex_f,  &dbg_ipe_forwarding_info, &fwd_valid);
    GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairHashEntryOffset_f,  &dbg_ipe_forwarding_info, &fwd_valid);
    GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairHashHit_f,  &dbg_ipe_forwarding_info, &fwd_valid);
    GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairHashIsLeft_f,  &dbg_ipe_forwarding_info, &fwd_valid);
    GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairHashKeyIndex_f,  &dbg_ipe_forwarding_info, &fwd_valid);
    GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairTcamHitIndex_f,  &dbg_ipe_forwarding_info, &fwd_valid);
    GetDbgIpeFwdProcessInfo(A, gCidLkp_cidPairTcamResultValid_f,  &dbg_ipe_forwarding_info, &fwd_valid);
    GetDbgIpeFwdProcessInfo(A, gCidLkp_destCategoryIdClassfied_f,  &dbg_ipe_forwarding_info, &fwd_valid);
    GetDbgIpeFwdProcessInfo(A, gCidLkp_destCategoryId_f,  &dbg_ipe_forwarding_info, &fwd_valid);
    GetDbgIpeFwdProcessInfo(A, gCidLkp_srcCategoryIdClassfied_f,  &dbg_ipe_forwarding_info, &fwd_valid);
    GetDbgIpeFwdProcessInfo(A, gCidLkp_srcCategoryId_f,  &dbg_ipe_forwarding_info, &fwd_valid);
#endif

     /*-GetDbgIpeFwdProcessInfo(A, ingfwdType_f,  &dbg_ipe_forwarding_info, &fwd_valid);*/
     /*-GetDbgIpeFwdProcessInfo(A, ingressHeaderValid_f,  &dbg_ipe_forwarding_info, &fwd_valid);*/
    GetDbgIpeFwdProcessInfo(A, ipfixFlowLookupEn_f,  &dbg_ipe_forwarding_info, &fwd_ipfix_en);
     /*-GetDbgIpeFwdProcessInfo(A, macKnown_f,  &dbg_ipe_forwarding_info, &fwd_valid);*/
     /*-GetDbgIpeFwdProcessInfo(A, mcastMac_f,  &dbg_ipe_forwarding_info, &fwd_valid);*/
     /*-GetDbgIpeFwdProcessInfo(A, microFlowLookupEn_f,  &dbg_ipe_forwarding_info, &fwd_valid);*/
     /*-GetDbgIpeFwdProcessInfo(A, mutationTableId_f,  &dbg_ipe_forwarding_info, &fwd_valid);*/
     /*-GetDbgIpeFwdProcessInfo(A, muxDestinationValid_f,  &dbg_ipe_forwarding_info, &fwd_valid);*/
    GetDbgIpeFwdProcessInfo(A, nextHopExt_f,  &dbg_ipe_forwarding_info, &fwd_nextHopExt);
    GetDbgIpeFwdProcessInfo(A, nextHopPtr_f,  &dbg_ipe_forwarding_info, &fwd_nextHopIndex);
     /*-GetDbgIpeFwdProcessInfo(A, policyProfId_f,  &dbg_ipe_forwarding_info, &fwd_valid);*/
    GetDbgIpeFwdProcessInfo(A, shareType_f,  &dbg_ipe_forwarding_info, &fwd_share_type);
    /*- GetDbgIpeFwdProcessInfo(A, skipTimestampOperation_f,  &dbg_ipe_forwarding_info, &fwd_valid);*/
    /*-  GetDbgIpeFwdProcessInfo(A, srcDscp_f,  &dbg_ipe_forwarding_info, &fwd_valid);*/
    /*-  GetDbgIpeFwdProcessInfo(A, stagCos_f,  &dbg_ipe_forwarding_info, &fwd_valid);*/
    GetDbgIpeFwdProcessInfo(A, valid_f,  &dbg_ipe_forwarding_info, &fwd_valid);

    cmd = DRV_IOR(DbgIpeOamInfo_t, DbgIpeOamInfo_tempRxOamType_f);
    DRV_IOCTL(lchip, slice, cmd, &rx_oam_type);

     if (fwd_valid)
     {
         CTC_DKIT_PRINT("Destination Process:\n");
         if (ecmp_valid && ecmp_ecmpEn && ecmp_ecmpGroupId)
         {
             CTC_DKIT_PATH_PRINT("%-15s:%d\n", "do ecmp, group", ecmp_ecmpGroupId);
         }
         if (fwd_apsBridgeEn0)
         {
             CTC_DKIT_PATH_PRINT("%-15s:%d\n", "do APS, group", fwd_apsGroup0);
             if (fwd_apsBridgeEn1)
             {
                 CTC_DKIT_PRINT("2nd level APS Process:\n");
                 CTC_DKIT_PATH_PRINT("%-15s:%d\n", "do APS, group", fwd_apsGroup1);
             }
         }
         else
         {
             if (fwd_apsSelectValid0)
             {
                 CTC_DKIT_PATH_PRINT("%-15s:%s\n", "APS selector0", "YES");
                 CTC_DKIT_PATH_PRINT("%-15s:%d\n", "APS group", fwd_apsSelectGroupId0);
             }
             if (fwd_apsSelectValid1)
             {
                 CTC_DKIT_PATH_PRINT("%-15s:%s\n", "APS selector2", "YES");
                 CTC_DKIT_PATH_PRINT("%-15s:%d\n", "APS group", fwd_apsSelectGroupId1);
             }
         }
         if (!fwd_discard)
         {
             if (rx_oam_type)
             {
                 CTC_DKIT_PATH_PRINT("%-15s:%s\n", "destination", "OAM engine");
                 _ctc_usw_dkit_captured_path_oam_rx(p_info);
             }
             else
             {
                 is_mcast = IS_BIT_SET(fwd_destMap, 18);
                 fwd_to_cpu = IS_BIT_SET(fwd_destMap, 16);
                 dest_port = fwd_destMap&0x1FF;
                 if (0x3FFFF == fwd_nextHopIndex)
                 {
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Drop!!!", "next hop is invalid, nextHopPtr=0x3FFFF");
                 }
                 else if (is_mcast)
                 {
                     CTC_DKIT_PATH_PRINT("%-15s:%d\n", "do mcast, group", fwd_destMap&0xFFFF);
                 }
                 else
                 {
                     dest_chip = (fwd_destMap >> 9)&0x7F;
                     is_link_agg = (dest_chip == 0x1F);
                     if (fwd_to_cpu)
                     {
                         CTC_DKIT_PATH_PRINT("%-15s:%s\n", "destination", "CPU");
                     }
                     else
                     {
                         if (is_link_agg)
                         {
                             CTC_DKIT_PATH_PRINT("%-15s:%d\n", "do linkagg, group", fwd_destMap&CTC_DKIT_DRV_LINKAGGID_MASK);
                         }
                         else
                         {
                             gport = CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(dest_chip, dest_port&0x1FF);
                             CTC_DKIT_PATH_PRINT("%-15s:0x%04x\n", "dest gport", gport);
                         }

                         cmd = DRV_IOR(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_dsNextHopInternalBase_f);
                         DRV_IOCTL(lchip, 0, cmd, &dsNextHopInternalBase);
                         CTC_DKIT_PATH_PRINT("%-15s\n", "nexthop info--->");
                         CTC_DKIT_PATH_PRINT("%-15s:%s\n", "is 8Wnhp", fwd_nextHopExt? "YES" : "NO");
                         if (((fwd_nextHopIndex >> 4)&0x3FFF) != dsNextHopInternalBase)
                         {
                             CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "nexthop index", fwd_nextHopIndex);
                             CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", fwd_nextHopExt? "DsNextHop8W" : "DsNextHop4W");
                         }
                         else
                         {
                             CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "nexthop index", (fwd_nextHopIndex&0xF) >> 1);
                             CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", "EpeNextHopInternal");
                         }
                     }
                 }
             }
         }
         else
         {
             _ctc_usw_dkit_captured_path_discard_process(p_info, fwd_discard, fwd_discardType, CTC_DKIT_IPE);
             if ((fwd_discard) && (IPE_FATAL_EXCEP_DIS == (fwd_discardType+IPE_FEATURE_START)))
             {
                 if (fwd_fatalExceptionValid)
                 {
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Fatal exception",
                                         ctc_usw_dkit_get_sub_reason_desc(fwd_fatalException + CTC_DKIT_SUB_IPE_FATAL_UC_IP_HDR_ERROR_OR_MARTION_ADDR));
                 }
             }
         }
         _ctc_usw_dkit_captured_path_exception_process(p_info, fwd_exceptionEn, fwd_exceptionIndex, fwd_exceptionSubIndex);
     }

     return CLI_SUCCESS;
 }
STATIC int32
_ctc_usw_dkit_captured_path_ipfix(ctc_dkit_captured_path_info_t* p_info, uint8 dir)
{
    uint32 cmd = 0;
    DbgIpfixAccIngInfo_m ipfix_info;
    DbgIpfixAccInfo0_m  ipfixAccInfo0;
    uint8 lchip = p_info->lchip;
    uint32 byPassIpfixProcess = 0;
    uint32 denyIpfixInsertOperation = 0;
    uint32 destinationInfo = 0;
    uint32 destinationType = 0;
    uint32 disableIpfixAndMfp = 0;
    uint32 flowFieldSel = 0;
    uint32 hashConflict = 0;
    uint32 hashKeyType = 0;
    uint32 ipfixCfgProfileId = 0;
    uint32 ipfixEn = 0;
    uint32 isAddOperation = 0;
    uint32 isSamplingPkt = 0;
    uint32 isUpdateOperation = 0;
    uint32 keyIndex = 0;
    uint32 mfpEn = 0;
    uint32 microflowPolicingProfId = 0;
    uint32 resultValid = 0;
    uint32 samplingEn = 0;
    uint32 valid = 0;

    if (DRV_IS_DUET2(lchip))
    {
        if (dir == 0)
        {
            cmd = DRV_IOR(DbgIpfixAccIngInfo_t, DRV_ENTRY_FLAG);
        }
        else
        {
            cmd = DRV_IOR(DbgIpfixAccEgrInfo_t, DRV_ENTRY_FLAG);
        }

        DRV_IOCTL(lchip, 0, cmd, &ipfix_info);

        GetDbgIpfixAccIngInfo(A, gIngr_byPassIpfixProcess_f, &ipfix_info, &byPassIpfixProcess);
        GetDbgIpfixAccIngInfo(A, gIngr_denyIpfixInsertOperation_f, &ipfix_info, &denyIpfixInsertOperation);
        GetDbgIpfixAccIngInfo(A, gIngr_destinationInfo_f, &ipfix_info, &destinationInfo);
        GetDbgIpfixAccIngInfo(A, gIngr_destinationType_f, &ipfix_info, &destinationType);
        GetDbgIpfixAccIngInfo(A, gIngr_disableIpfixAndMfp_f, &ipfix_info, &disableIpfixAndMfp);
        GetDbgIpfixAccIngInfo(A, gIngr_flowFieldSel_f, &ipfix_info, &flowFieldSel);
        GetDbgIpfixAccIngInfo(A, gIngr_hashConflict_f, &ipfix_info, &hashConflict);
        GetDbgIpfixAccIngInfo(A, gIngr_hashKeyType_f, &ipfix_info, &hashKeyType);
        GetDbgIpfixAccIngInfo(A, gIngr_ipfixCfgProfileId_f, &ipfix_info, &ipfixCfgProfileId);
        GetDbgIpfixAccIngInfo(A, gIngr_ipfixEn_f, &ipfix_info, &ipfixEn);
        GetDbgIpfixAccIngInfo(A, gIngr_isAddOperation_f, &ipfix_info, &isAddOperation);
        GetDbgIpfixAccIngInfo(A, gIngr_isSamplingPkt_f, &ipfix_info, &isSamplingPkt);
        GetDbgIpfixAccIngInfo(A, gIngr_isUpdateOperation_f, &ipfix_info, &isUpdateOperation);
        GetDbgIpfixAccIngInfo(A, gIngr_keyIndex_f, &ipfix_info, &keyIndex);
        GetDbgIpfixAccIngInfo(A, gIngr_mfpEn_f, &ipfix_info, &mfpEn);
        GetDbgIpfixAccIngInfo(A, gIngr_microflowPolicingProfId_f, &ipfix_info, &microflowPolicingProfId);
        GetDbgIpfixAccIngInfo(A, gIngr_resultValid_f, &ipfix_info, &resultValid);
        GetDbgIpfixAccIngInfo(A, gIngr_samplingEn_f, &ipfix_info, &samplingEn);
        GetDbgIpfixAccIngInfo(A, gIngr_valid_f, &ipfix_info, &valid);
    }
    else
    {
        if (dir == 0)
        {
            cmd = DRV_IOR(DbgIpfixAccInfo0_t, DRV_ENTRY_FLAG);
        }
        else
        {
            cmd = DRV_IOR(DbgIpfixAccInfo1_t, DRV_ENTRY_FLAG);
        }

        DRV_IOCTL(lchip, 0, cmd, &ipfixAccInfo0);
        GetDbgIpfixAccInfo0(A, byPassIpfixProcess_f, &ipfixAccInfo0, &byPassIpfixProcess);
        GetDbgIpfixAccInfo0(A, denyIpfixInsertOperation_f, &ipfixAccInfo0, &denyIpfixInsertOperation);
        GetDbgIpfixAccInfo0(A, destinationInfo_f, &ipfixAccInfo0, &destinationInfo);
        GetDbgIpfixAccInfo0(A, destinationType_f, &ipfixAccInfo0,&destinationType);
        GetDbgIpfixAccInfo0(A, flowFieldSel_f, &ipfixAccInfo0, &flowFieldSel);
        GetDbgIpfixAccInfo0(A, hashConflict_f, &ipfixAccInfo0, &hashConflict);
        GetDbgIpfixAccInfo0(A, hashKeyType_f, &ipfixAccInfo0, &hashKeyType);
        GetDbgIpfixAccInfo0(A, ipfixCfgProfileId_f, &ipfixAccInfo0, &ipfixCfgProfileId);
        GetDbgIpfixAccInfo0(A, ipfixEn_f, &ipfixAccInfo0, &ipfixEn);
        GetDbgIpfixAccInfo0(A, isAddOperation_f, &ipfixAccInfo0, &isAddOperation);
        GetDbgIpfixAccInfo0(A, isSamplingPkt_f, &ipfixAccInfo0, &isSamplingPkt);
        GetDbgIpfixAccInfo0(A, isUpdateOperation_f, &ipfixAccInfo0, &isUpdateOperation);
        GetDbgIpfixAccInfo0(A, keyIndex_f, &ipfixAccInfo0, &keyIndex);
        GetDbgIpfixAccInfo0(A, mfpEn_f, &ipfixAccInfo0, &mfpEn);
        GetDbgIpfixAccInfo0(A, microflowPolicingProfId_f, &ipfixAccInfo0, &microflowPolicingProfId);
        GetDbgIpfixAccInfo0(A, resultValid_f, &ipfixAccInfo0, &resultValid);
        GetDbgIpfixAccInfo0(A, samplingEn_f, &ipfixAccInfo0, &samplingEn);
        GetDbgIpfixAccInfo0(A, valid_f, &ipfixAccInfo0, &valid);
    }

    if (valid)
    {
        if(dir == 0)
        {
            CTC_DKIT_PRINT("Ipe ipfix process:\n");
        }
        else
        {
            CTC_DKIT_PRINT("Epe ipfix process:\n");
        }
        CTC_DKIT_PATH_PRINT("ipfix enable:    %s\n", ipfixEn?"enable":"disable");
        CTC_DKIT_PATH_PRINT("ipfix configure profile Id:    %d\n",ipfixCfgProfileId);
        CTC_DKIT_PATH_PRINT("hash key type:    %d\n", hashKeyType);
        CTC_DKIT_PATH_PRINT("Hash field select Id:    %d\n", flowFieldSel);

        if(resultValid)
        {
            CTC_DKIT_PATH_PRINT("Hash lookup result:\n");
            CTC_DKIT_PATH_PRINT("    Key index:    %d\n",keyIndex);
            CTC_DKIT_PATH_PRINT("    Is conflict packet: %s\n",hashConflict?"yes":"no");
        }

        if(samplingEn)
        {
            CTC_DKIT_PATH_PRINT("Enable sample and %s\n", isSamplingPkt?"is sample packet":"is not sample packet");
        }

        CTC_DKIT_PATH_PRINT("Is add operation:    %s\n", isAddOperation ? "yes" :"no");
        CTC_DKIT_PATH_PRINT("Is update operation:    %s\n", isUpdateOperation ? "yes":"no");
        CTC_DKIT_PATH_PRINT("Bypass ipfix process:    %s\n", byPassIpfixProcess ? "yes":"no");
        CTC_DKIT_PATH_PRINT("deny ipfix learning:    %s\n", denyIpfixInsertOperation ? "yes":"no");
        if (DRV_IS_DUET2(lchip))
        {
            CTC_DKIT_PATH_PRINT("disable ipfix:    %s\n", disableIpfixAndMfp ? "yes":"no");
        }
        if(mfpEn)
        {
            CTC_DKIT_PATH_PRINT("Enable ipfix with rate!!!\n");
            CTC_DKIT_PATH_PRINT("Mfp policing profile Id:    %d\n", microflowPolicingProfId);
        }

        CTC_DKIT_PATH_PRINT("Destination Info:    %d\n", destinationInfo);
        CTC_DKIT_PATH_PRINT("Destination Type:    %d\n", destinationType);
    }

    return CLI_SUCCESS;
}
STATIC int32
_ctc_usw_dkit_captured_path_ipe_ipfix(ctc_dkit_captured_path_info_t* p_info)
{
    _ctc_usw_dkit_captured_path_ipfix(p_info, 0);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_ipe_misc(ctc_dkit_captured_path_info_t* p_info)
{
    uint8  slice = p_info->slice_ipe;
    uint8  lchip = p_info->lchip;
    uint32 cmd = 0;

    char*  str_color[] = {"None", "Red", "Yellow", "Green"};
    char*  str_stmctl_type[] = {"Unknow-Ucast", "Broadcast", "Unknow-Mcast", " ",   \
                                    "Know-Ucast", "Broadcast", "Know-Mcast", " "};

    uint32 policerColor              = 0;
    uint32 policerMuxLenType         = 0;
    uint32 policer0SplitModeValid    = 0;
    uint32 policer1SplitModeValid    = 0;
    uint32 policerLayer3Offset       = 0;
    uint32 policerLength             = 0;
    uint32 policerPhbEn0             = 0;
    uint32 policerPhbEn1             = 0;
    uint32 policerPtr0               = 0;
    uint32 policerPtr1               = 0;
    uint32 policerStatsPtr0          = 0;
    uint32 policerStatsPtr1          = 0;
    uint32 policerValid0             = 0;
    uint32 policerValid1             = 0;
    uint32 policyProfId              = 0;
    uint32 portPolicer0En            = 0;
    uint32 portPolicer1En            = 0;
    uint32 vlanPolicer0En            = 0;
    uint32 vlanPolicer1En            = 0;
    uint32 policerInfoValid          = 0;
    DbgIpeFwdPolicingInfo_m dbg_policer_info;
    uint32 stormCtlColor       = 0;
    uint32 stormCtlOffsetSel   = 0;
    uint32 stormCtlDrop        = 0;
    uint32 stormCtlEn          = 0;
    uint32 stormCtlExceptionEn = 0;
    uint32 stormCtlLen         = 0;
    uint32 stormCtlPtr0        = 0;
    uint32 stormCtlPtr1        = 0;
    uint32 stormCtlPtrValid0   = 0;
    uint32 stormCtlPtrValid1   = 0;
    uint32 stormCtlInfoValid   = 0;
    DbgIpeFwdStormCtlInfo_m dbg_storm_ctl_info;
    uint32 coppInfoColor     = 0;
    uint32 coppInfoCoppDrop  = 0;
    uint32 coppInfoCoppLen   = 0;
    uint32 coppInfoCoppPtr   = 0;
    uint32 coppInfoCoppValid = 0;
    uint32 coppInfoValid     = 0;
    DbgIpeFwdCoppInfo_m dbg_copp_info;

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    sal_memset(&dbg_policer_info, 0, sizeof(DbgIpeFwdPolicingInfo_m));
    cmd = DRV_IOR(DbgIpeFwdPolicingInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_storm_ctl_info);
    GetDbgIpeFwdPolicingInfo(A, color_f                     , &dbg_policer_info, &policerColor);
    GetDbgIpeFwdPolicingInfo(A, muxLengthType_f             , &dbg_policer_info, &policerMuxLenType);
    GetDbgIpeFwdPolicingInfo(A, policer0SplitModeValid_f    , &dbg_policer_info, &policer0SplitModeValid);
    GetDbgIpeFwdPolicingInfo(A, policer1SplitModeValid_f    , &dbg_policer_info, &policer1SplitModeValid);
    GetDbgIpeFwdPolicingInfo(A, policerLayer3Offset_f       , &dbg_policer_info, &policerLayer3Offset);
    GetDbgIpeFwdPolicingInfo(A, policerLength_f             , &dbg_policer_info, &policerLength);
    GetDbgIpeFwdPolicingInfo(A, policerPhbEn0_f             , &dbg_policer_info, &policerPhbEn0);
    GetDbgIpeFwdPolicingInfo(A, policerPhbEn1_f             , &dbg_policer_info, &policerPhbEn1);
    GetDbgIpeFwdPolicingInfo(A, policerPtr0_f               , &dbg_policer_info, &policerPtr0);
    GetDbgIpeFwdPolicingInfo(A, policerPtr1_f               , &dbg_policer_info, &policerPtr1);
    GetDbgIpeFwdPolicingInfo(A, policerStatsPtr0_f          , &dbg_policer_info, &policerStatsPtr0);
    GetDbgIpeFwdPolicingInfo(A, policerStatsPtr1_f          , &dbg_policer_info, &policerStatsPtr1);
    GetDbgIpeFwdPolicingInfo(A, policerValid0_f             , &dbg_policer_info, &policerValid0);
    GetDbgIpeFwdPolicingInfo(A, policerValid1_f             , &dbg_policer_info, &policerValid1);
    GetDbgIpeFwdPolicingInfo(A, policyProfId_f              , &dbg_policer_info, &policyProfId);
    GetDbgIpeFwdPolicingInfo(A, portPolicer0En_f            , &dbg_policer_info, &portPolicer0En);
    GetDbgIpeFwdPolicingInfo(A, portPolicer1En_f            , &dbg_policer_info, &portPolicer1En);
    GetDbgIpeFwdPolicingInfo(A, valid_f                     , &dbg_policer_info, &policerInfoValid);
    GetDbgIpeFwdPolicingInfo(A, vlanPolicer0En_f            , &dbg_policer_info, &vlanPolicer0En);
    GetDbgIpeFwdPolicingInfo(A, vlanPolicer1En_f            , &dbg_policer_info, &vlanPolicer1En);

    /*policer*/
    if(policerInfoValid)
    {
        CTC_DKIT_PRINT("Policer Process:\n");
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Policer Color", str_color[policerColor]);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Policer0 SplitEn", policer0SplitModeValid);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Policer1 SplitEn", policer1SplitModeValid);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Policer Length", policerLength);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Policer0 PhbEn", policerPhbEn0);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Policer1 PhbEn", policerPhbEn1);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Policer0 Ptr", policerPtr0);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Policer1 Ptr", policerPtr1);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Policer0 StatsPtr", policerStatsPtr0);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Policer1 StatsPtr", policerStatsPtr1);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Policer0 Valid", policerValid0);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Policer1 Valid", policerValid1);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "PolicyProfId", policyProfId);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Port Policer0En", portPolicer0En);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Port Policer1En", portPolicer1En);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Vlan Policer0En", vlanPolicer0En);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Vlan Policer1En", vlanPolicer1En);

    }

    sal_memset(&dbg_storm_ctl_info, 0, sizeof(DbgIpeFwdStormCtlInfo_m));
    cmd = DRV_IOR(DbgIpeFwdStormCtlInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_storm_ctl_info);
    GetDbgIpeFwdStormCtlInfo(A, color_f               , &dbg_storm_ctl_info, &stormCtlColor);
    GetDbgIpeFwdStormCtlInfo(A, offsetSel_f           , &dbg_storm_ctl_info, &stormCtlOffsetSel);
    GetDbgIpeFwdStormCtlInfo(A, stormCtlDrop_f        , &dbg_storm_ctl_info, &stormCtlDrop);
    GetDbgIpeFwdStormCtlInfo(A, stormCtlEn_f          , &dbg_storm_ctl_info, &stormCtlEn);
    GetDbgIpeFwdStormCtlInfo(A, stormCtlExceptionEn_f , &dbg_storm_ctl_info, &stormCtlExceptionEn);
    GetDbgIpeFwdStormCtlInfo(A, stormCtlLen_f         , &dbg_storm_ctl_info, &stormCtlLen);
    GetDbgIpeFwdStormCtlInfo(A, stormCtlPtr0_f        , &dbg_storm_ctl_info, &stormCtlPtr0);
    GetDbgIpeFwdStormCtlInfo(A, stormCtlPtr1_f        , &dbg_storm_ctl_info, &stormCtlPtr1);
    GetDbgIpeFwdStormCtlInfo(A, stormCtlPtrValid0_f   , &dbg_storm_ctl_info, &stormCtlPtrValid0);
    GetDbgIpeFwdStormCtlInfo(A, stormCtlPtrValid1_f   , &dbg_storm_ctl_info, &stormCtlPtrValid1);
    GetDbgIpeFwdStormCtlInfo(A, valid_f               , &dbg_storm_ctl_info, &stormCtlInfoValid);
    /*stormctl*/
    if(stormCtlEn && stormCtlInfoValid)
    {
        CTC_DKIT_PRINT("StormCtl Process:\n");
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "StormCtl Type", str_stmctl_type[stormCtlOffsetSel]);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "StormCtl Ptr0", stormCtlPtr0);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Ptr0 valid", stormCtlPtrValid0);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "StormCtl Ptr1", stormCtlPtr1);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Ptr1 valid", stormCtlPtrValid1);
        if(stormCtlDrop)
        {
            CTC_DKIT_PATH_PRINT("StormCtl Drop!\n");
        }
        if(stormCtlExceptionEn)
        {
            CTC_DKIT_PATH_PRINT("StormCtl Exception!\n");
        }
    }

    sal_memset(&dbg_copp_info, 0, sizeof(DbgIpeFwdCoppInfo_m));
    cmd = DRV_IOR(DbgIpeFwdCoppInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_copp_info);
    GetDbgIpeFwdCoppInfo(A, color_f,     &dbg_copp_info, &coppInfoColor);
    GetDbgIpeFwdCoppInfo(A, coppDrop_f,  &dbg_copp_info, &coppInfoCoppDrop);
    GetDbgIpeFwdCoppInfo(A, coppLen_f,   &dbg_copp_info, &coppInfoCoppLen);
    GetDbgIpeFwdCoppInfo(A, coppPtr_f,   &dbg_copp_info, &coppInfoCoppPtr);
    GetDbgIpeFwdCoppInfo(A, coppValid_f, &dbg_copp_info, &coppInfoCoppValid);
    GetDbgIpeFwdCoppInfo(A, valid_f,     &dbg_copp_info, &coppInfoValid);
    /*copp*/
    if(coppInfoValid && coppInfoCoppValid)
    {
        CTC_DKIT_PRINT("Copp Process:\n");
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Copp Ptr", coppInfoCoppPtr);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Copp Length", coppInfoCoppLen);
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Copp Color", str_color[coppInfoColor]);
        if(coppInfoCoppDrop)
        {
            CTC_DKIT_PATH_PRINT("Copp Drop!\n");
        }
    }

Discard:
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_ipe(ctc_dkit_captured_path_info_t* p_info)
{

    CTC_DKIT_PRINT("\nINGRESS PROCESS  \n-----------------------------------\n");

    /*1. Parser*/
    if (!p_info->discard)
    {
        _ctc_usw_dkit_captured_path_ipe_parser(p_info);
    }
    /*2. SCL*/
    if ((!p_info->discard)||(p_info->rx_oam))
    {
        _ctc_usw_dkit_captured_path_ipe_scl(p_info);
    }
    /*3. Interface*/
    if ((!p_info->discard)||(p_info->rx_oam))
    {
        _ctc_usw_dkit_captured_path_ipe_interface(p_info);
    }
    /*4. Tunnel*/
    if ((!p_info->discard)||(p_info->rx_oam))
    {
        _ctc_usw_dkit_captured_path_ipe_tunnel(p_info);
    }
    /*5. Flow Hash*/
    if ((!p_info->discard)||(p_info->rx_oam))
    {
        _ctc_usw_dkit_captured_path_ipe_flow_hash(p_info);
    }
    /*6. Forward*/
    if ((!p_info->discard)||(p_info->rx_oam))
    {
        _ctc_usw_dkit_captured_path_ipe_forward(p_info);
    }
    /*7. OAM*/
    if ((!p_info->discard)||(p_info->rx_oam))
    {
        _ctc_usw_dkit_captured_path_ipe_oam(p_info);
    }
    /*8. Destination*/
    if ((!p_info->discard)||(p_info->rx_oam))
    {
        _ctc_usw_dkit_captured_path_ipe_destination(p_info);
    }
    /*9. Cid Pair*/
    if ((!p_info->discard)||(p_info->rx_oam))
    {
        _ctc_usw_dkit_capture_path_ipe_cid_pair(p_info);
    }
    /*10. ACL*/
    _ctc_usw_dkit_captured_path_ipe_acl(p_info);
    /*11. Ipfix*/
    _ctc_usw_dkit_captured_path_ipe_ipfix(p_info);
    /*12. policer/stormctl/copp...*/
    if ((!p_info->discard)||(p_info->rx_oam))
    {
        _ctc_usw_dkit_captured_path_ipe_misc(p_info);
    }

    if ((p_info->discard) && (!p_info->exception)&&(!p_info->rx_oam))/*check exception when discard but no exception*/
    {
        _ctc_usw_dkit_captured_path_ipe_exception_check(p_info);
    }

    if (p_info->discard && (0 == p_info->detail))
    {
        CTC_DKIT_PRINT("Packet Drop at IPE:\n");
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "reason", ctc_usw_dkit_get_reason_desc(p_info->discard_type + IPE_FEATURE_START));
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_bsr_mcast(ctc_dkit_captured_path_info_t* p_info)
{
    uint32 cmd = 0;
    uint32 ret = CLI_SUCCESS;
    uint32 local_chip = 0;
    uint32 dest_map = 0;
    uint32 groupIdShiftBit = 0;
    uint32 remaining_rcd = 0xFFF;
    uint32 end_replication = 0;
    uint32 dsMetEntryBase = 0;
    uint32 metEntryPtr =0;
    uint32 dsNextHopInternalBase = 0;
    uint8 lchip = p_info->lchip;
    /*DbgFwdMetFifoInfo*/
    uint32 met_valid = 0;
    uint32 met_enqueueDiscard = 0;
    uint32 exce_vec = 0;
    uint32 execMet = 0;
    uint32 dest_mcast_port = 0;
    uint32 valid_1 = 0;
    /*DsMetEntry*/
    uint32 currentMetEntryType = 0;
    uint32 nextMetEntryPtr = 0;
    uint32 endLocalRep = 0;
    uint32  isLinkAggregation = 0;
    uint32  remoteChip = 0;
    uint32  replicationCtl = 0;
    uint32  ucastId = 0;
    uint32   nexthopExt = 0;
    uint32  nexthopPtr = 0;
    uint32 mcastMode = 0;
    uint32 apsBridgeEn = 0;
    uint8 port_bitmap[8] = {0};
    uint8 port_bitmap_high[9] = {0};
    uint8 port_bitmap_low[8] = {0};

    DsMetEntry6W_m ds_met_entry6_w;
    DsMetEntry6W_m ds_met_entry;
    DbgFwdMetFifoInfo1_m dbg_fwd_met_fifo_info1;
    DbgFwdMetFifoInfo2_m dbg_fwd_met_fifo_info2;
    DbgFwdBufStoreInfo_m dbg_fwd_buf_store_info;

    sal_memset(&ds_met_entry6_w, 0, sizeof(ds_met_entry6_w));
    sal_memset(&ds_met_entry, 0, sizeof(ds_met_entry));

    /*DbgFwdMetFifoInfo1*/
    sal_memset(&dbg_fwd_met_fifo_info1, 0, sizeof(dbg_fwd_met_fifo_info1));
    cmd = DRV_IOR(DbgFwdMetFifoInfo1_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_fwd_met_fifo_info1);
    GetDbgFwdMetFifoInfo1(A, exceptionVector_f, &dbg_fwd_met_fifo_info1, &exce_vec);
    GetDbgFwdMetFifoInfo1(A, destMap_f, &dbg_fwd_met_fifo_info1, &dest_map);
    GetDbgFwdMetFifoInfo1(A, execMet_f, &dbg_fwd_met_fifo_info1, &execMet);
    GetDbgFwdMetFifoInfo1(A, valid_f, &dbg_fwd_met_fifo_info1, &valid_1);

    /*DbgFwdMetFifoInfo2*/
    sal_memset(&dbg_fwd_met_fifo_info2, 0, sizeof(dbg_fwd_met_fifo_info2));
    cmd = DRV_IOR(DbgFwdMetFifoInfo2_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_fwd_met_fifo_info2);
    GetDbgFwdMetFifoInfo2(A, valid_f, &dbg_fwd_met_fifo_info2, &met_valid);
    GetDbgFwdMetFifoInfo2(A, destMap_f, &dbg_fwd_met_fifo_info2, &dest_mcast_port);
    GetDbgFwdMetFifoInfo2(A, enqueueDiscard_f, &dbg_fwd_met_fifo_info2, &met_enqueueDiscard);
    /*DbgFwdBufStoreInfo*/
    sal_memset(&dbg_fwd_buf_store_info, 0, sizeof(dbg_fwd_buf_store_info));
    cmd = DRV_IOR(DbgFwdBufStoreInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_fwd_buf_store_info);

    cmd = DRV_IOR(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_dsNextHopInternalBase_f);
    DRV_IOCTL(lchip, 0, cmd, &dsNextHopInternalBase);

    if (!IS_BIT_SET(dest_map, 18))
    {
        return CLI_SUCCESS;
    }
    if ((met_valid)&&(valid_1))
    {
        if(p_info->path_sel == 1)
        {
            CTC_DKIT_PRINT("Multicast Process:\n");
            if (execMet)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%d\n", "mcast group", dest_map&0xFFFF);
            }

            cmd = DRV_IOR(MetFifoCtl_t, MetFifoCtl_chipId_f);
            DRV_IOCTL(lchip, 0, cmd, &local_chip);
            cmd = DRV_IOR(MetFifoCtl_t, MetFifoCtl_dsMetEntryBase_f);
            DRV_IOCTL(lchip, 0, cmd, &dsMetEntryBase);
            cmd = DRV_IOR(MetFifoCtl_t, MetFifoCtl_groupIdShiftBit_f);
            DRV_IOCTL(lchip, 0, cmd, &groupIdShiftBit);
            nextMetEntryPtr = (dest_map&0xFFFF) << groupIdShiftBit;
            for (;;)
            {
                metEntryPtr =  nextMetEntryPtr + dsMetEntryBase;
                CTC_DKIT_PATH_PRINT("%-15s\n", "DsMetEntry--->");
                CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "index", metEntryPtr);
                cmd = DRV_IOR(DsMetEntry6W_t, DRV_ENTRY_FLAG);
                ret = DRV_IOCTL(lchip, (metEntryPtr&0x1FFFE), cmd, &ds_met_entry6_w);
                if (ret != DRV_E_NONE)
                {
                    return CLI_ERROR;
                }
                GetDsMetEntry6W(A, currentMetEntryType_f, &ds_met_entry6_w, &currentMetEntryType);
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", currentMetEntryType? "DsMetEntry6W" : "DsMetEntry3W");
                if (currentMetEntryType)
                {
                    sal_memcpy((uint8*)&ds_met_entry, (uint8*)&ds_met_entry6_w, sizeof(ds_met_entry6_w));
                }
                else
                {
                    if (IS_BIT_SET(metEntryPtr, 0))
                    {
                        sal_memcpy((uint8*)&ds_met_entry, ((uint8 *)&ds_met_entry6_w) + sizeof(DsMetEntry3W_m), sizeof(DsMetEntry3W_m));
                    }
                    else
                    {
                        sal_memcpy((uint8*)&ds_met_entry, ((uint8 *)&ds_met_entry6_w), sizeof(DsMetEntry3W_m));
                    }
                    sal_memset(((uint8*)&ds_met_entry) + sizeof(DsMetEntry3W_m), 0, sizeof(DsMetEntry3W_m));
                }

                GetDsMetEntry(A, nextMetEntryPtr_f, &ds_met_entry, &nextMetEntryPtr);
                GetDsMetEntry(A, endLocalRep_f, &ds_met_entry, &endLocalRep);
                GetDsMetEntry(A, isLinkAggregation_f, &ds_met_entry, &isLinkAggregation);
                GetDsMetEntry(A, remoteChip_f, &ds_met_entry, &remoteChip);
                GetDsMetEntry(A, replicationCtl_f, &ds_met_entry, &replicationCtl);
                GetDsMetEntry(A, ucastId_f, &ds_met_entry, &ucastId);
                GetDsMetEntry(A, nextHopExt_f, &ds_met_entry, &nexthopExt);
                if (currentMetEntryType)
                {
                    GetDsMetEntry(A, nextHopPtr_f, &ds_met_entry, &nexthopPtr);
                }
                else
                {
                    cmd = DRV_IOR(MetFifoCtl_t, MetFifoCtl_portBitmapNextHopPtr_f);
                    DRV_IOCTL(lchip, 0, cmd, &nexthopPtr);
                }
                GetDsMetEntry(A, mcastMode_f, &ds_met_entry, &mcastMode);
                GetDsMetEntry(A, apsBridgeEn_f, &ds_met_entry, &apsBridgeEn);

                if(remoteChip)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "to remote chip", "YES");
                }
                else
                {
                    if (!mcastMode && apsBridgeEn)
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "APS bridge", "YES");
                        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "APS group", ucastId);
                    }

                    if (!mcastMode)
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "met mode", "link list");
                        if (isLinkAggregation)
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "do linkagg, group", ucastId&CTC_DKIT_DRV_LINKAGGID_MASK);
                        }
                        else
                        {
                            if (!apsBridgeEn)
                            {
                                CTC_DKIT_PATH_PRINT("%-15s:0x%04x\n", "dest gport",
                                                                          CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(local_chip, ucastId&0x1FF));
                            }
                        }
                        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "replication cnt", replicationCtl&0x1F);

                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "is 8Wnhp", nexthopExt? "YES" : "NO");
                        if ((((replicationCtl >> 5) >> 4)&0x3FFF) != dsNextHopInternalBase)
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "nexthop index", (replicationCtl >> 5));
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", nexthopExt? "DsNextHop8W" : "DsNextHop4W");
                        }
                        else
                        {
                            /*nexthop index equals to bit1~bit3*/
                            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "nexthop index", (((replicationCtl >> 5)&0xF) >> 1));
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", "EpeNextHopInternal");
                        }
                    }
                    else
                    {
                        GetDsMetEntry(A, portBitmapHigh_f, &ds_met_entry, &port_bitmap_high);
                        GetDsMetEntry(A, portBitmap_f, &ds_met_entry, &port_bitmap_low);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "met mode", "port bitmap");
                        if (isLinkAggregation)
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "do linkagg", "YES");
                        }
                        else
                        {
                            sal_memcpy(port_bitmap, port_bitmap_low, 8);
                            port_bitmap[7] |= (port_bitmap_high[0] << 3);
                            CTC_DKIT_PATH_PRINT("%-15s:", "dest port63~0");
                            CTC_DKIT_PRINT("0x%02x", port_bitmap[7]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[6]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[5]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[4]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[3]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[2]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[1]);
                            CTC_DKIT_PRINT("%02x\n", port_bitmap[0]);
                            sal_memset(port_bitmap, 0 , 8);
                            port_bitmap[0] = (port_bitmap_high[1] << 3) + (port_bitmap_high[0] >> 5);
                            port_bitmap[1] = (port_bitmap_high[2] << 3) + (port_bitmap_high[1] >> 5);
                            port_bitmap[2] = (port_bitmap_high[3] << 3) + (port_bitmap_high[2] >> 5);
                            port_bitmap[3] = (port_bitmap_high[4] << 3) + (port_bitmap_high[3] >> 5);
                            port_bitmap[4] = (port_bitmap_high[5] << 3) + (port_bitmap_high[4] >> 5);
                            port_bitmap[5] = (port_bitmap_high[6] << 3) + (port_bitmap_high[5] >> 5);
                            port_bitmap[6] = (port_bitmap_high[7] << 3) + (port_bitmap_high[6] >> 5);
                            port_bitmap[7] = (port_bitmap_high[8] << 3) + (port_bitmap_high[7] >> 5);
                            CTC_DKIT_PATH_PRINT("%-15s:", "dest port127~64");
                            CTC_DKIT_PRINT("0x%02x", port_bitmap[7]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[6]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[5]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[4]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[3]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[2]);
                            CTC_DKIT_PRINT("%02x", port_bitmap[1]);
                            CTC_DKIT_PRINT("%02x\n", port_bitmap[0]);

                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "is 8wnhp", nexthopExt? "YES" : "NO");
                            if (((nexthopPtr >> 4)&0x3FFF) != dsNextHopInternalBase)
                            {
                                CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "nexthop index", nexthopPtr);
                                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", nexthopExt? "DsNextHop8W" : "DsNextHop4W");
                            }
                            else
                            {
                                /*nexthop index equals to bit1~bit3*/
                                CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "nexthop index", ((nexthopPtr &0xF) >> 1));
                                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", "EpeNextHopInternal");
                            }
                        }
                    }
                }
                end_replication = (0xFFFF == nextMetEntryPtr) || (1 == remaining_rcd);
                remaining_rcd--;

                if (end_replication)
                {
                    break;
                }
            }
        }
        else if(p_info->path_sel == 2 || p_info->path_sel == 3)
        {
            CTC_DKIT_PRINT("Exception Process:\n");
            CTC_DKIT_PATH_PRINT("%-15s: 0x%-08x\n", "exception Vector", exce_vec);
        }
    }
    if (met_enqueueDiscard)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Drop!!!", "Drop at METFIFO");
    }
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_bsr_linkagg(ctc_dkit_captured_path_info_t* p_info)
{
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_bsr_queue(ctc_dkit_captured_path_info_t* p_info)
{
    uint32 cmd = 0;
    uint8 lchip = p_info->lchip;
    /*DbgFwdQWriteInfo*/
    uint32 qmgr_channelIdValid = 0;
    uint32 qmgr_channelId = 0;
    uint32 qmgr_cnAction = 0;
    uint32 qmgr_cutThroughEn = 0;
    uint32 qmgr_dsQueueMapTcamData = 0;
    uint32 qmgr_dsQueueMapTcamKey = 0;
    uint32 qmgr_enqueueDiscard = 0;
    uint32 qmgr_forwardDropValid = 0;
    uint32 qmgr_freePtr = 0;
    uint32 qmgr_hitIndex = 0;
    uint32 qmgr_mcastLinkAggregationDiscard = 0;
    uint32 qmgr_noLinkAggregationMemberDiscard = 0;
    uint32 qmgr_pktLenAdjFinal = 0;
    uint32 qmgr_queueId = 0;
    uint32 qmgr_replicationCtl = 0;
    uint32 qmgr_reservedChannelDropValid = 0;
    uint32 qmgr_spanOnDropValid = 0;
    uint32 qmgr_stackingDiscard = 0;
    uint32 qmgr_statsPtr = 0;
    uint32 qmgr_tcamResultValid = 0;
    uint32 qmgr_valid = 0;
    DbgFwdQWriteInfo_m dbg_fwd_que_mgr_info;

    /*DbgErmResrcAllocInfo*/
    uint32 enq_aqmPortCnt = 0;
    uint32 enq_aqmPortEn = 0;
    uint32 enq_aqmPortStall = 0;
    uint32 enq_avgQueueLen = 0;
    uint32 enq_c2cCnt = 0;
    uint32 enq_c2cPass = 0;
    uint32 enq_cngLevel = 0;
    uint32 enq_congestionValid = 0;
    uint32 enq_criticalCnt = 0;
    uint32 enq_criticalPass = 0;
    uint32 enq_dpThrd = 0;
    uint32 enq_isC2cPacket = 0;
    uint32 enq_isCriticalPacket = 0;
    uint32 enq_isEcnMarkValid = 0;
    uint32 enq_isPortGuaranteed = 0;
    uint32 enq_isQueueGuaranteed = 0;
    uint32 enq_microburstValid = 0;
    uint32 enq_portCnt = 0;
    uint32 enq_portRemain = 0;
    uint32 enq_portThrdPass = 0;
    uint32 enq_resourceCheckPass = 0;
    uint32 enq_scCnt = 0;
    uint32 enq_scRemain = 0;
    uint32 enq_scThrdPass = 0;
    uint32 enq_spanCnt = 0;
    uint32 enq_spanOnDropPass = 0;
    uint32 enq_spanOnDropValid = 0;
    uint32 enq_totalCnt = 0;
    uint32 enq_totalThrdPass = 0;
    uint32 enq_valid = 0;
    DbgErmResrcAllocInfo_m  dbg_q_mgr_enq_info;

    sal_memset(&dbg_fwd_que_mgr_info, 0, sizeof(DbgFwdQWriteInfo_m));
    cmd = DRV_IOR(DbgFwdQWriteInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_fwd_que_mgr_info);
    GetDbgFwdQWriteInfo(A, channelIdValid_f, &dbg_fwd_que_mgr_info, &qmgr_channelIdValid);
    GetDbgFwdQWriteInfo(A, channelId_f, &dbg_fwd_que_mgr_info, &qmgr_channelId);
    GetDbgFwdQWriteInfo(A, cnAction_f, &dbg_fwd_que_mgr_info, &qmgr_cnAction);
    GetDbgFwdQWriteInfo(A, cutThroughEn_f, &dbg_fwd_que_mgr_info, &qmgr_cutThroughEn);
    GetDbgFwdQWriteInfo(A, dsQueueMapTcamData_f, &dbg_fwd_que_mgr_info, &qmgr_dsQueueMapTcamData);
    GetDbgFwdQWriteInfo(A, dsQueueMapTcamKey_f, &dbg_fwd_que_mgr_info, &qmgr_dsQueueMapTcamKey);
    GetDbgFwdQWriteInfo(A, enqueueDiscard_f, &dbg_fwd_que_mgr_info, &qmgr_enqueueDiscard);
    GetDbgFwdQWriteInfo(A, forwardDropValid_f, &dbg_fwd_que_mgr_info, &qmgr_forwardDropValid);
    GetDbgFwdQWriteInfo(A, freePtr_f, &dbg_fwd_que_mgr_info, &qmgr_freePtr);
    GetDbgFwdQWriteInfo(A, hitIndex_f, &dbg_fwd_que_mgr_info, &qmgr_hitIndex);
    GetDbgFwdQWriteInfo(A, mcastLinkAggregationDiscard_f, &dbg_fwd_que_mgr_info, &qmgr_mcastLinkAggregationDiscard);
    GetDbgFwdQWriteInfo(A, noLinkAggregationMemberDiscard_f, &dbg_fwd_que_mgr_info, &qmgr_noLinkAggregationMemberDiscard);
    GetDbgFwdQWriteInfo(A, pktLenAdjFinal_f, &dbg_fwd_que_mgr_info, &qmgr_pktLenAdjFinal);
    GetDbgFwdQWriteInfo(A, queueId_f, &dbg_fwd_que_mgr_info, &qmgr_queueId);
    GetDbgFwdQWriteInfo(A, replicationCtl_f, &dbg_fwd_que_mgr_info, &qmgr_replicationCtl);
    GetDbgFwdQWriteInfo(A, reservedChannelDropValid_f, &dbg_fwd_que_mgr_info, &qmgr_reservedChannelDropValid);
    GetDbgFwdQWriteInfo(A, spanOnDropValid_f, &dbg_fwd_que_mgr_info, &qmgr_spanOnDropValid);
    GetDbgFwdQWriteInfo(A, stackingDiscard_f, &dbg_fwd_que_mgr_info, &qmgr_stackingDiscard);
    GetDbgFwdQWriteInfo(A, statsPtr_f, &dbg_fwd_que_mgr_info, &qmgr_statsPtr);
    GetDbgFwdQWriteInfo(A, tcamResultValid_f, &dbg_fwd_que_mgr_info, &qmgr_tcamResultValid);
    GetDbgFwdQWriteInfo(A, valid_f, &dbg_fwd_que_mgr_info, &qmgr_valid);

    sal_memset(&dbg_q_mgr_enq_info, 0, sizeof(DbgErmResrcAllocInfo_m));
    cmd = DRV_IOR(DbgErmResrcAllocInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_q_mgr_enq_info);
    GetDbgErmResrcAllocInfo(A, aqmPortCnt_f, &dbg_q_mgr_enq_info, &enq_aqmPortCnt);
    GetDbgErmResrcAllocInfo(A, aqmPortEn_f, &dbg_q_mgr_enq_info, &enq_aqmPortEn);
    GetDbgErmResrcAllocInfo(A, aqmPortStall_f, &dbg_q_mgr_enq_info, &enq_aqmPortStall);
     /*-GetDbgErmResrcAllocInfo(A, admQueueEn_f, &dbg_q_mgr_enq_info, &enq_aqmQueueEn);*/
    GetDbgErmResrcAllocInfo(A, avgQueueLen_f, &dbg_q_mgr_enq_info, &enq_avgQueueLen);
    GetDbgErmResrcAllocInfo(A, c2cCnt_f, &dbg_q_mgr_enq_info, &enq_c2cCnt);
    GetDbgErmResrcAllocInfo(A, c2cPass_f, &dbg_q_mgr_enq_info, &enq_c2cPass);
    GetDbgErmResrcAllocInfo(A, cngLevel_f, &dbg_q_mgr_enq_info, &enq_cngLevel);
    GetDbgErmResrcAllocInfo(A, congestionValid_f, &dbg_q_mgr_enq_info, &enq_congestionValid);
    GetDbgErmResrcAllocInfo(A, criticalCnt_f, &dbg_q_mgr_enq_info, &enq_criticalCnt);
    GetDbgErmResrcAllocInfo(A, criticalPass_f, &dbg_q_mgr_enq_info, &enq_criticalPass);
    GetDbgErmResrcAllocInfo(A, dpThrd_f, &dbg_q_mgr_enq_info, &enq_dpThrd);
    GetDbgErmResrcAllocInfo(A, isC2cPacket_f, &dbg_q_mgr_enq_info, &enq_isC2cPacket);
    GetDbgErmResrcAllocInfo(A, isCriticalPacket_f, &dbg_q_mgr_enq_info, &enq_isCriticalPacket);
    GetDbgErmResrcAllocInfo(A, isEcnMarkValid_f, &dbg_q_mgr_enq_info, &enq_isEcnMarkValid);
    GetDbgErmResrcAllocInfo(A, isPortGuaranteed_f, &dbg_q_mgr_enq_info, &enq_isPortGuaranteed);
    GetDbgErmResrcAllocInfo(A, isQueueGuaranteed_f, &dbg_q_mgr_enq_info, &enq_isQueueGuaranteed);
    GetDbgErmResrcAllocInfo(A, microburstValid_f, &dbg_q_mgr_enq_info, &enq_microburstValid);
    GetDbgErmResrcAllocInfo(A, scCnt_f, &dbg_q_mgr_enq_info, &enq_portCnt);
    GetDbgErmResrcAllocInfo(A, scRemain_f, &dbg_q_mgr_enq_info, &enq_portRemain);
    GetDbgErmResrcAllocInfo(A, scThrdPass_f, &dbg_q_mgr_enq_info, &enq_portThrdPass);
    GetDbgErmResrcAllocInfo(A, resourceCheckPass_f, &dbg_q_mgr_enq_info, &enq_resourceCheckPass);
    GetDbgErmResrcAllocInfo(A, scCnt_f, &dbg_q_mgr_enq_info, &enq_scCnt);
    GetDbgErmResrcAllocInfo(A, scRemain_f, &dbg_q_mgr_enq_info, &enq_scRemain);
    GetDbgErmResrcAllocInfo(A, scThrdPass_f, &dbg_q_mgr_enq_info, &enq_scThrdPass);
    GetDbgErmResrcAllocInfo(A, spanCnt_f, &dbg_q_mgr_enq_info, &enq_spanCnt);
    GetDbgErmResrcAllocInfo(A, spanOnDropPass_f, &dbg_q_mgr_enq_info, &enq_spanOnDropPass);
    GetDbgErmResrcAllocInfo(A, spanOnDropValid_f, &dbg_q_mgr_enq_info, &enq_spanOnDropValid);
    GetDbgErmResrcAllocInfo(A, totalCnt_f, &dbg_q_mgr_enq_info, &enq_totalCnt);
    GetDbgErmResrcAllocInfo(A, totalThrdPass_f, &dbg_q_mgr_enq_info, &enq_totalThrdPass);
    GetDbgErmResrcAllocInfo(A, valid_f, &dbg_q_mgr_enq_info, &enq_valid);

    if (qmgr_valid)
    {
        CTC_DKIT_PRINT("Queue Process:\n");

        if (qmgr_noLinkAggregationMemberDiscard)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Drop!!!", "port linkagg no member");
        }
        else if (qmgr_mcastLinkAggregationDiscard)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Drop!!!", "mcast linkagg no member");
        }
        else if (qmgr_stackingDiscard)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Drop!!!", "stacking discard");
        }
        else
        {
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "queue id", qmgr_queueId);
        }

        if (enq_valid)
        {
            if (!enq_resourceCheckPass)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Drop!!!", "enqueue discard");
            }
        }

    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_bsr(ctc_dkit_captured_path_info_t* p_info)
{
    CTC_DKIT_PRINT("\nRELAY PROCESS    \n-----------------------------------\n");

    /*1. Mcast Or exception*/
    if (p_info->detail)
    {
        _ctc_usw_dkit_captured_path_bsr_mcast(p_info);
    }
    /*2. Linkagg*/
    if ((!p_info->discard) && (p_info->detail))
    {
        _ctc_usw_dkit_captured_path_bsr_linkagg(p_info);
    }
    /*3. Queue*/
    if (p_info->detail)
    {
        _ctc_usw_dkit_captured_path_bsr_queue(p_info);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_epe_parser(ctc_dkit_captured_path_info_t* p_info)
{
    _ctc_usw_dkit_captured_path_parser(p_info, 2);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_epe_scl(ctc_dkit_captured_path_info_t* p_info)
{
     uint8 slice = p_info->slice_epe;
     uint8 lchip = p_info->lchip;
     uint32 cmd = 0;
     char* key_name[11] =
     {
         "None", "DsEgressXcOamDoubleVlanPortHashKey", "DsEgressXcOamSvlanPortHashKey", "DsEgressXcOamCvlanPortHashKey",
          "DsEgressXcOamSvlanCosPortHashKey", "DsEgressXcOamCvlanCosPortHashKey", "DsEgressXcOamPortVlanCrossHashKey",
          "DsEgressXcOamPortCrossHashKey", "DsEgressXcOamPortHashKey", "DsEgressXcOamSvlanPortMacHashKey",
          "DsEgressXcOamTunnelPbbHashKey"
     };

     /*DbgEgrXcOamHash0EngineFromEpeNhpInfo*/
     uint32 hash0_resultValid = 0;
     uint32 hash0_defaultEntryValid = 0;
      /*uint32 hash0_egressXcOamHashConflict = 0;*/
     uint32 hash0_keyIndex = 0;
     uint32 hash0_valid = 0;
     DbgEgrXcOamHash0EngineFromEpeNhpInfo_m dbg_egr_xc_oam_hash0_engine_from_epe_nhp_info;
     /*DbgEgrXcOamHash1EngineFromEpeNhpInfo*/
     uint32 hash1_resultValid = 0;
     uint32 hash1_defaultEntryValid = 0;
      /*uint32 hash1_egressXcOamHashConflict = 0;*/
     uint32 hash1_keyIndex = 0;
     uint32 hash1_valid = 0;
     DbgEgrXcOamHash1EngineFromEpeNhpInfo_m dbg_egr_xc_oam_hash1_engine_from_epe_nhp_info;
     /*DbgEpeNextHopInfo*/
     uint32 scl_discard = 0;
     uint32 scl_discardType = 0;
     uint32 scl_lookup1Valid = 0;
     uint32 scl_vlanHash1Type = 0;
     uint32 scl_lookup2Valid = 0;
     uint32 scl_vlanHash2Type = 0;
     uint32 scl_vlanXlateResult1Valid = 0;
     uint32 scl_vlanXlateResult2Valid = 0;
     uint32 scl_hashPort = 0;
     uint32 scl_valid = 0;
     DbgEpeNextHopInfo_m dbg_epe_next_hop_info;

     /*DbgEgrXcOamHash0EngineFromEpeNhpInfo*/
     sal_memset(&dbg_egr_xc_oam_hash0_engine_from_epe_nhp_info, 0, sizeof(dbg_egr_xc_oam_hash0_engine_from_epe_nhp_info));
     cmd = DRV_IOR(DbgEgrXcOamHash0EngineFromEpeNhpInfo_t, DRV_ENTRY_FLAG);
     DRV_IOCTL(lchip, slice, cmd, &dbg_egr_xc_oam_hash0_engine_from_epe_nhp_info);
     GetDbgEgrXcOamHash0EngineFromEpeNhpInfo(A, resultValid_f, &dbg_egr_xc_oam_hash0_engine_from_epe_nhp_info, &hash0_resultValid);
     GetDbgEgrXcOamHash0EngineFromEpeNhpInfo(A, defaultEntryValid_f, &dbg_egr_xc_oam_hash0_engine_from_epe_nhp_info, &hash0_defaultEntryValid);
     GetDbgEgrXcOamHash0EngineFromEpeNhpInfo(A, keyIndex_f, &dbg_egr_xc_oam_hash0_engine_from_epe_nhp_info, &hash0_keyIndex);
     GetDbgEgrXcOamHash0EngineFromEpeNhpInfo(A, valid_f, &dbg_egr_xc_oam_hash0_engine_from_epe_nhp_info, &hash0_valid);

     /*DbgEgrXcOamHash1EngineFromEpeNhpInfo*/
     sal_memset(&dbg_egr_xc_oam_hash1_engine_from_epe_nhp_info, 0, sizeof(dbg_egr_xc_oam_hash1_engine_from_epe_nhp_info));
     cmd = DRV_IOR(DbgEgrXcOamHash1EngineFromEpeNhpInfo_t, DRV_ENTRY_FLAG);
     DRV_IOCTL(lchip, slice, cmd, &dbg_egr_xc_oam_hash1_engine_from_epe_nhp_info);
     GetDbgEgrXcOamHash1EngineFromEpeNhpInfo(A, resultValid_f, &dbg_egr_xc_oam_hash1_engine_from_epe_nhp_info, &hash1_resultValid);
     GetDbgEgrXcOamHash1EngineFromEpeNhpInfo(A, defaultEntryValid_f, &dbg_egr_xc_oam_hash1_engine_from_epe_nhp_info, &hash1_defaultEntryValid);
     GetDbgEgrXcOamHash1EngineFromEpeNhpInfo(A, keyIndex_f, &dbg_egr_xc_oam_hash1_engine_from_epe_nhp_info, &hash1_keyIndex);
     GetDbgEgrXcOamHash1EngineFromEpeNhpInfo(A, valid_f, &dbg_egr_xc_oam_hash1_engine_from_epe_nhp_info, &hash1_valid);
     /*DbgEpeNextHopInfo*/
     sal_memset(&dbg_epe_next_hop_info, 0, sizeof(dbg_epe_next_hop_info));
     cmd = DRV_IOR(DbgEpeNextHopInfo_t, DRV_ENTRY_FLAG);
     DRV_IOCTL(lchip, slice, cmd, &dbg_epe_next_hop_info);
     GetDbgEpeNextHopInfo(A, discard_f, &dbg_epe_next_hop_info, &scl_discard);
     GetDbgEpeNextHopInfo(A, discardType_f, &dbg_epe_next_hop_info, &scl_discardType);
     if (DRV_IS_DUET2(lchip))
     {
         GetDbgEpeNextHopInfo(A, lookup1Valid_f, &dbg_epe_next_hop_info, &scl_lookup1Valid);
         GetDbgEpeNextHopInfo(A, vlanHash1Type_f, &dbg_epe_next_hop_info, &scl_vlanHash1Type);
         GetDbgEpeNextHopInfo(A, lookup2Valid_f, &dbg_epe_next_hop_info, &scl_lookup2Valid);
         GetDbgEpeNextHopInfo(A, vlanHash2Type_f, &dbg_epe_next_hop_info, &scl_vlanHash2Type);
         GetDbgEpeNextHopInfo(A, vlanXlateResult1Valid_f, &dbg_epe_next_hop_info, &scl_vlanXlateResult1Valid);
         GetDbgEpeNextHopInfo(A, vlanXlateResult2Valid_f, &dbg_epe_next_hop_info, &scl_vlanXlateResult2Valid);
         GetDbgEpeNextHopInfo(A, hashPort_f, &dbg_epe_next_hop_info, &scl_hashPort);
     }
     else
     {
         GetDbgEpeNextHopInfo(A, lookup0Valid_f, &dbg_epe_next_hop_info, &scl_lookup1Valid);
         GetDbgEpeNextHopInfo(A, vlanHash0Type_f, &dbg_epe_next_hop_info, &scl_vlanHash1Type);
         GetDbgEpeNextHopInfo(A, lookup1Valid_f, &dbg_epe_next_hop_info, &scl_lookup2Valid);
         GetDbgEpeNextHopInfo(A, vlanHash1Type_f, &dbg_epe_next_hop_info, &scl_vlanHash2Type);
         GetDbgEpeNextHopInfo(A, vlanXlateResult0Valid_f, &dbg_epe_next_hop_info, &scl_vlanXlateResult1Valid);
         GetDbgEpeNextHopInfo(A, vlanXlateResult1Valid_f, &dbg_epe_next_hop_info, &scl_vlanXlateResult2Valid);
     }
     GetDbgEpeNextHopInfo(A, valid_f, &dbg_epe_next_hop_info, &scl_valid);

    if (0 == p_info->detail)
    {
        goto Discard;
    }

     if (scl_valid)
     {
         if (hash0_valid || hash1_valid)
         {
             CTC_DKIT_PRINT("SCL Process:\n");
             if (scl_lookup1Valid)
             {
                 CTC_DKIT_PATH_PRINT("%-15s\n", "HASH0 lookup--->");
                 if (hash0_defaultEntryValid)
                 {
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_DEFAULT);
                     CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", hash0_keyIndex);
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name[scl_vlanHash1Type]);
                 }
                 /*else if (hash1_egressXcOamHashConflict)
                 {
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_CONFLICT);
                     CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", hash0_keyIndex);
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", "");
                 }*/
                 else if (hash0_resultValid)
                 {
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
                     CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", hash0_keyIndex);
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name[scl_vlanHash1Type]);
                 }
                 else
                 {
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
                 }
             }

             if (scl_lookup2Valid)
             {
                 CTC_DKIT_PATH_PRINT("%-15s\n", "HASH1 lookup--->");
                 if (hash1_defaultEntryValid)
                 {
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_DEFAULT);
                     CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", hash1_keyIndex);
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name[scl_vlanHash2Type]);
                 }
                 /*else if (hash1_egressXcOamHashConflict)
                 {
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_CONFLICT);
                     CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", hash1_keyIndex);
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name[scl_vlanHash2Type]);
                 }*/
                 else if (hash1_resultValid)
                 {
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
                     CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", hash1_keyIndex);
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name[scl_vlanHash2Type]);
                 }
                 else
                 {
                     CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
                 }
             }
         }
     }

Discard:
         _ctc_usw_dkit_captured_path_discard_process(p_info, scl_discard, scl_discardType, CTC_DKIT_EPE);

    return CLI_SUCCESS;
}
STATIC int32
_ctc_usw_dkit_captured_path_epe_interface(ctc_dkit_captured_path_info_t* p_info)
{
    uint8 slice = p_info->slice_epe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;
    uint32 localPhyPort = 0;
    uint32 interfaceId = 0;
    uint32 destVlanPtr = 0;
    uint32 globalDestPort = 0;
    uint16 gport = 0;
    uint8 gchip = 0;
    uint32 valid = 0;
    uint32 packetOffset = 0;
    uint32 discard = 0;
    uint32 discard_type = 0;

    CTC_DKIT_PRINT("NextHopMapper:\n");

    cmd = DRV_IOR(DbgEpeHdrAdjInfo_t, DbgEpeHdrAdjInfo_valid_f);
    DRV_IOCTL(lchip, slice, cmd, &valid);
    if (valid)
    {
        cmd = DRV_IOR(DbgEpeHdrAdjInfo_t, DbgEpeHdrAdjInfo_localPhyPort_f);
        DRV_IOCTL(lchip, slice, cmd, &localPhyPort);
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_globalDestPort_f);
        DRV_IOCTL(lchip, (localPhyPort + slice*CTC_DKIT_ONE_SLICE_PORT_NUM), cmd, &globalDestPort);
        if (CTC_DKIT_DRV_GPORT_IS_LINKAGG_PORT(globalDestPort))
        {
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "linkagg group", CTC_DKIT_DRV_GPORT_TO_LINKAGG_ID(globalDestPort));
        }
        CTC_DKIT_GET_GCHIP(lchip, gchip);
        gport = CTC_DKIT_DRV_LPORT_TO_CTC_GPORT(gchip, (localPhyPort + slice*CTC_DKIT_ONE_SLICE_PORT_NUM));
        CTC_DKIT_PATH_PRINT("%-15s:0x%04x\n", "dest gport", gport);

        cmd = DRV_IOR(DbgEpeHdrAdjInfo_t, DbgEpeHdrAdjInfo_packetOffset_f);
        DRV_IOCTL(lchip, slice, cmd, &packetOffset);
        if (packetOffset)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%d bytes\n", "strip packet", packetOffset);
        }
    }

    cmd = DRV_IOR(DbgEpeNextHopInfo_t, DbgEpeNextHopInfo_valid_f);
    DRV_IOCTL(lchip, slice, cmd, &valid);
    if (valid)
    {
        cmd = DRV_IOR(DbgEpeNextHopInfo_t, DbgEpeNextHopInfo_destVlanPtr_f);
        DRV_IOCTL(lchip, slice, cmd, &destVlanPtr);
        cmd = DRV_IOR(DbgEpeNextHopInfo_t, DbgEpeNextHopInfo_interfaceId_f);
        DRV_IOCTL(lchip, slice, cmd, &interfaceId);
        if (destVlanPtr < 4096)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "vlan ptr", destVlanPtr);
        }
        if (interfaceId)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "l3 intf id", interfaceId);
        }
    }

    cmd = DRV_IOR(DbgEpeHdrAdjInfo_t, DbgEpeHdrAdjInfo_discard_f);
    DRV_IOCTL(lchip, slice, cmd, &discard);
    cmd = DRV_IOR(DbgEpeHdrAdjInfo_t, DbgEpeHdrAdjInfo_discardType_f);
    DRV_IOCTL(lchip, slice, cmd, &discard_type);
    if (!discard)
    {
        cmd = DRV_IOR(DbgEpeNextHopInfo_t, DbgEpeNextHopInfo_discard_f);
        DRV_IOCTL(lchip, slice, cmd, &discard);
        cmd = DRV_IOR(DbgEpeNextHopInfo_t, DbgEpeNextHopInfo_discardType_f);
        DRV_IOCTL(lchip, slice, cmd, &discard_type);
    }
    _ctc_usw_dkit_captured_path_discard_process(p_info, discard, discard_type, CTC_DKIT_EPE);

    return CLI_SUCCESS;
}
STATIC int32
_ctc_usw_dkit_captured_path_epe_acl(ctc_dkit_captured_path_info_t* p_info)
{
    uint8 slice = p_info->slice_epe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;
    uint8  idx = 0;
    uint32 key_index = 0;
    char* acl_key_type_desc[] = {"TCAML2KEY", "TCAMl3KEY","TCAML3EXTKEY","TCAMIPV6KEY","TCAMIPV6EXTKEY","TCAML2L3KEY","TCAML2L3KEYEXT",
                                 "TCAML2IPV6KEY","TCAMCIDKEY", "TCAMINTFKEY", "TCAMFWDKEY", "TCAMFWDEXT","TCAMCOPPKEY","TCAMCOPPEXTKEY","TCAMUDFKEY"};
    EpeAclQosCtl_m       epe_acl_ctl;
    /*DbgFlowTcamEngineEpeAclInfo*/
    uint32 tcam_egressAcl0TcamResultValid = 0;
    uint32 tcam_egressAcl0TcamIndex = 0;
    uint32 tcam_egressAcl0_valid = 0;
    uint32 tcam_egressAcl1TcamResultValid = 0;
    uint32 tcam_egressAcl1TcamIndex = 0;
    uint32 tcam_egressAcl1_valid = 0;
    uint32 tcam_egressAcl2TcamResultValid = 0;
    uint32 tcam_egressAcl2TcamIndex = 0;
    uint32 tcam_egressAcl2_valid = 0;
    DbgFlowTcamEngineEpeAclInfo0_m dbg_flow_tcam_engine_epe_acl_info;

    /*DbgFlowTcamEngineEpeAclKeyInfo0-3*/
    uint32 acl_key_info[10] = {0};
    uint32 acl_key_valid = 0;
    uint32 acl_key_size = 0;  /* 0:80bit; 1:160bit; 2:320bit; 3:640bit */
    DbgFlowTcamEngineEpeAclKeyInfo0_m dbg_acl_key_info;
    uint8  step = DbgFlowTcamEngineEpeAclKeyInfo1_key_f - DbgFlowTcamEngineEpeAclKeyInfo0_key_f;
    uint32 acl_key_info_key_id[3] = {DbgFlowTcamEngineEpeAclKeyInfo0_t, DbgFlowTcamEngineEpeAclKeyInfo1_t,
                                 DbgFlowTcamEngineEpeAclKeyInfo2_t};

    uint8 step2 = EpeAclQosCtl_gGlbAcl_1_aclLookupType_f - EpeAclQosCtl_gGlbAcl_0_aclLookupType_f ;
    uint32 acl_key_type = 0;

    /*DbgEpeAclInfo*/
    uint32 acl_discard = 0;
    uint32 acl_discardType = 0;
    uint32 acl_exceptionEn = 0;
    uint32 acl_exceptionIndex = 0;
    uint32 acl_en_bitmap = 0;
    uint32 acl_valid = 0;
    DbgEpeAclInfo_m dbg_epe_acl_info;

    sal_memset(&epe_acl_ctl, 0, sizeof(epe_acl_ctl));
    /*DbgFlowTcamEngineEpeAclInfo0*/
    sal_memset(&dbg_flow_tcam_engine_epe_acl_info, 0, sizeof(dbg_flow_tcam_engine_epe_acl_info));
    cmd = DRV_IOR(DbgFlowTcamEngineEpeAclInfo0_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_epe_acl_info);
    GetDbgFlowTcamEngineEpeAclInfo0(A, egrTcamResultValid0_f, &dbg_flow_tcam_engine_epe_acl_info, &tcam_egressAcl0TcamResultValid);
    GetDbgFlowTcamEngineEpeAclInfo0(A, egrTcamIndex0_f, &dbg_flow_tcam_engine_epe_acl_info, &tcam_egressAcl0TcamIndex);
    GetDbgFlowTcamEngineEpeAclInfo0(A, valid_f, &dbg_flow_tcam_engine_epe_acl_info, &tcam_egressAcl0_valid);

    /*DbgFlowTcamEngineEpeAclInfo1*/
    sal_memset(&dbg_flow_tcam_engine_epe_acl_info, 0, sizeof(dbg_flow_tcam_engine_epe_acl_info));
    cmd = DRV_IOR(DbgFlowTcamEngineEpeAclInfo1_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_epe_acl_info);
    GetDbgFlowTcamEngineEpeAclInfo1(A, egrTcamResultValid1_f, &dbg_flow_tcam_engine_epe_acl_info, &tcam_egressAcl1TcamResultValid);
    GetDbgFlowTcamEngineEpeAclInfo1(A, egrTcamIndex1_f, &dbg_flow_tcam_engine_epe_acl_info, &tcam_egressAcl1TcamIndex);
    GetDbgFlowTcamEngineEpeAclInfo1(A, valid_f, &dbg_flow_tcam_engine_epe_acl_info, &tcam_egressAcl1_valid);

    /*DbgFlowTcamEngineEpeAclInfo2*/
    sal_memset(&dbg_flow_tcam_engine_epe_acl_info, 0, sizeof(dbg_flow_tcam_engine_epe_acl_info));
    cmd = DRV_IOR(DbgFlowTcamEngineEpeAclInfo2_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &dbg_flow_tcam_engine_epe_acl_info);
    GetDbgFlowTcamEngineEpeAclInfo2(A, egrTcamResultValid2_f, &dbg_flow_tcam_engine_epe_acl_info, &tcam_egressAcl2TcamResultValid);
    GetDbgFlowTcamEngineEpeAclInfo2(A, egrTcamIndex2_f, &dbg_flow_tcam_engine_epe_acl_info, &tcam_egressAcl2TcamIndex);
    GetDbgFlowTcamEngineEpeAclInfo2(A, valid_f, &dbg_flow_tcam_engine_epe_acl_info, &tcam_egressAcl2_valid);

    /*IpePreLookupAclCtl*/
    cmd = DRV_IOR(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &epe_acl_ctl);

    /*DbgEpeAclInfo*/
    sal_memset(&dbg_epe_acl_info, 0, sizeof(dbg_epe_acl_info));
    cmd = DRV_IOR(DbgEpeAclInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_epe_acl_info);
    GetDbgEpeAclInfo(A, exceptionEn_f, &dbg_epe_acl_info, &acl_exceptionEn);
    GetDbgEpeAclInfo(A, exceptionIndex_f, &dbg_epe_acl_info, &acl_exceptionIndex);
    GetDbgEpeAclInfo(A, aclLookupLevelEnBitMap_f, &dbg_epe_acl_info, &acl_en_bitmap);
    GetDbgEpeAclInfo(A, valid_f, &dbg_epe_acl_info, &acl_valid);

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    if (acl_en_bitmap && acl_valid)
    {
        CTC_DKIT_PRINT("ACL Process:\n");

        /*tcam*/
        if (tcam_egressAcl0_valid || tcam_egressAcl1_valid || tcam_egressAcl2_valid)
        {
            char* lookup_result[3] = {"TCAM0 lookup--->", "TCAM1 lookup--->", "TCAM2 lookup--->"};
            uint32 result_valid[3] = {tcam_egressAcl0TcamResultValid, tcam_egressAcl1TcamResultValid,
                                        tcam_egressAcl2TcamResultValid};
            uint32 tcam_index[3] = {tcam_egressAcl0TcamIndex, tcam_egressAcl1TcamIndex,
                                        tcam_egressAcl2TcamIndex};

            for (idx = 0; idx < 3 ; idx++)
            {
                if ((acl_en_bitmap >> idx) & 0x1)
                {
                    CTC_DKIT_PATH_PRINT("%-15s\n", lookup_result[idx]);

                    cmd = DRV_IOR(acl_key_info_key_id[idx], DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, 0, cmd, &dbg_acl_key_info);
                    DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineEpeAclKeyInfo0_key_f + step * idx,
                                    &dbg_acl_key_info, &acl_key_info);
                    DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineEpeAclKeyInfo0_valid_f + step * idx,
                                    &dbg_acl_key_info, &acl_key_valid);
                    DRV_GET_FIELD_A(lchip, acl_key_info_key_id[idx], DbgFlowTcamEngineEpeAclKeyInfo0_keySize_f + step * idx,
                                    &dbg_acl_key_info, &acl_key_size);
                    /*get key type*/
                    if(acl_key_valid)
                    {
                        if(11 == GetEpeAclQosCtl(V, gGlbAcl_0_aclLookupType_f + step2 * idx, &epe_acl_ctl))
                        {
                            GetDsAclQosCoppKey320Ing0(A, keyType_f, acl_key_info, &acl_key_type);
                            if(0 == acl_key_type)
                            {
                                acl_key_type = 12;
                            }
                            else
                            {
                                acl_key_type = 13;
                            }
                        }
                        else
                        {
                            GetDsAclQosL3Key320Ing0(A, aclQosKeyType0_f, acl_key_info, &acl_key_type);
                        }
                        if(acl_key_type<=14)
                        {
                            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key type", acl_key_type_desc[acl_key_type]);
                        }
                    }
                    if (result_valid[idx])
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_TCAM_HIT);
                        _ctc_usw_dkit_map_tcam_key_index(lchip, tcam_index[idx], 1, idx, &key_index);
                        CTC_DKIT_PATH_PRINT("%-15s:%u (%dbit)\n", "key index", key_index/(1<<acl_key_size),80*(1<<acl_key_size));
                    }
                    else
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
                    }
                }
            }
        }

    }


Discard:
     if (acl_discard)
     {
         if ((acl_discardType + EPE_FEATURE_START) == EPE_DS_ACL_DIS)
         {
             _ctc_usw_dkit_captured_path_discard_process(p_info, acl_discard, acl_discardType, CTC_DKIT_EPE);
         }
     }

     if (acl_exceptionEn)
     {
         CTC_DKIT_PATH_PRINT("%-15s:%s!!!\n", "Exception", "To CPU");
     }

    return CLI_SUCCESS;
}
STATIC int32
_ctc_usw_dkit_captured_path_epe_oam(ctc_dkit_captured_path_info_t* p_info)
{
    uint8 slice = p_info->slice_epe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;
    uint8 i = 0;
    char* lookup_result[4] = {"ETHER lookup--->", "BFD lookup--->", "TP lookup--->", "Section lookup--->"};
    char* key_name[4] = {"DsEgressXcOamEthHashKey", "DsEgressXcOamBfdHashKey",
                                             "DsEgressXcOamMplsLabelHashKey", "DsEgressXcOamMplsSectionHashKey"};

    /*DbgEgrXcOamHashEngineFromEpeOam0Info*/
    uint32 hash0_resultValid = 0;
    uint32 hash0_egressXcOamHashConflict = 0;
    uint32 hash0_keyIndex = 0;
    uint32 hash0_valid = 0;
    DbgEgrXcOamHashEngineFromEpeOam0Info_m dbg_egr_xc_oam_hash_engine_from_epe_oam0_info;
    /*DbgEgrXcOamHashEngineFromEpeOam1Info*/
    uint32 hash1_resultValid = 0;
    uint32 hash1_egressXcOamHashConflict = 0;
    uint32 hash1_keyIndex = 0;
    uint32 hash1_valid = 0;
    DbgEgrXcOamHashEngineFromEpeOam1Info_m dbg_egr_xc_oam_hash_engine_from_epe_oam1_info;
    /*DbgEgrXcOamHashEngineFromEpeOam2Info*/
    uint32 hash2_resultValid = 0;
    uint32 hash2_egressXcOamHashConflict = 0;
    uint32 hash2_keyIndex = 0;
    uint32 hash2_valid = 0;
    DbgEgrXcOamHashEngineFromEpeOam2Info_m dbg_egr_xc_oam_hash_engine_from_epe_oam2_info;
    /*DbgEpeAclInfo*/
    uint32 lkp_egressXcOamKeyType0 = 0;
    uint32 lkp_egressXcOamKeyType1 = 0;
    uint32 lkp_egressXcOamKeyType2 = 0;
    /*DbgEpeOamInfo*/
    uint32 oam_lmStatsEn0 = 0;
    uint32 oam_lmStatsEn1 = 0;
    uint32 oam_lmStatsEn2 = 0;
    uint32 oam_lmStatsIndex = 0;
    uint32 oam_lmStatsPtr0 = 0;
    uint32 oam_lmStatsPtr1 = 0;
    uint32 oam_lmStatsPtr2 = 0;
    uint32 oam_etherLmValid = 0;
    uint32 oam_mplsLmValid = 0;
    uint32 oam_oamDestChipId = 0;
    uint32 oam_mepIndex = 0;
    uint32 oam_mepEn = 0;
    uint32 oam_mipEn = 0;
    uint32 oam_tempRxOamType = 0;
    uint32 oam_valid = 0;
    DbgEpeOamInfo_m dbg_epe_oam_info;

    /*DbgEpeHdrEditInfo*/
    uint32 hdr_valid = 0;
    uint32 hdr_shareType = 0;
    uint32 hdr_loopbackEn = 0;

    uint8 key_type[3] = {lkp_egressXcOamKeyType0, lkp_egressXcOamKeyType1, lkp_egressXcOamKeyType2};
    uint8 hash_valid[3] = {hash0_valid, hash1_valid, hash2_valid};
    uint8 confict[3] = {hash0_egressXcOamHashConflict, hash1_egressXcOamHashConflict, hash2_egressXcOamHashConflict};
    uint32 key_index[3] = {hash0_keyIndex, hash1_keyIndex, hash2_keyIndex};
    uint8 hash_resultValid[3] = {hash0_resultValid,hash1_resultValid,hash2_resultValid};

    /*DbgEgrXcOamHashEngineFromEpeOam0Info*/
    sal_memset(&dbg_egr_xc_oam_hash_engine_from_epe_oam0_info, 0, sizeof(dbg_egr_xc_oam_hash_engine_from_epe_oam0_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromEpeOam0Info_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_egr_xc_oam_hash_engine_from_epe_oam0_info);
    GetDbgEgrXcOamHashEngineFromEpeOam0Info(A, resultValid_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam0_info, &hash0_resultValid);
    GetDbgEgrXcOamHashEngineFromEpeOam0Info(A, hashConflict_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam0_info, &hash0_egressXcOamHashConflict);
    GetDbgEgrXcOamHashEngineFromEpeOam0Info(A, keyIndex_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam0_info, &hash0_keyIndex);
    GetDbgEgrXcOamHashEngineFromEpeOam0Info(A, oamKeyType_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam0_info, &lkp_egressXcOamKeyType0);
    /*DbgEgrXcOamHashEngineFromEpeOam1Info*/
    sal_memset(&dbg_egr_xc_oam_hash_engine_from_epe_oam1_info, 0, sizeof(dbg_egr_xc_oam_hash_engine_from_epe_oam1_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromEpeOam1Info_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_egr_xc_oam_hash_engine_from_epe_oam1_info);
    GetDbgEgrXcOamHashEngineFromEpeOam1Info(A, resultValid_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam1_info, &hash1_resultValid);
    GetDbgEgrXcOamHashEngineFromEpeOam1Info(A, hashConflict_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam1_info, &hash1_egressXcOamHashConflict);
    GetDbgEgrXcOamHashEngineFromEpeOam1Info(A, keyIndex_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam1_info, &hash1_keyIndex);
    GetDbgEgrXcOamHashEngineFromEpeOam1Info(A, valid_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam1_info, &hash1_valid);
    GetDbgEgrXcOamHashEngineFromEpeOam1Info(A, oamKeyType_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam1_info, &lkp_egressXcOamKeyType1);
    /*DbgEgrXcOamHashEngineFromEpeOam2Info*/
    sal_memset(&dbg_egr_xc_oam_hash_engine_from_epe_oam2_info, 0, sizeof(dbg_egr_xc_oam_hash_engine_from_epe_oam2_info));
    cmd = DRV_IOR(DbgEgrXcOamHashEngineFromEpeOam2Info_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_egr_xc_oam_hash_engine_from_epe_oam2_info);
    GetDbgEgrXcOamHashEngineFromEpeOam2Info(A, resultValid_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam2_info, &hash2_resultValid);
    GetDbgEgrXcOamHashEngineFromEpeOam2Info(A, hashConflict_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam2_info, &hash2_egressXcOamHashConflict);
    GetDbgEgrXcOamHashEngineFromEpeOam2Info(A, keyIndex_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam2_info, &hash2_keyIndex);
    GetDbgEgrXcOamHashEngineFromEpeOam2Info(A, valid_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam2_info, &hash2_valid);
    GetDbgEgrXcOamHashEngineFromEpeOam2Info(A, oamKeyType_f, &dbg_egr_xc_oam_hash_engine_from_epe_oam2_info, &lkp_egressXcOamKeyType2);
    /*DbgEpeOamInfo*/
    sal_memset(&dbg_epe_oam_info, 0, sizeof(dbg_epe_oam_info));
    cmd = DRV_IOR(DbgEpeOamInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_epe_oam_info);
    GetDbgEpeOamInfo(A, lmStatsEn0_f, &dbg_epe_oam_info, &oam_lmStatsEn0);
    GetDbgEpeOamInfo(A, lmStatsEn1_f, &dbg_epe_oam_info, &oam_lmStatsEn1);
    GetDbgEpeOamInfo(A, lmStatsEn2_f, &dbg_epe_oam_info, &oam_lmStatsEn2);
    GetDbgEpeOamInfo(A, lmStatsIndex_f, &dbg_epe_oam_info, &oam_lmStatsIndex);
    GetDbgEpeOamInfo(A, lmStatsPtr0_f, &dbg_epe_oam_info, &oam_lmStatsPtr0);
    GetDbgEpeOamInfo(A, lmStatsPtr1_f, &dbg_epe_oam_info, &oam_lmStatsPtr1);
    GetDbgEpeOamInfo(A, lmStatsPtr2_f, &dbg_epe_oam_info, &oam_lmStatsPtr2);
    GetDbgEpeOamInfo(A, etherLmValid_f, &dbg_epe_oam_info, &oam_etherLmValid);
    GetDbgEpeOamInfo(A, mplsLmValid_f, &dbg_epe_oam_info, &oam_mplsLmValid);
    GetDbgEpeOamInfo(A, oamDestChipId_f, &dbg_epe_oam_info, &oam_oamDestChipId);
    GetDbgEpeOamInfo(A, mepIndex_f, &dbg_epe_oam_info, &oam_mepIndex);
    GetDbgEpeOamInfo(A, mepEn_f, &dbg_epe_oam_info, &oam_mepEn);
    GetDbgEpeOamInfo(A, mipEn_f, &dbg_epe_oam_info, &oam_mipEn);
    GetDbgEpeOamInfo(A, tempRxOamType_f, &dbg_epe_oam_info, &oam_tempRxOamType);
    GetDbgEpeOamInfo(A, valid_f, &dbg_epe_oam_info, &oam_valid);

    /*DbgEpeHdrEditInfo*/
    cmd = DRV_IOR(DbgEpeHdrEditInfo_t, DbgEpeHdrEditInfo_valid_f);
    DRV_IOCTL(lchip, slice, cmd, &hdr_valid);
    cmd = DRV_IOR(DbgEpeHdrEditInfo_t, DbgEpeHdrEditInfo_shareType_f);
    DRV_IOCTL(lchip, slice, cmd, &hdr_shareType);
    cmd = DRV_IOR(DbgEpeHdrEditInfo_t, DbgEpeHdrEditInfo_loopbackEn_f);
    DRV_IOCTL(lchip, slice, cmd, &hdr_loopbackEn);

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    if (lkp_egressXcOamKeyType0 || lkp_egressXcOamKeyType1 || lkp_egressXcOamKeyType2)
    {

        CTC_DKIT_PRINT("OAM Process:\n");

        for (i = 0; i < 3; i++)
        {
            if (key_type[i])
            {
                CTC_DKIT_PATH_PRINT("%s%d %s\n", "hash",i,lookup_result[key_type[i] - EGRESSXCOAMHASHTYPE_ETH]);
                if (hash_valid[i])
                {
                    if (confict[i])
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_CONFLICT);
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", key_index[i]);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name[key_type[i] - EGRESSXCOAMHASHTYPE_ETH]);
                    }
                    else if (hash_resultValid[i])
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_HASH_HIT);
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "key index", key_index[i]);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "key name", key_name[key_type[i] - EGRESSXCOAMHASHTYPE_ETH]);
                    }
                    else
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "result", CTC_DKIT_CAPTURED_NOT_HIT);
                    }
                }
            }
        }
    }

    if (oam_valid)
    {
        if (hash0_resultValid || hash1_resultValid || hash2_resultValid)
        {

            if (oam_mepEn)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "hit MEP", "YES");
                CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "mep index", oam_mepIndex);
                CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Master chip id", oam_oamDestChipId);
            }
            if (oam_mipEn)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "hit MIP", "YES");
                CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Master chip id", oam_oamDestChipId);
            }
            if (oam_tempRxOamType)
            {
                char* oam_type[RXOAMTYPE_TRILLBFD] =
                {
                    "ETHER OAM", "IP BFD", "PBT OAM", "PBBBSI", "PBBBV",
                                                                                      "MPLS OAM", "MPLS BFD", "ACH OAM", "RSV", "TRILL BFD"};
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "OAM type", oam_type[oam_tempRxOamType - 1]);
                }
                if (oam_mplsLmValid || oam_etherLmValid)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s 0x%x\n", "read LM from", "DsOamLmStats", oam_lmStatsIndex);
                }
                if (oam_lmStatsEn0)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s 0x%x\n", "write LM to", "DsOamLmStats", oam_lmStatsPtr0);
                }
                if (oam_lmStatsEn1)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s 0x%x\n", "write LM to", "DsOamLmStats", oam_lmStatsPtr1);
                }
                if (oam_lmStatsEn2)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s 0x%x\n", "write LM to", "DsOamLmStats", oam_lmStatsPtr2);
                }
            }
        }

    if ((hdr_valid) && (SHARETYPE_OAM == hdr_shareType))
    {
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "destination", "OAM engine");
        _ctc_usw_dkit_captured_path_oam_rx(p_info);
    }

Discard:

    return CLI_SUCCESS;
}
STATIC int32
_ctc_usw_dkit_captured_path_epe_edit(ctc_dkit_captured_path_info_t* p_info)
{
    uint8 slice = p_info->slice_epe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 next_edit_ptr = 0;
    uint32 next_edit_type = 0;
    uint32 dsNextHopInternalBase = 0;
    uint32 nhpShareType = 0;
    uint32 discard = 0;
    uint32 discard_type = 0;
    uint8 do_l2_edit = 0;
    uint32 l2_edit_ptr = 0;
    uint8 l2_individual_memory = 0;
    uint32 dest_map = 0;
    uint8 no_l3_edit = 1;
    uint8 no_l2_edit = 1;
    uint32 nhpOuterEditPtrType = 0;

    hw_mac_addr_t hw_mac = {0};
    mac_addr_t mac_da = {0};
    DsNextHop8W_m ds_next_hop8_w;
    DsNextHop4W_m ds_next_hop4_w;
    DsL2EditEth3W_m ds_l2_edit_eth3_w;
    DsL3EditMpls3W_m ds_l3_edit_mpls3_w;
    DsL2Edit6WOuter_m ds_l2_edit6_w_outer;
    DsL2EditSwap_m ds_l2_edit_swap;
    DsL2EditInnerSwap_m ds_l2_edit_inner_swap;
    DsL3Edit6W3rd_m ds_l3_edit_6w_3rd;
    char* l3_edit[12] =
    {
        "NONE edit--->",
        "MPLS edit--->",
        "RESERVED edit--->",
        "NAT edit--->",
        "NAT edit--->",
        "TUNNELV4 edit--->",
        "TUNNELV6 edit--->",
        "L3FLEX edit--->",
        "OPEN FLOW edit--->",
        "OPEN FLOW edit--->",
        "LOOPBACK edit--->",
        "TRILL edit--->"
    };
    char* l3_edit_name[12] =
    {
        " ",
        "DsL3EditMpls3W",
        " ",
        "DsL3EditNat3W",
        "DsL3EditNat6W",
        "DsL3EditTunnelV4",
        "DsL3EditTunnelV6",
        "DsL3EditFlex",
        "DsL3EditOf6W",
        "DsL3EditOf12W",
        "DsL3EditLoopback",
        "DsL3EditTrill"
    };
    char* vlan_action[4] = {"None", "Replace", "Add","Delete"};

    /*DbgEpeHdrAdjInfo*/
    uint32 adj_channelId = 0;
    uint32 adj_packetHeaderEn = 0;
    uint32 adj_packetHeaderEnEgress = 0;
    uint32 adj_bypassAll = 0;
    uint32 adj_discard = 0;
    uint32 adj_discardType = 0;
    uint32 adj_isSendtoCpuEn = 0;
    uint32 adj_localPhyPort = 0;
    uint32 adj_nextHopPtr = 0;
    uint32 adj_parserOffset = 0;
    uint32 adj_packetLengthAdjustType = 0;
    uint32 adj_packetLengthAdjust = 0;
    uint32 adj_sTagAction = 0;
    uint32 adj_cTagAction = 0;
    uint32 adj_packetOffset = 0;
    uint32 adj_isidValid = 0;
    uint32 adj_muxLengthType = 0;
    uint32 adj_cvlanTagOperationValid = 0;
    uint32 adj_svlanTagOperationValid = 0;
    uint32 adj_valid = 0;
    DbgEpeHdrAdjInfo_m dbg_epe_hdr_adj_info;
    /*DbgEpeNextHopInfo*/
    uint32 nhp_discard = 0;
    uint32 nhp_discardType = 0;
    uint32 nhp_payloadOperation = 0;
    uint32 nhp_editBypass = 0;
    uint32 nhp_innerEditHashEn = 0;
    uint32 nhp_outerEditHashEn = 0;
    uint32 nhp_etherOamDiscard = 0;
    uint32 nhp_exceptionEn = 0;
    uint32 nhp_exceptionIndex = 0;
    uint32 nhp_portReflectiveDiscard = 0;
    uint32 nhp_interfaceId = 0;
    uint32 nhp_destVlanPtr = 0;
    uint32 nhp_routeNoL2Edit = 0;
    uint32 nhp_outputSvlanIdValid = 0;
    uint32 nhp_outputSvlanId = 0;
    uint32 nhp_logicDestPort = 0;
    uint32 nhp_outputCvlanIdValid = 0;
    uint32 nhp_outputCvlanId = 0;
    uint32 nhp_l3EditDisable = 0;
    uint32 nhp_l2EditDisable = 0;
    uint32 nhp_isEnd = 0;
    uint32 nhp_bypassAll = 0;
    uint32 nhp_nhpIs8w = 0;
    uint32 nhp_portIsolateValid = 0;
    uint32 nhp_lookup1Valid = 0;
    uint32 nhp_vlanHash1Type = 0;
    uint32 nhp_lookup2Valid = 0;
    uint32 nhp_vlanHash2Type = 0;
    uint32 nhp_vlanXlateResult1Valid = 0;
    uint32 nhp_vlanXlateResult2Valid = 0;
    uint32 nhp_hashPort = 0;
    uint32 nhp_innerEditPtrOffset = 0;
    uint32 nhp_outerEditPtrOffset = 0;
    uint32 nhp_valid = 0;
    uint32 nhp_innerEditPtr = 0;
    uint32 nhp_outerEditPtr = 0;
    DbgEpeNextHopInfo_m dbg_epe_next_hop_info;
    /*DbgEpePayLoadInfo*/
    uint32 pld_discard = 0;
    uint32 pld_discardType = 0;
    uint32 pld_exceptionEn = 0;
    uint32 pld_exceptionIndex = 0;
     /*uint32 pld_shareType = 0;*/
     /*uint32 pld_fid = 0;*/
    uint32 pld_payloadOperation = 0;
    uint32 pld_newTtlValid = 0;
    uint32 pld_newTtl = 0;
    uint32 pld_newDscpValid = 0;
     /*uint32 pld_replaceDscp = 0;*/
     /*uint32 pld_useOamTtlOrExp = 0;*/
     /*uint32 pld_copyDscp = 0;*/
    uint32 pld_newDscp = 0;
    uint32 pld_svlanTagOperation = 0;
    uint32 pld_l2NewSvlanTag = 0;
    uint32 pld_cvlanTagOperation = 0;
    uint32 pld_l2NewCvlanTag = 0;
    uint32 pld_stripOffset = 0;
     /*uint32 pld_mirroredPacket = 0;*/
     /*uint32 pld_isL2Ptp = 0;*/
     /*uint32 pld_isL4Ptp = 0;*/
    uint32 pld_valid = 0;
    DbgEpePayLoadInfo_m dbg_epe_pay_load_info;
    /*DbgEpeEgressEditInfo*/
    uint32 edit_discard = 0;
    uint32 edit_discardType = 0;
    uint32 edit_nhpOuterEditLocation = 0;
    uint32 edit_innerEditExist = 0;
    uint32 edit_innerEditPtrType = 0;
    uint32 edit_outerEditPipe1Exist = 0;
    uint32 edit_outerEditPipe1Type = 0;
    uint32 edit_outerEditPipe2Exist = 0;
    uint32 edit_outerEditPipe3Exist = 0;
    uint32 edit_outerEditPipe3Type = 0;
    uint32 edit_l3RewriteType = 0;
    uint32 edit_l3EditMplsPwExist = 0;
    uint32 edit_l3EditMplsLspExist = 0;
    uint32 edit_l3EditMplsSpmeExist = 0;
    uint32 edit_innerL2RewriteType = 0;
    uint32 edit_l2RewriteType = 0;
    uint32 edit_valid = 0;
    DbgEpeEgressEditInfo_m dbg_epe_egress_edit_info;
    /*DbgEpeInnerL2EditInfo*/
    uint32 innerL2_valid = 0;
    uint32 innerL2_isInnerL2EthSwapOp = 0;
    uint32 innerL2_isInnerL2EthAddOp = 0;
    uint32 innerL2_isInnerDsLiteOp = 0;
    DbgEpeInnerL2EditInfo_m dbg_epe_inner_l2_edit_info;
    /*DbgEpeOuterL2EditInfo*/
    uint32 l2edit_l2EditDisable = 0;
    uint32 l2edit_routeNoL2Edit = 0;
    uint32 l2edit_newMacSaValid = 0;
    uint32 l2edit_newMacDaValid = 0;
    uint32 l2edit_svlanTagOperation = 0;
    uint32 l2edit_cvlanTagOperation = 0;
    uint32 l2edit_l2RewriteType = 0;
    uint32 l2edit_isEnd = 0;
    uint32 l2edit_valid = 0;
    DbgEpeOuterL2EditInfo_m dbg_epe_outer_l2_edit_info;
    /*DbgEpeL3EditInfo*/
    uint32 l3edit_l3EditDisable = 0;
    uint32 l3edit_l3RewriteType = 0;
    uint32 l3edit_label0Valid = 0;
    uint32 l3edit_label1Valid = 0;
    uint32 l3edit_label2Valid = 0;
    uint32 l3edit_isLspLabelEntropy0 = 0;
    uint32 l3edit_isLspLabelEntropy1 = 0;
    uint32 l3edit_isLspLabelEntropy2 = 0;
    uint32 l3edit_isPwLabelEntropy0 = 0;
    uint32 l3edit_isPwLabelEntropy1 = 0;
    uint32 l3edit_isPwLabelEntropy2 = 0;
    uint32 l3edit_cwLabelValid = 0;
    uint32 l3edit_needDot1aeEncrypt = 0;
    uint32 l3edit_dot1aeSaIndex = 0;
    uint32 l3edit_dot1aeSaIndexBase = 0;
    uint32 l3edit_isEnd = 0;
    uint32 l3edit_valid = 0;
    DbgEpeL3EditInfo_m dbg_epe_l3_edit_info;

    /*Dot1AE*/
    uint8  step = 0;
    uint32 dot1ae_sci_en = 0;
    uint32 dot1ae_current_an = 0;
    uint32 dot1ae_next_an = 0;
    uint32 dot1ae_pn = 0;
    uint64 dot1ae_pn_thrd = 0;
    uint32 dot1ae_ebit_cbit[4] = {0};

    /*DbgEpeHdrEditInfo*/
    uint32 hdr_valid = 0;
    uint32 hdr_stripOffset = 0;

    cmd = DRV_IOR(DbgEpeHdrEditInfo_t, DbgEpeHdrEditInfo_valid_f);
    DRV_IOCTL(lchip, slice, cmd, &hdr_valid);
    cmd = DRV_IOR(DbgEpeHdrEditInfo_t, DbgEpeHdrEditInfo_stripOffset_f);
    DRV_IOCTL(lchip, slice, cmd, &hdr_stripOffset);

    sal_memset(&ds_next_hop8_w, 0, sizeof(ds_next_hop8_w));
    sal_memset(&ds_next_hop4_w, 0, sizeof(ds_next_hop4_w));
    sal_memset(&ds_l2_edit_eth3_w, 0, sizeof(ds_l2_edit_eth3_w));
    sal_memset(&ds_l3_edit_mpls3_w, 0, sizeof(ds_l3_edit_mpls3_w));
    sal_memset(&ds_l2_edit6_w_outer, 0, sizeof(ds_l2_edit6_w_outer));
    sal_memset(&ds_l2_edit_swap, 0, sizeof(ds_l2_edit_swap));
    sal_memset(&ds_l2_edit_inner_swap, 0, sizeof(ds_l2_edit_inner_swap));
    sal_memset(&ds_l3_edit_6w_3rd, 0, sizeof(ds_l3_edit_6w_3rd));

    /*DbgEpeHdrAdjInfo*/
    sal_memset(&dbg_epe_hdr_adj_info, 0, sizeof(dbg_epe_hdr_adj_info));
    cmd = DRV_IOR(DbgEpeHdrAdjInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_epe_hdr_adj_info);
    GetDbgEpeHdrAdjInfo(A, channelId_f, &dbg_epe_hdr_adj_info, &adj_channelId);
    GetDbgEpeHdrAdjInfo(A, packetHeaderEn_f, &dbg_epe_hdr_adj_info, &adj_packetHeaderEn);
    GetDbgEpeHdrAdjInfo(A, packetHeaderEnEgress_f, &dbg_epe_hdr_adj_info, &adj_packetHeaderEnEgress);
    GetDbgEpeHdrAdjInfo(A, bypassAll_f, &dbg_epe_hdr_adj_info, &adj_bypassAll);
    GetDbgEpeHdrAdjInfo(A, discard_f, &dbg_epe_hdr_adj_info, &adj_discard);
    GetDbgEpeHdrAdjInfo(A, discardType_f, &dbg_epe_hdr_adj_info, &adj_discardType);
    /* GetDbgEpeHdrAdjInfo(A, microBurstValid_f, &dbg_epe_hdr_adj_info, &adj_microBurstValid);*/
    GetDbgEpeHdrAdjInfo(A, isSendtoCpuEn_f, &dbg_epe_hdr_adj_info, &adj_isSendtoCpuEn);
    /* GetDbgEpeHdrAdjInfo(A, packetLengthIsTrue_f, &dbg_epe_hdr_adj_info, &adj_packetLengthIsTrue);*/
    GetDbgEpeHdrAdjInfo(A, localPhyPort_f, &dbg_epe_hdr_adj_info, &adj_localPhyPort);
    GetDbgEpeHdrAdjInfo(A, nextHopPtr_f, &dbg_epe_hdr_adj_info, &adj_nextHopPtr);
    GetDbgEpeHdrAdjInfo(A, parserOffset_f, &dbg_epe_hdr_adj_info, &adj_parserOffset);
    GetDbgEpeHdrAdjInfo(A, packetLengthAdjustType_f, &dbg_epe_hdr_adj_info, &adj_packetLengthAdjustType);
    GetDbgEpeHdrAdjInfo(A, packetLengthAdjust_f, &dbg_epe_hdr_adj_info, &adj_packetLengthAdjust);
    GetDbgEpeHdrAdjInfo(A, sTagAction_f, &dbg_epe_hdr_adj_info, &adj_sTagAction);
    GetDbgEpeHdrAdjInfo(A, cTagAction_f, &dbg_epe_hdr_adj_info, &adj_cTagAction);
    GetDbgEpeHdrAdjInfo(A, packetOffset_f, &dbg_epe_hdr_adj_info, &adj_packetOffset);
    GetDbgEpeHdrAdjInfo(A, isidValid_f, &dbg_epe_hdr_adj_info, &adj_isidValid);
    GetDbgEpeHdrAdjInfo(A, muxLengthType_f, &dbg_epe_hdr_adj_info, &adj_muxLengthType);
    GetDbgEpeHdrAdjInfo(A, cvlanTagOperationValid_f, &dbg_epe_hdr_adj_info, &adj_cvlanTagOperationValid);
    GetDbgEpeHdrAdjInfo(A, svlanTagOperationValid_f, &dbg_epe_hdr_adj_info, &adj_svlanTagOperationValid);
     /*GetDbgEpeHdrAdjInfo(A, debugOpType_f, &dbg_epe_hdr_adj_info, &adj_debugOpType);*/
    GetDbgEpeHdrAdjInfo(A, valid_f, &dbg_epe_hdr_adj_info, &adj_valid);
    /*DbgEpeNextHopInfo*/
    sal_memset(&dbg_epe_next_hop_info, 0, sizeof(dbg_epe_next_hop_info));
    cmd = DRV_IOR(DbgEpeNextHopInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_epe_next_hop_info);
    GetDbgEpeNextHopInfo(A, discard_f, &dbg_epe_next_hop_info, &nhp_discard);
    GetDbgEpeNextHopInfo(A, discardType_f, &dbg_epe_next_hop_info, &nhp_discardType);
    GetDbgEpeNextHopInfo(A, payloadOperation_f, &dbg_epe_next_hop_info, &nhp_payloadOperation);
    GetDbgEpeNextHopInfo(A, editBypass_f, &dbg_epe_next_hop_info, &nhp_editBypass);
    GetDbgEpeNextHopInfo(A, innerEditHashEn_f, &dbg_epe_next_hop_info, &nhp_innerEditHashEn);
    GetDbgEpeNextHopInfo(A, outerEditHashEn_f, &dbg_epe_next_hop_info, &nhp_outerEditHashEn);
    GetDbgEpeNextHopInfo(A, etherOamDiscard_f, &dbg_epe_next_hop_info, &nhp_etherOamDiscard);
     /*GetDbgEpeNextHopInfo(A, isVxlanRouteOp_f, &dbg_epe_next_hop_info, &nhp_isVxlanRouteOp);*/
    GetDbgEpeNextHopInfo(A, exceptionEn_f, &dbg_epe_next_hop_info, &nhp_exceptionEn);
    GetDbgEpeNextHopInfo(A, exceptionIndex_f, &dbg_epe_next_hop_info, &nhp_exceptionIndex);
    GetDbgEpeNextHopInfo(A, portReflectiveDiscard_f, &dbg_epe_next_hop_info, &nhp_portReflectiveDiscard);
    GetDbgEpeNextHopInfo(A, interfaceId_f, &dbg_epe_next_hop_info, &nhp_interfaceId);
    GetDbgEpeNextHopInfo(A, destVlanPtr_f, &dbg_epe_next_hop_info, &nhp_destVlanPtr);
     /*GetDbgEpeNextHopInfo(A, mcast_f, &dbg_epe_next_hop_info, &nhp_mcast);*/
    GetDbgEpeNextHopInfo(A, routeNoL2Edit_f, &dbg_epe_next_hop_info, &nhp_routeNoL2Edit);
    GetDbgEpeNextHopInfo(A, outputSvlanIdValid_f, &dbg_epe_next_hop_info, &nhp_outputSvlanIdValid);
    GetDbgEpeNextHopInfo(A, outputSvlanId_f, &dbg_epe_next_hop_info, &nhp_outputSvlanId);
    GetDbgEpeNextHopInfo(A, logicDestPort_f, &dbg_epe_next_hop_info, &nhp_logicDestPort);
    GetDbgEpeNextHopInfo(A, outputCvlanIdValid_f, &dbg_epe_next_hop_info, &nhp_outputCvlanIdValid);
    GetDbgEpeNextHopInfo(A, outputCvlanId_f, &dbg_epe_next_hop_info, &nhp_outputCvlanId);
     /*GetDbgEpeNextHopInfo(A, _priority_f, &dbg_epe_next_hop_info, &nhp__priority);*/
     /*GetDbgEpeNextHopInfo(A, color_f, &dbg_epe_next_hop_info, &nhp_color);*/
    GetDbgEpeNextHopInfo(A, l3EditDisable_f, &dbg_epe_next_hop_info, &nhp_l3EditDisable);
    GetDbgEpeNextHopInfo(A, l2EditDisable_f, &dbg_epe_next_hop_info, &nhp_l2EditDisable);
    GetDbgEpeNextHopInfo(A, isEnd_f, &dbg_epe_next_hop_info, &nhp_isEnd);
    GetDbgEpeNextHopInfo(A, bypassAll_f, &dbg_epe_next_hop_info, &nhp_bypassAll);
    GetDbgEpeNextHopInfo(A, nhpIs8w_f, &dbg_epe_next_hop_info, &nhp_nhpIs8w);
    GetDbgEpeNextHopInfo(A, portIsolateValid_f, &dbg_epe_next_hop_info, &nhp_portIsolateValid);
    GetDbgEpeNextHopInfo(A, lookup1Valid_f, &dbg_epe_next_hop_info, &nhp_lookup1Valid);/*DUET2,and not used*/
    GetDbgEpeNextHopInfo(A, vlanHash1Type_f, &dbg_epe_next_hop_info, &nhp_vlanHash1Type);/*DUET2,and not used*/
    GetDbgEpeNextHopInfo(A, lookup2Valid_f, &dbg_epe_next_hop_info, &nhp_lookup2Valid);/*DUET2,and not used*/
    GetDbgEpeNextHopInfo(A, vlanHash2Type_f, &dbg_epe_next_hop_info, &nhp_vlanHash2Type);/*DUET2,and not used*/
    GetDbgEpeNextHopInfo(A, vlanXlateResult1Valid_f, &dbg_epe_next_hop_info, &nhp_vlanXlateResult1Valid);/*DUET2,and not used*/
    GetDbgEpeNextHopInfo(A, vlanXlateResult2Valid_f, &dbg_epe_next_hop_info, &nhp_vlanXlateResult2Valid);/*DUET2,and not used*/
    GetDbgEpeNextHopInfo(A, hashPort_f, &dbg_epe_next_hop_info, &nhp_hashPort);/*DUET2,and not used*/
    GetDbgEpeNextHopInfo(A, innerEditPtrOffset_f, &dbg_epe_next_hop_info, &nhp_innerEditPtrOffset);
    GetDbgEpeNextHopInfo(A, outerEditPtrOffset_f, &dbg_epe_next_hop_info, &nhp_outerEditPtrOffset);
    GetDbgEpeNextHopInfo(A, valid_f, &dbg_epe_next_hop_info, &nhp_valid);
    GetDbgEpeNextHopInfo(A, payloadOperation_f, &dbg_epe_next_hop_info, &pld_payloadOperation);
    GetDbgEpeNextHopInfo(A, nhpInnerEditPtr_f, &dbg_epe_next_hop_info, &nhp_innerEditPtr);
    GetDbgEpeNextHopInfo(A, nhpOuterEditPtr_f, &dbg_epe_next_hop_info, &nhp_outerEditPtr);
    GetDbgEpeNextHopInfo(A, nhpOuterEditLocation_f, &dbg_epe_next_hop_info, &edit_nhpOuterEditLocation);
    /*DbgEpePayLoadInfo*/
    sal_memset(&dbg_epe_pay_load_info, 0, sizeof(dbg_epe_pay_load_info));
    cmd = DRV_IOR(DbgEpePayLoadInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_epe_pay_load_info);
    GetDbgEpePayLoadInfo(A, discard_f, &dbg_epe_pay_load_info, &pld_discard);
    GetDbgEpePayLoadInfo(A, discardType_f, &dbg_epe_pay_load_info, &pld_discardType);
    GetDbgEpePayLoadInfo(A, exceptionEn_f, &dbg_epe_pay_load_info, &pld_exceptionEn);
    GetDbgEpePayLoadInfo(A, exceptionIndex_f, &dbg_epe_pay_load_info, &pld_exceptionIndex);
     /*GetDbgEpePayLoadInfo(A, shareType_f, &dbg_epe_pay_load_info, &pld_shareType);*/
     /*GetDbgEpePayLoadInfo(A, fid_f, &dbg_epe_pay_load_info, &pld_fid);*/

    GetDbgEpePayLoadInfo(A, newTtlValid_f, &dbg_epe_pay_load_info, &pld_newTtlValid);
    GetDbgEpePayLoadInfo(A, newTtl_f, &dbg_epe_pay_load_info, &pld_newTtl);
    GetDbgEpePayLoadInfo(A, newDscpValid_f, &dbg_epe_pay_load_info, &pld_newDscpValid);
     /*GetDbgEpePayLoadInfo(A, replaceDscp_f, &dbg_epe_pay_load_info, &pld_replaceDscp);*/
     /*GetDbgEpePayLoadInfo(A, useOamTtlOrExp_f, &dbg_epe_pay_load_info, &pld_useOamTtlOrExp);*/
     /*GetDbgEpePayLoadInfo(A, copyDscp_f, &dbg_epe_pay_load_info, &pld_copyDscp);*/
    GetDbgEpePayLoadInfo(A, newDscp_f, &dbg_epe_pay_load_info, &pld_newDscp);
    GetDbgEpePayLoadInfo(A, svlanTagOperation_f, &dbg_epe_pay_load_info, &pld_svlanTagOperation);
    GetDbgEpePayLoadInfo(A, l2NewSvlanTag_f, &dbg_epe_pay_load_info, &pld_l2NewSvlanTag);
    GetDbgEpePayLoadInfo(A, cvlanTagOperation_f, &dbg_epe_pay_load_info, &pld_cvlanTagOperation);
    GetDbgEpePayLoadInfo(A, l2NewCvlanTag_f, &dbg_epe_pay_load_info, &pld_l2NewCvlanTag);
    GetDbgEpePayLoadInfo(A, stripOffset_f, &dbg_epe_pay_load_info, &pld_stripOffset);
     /*GetDbgEpePayLoadInfo(A, mirroredPacket_f, &dbg_epe_pay_load_info, &pld_mirroredPacket);*/
     /*GetDbgEpePayLoadInfo(A, isL2Ptp_f, &dbg_epe_pay_load_info, &pld_isL2Ptp);*/
     /*GetDbgEpePayLoadInfo(A, isL4Ptp_f, &dbg_epe_pay_load_info, &pld_isL4Ptp);*/
    GetDbgEpePayLoadInfo(A, valid_f, &dbg_epe_pay_load_info, &pld_valid);
    /*DbgEpeEgressEditInfo*/
    sal_memset(&dbg_epe_egress_edit_info, 0, sizeof(dbg_epe_egress_edit_info));
    cmd = DRV_IOR(DbgEpeEgressEditInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_epe_egress_edit_info);
    GetDbgEpeEgressEditInfo(A, discard_f, &dbg_epe_egress_edit_info, &edit_discard);
    GetDbgEpeEgressEditInfo(A, discardType_f, &dbg_epe_egress_edit_info, &edit_discardType);
    GetDbgEpeEgressEditInfo(A, innerEditExist_f, &dbg_epe_egress_edit_info, &edit_innerEditExist);
    GetDbgEpeEgressEditInfo(A, innerEditPtrType_f, &dbg_epe_egress_edit_info, &edit_innerEditPtrType);
    GetDbgEpeEgressEditInfo(A, outerEditPipe1Exist_f, &dbg_epe_egress_edit_info, &edit_outerEditPipe1Exist);
    GetDbgEpeEgressEditInfo(A, outerEditPipe1Type_f, &dbg_epe_egress_edit_info, &edit_outerEditPipe1Type);
    GetDbgEpeEgressEditInfo(A, outerEditPipe2Exist_f, &dbg_epe_egress_edit_info, &edit_outerEditPipe2Exist);
    GetDbgEpeEgressEditInfo(A, outerEditPipe3Exist_f, &dbg_epe_egress_edit_info, &edit_outerEditPipe3Exist);
    GetDbgEpeEgressEditInfo(A, outerEditPipe3Type_f, &dbg_epe_egress_edit_info, &edit_outerEditPipe3Type);
    GetDbgEpeEgressEditInfo(A, l3RewriteType_f, &dbg_epe_egress_edit_info, &edit_l3RewriteType);
    GetDbgEpeEgressEditInfo(A, l3EditMplsPwExist_f, &dbg_epe_egress_edit_info, &edit_l3EditMplsPwExist);
    GetDbgEpeEgressEditInfo(A, l3EditMplsLspExist_f, &dbg_epe_egress_edit_info, &edit_l3EditMplsLspExist);
    GetDbgEpeEgressEditInfo(A, l3EditMplsSpmeExist_f, &dbg_epe_egress_edit_info, &edit_l3EditMplsSpmeExist);
    GetDbgEpeEgressEditInfo(A, innerL2RewriteType_f, &dbg_epe_egress_edit_info, &edit_innerL2RewriteType);
    GetDbgEpeEgressEditInfo(A, l2RewriteType_f, &dbg_epe_egress_edit_info, &edit_l2RewriteType);
    GetDbgEpeEgressEditInfo(A, valid_f, &dbg_epe_egress_edit_info, &edit_valid);
    /*DbgEpeInnerL2EditInfo*/
    sal_memset(&dbg_epe_inner_l2_edit_info, 0, sizeof(dbg_epe_inner_l2_edit_info));
    cmd = DRV_IOR(DbgEpeInnerL2EditInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_epe_inner_l2_edit_info);
    GetDbgEpeInnerL2EditInfo(A, valid_f, &dbg_epe_inner_l2_edit_info, &innerL2_valid);
    GetDbgEpeInnerL2EditInfo(A, isInnerL2EthSwapOp_f, &dbg_epe_inner_l2_edit_info, &innerL2_isInnerL2EthSwapOp);
    GetDbgEpeInnerL2EditInfo(A, isInnerL2EthAddOp_f, &dbg_epe_inner_l2_edit_info, &innerL2_isInnerL2EthAddOp);
    GetDbgEpeInnerL2EditInfo(A, isInnerDsLiteOp_f, &dbg_epe_inner_l2_edit_info, &innerL2_isInnerDsLiteOp);
    /*DbgEpeOuterL2EditInfo*/
    sal_memset(&dbg_epe_outer_l2_edit_info, 0, sizeof(dbg_epe_outer_l2_edit_info));
    cmd = DRV_IOR(DbgEpeOuterL2EditInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_epe_outer_l2_edit_info);
    GetDbgEpeOuterL2EditInfo(A, l2EditDisable_f, &dbg_epe_outer_l2_edit_info, &l2edit_l2EditDisable);
    GetDbgEpeOuterL2EditInfo(A, routeNoL2Edit_f, &dbg_epe_outer_l2_edit_info, &l2edit_routeNoL2Edit);
     /*GetDbgEpeOuterL2EditInfo(A, destMuxPortType_f, &dbg_epe_outer_l2_edit_info, &l2edit_destMuxPortType);*/
    GetDbgEpeOuterL2EditInfo(A, newMacSaValid_f, &dbg_epe_outer_l2_edit_info, &l2edit_newMacSaValid);
    GetDbgEpeOuterL2EditInfo(A, newMacDaValid_f, &dbg_epe_outer_l2_edit_info, &l2edit_newMacDaValid);
     /*GetDbgEpeOuterL2EditInfo(A, portMacSaEn_f, &dbg_epe_outer_l2_edit_info, &l2edit_portMacSaEn);*/
     /*GetDbgEpeOuterL2EditInfo(A, loopbackEn_f, &dbg_epe_outer_l2_edit_info, &l2edit_loopbackEn);*/
    GetDbgEpeOuterL2EditInfo(A, svlanTagOperation_f, &dbg_epe_outer_l2_edit_info, &l2edit_svlanTagOperation);
    GetDbgEpeOuterL2EditInfo(A, cvlanTagOperation_f, &dbg_epe_outer_l2_edit_info, &l2edit_cvlanTagOperation);
    GetDbgEpeOuterL2EditInfo(A, l2RewriteType_f, &dbg_epe_outer_l2_edit_info, &l2edit_l2RewriteType);
    GetDbgEpeOuterL2EditInfo(A, isEnd_f, &dbg_epe_outer_l2_edit_info, &l2edit_isEnd);
     /*GetDbgEpeOuterL2EditInfo(A, len_f, &dbg_epe_outer_l2_edit_info, &l2edit_len);*/
    GetDbgEpeOuterL2EditInfo(A, valid_f, &dbg_epe_outer_l2_edit_info, &l2edit_valid);
    /*DbgEpeL3EditInfo*/
    sal_memset(&dbg_epe_l3_edit_info, 0, sizeof(dbg_epe_l3_edit_info));
    cmd = DRV_IOR(DbgEpeL3EditInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_epe_l3_edit_info);
    GetDbgEpeL3EditInfo(A, l3EditDisable_f, &dbg_epe_l3_edit_info, &l3edit_l3EditDisable);
    GetDbgEpeL3EditInfo(A, l3RewriteType_f, &dbg_epe_l3_edit_info, &l3edit_l3RewriteType);
    GetDbgEpeL3EditInfo(A, label0Valid_f, &dbg_epe_l3_edit_info, &l3edit_label0Valid);
    GetDbgEpeL3EditInfo(A, label1Valid_f, &dbg_epe_l3_edit_info, &l3edit_label1Valid);
    GetDbgEpeL3EditInfo(A, label2Valid_f, &dbg_epe_l3_edit_info, &l3edit_label2Valid);
    GetDbgEpeL3EditInfo(A, isLspLabelEntropy0_f, &dbg_epe_l3_edit_info, &l3edit_isLspLabelEntropy0);
    GetDbgEpeL3EditInfo(A, isLspLabelEntropy1_f, &dbg_epe_l3_edit_info, &l3edit_isLspLabelEntropy1);
    GetDbgEpeL3EditInfo(A, isLspLabelEntropy2_f, &dbg_epe_l3_edit_info, &l3edit_isLspLabelEntropy2);
    GetDbgEpeL3EditInfo(A, isPwLabelEntropy0_f, &dbg_epe_l3_edit_info, &l3edit_isPwLabelEntropy0);
    GetDbgEpeL3EditInfo(A, isPwLabelEntropy1_f, &dbg_epe_l3_edit_info, &l3edit_isPwLabelEntropy1);
    GetDbgEpeL3EditInfo(A, isPwLabelEntropy2_f, &dbg_epe_l3_edit_info, &l3edit_isPwLabelEntropy2);
    GetDbgEpeL3EditInfo(A, cwLabelValid_f, &dbg_epe_l3_edit_info, &l3edit_cwLabelValid);
    GetDbgEpeL3EditInfo(A, needDot1AeEncrypt_f, &dbg_epe_l3_edit_info, &l3edit_needDot1aeEncrypt);
    GetDbgEpeL3EditInfo(A, dot1AeSaIndexBase_f, &dbg_epe_l3_edit_info, &l3edit_dot1aeSaIndexBase);
    GetDbgEpeL3EditInfo(A, dot1AeSaIndex_f, &dbg_epe_l3_edit_info, &l3edit_dot1aeSaIndex);
    GetDbgEpeL3EditInfo(A, isEnd_f, &dbg_epe_l3_edit_info, &l3edit_isEnd);
    GetDbgEpeL3EditInfo(A, valid_f, &dbg_epe_l3_edit_info, &l3edit_valid);

    if (0 == p_info->detail)
    {
        goto Discard;
    }

    CTC_DKIT_PRINT("Edit Process:\n");
    if (hdr_valid&&hdr_stripOffset)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%d bytes\n", "strip packet", hdr_stripOffset);
    }

    if (pld_valid)
    {

        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "pld Operation", pld_payloadOperation);
        if (pld_svlanTagOperation || pld_cvlanTagOperation)
        {
            CTC_DKIT_PATH_PRINT("%-15s\n", "vlan edit on pld operation--->");
            no_l2_edit = 0;
            if (pld_svlanTagOperation)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "svlan edit", vlan_action[pld_svlanTagOperation]);
                if ((VTAGACTIONTYPE_MODIFY == pld_svlanTagOperation) || (VTAGACTIONTYPE_ADD == pld_svlanTagOperation))
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new COS", (pld_l2NewSvlanTag >> 13)&0x7);
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new CFI", (pld_l2NewSvlanTag >> 12)&0x1);
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new vlan id", pld_l2NewSvlanTag&0xFFF);
                }
            }
            if (pld_cvlanTagOperation)
            {
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "cvlan edit", vlan_action[pld_cvlanTagOperation]);
                if ((VTAGACTIONTYPE_MODIFY == pld_cvlanTagOperation) || (VTAGACTIONTYPE_ADD == pld_cvlanTagOperation))
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new COS", (pld_l2NewCvlanTag >> 13)&0x7);
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new CFI", (pld_l2NewCvlanTag >> 12)&0x1);
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new vlan id", pld_l2NewCvlanTag&0xFFF);
                }
            }
        }
        if (pld_newTtlValid)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new TTL", pld_newTtl);
        }
        if (pld_newDscpValid)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new DSCP", pld_newDscp);
        }
    }

    cmd = DRV_IOR(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_dsNextHopInternalBase_f);
    DRV_IOCTL(lchip, 0, cmd, &dsNextHopInternalBase);

    if ((adj_nextHopPtr >> 4) != dsNextHopInternalBase)
    {
        cmd = DRV_IOR(DsNextHop8W_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, (adj_nextHopPtr >> 1) << 1, cmd, &ds_next_hop8_w);

        if (!nhp_nhpIs8w)
        {
            if (IS_BIT_SET((adj_nextHopPtr), 0))
            {
                sal_memcpy((uint8*)&ds_next_hop4_w, (uint8*)&ds_next_hop8_w + sizeof(ds_next_hop4_w), sizeof(ds_next_hop4_w));
            }
            else
            {
                sal_memcpy((uint8*)&ds_next_hop4_w, (uint8*)&ds_next_hop8_w , sizeof(ds_next_hop4_w));
            }
        }
    }
    else
    {
        cmd = DRV_IOR(EpeNextHopInternal_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, ((adj_nextHopPtr&0xF) >> 1), cmd, &ds_next_hop8_w);
        if (!nhp_nhpIs8w)
        {
            if (IS_BIT_SET((adj_nextHopPtr&0xF), 0))
            {
                sal_memcpy((uint8*)&ds_next_hop4_w, (uint8*)&ds_next_hop8_w + sizeof(ds_next_hop4_w), sizeof(ds_next_hop4_w));
            }
            else
            {
                sal_memcpy((uint8*)&ds_next_hop4_w, (uint8*)&ds_next_hop8_w , sizeof(ds_next_hop4_w));
            }
        }
    }

    if(l3edit_needDot1aeEncrypt)
    {
        step = EpeDot1AeAnCtl0_array_1_currentAn_f - EpeDot1AeAnCtl0_array_0_currentAn_f;
        cmd = DRV_IOR(EpeDot1AeAnCtl0_t, EpeDot1AeAnCtl0_array_0_currentAn_f + step * l3edit_dot1aeSaIndexBase);
        DRV_IOCTL(lchip, 0, cmd, &dot1ae_current_an);
        step = EpeDot1AeAnCtl1_array_1_nextAn_f - EpeDot1AeAnCtl1_array_0_nextAn_f;
        cmd = DRV_IOR(EpeDot1AeAnCtl1_t, EpeDot1AeAnCtl1_array_0_nextAn_f + step * l3edit_dot1aeSaIndexBase);
        DRV_IOCTL(lchip, 0, cmd, &dot1ae_next_an);
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_dot1AeSciEn_f);
        DRV_IOCTL(lchip, adj_localPhyPort, cmd, &dot1ae_sci_en);
        cmd = DRV_IOR(DsEgressDot1AePn_t, DsEgressDot1AePn_pn_f);
        DRV_IOCTL(lchip, l3edit_dot1aeSaIndex, cmd, &dot1ae_pn);
        cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AePnThreshold_f);
        DRV_IOCTL(lchip, 0, cmd, &dot1ae_pn_thrd);
        cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AeTciEbitCbit_f);
        DRV_IOCTL(lchip, 0, cmd, &dot1ae_ebit_cbit);
        CTC_DKIT_PATH_PRINT("%-15s\n", "Dot1AE Edit--->");
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Current AN", dot1ae_current_an);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Next AN", dot1ae_next_an);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Current PN", dot1ae_pn - 1);
        CTC_DKIT_PATH_PRINT("%-15s:%"PRIu64"\n", "PN Threhold", dot1ae_pn_thrd);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Tx SCI En", dot1ae_sci_en);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Ebit Cbit",
                            CTC_DKIT_IS_BIT_SET(dot1ae_ebit_cbit[l3edit_dot1aeSaIndex/32], l3edit_dot1aeSaIndex%32));
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "SA Index Base", l3edit_dot1aeSaIndexBase);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "SA Index", l3edit_dot1aeSaIndex);
        no_l3_edit = 0;
    }

    if (nhp_nhpIs8w)
    {
        GetDsNextHop8W(A, shareType_f, &ds_next_hop8_w, &nhpShareType);
    }
    else
    {
        GetDsNextHop4W(A, shareType_f, &ds_next_hop4_w, &nhpShareType);
    }

    if ((PAYLOADOPERATION_ROUTE_COMPACT == pld_payloadOperation)
        ||((PAYLOADOPERATION_NONE == pld_payloadOperation)&&nhp_routeNoL2Edit))
    {
        cmd = DRV_IOR( DbgFwdMetFifoInfo1_t,  DbgFwdMetFifoInfo1_destMap_f);
        DRV_IOCTL(lchip, 0, cmd, &dest_map);
        if (!IS_BIT_SET(dest_map,18))
        {
            GetDsNextHop4W(A, macDa_f, &ds_next_hop4_w, hw_mac);
            CTC_DKIT_SET_USER_MAC(mac_da, hw_mac);
            if (IS_BIT_SET(mac_da[0], 0))/* mcast mode */
            {

            }
            else
            {
                CTC_DKIT_PATH_PRINT("%-15s\n", "MAC edit--->");
                CTC_DKIT_PATH_PRINT("%-15s:%02X%02X.%02X%02X.%02X%02X\n", "new MAC DA",
                                    mac_da[0], mac_da[1], mac_da[2], mac_da[3], mac_da[4], mac_da[5]);
            }
            no_l2_edit = 0;
        }
    }
    else if ((NHPSHARETYPE_L2EDIT_PLUS_L3EDIT_OP == nhpShareType) || (nhp_nhpIs8w))
    {
        if (nhp_nhpIs8w)
        {
            GetDsNextHop8W(A, outerEditPtrType_f, &ds_next_hop8_w, &nhpOuterEditPtrType);
        }
        else
        {
            GetDsNextHop4W(A, outerEditPtrType_f, &ds_next_hop4_w, &nhpOuterEditPtrType);
        }
        /*inner edit*/
        if (edit_valid && edit_innerEditExist && (!edit_innerEditPtrType))/* inner for l2edit */
        {
            if (innerL2_valid)
            {
                CTC_DKIT_PATH_PRINT("%-15s\n", "Inner MAC edit--->");
                CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "l2edit index", nhp_innerEditPtr);
                if (innerL2_isInnerL2EthSwapOp)
                {
                    uint32 replaceInnerMacDa;
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", "DsL2EditInnerSwap");
                    cmd = DRV_IOR(DsL2EditInnerSwap_t, DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, nhp_innerEditPtr, cmd, &ds_l2_edit_inner_swap);
                    GetDsL2EditInnerSwap(A, replaceInnerMacDa_f, &ds_l2_edit_inner_swap, &replaceInnerMacDa);
                    if (replaceInnerMacDa)
                    {
                        GetDsL2EditInnerSwap(A, macDa_f, &ds_l2_edit_inner_swap, hw_mac);
                        CTC_DKIT_SET_USER_MAC(mac_da, hw_mac);
                        CTC_DKIT_PATH_PRINT("%-15s:%02X%02X.%02X%02X.%02X%02X\n", "new MAC DA",
                                            mac_da[0], mac_da[1], mac_da[2], mac_da[3], mac_da[4], mac_da[5]);
                    }
                }
                else if (innerL2_isInnerL2EthAddOp)
                {
                    if (L2REWRITETYPE_ETH8W == edit_innerL2RewriteType)
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", "DsL2EditEth6W");
                    }
                    else if (L2REWRITETYPE_ETH4W == edit_innerL2RewriteType)
                    {
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", "DsL2EditEth3W");
                    }
                }
                else if (innerL2_isInnerDsLiteOp)
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", "DsL2EditDsLite");
                }
            }
            else if (l2edit_valid)
            {
                do_l2_edit = 1;
                l2_edit_ptr = nhp_innerEditPtr;
                l2_individual_memory = 0;
            }
            no_l2_edit = 0;
        }
        else if (edit_valid && edit_innerEditExist && (edit_innerEditPtrType))/* inner for l3edit */
        {
            if (l3edit_valid)
            {
                if (L3REWRITETYPE_MPLS4W == l3edit_l3RewriteType)
                {
                    cmd = DRV_IOR(DsL3EditMpls3W_t, DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, nhp_innerEditPtr, cmd, &ds_l3_edit_mpls3_w);
                    GetDsL3EditMpls3W(A, label_f, &ds_l3_edit_mpls3_w, &value);
                    CTC_DKIT_PATH_PRINT("%-15s\n", "MPLS edit--->");
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "l3edit index", nhp_innerEditPtr);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", "DsL3EditMpls3W");
                    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new label", value);
                    no_l3_edit = 0;
                }
                else if (L3REWRITETYPE_NONE != l3edit_l3RewriteType)
                {
                    CTC_DKIT_PATH_PRINT("%-15s\n", l3_edit[l3edit_l3RewriteType]);
                    CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "l3edit index", nhp_innerEditPtr);
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", l3_edit_name[l3edit_l3RewriteType]);
                    no_l3_edit = 0;
                }
            }
        }

        /*outer pipeline1*/
        if (edit_valid && edit_outerEditPipe1Exist)
        {
            if (l3edit_valid && nhpOuterEditPtrType)/* l3edit */
            {
                if (edit_nhpOuterEditLocation)/* individual memory */
                {
                    /* no use */
                }
                else/* share memory */
                {
                    cmd = DRV_IOR(DsL3EditMpls3W_t, DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, nhp_outerEditPtr, cmd, &ds_l3_edit_mpls3_w);
                    /* outerEditPtr and outerEditPtrType are in the same position in all l3edit, refer to DsL3Edit3WTemplate  */
                    GetDsL3EditMpls3W(A, outerEditPtr_f, &ds_l3_edit_mpls3_w, &next_edit_ptr);
                    GetDsL3EditMpls3W(A, outerEditPtrType_f, &ds_l3_edit_mpls3_w, &next_edit_type);
                    if (L3REWRITETYPE_MPLS4W == l3edit_l3RewriteType)
                    {
                        GetDsL3EditMpls3W(A, label_f, &ds_l3_edit_mpls3_w, &value);
                        CTC_DKIT_PATH_PRINT("%-15s\n", "MPLS edit--->");
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "l3edit index", nhp_outerEditPtr);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", "DsL3EditMpls3W");
                        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new label", value);
                        no_l3_edit = 0;
                    }
                    else if (L3REWRITETYPE_NONE != l3edit_l3RewriteType)
                    {
                        CTC_DKIT_PATH_PRINT("%-15s\n", l3_edit[l3edit_l3RewriteType]);
                        CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "l3edit index", nhp_outerEditPtr);
                        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", l3_edit_name[l3edit_l3RewriteType]);
                        no_l3_edit = 0;
                    }
                }
            }
            else if (l2edit_valid)/* l2edit */
            {
                do_l2_edit = 1;
                l2_edit_ptr = nhp_outerEditPtr;
                l2_individual_memory = edit_nhpOuterEditLocation? 1 : 0;
            }
        }
        /*outer pipeline2*/
        if (edit_valid && edit_outerEditPipe2Exist)/* outer pipeline2, individual memory for SPME */
        {
            if (l3edit_valid && next_edit_type)/* l3edit */
            {
                cmd = DRV_IOR(DsL3Edit6W3rd_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, next_edit_ptr/2, cmd, &ds_l3_edit_6w_3rd);
                if (IS_BIT_SET((next_edit_ptr), 0))
                {
                    sal_memcpy((uint8*)&ds_l3_edit_mpls3_w, (uint8*)&ds_l3_edit_6w_3rd + sizeof(ds_l3_edit_mpls3_w), sizeof(ds_l3_edit_mpls3_w));
                }
                else
                {
                    sal_memcpy((uint8*)&ds_l3_edit_mpls3_w, (uint8*)&ds_l3_edit_6w_3rd , sizeof(ds_l3_edit_mpls3_w));
                }
                GetDsL3EditMpls3W(A, label_f, &ds_l3_edit_mpls3_w, &value);
                CTC_DKIT_PATH_PRINT("%-15s\n", "MPLS edit--->");
                CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "l3edit index", next_edit_ptr/2);
                CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", "DsL3Edit6W3rd");
                CTC_DKIT_PATH_PRINT("%-15s:%d\n", "new label", value);
                GetDsL3EditMpls3W(A, outerEditPtr_f, &ds_l3_edit_mpls3_w, &next_edit_ptr);
                GetDsL3EditMpls3W(A, outerEditPtrType_f, &ds_l3_edit_mpls3_w, &next_edit_type);
                no_l3_edit = 0;
            }
            else if (l2edit_valid)/* l2edit */
            {
                do_l2_edit = 1;
                if (!edit_outerEditPipe1Exist)
                {
                    l2_edit_ptr = nhp_outerEditPtr;
                }
                else
                {
                    l2_edit_ptr = next_edit_ptr;
                }
                l2_individual_memory = 1;
            }
        }
        /*outer pipeline3*/
        if (l2edit_valid && edit_valid && edit_outerEditPipe3Exist)/* outer pipeline3, individual memory for l2edit */
        {
            do_l2_edit = 1;
            if (!edit_outerEditPipe1Exist)
            {
                l2_edit_ptr = nhp_outerEditPtr;
            }
            else
            {
                l2_edit_ptr = next_edit_ptr;
            }
            l2_individual_memory = 1;
        }


    }
    else if (NHPSHARETYPE_L2EDIT_PLUS_STAG_OP == nhpShareType)
    {
        if (l2edit_valid)/* l2edit */
        {
            do_l2_edit = 1;
            l2_edit_ptr = nhp_outerEditPtr;
            l2_individual_memory = edit_nhpOuterEditLocation? 1 : 0;
        }
    }

    if (do_l2_edit)
    {
        if (l2_individual_memory)/* individual memory */
        {
            cmd = DRV_IOR(DsL2Edit6WOuter_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, (l2_edit_ptr >> 1), cmd, &ds_l2_edit6_w_outer);
            if (L2REWRITETYPE_ETH4W == l2edit_l2RewriteType)
            {
                if (l2_edit_ptr % 2 == 0)
                {
                    sal_memcpy((uint8*)&ds_l2_edit_eth3_w, (uint8*)&ds_l2_edit6_w_outer, sizeof(ds_l2_edit_eth3_w));
                }
                else
                {
                    sal_memcpy((uint8*)&ds_l2_edit_eth3_w, (uint8*)&ds_l2_edit6_w_outer + sizeof(ds_l2_edit_eth3_w), sizeof(ds_l2_edit_eth3_w));
                }
            }
            else if (L2REWRITETYPE_MAC_SWAP == l2edit_l2RewriteType)
            {
                sal_memcpy((uint8*)&ds_l2_edit_swap, (uint8*)&ds_l2_edit6_w_outer, sizeof(ds_l2_edit_swap));
            }
        }
        else/* share memory */
        {
            if (L2REWRITETYPE_ETH4W == l2edit_l2RewriteType)
            {
                cmd = DRV_IOR(DsL2EditEth3W_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, l2_edit_ptr, cmd, &ds_l2_edit_eth3_w);

            }
            else if (L2REWRITETYPE_MAC_SWAP == l2edit_l2RewriteType)
            {
                cmd = DRV_IOR(DsL2EditSwap_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, l2_edit_ptr, cmd, &ds_l2_edit_swap);
            }
        }

        if (L2REWRITETYPE_ETH4W == l2edit_l2RewriteType)
        {
            GetDsL2EditEth3W(A, macDa_f, &ds_l2_edit_eth3_w, hw_mac);
            CTC_DKIT_SET_USER_MAC(mac_da, hw_mac);
            CTC_DKIT_PATH_PRINT("%-15s\n", "MAC edit--->");
            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "l2edit index", l2_edit_ptr);
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", l2_individual_memory ? "DsL2Edit3WOuter" : "DsL2EditEth3W");
            CTC_DKIT_PATH_PRINT("%-15s:%02X%02X.%02X%02X.%02X%02X\n", "new MAC DA",
                                mac_da[0], mac_da[1], mac_da[2], mac_da[3], mac_da[4], mac_da[5]);
            no_l2_edit = 0;
        }
        else if (L2REWRITETYPE_MAC_SWAP == l2edit_l2RewriteType)
        {
            uint32 deriveMcastMac;
            uint32 _type = 0;
            uint32 outputVlanIdValid = 0;
            uint32 overwriteEtherType = 0;
            GetDsL2EditSwap(A, deriveMcastMac_f, &ds_l2_edit_swap, &deriveMcastMac);
            GetDsL2EditSwap(A, _type_f, &ds_l2_edit_swap, &_type);
            GetDsL2EditSwap(A, outputVlanIdValid_f, &ds_l2_edit_swap, &outputVlanIdValid);
            GetDsL2EditSwap(A, overwriteEtherType_f, &ds_l2_edit_swap, &overwriteEtherType);

            CTC_DKIT_PATH_PRINT("%-15s\n", "MAC swap--->");
            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "l2edit index", l2_edit_ptr);
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "table name", l2_individual_memory? "DsL2Edit3WOuter" : "DsL2EditSwap");
            if (deriveMcastMac)
            {
                if (_type)
                {
                    GetDsL2EditSwap(A, macDa_f, &ds_l2_edit_eth3_w, hw_mac);
                    CTC_DKIT_SET_USER_MAC(mac_da, hw_mac);
                    CTC_DKIT_PATH_PRINT("%-15s:%02X%02X.%02X%02X.%02X%02X\n", "new MAC DA",
                                        mac_da[0], mac_da[1], mac_da[2], mac_da[3], mac_da[4], mac_da[5]);
                }
                else
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "new MAC DA", "MAC SA of the packet");
                }
            }
            if (outputVlanIdValid)
            {
                if (overwriteEtherType)
                {
                    GetDsL2EditSwap(A, macSa_f, &ds_l2_edit_eth3_w, hw_mac);
                    CTC_DKIT_SET_USER_MAC(mac_da, hw_mac);
                    CTC_DKIT_PATH_PRINT("%-15s:%02X%02X.%02X%02X.%02X%02X\n", "new MAC SA",
                                        mac_da[0], mac_da[1], mac_da[2], mac_da[3], mac_da[4], mac_da[5]);
                }
                else
                {
                    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "new MAC SA", "MAC DA of the packet");
                }
            }
            no_l2_edit = 0;
        }
        else if (L2REWRITETYPE_NONE != l2edit_l2RewriteType)
        {
            CTC_DKIT_PATH_PRINT("%-15s\n", "L2edit--->");
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "L2edit type", l2edit_l2RewriteType);
            CTC_DKIT_PATH_PRINT("%-15s:0x%x\n", "l2edit index", l2_edit_ptr);
            no_l2_edit = 0;
        }
    }


    if (no_l3_edit)
    {
        CTC_DKIT_PATH_PRINT("%-15s\n", "not do L3 edit!");
    }
    if (no_l2_edit)
    {
        CTC_DKIT_PATH_PRINT("%-15s\n", "not do L2 edit!");
    }

Discard:
    if (nhp_discard)
    {
        discard = 1;
        discard_type = nhp_discardType;
    }
    else if (pld_discard)
    {
        discard = 1;
        discard_type = pld_discardType;
    }
    else if (edit_discard)
    {
        discard = 1;
        discard_type = edit_discardType;
    }
    _ctc_usw_dkit_captured_path_discard_process(p_info, discard, discard_type, CTC_DKIT_EPE);
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_epe_ipfix(ctc_dkit_captured_path_info_t* p_info)
{
    _ctc_usw_dkit_captured_path_ipfix(p_info, 1);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_wlan_engine(ctc_dkit_captured_path_info_t* p_info)
{
    uint8  lchip = p_info->lchip;
    uint32 cmd = 0;
    char*  str_error_code[] = {"None", "WlanEncryptGeneralError", "WlanDecryptGeneralError", "WlanDecryptMacError",
                               "WlanDecryptSeqError", "WlanDecryptEpochError", "WlanReassembleError",
                               "Dot1aeEncryptError", "Dot1aeDecryptUnknowPkt", "Dot1aeDecryptIcvError",
                               DRV_IS_DUET2(lchip)?"Dot1aeCipherModeMismatch":"Dot1aeDecryptPnError",
                               DRV_IS_DUET2(lchip)?"Dot1aeDecryptPnError":"Dot1aeSciError",
                               DRV_IS_DUET2(lchip)?"Dot1aeSciError":"Dot1aeSANouseError"};
    char*  str_exc_code[] = {"None", "WlanSnThrdMatch", "Dot1aeThrdMatch", "Dot1aeIcvFailOrCipherMis"};
    uint32 macsecDecryptEn = 0;
    uint32 macsecDecryptKeyIndex = 0;
    uint32 macsecDecryptDot1AeEs = 0;
    uint32 macsecDecryptDot1AePn = 0;
    uint32 macsecDecryptDot1AeSc = 0;
    uint32 macsecDecryptDot1AeSci[2] = {0};
    uint32 macsecDecryptDot1AeSl = 0;
    uint32 macsecDecryptErrorCode = 0;
    uint32 macsecDecryptExceptionCode = 0;
    uint32 macsecDecryptPacketLength = 0;
    uint32 macsecDecryptSecTagCbit = 0;
    uint32 macsecDecryptSecTagEbit = 0;
    uint32 macsecDecryptValid = 0;
    DbgMACSecDecryptEngineInfo_m macsec_decrypt_info;

    uint32 macsecEncryptDot1AePn = 0;
    uint32 macsecEncryptDot1AeSci[2] = {0};
    uint32 macsecEncryptEn = 0;
    uint32 macsecEncryptKeyIndex = 0;
    uint32 macsecEncryptErrorCode = 0;
    uint32 macsecEncryptPacketLength = 0;
    uint32 macsecEncryptSecTagSc = 0;
    uint32 macsecEncryptValid = 0;
    DbgMACSecEncryptEngineInfo_m macsec_encrypt_info;

    sal_memset(&macsec_decrypt_info, 0, sizeof(macsec_decrypt_info));
    cmd = DRV_IOR(DbgMACSecDecryptEngineInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &macsec_decrypt_info);
    GetDbgMACSecDecryptEngineInfo(A, decryptEn_f,       &macsec_decrypt_info, &macsecDecryptEn);
    GetDbgMACSecDecryptEngineInfo(A, decryptKeyIndex_f, &macsec_decrypt_info, &macsecDecryptKeyIndex);
    GetDbgMACSecDecryptEngineInfo(A, dot1AeEs_f,        &macsec_decrypt_info, &macsecDecryptDot1AeEs);
    GetDbgMACSecDecryptEngineInfo(A, dot1AePn_f,        &macsec_decrypt_info, &macsecDecryptDot1AePn);
    GetDbgMACSecDecryptEngineInfo(A, dot1AeSc_f,        &macsec_decrypt_info, &macsecDecryptDot1AeSc);
    DRV_GET_FIELD_A(lchip, DbgMACSecDecryptEngineInfo_t, DbgMACSecDecryptEngineInfo_dot1AeSci_f, &macsec_decrypt_info, &macsecDecryptDot1AeSci);
    GetDbgMACSecDecryptEngineInfo(A, dot1AeSl_f,        &macsec_decrypt_info, &macsecDecryptDot1AeSl);
    GetDbgMACSecDecryptEngineInfo(A, errorCode_f,       &macsec_decrypt_info, &macsecDecryptErrorCode);
    GetDbgMACSecDecryptEngineInfo(A, exceptionCode_f,   &macsec_decrypt_info, &macsecDecryptExceptionCode);
    GetDbgMACSecDecryptEngineInfo(A, packetLength_f,    &macsec_decrypt_info, &macsecDecryptPacketLength);
    GetDbgMACSecDecryptEngineInfo(A, secTagCbit_f,      &macsec_decrypt_info, &macsecDecryptSecTagCbit);
    GetDbgMACSecDecryptEngineInfo(A, secTagEbit_f,      &macsec_decrypt_info, &macsecDecryptSecTagEbit);
    GetDbgMACSecDecryptEngineInfo(A, valid_f,           &macsec_decrypt_info, &macsecDecryptValid);

    sal_memset(&macsec_encrypt_info, 0, sizeof(macsec_encrypt_info));
    cmd = DRV_IOR(DbgMACSecEncryptEngineInfo_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &macsec_encrypt_info);
    GetDbgMACSecEncryptEngineInfo(A, dot1AePn_f,        &macsec_encrypt_info, &macsecEncryptDot1AePn);
    DRV_GET_FIELD_A(lchip, DbgMACSecEncryptEngineInfo_t, DbgMACSecEncryptEngineInfo_dot1AeSci_f, &macsec_encrypt_info, &macsecEncryptDot1AeSci);
    GetDbgMACSecEncryptEngineInfo(A, encryptEn_f,       &macsec_encrypt_info, &macsecEncryptEn);
    GetDbgMACSecEncryptEngineInfo(A, encryptKeyIndex_f, &macsec_encrypt_info, &macsecEncryptKeyIndex);
    GetDbgMACSecEncryptEngineInfo(A, errorCode_f,       &macsec_encrypt_info, &macsecEncryptErrorCode);
    GetDbgMACSecEncryptEngineInfo(A, packetLength_f,    &macsec_encrypt_info, &macsecEncryptPacketLength);
    GetDbgMACSecEncryptEngineInfo(A, secTagSc_f,        &macsec_encrypt_info, &macsecEncryptSecTagSc);
    GetDbgMACSecEncryptEngineInfo(A, valid_f,           &macsec_encrypt_info, &macsecEncryptValid);

    CTC_DKIT_PRINT("\nWLAN PROCESS   \n-----------------------------------\n");

    if(macsecDecryptValid && macsecDecryptEn)
    {
        CTC_DKIT_PRINT("MacSec Decrypt Process:\n");
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "SecTag ES", macsecDecryptDot1AeEs);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "SecTag SC", macsecDecryptDot1AeSc);
        if(macsecDecryptDot1AeSc)
        {
            CTC_DKIT_PATH_PRINT("%-15s:0x%x%x\n", "SecTag SCI", macsecDecryptDot1AeSci[1], macsecDecryptDot1AeSci[0]);
        }
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "SecTag SL", macsecDecryptDot1AeSl);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "SecTag Cbit", macsecDecryptSecTagCbit);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "SecTag Ebit", macsecDecryptSecTagEbit);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "SecTag PN", macsecDecryptDot1AePn);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Key Index", macsecDecryptKeyIndex);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Pkt Length", macsecDecryptPacketLength);
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Error Code", str_error_code[macsecDecryptErrorCode]);
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Exception Code", str_exc_code[macsecDecryptExceptionCode]);
    }

    if(macsecEncryptValid && macsecEncryptEn)
    {
        CTC_DKIT_PRINT("MacSec Encrypt Process:\n");
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "SecTag SC", macsecEncryptSecTagSc);
        if(macsecDecryptDot1AeSc)
        {
            CTC_DKIT_PATH_PRINT("%-15s:0x%x%x\n", "SecTag SCI", macsecEncryptDot1AeSci[0], macsecEncryptDot1AeSci[1]);
        }
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "SecTag PN", macsecEncryptDot1AePn);
        CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Key Index", macsecEncryptKeyIndex);
        if (DRV_IS_DUET2(lchip))
        {
            CTC_DKIT_PATH_PRINT("%-15s:%u\n", "Pkt Length", macsecEncryptPacketLength);
        }
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Error Code", str_error_code[macsecEncryptErrorCode]);
    }

    return CLI_SUCCESS;
}


STATIC int32
_ctc_usw_dkit_captured_path_oam_rx(ctc_dkit_captured_path_info_t* p_info)
{

    uint8 slice = p_info->slice_epe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;
    uint32 bypass = 0;
    char* oam_pdu_type[40] = {"OAM_TYPE_NONE", "ETHER_CCM", "ETHER_APS", "ETHER_1DM", "ETHER_DMM",
                          "ETHER_DMR", "ETHER_RAPS", "ETHER_LBM", "ETHER_LBR", "ETHER_LMR",
                          "ETHER_LMM", "MPLSTP_LBM", "ETHER_TST", "ETHER_LTR", "ETHER_LTM",
                          "ETHER_OTHER", "MPLSTP_DM", "MPLSTP_DLM", "MPLSTP_DLMDM", "MPLSTP_CV",
                          "MPLSTP_CSF", "ETHER_CSF", "ETHER_MCC", "MPLS_OTHER", "MPLSTP_MCC",
                          "MPLSTP_FM", "ETHER_SLM", "ETHER_SLR", "MPLS_OAM", "BFD_OAM",
                          "ETHER_SCC", "MPLSTP_SCC", "FLEX_TEST", "TWAMP", "ETHER_1SL",
                          "ETHER_LLM", "ETHER_LLR", "ETHER_SCM", "ETHER_SCR"};

    /*DbgOamHdrAdj*/
    uint32 hdr_adj_discard = 0;
    uint32 hdr_adj_valid = 0;
    DbgOamHdrAdj_m  dbg_oam_hdr_adj;

    /*DbgOamParser*/
    uint32 oam_type = 0;
    uint32 desired_min_tx_interval = 0;
    uint32 my_disc = 0;
    uint32 required_min_rx_interval = 0;
    uint32 your_disc = 0;
    uint32 md_level = 0;
    uint32 bfd_oam_pdu_invalid = 0;
    uint32 oam_pdu_invalid = 0;
    uint32 dbit = 0;
    DbgOamParser_m   dbg_oam_parser;

    /*DbgOamRxProc*/
    uint32 bypass_all   = 0;
    uint32 equal_bfd    = 0;
    uint32 equal_ccm    = 0;
    uint32 equal_dm     = 0;
    uint32 equal_eth_tst = 0;
    uint32 equal_flex_test = 0;
    uint32 equal_lb     = 0;
    uint32 equal_lm     = 0;
    uint32 equal_sm     = 0;
    uint32 equal_twamp  = 0;
    uint32 exception   = 0;
    uint32 is_mdlvl_match = 0;
    uint32 is_up        = 0;
    uint32 link_oam     = 0;
    uint32 low_ccm      = 0;
    uint32 low_slow_oam  = 0;
    uint32 ma_index = 0;
    uint32 mip_en     = 0;
    uint32 rmep_index = 0;
    uint32 mep_index = 0;
    uint32 rx_valid = 0;
    uint32 equal_sbfd_ref = 0;
    DbgOamRxProc_m  dbg_oam_rx_proc;

    /*DbgOamDefectProc*/
    uint32  defect_subType   = 0;
    uint32  defect_type    = 0;
    uint32  defect_valid  = 0;
    DbgOamDefectProc_m  dbg_oam_defect_proc;

    /*DbgOamHdrEdit*/
    uint32 hdr_edit_bypassall =0;
    uint32 hdr_edit_discard  =0;
    uint32 hdr_edit_equal_dm   =0;
    uint32 hdr_edit_equal_lb    =0;
    uint32 hdr_edit_equal_lm    =0;
    uint32 hdr_edit_equal_sm     =0;
    uint32 hdr_edit_equal_twamp  =0;
    uint32 hdr_edit_exception    =0;
    uint32 hdr_edit_is_defect_free  =0;
    uint32 hdr_edit_twamp_ack     =0;
    uint32 hdr_edit_valid        =0;
    uint32 hdr_edit_equal_sbfd_ref        =0;
    DbgOamHdrEdit_m     dbg_oam_hdr_edit;

    /*DbgOamApsSwitch*/
    uint32 aps_groupId = 0;
    uint32 aps_protection_fail = 0;
    uint32 aps_signal_fail = 0;
    uint32 aps_signal_ok = 0;
    uint32 aps_valid = 0;
    uint32 aps_working_fail = 0;
    DbgOamApsSwitch_m dbg_oam_aps_switch;

    /*DbgOamApsProcess*/
    uint32 aps_proc_signal_fail = 0;
    uint32 aps_proc_signal_ok = 0;
    uint32 aps_proc_valid = 0;
    uint32 aps_proc_update_aps_en = 0;
    DbgOamApsProcess_m dbg_oam_aps_proc;

    sal_memset(&dbg_oam_hdr_adj, 0, sizeof(dbg_oam_hdr_adj));
    cmd = DRV_IOR(DbgOamHdrAdj_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_oam_hdr_adj);
    GetDbgOamHdrAdj(A, oamHdrAdjDiscard_f, &dbg_oam_hdr_adj, &hdr_adj_discard);
    GetDbgOamHdrAdj(A, oamHdrAdjValid_f, &dbg_oam_hdr_adj, &hdr_adj_valid);


    sal_memset(&dbg_oam_parser, 0, sizeof(dbg_oam_parser));
    cmd = DRV_IOR(DbgOamParser_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_oam_parser);
    GetDbgOamParser(A, oamType_f, &dbg_oam_parser, &oam_type);
    GetDbgOamParser(A, u_bfd_desiredMinTxInterval_f, &dbg_oam_parser, &desired_min_tx_interval);
    GetDbgOamParser(A, u_bfd_myDisc_f, &dbg_oam_parser, &my_disc);
    GetDbgOamParser(A, u_bfd_requiredMinRxInterval_f, &dbg_oam_parser, &required_min_rx_interval);
    GetDbgOamParser(A, u_bfd_yourDisc_f, &dbg_oam_parser, &your_disc);
    GetDbgOamParser(A, bfdOamPduInvalid_f, &dbg_oam_parser, &bfd_oam_pdu_invalid);
    GetDbgOamParser(A, u_ethoam_mdlevel_f, &dbg_oam_parser, &md_level);
    GetDbgOamParser(A, oamPduInvalid_f, &dbg_oam_parser, &oam_pdu_invalid);
    GetDbgOamParser(A, u_bfd_dbit_f, &dbg_oam_parser, &dbit);

    sal_memset(&dbg_oam_rx_proc, 0, sizeof(dbg_oam_rx_proc));
    cmd = DRV_IOR(DbgOamRxProc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_oam_rx_proc);
    GetDbgOamRxProc(A, bypassAll_f, &dbg_oam_rx_proc, &bypass_all);
    GetDbgOamRxProc(A, equalBfd_f, &dbg_oam_rx_proc, &equal_bfd);
    GetDbgOamRxProc(A, equalCcm_f, &dbg_oam_rx_proc, &equal_ccm);
    GetDbgOamRxProc(A, equalDm_f, &dbg_oam_rx_proc, &equal_dm);
    GetDbgOamRxProc(A, equalEthTst_f, &dbg_oam_rx_proc, &equal_eth_tst);
    GetDbgOamRxProc(A, equalFlexTest_f, &dbg_oam_rx_proc, &equal_flex_test);
    GetDbgOamRxProc(A, equalLb_f, &dbg_oam_rx_proc, &equal_lb);
    GetDbgOamRxProc(A, equalLm_f, &dbg_oam_rx_proc, &equal_lm);
    GetDbgOamRxProc(A, equalSm_f, &dbg_oam_rx_proc, &equal_sm);
    GetDbgOamRxProc(A, equalTwamp_f, &dbg_oam_rx_proc, &equal_twamp);
    GetDbgOamRxProc(A, exception_f, &dbg_oam_rx_proc, &exception);
    GetDbgOamRxProc(A, isMdLvlMatch_f, &dbg_oam_rx_proc, &is_mdlvl_match);
    GetDbgOamRxProc(A, isUp_f, &dbg_oam_rx_proc, &is_up);
    GetDbgOamRxProc(A, linkOam_f, &dbg_oam_rx_proc, &link_oam);
    GetDbgOamRxProc(A, lowCcm_f, &dbg_oam_rx_proc, &low_ccm);
    GetDbgOamRxProc(A, lowSlowOam_f, &dbg_oam_rx_proc, &low_slow_oam);
    GetDbgOamRxProc(A, maIndex_f, &dbg_oam_rx_proc, &ma_index);
    GetDbgOamRxProc(A, mipEn_f, &dbg_oam_rx_proc, &mip_en);
    GetDbgOamRxProc(A, rmepIndex_f, &dbg_oam_rx_proc, &rmep_index);
    GetDbgOamRxProc(A, valid_f, &dbg_oam_rx_proc, &rx_valid);
    GetDbgOamRxProc(A, equalSBfdReflector_f, &dbg_oam_rx_proc, &equal_sbfd_ref);
    GetDbgOamRxProc(A, mepIndex_f, &dbg_oam_rx_proc, &mep_index);

    sal_memset(&dbg_oam_defect_proc, 0, sizeof(dbg_oam_defect_proc));
    cmd = DRV_IOR(DbgOamDefectProc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_oam_defect_proc);
    GetDbgOamDefectProc(A, oamDefectProcDefectSubType_f, &dbg_oam_defect_proc, &defect_subType);
    GetDbgOamDefectProc(A, oamDefectProcDefectType_f, &dbg_oam_defect_proc, &defect_type);
    GetDbgOamDefectProc(A, oamDefectProcValid_f, &dbg_oam_defect_proc, &defect_valid);

    sal_memset(&dbg_oam_hdr_edit, 0, sizeof(dbg_oam_hdr_edit));
    cmd = DRV_IOR(DbgOamHdrEdit_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_oam_hdr_edit);
    GetDbgOamHdrEdit(A, oamHdrEditBypassAll_f, &dbg_oam_hdr_edit, &hdr_edit_bypassall);
    GetDbgOamHdrEdit(A, oamHdrEditDiscard_f, &dbg_oam_hdr_edit, &hdr_edit_discard);
    GetDbgOamHdrEdit(A, oamHdrEditEqualDM_f, &dbg_oam_hdr_edit, &hdr_edit_equal_dm);
    GetDbgOamHdrEdit(A, oamHdrEditEqualLB_f, &dbg_oam_hdr_edit, &hdr_edit_equal_lb);
    GetDbgOamHdrEdit(A, oamHdrEditEqualLM_f, &dbg_oam_hdr_edit, &hdr_edit_equal_lm);
    GetDbgOamHdrEdit(A, oamHdrEditEqualSm_f, &dbg_oam_hdr_edit, &hdr_edit_equal_sm);
    GetDbgOamHdrEdit(A, oamHdrEditEqualTwamp_f, &dbg_oam_hdr_edit, &hdr_edit_equal_twamp);
    GetDbgOamHdrEdit(A, oamHdrEditException_f, &dbg_oam_hdr_edit, &hdr_edit_exception);
    GetDbgOamHdrEdit(A, oamHdrEditIsDefectFree_f, &dbg_oam_hdr_edit, &hdr_edit_is_defect_free);
    GetDbgOamHdrEdit(A, oamHdrEditTwampAck_f, &dbg_oam_hdr_edit, &hdr_edit_twamp_ack);
    GetDbgOamHdrEdit(A, oamHdrEditValid_f, &dbg_oam_hdr_edit, &hdr_edit_valid);
    GetDbgOamHdrEdit(A, oamHdrEditEqualSBfdReflector_f, &dbg_oam_hdr_edit, &hdr_edit_equal_sbfd_ref);

    sal_memset(&dbg_oam_aps_switch, 0, sizeof(dbg_oam_aps_switch));
    cmd = DRV_IOR(DbgOamApsSwitch_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_oam_aps_switch);
    GetDbgOamApsSwitch(A, oamApsSwitchApsGroupId_f, &dbg_oam_aps_switch, &aps_groupId);
    GetDbgOamApsSwitch(A, oamApsSwitchProtectionFail_f, &dbg_oam_aps_switch, &aps_protection_fail);
    GetDbgOamApsSwitch(A, oamApsSwitchSignalFail_f, &dbg_oam_aps_switch, &aps_signal_fail);
    GetDbgOamApsSwitch(A, oamApsSwitchSignalOk_f, &dbg_oam_aps_switch, &aps_signal_ok);
    GetDbgOamApsSwitch(A, oamApsSwitchWorkingFail_f, &dbg_oam_aps_switch, &aps_working_fail);
    GetDbgOamApsSwitch(A, oamApsSwitchValid_f, &dbg_oam_aps_switch, &aps_valid);

    sal_memset(&dbg_oam_aps_proc, 0, sizeof(dbg_oam_aps_proc));
    cmd = DRV_IOR(DbgOamApsProcess_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_oam_aps_switch);
    GetDbgOamApsProcess(A, oamApsProcessSignalFail_f, &dbg_oam_aps_proc, &aps_proc_signal_fail);
    GetDbgOamApsProcess(A, oamApsProcessSignalOk_f, &dbg_oam_aps_proc, &aps_proc_signal_ok);
    GetDbgOamApsProcess(A, oamApsProcessUpdateApsEn_f, &dbg_oam_aps_proc, &aps_proc_update_aps_en);
    GetDbgOamApsProcess(A, oamApsProcessValid_f, &dbg_oam_aps_proc, &aps_proc_valid);

    if (0 == p_info->detail)
    {
        goto Discard;
    }


    bypass = hdr_edit_bypassall || bypass_all;

    if (!(aps_valid || hdr_edit_valid || defect_valid || rx_valid || oam_pdu_invalid || hdr_adj_valid))
    {
        goto Discard;
    }


    CTC_DKIT_PRINT("OAM RX Process:\n");
    if (bypass)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Bypassall:", "YES");
    }
    if (hdr_edit_discard)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Discard:", "YES");
    }
    if (hdr_edit_exception)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Exception id", hdr_edit_exception);
    }
    /*oam parser*/
    if (oam_pdu_invalid ||bfd_oam_pdu_invalid)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "OAM pdu type", oam_pdu_type[oam_type]);
        if (bfd_oam_pdu_invalid)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Mydisc", my_disc);
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "YourDisc", your_disc);
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "DesiredMinTxInterval", desired_min_tx_interval);
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "RequiredMinRxInterval", required_min_rx_interval);
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Dbit", dbit);
        }
        else if(oam_pdu_invalid)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Md level", md_level);
        }
    }

    /*oam rx proc*/
    if (rx_valid)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Mep index", mep_index);
        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "MaIndex", ma_index);
        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "RmepIndex", rmep_index);
        if (link_oam)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "LinkOAM", "YES");
        }
        if (low_ccm)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "LowCCM", "YES");
        }
        if (low_slow_oam)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "LowSlowCCM", "YES");
        }
        if (mip_en)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "MipEn", "YES");
        }
        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "Rx exception", exception);
        if (bypass_all)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Rx Bypassall", "YES");
        }
        if (equal_bfd)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "BFD proc", "YES");
        }
        if (equal_ccm)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "CCM proc", "YES");
        }
        if (equal_dm)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "DM proc", "YES");
        }
        if (equal_eth_tst)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Tst proc", "YES");
        }
        if (equal_flex_test)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "FlexTest proc", "YES");
        }
        if (equal_lb)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "LB proc", "YES");
        }
        if (equal_lm)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "LM proc", "YES");
        }
        if (equal_sm)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "SM proc", "YES");
        }
        if (equal_twamp)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "TWAMP proc", "YES");
        }
        if (equal_sbfd_ref)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "SBFD Ref proc", "YES");
        }
        if (is_mdlvl_match)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "MdLvlMatch", "YES");
        }
        if (is_up)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "isUp", "YES");
        }
        if (link_oam)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "LinkOAM", "YES");
        }
        if (low_ccm)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "LowCCM", "YES");
        }
        if (low_slow_oam)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "LowSlowCCM", "YES");
        }
        if (mip_en)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "MipEn", "YES");
        }
    }

    if ((defect_valid) && ((defect_subType) || (defect_type)))
    {
        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "OamDefectProcDefectType", defect_type);
        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "OamDefectProcDefectSubType", defect_subType);
    }

    /*oam head edit*/
    if (hdr_edit_valid && (hdr_edit_bypassall ||hdr_edit_is_defect_free || hdr_edit_discard || hdr_edit_equal_dm || hdr_edit_equal_lb
        || hdr_edit_equal_lm || hdr_edit_equal_sm || hdr_edit_equal_twamp || hdr_edit_twamp_ack || hdr_edit_equal_sbfd_ref))
    {
        CTC_DKIT_PRINT("OAM Header Edit Proc:\n");
        if (hdr_edit_bypassall)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Bypassall", "YES");
        }
        if (hdr_edit_is_defect_free)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "No defect", "YES");
        }
        if (hdr_edit_discard)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "Discard", "YES");
        }
        if (hdr_edit_equal_dm)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "DM proc", "YES");
        }
        if (hdr_edit_equal_lb)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "LB proc", "YES");
        }
        if (hdr_edit_equal_lm)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "LM proc", "YES");
        }
        if (hdr_edit_equal_sm)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "SM proc", "YES");
        }
        if (hdr_edit_equal_twamp)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "TWAMP proc", "YES");
        }
        if (hdr_edit_twamp_ack)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "TWAMP ACK proc", "YES");
        }
        if (hdr_edit_equal_sbfd_ref)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "SBFD proc", "YES");
        }
    }

    if (aps_valid)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%d\n", "ApsGroupId", aps_groupId);
        if (aps_signal_ok)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "SignalOk", "YES");
        }
        if (aps_signal_fail)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "SignalFail", "YES");
        }
        if (aps_protection_fail)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "ApsProtectionFail", "YES");
        }
        if (aps_working_fail)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "ApsWorkingFail", "YES");
        }
    }
    else if (!aps_valid && aps_proc_valid)
    {
        if (aps_proc_signal_ok)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "SignalOk", "YES");
        }
        if (aps_proc_signal_fail)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "SignalFail", "YES");
        }
        if (aps_proc_update_aps_en)
        {
            CTC_DKIT_PATH_PRINT("%-15s:%s\n", "APS enable", "YES");
        }
    }


Discard:

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_oam_tx(ctc_dkit_captured_path_info_t* p_info)
{

    uint8 slice = p_info->slice_epe;
    uint8 lchip = p_info->lchip;
    uint32 cmd = 0;
    char* rx_oam_type[15] = {"NONE", "ETHEROAM", "IPBFD", "PBTOAM", "PBBBSI",
                          "PBBBV", "MPLSOAM", "MPLSBFD", "ACHOAM", "TRILLBFD",
                          "TWAMP", "FLEX_PM_TEST"};
    char* packet_type[10] = {"ETHERNET", "IPV4", "MPLS", "IPV6", "MPLSUP",
                          "IP", "FLEXIBLE", "RESERVED"};

    uint32 tx_proc_destmap      =0;
    uint32 tx_proc_is_eth       =0;
    uint32 tx_proc_is_tx_cv     =0;
    uint32 tx_proc_is_up        =0;
    uint32 tx_proc_is_tx_csf    =0;
    uint32 tx_proc_nexthop_ptr  =0;
    uint32 tx_proc_packet_type  =0;
    uint32 tx_proc_rxOam_type   =0;
    uint32 tx_proc_valid        =0;
    DbgOamTxProc_m   dbg_oam_tx_proc;

    sal_memset(&dbg_oam_tx_proc, 0, sizeof(dbg_oam_tx_proc));
    cmd = DRV_IOR(DbgOamTxProc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, slice, cmd, &dbg_oam_tx_proc);
    GetDbgOamTxProc(A, oamTxProcDestmap_f, &dbg_oam_tx_proc, &tx_proc_destmap);
    GetDbgOamTxProc(A, oamTxProcIsEth_f, &dbg_oam_tx_proc, &tx_proc_is_eth);
    GetDbgOamTxProc(A, oamTxProcIsTxCv_f, &dbg_oam_tx_proc, &tx_proc_is_tx_cv);
    GetDbgOamTxProc(A, oamTxProcIsUp_f, &dbg_oam_tx_proc, &tx_proc_is_up);
    GetDbgOamTxProc(A, oamTxProcIstxCsf_f, &dbg_oam_tx_proc, &tx_proc_is_tx_csf);
    GetDbgOamTxProc(A, oamTxProcNextHopPtr_f, &dbg_oam_tx_proc, &tx_proc_nexthop_ptr);
    GetDbgOamTxProc(A, oamTxProcPacketType_f, &dbg_oam_tx_proc, &tx_proc_packet_type);
    GetDbgOamTxProc(A, oamTxProcRxOamType_f, &dbg_oam_tx_proc, &tx_proc_rxOam_type);
    GetDbgOamTxProc(A, oamTxProcValid_f, &dbg_oam_tx_proc, &tx_proc_valid);
    if ((0 == p_info->detail) || (!tx_proc_valid))
    {
        goto Discard;
    }
    CTC_DKIT_PRINT("OAM TX Process:\n");
    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "RxOamType:", rx_oam_type[tx_proc_rxOam_type]);
    CTC_DKIT_PATH_PRINT("%-15s:%s\n", "PackrtType:", packet_type[tx_proc_packet_type]);
    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "NextHopPtr", tx_proc_nexthop_ptr);
    CTC_DKIT_PATH_PRINT("%-15s:%d\n", "DestMap", tx_proc_destmap);
    if (tx_proc_is_eth)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "IsEth", "YES");
    }
    if (tx_proc_is_tx_cv)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "IsTxCv", "YES");
    }
    if (tx_proc_is_tx_csf)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "IsTxCsf", "YES");
    }
    if (tx_proc_is_up)
    {
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "IsUpMep", "YES");
    }

Discard:

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_captured_path_oam_engine(ctc_dkit_captured_path_info_t* p_info)
{
    CTC_DKIT_PRINT("\nOAMEngine PROCESS   \n-----------------------------------\n");
    /*1. RX oam*/
    if (!p_info->discard)
    {
        _ctc_usw_dkit_captured_path_oam_rx(p_info);
    }
    /*2. TX oam*/
    if (!p_info->discard)
    {
        _ctc_usw_dkit_captured_path_oam_tx(p_info);
    }

    return CLI_SUCCESS;
}


STATIC int32
_ctc_usw_dkit_captured_path_epe(ctc_dkit_captured_path_info_t* p_info)
{
    CTC_DKIT_PRINT("\nEGRESS PROCESS   \n-----------------------------------\n");

    /*1. Parser*/
    if (!p_info->discard)
    {
        _ctc_usw_dkit_captured_path_epe_parser(p_info);
    }
    /*2. SCL*/
    if (!p_info->discard)
    {
        _ctc_usw_dkit_captured_path_epe_scl(p_info);
    }
    /*3. Interface*/
    if (!p_info->discard)
    {
        _ctc_usw_dkit_captured_path_epe_interface(p_info);
    }
    /*4. ACL*/
    if (!p_info->discard)
    {
        _ctc_usw_dkit_captured_path_epe_acl(p_info);
    }
    /*5. OAM*/
    if (!p_info->discard)
    {
        _ctc_usw_dkit_captured_path_epe_oam(p_info);
    }
    /*6. Edit*/
    if (!p_info->discard)
    {
        _ctc_usw_dkit_captured_path_epe_edit(p_info);
    }

    /*7. Ipfix*/

    _ctc_usw_dkit_captured_path_epe_ipfix(p_info);

    if (p_info->discard && (0 == p_info->detail))
    {
        CTC_DKIT_PRINT("Packet Drop at EPE:\n");
        CTC_DKIT_PATH_PRINT("%-15s:%s\n", "reason", ctc_usw_dkit_get_reason_desc(p_info->discard_type + EPE_FEATURE_START));
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkit_captured_path_process(void* p_para)
{
    ctc_dkit_path_para_t* p_path_para = (ctc_dkit_path_para_t*)p_para;
    ctc_dkit_captured_path_info_t path_info ;
    ctc_dkit_captured_t ctc_dkit_captured;
    uint32 path_sel = 0;
    uint32 cmd = 0;
    uint8 lchip = 0;
    DKITS_PTR_VALID_CHECK(p_para);
    if (p_path_para->captured_session)
    {
        CTC_DKIT_PRINT("invalid session id!\n");
        return CLI_ERROR;
    }

    sal_memset(&path_info, 0, sizeof(ctc_dkit_captured_path_info_t));
    sal_memset(&ctc_dkit_captured, 0, sizeof(ctc_dkit_captured_t));

    path_info.lchip = p_path_para->lchip;
    _ctc_usw_dkit_captured_is_hit( &path_info);

    path_info.detail = p_path_para->detail;

    lchip = p_path_para->lchip;
    cmd = DRV_IOR(MetFifoDebugCtl_t, MetFifoDebugCtl_gSrcChanType_0_debugPathSel_f);
    DRV_IOCTL(lchip, 0, cmd, &path_sel);

    path_info.path_sel = path_sel;
    if ((path_info.is_ipe_captured || path_info.is_bsr_captured || path_info.is_epe_captured || path_info.oam_tx_captured)
        && (CTC_DKIT_FLOW_MODE_ALL == p_path_para->mode))
    {
        if (path_info.oam_tx_captured)
        {
            _ctc_usw_dkit_captured_path_oam_tx(&path_info);
        }
        if ((path_info.is_ipe_captured) && (!path_info.discard))
        {
            _ctc_usw_dkit_captured_path_ipe(&path_info);
        }
        if ((path_info.is_bsr_captured)  && (path_info.detail))
        {
            _ctc_usw_dkit_captured_path_bsr(&path_info);
        }
        if ((path_info.is_epe_captured) && (!path_info.discard) && (path_info.path_sel == 1))
        {
            _ctc_usw_dkit_captured_path_epe(&path_info);
        }
        if (!path_info.discard)
        {
            _ctc_usw_dkit_captured_path_wlan_engine(&path_info);
        }
    }
    else if(path_info.is_ipe_captured && (CTC_DKIT_FLOW_MODE_IPE == p_path_para->mode) && (!path_info.discard))
    {
        _ctc_usw_dkit_captured_path_ipe(&path_info);
    }
    else if(path_info.is_bsr_captured  && (!path_info.discard)
        && (CTC_DKIT_FLOW_MODE_BSR == p_path_para->mode || CTC_DKIT_FLOW_MODE_METFIFO == p_path_para->mode))
    {
        _ctc_usw_dkit_captured_path_bsr(&path_info);
    }
    else if(path_info.is_epe_captured && (CTC_DKIT_FLOW_MODE_EPE == p_path_para->mode) && (!path_info.discard))
    {
         _ctc_usw_dkit_captured_path_epe(&path_info);
    }
    else if((path_info.oam_rx_captured || path_info.oam_tx_captured) && (!path_info.discard)
        && (CTC_DKIT_FLOW_MODE_OAM_RX == p_path_para->mode || CTC_DKIT_FLOW_MODE_OAM_TX == p_path_para->mode))
    {
        _ctc_usw_dkit_captured_path_oam_engine(&path_info);
    }
    else
    {
        CTC_DKIT_PRINT("Not Captured Flow!\n");
    }
    ctc_dkit_captured.captured_session = p_path_para->captured_session;
    ctc_dkit_captured.flag = p_path_para->flag;
    ctc_usw_dkit_captured_clear((void*)(&ctc_dkit_captured));

    return CLI_SUCCESS;
}


int32
ctc_usw_dkit_captured_clear(void* p_para)
{
    ctc_dkit_captured_t* p_captured_para = (ctc_dkit_captured_t*)p_para;
    uint32 flag = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 lchip = 0;
    uint32 temp_v = 0;
    IpeUserIdFlowLoopCam_m ds_flow_loop_cam;
    BufStoreDebugCam_m ds_buff_store_cam;
    MetFifoDebugCam_m  ds_met_fifo_cam;
    EpeHdrAdjustDebugCam_m epe_dbg_cam;

    DKITS_PTR_VALID_CHECK(p_para);
    lchip = p_captured_para->lchip;
    CTC_DKIT_LCHIP_CHECK(lchip);
    DRV_INIT_CHECK(lchip);
    if (p_captured_para->captured_session)
    {
        CTC_DKIT_PRINT("invalid session id!\n");
        return CLI_ERROR;
    }
    flag = p_captured_para->flag;

    if ((CTC_DKIT_CAPTURED_CLEAR_RESULT == flag) || (CTC_DKIT_CAPTURED_CLEAR_ALL == flag))
    {
        value = 1;
        cmd = DRV_IOW(DebugSessionStatusCtl_t, DebugSessionStatusCtl_isFree_f);
        DRV_IOCTL(lchip, 0, cmd, &value);

        value = 1;
        cmd = DRV_IOW(EpeHdrAdjustChanCtl_t, EpeHdrAdjustChanCtl_debugIsFree_f);
        DRV_IOCTL(lchip, 0, cmd, &value);

        cmd = DRV_IOW(DbgFwdBufRetrvHeaderInfo_t, DbgFwdBufRetrvHeaderInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFwdBufRetrvInfo_t, DbgFwdBufRetrvInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFwdBufStoreInfo_t, DbgFwdBufRetrvInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFwdBufStoreInfo2_t, DbgFwdBufStoreInfo2_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIrmResrcAllocInfo_t, DbgIrmResrcAllocInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgWlanDecryptEngineInfo_t, DbgWlanDecryptEngineInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgWlanEncryptAndFragmentEngineInfo_t, DbgWlanEncryptAndFragmentEngineInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgWlanReassembleEngineInfo_t, DbgWlanReassembleEngineInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgDlbEngineInfo_t, DbgDlbEngineInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgOamApsSwitch_t, DbgOamApsSwitch_oamApsSwitchValid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEfdEngineInfo_t, DbgEfdEngineInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEgrXcOamHash0EngineFromEpeNhpInfo_t, DbgEgrXcOamHash0EngineFromEpeNhpInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEgrXcOamHash1EngineFromEpeNhpInfo_t, DbgEgrXcOamHash1EngineFromEpeNhpInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEgrXcOamHashEngineFromEpeOam0Info_t, DbgEgrXcOamHashEngineFromEpeOam0Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEgrXcOamHashEngineFromEpeOam1Info_t, DbgEgrXcOamHashEngineFromEpeOam0Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEgrXcOamHashEngineFromEpeOam2Info_t, DbgEgrXcOamHashEngineFromEpeOam2Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEgrXcOamHashEngineFromIpeOam0Info_t, DbgEgrXcOamHashEngineFromIpeOam0Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEgrXcOamHashEngineFromIpeOam1Info_t, DbgEgrXcOamHashEngineFromIpeOam1Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEgrXcOamHashEngineFromIpeOam2Info_t, DbgEgrXcOamHashEngineFromIpeOam2Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpeAclInfo_t, DbgEpeAclInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpeClassificationInfo_t, DbgEpeClassificationInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpeOamInfo_t, DbgEpeOamInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpeHdrAdjInfo_t, DbgEpeHdrAdjInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpeHdrEditCflexHdrInfo_t, DbgEpeHdrEditCflexHdrInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpeHdrEditInfo_t, DbgEpeHdrEditInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpeEgressEditInfo_t, DbgEpeEgressEditInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpeInnerL2EditInfo_t, DbgEpeInnerL2EditInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpeL3EditInfo_t, DbgEpeL3EditInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpeOuterL2EditInfo_t, DbgEpeOuterL2EditInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpePayLoadInfo_t, DbgEpePayLoadInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgParserFromEpeHdrAdjInfo_t, DbgParserFromEpeHdrAdjInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgEpeNextHopInfo_t, DbgEpeNextHopInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFibLkpEngineFlowHashInfo_t, DbgFibLkpEngineFlowHashInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFibLkpEngineHostUrpfHashInfo_t, DbgFibLkpEngineHostUrpfHashInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgLpmPipelineLookupResultInfo_t, DbgLpmPipelineLookupResultInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineUserIdKeyInfo2_t, DbgFlowTcamEngineUserIdKeyInfo2_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeFwdInputHeaderInfo_t, DbgIpeFwdInputHeaderInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFibLkpEngineMacDaHashInfo_t, DbgFibLkpEngineMacDaHashInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFibLkpEngineMacSaHashInfo_t, DbgFibLkpEngineMacSaHashInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFibLkpEnginel3DaHashInfo_t, DbgFibLkpEnginel3DaHashInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFibLkpEnginel3SaHashInfo_t, DbgFibLkpEnginel3SaHashInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpfixAccEgrInfo_t, DbgIpfixAccEgrInfo_gEgr_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpfixAccIngInfo_t, DbgIpfixAccIngInfo_gIngr_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpfixAccInfo0_t, DbgIpfixAccInfo0_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpfixAccInfo1_t, DbgIpfixAccInfo1_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineEpeAclInfo0_t, DbgFlowTcamEngineEpeAclInfo0_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineEpeAclInfo1_t, DbgFlowTcamEngineEpeAclInfo1_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineEpeAclInfo2_t, DbgFlowTcamEngineEpeAclInfo2_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineEpeAclKeyInfo0_t, DbgFlowTcamEngineEpeAclKeyInfo0_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineEpeAclKeyInfo1_t, DbgFlowTcamEngineEpeAclKeyInfo1_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineEpeAclKeyInfo2_t, DbgFlowTcamEngineEpeAclKeyInfo2_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclInfo0_t, DbgFlowTcamEngineIpeAclInfo0_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclInfo1_t, DbgFlowTcamEngineIpeAclInfo1_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclInfo2_t, DbgFlowTcamEngineIpeAclInfo2_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclInfo3_t, DbgFlowTcamEngineIpeAclInfo3_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclInfo4_t, DbgFlowTcamEngineIpeAclInfo4_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclInfo5_t, DbgFlowTcamEngineIpeAclInfo5_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclInfo6_t, DbgFlowTcamEngineIpeAclInfo6_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclInfo7_t, DbgFlowTcamEngineIpeAclInfo7_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclKeyInfo0_t, DbgFlowTcamEngineIpeAclKeyInfo0_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclKeyInfo1_t, DbgFlowTcamEngineIpeAclKeyInfo1_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclKeyInfo2_t, DbgFlowTcamEngineIpeAclKeyInfo2_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclKeyInfo3_t, DbgFlowTcamEngineIpeAclKeyInfo3_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclKeyInfo4_t, DbgFlowTcamEngineIpeAclKeyInfo4_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclKeyInfo5_t, DbgFlowTcamEngineIpeAclKeyInfo5_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclKeyInfo6_t, DbgFlowTcamEngineIpeAclKeyInfo6_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineIpeAclKeyInfo7_t, DbgFlowTcamEngineIpeAclKeyInfo7_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineUserIdInfo0_t, DbgFlowTcamEngineUserIdInfo0_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineUserIdInfo1_t, DbgFlowTcamEngineUserIdInfo1_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineUserIdInfo2_t, DbgFlowTcamEngineUserIdInfo2_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineUserIdKeyInfo0_t, DbgFlowTcamEngineUserIdKeyInfo0_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFlowTcamEngineUserIdKeyInfo1_t, DbgFlowTcamEngineUserIdKeyInfo1_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeAclProcInfo_t, DbgIpeAclProcInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeEcmpProcInfo_t, DbgIpeEcmpProcInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeFwdCoppInfo_t, DbgIpeFwdCoppInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeFwdPolicingInfo_t, DbgIpeFwdPolicingInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeFwdProcessInfo_t, DbgIpeFwdProcessInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeAclProcInfo0_t, DbgIpeAclProcInfo0_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeAclProcInfo1_t, DbgIpeAclProcInfo1_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeFwdStormCtlInfo_t, DbgIpeFwdStormCtlInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeIntfMapperInfo_t, DbgIpeIntfMapperInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeMplsDecapInfo_t, DbgIpeMplsDecapInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeUserIdInfo_t, DbgIpeUserIdInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeLkpMgrInfo_t, DbgIpeLkpMgrInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgParserFromIpeIntfInfo_t, DbgParserFromIpeIntfInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeFcoeInfo_t, DbgIpeFcoeInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeFlowProcessInfo_t, DbgIpeFlowProcessInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeIpRoutingInfo_t, DbgIpeIpRoutingInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeMacBridgingInfo_t, DbgIpeMacBridgingInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeMacLearningInfo_t, DbgIpeMacLearningInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeOamInfo_t, DbgIpeOamInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpePacketProcessInfo_t, DbgIpePacketProcessInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpePerHopBehaviorInfo_t, DbgIpePerHopBehaviorInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgIpeTrillInfo_t, DbgIpeTrillInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgLagEngineInfoFromBsrEnqueue_t, DbgLagEngineInfoFromBsrEnqueue_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgLagEngineInfoFromIpeFwd_t, DbgLagEngineInfoFromIpeFwd_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgLpmTcamEngineResult0Info_t, DbgLpmTcamEngineResult0Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgLpmTcamEngineResult1Info_t, DbgLpmTcamEngineResult1Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgMACSecDecryptEngineInfo_t, DbgMACSecDecryptEngineInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgMACSecEncryptEngineInfo_t, DbgMACSecEncryptEngineInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFwdBufStoreInfo_t, DbgFwdBufStoreInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFwdBufStoreInfo2_t, DbgFwdBufStoreInfo2_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFwdMetFifoInfo1_t, DbgFwdMetFifoInfo1_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFwdMetFifoInfo2_t, DbgFwdMetFifoInfo2_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgOamHdrEdit_t, DbgOamHdrEdit_oamHdrEditValid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjValid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgOamParser_t, DbgOamParser_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgOamDefectProc_t, DbgOamDefectProc_oamDefectProcValid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgOamRxProc_t, DbgOamRxProc_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgOamTxProc_t, DbgOamTxProc_oamTxProcValid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgErmResrcAllocInfo_t, DbgErmResrcAllocInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgFwdQWriteInfo_t, DbgFwdQWriteInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgParserFromIpeHdrAdjInfo_t, DbgParserFromIpeHdrAdjInfo_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        /*cmd = DRV_IOW(DbgSubSchProcDmaInfo_t, DbgSubSchProcDmaInfo_subSchProcDmaInfoValid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgSubSchProcNetInfo_t, DbgSubSchProcNetInfo_subSchProcNetInfoValid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);*/                                                         /*TBD*/
        cmd = DRV_IOW(DbgUserIdHashEngineForMpls0Info_t, DbgUserIdHashEngineForMpls0Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgUserIdHashEngineForMpls1Info_t, DbgUserIdHashEngineForMpls1Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgUserIdHashEngineForMpls2Info_t, DbgUserIdHashEngineForMpls2Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgUserIdHashEngineForUserId0Info_t, DbgUserIdHashEngineForUserId0Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);
        cmd = DRV_IOW(DbgUserIdHashEngineForUserId1Info_t, DbgUserIdHashEngineForUserId1Info_valid_f);
        DRV_IOCTL(lchip, 0, cmd, &temp_v);


    }

    if ((CTC_DKIT_CAPTURED_CLEAR_FLOW == flag) || (CTC_DKIT_CAPTURED_CLEAR_ALL == flag))
    {
        value = 0;
        cmd = DRV_IOW(IpeUserIdFlowCtl_t, IpeUserIdFlowCtl_flowEn_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpeUserIdFlowCtl_t, IpeUserIdFlowCtl_debugMode_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(IpeUserIdFlowCtl_t, IpeUserIdFlowCtl_flowLogEn_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(MetFifoCtl_t, MetFifoCtl_debugLookupEn_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        sal_memset(&ds_flow_loop_cam, 0, sizeof(IpeUserIdFlowLoopCam_m));
        sal_memset(&ds_buff_store_cam, 0, sizeof(BufStoreDebugCam_m));
        sal_memset(&ds_met_fifo_cam, 0, sizeof(MetFifoDebugCam_m));
        sal_memset(&epe_dbg_cam, 0, sizeof(EpeHdrAdjustDebugCam_m));
        cmd = DRV_IOW(IpeUserIdFlowLoopCam_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &ds_flow_loop_cam);
        cmd = DRV_IOW(BufStoreDebugCam_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &ds_buff_store_cam);
        cmd = DRV_IOW(MetFifoDebugCam_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &ds_met_fifo_cam);
        cmd = DRV_IOW(EpeHdrAdjustDebugCam_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &epe_dbg_cam);

    }
    return CLI_SUCCESS;
}




int32
ctc_usw_dkit_captured_install_flow(void* p_para)
{
    ctc_dkit_captured_t* p_captured_para = (ctc_dkit_captured_t*)p_para;
    uint32 value = 0;
    uint32 path_sel = 0;
    uint32 cmd = 0;
    uint16 local_phy_port = 0;
    uint16 lport = 0;
    uint8 gchip = 0;
    uint32 is_bfd = 0;
    uint32 ma_index = 0;
    mac_addr_t mac_mask;
    hw_mac_addr_t mac_da = {0};
    hw_mac_addr_t mac_sa = {0};
    ipv6_addr_t ipv6_mask;
    ParserResult_m parser_result;
    ParserResult_m parser_result_mask;
    IpeUserIdFlowCam_m ds_flow_cam;
    IpeUserIdFlowLoopCam_m ds_flow_loop_cam;
    BufStoreDebugCam_m ds_buff_store_cam;
    MetFifoDebugCam_m  ds_met_fifo_cam;
    EpeHdrAdjustDebugCam_m epe_dbg_cam;
    IpeUserIdFlowCtl_m ipe_flow_ctl;
    uint8 lchip = 0;
    uint32 index = 0;
    uint32 dest_map = 0;
    uint8  loop = 0;

    DKITS_PTR_VALID_CHECK(p_para);
    lchip = p_captured_para->lchip;
    CTC_DKIT_LCHIP_CHECK(lchip);
    DRV_INIT_CHECK(lchip);

    sal_memset(&ipe_flow_ctl, 0, sizeof(IpeUserIdFlowCtl_m));
    sal_memset(&parser_result, 0, sizeof(ParserResult_m));
    sal_memset(&parser_result_mask, 0, sizeof(ParserResult_m));
    sal_memset(&ds_flow_cam, 0, sizeof(IpeUserIdFlowCam_m));
    sal_memset(&ds_flow_loop_cam, 0, sizeof(IpeUserIdFlowLoopCam_m));
    sal_memset(&ds_buff_store_cam, 0, sizeof(BufStoreDebugCam_m));
    sal_memset(&ds_met_fifo_cam, 0, sizeof(MetFifoDebugCam_m));
    sal_memset(&epe_dbg_cam, 0, sizeof(EpeHdrAdjustDebugCam_m));
    sal_memset(&mac_mask, 0xFF, sizeof(mac_mask));
    sal_memset(&ipv6_mask, 0xFF, sizeof(ipv6_mask));

    if (p_captured_para->captured_session)
    {
        CTC_DKIT_PRINT("invalid session id!\n");
        return CLI_ERROR;
    }
    p_captured_para->flag = CTC_DKIT_CAPTURED_CLEAR_ALL;
    ctc_usw_dkit_captured_clear(p_captured_para);

    if(p_captured_para->path_sel > 2)
    {
        CTC_DKIT_PRINT("invalid path select id!\n");
        return CLI_ERROR;
    }
    path_sel = p_captured_para->path_sel + 1;
    /*1. enable debug session*/
    value = 1;
    cmd = DRV_IOW(DebugSessionStatusCtl_t, DebugSessionStatusCtl_isFree_f);
    DRV_IOCTL(lchip, 0, cmd, &value);

    cmd = DRV_IOW(EpeHdrAdjustChanCtl_t, EpeHdrAdjustChanCtl_debugIsFree_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_debugLookupEn_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(IpeUserIdFlowCtl_t, IpeUserIdFlowCtl_flowEn_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(BufStoreDebugCtl_t, BufStoreDebugCtl_debugLookupEn_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjDebugSessionEn_f);
    DRV_IOCTL(lchip, 0, cmd, &value);

    for (loop = 0; loop < 8; loop++)
    {
        cmd = DRV_IOW(MetFifoDebugCtl_t, MetFifoDebugCtl_gSrcChanType_0_debugPathSel_f + loop);
        DRV_IOCTL(lchip, 0, cmd, &path_sel);
    }

    cmd = DRV_IOW(DsMetFifoExcp_t, DsMetFifoExcp_debugEn_f);
    for(index=0; index < TABLE_MAX_INDEX(lchip, DsMetFifoExcp_t); ++index)
    {
        DRV_IOCTL(lchip, index, cmd, &value);
    }

    if (p_captured_para->to_cpu_en)
    {
        value = 1;
        cmd = DRV_IOW(IpeUserIdFlowCtl_t, IpeUserIdFlowCtl_flowLogEn_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
    }

    if (CTC_DKIT_FLOW_MODE_IPE == p_captured_para->mode)
    {
        value = 1;
        cmd = DRV_IOW(IpeUserIdFlowCtl_t, IpeUserIdFlowCtl_debugMode_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        if (1 == p_captured_para->flow_info_mask.port)
        {
            local_phy_port = CTC_DKIT_CTC_GPORT_TO_DRV_LPORT(p_captured_para->flow_info.port);
            SetIpeUserIdFlowLoopCam(V, localPhyPort_f, &ds_flow_loop_cam, local_phy_port);
            SetIpeUserIdFlowLoopCam(V, localPhyPortMask_f, &ds_flow_loop_cam, 0xFF);
        }
        SetIpeUserIdFlowLoopCam(V, data_0_channelId_f, &ds_flow_loop_cam, p_captured_para->flow_info_mask.channel_id_0 ? p_captured_para->flow_info.channel_id_0 : 0xFF);
        SetIpeUserIdFlowLoopCam(V, data_1_channelId_f, &ds_flow_loop_cam, p_captured_para->flow_info_mask.channel_id_1 ? p_captured_para->flow_info.channel_id_1 : 0xFF);
        SetIpeUserIdFlowLoopCam(V, data_2_channelId_f, &ds_flow_loop_cam, p_captured_para->flow_info_mask.channel_id_2 ? p_captured_para->flow_info.channel_id_2 : 0xFF);
        SetIpeUserIdFlowLoopCam(V, data_3_channelId_f, &ds_flow_loop_cam, p_captured_para->flow_info_mask.channel_id_3 ? p_captured_para->flow_info.channel_id_3 : 0xFF);
        if ((0 == p_captured_para->flow_info_mask.channel_id_0) && (0 == p_captured_para->flow_info_mask.channel_id_1)
            && (0 == p_captured_para->flow_info_mask.channel_id_2) && (0 == p_captured_para->flow_info_mask.channel_id_3))
        {
            SetIpeUserIdFlowLoopCam(V, mask_0_channelId_f, &ds_flow_loop_cam, 0);
            SetIpeUserIdFlowLoopCam(V, mask_1_channelId_f, &ds_flow_loop_cam, 0);
            SetIpeUserIdFlowLoopCam(V, mask_2_channelId_f, &ds_flow_loop_cam, 0);
            SetIpeUserIdFlowLoopCam(V, mask_3_channelId_f, &ds_flow_loop_cam, 0);
        }
        else
        {
            SetIpeUserIdFlowLoopCam(V, mask_0_channelId_f, &ds_flow_loop_cam, 0xFF);
            SetIpeUserIdFlowLoopCam(V, mask_1_channelId_f, &ds_flow_loop_cam, 0xFF);
            SetIpeUserIdFlowLoopCam(V, mask_2_channelId_f, &ds_flow_loop_cam, 0xFF);
            SetIpeUserIdFlowLoopCam(V, mask_3_channelId_f, &ds_flow_loop_cam, 0xFF);
        }

        if (1 == p_captured_para->flow_info_mask.is_debugged_pkt)
        {
            SetIpeUserIdFlowLoopCam(V, isDebuggedPkt_f, &ds_flow_loop_cam, p_captured_para->flow_info.is_debugged_pkt);
            SetIpeUserIdFlowLoopCam(V, isDebuggedPktMask_f, &ds_flow_loop_cam, 0xFF);
        }
        if (1 == p_captured_para->flow_info_mask.from_cflex_channel)
        {
            SetIpeUserIdFlowLoopCam(V, debuggedPktFromCflexChannel_f, &ds_flow_loop_cam, p_captured_para->flow_info.from_cflex_channel);
        }

    }
    else if (CTC_DKIT_FLOW_MODE_EPE == p_captured_para->mode)
    {
        if (1 == p_captured_para->flow_info_mask.channel_id_0)
        {
            /*dest channel*/
            SetEpeHdrAdjustDebugCam(V, g_0_channelId_f, &epe_dbg_cam, p_captured_para->flow_info.channel_id_0);
            SetEpeHdrAdjustDebugCam(V, g_1_channelId_f, &epe_dbg_cam, 0xFF);

        }
        if (1 == p_captured_para->flow_info_mask.is_debugged_pkt)
        {
            SetEpeHdrAdjustDebugCam(V, g_0_isDebuggedPkt_f, &epe_dbg_cam, p_captured_para->flow_info.is_debugged_pkt);
            SetEpeHdrAdjustDebugCam(V, g_1_isDebuggedPkt_f, &epe_dbg_cam, 0xFF);
        }
        if (1 == p_captured_para->flow_info_mask.port)
        {
            SetEpeHdrAdjustDebugCam(V, g_0_sourcePort_f, &epe_dbg_cam, p_captured_para->flow_info.port);
            SetEpeHdrAdjustDebugCam(V, g_1_sourcePort_f, &epe_dbg_cam, 0xFFFF);
        }
        if (1 == p_captured_para->flow_info_mask.dest_port)
        {
            lport = CTC_DKIT_CTC_GPORT_TO_SYS_LPORT(p_captured_para->flow_info.dest_port);
            gchip = CTC_DKIT_CTC_GPORT_TO_GCHIP(p_captured_para->flow_info.dest_port);
            SetEpeHdrAdjustDebugCam(V, g_1_destMap_f, &epe_dbg_cam, 0x7FFFF);
        }
        if (1 == p_captured_para->flow_info_mask.is_mcast)
        {
            dest_map = ((1 << 18) | CTC_DKITS_ENCODE_DESTMAP(gchip, lport));
        }
        else
        {
            dest_map = CTC_DKITS_ENCODE_DESTMAP(gchip, lport);
        }
        SetEpeHdrAdjustDebugCam(V, g_0_destMap_f, &epe_dbg_cam, dest_map & 0x7FFFF);

    }
    else if (CTC_DKIT_FLOW_MODE_BSR == p_captured_para->mode)
    {
        if (1 == p_captured_para->flow_info_mask.channel_id_0)
        {
            SetBufStoreDebugCam(V, g_0_channelId_f, &ds_buff_store_cam, p_captured_para->flow_info.channel_id_0);
            SetBufStoreDebugCam(V, g_1_channelId_f, &ds_buff_store_cam, 0xFF);
        }
        if (1 == p_captured_para->flow_info_mask.is_debugged_pkt)
        {
            SetBufStoreDebugCam(V, g_0_isDebuggedPkt_f, &ds_buff_store_cam, p_captured_para->flow_info.is_debugged_pkt);
            SetBufStoreDebugCam(V, g_1_isDebuggedPkt_f, &ds_buff_store_cam, 0xFF);
        }
        if ((1 == p_captured_para->flow_info_mask.is_mcast)&&(p_captured_para->flow_info_mask.group_id))
        {
            dest_map = ((1 << 18) | CTC_DKITS_ENCODE_MCAST_IPE_DESTMAP(p_captured_para->flow_info.group_id));
            SetBufStoreDebugCam(V, g_0_destMap_f, &ds_buff_store_cam, dest_map & 0x7FFFF);
            SetBufStoreDebugCam(V, g_1_destMap_f, &ds_buff_store_cam, 0x7FFFF);
        }

        if(p_captured_para->flow_info_mask.dest_port)
        {
            lport = CTC_DKIT_CTC_GPORT_TO_SYS_LPORT(p_captured_para->flow_info.dest_port);
            gchip = CTC_DKIT_CTC_GPORT_TO_GCHIP(p_captured_para->flow_info.dest_port);
            dest_map = CTC_DKITS_ENCODE_DESTMAP(gchip, lport);
            SetBufStoreDebugCam(V, g_0_destMap_f, &ds_buff_store_cam, dest_map & 0x7FFFF);
            SetBufStoreDebugCam(V, g_1_destMap_f, &ds_buff_store_cam, 0x7FFFF);
        }
        if(p_captured_para->flow_info_mask.port)
        {
            local_phy_port = CTC_DKIT_CTC_GPORT_TO_DRV_LPORT(p_captured_para->flow_info.port);
            SetBufStoreDebugCam(V, g_0_sourcePort_f, &ds_buff_store_cam, local_phy_port);
            SetBufStoreDebugCam(V, g_1_sourcePort_f, &ds_buff_store_cam, 0xFFFF);
        }

    }
    else if (CTC_DKIT_FLOW_MODE_METFIFO == p_captured_para->mode)
    {
        value = 1;
        cmd = DRV_IOW(MetFifoCtl_t, MetFifoCtl_debugLookupEn_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        if(p_captured_para->flow_info_mask.dest_port)
        {
            lport = CTC_DKIT_CTC_GPORT_TO_SYS_LPORT(p_captured_para->flow_info.dest_port);
            gchip = CTC_DKIT_CTC_GPORT_TO_GCHIP(p_captured_para->flow_info.dest_port);
            dest_map = ((1 << 18) | ((((gchip)&0x7F) << 9) | ((lport)&0x1FF)));
            SetMetFifoDebugCam(V, g_0_destMap_f, &ds_met_fifo_cam, dest_map & 0x7FFFF);
            SetMetFifoDebugCam(V, g_1_destMap_f, &ds_met_fifo_cam, 0x7FFFF);
        }
        if (1 == p_captured_para->flow_info_mask.is_debugged_pkt)
        {
            SetMetFifoDebugCam(V, g_0_isDebuggedPkt_f, &ds_met_fifo_cam, p_captured_para->flow_info.is_debugged_pkt);
            SetMetFifoDebugCam(V, g_1_isDebuggedPkt_f, &ds_met_fifo_cam, 0xFF);
        }
    }
    else if (CTC_DKIT_FLOW_MODE_OAM_RX == p_captured_para->mode)
    {
        value = 1;
        cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjDebugUseMepIdx_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        if (1 == p_captured_para->flow_info_mask.mep_index)
        {
            value = p_captured_para->flow_info.mep_index;
            cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjMepIndex_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            value = 0;
            cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjDebugSessionEn_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
        }
        value = 1;
        cmd = DRV_IOW(DbgOamApsSwitch_t, DbgOamApsSwitch_oamApsSwitchDebugUseMepIdx_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        value = p_captured_para->flow_info.mep_index;
        cmd = DRV_IOW(DbgOamApsSwitch_t, DbgOamApsSwitch_oamApsSwitchMepIndex_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        value = 1;
        cmd = DRV_IOW(DbgOamApsProcess_t, DbgOamApsProcess_oamApsProcessDebugUseMepIdx_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        value = p_captured_para->flow_info.mep_index;
        cmd = DRV_IOW(DbgOamApsProcess_t, DbgOamApsProcess_oamApsProcessMepIndex_f);
        DRV_IOCTL(lchip, 0, cmd, &value);


    }
    else if (CTC_DKIT_FLOW_MODE_OAM_TX == p_captured_para->mode)
    {
        value = 1;
        cmd = DRV_IOW(DbgOamTxProc_t, DbgOamTxProc_oamTxProcDebugUseMepIdx_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        if (1 == p_captured_para->flow_info_mask.mep_index)
        {
            value = p_captured_para->flow_info.mep_index;
            cmd = DRV_IOW(DbgOamTxProc_t, DbgOamTxProc_oamTxProcMepIndex_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            value = 0;
            cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjDebugSessionEn_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
        }
    }
    else/*CTC_DKIT_FLOW_MODE_ALL*/
    {
        if (1 == p_captured_para->flow_info_mask.port)
        {
            local_phy_port = CTC_DKIT_CTC_GPORT_TO_DRV_LPORT(p_captured_para->flow_info.port);
            SetIpeUserIdFlowCam(V, g_0_localPhyPort_f, &ds_flow_cam, local_phy_port);  /*key*/
            SetIpeUserIdFlowCam(V, g_1_localPhyPort_f, &ds_flow_cam, 0xFF);            /*mask*/
        }
        if (1 == p_captured_para->flow_info_mask.channel_id_0)
        {
            SetIpeUserIdFlowCam(V, g_0_channelId0_f, &ds_flow_cam, p_captured_para->flow_info.channel_id_0);
            SetIpeUserIdFlowCam(V, g_0_channelId1_f, &ds_flow_cam, 0xFF);
            SetIpeUserIdFlowCam(V, g_0_channelId2_f, &ds_flow_cam, 0xFF);
            SetIpeUserIdFlowCam(V, g_0_channelId3_f, &ds_flow_cam, 0xFF);
            SetIpeUserIdFlowCam(V, g_1_channelId0_f, &ds_flow_cam, 0xFF);
            SetIpeUserIdFlowCam(V, g_1_channelId1_f, &ds_flow_cam, 0xFF);
            SetIpeUserIdFlowCam(V, g_1_channelId2_f, &ds_flow_cam, 0xFF);
            SetIpeUserIdFlowCam(V, g_1_channelId3_f, &ds_flow_cam, 0xFF);
        }
        /*When CTC_DKIT_FLOW_MODE_ALL mode, isDebuggedPkt is used as key*/
        SetBufStoreDebugCam(V, g_0_isDebuggedPkt_f, &ds_buff_store_cam, 1);
        SetBufStoreDebugCam(V, g_1_isDebuggedPkt_f, &ds_buff_store_cam, 0xFF);
        SetMetFifoDebugCam(V, g_0_isDebuggedPkt_f, &ds_met_fifo_cam, 1);
        SetMetFifoDebugCam(V, g_1_isDebuggedPkt_f, &ds_met_fifo_cam, 0xFF);
        SetEpeHdrAdjustDebugCam(V, g_0_isDebuggedPkt_f, &epe_dbg_cam, 1);
        SetEpeHdrAdjustDebugCam(V, g_1_isDebuggedPkt_f, &epe_dbg_cam, 0xFF);
        value = 1;
        cmd = DRV_IOW(DbgOamHdrAdj_t, DbgOamHdrAdj_oamHdrAdjDebug1stPkt_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
        cmd = DRV_IOW(DbgOamTxProc_t, DbgOamTxProc_oamTxProcDebug1stPkt_f);
        DRV_IOCTL(lchip, 0, cmd, &value);
    }

    if ((CTC_DKIT_FLOW_MODE_OAM_TX == p_captured_para->mode) || (CTC_DKIT_FLOW_MODE_OAM_RX == p_captured_para->mode))
    {
        cmd = DRV_IOR(DsEthMep_t, DsEthMep_isBfd_f);
        DRV_IOCTL(lchip, p_captured_para->flow_info.mep_index, cmd, &is_bfd);
        if (is_bfd)
        {
            cmd = DRV_IOR(DsBfdMep_t, DsBfdMep_maIndex_f);
        }
        else
        {
            cmd = DRV_IOR(DsEthMep_t, DsEthMep_maIndex_f);
        }
        DRV_IOCTL(lchip, p_captured_para->flow_info.mep_index, cmd, &ma_index);
        value = p_captured_para->flow_info.oam_dbg_en ? 1:0;
        cmd = DRV_IOW(DsMa_t, DsMa_debugSessionEn_f);
        DRV_IOCTL(lchip, ma_index, cmd, &value);
    }

    /*2. store flow by parser result*/
    if (1 == p_captured_para->flow_info_mask.mac_da[0])
    {
        CTC_DKIT_SET_HW_MAC(mac_da, p_captured_para->flow_info.mac_da);
        SetParserResult(A, macDa_f, &parser_result, mac_da);
        SetParserResult(A, macDa_f, &parser_result_mask, mac_mask);

    }
    if (1 == p_captured_para->flow_info_mask.mac_sa[0])
    {
        CTC_DKIT_SET_HW_MAC(mac_sa, p_captured_para->flow_info.mac_sa);
        SetParserResult(A, macSa_f, &parser_result, mac_sa);
        SetParserResult(A, macSa_f, &parser_result_mask, mac_mask);
    }
    if (1 == p_captured_para->flow_info_mask.svlan_id)
    {
        SetParserResult(V, svlanId_f, &parser_result, p_captured_para->flow_info.svlan_id);
        SetParserResult(V, svlanId_f, &parser_result_mask, 0xFFF);
        SetParserResult(V, svlanIdValid_f, &parser_result, 1);
        SetParserResult(V, svlanIdValid_f, &parser_result_mask, 1);
    }
    if (1 == p_captured_para->flow_info_mask.cvlan_id)
    {
        SetParserResult(V, cvlanId_f, &parser_result, p_captured_para->flow_info.cvlan_id);
        SetParserResult(V, cvlanId_f, &parser_result_mask, 0xFFF);
        SetParserResult(V, cvlanIdValid_f, &parser_result, 1);
        SetParserResult(V, cvlanIdValid_f, &parser_result_mask, 1);
    }
    if (1 == p_captured_para->flow_info_mask.ether_type)
    {
        SetParserResult(V, etherType_f, &parser_result, p_captured_para->flow_info.ether_type);
        SetParserResult(V, etherType_f, &parser_result_mask, 0xFFFF);
    }
    if (1 == p_captured_para->flow_info_mask.l3_type)
    {
        switch (p_captured_para->flow_info.l3_type)
        {
        case CTC_DKIT_CAPTURED_IPV4:
            if (1 == p_captured_para->flow_info_mask.u_l3.ipv4.ip_da)
            {
                SetParserResult(V, uL3Dest_gIpv4_ipDa_f, &parser_result, p_captured_para->flow_info.u_l3.ipv4.ip_da);
                SetParserResult(V, uL3Dest_gIpv4_ipDa_f, &parser_result_mask, 0xFFFFFFFF);
            }

            if (1 == p_captured_para->flow_info_mask.u_l3.ipv4.ip_sa)
            {
                SetParserResult(V, uL3Source_gIpv4_ipSa_f, &parser_result, p_captured_para->flow_info.u_l3.ipv4.ip_sa);
                SetParserResult(V, uL3Source_gIpv4_ipSa_f, &parser_result_mask, 0xFFFFFFFF);
            }
            if (1 == p_captured_para->flow_info_mask.u_l3.ipv4.ip_check_sum)
            {
                SetParserResult(V, uL3Source_gIpv4_ipChecksum_f, &parser_result, p_captured_para->flow_info.u_l3.ipv4.ip_check_sum);
                SetParserResult(V, uL3Source_gIpv4_ipChecksum_f, &parser_result_mask, 0xFFFF);
            }
            break;
        case CTC_DKIT_CAPTURED_IPV6:
            if (1 == p_captured_para->flow_info_mask.u_l3.ipv6.ip_da[0])
            {
                CTC_DKITS_IPV6_ADDR_SORT(p_captured_para->flow_info.u_l3.ipv6.ip_da);
                SetParserResult(A, uL3Dest_gIpv6_ipDa_f, &parser_result, p_captured_para->flow_info.u_l3.ipv6.ip_da);
                SetParserResult(A, uL3Dest_gIpv6_ipDa_f, &parser_result_mask, ipv6_mask);
            }
            if (1 == p_captured_para->flow_info_mask.u_l3.ipv6.ip_sa[0])
            {
                CTC_DKITS_IPV6_ADDR_SORT(p_captured_para->flow_info.u_l3.ipv6.ip_sa);
                SetParserResult(A, uL3Source_gIpv6_ipSa_f, &parser_result, p_captured_para->flow_info.u_l3.ipv6.ip_sa);
                SetParserResult(A, uL3Source_gIpv6_ipSa_f, &parser_result_mask, ipv6_mask);
            }
            break;
        case CTC_DKIT_CAPTURED_MPLS:
            if (1 == p_captured_para->flow_info_mask.u_l3.mpls.mpls_label0)/*4 bytes MPLS Header, not 20bits label value*/
            {
                SetParserResult(V, uL3Dest_gMpls_mplsLabel0_f, &parser_result, p_captured_para->flow_info.u_l3.mpls.mpls_label0);
                SetParserResult(V, uL3Dest_gMpls_mplsLabel0_f, &parser_result_mask, 0xFFFFFFFF);
            }
            if (1 == p_captured_para->flow_info_mask.u_l3.mpls.mpls_label1)
            {
                SetParserResult(V, uL3Dest_gMpls_mplsLabel1_f, &parser_result, p_captured_para->flow_info.u_l3.mpls.mpls_label1);
                SetParserResult(V, uL3Dest_gMpls_mplsLabel1_f, &parser_result_mask, 0xFFFFFFFF);
            }
            if (1 == p_captured_para->flow_info_mask.u_l3.mpls.mpls_label2)
            {
                SetParserResult(V, uL3Dest_gMpls_mplsLabel2_f, &parser_result, p_captured_para->flow_info.u_l3.mpls.mpls_label2);
                SetParserResult(V, uL3Dest_gMpls_mplsLabel2_f, &parser_result_mask, 0xFFFFFFFF);
            }
            if (1 == p_captured_para->flow_info_mask.u_l3.mpls.mpls_label3)
            {
                SetParserResult(V, uL3Dest_gMpls_mplsLabel3_f, &parser_result, p_captured_para->flow_info.u_l3.mpls.mpls_label3);
                SetParserResult(V, uL3Dest_gMpls_mplsLabel3_f, &parser_result_mask, 0xFFFFFFFF);
            }
            break;
        case CTC_DKIT_CAPTURED_SLOW_PROTOCOL:
            if (1 == p_captured_para->flow_info_mask.u_l3.slow_proto.sub_type)
            {
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolSubType_f, &parser_result, p_captured_para->flow_info.u_l3.slow_proto.sub_type);
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolSubType_f, &parser_result_mask, 0xFF);
            }
            if (1 == p_captured_para->flow_info_mask.u_l3.slow_proto.flags)
            {
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolFlags_f, &parser_result, p_captured_para->flow_info.u_l3.slow_proto.flags);
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolFlags_f, &parser_result_mask, 0xFFFF);
            }
            if (1 == p_captured_para->flow_info_mask.u_l3.slow_proto.code)
            {
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolCode_f, &parser_result, p_captured_para->flow_info.u_l3.slow_proto.code);
                SetParserResult(V, uL3Dest_gSlowProto_slowProtocolCode_f, &parser_result_mask, 0xFF);
            }
            break;
        case CTC_DKIT_CAPTURED_ETHOAM:
            if (1 == p_captured_para->flow_info_mask.u_l3.ether_oam.level)
            {
                SetParserResult(V, uL3Dest_gEtherOam_etherOamLevel_f, &parser_result, p_captured_para->flow_info.u_l3.ether_oam.level);
                SetParserResult(V, uL3Dest_gEtherOam_etherOamLevel_f, &parser_result_mask, 0x7);
            }
            if (1 == p_captured_para->flow_info_mask.u_l3.ether_oam.version)
            {
                SetParserResult(V, uL3Dest_gEtherOam_etherOamVersion_f, &parser_result, p_captured_para->flow_info.u_l3.ether_oam.version);
                SetParserResult(V, uL3Dest_gEtherOam_etherOamVersion_f, &parser_result_mask, 0x1F);
            }
            if (1 == p_captured_para->flow_info_mask.u_l3.ether_oam.opcode)
            {
                SetParserResult(V, uL3Dest_gEtherOam_etherOamOpCode_f, &parser_result, p_captured_para->flow_info.u_l3.ether_oam.opcode);
                SetParserResult(V, uL3Dest_gEtherOam_etherOamOpCode_f, &parser_result_mask, 0xFF);
            }
            break;
        default:
            break;
        }
    }
    if (1 == p_captured_para->flow_info_mask.l4_type)
    {
        switch (p_captured_para->flow_info.l4_type)
        {
            case CTC_DKIT_CAPTURED_L4_SOURCE_PORT:
                if (1 == p_captured_para->flow_info_mask.u_l4.l4port.source_port)
                {
                    SetParserResult(V, uL4Source_gPort_l4SourcePort_f, &parser_result, p_captured_para->flow_info.u_l4.l4port.source_port);
                    SetParserResult(V, uL4Source_gPort_l4SourcePort_f, &parser_result_mask, 0xFFFF);
                }
                break;
            case CTC_DKIT_CAPTURED_L4_DEST_PORT:
                if (1 == p_captured_para->flow_info_mask.u_l4.l4port.dest_port)
                {
                    SetParserResult(V, uL4Dest_gPort_l4DestPort_f, &parser_result, p_captured_para->flow_info.u_l4.l4port.dest_port);
                    SetParserResult(V, uL4Dest_gPort_l4DestPort_f, &parser_result_mask, 0xFFFF);
                }
                break;
            default:
                break;
        }
    }
    SetIpeUserIdFlowCam(A, g_0_data_f, &ds_flow_cam, &parser_result);
    SetIpeUserIdFlowCam(A, g_1_data_f, &ds_flow_cam, &parser_result_mask);
    cmd = DRV_IOW(IpeUserIdFlowCam_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ds_flow_cam);

    cmd = DRV_IOW(IpeUserIdFlowLoopCam_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ds_flow_loop_cam);

    cmd = DRV_IOW(BufStoreDebugCam_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ds_buff_store_cam);

    cmd = DRV_IOW(MetFifoDebugCam_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ds_met_fifo_cam);

    cmd = DRV_IOW(EpeHdrAdjustDebugCam_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &epe_dbg_cam);

    return CLI_SUCCESS;
}

extern int32
_ctc_usw_dkit_memory_show_ram_by_data_tbl_id( uint32 tbl_id, uint32 index, uint32* data_entry, char* regex, uint8 acc_io);
int32
ctc_usw_dkit_captured_show_bridge_header(uint8 lchip, uint8 mode)
{
    uint32 cmd = 0;
    uint32 header_valid   = 0;

    MsPacketHeader_m ms_pkt_hdr;
    DbgFwdBufRetrvHeaderInfo_m dbg_buf_retrv_header;
    DbgFwdBufStoreInfo2_m      dbg_buf_store_info;

    sal_memset(&ms_pkt_hdr, 0, sizeof(MsPacketHeader_m));
    sal_memset(&dbg_buf_retrv_header, 0, sizeof(DbgFwdBufRetrvHeaderInfo_m));
    sal_memset(&dbg_buf_store_info, 0, sizeof(DbgFwdBufStoreInfo2_m));

    CTC_DKIT_PRINT("\nBridge Header Info\n-----------------------------------\n");

    if(0 == mode)
    {
        cmd = DRV_IOR(DbgFwdBufRetrvHeaderInfo_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &dbg_buf_retrv_header);
        GetDbgFwdBufRetrvHeaderInfo(A, valid_f, &dbg_buf_retrv_header, &header_valid);
        GetDbgFwdBufRetrvHeaderInfo(A, packetHeader_f, &dbg_buf_retrv_header, &ms_pkt_hdr);
    }
    else if(1 == mode)
    {
        cmd = DRV_IOR(DbgFwdBufStoreInfo2_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &dbg_buf_store_info);
        GetDbgFwdBufStoreInfo2(A, valid_f, &dbg_buf_store_info, &header_valid);
        GetDbgFwdBufStoreInfo2(A, packetHeader_f, &dbg_buf_store_info, &ms_pkt_hdr);
    }

    if(header_valid)
    {
        _ctc_usw_dkit_memory_show_ram_by_data_tbl_id(MsPacketHeader_t, 0, (uint32*)(&ms_pkt_hdr), NULL, 0);
    }
    else
    {
        CTC_DKIT_PATH_PRINT("Header Info not Invalid!\n");
    }


    return CLI_SUCCESS;
}

