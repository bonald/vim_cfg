/**
  @file drv_acc.c

  @date 2010-02-26

  @version v5.1

  The file implement driver acc IOCTL defines and macros
*/

#include "sal.h"
#include "drv_api.h"
#include "usw/include/drv_ftm.h"
#include "usw/include/drv_common.h"
#include "usw/include/drv_error.h"
#include "usw/include/drv_enum.h"
#include "usw/include/drv_chip_agent.h"  /*delete later, TBD*/
#include "usw/include/drv_ser_db.h"
/**********************************************************************************
              Define Gloabal var, Typedef, define and Data Structure
***********************************************************************************/
#ifdef EMULATION_ENV
#define DRV_APP_LOOP_CNT 2000
#else
#define DRV_APP_LOOP_CNT (((1 == SDK_WORK_PLATFORM) || (0 == SDK_WORK_PLATFORM) )?2000:200)
#endif

#define DRV_DBG_OUT(fmt, args...)       if(0){sal_printf(fmt, ##args);}

enum hash_module_e
{
    HASH_MODULE_EGRESS_XC,
    HASH_MODULE_FIB_HOST0,
    HASH_MODULE_FIB_HOST1,
    HASH_MODULE_FLOW,
    HASH_MODULE_IPFIX,
    HASH_MODULE_USERID,
    HASH_MODULE_NUM,
};
typedef enum hash_module_e hash_module_t;

enum drv_acc_chip_type_e
{
    DRV_ACC_CHIP_MAC_WRITE_BY_INDEX,       /*0*/
    DRV_ACC_CHIP_MAC_WRITE_BY_KEY,         /*1*/
    DRV_ACC_CHIP_MAC_DEL_BY_INDEX,         /*2*/
    DRV_ACC_CHIP_MAC_DEL_BY_KEY,           /*3*/
    DRV_ACC_CHIP_MAC_FLUSH_BY_PORT_VLAN,   /*4*/
    DRV_ACC_CHIP_MAC_FLUSH_DUMP_BY_ALL,    /*5*/
    DRV_ACC_CHIP_MAC_LOOKUP_BY_KEY,        /*6*/
    DRV_ACC_CHIP_HOST0_lOOKUP_BY_KEY,      /*7*/
    DRV_ACC_CHIP_HOST0_lOOKUP_BY_INDEX,    /*8*/
    DRV_ACC_CHIP_HOST0_WRITE_BY_INDEX,     /*9*/
    DRV_ACC_CHIP_HOST0_WRITE_BY_KEY,       /*10*/
    DRV_ACC_CHIP_LIMIT_UPDATE,             /*11*/
    DRV_ACC_CHIP_LIMIT_READ,               /*12*/
    DRV_ACC_CHIP_LIMIT_WRITE,              /*13*/
    DRV_ACC_CHIP_MAC_FLUSH_BY_MAC_PORT,    /*14*/
    DRV_ACC_CHIP_MAC_DUMP_BY_PORT_VLAN,    /*15*/
    DRV_ACC_CHIP_WRITE_DSMAC,              /*16*/
    DRV_ACC_CHIP_WRITE_DSAGING,            /*17*/
    DRV_ACC_CHIP_MAX_TYPE
};
typedef enum drv_acc_chip_type_e drv_acc_chip_type_t;

enum drv_ipfix_acc_req_type_e
{
    DRV_IPFIX_REQ_WRITE_BY_IDX,
    DRV_IPFIX_REQ_WRITE_BY_KEY,
    DRV_IPFIX_REQ_LKP_BY_KEY,
    DRV_IPFIX_REQ_LKP_BY_IDX
};
typedef enum drv_ipfix_acc_req_type_e drv_ipfix_acc_req_type_t;

struct key_lookup_result_s
{
   uint32 bucket_index;
};
typedef struct key_lookup_result_s key_lookup_result_t;

struct key_lookup_info_s
{
    uint8* key_data;
    uint32 key_bits;
    uint32 polynomial;
    uint32 poly_len;
    uint32 type;
    uint32 crc_bits;
};
typedef struct key_lookup_info_s key_lookup_info_t;


#define DRV_FIB_ACC_SLEEP_TIME    1

#define CPU_ACC_LOCK(lchip)    \
  if(p_drv_master[lchip]->cpu_acc_mutex) {sal_mutex_lock(p_drv_master[lchip]->cpu_acc_mutex);}
#define CPU_ACC_UNLOCK(lchip)  \
  if(p_drv_master[lchip]->cpu_acc_mutex) {sal_mutex_unlock(p_drv_master[lchip]->cpu_acc_mutex);}
#define IPFIX_ACC_LOCK(lchip)  \
  if(p_drv_master[lchip]->ipfix_acc_mutex) {sal_mutex_lock(p_drv_master[lchip]->ipfix_acc_mutex);}
#define IPFIX_ACC_UNLOCK(lchip)\
  if(p_drv_master[lchip]->ipfix_acc_mutex) {sal_mutex_unlock(p_drv_master[lchip]->ipfix_acc_mutex);}
#define CID_ACC_UNLOCK(lchip)\
  if(p_drv_master[lchip]->cid_acc_mutex) {sal_mutex_unlock(p_drv_master[lchip]->cid_acc_mutex);}
#define CID_ACC_LOCK(lchip)  \
  if(p_drv_master[lchip]->cid_acc_mutex) {sal_mutex_lock(p_drv_master[lchip]->cid_acc_mutex);}

#define MPLS_ACC_LOCK(lchip)  \
  if(p_drv_master[lchip]->mpls_acc_mutex) {sal_mutex_lock(p_drv_master[lchip]->mpls_acc_mutex);}
#define MPLS_ACC_UNLOCK(lchip)  \
  if(p_drv_master[lchip]->mpls_acc_mutex) {sal_mutex_unlock(p_drv_master[lchip]->mpls_acc_mutex);}

#define GEMPORT_ACC_LOCK(lchip)  \
  if(p_drv_master[lchip]->gemport_acc_mutex) {sal_mutex_lock(p_drv_master[lchip]->gemport_acc_mutex);}
#define GEMPORT_ACC_UNLOCK(lchip)  \
  if(p_drv_master[lchip]->gemport_acc_mutex) {sal_mutex_unlock(p_drv_master[lchip]->gemport_acc_mutex);}




#define SINGLE_KEY_BYTE           sizeof(DsFibHost0MacHashKey_m)
#define DOUBLE_KEY_BYTE           sizeof(DsFibHost0Ipv6UcastHashKey_m)
#define QUAD_KEY_BYTE             sizeof(DsFibHost0HashCam_m)
#define DS_USER_ID_HASH_CAM_BYTES sizeof(DsUserIdHashCam_m)

extern int32
drv_model_hash_combined_key(uint8* dest_key, uint8* src_key, uint32 len, uint32 tbl_id);
extern int32
drv_model_hash_mask_key(uint8* dest_key, uint8* src_key, hash_module_t hash_module, uint32 key_type);
extern int32
drv_model_hash_un_combined_key(uint8* dest_key, uint8* src_key, uint32 len, uint32 tbl_id);

int32
drv_usw_acc_host0_prepare(uint8 lchip, drv_acc_in_t* acc_in, uint32* req);
int32
drv_usw_acc_host0_process(uint8 lchip, uint32* i_cpu_req, uint32* o_cpu_rlt);
int32
drv_usw_acc_host0(uint8 lchip, drv_acc_in_t* acc_in, drv_acc_out_t* acc_out);

int32
drv_usw_acc_host0_result(uint8 lchip, drv_acc_in_t* acc_in, uint32* fib_acc_cpu_rlt, drv_acc_out_t* out);


uint32 g_usw_drv_io_is_asic;         /*delete later, TBD*/


typedef struct
{
    uint32 key_index;
    uint32 tbl_id;
    uint8 is_read;
    uint8 cam_num;
    uint8 is_left;     /*for cid acc*/
    uint8 is_high;     /*for cid acc*/
    void* data;
    uint8  is_cam;
    uint8  cam_step;
    uint8  conflict;  /*for asic lookup result*/
    uint8  valid;     /*for asic lookup result*/
    uint32 cam_table;
    uint8   hash_type;
}drv_cpu_acc_prepare_info_t;

uint32
drv_hash_crc(uint32 seed, uint8 *data, uint32 bit_len, uint32 poly, uint32 poly_len)
{
    uint32 crc = 0;
    int32 i = 0;
    uint32 msb_bit = 0;
    uint32 poly_mask = 0;
    uint32 topone=0;
    poly_mask = (1<<poly_len) -1;
    topone = (1 << (poly_len-1));
    crc = seed & poly_mask;

    for (i = (bit_len-1); i >=0; i--) {  /* bits*/
      msb_bit = ((data[i/8]>>(i%8))&0x1) << (poly_len-1);
      crc ^= msb_bit;
      crc = (crc << 1) ^ ((crc & topone) ? poly: 0);
       /*bit_len --;*/
    }

    crc &= poly_mask;
    return crc;
}

uint16
drv_hash_xor16(uint16* data, uint32 bit_len)
{
    uint32 i = 0;
    uint16 result = 0;
    uint16 len = bit_len/16;
    uint16 last_bits = bit_len%16;

    for (i = 0; i < len; i++)
    {
        result ^= data[i];
    }

    if (last_bits)
    {
        result ^= (data[len]&((1<<last_bits)-1));
    }

    return result;
}

int32
drv_usw_get_cam_info(uint8 lchip, uint8 hash_module, uint32* tbl_id, uint8* num)
{
    if (hash_module == DRV_ACC_HASH_MODULE_FIB_HOST1)
    {
        *tbl_id = DsFibHost1HashCam_t;
        *num = FIB_HOST1_CAM_NUM;
    }
    else if (hash_module == DRV_ACC_HASH_MODULE_EGRESS_XC)
    {
        *tbl_id = DsEgressXcOamHashCam_t;
        *num = XC_OAM_HASH_CAM_NUM;
    }
    else if (hash_module == DRV_ACC_HASH_MODULE_FLOW)
    {
        *tbl_id = DsFlowHashCam_t;
        *num = FLOW_HASH_CAM_NUM;
    }
    else if (hash_module == DRV_ACC_HASH_MODULE_USERID)
    {
        *tbl_id = DsUserIdHashCam_t;
        if (DRV_IS_DUET2(lchip))
        {
            *num = USER_ID_HASH_CAM_NUM;
        }
    }
    else if (hash_module == DRV_ACC_HASH_MODULE_FIB_HOST0)
    {
        *tbl_id = (DRV_IS_DUET2(lchip)) ? DsFibHost0HashCam_t : DsFibHost0HashIpCam_t;
        *num = FIB_HOST0_CAM_NUM;
    }
    else if(hash_module == DRV_ACC_HASH_MODULE_FDB)
    {
        *tbl_id = (DRV_IS_DUET2(lchip)) ? DsFibHost0HashCam_t : DsFibHost0HashMacCam_t;
        *num = FIB_HOST0_CAM_NUM;
    }
    else if (hash_module == DRV_ACC_HASH_MODULE_MPLS_HASH)
    {
        *tbl_id = DsMplsHashCam_t;
        *num = 64;
    }
    else if (hash_module == DRV_ACC_HASH_MODULE_GEMPORT_HASH)
    {
        *tbl_id = DsGemHashCam_t;
        *num = 64;
    }
    else
    {
        return DRV_E_INVAILD_TYPE;
    }

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_hash_get_valid(uint8 lchip, uint8 hash_module, uint32* p_valid)
{
    uint32 valid = 0;
    uint32 cmd = 0;
    uint32 data[MAX_ENTRY_BYTE / 4] = { 0 };

    if (DRV_ACC_HASH_MODULE_FIB_HOST1 == hash_module)
    {
        cmd = DRV_IOR(FibHost1FlowHashCpuLookupReq_t, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, data));
        valid = GetFibHost1FlowHashCpuLookupReq(V, cpuLookupReqValid_f, data);
    }
    else if (DRV_ACC_HASH_MODULE_USERID == hash_module)
    {
        cmd = DRV_IOR(UserIdCpuLookupReq_t, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, data));
        valid = GetUserIdCpuLookupReq(V, cpuLookupReqValid_f, data);
    }
    else if (DRV_ACC_HASH_MODULE_EGRESS_XC == hash_module)
    {
        cmd = DRV_IOR(EgressXcOamCpuLookupReq_t, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, data));
        valid = GetEgressXcOamCpuLookupReq(V, cpuLookupReqValid_f, data);
    }
    else if (DRV_ACC_HASH_MODULE_FLOW == hash_module)
    {
        cmd = DRV_IOR(FibHost1FlowHashCpuLookupReq_t, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, data));
        valid = GetFibHost1FlowHashCpuLookupReq(V, cpuLookupReqValid_f, data);
    }
    else if (DRV_ACC_HASH_MODULE_CID == hash_module)
    {
        cmd = DRV_IOR(CidPairHashCpuLookupReq_t, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, data));
        valid = GetCidPairHashCpuLookupReq(V, cpuLookupReqValid_f, data);
    }
    else if (DRV_ACC_HASH_MODULE_FIB_HASH == hash_module)
    {
        cmd = DRV_IOR(FibHashCpuLookupReq_t, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, data));
        valid = GetFibHashCpuLookupReq(V, cpuLookupReqValid_f, data);
    }
    else if (DRV_ACC_HASH_MODULE_MPLS_HASH == hash_module)
    {
        cmd = DRV_IOR(MplsCpuLookupReq_t, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, data));
        valid = GetMplsCpuLookupReq(V, mplsCpuLookupReqValid_f, data);
        DRV_DBG_OUT("GetMplsCpuLookupReq valid  = %d\n", valid);
    }
    else if (DRV_ACC_HASH_MODULE_GEMPORT_HASH == hash_module)
    {
        cmd = DRV_IOR(GemPortCpuLookupReq_t, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, data));
        valid = GetGemPortCpuLookupReq(V, lookupValid_f, data);
    }


    *p_valid = valid;

    return  DRV_E_NONE;
}

int32
drv_usw_acc_get_hash_module_from_tblid(uint8 lchip, uint32 tbl_id, uint8* p_module)
{
    switch (tbl_id)
    {
        case DsFibHost0MacHashKey_t:
            *p_module = DRV_ACC_HASH_MODULE_FDB;
            break;

        case DsFibHost0FcoeHashKey_t:
        case DsFibHost0Ipv4HashKey_t:
        case DsFibHost0Ipv6McastHashKey_t:
        case DsFibHost0Ipv6UcastHashKey_t:
        case DsFibHost0MacIpv6McastHashKey_t:
        case DsFibHost0TrillHashKey_t:
            *p_module = DRV_ACC_HASH_MODULE_FIB_HOST0;
            break;

        case DsFibHost1FcoeRpfHashKey_t:
        case DsFibHost1Ipv4McastHashKey_t:
        case DsFibHost1Ipv4NatDaPortHashKey_t:
        case DsFibHost1Ipv4NatSaPortHashKey_t:
        case DsFibHost1Ipv6McastHashKey_t:
        case DsFibHost1Ipv6NatDaPortHashKey_t:
        case DsFibHost1Ipv6NatSaPortHashKey_t:
        case DsFibHost1MacIpv4McastHashKey_t:
        case DsFibHost1MacIpv6McastHashKey_t:
        case DsFibHost1TrillMcastVlanHashKey_t:
            *p_module = DRV_ACC_HASH_MODULE_FIB_HOST1;
            break;

        case DsEgressXcOamBfdHashKey_t:
        case DsEgressXcOamCvlanCosPortHashKey_t:
        case DsEgressXcOamCvlanPortHashKey_t:
        case DsEgressXcOamDoubleVlanPortHashKey_t:
        case DsEgressXcOamEthHashKey_t:
        case DsEgressXcOamMplsLabelHashKey_t:
        case DsEgressXcOamMplsSectionHashKey_t:
        case DsEgressXcOamPortCrossHashKey_t:
        case DsEgressXcOamPortHashKey_t:
        case DsEgressXcOamPortVlanCrossHashKey_t:
        case DsEgressXcOamRmepHashKey_t:
        case DsEgressXcOamSvlanCosPortHashKey_t:
        case DsEgressXcOamSvlanPortHashKey_t:
        case DsEgressXcOamSvlanPortMacHashKey_t:
        case DsEgressXcOamTunnelPbbHashKey_t:
            *p_module = DRV_ACC_HASH_MODULE_EGRESS_XC;
            break;

        case DsFlowL2HashKey_t:
        case DsFlowL2L3HashKey_t:
        case DsFlowL3Ipv4HashKey_t:
        case DsFlowL3Ipv6HashKey_t:
        case DsFlowL3MplsHashKey_t:
            *p_module = DRV_ACC_HASH_MODULE_FLOW;
            break;

        case DsUserIdCapwapMacDaForwardHashKey_t:
        case DsUserIdCapwapStaStatusHashKey_t:
        case DsUserIdCapwapStaStatusMcHashKey_t:
        case DsUserIdCapwapVlanForwardHashKey_t:
        case DsUserIdCvlanCosPortHashKey_t:
        case DsUserIdCvlanPortHashKey_t:
        case DsUserIdDoubleVlanPortHashKey_t:
        case DsUserIdEcidNameSpaceHashKey_t:
        case DsUserIdIngEcidNameSpaceHashKey_t:
        case DsUserIdIpv4PortHashKey_t:
        case DsUserIdIpv4SaHashKey_t:
        case DsUserIdIpv6PortHashKey_t:
        case DsUserIdIpv6SaHashKey_t:
        case DsUserIdMacHashKey_t:
        case DsUserIdMacPortHashKey_t:
        case DsUserIdPortHashKey_t:
        case DsUserIdSclFlowL2HashKey_t:
        case DsUserIdSvlanCosPortHashKey_t:
        case DsUserIdSvlanHashKey_t:
        case DsUserIdSvlanMacSaHashKey_t:
        case DsUserIdSvlanPortHashKey_t:
        case DsUserIdTunnelCapwapRmacHashKey_t:
        case DsUserIdTunnelCapwapRmacRidHashKey_t:
        case DsUserIdTunnelIpv4CapwapHashKey_t:
        case DsUserIdTunnelIpv4DaHashKey_t:
        case DsUserIdTunnelIpv4GreKeyHashKey_t:
        case DsUserIdTunnelIpv4HashKey_t:
        case DsUserIdTunnelIpv4McNvgreMode0HashKey_t:
        case DsUserIdTunnelIpv4McVxlanMode0HashKey_t:
        case DsUserIdTunnelIpv4NvgreMode1HashKey_t:
        case DsUserIdTunnelIpv4RpfHashKey_t:
        case DsUserIdTunnelIpv4UcNvgreMode0HashKey_t:
        case DsUserIdTunnelIpv4UcNvgreMode1HashKey_t:
        case DsUserIdTunnelIpv4UcVxlanMode0HashKey_t:
        case DsUserIdTunnelIpv4UcVxlanMode1HashKey_t:
        case DsUserIdTunnelIpv4UdpHashKey_t:
        case DsUserIdTunnelIpv4VxlanMode1HashKey_t:
        case DsUserIdTunnelIpv6DaHashKey_t:
        case DsUserIdTunnelIpv6GreKeyHashKey_t:
        case DsUserIdTunnelIpv6HashKey_t:
        case DsUserIdTunnelIpv6UdpHashKey_t:
        case DsUserIdTunnelIpv6CapwapHashKey_t:
        case DsUserIdTunnelIpv6McNvgreMode0HashKey_t:
        case DsUserIdTunnelIpv6McNvgreMode1HashKey_t:
        case DsUserIdTunnelIpv6McVxlanMode0HashKey_t:
        case DsUserIdTunnelIpv6McVxlanMode1HashKey_t:
        case DsUserIdTunnelIpv6UcNvgreMode0HashKey_t:
        case DsUserIdTunnelIpv6UcNvgreMode1HashKey_t:
        case DsUserIdTunnelIpv6UcVxlanMode0HashKey_t:
        case DsUserIdTunnelIpv6UcVxlanMode1HashKey_t:
        case DsUserIdTunnelMplsHashKey_t:
        case DsUserIdTunnelPbbHashKey_t:
        case DsUserIdTunnelTrillMcAdjHashKey_t:
        case DsUserIdTunnelTrillMcDecapHashKey_t:
        case DsUserIdTunnelTrillMcRpfHashKey_t:
        case DsUserIdTunnelTrillUcDecapHashKey_t:
        case DsUserIdTunnelTrillUcRpfHashKey_t:
        /**< [TM] */
        case DsUserId1CapwapMacDaForwardHashKey_t:
        case DsUserId1CapwapStaStatusHashKey_t:
        case DsUserId1CapwapStaStatusMcHashKey_t:
        case DsUserId1CapwapVlanForwardHashKey_t:
        case DsUserId1CvlanCosPortHashKey_t:
        case DsUserId1CvlanPortHashKey_t:
        case DsUserId1DoubleVlanPortHashKey_t:
        case DsUserId1EcidNameSpaceHashKey_t:
        case DsUserId1IngEcidNameSpaceHashKey_t:
        case DsUserId1Ipv4PortHashKey_t:
        case DsUserId1Ipv4SaHashKey_t:
        case DsUserId1Ipv6PortHashKey_t:
        case DsUserId1Ipv6SaHashKey_t:
        case DsUserId1MacHashKey_t:
        case DsUserId1MacPortHashKey_t:
        case DsUserId1PortHashKey_t:
        case DsUserId1SclFlowL2HashKey_t:
        case DsUserId1SvlanCosPortHashKey_t:
        case DsUserId1SvlanHashKey_t:
        case DsUserId1SvlanMacSaHashKey_t:
        case DsUserId1SvlanPortHashKey_t:
        case DsUserId1TunnelCapwapRmacHashKey_t:
        case DsUserId1TunnelCapwapRmacRidHashKey_t:
        case DsUserId1TunnelIpv4CapwapHashKey_t:
        case DsUserId1TunnelIpv4DaHashKey_t:
        case DsUserId1TunnelIpv4GreKeyHashKey_t:
        case DsUserId1TunnelIpv4HashKey_t:
        case DsUserId1TunnelIpv4McNvgreMode0HashKey_t:
        case DsUserId1TunnelIpv4McVxlanMode0HashKey_t:
        case DsUserId1TunnelIpv4NvgreMode1HashKey_t:
        case DsUserId1TunnelIpv4RpfHashKey_t:
        case DsUserId1TunnelIpv4UcNvgreMode0HashKey_t:
        case DsUserId1TunnelIpv4UcNvgreMode1HashKey_t:
        case DsUserId1TunnelIpv4UcVxlanMode0HashKey_t:
        case DsUserId1TunnelIpv4UcVxlanMode1HashKey_t:
        case DsUserId1TunnelIpv4UdpHashKey_t:
        case DsUserId1TunnelIpv4VxlanMode1HashKey_t:
        case DsUserId1TunnelIpv6DaHashKey_t:
        case DsUserId1TunnelIpv6GreKeyHashKey_t:
        case DsUserId1TunnelIpv6HashKey_t:
        case DsUserId1TunnelIpv6UdpHashKey_t:
        case DsUserId1TunnelIpv6CapwapHashKey_t:
        case DsUserId1TunnelIpv6McNvgreMode0HashKey_t:
        case DsUserId1TunnelIpv6McNvgreMode1HashKey_t:
        case DsUserId1TunnelIpv6McVxlanMode0HashKey_t:
        case DsUserId1TunnelIpv6McVxlanMode1HashKey_t:
        case DsUserId1TunnelIpv6UcNvgreMode0HashKey_t:
        case DsUserId1TunnelIpv6UcNvgreMode1HashKey_t:
        case DsUserId1TunnelIpv6UcVxlanMode0HashKey_t:
        case DsUserId1TunnelIpv6UcVxlanMode1HashKey_t:
        case DsUserId1TunnelTrillMcAdjHashKey_t:
        case DsUserId1TunnelTrillMcDecapHashKey_t:
        case DsUserId1TunnelTrillMcRpfHashKey_t:
        case DsUserId1TunnelTrillUcDecapHashKey_t:
        case DsUserId1TunnelTrillUcRpfHashKey_t:
            *p_module = DRV_ACC_HASH_MODULE_USERID;
            break;

        case DsIpfixL2HashKey_t:
        case DsIpfixL2L3HashKey_t:
        case DsIpfixL3Ipv4HashKey_t:
        case DsIpfixL3Ipv6HashKey_t:
        case DsIpfixL3MplsHashKey_t:
        case DsIpfixSessionRecord_t:
        case DsIpfixL2HashKey0_t:
        case DsIpfixL2L3HashKey0_t:
        case DsIpfixL3Ipv4HashKey0_t:
        case DsIpfixL3Ipv6HashKey0_t:
        case DsIpfixL3MplsHashKey0_t:
        case DsIpfixSessionRecord0_t:
        case DsIpfixL2HashKey1_t:
        case DsIpfixL2L3HashKey1_t:
        case DsIpfixL3Ipv4HashKey1_t:
        case DsIpfixL3Ipv6HashKey1_t:
        case DsIpfixL3MplsHashKey1_t:
        case DsIpfixSessionRecord1_t:
            *p_module = DRV_ACC_HASH_MODULE_IPFIX;
            break;

        case DsMacLimitCount_t:
        case DsMacSecurity_t:
            *p_module = DRV_ACC_HASH_MODULE_MAC_LIMIT;
            break;

        case DsCategoryIdPairHashLeftKey_t:
        case DsCategoryIdPairHashRightKey_t:
            *p_module = DRV_ACC_HASH_MODULE_CID;
            break;

        case DsAging_t:
        case DsAgingStatusFib_t:
        case DsAgingStatusTcam_t:
            *p_module = DRV_ACC_HASH_MODULE_AGING;
            break;

        case DsMplsLabelHashKey_t:
           *p_module = DRV_ACC_HASH_MODULE_MPLS_HASH;
            break;

        case DsGemPortHashKey_t:
           *p_module = DRV_ACC_HASH_MODULE_GEMPORT_HASH;
            break;

        default:
            return DRV_E_INVAILD_TYPE;
    }

    return DRV_E_NONE;
}

#define __ACC_FDB__

STATIC uint32
_drv_usw_hash_calculate_index(key_lookup_info_t * key_info, key_lookup_result_t * key_result)
{
    uint32 seed = 0xffffffff;
    uint8 * data = key_info->key_data;
    uint32 data_length = key_info->crc_bits;
    uint32 poly = 0 ;
    uint32 poly_len = 0;
    uint32 bucket_index = 0;
    if (0 == key_info->type)/*HASH_CRC*/
    {
        poly = key_info->polynomial;
        poly_len = key_info->poly_len;
        bucket_index = drv_hash_crc(seed,data,data_length,poly,poly_len);
    }
    else
    {
        bucket_index = drv_hash_xor16((uint16*) data, data_length);
    }

    key_result->bucket_index = bucket_index;
    return DRV_E_NONE;
}

STATIC int32
_drv_usw_host0_calc_fdb_index(uint8 lchip, drv_acc_in_t* in, drv_acc_out_t* out)
{
#define HASH_LEVEL_NUM 6

    drv_acc_in_t  host0_in;
    drv_acc_out_t host0_out;
    key_lookup_info_t key_info;
    uint8 level = 0;
    uint8 index = 0;
    uint32 cmd = 0;
    uint32 bucket_num = 4;/*only for fib_host0*/
    key_lookup_result_t  key_result;
    uint32 rslt_index = 0;
    FibHost0HashLookupCtl_m hash_lookup_ctl;
    uint8 key[MAX_ENTRY_BYTE] = {0};
    uint8 poly_type[HASH_LEVEL_NUM] = {0};
    uint8 level_en[HASH_LEVEL_NUM] = {0};
    uint32 index_base[HASH_LEVEL_NUM] = {0};
    uint32 key_index = 0;
    uint32 hw_hashtype = 0;
    uint32 crc[][3]= { /*refer to fib_host0_crc*/
         {0, 0x00000005, 11},/*HASH_CRC*/
         {0, 0x00000017, 11},/*HASH_CRC*/
         {0, 0x0000002b, 11},/*HASH_CRC*/
         {0, 0x00000053, 12},/*HASH_CRC*/
         {0, 0x00000099, 12},/*HASH_CRC*/
         {0, 0x00000107, 12},/*HASH_CRC*/
         {0, 0x0000001b, 13},/*HASH_CRC*/
         {0, 0x00000027, 13},/*HASH_CRC*/
         {0, 0x000000c3, 13},/*HASH_CRC*/
         {1, 16        , 0}, /*HASH_XOR*/
      };

    if (in->tbl_id != DsFibHost0MacHashKey_t)
    {
        return DRV_E_INVALID_TBL;
    }

    sal_memset(&hash_lookup_ctl, 0, sizeof(FibHost0HashLookupCtl_m));
    sal_memset(&key_info, 0, sizeof(key_lookup_info_t));
    sal_memset(&key_result, 0, sizeof(key_lookup_result_t));
    cmd = DRV_IOR(FibHost0HashLookupCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_lookup_ctl));
    level_en[0] = GetFibHost0HashLookupCtl(V, fibHost0Level0HashEn_f, &hash_lookup_ctl);
    level_en[1] = GetFibHost0HashLookupCtl(V, fibHost0Level1HashEn_f, &hash_lookup_ctl);
    level_en[2] = GetFibHost0HashLookupCtl(V, fibHost0Level2HashEn_f, &hash_lookup_ctl);
    level_en[3] = GetFibHost0HashLookupCtl(V, fibHost0Level3HashEn_f, &hash_lookup_ctl);
    level_en[4] = GetFibHost0HashLookupCtl(V, fibHost0Level4HashEn_f, &hash_lookup_ctl);
    level_en[5] = GetFibHost0HashLookupCtl(V, fibHost0Level5HashEn_f, &hash_lookup_ctl);
    index_base[1] = GetFibHost0HashLookupCtl(V, fibHost0Level1IndexBase_f, &hash_lookup_ctl);
    index_base[2] = GetFibHost0HashLookupCtl(V, fibHost0Level2IndexBase_f, &hash_lookup_ctl);
    index_base[3] = GetFibHost0HashLookupCtl(V, fibHost0Level3IndexBase_f, &hash_lookup_ctl);
    index_base[4] = GetFibHost0HashLookupCtl(V, fibHost0Level4IndexBase_f, &hash_lookup_ctl);
    index_base[5] = GetFibHost0HashLookupCtl(V, fibHost0Level5IndexBase_f, &hash_lookup_ctl);
    poly_type[0] = GetFibHost0HashLookupCtl(V, fibHost0Level0HashType_f, &hash_lookup_ctl);
    poly_type[1] = GetFibHost0HashLookupCtl(V, fibHost0Level1HashType_f, &hash_lookup_ctl);
    poly_type[2] = GetFibHost0HashLookupCtl(V, fibHost0Level2HashType_f, &hash_lookup_ctl);
    poly_type[3] = GetFibHost0HashLookupCtl(V, fibHost0Level3HashType_f, &hash_lookup_ctl);
    poly_type[4] = GetFibHost0HashLookupCtl(V, fibHost0Level4HashType_f, &hash_lookup_ctl);
    poly_type[5] = GetFibHost0HashLookupCtl(V, fibHost0Level5HashType_f, &hash_lookup_ctl);

    key_info.key_data = key;
    sal_memcpy(key_info.key_data, (uint8*)in->data, sizeof(DsFibHost0MacHashKey_m));
    key_info.crc_bits = 170;/*always 170 for host0, refer to gg_hash_key_length*/
    key_info.key_bits = 85;

    host0_in.tbl_id    = DsFibHost0MacHashKey_t;
    host0_in.op_type = DRV_ACC_OP_BY_INDEX;
    host0_in.type = DRV_ACC_TYPE_LOOKUP;

    for (level = 0; level < HASH_LEVEL_NUM; level++)
    {
        if (level_en[level] == 0)
        {
            continue;
        }
        key_info.type       = crc[poly_type[level]][0];
        key_info.polynomial = crc[poly_type[level]][1];
        key_info.poly_len   = crc[poly_type[level]][2];
        _drv_usw_hash_calculate_index(&key_info, &key_result);
        rslt_index = index_base[level] + key_result.bucket_index * bucket_num + 32;
        for (index = 0; index < bucket_num; index++)
        {
            key_index = (rslt_index & 0xFFFFFFFC) | index;
             /*check if hashtype*/
            host0_in.index = key_index ;
            DRV_IF_ERROR_RETURN(drv_usw_acc(lchip, &host0_in, &host0_out));
            hw_hashtype = GetDsFibHost0MacHashKey(V, hashType_f, &host0_out.data);
            if (hw_hashtype != 4)
            {
                continue;
            }
            out->data[out->query_cnt++] = key_index;
        }
    }

    return DRV_E_NONE;
}


STATIC int32
_drv_usw_host0_calc_fdb_index_tsingma(uint8 lchip, drv_acc_in_t* in, drv_acc_out_t* out)
{
#undef HASH_LEVEL_NUM
#define HASH_LEVEL_NUM 10

    drv_acc_in_t  host0_in;
    drv_acc_out_t host0_out;
    key_lookup_info_t key_info;
    uint8 level = 0;
    uint8 index = 0;
    uint32 cmd = 0;
    uint32 bucket_num = 4;/*only for fib_host0*/
    key_lookup_result_t  key_result;
    uint32 rslt_index = 0;
    FibHost0HashLookupCtl_m hash_lookup_ctl;
    FibHost0ExternalHashLookupCtl_m extern_hash_ctl;
    uint8 key[MAX_ENTRY_BYTE] = {0};
    uint8 poly_type[HASH_LEVEL_NUM] = {0};
    uint8 level_en[HASH_LEVEL_NUM] = {0};
    uint32 index_base[HASH_LEVEL_NUM] = {0};
    uint32 key_index = 0;
    uint32 hw_hashtype = 0;
    uint32 field_val = 0;
    uint8 couple_en = 0;
    uint32 crc[][3]= { /*refer to fib_host0_crc*/
         {0, 0x0000001b, 10},
         {0, 0x00000017, 11},
         {0, 0x0000002b, 11},
         {0, 0x00000053, 12},
         {0, 0x00000099, 12},
         {0, 0x00000107, 12},
         {0, 0x0000001b, 13},
         {0, 0x00000027, 13},
         {0, 0x00000053, 13},
         {0, 0x000000c3, 13},
         {0, 0x0000005f, 14},
         {0, 0x0000007b, 14},
         {0, 0x000000af, 14},
         {0, 0x000000bb, 14},
         {1, 16        , 0},
      };


    if (in->tbl_id != DsFibHost0MacHashKey_t)
    {
        return DRV_E_INVALID_TBL;
    }

    sal_memset(&hash_lookup_ctl, 0, sizeof(FibHost0HashLookupCtl_m));
    sal_memset(&key_info, 0, sizeof(key_lookup_info_t));
    sal_memset(&key_result, 0, sizeof(key_lookup_result_t));
    cmd = DRV_IOR(FibHost0HashLookupCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_lookup_ctl));
    level_en[0] = GetFibHost0HashLookupCtl(V, fibHost0Level0HashEn_f, &hash_lookup_ctl) &&
                  GetFibHost0HashLookupCtl(V, fibHost0Level0MacKeyEn_f, &hash_lookup_ctl);

    level_en[1] = GetFibHost0HashLookupCtl(V, fibHost0Level1HashEn_f, &hash_lookup_ctl) &&
                  GetFibHost0HashLookupCtl(V, fibHost0Level1MacKeyEn_f, &hash_lookup_ctl);

    level_en[2] = GetFibHost0HashLookupCtl(V, fibHost0Level2HashEn_f, &hash_lookup_ctl) &&
                  GetFibHost0HashLookupCtl(V, fibHost0Level2MacKeyEn_f, &hash_lookup_ctl);

    level_en[3] = GetFibHost0HashLookupCtl(V, fibHost0Level3HashEn_f, &hash_lookup_ctl) &&
                  GetFibHost0HashLookupCtl(V, fibHost0Level3MacKeyEn_f, &hash_lookup_ctl);

    level_en[4] = GetFibHost0HashLookupCtl(V, fibHost0Level4HashEn_f, &hash_lookup_ctl) &&
                  GetFibHost0HashLookupCtl(V, fibHost0Level4MacKeyEn_f, &hash_lookup_ctl);

    level_en[5] = GetFibHost0HashLookupCtl(V, fibHost0Level5HashEn_f, &hash_lookup_ctl) &&
                  GetFibHost0HashLookupCtl(V, fibHost0Level5MacKeyEn_f, &hash_lookup_ctl);

    index_base[1] = GetFibHost0HashLookupCtl(V, fibHost0Level1IndexBase_f, &hash_lookup_ctl);
    index_base[2] = GetFibHost0HashLookupCtl(V, fibHost0Level2IndexBase_f, &hash_lookup_ctl);
    index_base[3] = GetFibHost0HashLookupCtl(V, fibHost0Level3IndexBase_f, &hash_lookup_ctl);
    index_base[4] = GetFibHost0HashLookupCtl(V, fibHost0Level4IndexBase_f, &hash_lookup_ctl);
    index_base[5] = GetFibHost0HashLookupCtl(V, fibHost0Level5IndexBase_f, &hash_lookup_ctl);
    poly_type[0] = GetFibHost0HashLookupCtl(V, fibHost0Level0HashType_f, &hash_lookup_ctl);
    poly_type[1] = GetFibHost0HashLookupCtl(V, fibHost0Level1HashType_f, &hash_lookup_ctl);
    poly_type[2] = GetFibHost0HashLookupCtl(V, fibHost0Level2HashType_f, &hash_lookup_ctl);
    poly_type[3] = GetFibHost0HashLookupCtl(V, fibHost0Level3HashType_f, &hash_lookup_ctl);
    poly_type[4] = GetFibHost0HashLookupCtl(V, fibHost0Level4HashType_f, &hash_lookup_ctl);
    poly_type[5] = GetFibHost0HashLookupCtl(V, fibHost0Level5HashType_f, &hash_lookup_ctl);


    cmd = DRV_IOR(FibEngineLookupCtl_t, FibEngineLookupCtl_externalLookupEn_f);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0,  cmd, &field_val));
    level_en[6] = field_val;
    level_en[7] = field_val;
    level_en[8] = field_val;
    level_en[9] = field_val;

    cmd = DRV_IOR(FibHost0ExternalHashLookupCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &extern_hash_ctl));

    couple_en = GetFibHost0ExternalHashLookupCtl(V, externalMemoryCouple_f, &extern_hash_ctl);
    index_base[6] = GetFibHost0ExternalHashLookupCtl(V, externalKeyIndexBase_f, &extern_hash_ctl) - 32;
    index_base[7] =  index_base[6] + 32*1024*(couple_en?2:1) ;
    index_base[8] =  index_base[7] + 32*1024*(couple_en?2:1) ;
    index_base[9] =  index_base[8] + 32*1024*(couple_en?2:1) ;

    poly_type[6] = GetFibHost0ExternalHashLookupCtl(V, externalLevel0HashType_f, &extern_hash_ctl);
    poly_type[7] = GetFibHost0ExternalHashLookupCtl(V, externalLevel1HashType_f, &extern_hash_ctl);
    poly_type[8] = GetFibHost0ExternalHashLookupCtl(V, externalLevel2HashType_f, &extern_hash_ctl);
    poly_type[9] = GetFibHost0ExternalHashLookupCtl(V, externalLevel3HashType_f, &extern_hash_ctl);

    key_info.key_data = key;
    sal_memcpy(key_info.key_data, (uint8*)in->data, sizeof(DsFibHost0MacHashKey_m));
    key_info.crc_bits = 170;/*always 170 for host0, refer to gg_hash_key_length*/
    key_info.key_bits = 85;

    host0_in.tbl_id    = DsFibHost0MacHashKey_t;
    host0_in.op_type = DRV_ACC_OP_BY_INDEX;
    host0_in.type = DRV_ACC_TYPE_LOOKUP;

    for (level = 0; level < HASH_LEVEL_NUM; level++)
    {
        if (level_en[level] == 0)
        {
            continue;
        }
        key_info.type       = crc[poly_type[level]][0];
        key_info.polynomial = crc[poly_type[level]][1];
        key_info.poly_len   = crc[poly_type[level]][2];
        _drv_usw_hash_calculate_index(&key_info, &key_result);
        rslt_index = index_base[level] + key_result.bucket_index * bucket_num + 32;
        for (index = 0; index < bucket_num; index++)
        {
            key_index = (rslt_index & 0xFFFFFFFC) | index;
             /*check if hashtype*/
            host0_in.index = key_index ;
            DRV_IF_ERROR_RETURN(drv_usw_acc(lchip, &host0_in, &host0_out));
            hw_hashtype = GetDsFibHost0MacHashKey(V, hashType_f, &host0_out.data);
            if (hw_hashtype != 4)
            {
                continue;
            }
            out->data[out->query_cnt++] = key_index;
        }
    }

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_write_mac_by_idx(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    hw_mac_addr_t   mac = {0};
    uint16 fid = 0;
    uint32 ad_idx = 0;
    DsFibHost0MacHashKey_m* p_mac_key = NULL;

    DRV_PTR_VALID_CHECK(in->data);

    if (req_type == DRV_ACC_CHIP_MAX_TYPE)
    {
        return DRV_E_INVAILD_TYPE;
    }

    p_mac_key = (DsFibHost0MacHashKey_m*)in->data;

    GetDsFibHost0MacHashKey(A, mappedMac_f, p_mac_key, mac);
    fid = GetDsFibHost0MacHashKey(V, vsiId_f, p_mac_key);
    ad_idx = GetDsFibHost0MacHashKey(V, dsAdIndex_f, p_mac_key);

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(A, cpuKeyMac_f, cpu_req, mac);
    SetFibHashKeyCpuReq(V, cpuKeyVrfId_f, cpu_req, fid);
    SetFibHashKeyCpuReq(V, cpuKeyDsAdIndex_f, cpu_req, ad_idx);
    SetFibHashKeyCpuReq(V, cpuKeyIndex_f, cpu_req, in->index);
    return DRV_E_NONE;
}


STATIC int32
_drv_usw_acc_prepare_write_mac_by_key(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    uint32 key_index       = 0;
    uint32 static_count_en = 0;
    uint32 hw_aging_en = 0;
    hw_mac_addr_t   mac = {0};
    uint16 fid = 0;
    uint32 ad_idx = 0;
    uint8  is_mcast = 0;
    DsFibHost0MacHashKey_m* p_mac_key = NULL;

    DRV_PTR_VALID_CHECK(in->data);

    p_mac_key = (DsFibHost0MacHashKey_m*)in->data;

    GetDsFibHost0MacHashKey(A, mappedMac_f, p_mac_key, mac);
    fid = GetDsFibHost0MacHashKey(V, vsiId_f, p_mac_key);
    ad_idx = GetDsFibHost0MacHashKey(V, dsAdIndex_f, p_mac_key);

    is_mcast = (mac[1] >> 8) & 0x01;
    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_UPDATE_MAC_LIMIT_CNT) && !is_mcast)
    {
        key_index |= 1 << 14;    /* port mac limit */
        key_index |= 1 << 15;    /* vlan mac limit */

        if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_DYNAMIC))
        {
            key_index |= 1 << 16;    /* dynamic mac limit */
        }
    }

    if (!is_mcast)
    {
        key_index |= 1 << 17;    /* profile mac limit */
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_USE_LOGIC_PORT))
    {
        /*logic port do not update port mac limit*/
        key_index &= ~(1<<14);
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_STATIC_MAC_LIMIT_CNT))
    {
        static_count_en = 1;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_AGING_EN))
    {
        hw_aging_en = 1;
    }

    if (req_type == DRV_ACC_CHIP_MAX_TYPE)
    {
        return DRV_E_INVAILD_TYPE;
    }

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(A, cpuKeyMac_f, cpu_req, mac);
    SetFibHashKeyCpuReq(V, staticCountEn_f, cpu_req, static_count_en);
    SetFibHashKeyCpuReq(V, cpuKeyVrfId_f, cpu_req, fid);
    SetFibHashKeyCpuReq(V, cpuKeyDsAdIndex_f, cpu_req, ad_idx);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqPort_f, cpu_req, in->gport);
    SetFibHashKeyCpuReq(V, cpuKeyIndex_f, cpu_req, key_index);
    SetFibHashKeyCpuReq(V, cpuKeyDelType_f, cpu_req, in->timer_index);
    SetFibHashKeyCpuReq(V, cpuKeyPortDelMode_f, cpu_req, hw_aging_en);

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_del_mac_by_idx(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    if (req_type == DRV_ACC_CHIP_MAX_TYPE)
    {
        return DRV_E_INVAILD_TYPE;
    }

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(V, cpuKeyIndex_f, cpu_req, in->index);
    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_del_mac_by_key(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    uint32 key_index = 0;
    hw_mac_addr_t   mac = {0};
    uint16 fid = 0;
    DsFibHost0MacHashKey_m* p_mac_key = NULL;
    uint8    is_mcast = 0;

    DRV_PTR_VALID_CHECK(in->data);

    p_mac_key = (DsFibHost0MacHashKey_m*)in->data;

    GetDsFibHost0MacHashKey(A, mappedMac_f, p_mac_key, mac);
    fid = GetDsFibHost0MacHashKey(V, vsiId_f, p_mac_key);

    is_mcast = (mac[1] >> 8) & 0x01;
    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_UPDATE_MAC_LIMIT_CNT) && !is_mcast)
    {
        key_index |= 1 << 14;    /* port mac limit */
        key_index |= 1 << 15;    /* vlan mac limit */
        if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_DYNAMIC))
        {
            key_index |= 1 << 16;    /* dynamic mac limit */
        }
    }

    if (!is_mcast)
    {
        key_index |= 1 << 17;    /* profile mac limit */
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_USE_LOGIC_PORT))
    {
        /*logic port do not update port mac limit*/
        key_index &= ~(1<<14);
    }

    if (req_type == DRV_ACC_CHIP_MAX_TYPE)
    {
        return DRV_E_INVAILD_TYPE;
    }

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(A, cpuKeyMac_f, cpu_req, mac);
    SetFibHashKeyCpuReq(V, cpuKeyVrfId_f, cpu_req, fid);
    SetFibHashKeyCpuReq(V, cpuKeyIndex_f, cpu_req, key_index);

    return DRV_E_NONE;
}



STATIC int32
_drv_usw_acc_prepare_lkp_mac_by_key(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    uint16 fid = 0;
    DsFibHost0MacHashKey_m* p_mac_key = NULL;
    hw_mac_addr_t   mac = {0};

    DRV_PTR_VALID_CHECK(in->data);

    p_mac_key = (DsFibHost0MacHashKey_m*)in->data;

    GetDsFibHost0MacHashKey(A, mappedMac_f, p_mac_key, mac);
    fid = GetDsFibHost0MacHashKey(V, vsiId_f, p_mac_key);

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(V, cpuKeyVrfId_f, cpu_req, fid);
    SetFibHashKeyCpuReq(A, cpuKeyMac_f, cpu_req, mac);

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_dump_mac_by_port_vlan(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    uint32        del_type  = 0;
    uint32        del_mode  = 0;
    uint32        key_index = 0;
    hw_mac_addr_t hw_mac    = { 0 };
    uint16 fid = 0;
    DsFibHost0MacHashKey_m* p_mac_key = NULL;

    DRV_PTR_VALID_CHECK(in->data);

    hw_mac[0] = (in->index & 0x7FFFF) |
                ((in->query_end_index & 0x1f) << 19) |
                (((in->query_end_index >> 5) & 0xff) << 24);

    hw_mac[1] = (in->query_end_index >> 13) & 0x3F;

    key_index |= 1<<4;
    if (in->op_type == DRV_ACC_OP_BY_PORT_VLAN)
    {    /*dump by port+vsi*/
        del_type = 0;
    }
    else if (in->op_type == DRV_ACC_OP_BY_PORT)
    {    /*dump by port*/
        del_type = 1;
    }
    else if (in->op_type == DRV_ACC_OP_BY_VLAN)
    {    /*dump by vsi*/
        del_type = 2;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_STATIC))
    {
        key_index |= 1;
    }
    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_DYNAMIC))
    {
        key_index |= 1 << 1;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_USE_LOGIC_PORT))
    {
        del_mode = 1;
        key_index |= (1 << 3);
    }

    p_mac_key = (DsFibHost0MacHashKey_m*)in->data;
    fid = GetDsFibHost0MacHashKey(V, vsiId_f, p_mac_key);

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(V, cpuKeyVrfId_f, cpu_req, fid);
    SetFibHashKeyCpuReq(V, cpuKeyDelType_f, cpu_req, del_type);
    SetFibHashKeyCpuReq(V, cpuKeyIndex_f, cpu_req, key_index);
    SetFibHashKeyCpuReq(V, cpuKeyPortDelMode_f, cpu_req, del_mode);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqPort_f, cpu_req, in->gport);
    SetFibHashKeyCpuReq(A, cpuKeyMac_f, cpu_req, hw_mac);
    SetFibHashKeyCpuReq(V, cpuKeyDsAdIndex_f, cpu_req, in->query_cnt);
    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_dump_mac_all(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    hw_mac_addr_t hw_mac    = { 0 };
    uint32        key_index = 0;

    hw_mac[0] = (in->index & 0x7FFFF) |
                ((in->query_end_index & 0x1FFF) << 19);

    hw_mac[1] = (in->query_end_index >> 13) & 0x3F;

    key_index |= 1<<4;   /*Dump no pending*/

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_STATIC))
    {
        key_index |= 1;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_DYNAMIC))
    {
        key_index |= 1 << 1;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_UCAST))
    {
        key_index |= 1<<2;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_MCAST))
    {
        key_index |= 1 << 3;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_PENDING))
    {
        key_index &= ~(1 << 4); /*do not dump no-pending*/
        key_index |= 1 << 5;
    }

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(V, cpuKeyPortDelMode_f, cpu_req, 1);  /*means dump all*/
    SetFibHashKeyCpuReq(V, cpuKeyIndex_f, cpu_req, key_index);
    SetFibHashKeyCpuReq(A, cpuKeyMac_f, cpu_req, hw_mac);
    SetFibHashKeyCpuReq(V, cpuKeyDsAdIndex_f, cpu_req, in->query_cnt);

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_flush_mac_by_port_vlan(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    uint32 key_index = 0;
    uint32 del_type  = 0;
    uint32 del_mode  = 0;
    uint16 fid = 0;
    DsFibHost0MacHashKey_m* p_mac_key = NULL;

    DRV_PTR_VALID_CHECK(in->data);

    if (in->op_type == DRV_ACC_OP_BY_PORT_VLAN)
    {    /*delete by port+vsi*/
        del_type = 0;
    }
    else if (in->op_type == DRV_ACC_OP_BY_PORT)
    {    /*delete by port*/
        del_type = 1;
    }
    else if (in->op_type == DRV_ACC_OP_BY_VLAN)
    {    /*delete by vsi*/
        del_type = 2;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_STATIC))
    {
        key_index |= 1;
    }
    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_DYNAMIC))
    {
        key_index |= 1 << 1;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_UPDATE_MAC_LIMIT_CNT))
    {
        key_index |= 1 << 5;
        key_index |= 1 << 7;
        key_index |= 1 << 11;
    }

    /*profile*/
    key_index |= 1 << 9;

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_USE_LOGIC_PORT))
    {
        /*logic port do not update port mac limit*/
        key_index &= ~(1<<5);
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_STATIC_MAC_LIMIT_CNT))
    {
        key_index |= 1 << 4;
        key_index |= 1 << 6;
        key_index |= 1 << 8;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_USE_LOGIC_PORT))
    {
        del_mode = 1;
        key_index |= (1 << 3);
    }

    p_mac_key = (DsFibHost0MacHashKey_m*)in->data;
    fid = GetDsFibHost0MacHashKey(V, vsiId_f, p_mac_key);

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(V, cpuKeyVrfId_f, cpu_req, fid);
    SetFibHashKeyCpuReq(V, cpuKeyDelType_f, cpu_req, del_type);
    SetFibHashKeyCpuReq(V, cpuKeyIndex_f, cpu_req, key_index);
    SetFibHashKeyCpuReq(V, cpuKeyPortDelMode_f, cpu_req, del_mode);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqPort_f, cpu_req, in->gport);

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_flush_mac_by_mac_port(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    uint32 key_index = 0;
    uint32 del_type  = 0;
    uint32 del_mode  = 0;
    uint16 fid = 0;
    DsFibHost0MacHashKey_m* p_mac_key = NULL;
    hw_mac_addr_t   mac = {0};

    DRV_PTR_VALID_CHECK(in->data);

    if (in->op_type == DRV_ACC_OP_BY_PORT_MAC)
    {    /*delete by mac+port*/
        del_type = 2;
    }
    else
    {    /*delete by mac*/
        del_type = 1;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_STATIC))
    {
        key_index |= 1 << 0;
    }
    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_DYNAMIC))
    {
        key_index |= 1 << 1;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_UPDATE_MAC_LIMIT_CNT))
    {
        key_index |= 1 << 5;
        key_index |= 1 << 7;
        key_index |= 1 << 11;
    }

    /*profile*/
    key_index |= 1 << 9;

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_USE_LOGIC_PORT))
    {
        /*logic port do not update port mac limit*/
        key_index &= ~(1<<5);
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_STATIC_MAC_LIMIT_CNT))
    {
        key_index |= 1 << 4;
        key_index |= 1 << 6;
        key_index |= 1 << 8;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_USE_LOGIC_PORT))
    {
        del_mode = 1;
    }

    p_mac_key = (DsFibHost0MacHashKey_m*)in->data;

    GetDsFibHost0MacHashKey(A, mappedMac_f, p_mac_key, mac);
    fid = GetDsFibHost0MacHashKey(V, vsiId_f, p_mac_key);

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(A, cpuKeyMac_f, cpu_req, mac);
    SetFibHashKeyCpuReq(V, cpuKeyVrfId_f, cpu_req, fid);
    SetFibHashKeyCpuReq(V, cpuKeyDelType_f, cpu_req, del_type);
    SetFibHashKeyCpuReq(V, cpuKeyIndex_f, cpu_req, key_index);
    SetFibHashKeyCpuReq(V, cpuKeyPortDelMode_f, cpu_req, del_mode);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqPort_f, cpu_req, in->gport);

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_flush_mac_all(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    uint32 key_index = 0;

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_STATIC))
    {
        key_index |= 1 << 0;
    }
    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_ENTRY_DYNAMIC))
    {
        key_index |= 1 << 1;
    }

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_UPDATE_MAC_LIMIT_CNT))
    {
        key_index |= 1 << 5;
        key_index |= 1 << 7;
        key_index |= 1 << 11;
    }

    /*profile*/
    key_index |= 1 << 9;

    if (DRV_IS_BIT_SET(in->flag, DRV_ACC_STATIC_MAC_LIMIT_CNT))
    {
        key_index |= 1 << 4;
        key_index |= 1 << 6;
        key_index |= 1 << 8;
    }

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(V, cpuKeyIndex_f, cpu_req, key_index);
    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_result_write_mac_by_key(uint8 lchip, uint32* fib_acc_cpu_rlt, drv_acc_out_t* out)
{
    out->key_index = GetFibHashKeyCpuResult(V, hashCpuLuIndex_f, fib_acc_cpu_rlt);
    out->is_full   = GetFibHashKeyCpuResult(V, pending_f, fib_acc_cpu_rlt);
    out->is_conflict  = GetFibHashKeyCpuResult(V, conflict_f, fib_acc_cpu_rlt);

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_result_del_mac_by_key(uint8 lchip, uint32* fib_acc_cpu_rlt, drv_acc_out_t* out)
{
    out->key_index = GetFibHashKeyCpuResult(V, hashCpuLuIndex_f, fib_acc_cpu_rlt);
    out->is_hit       = GetFibHashKeyCpuResult(V, hashCpuKeyHit_f, fib_acc_cpu_rlt);
    out->is_conflict  = GetFibHashKeyCpuResult(V, conflict_f, fib_acc_cpu_rlt);

    return DRV_E_NONE;
}


STATIC int32
_drv_usw_acc_result_lkp_mac_by_key(uint8 lchip, uint32* fib_acc_cpu_rlt, drv_acc_out_t* out)
{
    out->is_hit       = GetFibHashKeyCpuResult(V, hashCpuKeyHit_f, fib_acc_cpu_rlt);
    out->key_index = GetFibHashKeyCpuResult(V, hashCpuLuIndex_f, fib_acc_cpu_rlt);
    out->is_conflict  = GetFibHashKeyCpuResult(V, hashCpuConflictValid_f, fib_acc_cpu_rlt);
    out->is_pending   = GetFibHashKeyCpuResult(V, pending_f, fib_acc_cpu_rlt);
    out->ad_index  = GetFibHashKeyCpuResult(V, hashCpuDsIndex_f, fib_acc_cpu_rlt);

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_result_dump_mac(uint8 lchip, uint32* fib_acc_cpu_rlt, drv_acc_out_t* out)
{

    uint32 confict = 0;
    uint32 pending = 0;

    confict = GetFibHashKeyCpuResult(V, conflict_f, fib_acc_cpu_rlt);
    pending = GetFibHashKeyCpuResult(V, pending_f, fib_acc_cpu_rlt);

    out->key_index = GetFibHashKeyCpuResult(V, hashCpuLuIndex_f, fib_acc_cpu_rlt);
    out->is_full   = (confict && (!pending));

    if (DRV_IS_DUET2(lchip))
    {
        out->is_end = confict && pending;
    }
    else
    {
        out->is_end = (out->key_index == (TABLE_MAX_INDEX(lchip,  DsFibHost0MacHashKey_t) + 32 - 1));
    }

    out->query_cnt      = GetFibHashKeyCpuResult(V, hashCpuDsIndex_f, fib_acc_cpu_rlt);
    return DRV_E_NONE;
}

int32
drv_usw_acc_fdb_prepare(uint8 lchip, drv_acc_in_t* acc_in, uint32* req)
{
    uint32 type = DRV_ACC_CHIP_MAX_TYPE;

    switch(acc_in->type)
    {
        case DRV_ACC_TYPE_ADD:
            if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
            {
                type = DRV_ACC_CHIP_MAC_WRITE_BY_INDEX;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_write_mac_by_idx(lchip, type, acc_in, req));
            }
            else
            {
                type = DRV_ACC_CHIP_MAC_WRITE_BY_KEY;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_write_mac_by_key(lchip, type, acc_in, req));
            }
            break;

        case DRV_ACC_TYPE_DEL:
            if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
            {
                type = DRV_ACC_CHIP_MAC_DEL_BY_INDEX;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_del_mac_by_idx(lchip, type, acc_in, req));
            }
            else
            {
                type = DRV_ACC_CHIP_MAC_DEL_BY_KEY;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_del_mac_by_key(lchip, type, acc_in, req));
            }
            break;

         case DRV_ACC_TYPE_LOOKUP:
            if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
            {
                return DRV_E_INVAILD_TYPE;
            }
            else
            {
                type = DRV_ACC_CHIP_MAC_LOOKUP_BY_KEY;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_lkp_mac_by_key(lchip, type, acc_in, req));
            }
            break;

         case DRV_ACC_TYPE_DUMP:
            if ((acc_in->op_type == DRV_ACC_OP_BY_PORT) ||
                (acc_in->op_type == DRV_ACC_OP_BY_VLAN) ||
                (acc_in->op_type == DRV_ACC_OP_BY_PORT_VLAN))
            {
                type = DRV_ACC_CHIP_MAC_DUMP_BY_PORT_VLAN;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_dump_mac_by_port_vlan(lchip, type, acc_in, req));
            }
            else if (acc_in->op_type == DRV_ACC_OP_BY_ALL)
            {
                type = DRV_ACC_CHIP_MAC_FLUSH_DUMP_BY_ALL;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_dump_mac_all(lchip, type, acc_in, req));
            }
            break;

         case DRV_ACC_TYPE_FLUSH:
            if ((acc_in->op_type == DRV_ACC_OP_BY_PORT) ||
                (acc_in->op_type == DRV_ACC_OP_BY_VLAN) ||
                (acc_in->op_type == DRV_ACC_OP_BY_PORT_VLAN))
            {
                type = DRV_ACC_CHIP_MAC_FLUSH_BY_PORT_VLAN;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_flush_mac_by_port_vlan(lchip, type, acc_in, req));
            }
            else if ((acc_in->op_type == DRV_ACC_OP_BY_MAC)||
                     (acc_in->op_type == DRV_ACC_OP_BY_PORT_MAC))
            {
                type = DRV_ACC_CHIP_MAC_FLUSH_BY_MAC_PORT;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_flush_mac_by_mac_port(lchip, type, acc_in, req));
            }
            else if (acc_in->op_type == DRV_ACC_OP_BY_ALL)
            {
                type = DRV_ACC_CHIP_MAC_FLUSH_DUMP_BY_ALL;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_flush_mac_all(lchip, type, acc_in, req));
            }
            break;

        default:
            return DRV_E_INVAILD_TYPE;

    }

    return 0;
}

int32
drv_usw_acc_fdb_result(uint8 lchip, drv_acc_in_t* acc_in, uint32* fib_acc_cpu_rlt, drv_acc_out_t* out)
{
    switch(acc_in->type)
    {
        case DRV_ACC_TYPE_ADD:

            if ((acc_in->op_type != DRV_ACC_OP_BY_INDEX))
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_result_write_mac_by_key(lchip, fib_acc_cpu_rlt, out));
            }

            break;

        case DRV_ACC_TYPE_DEL:

            if ((acc_in->op_type != DRV_ACC_OP_BY_INDEX))
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_result_del_mac_by_key(lchip, fib_acc_cpu_rlt, out));
            }

            break;

         case DRV_ACC_TYPE_LOOKUP:

            if (acc_in->op_type != DRV_ACC_OP_BY_INDEX)
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_result_lkp_mac_by_key(lchip, fib_acc_cpu_rlt, out));
            }
            break;

         case DRV_ACC_TYPE_DUMP:
                DRV_IF_ERROR_RETURN(_drv_usw_acc_result_dump_mac(lchip, fib_acc_cpu_rlt, out));

            break;

         case DRV_ACC_TYPE_FLUSH:
            break;

        default:
            return DRV_E_INVAILD_TYPE;

    }

    return 0;
}

#define __ACC_HOST0__

STATIC int32
_drv_usw_acc_prepare_write_fib0_by_key(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{

    drv_fib_acc_fib_data_t fib_data;
    hw_mac_addr_t          hw_mac     = { 0 };
/*#if (SDK_WORK_PLATFORM == 1)
    uint32                 key_length = 0;
    uint32                 tbl_id     = 0;
#endif*/
    uint32                 length = 0;
    uint32                 overwrite_en = 0;
    uint8                  hash_type = 0;
    DsFibHost0FcoeHashKey_m* p_host0_key = NULL;

    DRV_PTR_VALID_CHECK(in->data);
    sal_memset(&fib_data, 0, sizeof(drv_fib_acc_fib_data_t));

    p_host0_key = (DsFibHost0FcoeHashKey_m*)in->data;
    hash_type = GetDsFibHost0FcoeHashKey(V, hashType_f, p_host0_key);

    hw_mac[0] = hash_type;

    if ((in->tbl_id == DsFibHost0FcoeHashKey_t) || (in->tbl_id == DsFibHost0Ipv4HashKey_t)
        || (in->tbl_id == DsFibHost0MacHashKey_t) || (in->tbl_id == DsFibHost0TrillHashKey_t))
    {
        hw_mac[0] |= HASH_KEY_LENGTH_MODE_SINGLE << 3;
/*#if (SDK_WORK_PLATFORM == 1)
        key_length = DRV_HASH_85BIT_KEY_LENGTH;
        tbl_id     = DsFibHost0MacHashKey_t;
#endif*/
        length     = SINGLE_KEY_BYTE;
    }
    else
    {
        hw_mac[0] |= HASH_KEY_LENGTH_MODE_DOUBLE << 3;
/*#if (SDK_WORK_PLATFORM == 1)
        key_length = DRV_HASH_170BIT_KEY_LENGTH;
        tbl_id     = DsFibHost0MacIpv6McastHashKey_t;
#endif*/
        length     = DOUBLE_KEY_BYTE;
    }

    overwrite_en = DRV_IS_BIT_SET(in->flag, DRV_ACC_OVERWRITE_EN);

    sal_memcpy((uint8*)cpu_req, (uint8 *)in->data, length);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(V, cpuKeyPortDelMode_f, cpu_req, overwrite_en);
    SetFibHashKeyCpuReq(A, cpuKeyMac_f, cpu_req, hw_mac);
    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_lkp_fib0_by_idx(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    uint32        length_mode= 0;
    uint32        count      = 0;
    hw_mac_addr_t hw_mac     = { 0 };

    if (DRV_IS_DUET2(lchip))
    {
        hw_mac[0] = in->index - (in->index % 4);
        count     = (in->index % 4) / 2;
    }
    else
    {
        hw_mac[0] = in->index ;
    }

    if ((in->tbl_id == DsFibHost0FcoeHashKey_t) || (in->tbl_id == DsFibHost0Ipv4HashKey_t)
        || (in->tbl_id == DsFibHost0MacHashKey_t) || (in->tbl_id == DsFibHost0TrillHashKey_t))
    {
        length_mode = HASH_KEY_LENGTH_MODE_SINGLE;
    }
    else
    {
        length_mode = HASH_KEY_LENGTH_MODE_DOUBLE;
    }

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(A, cpuKeyMac_f, cpu_req, hw_mac);

    /* only cmodel need spec and rtl do not need */
    if (1 == count)
    {
        SetFibHashKeyCpuReq(V, staticCountEn_f, cpu_req, length_mode);
    }
    else
    {
        SetFibHashKeyCpuReq(V, cpuKeyPortDelMode_f, cpu_req, length_mode);
    }

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_write_fib0_by_idx(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    drv_fib_acc_rw_data_t tmp_rw_data;
    drv_acc_in_t      acc_read_in;
    drv_acc_out_t     acc_read_out;
    uint32                key_length = 0;
#if (SDK_WORK_PLATFORM == 1)
    uint32                count      = 0;
    uint32                key_length_mode[2] = { 0 };
#endif
    uint32                key_index  = 0;
    uint8                 shift      = 0;
    hw_mac_addr_t         hw_mac = { 0 };
    FibHashKeyCpuReq_m fib_req;
    FibHashKeyCpuResult_m fib_result;

    DRV_PTR_VALID_CHECK(in->data);

    sal_memset(&tmp_rw_data, 0, sizeof(drv_fib_acc_rw_data_t));
    sal_memset(&acc_read_in, 0, sizeof(drv_acc_in_t));
    sal_memset(&acc_read_out, 0, sizeof(drv_acc_out_t));

    hw_mac[0] = in->index - (in->index % 4);
#if (SDK_WORK_PLATFORM == 1)
    count     = (in->index % 4) / 2;
#endif

    if ((in->tbl_id == DsFibHost0FcoeHashKey_t) || (in->tbl_id == DsFibHost0Ipv4HashKey_t)
        || (in->tbl_id == DsFibHost0MacHashKey_t) || (in->tbl_id == DsFibHost0TrillHashKey_t))
    {
#if (SDK_WORK_PLATFORM == 1)
        key_length_mode[count] = HASH_KEY_LENGTH_MODE_SINGLE;
#endif
        key_length             = SINGLE_KEY_BYTE;
    }
    else
    {
#if (SDK_WORK_PLATFORM == 1)
        key_length_mode[count] = HASH_KEY_LENGTH_MODE_DOUBLE;
#endif
        key_length             = SINGLE_KEY_BYTE*2;
    }

    key_index = in->index - (in->index % 4);

    /* read by index, for 340bits */
    acc_read_in.type = DRV_ACC_TYPE_LOOKUP;
    acc_read_in.op_type = DRV_ACC_OP_BY_INDEX;
    acc_read_in.index = key_index;
    acc_read_in.tbl_id = DsFibHost0HashCam_t;
    /*Note: can not using drv_acc_host0 directly, because of lock, using internal interface*/
    DRV_IF_ERROR_RETURN(drv_usw_acc_host0_prepare(lchip, &acc_read_in, (uint32*)&fib_req));
    DRV_IF_ERROR_RETURN(drv_usw_acc_host0_process(lchip, (uint32*)&fib_req, (uint32*)&fib_result));
    DRV_IF_ERROR_RETURN(drv_usw_acc_host0_result(lchip, &acc_read_in, (uint32*)&fib_result, &acc_read_out));

    /* move request data to read data */
    shift = (in->index % 4);
    sal_memcpy(tmp_rw_data, acc_read_out.data, sizeof(tmp_rw_data));
    sal_memcpy(((uint32 *) tmp_rw_data + shift * 3), in->data, key_length);

    sal_memcpy((uint8*)cpu_req, (uint8 *)tmp_rw_data, QUAD_KEY_BYTE);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(A, cpuKeyMac_f, cpu_req, hw_mac);

#if (SDK_WORK_PLATFORM == 1)
    /* only cmodel need spec and rtl do not need */
    SetFibHashKeyCpuReq(V, cpuKeyPortDelMode_f, cpu_req, key_length_mode[0]);
    SetFibHashKeyCpuReq(V, staticCountEn_f, cpu_req, key_length_mode[1]);
#endif
    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_lkp_fib0_by_key(uint8 lchip, uint32 req_type, drv_acc_in_t* in, uint32* cpu_req)
{
    drv_fib_acc_fib_data_t fib_data;
    hw_mac_addr_t          hw_mac     = { 0 };
    uint8                  hash_type = 0;
    DsFibHost0FcoeHashKey_m* p_host0_key = NULL;
    uint16    key_size = 0;

    DRV_PTR_VALID_CHECK(in->data);
    sal_memset(&fib_data, 0, sizeof(drv_fib_acc_fib_data_t));

    p_host0_key = (DsFibHost0FcoeHashKey_m*)in->data;
    hash_type = GetDsFibHost0FcoeHashKey(V, hashType_f, p_host0_key);

    hw_mac[0] = hash_type;
    if ((in->tbl_id == DsFibHost0FcoeHashKey_t) || (in->tbl_id == DsFibHost0Ipv4HashKey_t)
        || (in->tbl_id == DsFibHost0MacHashKey_t) || (in->tbl_id == DsFibHost0TrillHashKey_t))
    {
        hw_mac[0] |= HASH_KEY_LENGTH_MODE_SINGLE << 3;
        key_size  = SINGLE_KEY_BYTE;
    }
    else
    {
        hw_mac[0] |= HASH_KEY_LENGTH_MODE_DOUBLE << 3;
        key_size  = DOUBLE_KEY_BYTE;
    }

    sal_memcpy((uint8*)cpu_req, (uint8 *)in->data, key_size);

    SetDsFibHost0FcoeHashKey(V, dsAdIndex_f, (DsFibHost0FcoeHashKey_m*)cpu_req, 0);

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, cpu_req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, cpu_req, req_type);
    SetFibHashKeyCpuReq(A, cpuKeyMac_f, cpu_req, hw_mac);

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_result_write_fib0_by_key(uint8 lchip, uint32* cpu_rlt, drv_acc_out_t* out)
{
    out->key_index = GetFibHashKeyCpuResult(V, hashCpuLuIndex_f, cpu_rlt);
    out->is_conflict  = GetFibHashKeyCpuResult(V, conflict_f, cpu_rlt);
    out->ad_index = GetFibHashKeyCpuResult(V, hashCpuDsIndex_f, cpu_rlt);
    out->is_hit       = GetFibHashKeyCpuResult(V, hashCpuKeyHit_f, cpu_rlt);

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_result_lkp_fib0_by_idx(uint8 lchip, drv_acc_in_t* acc_in, uint32* cpu_rlt, drv_acc_out_t* out)
{
    uint8         shift = 0;
    uint32        length = 0;
    drv_fib_acc_rw_data_t tmp_rw_data;

    sal_memset(&tmp_rw_data, 0, sizeof(tmp_rw_data));

    if ((acc_in->tbl_id == DsFibHost0FcoeHashKey_t) || (acc_in->tbl_id == DsFibHost0Ipv4HashKey_t)
        || (acc_in->tbl_id == DsFibHost0MacHashKey_t) || (acc_in->tbl_id == DsFibHost0TrillHashKey_t))
    {
        length     = SINGLE_KEY_BYTE;
    }
    else if(acc_in->tbl_id == DsFibHost0HashCam_t)
    {
        length = QUAD_KEY_BYTE;
    }
    else
    {
        length     = DOUBLE_KEY_BYTE;
    }


    if (DRV_IS_DUET2(lchip))
    {
        sal_memcpy((uint8*)tmp_rw_data, (uint8 *)cpu_rlt, QUAD_KEY_BYTE);

        /* asic only read 340 key drv need to adjust the key */
        if (acc_in->index < FIB_HOST0_CAM_NUM)
        {
            shift = (acc_in->index % 4);
        }
        else
        {
            shift = ((acc_in->index - FIB_HOST0_CAM_NUM) % 4);
        }
        sal_memcpy(out->data, ((uint32 *) tmp_rw_data + shift * 3), length);
    }
    else
    {
        sal_memcpy(out->data, (uint8 *)cpu_rlt, SINGLE_KEY_BYTE);
    }

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_result_lkp_fib0_by_key(uint8 lchip, drv_acc_in_t* acc_in, uint32* cpu_rlt, drv_acc_out_t* out)
{
    out->is_hit       = GetFibHashKeyCpuResult(V, hashCpuKeyHit_f, cpu_rlt);
    out->key_index = GetFibHashKeyCpuResult(V, hashCpuLuIndex_f, cpu_rlt);
    out->is_conflict  = GetFibHashKeyCpuResult(V, hashCpuConflictValid_f, cpu_rlt);
    out->ad_index  = GetFibHashKeyCpuResult(V, hashCpuDsIndex_f, cpu_rlt);
    return DRV_E_NONE;
}

int32
drv_usw_acc_host0_prepare(uint8 lchip, drv_acc_in_t* acc_in, uint32* req)
{
    uint32 type = DRV_ACC_CHIP_MAX_TYPE;

    switch(acc_in->type)
    {
        case DRV_ACC_TYPE_ADD:
            if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
            {
                type = DRV_ACC_CHIP_HOST0_WRITE_BY_INDEX;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_write_fib0_by_idx(lchip, type, acc_in, req));
            }
            else
            {
                type = DRV_ACC_CHIP_HOST0_WRITE_BY_KEY;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_write_fib0_by_key(lchip, type, acc_in, req));
            }

            break;

         case DRV_ACC_TYPE_LOOKUP:
            if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
            {
                type = DRV_ACC_CHIP_HOST0_lOOKUP_BY_INDEX;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_lkp_fib0_by_idx(lchip, type, acc_in, req));            }
            else
            {
                type = DRV_ACC_CHIP_HOST0_lOOKUP_BY_KEY;
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_lkp_fib0_by_key(lchip, type, acc_in, req));
            }
            break;

        default:
            return DRV_E_INVAILD_TYPE;

    }

    return 0;
}

int32
drv_usw_acc_host0_process(uint8 lchip, uint32* i_cpu_req, uint32* o_cpu_rlt)
{
    uint32        cmd   = 0;
    uint16         loop  = 0;
    uint8         count = 0;
    uint8         done  = 0;

    cmd = DRV_IOW(FibHashKeyCpuReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, i_cpu_req));

    cmd = DRV_IOR(FibHashKeyCpuReq_t, DRV_ENTRY_FLAG);

    for (loop = 0; (loop < DRV_APP_LOOP_CNT) && !done; loop++)
    {
        for (count = 0; (count < 150) && !done; count++)
        {
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, i_cpu_req));
            done = !GetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, i_cpu_req);
        }

        if (done)
        {
            break;
        }

        sal_task_sleep(DRV_FIB_ACC_SLEEP_TIME);
    }

    if (!done)
    {
        DRV_IF_ERROR_RETURN(DRV_E_TIME_OUT);
    }

    cmd = DRV_IOR(FibHashKeyCpuResult_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, o_cpu_rlt));

#ifdef NEVER
    ecc_en = drv_ecc_recover_get_enable();

    if (is_store && ecc_en)
    {
        _drv_chip_fib_acc_store(lchip, i_fib_acc_cpu_req, o_fib_acc_cpu_rlt);
    }
#endif
    return DRV_E_NONE;
}

int32
drv_usw_acc_host0_result(uint8 lchip, drv_acc_in_t* acc_in, uint32* fib_acc_cpu_rlt, drv_acc_out_t* out)
{
    switch(acc_in->type)
    {
        case DRV_ACC_TYPE_ADD:

            if (acc_in->op_type != DRV_ACC_OP_BY_INDEX)
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_result_write_fib0_by_key(lchip, fib_acc_cpu_rlt, out));
            }

            break;

        case DRV_ACC_TYPE_DEL:

            break;

         case DRV_ACC_TYPE_LOOKUP:

            if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_result_lkp_fib0_by_idx(lchip, acc_in, fib_acc_cpu_rlt, out));
            }
            else if (acc_in->op_type == DRV_ACC_OP_BY_KEY)
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_result_lkp_fib0_by_key(lchip, acc_in, fib_acc_cpu_rlt, out));
            }
            break;

        default:
            return DRV_E_INVAILD_TYPE;

    }

    return 0;
}

STATIC int32
_drv_usw_acc_get_hash_type_by_tbl_id(uint8 lchip, uint32 tbl_id, uint32* p_hash_type, uint32* p_hash)
{
   uint32 out = 0;

   switch(tbl_id)
   {
   case DsFibHost0FcoeHashKey_t:
       out  = FIBHOST0PRIMARYHASHTYPE_FCOE;
       break;

   case DsFibHost0Ipv4HashKey_t:
       out  = FIBHOST0PRIMARYHASHTYPE_IPV4;
       SetDsFibHost0Ipv4HashKey(V, pointer_f, p_hash, 0);
       break;

   case DsFibHost0Ipv6McastHashKey_t:
       out  = FIBHOST0PRIMARYHASHTYPE_IPV6MCAST;
       SetDsFibHost0Ipv6McastHashKey(V, pointer_f, p_hash, 0);
       break;

   case DsFibHost0Ipv6UcastHashKey_t:
       out  = FIBHOST0PRIMARYHASHTYPE_IPV6UCAST;
       break;

   case DsFibHost0MacIpv6McastHashKey_t:
       out  = FIBHOST0PRIMARYHASHTYPE_MACIPV6MCAST;
       SetDsFibHost0MacIpv6McastHashKey(V, pointer_f, p_hash, 0);
       break;

   case DsFibHost0TrillHashKey_t:
       out  = FIBHOST0PRIMARYHASHTYPE_TRILL;
       break;

   default:
       return DRV_E_INVAILD_TYPE;
   }

    *p_hash_type = out;

    return 0;
}

STATIC int32
_drv_usw_acc_prepare_fib_hash_by_key(uint8 lchip, drv_acc_in_t* in, drv_cpu_acc_prepare_info_t* pre_info)
{
    FibHashCpuLookupReq_m host1_req;
    FibHashCpuLookupResult_m host1_result;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 entry_size = 0;
    uint8 length = 0;
    uint32 mode = 0;
    uint32 loop = 0;
    uint32 count = 0;
    uint32 done = 0;
    uint32 data[12] = { 0 };
    uint32 req_valid = 0;
    uint8 req_interface = 0;
    uint32 hash_type = 0;
    DsFibHost1Ipv4McastHashKey_m* p_hash = NULL;

    /* check valid bit clear */
    _drv_usw_acc_hash_get_valid(lchip, DRV_ACC_HASH_MODULE_FIB_HASH, &req_valid);
    if (req_valid)
    {
        return DRV_E_OCCPANCY;
    }

    sal_memset(&host1_req, 0, sizeof(host1_req));
    sal_memset(&host1_result, 0, sizeof(host1_result));

    cmd = DRV_IOW(FibHashCpuLookupResult_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &host1_result));

    /*for action by key, need do lookup first*/
    pre_info->tbl_id = in->tbl_id;
    tbl_id = in->tbl_id;

    entry_size  = TABLE_ENTRY_SIZE(lchip, tbl_id) / 12;

    switch (entry_size)
    {
        case 1:
            mode = HASH_KEY_LENGTH_MODE_SINGLE;
            length     = SINGLE_KEY_BYTE;
            break;

        case 2:

            mode = HASH_KEY_LENGTH_MODE_DOUBLE;
            length     = DOUBLE_KEY_BYTE;
            break;

        case 4:

            mode = HASH_KEY_LENGTH_MODE_QUAD;
            length     = QUAD_KEY_BYTE;
            break;
        default:
            return DRV_E_INVALID_TBL;
    }


    sal_memcpy((uint8*)data, (uint8 *)in->data, length);

    p_hash = (DsFibHost1Ipv4McastHashKey_m*)data;


    if (pre_info->hash_type == DRV_ACC_HASH_MODULE_FIB_HOST0)
    {
        req_interface = 2;

        _drv_usw_acc_get_hash_type_by_tbl_id(lchip, tbl_id, &hash_type, (uint32*)p_hash);
        SetFibHashCpuLookupReq(V, hashKeyType_f, &host1_req, hash_type);

        if (length == SINGLE_KEY_BYTE)
        {
            SetDsFibHost0Ipv4HashKey(V, valid_f, p_hash, 1);

        }
        else if(length == DOUBLE_KEY_BYTE)
        {
            SetDsFibHost0Ipv6UcastHashKey(V, valid0_f, p_hash, 1);
            SetDsFibHost0Ipv6UcastHashKey(V, valid1_f, p_hash, 1);
        }

        /*Clear adindex*/
        SetDsFibHost0Ipv4HashKey(V, dsAdIndex_f, p_hash, 0);

    }
    else if(pre_info->hash_type == DRV_ACC_HASH_MODULE_FLOW)
    {
        req_interface = 1;
        SetDsFlowL2HashKey(V, dsAdIndex_f, p_hash, 0);
    }
    else  /*fib host 1*/
    {
        req_interface = 0;

        /*Clear adindex*/
        SetDsFibHost1Ipv4McastHashKey(V, dsAdIndex_f, p_hash, 0);
    }


    sal_memcpy((uint8*)&host1_req, (uint8 *)data, length);

    SetFibHashCpuLookupReq(V, cpuLookupReqValid_f, &host1_req, 1);
    SetFibHashCpuLookupReq(V, lengthMode_f, &host1_req, mode);
    SetFibHashCpuLookupReq(V, reqInterface_f, &host1_req, req_interface);

    cmd = DRV_IOW(FibHashCpuLookupReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &host1_req));

    cmd = DRV_IOR(FibHashCpuLookupResult_t, DRV_ENTRY_FLAG);

    for (loop = 0; (loop < DRV_APP_LOOP_CNT) && !done; loop++)
    {
        if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
        {
            done = 1;
            break;
        }

        for (count = 0; (count < 50) && !done; count++)
        {
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &host1_result));
            done = GetFibHashCpuLookupResult(V, cpuLookupReqDone_f, &host1_result);
        }

        if (done)
        {
            break;
        }
        sal_task_sleep(1);
    }

    if (!done)
    {
        DRV_IF_ERROR_RETURN(DRV_E_CMD_NOT_DONE);
    }

    pre_info->conflict = GetFibHashCpuLookupResult(V, conflict_f, &host1_result);
    pre_info->valid = GetFibHashCpuLookupResult(V, lookupResultValid_f, &host1_result);

    /*acording lookup result build prepare information*/
    if (!GetFibHashCpuLookupResult(V, conflict_f, &host1_result))
    {
        pre_info->tbl_id = tbl_id;
        pre_info->key_index = GetFibHashCpuLookupResult(V, resultIndex_f, &host1_result);
    }

    return DRV_E_NONE;
}


#define __ACC_HOST1__
STATIC uint8
_drv_usw_acc_get_host1_hash_type(uint8 lchip, drv_acc_in_t* in)
{
    uint8 hash_type = 0;
    switch(in->tbl_id)
    {
        case DsFibHost1FcoeRpfHashKey_t:
            hash_type =  FIBHOST1HASHTYPE_FCOERPF;
            break;
        case DsFibHost1Ipv4McastHashKey_t:
            hash_type =  FIBHOST1HASHTYPE_IPV4MCAST;
            break;
        case DsFibHost1Ipv4NatDaPortHashKey_t:
            hash_type =  FIBHOST1HASHTYPE_IPV4NATDAPORT;
            break;
        case DsFibHost1Ipv4NatSaPortHashKey_t:
            hash_type =  FIBHOST1HASHTYPE_IPV4NATSAPORT;
            break;
        case DsFibHost1Ipv6McastHashKey_t:
            hash_type =  FIBHOST1HASHTYPE_IPV6MCAST;
            break;
        case DsFibHost1Ipv6NatDaPortHashKey_t:
            hash_type =  FIBHOST1HASHTYPE_IPV6NATDAPORT;
            break;
        case DsFibHost1Ipv6NatSaPortHashKey_t:
            hash_type =  FIBHOST1HASHTYPE_IPV6NATSAPORT;
            break;
        case DsFibHost1MacIpv4McastHashKey_t:
            hash_type =  FIBHOST1HASHTYPE_MACIPV4MCAST;
            break;
        case DsFibHost1MacIpv6McastHashKey_t:
            hash_type =  FIBHOST1HASHTYPE_MACIPV6MCAST;
            break;
        case DsFibHost1TrillMcastVlanHashKey_t:
            hash_type =  FIBHOST1HASHTYPE_TRILLMCASTVLAN;
            break;
        default:
            break;
    }

    return hash_type;
}
STATIC int32
_drv_usw_acc_prepare_host1_by_key(uint8 lchip, drv_acc_in_t* in, drv_cpu_acc_prepare_info_t* pre_info)
{
    FibHost1FlowHashCpuLookupReq_m host1_req;
    FibHost1FlowHashCpuLookupResult_m host1_result;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 entry_size = 0;
    uint8 length = 0;
    uint32 mode = 0;
    uint32 loop = 0;
    uint32 count = 0;
    uint32 done = 0;
    uint32 data[12] = { 0 };
    uint32 req_valid = 0;
    DsFibHost1Ipv4McastHashKey_m* p_hash = NULL;
/*#if (SDK_WORK_PLATFORM == 1)
    uint8 c_length = 0;
#endif*/

    /* check valid bit clear */
    _drv_usw_acc_hash_get_valid(lchip, DRV_ACC_HASH_MODULE_FIB_HOST1, &req_valid);
    if (req_valid)
    {
        return DRV_E_OCCPANCY;
    }

    sal_memset(&host1_req, 0, sizeof(FibHost1FlowHashCpuLookupReq_m));
    sal_memset(&host1_result, 0, sizeof(FibHost1FlowHashCpuLookupResult_m));

    cmd = DRV_IOW(FibHost1FlowHashCpuLookupResult_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &host1_result));

    /*for action by key, need do lookup first*/
    pre_info->tbl_id = in->tbl_id;
    tbl_id = in->tbl_id;

    entry_size  = TABLE_ENTRY_SIZE(lchip, tbl_id) / 12;

    switch (entry_size)
    {
        case 1:
/*#if (SDK_WORK_PLATFORM == 1)
            c_length = DRV_HASH_85BIT_KEY_LENGTH;
#endif*/
            mode = HASH_KEY_LENGTH_MODE_SINGLE;
            length     = SINGLE_KEY_BYTE;
            break;

        case 2:
/*#if (SDK_WORK_PLATFORM == 1)
            c_length = DRV_HASH_170BIT_KEY_LENGTH;
#endif*/
            mode = HASH_KEY_LENGTH_MODE_DOUBLE;
            length     = DOUBLE_KEY_BYTE;
            break;

        case 4:
/*#if (SDK_WORK_PLATFORM == 1)
            c_length = DRV_HASH_340BIT_KEY_LENGTH;
#endif*/
            mode = HASH_KEY_LENGTH_MODE_QUAD;
            length     = QUAD_KEY_BYTE;
            break;
        default:
            return DRV_E_INVALID_TBL;
    }

    sal_memcpy((uint8*)data, (uint8 *)in->data, length);
    p_hash = (DsFibHost1Ipv4McastHashKey_m*)data;
    SetDsFibHost1Ipv4McastHashKey(V, dsAdIndex_f, p_hash, 0);

    sal_memcpy((uint8*)&host1_req, (uint8 *)data, length);
    SetFibHost1FlowHashCpuLookupReq(V, cpuLookupReqValid_f, &host1_req, 1);
    SetFibHost1FlowHashCpuLookupReq(V, lengthMode_f, &host1_req, mode);
    SetFibHost1FlowHashCpuLookupReq(V, fibHost1Req_f, &host1_req, 1);
    SetFibHost1FlowHashCpuLookupReq(V, fibHost1HashType_f, &host1_req, (_drv_usw_acc_get_host1_hash_type(lchip, in)));

    cmd = DRV_IOW(FibHost1FlowHashCpuLookupReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &host1_req));

    cmd = DRV_IOR(FibHost1FlowHashCpuLookupResult_t, DRV_ENTRY_FLAG);

    for (loop = 0; (loop < DRV_APP_LOOP_CNT) && !done; loop++)
    {
        if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
        {
            done = 1;
            break;
        }

        for (count = 0; (count < 50) && !done; count++)
        {
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &host1_result));
            done = GetFibHost1FlowHashCpuLookupResult(V, cpuLookupReqDone_f, &host1_result);
        }

        if (done)
        {
            break;
        }
        sal_task_sleep(1);
    }

    if (!done)
    {
        DRV_IF_ERROR_RETURN(DRV_E_CMD_NOT_DONE);
    }

    pre_info->conflict = GetFibHost1FlowHashCpuLookupResult(V, conflict_f, &host1_result);
    pre_info->valid = GetFibHost1FlowHashCpuLookupResult(V, lookupResultValid_f, &host1_result);

    /*acording lookup result build prepare information*/
    if (!GetFibHost1FlowHashCpuLookupResult(V, conflict_f, &host1_result))
    {
        pre_info->tbl_id = tbl_id;
        pre_info->key_index = GetFibHost1FlowHashCpuLookupResult(V, resultIndex_f, &host1_result);
    }

    return DRV_E_NONE;
}

#define __ACC_USERID__



STATIC int32
_drv_usw_acc_prepare_userid_by_key(uint8 lchip, drv_acc_in_t* in, drv_cpu_acc_prepare_info_t* pre_info)
{
    UserIdCpuLookupReq_m userid_req;
    UserIdCpuLookupResult_m userid_result;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 entry_size = 0;
    uint8  length= 0;
    uint32 mode = 0;
    uint32 loop = 0;
    uint32 count = 0;
    uint32 done = 0;
    uint32 data[12] = { 0 };
    uint32 req_valid = 0;
    DsUserIdCvlanPortHashKey_m* p_hash = NULL;

    /* check valid bit clear */
    _drv_usw_acc_hash_get_valid(lchip, DRV_ACC_HASH_MODULE_USERID, &req_valid);
    if (req_valid)
    {
        return DRV_E_OCCPANCY;
    }

    sal_memset(&userid_req, 0, sizeof(UserIdCpuLookupReq_m));
    sal_memset(&userid_result, 0, sizeof(UserIdCpuLookupResult_m));

    cmd = DRV_IOW(UserIdCpuLookupResult_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &userid_result));

    /*for action by key, need do lookup first*/
    tbl_id = in->tbl_id;

    entry_size  = TABLE_ENTRY_SIZE(lchip, tbl_id) / 12;

    switch (entry_size)
    {
        case 1:
            mode = HASH_KEY_LENGTH_MODE_SINGLE;
            length     = SINGLE_KEY_BYTE;
            break;

        case 2:
            mode = HASH_KEY_LENGTH_MODE_DOUBLE;
            length     = DOUBLE_KEY_BYTE;
            break;

        case 4:
            mode = HASH_KEY_LENGTH_MODE_QUAD;
            length     = QUAD_KEY_BYTE;
            break;
        default:
            return DRV_E_INVALID_TBL;
    }


    sal_memcpy((uint8*)data, (uint8 *)in->data, length);


    p_hash = (DsUserIdCvlanPortHashKey_m*)data;
    SetDsUserIdCvlanPortHashKey(V, dsAdIndex_f, p_hash, 0);

    if (DRV_IS_TSINGMA(lchip))
    {
        if (in->level == 1)
        {
            SetUserIdCpuLookupReq(V, requestInterface_f, &userid_req, 1);
        }
    }

    sal_memcpy((uint8*)&userid_req, (uint8 *)data, length);

    SetUserIdCpuLookupReq(V, cpuLookupReqValid_f, &userid_req, 1);

    SetUserIdCpuLookupReq(V, lengthMode_f, &userid_req, mode);

    cmd = DRV_IOW(UserIdCpuLookupReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &userid_req));

    cmd = DRV_IOR(UserIdCpuLookupResult_t, DRV_ENTRY_FLAG);

    for (loop = 0; (loop < DRV_APP_LOOP_CNT) && !done; loop++)
    {
        if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
        {
            done = 1;
            break;
        }

        for (count = 0; (count < 150) && !done; count++)
        {
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &userid_result));
            done = GetUserIdCpuLookupResult(V, cpuLookupReqDone_f, &userid_result);
        }

        if (done)
        {
            break;
        }
        sal_task_sleep(1);
    }

    if (!done)
    {
        DRV_IF_ERROR_RETURN(DRV_E_CMD_NOT_DONE);
    }

    pre_info->conflict = GetUserIdCpuLookupResult(V, conflict_f, &userid_result);
    pre_info->valid = GetUserIdCpuLookupResult(V, lookupResultValid_f, &userid_result);

    /*acording lookup result build prepare information*/
    if (!GetUserIdCpuLookupResult(V, conflict_f, &userid_result))
    {
        pre_info->key_index = GetUserIdCpuLookupResult(V, resultIndex_f, &userid_result);
    }
    DRV_DBG_OUT("=============================\n");
    DRV_DBG_OUT("UserID Lookup keyinex:%d\n", pre_info->key_index);
    DRV_DBG_OUT("conflict:%d\n", pre_info->conflict);
    DRV_DBG_OUT("valid:%d\n", pre_info->valid);


    return DRV_E_NONE;
}

#define __ACC_XCOAM__

STATIC int32
_drv_usw_acc_prepare_xcoam_by_key(uint8 lchip, drv_acc_in_t* in, drv_cpu_acc_prepare_info_t* pre_info)
{
    EgressXcOamCpuLookupReq_m xcoam_req;
    EgressXcOamCpuLookupResult_m xcoam_result;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 entry_size = 0;
#if (SDK_WORK_PLATFORM == 1)
    uint8 c_length = 0;
#else
    uint8 length = 0;
#endif
    uint32 loop = 0;
    uint32 count = 0;
    uint32 done = 0;
    uint32 data[12] = { 0 };
    uint32 req_valid = 0;

    /* check valid bit clear */
    _drv_usw_acc_hash_get_valid(lchip, DRV_ACC_HASH_MODULE_EGRESS_XC, &req_valid);
    if (req_valid)
    {
        return DRV_E_OCCPANCY;
    }

    sal_memset(&xcoam_req, 0, sizeof(EgressXcOamCpuLookupReq_m));
    sal_memset(&xcoam_result, 0, sizeof(EgressXcOamCpuLookupResult_m));

    /*clear request result*/
    cmd = DRV_IOW(EgressXcOamCpuLookupResult_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xcoam_result));

    /*for action by key, need do lookup first*/
    pre_info->tbl_id = in->tbl_id;
    tbl_id = in->tbl_id;

    entry_size  = TABLE_ENTRY_SIZE(lchip, tbl_id) / 12;

    switch (entry_size)
    {
        case 1:
#if (SDK_WORK_PLATFORM == 1)
            c_length = DRV_HASH_85BIT_KEY_LENGTH;
#else
            length     = SINGLE_KEY_BYTE;
#endif
            break;

        case 2:
#if (SDK_WORK_PLATFORM == 1)
            c_length = DRV_HASH_170BIT_KEY_LENGTH;
#else
            length     = DOUBLE_KEY_BYTE;
#endif
            break;

        case 4:
#if (SDK_WORK_PLATFORM == 1)
            c_length = DRV_HASH_340BIT_KEY_LENGTH;
#else
            length     = QUAD_KEY_BYTE;
#endif
            break;
        default:
            return DRV_E_INVALID_TBL;
    }

#if (SDK_WORK_PLATFORM == 1)
    drv_model_hash_combined_key((uint8 *)data, (uint8 *)in->data, c_length, tbl_id);
    drv_model_hash_mask_key((uint8 *)data, (uint8 *)data, HASH_MODULE_EGRESS_XC, tbl_id);
#else
    sal_memcpy((uint8*)data, (uint8 *)in->data, length);
    if ((tbl_id == DsEgressXcOamBfdHashKey_t)||(tbl_id == DsEgressXcOamEthHashKey_t)||(tbl_id == DsEgressXcOamMplsLabelHashKey_t) ||
        (tbl_id == DsEgressXcOamMplsSectionHashKey_t) || (tbl_id == DsEgressXcOamRmepHashKey_t))
    {
        /*For oam hash key mask ad info*/
        *((uint32*)data+0) &= 0xffffffff;
        *((uint32*)data+1) &= 0x3ffff;
        *((uint32*)data+2) &= 0;
    }
#endif

    SetEgressXcOamCpuLookupReq(V, cpuLookupReqValid_f, &xcoam_req, 1);
    SetEgressXcOamCpuLookupReq(A, data_f, &xcoam_req, data);

    cmd = DRV_IOW(EgressXcOamCpuLookupReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xcoam_req));

    cmd = DRV_IOR(EgressXcOamCpuLookupResult_t, DRV_ENTRY_FLAG);

    for (loop = 0; (loop < DRV_APP_LOOP_CNT) && !done; loop++)
    {
        if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
        {
            done = 1;
            break;
        }

        for (count = 0; (count < 150) && !done; count++)
        {
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &xcoam_result));
            done = GetEgressXcOamCpuLookupResult(V, cpuLookupReqDone_f, &xcoam_result);
        }

        if (done)
        {
            break;
        }
        sal_task_sleep(1);
    }

    if (!done)
    {
        DRV_IF_ERROR_RETURN(DRV_E_CMD_NOT_DONE);
    }

    pre_info->conflict = GetEgressXcOamCpuLookupResult(V, conflict_f, &xcoam_result);
    pre_info->valid = GetEgressXcOamCpuLookupResult(V, lookupResultValid_f, &xcoam_result);

    /*acording lookup result build prepare information*/
    if (!GetEgressXcOamCpuLookupResult(V, conflict_f, &xcoam_result))
    {
        pre_info->tbl_id = tbl_id;
        pre_info->key_index = GetEgressXcOamCpuLookupResult(V, resultIndex_f, &xcoam_result);
    }

    return DRV_E_NONE;
}

#define __ACC_FLOW__

STATIC int32
_drv_usw_acc_prepare_flow_by_key(uint8 lchip, drv_acc_in_t* in, drv_cpu_acc_prepare_info_t* pre_info)
{
    FibHost1FlowHashCpuLookupReq_m flow_req;
    /*- FlowHashCpuLookupReq1_m flow_req1;*/
    FibHost1FlowHashCpuLookupResult_m flow_result;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 entry_size = 0;
    uint8 length = 0;
    uint32 mode = 0;
    uint32 loop = 0;
    uint32 count = 0;
    uint32 done = 0;
    uint32 data[12] = { 0 };
    uint32 req_valid = 0;
    DsFlowL2HashKey_m* p_l2hash = NULL;
/*#if (SDK_WORK_PLATFORM == 1)
    uint8 c_length = 0;
#endif*/

    /* check valid bit clear */
    _drv_usw_acc_hash_get_valid(lchip, DRV_ACC_HASH_MODULE_FLOW, &req_valid);
    if (req_valid)
    {
        return DRV_E_OCCPANCY;
    }

    sal_memset(&flow_req, 0, sizeof(FibHost1FlowHashCpuLookupReq_m));
     /*-sal_memset(&flow_req1, 0, sizeof(FlowHashCpuLookupReq1_m));*/
    sal_memset(&flow_result, 0, sizeof(FibHost1FlowHashCpuLookupResult_m));

    cmd = DRV_IOW(FibHost1FlowHashCpuLookupResult_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flow_result));

    /*for action by key, need do lookup first*/
    pre_info->tbl_id = in->tbl_id;
    tbl_id = in->tbl_id;

    entry_size  = TABLE_ENTRY_SIZE(lchip, tbl_id) / 12;

    switch (entry_size)
    {
        case 1:
/*#if (SDK_WORK_PLATFORM == 1)
            c_length = DRV_HASH_85BIT_KEY_LENGTH;
#endif*/
            mode = HASH_KEY_LENGTH_MODE_SINGLE;
            length     = SINGLE_KEY_BYTE;
            break;

        case 2:
/*#if (SDK_WORK_PLATFORM == 1)
            c_length = DRV_HASH_170BIT_KEY_LENGTH;
#endif*/
            mode = HASH_KEY_LENGTH_MODE_DOUBLE;
            length     = DOUBLE_KEY_BYTE;
            break;

        case 4:
/*#if (SDK_WORK_PLATFORM == 1)
            c_length = DRV_HASH_340BIT_KEY_LENGTH;
#endif*/
            mode = HASH_KEY_LENGTH_MODE_QUAD;
            length     = QUAD_KEY_BYTE;
            break;
        default:
            return DRV_E_INVALID_TBL;
    }

    sal_memcpy((uint8*)data, (uint8 *)in->data, length);
    p_l2hash = (DsFlowL2HashKey_m*)data;
    SetDsFlowL2HashKey(V, dsAdIndex_f, p_l2hash, 0);
    sal_memcpy((uint8*)&flow_req, (uint8 *)data, length);

    SetFibHost1FlowHashCpuLookupReq(V, cpuLookupReqValid_f, &flow_req, 1);
    SetFibHost1FlowHashCpuLookupReq(V, lengthMode_f, &flow_req, mode);
    SetFibHost1FlowHashCpuLookupReq(V, fibHost1Req_f, &flow_req, 0);

    cmd = DRV_IOW(FibHost1FlowHashCpuLookupReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flow_req));

    cmd = DRV_IOR(FibHost1FlowHashCpuLookupResult_t, DRV_ENTRY_FLAG);

    for (loop = 0; (loop < DRV_APP_LOOP_CNT) && !done; loop++)
    {
        if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
        {
            done = 1;
            break;
        }

        for (count = 0; (count < 150) && !done; count++)
        {
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flow_result));
            done = GetFibHost1FlowHashCpuLookupResult(V, cpuLookupReqDone_f, &flow_result);
        }

        if (done)
        {
            break;
        }
        sal_task_sleep(1);
    }

    if (!done)
    {
        DRV_IF_ERROR_RETURN(DRV_E_CMD_NOT_DONE);
    }

    pre_info->conflict = GetFibHost1FlowHashCpuLookupResult(V, conflict_f, &flow_result);
    pre_info->valid = GetFibHost1FlowHashCpuLookupResult(V, lookupResultValid_f, &flow_result);

    /*acording lookup result build prepare information*/
    if (!GetFibHost1FlowHashCpuLookupResult(V, conflict_f, &flow_result))
    {
        pre_info->tbl_id = tbl_id;
        pre_info->key_index = GetFibHost1FlowHashCpuLookupResult(V, resultIndex_f, &flow_result);
    }

    return DRV_E_NONE;
}

int32
drv_usw_acc_hash_prepare(uint8 lchip, uint8 hash_type, drv_acc_in_t* acc_in, drv_cpu_acc_prepare_info_t* pre_info,
drv_acc_out_t* acc_out)
{
    uint32 tbl_id = 0;
    uint8 cam_num = 0;

    DRV_IF_ERROR_RETURN(drv_usw_get_cam_info(lchip, hash_type, &tbl_id, &cam_num));

    pre_info->cam_step = (TABLE_ENTRY_SIZE(lchip, tbl_id))/SINGLE_KEY_BYTE;
    pre_info->tbl_id = acc_in->tbl_id;
    pre_info->key_index = acc_in->index;
    pre_info->cam_num = cam_num;
    pre_info->hash_type = hash_type;



    if (acc_in->op_type != DRV_ACC_OP_BY_INDEX)
    {
        if (hash_type == DRV_ACC_HASH_MODULE_FIB_HOST1 ||
            hash_type == DRV_ACC_HASH_MODULE_FIB_HOST0)
        {
            if (DRV_IS_DUET2(lchip))
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_host1_by_key(lchip, acc_in, pre_info));
            }
            else
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_fib_hash_by_key(lchip, acc_in, pre_info));
            }
        }
        else if (hash_type == DRV_ACC_HASH_MODULE_EGRESS_XC)
        {
            DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_xcoam_by_key(lchip, acc_in, pre_info));
        }
        else if (hash_type == DRV_ACC_HASH_MODULE_FLOW)
        {
            if (DRV_IS_DUET2(lchip))
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_flow_by_key(lchip, acc_in, pre_info));
            }
            else
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_fib_hash_by_key(lchip, acc_in, pre_info));
            }
        }
        else if (hash_type == DRV_ACC_HASH_MODULE_USERID)
        {
            DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_userid_by_key(lchip, acc_in, pre_info));
        }

    }


    switch(acc_in->type)
    {
        case DRV_ACC_TYPE_ADD:
            pre_info->data = acc_in->data;
            pre_info->is_read = 0;
            break;

         case DRV_ACC_TYPE_LOOKUP:
            pre_info->is_read = 1;
            break;

         case DRV_ACC_TYPE_DEL:
            sal_memset((uint8*)pre_info->data, 0, 48);
            pre_info->is_read = 0;
            break;

         case DRV_ACC_TYPE_ALLOC:
            sal_memset((uint8*)pre_info->data, 0xFF, 48);
            pre_info->is_read = 0;
            break;
        default:
            return DRV_E_INVAILD_TYPE;

    }

    if (pre_info->key_index < cam_num)
    {
        pre_info->is_cam = 1;
        pre_info->cam_table = tbl_id;
    }
    else
    {
        pre_info->is_cam = 0;
    }

    return 0;
}

int32
drv_usw_acc_hash_process(uint8 lchip, drv_cpu_acc_prepare_info_t* p_info)
{
    uint32 cmdr                        = 0;
    uint32 cmdw                        = 0;
    uint32 index                       = 0;
    uint32 step                        = 0;
    uint32 cam_key[12] = { 0 };

    /*check whether using cam*/
    if (p_info->is_cam)
    {
        /*TBD*/
        /*If software alloc cam using, alloc cam index do not using acc return index*/

        /*If cam not shared, using acc return index, SDK should using this mode, check spec*/
    }

    if (p_info->is_cam)
    {
        index = p_info->key_index / p_info->cam_step;
        step  = p_info->key_index % p_info->cam_step;

        cmdr = DRV_IOR(p_info->cam_table, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, index, cmdr, cam_key));
        if (!p_info->is_read)
        {
            sal_memcpy(&(cam_key[(SINGLE_KEY_BYTE / 4) * step]), p_info->data, TABLE_ENTRY_SIZE(lchip, p_info->tbl_id));
            cmdw = DRV_IOW(p_info->cam_table, DRV_ENTRY_FLAG);
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, index, cmdw, cam_key));
        }
        else
        {
            sal_memcpy(p_info->data, &(cam_key[(SINGLE_KEY_BYTE / 4) * step]), TABLE_ENTRY_SIZE(lchip, p_info->tbl_id));
        }
    }
    else
    {

        if (!p_info->is_read)
        {
            cmdw = DRV_IOW(p_info->tbl_id, DRV_ENTRY_FLAG);
            DRV_DBG_OUT("Write %s (%d)\n", TABLE_NAME(lchip, p_info->tbl_id), (p_info->key_index-p_info->cam_num));
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, (p_info->key_index-p_info->cam_num), cmdw, p_info->data));
        }
        else
        {
            cmdr = DRV_IOR(p_info->tbl_id, DRV_ENTRY_FLAG);
            DRV_DBG_OUT("Read %s (%d)\n", TABLE_NAME(lchip, p_info->tbl_id), (p_info->key_index-p_info->cam_num));
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, (p_info->key_index-p_info->cam_num), cmdr, p_info->data));
        }
    }

    return DRV_E_NONE;
}

int32
drv_usw_acc_hash_result(uint8 lchip, drv_acc_in_t* acc_in, drv_cpu_acc_prepare_info_t* pre_info, drv_acc_out_t* out)
{
    out->is_conflict = pre_info->conflict;
    out->is_hit = pre_info->valid;
    out->key_index = pre_info->key_index;

    if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
    {
        sal_memcpy((uint8*)out->data, (uint8*)pre_info->data, TABLE_ENTRY_SIZE(lchip, pre_info->tbl_id));
    }

    if (acc_in->type == DRV_ACC_TYPE_LOOKUP  &&
        acc_in->op_type == DRV_ACC_OP_BY_KEY &&
        pre_info->hash_type == DRV_ACC_HASH_MODULE_FIB_HOST0)
    {
        out->ad_index =  GetDsFibHost0Ipv4HashKey(V, dsAdIndex_f, pre_info->data);
    }

    return 0;
}

#define __MAC_LIMIT__
STATIC int32
_drv_usw_acc_prepare_read_mac_limit(uint8 lchip, drv_acc_in_t* acc_in, uint32* req)
{
    uint32 mac_limit_type = 0;
    uint32 cpu_req_port = 0;
    uint32 index = 0;
    uint16 fid = 0;

    if (acc_in->op_type == DRV_ACC_OP_BY_PORT)
    {
        /*Get port base mac limit state*/
        if (DRV_GPORT_TO_GCHIP(acc_in->gport) == 0x1f)
        {
            index =  (acc_in->gport) & 0xFF;
        }
        else
        {
            index = DRV_GPORT_TO_LPORT(acc_in->gport) + TABLE_MAX_INDEX(lchip, DsLinkAggregateGroup_t)/2;
        }

        mac_limit_type = 1;
    }
    else if (acc_in->op_type == DRV_ACC_OP_BY_VLAN)
    {
        DRV_PTR_VALID_CHECK(acc_in->data);
        fid = *(uint16*)(acc_in->data);

        /*Get vlan base mac limit state*/
        index = TABLE_MAX_INDEX(lchip, DsLinkAggregateGroup_t)/2 + 256 + fid;
        mac_limit_type = 1;
    }
    else if (acc_in->op_type == DRV_ACC_OP_BY_ALL)
    {
        if (DRV_IS_BIT_SET(acc_in->flag, DRV_ACC_STATIC_MAC_LIMIT_CNT))
        {
            /*Get profile base mac limit state*/
            mac_limit_type = 3;
        }
        else
        {
            /*Get dynamic base mac limit state*/
            mac_limit_type = 2;
        }
    }

    if (mac_limit_type == 1)
    {
        cpu_req_port = (((index & 0x1f) << 8) | (index >> 5));
        SetFibHashKeyCpuReq(V, cpuKeyVrfId_f, req, index);
        SetFibHashKeyCpuReq(V, hashKeyCpuReqPort_f, req, cpu_req_port);
    }

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, req, 1);
    SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, req, DRV_ACC_CHIP_LIMIT_READ);
    SetFibHashKeyCpuReq(V, cpuKeyDelType_f, req, mac_limit_type);

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_result_read_mac_limit(uint8 lchip, uint32* fib_acc_cpu_rlt, drv_acc_out_t* out)
{
    out->query_cnt         = GetFibHashKeyCpuResult(V, hashCpuLuIndex_f, fib_acc_cpu_rlt);
    out->is_full = GetFibHashKeyCpuResult(V, hashCpuKeyHit_f, fib_acc_cpu_rlt);
    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_write_mac_limit(uint8 lchip, drv_acc_in_t* acc_in, uint32* req)
{
    uint32 index = 0;
    uint16 fid = 0;
    uint32 del_type = 0;
    hw_mac_addr_t hw_mac       = { 0 };
    uint32   cpu_req_port = 0;

    if (acc_in->op_type == DRV_ACC_OP_BY_PORT)
    {
        /*port base mac limit state*/
        if (DRV_GPORT_TO_GCHIP(acc_in->gport) == 0x1f)
        {
            index =  (acc_in->gport) & 0xFF;
        }
        else
        {
            index = DRV_GPORT_TO_LPORT(acc_in->gport) + TABLE_MAX_INDEX(lchip, DsLinkAggregateGroup_t)/2;
        }
        del_type = 1;
        SetFibHashKeyCpuReq(V, cpuKeyVrfId_f, req, index);
    }
    else if (acc_in->op_type == DRV_ACC_OP_BY_VLAN)
    {
        DRV_PTR_VALID_CHECK(acc_in->data);
        fid = *(uint16*)(acc_in->data);

        /*vlan base mac limit state*/
        index = TABLE_MAX_INDEX(lchip, DsLinkAggregateGroup_t)/2 + 256 + fid;
        del_type = 1;
        SetFibHashKeyCpuReq(V, cpuKeyVrfId_f, req, index);
    }
    else if (acc_in->op_type == DRV_ACC_OP_BY_ALL)
    {
        if (DRV_IS_BIT_SET(acc_in->flag, DRV_ACC_STATIC_MAC_LIMIT_CNT))
        {
            /*profile base mac limit state*/
            del_type = 3;
        }
        else
        {
            /*dynamic base mac limit state*/
            del_type = 2;
        }
    }

    if (!DRV_IS_BIT_SET(acc_in->flag, DRV_ACC_OVERWRITE_EN))
    {
        cpu_req_port = (((index & 0x1f) << 8) | (index >> 5));
        if (DRV_IS_BIT_SET(acc_in->flag, DRV_ACC_MAC_LIMIT_NEGATIVE))
        {
            cpu_req_port |= 1 <<13;
        }

        SetFibHashKeyCpuReq(V, hashKeyCpuReqPort_f, req, cpu_req_port);

        if (DRV_IS_BIT_SET(acc_in->flag, DRV_ACC_MAC_LIMIT_STATUS))
        {
            SetFibHashKeyCpuReq(V, cpuKeyPortDelMode_f, req, acc_in->limit_cnt);
            hw_mac[0] = 1<<1;
        }
        else
        {
            SetFibHashKeyCpuReq(V, cpuKeyIndex_f, req, acc_in->limit_cnt);
            hw_mac[0] = 1;
        }

        SetFibHashKeyCpuReq(A, cpuKeyMac_f, req, hw_mac);
        SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, req, DRV_ACC_CHIP_LIMIT_UPDATE);
    }
    else
    {
        SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, req, DRV_ACC_CHIP_LIMIT_WRITE);
        SetFibHashKeyCpuReq(V, cpuKeyIndex_f, req, acc_in->limit_cnt);
    }

    SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, req, 1);
    SetFibHashKeyCpuReq(V, cpuKeyDelType_f, req, del_type);

    return DRV_E_NONE;
}

int32
drv_usw_acc_mac_limit_prepare(uint8 lchip, drv_acc_in_t* acc_in, uint32* req)
{

    switch(acc_in->type)
    {
        case DRV_ACC_TYPE_ADD:
            /*write mac limit*/
            DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_write_mac_limit(lchip, acc_in, req));
            break;

         case DRV_ACC_TYPE_LOOKUP:
            DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_read_mac_limit(lchip, acc_in, req));
            /*read mac limit*/

            break;

        default:
            return DRV_E_INVAILD_TYPE;

    }

    return 0;
}

int32
drv_usw_acc_mac_limit_result(uint8 lchip, drv_acc_in_t* acc_in, uint32* fib_acc_cpu_rlt, drv_acc_out_t* out)
{
    switch(acc_in->type)
    {
        case DRV_ACC_TYPE_ADD:
            break;

         case DRV_ACC_TYPE_LOOKUP:
            _drv_usw_acc_result_read_mac_limit(lchip, fib_acc_cpu_rlt, out);
            break;

        default:
            return DRV_E_INVAILD_TYPE;

    }

    return 0;
}

#define __ACC_IPFIX__

STATIC int32
_drv_usw_acc_prepare_write_ipfix(uint8 lchip, uint8 by_index, drv_acc_in_t* acc_in, uint32* req)
{

    uint8 is_340bits = 0;
    uint8 len = 0;

    if (TABLE_ENTRY_SIZE(lchip, acc_in->tbl_id) == (DRV_BYTES_PER_ENTRY*4))
    {
        len = QUAD_KEY_BYTE;
        is_340bits = 1;
    }
    else
    {
        len = DOUBLE_KEY_BYTE;
        is_340bits = 0;
    }


    SetIpfixHashAdCpuReq(V, cpuReqType_f, req, (by_index?DRV_IPFIX_REQ_WRITE_BY_IDX:DRV_IPFIX_REQ_WRITE_BY_KEY));
    SetIpfixHashAdCpuReq(V, cpuReqValid_f, req, 1);
    if(by_index)
    {
        SetIpfixHashAdCpuReq(V, u1_gWriteByIndex_cpuIndex_f, req, acc_in->index);
        SetIpfixHashAdCpuReq(V, u1_gWriteByIndex_dataType_f, req, is_340bits);
    }
    else
    {
        SetIpfixHashAdCpuReq(V, u1_gWriteByKey_dataType_f, req, is_340bits);
    }
    sal_memcpy((uint8*)req, (uint8 *)acc_in->data, len);


    if (acc_in->tbl_id == DsIpfixSessionRecord_t ||
        acc_in->tbl_id == DsIpfixSessionRecord0_t||
        acc_in->tbl_id == DsIpfixSessionRecord1_t)
    {
        /*write ad*/
        SetIpfixHashAdCpuReq(V, isAdMemOperation_f, req, 1);
    }
    else
    {
        SetIpfixHashAdCpuReq(V, isAdMemOperation_f, req, 0);
    }

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_del_ipfix(uint8 lchip, drv_acc_in_t* acc_in, uint32* req)
{
    uint8  is_340bits = 0;
    uint8 len = 0;

    if (TABLE_ENTRY_SIZE(lchip, acc_in->tbl_id) == (DRV_BYTES_PER_ENTRY*4))
    {
        is_340bits = 1;
        len = QUAD_KEY_BYTE;
    }
    else
    {
        len = DOUBLE_KEY_BYTE;
        is_340bits = 0;
    }

    SetIpfixHashAdCpuReq(V, cpuReqType_f, req, DRV_IPFIX_REQ_WRITE_BY_IDX);
    SetIpfixHashAdCpuReq(V, cpuReqValid_f, req, 1);
    SetIpfixHashAdCpuReq(V, u1_gWriteByIndex_cpuIndex_f, req, acc_in->index);
    SetIpfixHashAdCpuReq(V, u1_gWriteByIndex_dataType_f, req, is_340bits);

    sal_memcpy((uint8*)req, (uint8 *)acc_in->data, len);

    if (acc_in->tbl_id == DsIpfixSessionRecord_t ||
        acc_in->tbl_id == DsIpfixSessionRecord0_t||
        acc_in->tbl_id == DsIpfixSessionRecord1_t)
    {
        /*write ad*/
        SetIpfixHashAdCpuReq(V, isAdMemOperation_f, req, 1);
    }
    else
    {
        SetIpfixHashAdCpuReq(V, isAdMemOperation_f, req, 0);
    }

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_acc_prepare_lkp_ipfix(uint8 lchip, uint8 by_idx, drv_acc_in_t* acc_in, uint32* req)
{
    uint8  is_340bits = 0;
    uint8 len = 0;

    if (TABLE_ENTRY_SIZE(lchip, acc_in->tbl_id) == (DRV_BYTES_PER_ENTRY*4))
    {
        len = QUAD_KEY_BYTE;
        is_340bits = 1;
    }
    else
    {

        len = DOUBLE_KEY_BYTE;
        is_340bits = 0;
    }

    SetIpfixHashAdCpuReq(V, cpuReqValid_f, req, 1);
    if (!by_idx)
    {

        sal_memcpy((uint8*)req, (uint8 *)acc_in->data, len);
        SetIpfixHashAdCpuReq(V, cpuReqType_f, req, DRV_IPFIX_REQ_LKP_BY_KEY);
        SetIpfixHashAdCpuReq(V, u1_gLookupByKey_dataType_f, req, is_340bits);
    }
    else
    {
        SetIpfixHashAdCpuReq(V, cpuReqType_f, req, DRV_IPFIX_REQ_LKP_BY_IDX);
        SetIpfixHashAdCpuReq(V, u1_gReadByIndex_dataType_f, req, is_340bits);
        SetIpfixHashAdCpuReq(V, u1_gReadByIndex_cpuIndex_f, req, acc_in->index);
    }

    if (acc_in->tbl_id == DsIpfixSessionRecord_t ||
        acc_in->tbl_id == DsIpfixSessionRecord0_t||
        acc_in->tbl_id == DsIpfixSessionRecord1_t)
    {
        /*write ad*/
        SetIpfixHashAdCpuReq(V, isAdMemOperation_f, req, 1);
    }
    else
    {
        SetIpfixHashAdCpuReq(V, isAdMemOperation_f, req, 0);
    }

    return DRV_E_NONE;
}

int32
drv_usw_acc_ipfix_prepare(uint8 lchip, drv_acc_in_t* acc_in, uint32* req)
{
    switch(acc_in->type)
    {
        case DRV_ACC_TYPE_ADD:
            if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_write_ipfix(lchip, 1, acc_in, req));
            }
            else
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_write_ipfix(lchip, 0, acc_in, req));
            }
            break;

         case DRV_ACC_TYPE_LOOKUP:
            if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_lkp_ipfix(lchip, 1, acc_in, req));
            }
            else
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_lkp_ipfix(lchip, 0, acc_in, req));
            }
            break;

         case DRV_ACC_TYPE_DEL:
            if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
            {
                DRV_IF_ERROR_RETURN(_drv_usw_acc_prepare_del_ipfix(lchip, acc_in, req));
            }
            else
            {
                return DRV_E_INVAILD_TYPE;
            }
            break;
        default:
            return DRV_E_INVAILD_TYPE;

    }

    return 0;
}

int32
drv_usw_acc_ipfix_process(uint8 lchip, uint8 dir, void* i_ipfix_req, void* o_ipfix_result)
{

    uint32                      loop  = 0;
    uint8                      count = 0;
    uint32                     done  = 0;
    uint32                     req_tbl_id = 0;
    uint32                     rlt_tbl_id = 0;
    uint32                     cmdw =0;
    uint32                     cmdw_result =0;
    uint32                     cmdr =0;
    IpfixHashAdCpuResult0_m result;
    if (DRV_IS_TSINGMA(lchip))
    {
        if (dir == 0)
        {
            req_tbl_id = IpfixHashAdCpuReq0_t;
            rlt_tbl_id = IpfixHashAdCpuResult0_t;
        }
        if (dir == 1)
        {
            req_tbl_id = IpfixHashAdCpuReq1_t;
            rlt_tbl_id = IpfixHashAdCpuResult1_t;
        }
    }
    else
    {
        req_tbl_id = IpfixHashAdCpuReq_t;
        rlt_tbl_id = IpfixHashAdCpuResult_t;
    }

    cmdw = DRV_IOW(req_tbl_id, DRV_ENTRY_FLAG);
    cmdw_result = DRV_IOW(rlt_tbl_id, DRV_ENTRY_FLAG);
    cmdr = DRV_IOR(rlt_tbl_id, DRV_ENTRY_FLAG);

    /*clear IpfixHashAdCpuResult*/
    sal_memset(&result,0,sizeof(result));
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmdw_result, &result));

    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmdw, i_ipfix_req));
    for (loop = 0; (loop < DRV_APP_LOOP_CNT) && !done; loop++)
    {
        if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
        {
            done = 1;
            break;
        }
        for (count = 0; (count < 200) && !done; count++)
        {
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmdr, o_ipfix_result));
            done = GetIpfixHashAdCpuResult(V, operationDone_f, o_ipfix_result);
        }
        if(done)
        {
            break;
        }
        sal_task_sleep(1);
    }

    if (!done)
    {
        DRV_IF_ERROR_RETURN(DRV_E_CMD_NOT_DONE);
    }

    /*for ipfix acc sometime cannot return data following done is set, do one more io*/
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmdr, o_ipfix_result));
    return DRV_E_NONE;
}

int32
drv_usw_acc_ipfix_result(uint8 lchip, drv_acc_in_t* in, void* req_result, drv_acc_out_t* out)
{
    uint32                     ret_idx     = 0;
    uint32                     is_conflict = 0;
    uint32                     idx_invalid = 0;

    is_conflict = GetIpfixHashAdCpuResult(V, conflictValid_f, req_result);
    idx_invalid = GetIpfixHashAdCpuResult(V, invalidIndex_f, req_result);
    ret_idx     = GetIpfixHashAdCpuResult(V, lookupReturnIndex_f, req_result);

    out->is_conflict = is_conflict;
    out->is_hit       = !idx_invalid;

    if (in->op_type == DRV_ACC_OP_BY_KEY)
    {
        out->key_index = ret_idx;
    }



    if (in->type == DRV_ACC_TYPE_LOOKUP && in->op_type == DRV_ACC_OP_BY_INDEX)
    {
        sal_memcpy((uint8*)out->data, (uint8 *)req_result, QUAD_KEY_BYTE);
    }

    return DRV_E_NONE;
}

#define __ACC_CID__
int32
drv_usw_acc_cid_lookup(uint8 lchip, drv_acc_in_t* acc_in, drv_cpu_acc_prepare_info_t* pre_info)
{
    CidPairHashCpuLookupReq_m cid_req;
    CidPairHashCpuLookupResult_m cid_result;
    uint32 cmd = 0;
    uint32 loop = 0;
    uint32 count = 0;
    uint32 done = 0;
    uint32 data = 0;
    DsCategoryIdPairHashLeftKey_m cid_data;
    uint32 src_cid = 0;
    uint32 dst_cid = 0;
    uint32 src_valid = 0;
    uint32 dst_valid = 0;
    uint32 entry_valid = 0;

    sal_memset(&cid_req, 0, sizeof(CidPairHashCpuLookupReq_m));
    sal_memset(&cid_result, 0, sizeof(CidPairHashCpuLookupResult_m));

    /*clear request result*/
    cmd = DRV_IOW(CidPairHashCpuLookupResult_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cid_result));

    /*for action by key, need do lookup first*/
    pre_info->tbl_id = acc_in->tbl_id;

    /*Data must using 1st array for acc interface, decode data using DsCategoryIdPairHashLeftKey*/
    /*Format is: SrcCid(8)+DstCid(8)+SrcValid(1)+DstValid(1)+EntryValid(1)*/
    src_cid = (GetDsCategoryIdPairHashLeftKey(V, array_0_srcCategoryId_f, (uint32*)acc_in->data)) & 0xff;
    dst_cid = (GetDsCategoryIdPairHashLeftKey(V, array_0_destCategoryId_f, (uint32*)acc_in->data)) & 0xff;
    src_valid = (GetDsCategoryIdPairHashLeftKey(V, array_0_srcCategoryIdClassfied_f, (uint32*)acc_in->data));
    dst_valid = (GetDsCategoryIdPairHashLeftKey(V, array_0_destCategoryIdClassfied_f, (uint32*)acc_in->data));
    entry_valid = (GetDsCategoryIdPairHashLeftKey(V, array_0_valid_f, (uint32*)acc_in->data));
    data = ((src_cid << 11) | (dst_cid << 3) | (src_valid << 2) | (dst_valid << 1) | entry_valid);

    SetCidPairHashCpuLookupReq(V, data_f, &cid_req, data);
    SetCidPairHashCpuLookupReq(V, cpuLookupReqValid_f, &cid_req, 1);

    cmd = DRV_IOW(CidPairHashCpuLookupReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cid_req));

    cmd = DRV_IOR(CidPairHashCpuLookupResult_t, DRV_ENTRY_FLAG);

    for (loop = 0; (loop < DRV_APP_LOOP_CNT) && !done; loop++)
    {
        if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
        {
            done = 1;
            break;
        }

        for (count = 0; (count < 150) && !done; count++)
        {
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cid_result));
            done = GetCidPairHashCpuLookupResult(V, cpuLookupReqDone_f, &cid_result);
        }

        if (done)
        {
            break;
        }
        sal_task_sleep(1);
    }

    if (!done)
    {
        DRV_IF_ERROR_RETURN(DRV_E_CMD_NOT_DONE);
    }

    pre_info->conflict = !GetCidPairHashCpuLookupResult(V, hit_f, &cid_result) && !GetCidPairHashCpuLookupResult(V, freeValid_f, &cid_result);
    pre_info->valid = GetCidPairHashCpuLookupResult(V, hit_f, &cid_result);

    /*acording lookup result build prepare information*/
    if (!pre_info->conflict)
    {
        pre_info->is_left = GetCidPairHashCpuLookupResult(V, isLeft_f, &cid_result);
        pre_info->is_high = GetCidPairHashCpuLookupResult(V, entryOffset_f, &cid_result);
        pre_info->key_index = GetCidPairHashCpuLookupResult(V, hashKeyIndex_f, &cid_result);
        pre_info->tbl_id = pre_info->is_left?DsCategoryIdPairHashLeftKey_t:DsCategoryIdPairHashRightKey_t;

        /*re-coding hash data*/
        if (acc_in->type == DRV_ACC_TYPE_DEL)
        {
            src_cid = 0;
            dst_cid = 0;
            src_valid = 0;
            dst_valid = 0;
            entry_valid = 0;
        }

        cmd = DRV_IOR(pre_info->tbl_id, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, pre_info->key_index, cmd, &cid_data));
        if (!pre_info->is_high)
        {
            SetDsCategoryIdPairHashLeftKey(V, array_0_srcCategoryId_f, &cid_data, src_cid);
            SetDsCategoryIdPairHashLeftKey(V, array_0_destCategoryId_f, &cid_data, dst_cid);
            SetDsCategoryIdPairHashLeftKey(V, array_0_srcCategoryIdClassfied_f, &cid_data, src_valid);
            SetDsCategoryIdPairHashLeftKey(V, array_0_destCategoryIdClassfied_f, &cid_data, dst_valid);
            SetDsCategoryIdPairHashLeftKey(V, array_0_valid_f, &cid_data, entry_valid);
        }
        else
        {
            SetDsCategoryIdPairHashLeftKey(V, array_1_srcCategoryId_f, &cid_data, src_cid);
            SetDsCategoryIdPairHashLeftKey(V, array_1_destCategoryId_f, &cid_data, dst_cid);
            SetDsCategoryIdPairHashLeftKey(V, array_1_srcCategoryIdClassfied_f, &cid_data, src_valid);
            SetDsCategoryIdPairHashLeftKey(V, array_1_destCategoryIdClassfied_f, &cid_data, dst_valid);
            SetDsCategoryIdPairHashLeftKey(V, array_1_valid_f, &cid_data, entry_valid);
        }

        sal_memcpy(pre_info->data, &cid_data, sizeof(DsCategoryIdPairHashLeftKey_m));
    }
    else
    {
        return DRV_E_NONE;
    }

    return DRV_E_NONE;
}

int32
drv_usw_acc_cid_prepare(uint8 lchip, drv_acc_in_t* acc_in, drv_cpu_acc_prepare_info_t* pre_info)
{
    uint32 req_valid = 0;
    uint32 cmd = 0;
    DsCategoryIdPairHashLeftKey_m cid_data;
    pre_info->tbl_id = acc_in->tbl_id;
    pre_info->key_index = acc_in->index;

    /* check valid bit clear */
    DRV_IF_ERROR_RETURN(_drv_usw_acc_hash_get_valid(lchip, DRV_ACC_HASH_MODULE_CID, &req_valid));
    if (req_valid)
    {
        return DRV_E_OCCPANCY;
    }

    /*only support add by index, del/lookup should by key*/
    if ((acc_in->op_type == DRV_ACC_OP_BY_INDEX) && (acc_in->type != DRV_ACC_TYPE_ADD))
    {
        return DRV_E_INVAILD_TYPE;
    }

    if (acc_in->op_type != DRV_ACC_OP_BY_INDEX)
    {
        DRV_IF_ERROR_RETURN(drv_usw_acc_cid_lookup(lchip, acc_in, pre_info));
    }

    switch(acc_in->type)
    {
        case DRV_ACC_TYPE_ADD:
            if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
            {
                /*re-coding hash data*/
                cmd = DRV_IOR(acc_in->tbl_id, DRV_ENTRY_FLAG);
                DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, pre_info->key_index, cmd, &cid_data));

                if (!acc_in->offset)
                {
                    sal_memcpy(&cid_data, (uint32*)acc_in->data, sizeof(uint32));
                }
                else
                {
                    sal_memcpy(((uint32*)&cid_data+1), (uint32*)acc_in->data, sizeof(uint32));
                }

                sal_memcpy(pre_info->data, &cid_data, sizeof(DsCategoryIdPairHashLeftKey_m));
            }


            pre_info->is_read = 0;
            break;

         case DRV_ACC_TYPE_LOOKUP:
            pre_info->is_read = 1;
            break;

         case DRV_ACC_TYPE_DEL:
            pre_info->is_read = 0;
            break;

        default:
            return DRV_E_INVAILD_TYPE;

    }

    return 0;
}


int32
drv_usw_acc_cid_process(uint8 lchip, drv_cpu_acc_prepare_info_t* p_info)
{
    uint32 cmdr                        = 0;
    uint32 cmdw                        = 0;

    if (!p_info->is_read)
    {
        cmdw = DRV_IOW(p_info->tbl_id, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, (p_info->key_index), cmdw, p_info->data));
    }
    else
    {
        cmdr = DRV_IOR(p_info->tbl_id, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, (p_info->key_index), cmdr, p_info->data));
    }

    return DRV_E_NONE;
}

int32
drv_usw_acc_cid_result(uint8 lchip, drv_acc_in_t* acc_in, drv_cpu_acc_prepare_info_t* pre_info, drv_acc_out_t* out)
{
    out->is_conflict = pre_info->conflict;
    out->is_hit = pre_info->valid;
    out->key_index = pre_info->key_index;
    out->is_left = pre_info->is_left;
    out->offset = pre_info->is_high;

    if (acc_in->type == DRV_ACC_TYPE_LOOKUP)
    {
        if (!pre_info->is_high)
        {
            out->ad_index = (GetDsCategoryIdPairHashLeftKey(V, array_0_adIndex_f, (uint32*)pre_info->data));
        }
        else
        {
            out->ad_index = (GetDsCategoryIdPairHashLeftKey(V, array_1_adIndex_f, (uint32*)pre_info->data));
        }
    }

    return 0;
}

#define __ACC_AGING__
int32
drv_usw_acc_aging_prepare(uint8 lchip, drv_acc_in_t* acc_in, uint32* req)
{
    uint32 enable = 0;
    uint32 index = 0;
    if(acc_in->type == DRV_ACC_TYPE_UPDATE && acc_in->op_type == DRV_ACC_OP_BY_INDEX)
    {
        if(acc_in->data)
        {
            enable = *(uint8*)(acc_in->data);
        }

        index = (acc_in->index)&0x3FFFF;

        if (DRV_IS_BIT_SET(acc_in->flag, DRV_ACC_AGING_EN))
        {
            /*use aging ptr*/
            index = (index | (1<<20));
        }

        SetFibHashKeyCpuReq(V, hashKeyCpuReqType_f, req, DRV_ACC_CHIP_WRITE_DSAGING);
        SetFibHashKeyCpuReq(V, cpuKeyIndex_f, req, index);
        SetFibHashKeyCpuReq(V, cpuKeyDelType_f, req, (acc_in->timer_index)&0x3);
        SetFibHashKeyCpuReq(V, staticCountEn_f, req, enable);
        SetFibHashKeyCpuReq(V, hashKeyCpuReqValid_f, req, 1);


        return DRV_E_NONE;

    }
    else
    {
        return DRV_E_INVAILD_TYPE;
    }
}

int32
drv_usw_acc_aging(uint8 lchip, drv_acc_in_t* acc_in, drv_acc_out_t* acc_out)
{
    uint32 cmd = 0;
    FibHashKeyCpuReq_m fib_req;
    FibHashKeyCpuResult_m fib_result;

    sal_memset(&fib_result, 0, sizeof(FibHashKeyCpuResult_m));

    FIB_ACC_LOCK(lchip);
    /* check valid bit clear */
    cmd = DRV_IOR(FibHashKeyCpuReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &fib_req), p_drv_master[lchip]->fib_acc_mutex);
    if (GetFibHashKeyCpuReq(V,hashKeyCpuReqValid_f,&fib_req))
    {
        FIB_ACC_UNLOCK(lchip);
        return DRV_E_OCCPANCY;
    }

    sal_memset(&fib_req, 0, sizeof(FibHashKeyCpuReq_m));

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_aging_prepare(lchip, acc_in, (uint32*)&fib_req),
        p_drv_master[lchip]->fib_acc_mutex);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_host0_process(lchip, (uint32*)&fib_req, (uint32*)&fib_result),
        p_drv_master[lchip]->fib_acc_mutex);
    FIB_ACC_UNLOCK(lchip);
    return DRV_E_NONE;
}


#define __ACC_MPLS__

int32
drv_usw_acc_mpls_lookup(uint8 lchip, drv_acc_in_t* acc_in, drv_cpu_acc_prepare_info_t* pre_info)
{
    uint32 cmd = 0;
    uint32 loop = 0;
    uint32 count = 0;
    uint32 done = 0;
    uint32 valid = 0;
    MplsCpuLookupReq_m req;
    MplsCpuLookupResult_m result;

    sal_memset(&req, 0, sizeof(req));
    sal_memset(&result, 0, sizeof(result));

    cmd = DRV_IOW(MplsCpuLookupResult_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &result));

    SetMplsCpuLookupReq(V, mplsCpuLookupReqValid_f, &req, 1);

    SetMplsCpuLookupReq(A, mplsData_f, &req, acc_in->data);

    cmd = DRV_IOW(MplsCpuLookupReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &req));

    cmd = DRV_IOR(MplsCpuLookupResult_t, DRV_ENTRY_FLAG);

    for (loop = 0; (loop < DRV_APP_LOOP_CNT) && !done; loop++)
    {
        if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
        {
            done = 1;
            break;
        }

        for (count = 0; (count < 150) && !done; count++)
        {
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &result));
            done = GetMplsCpuLookupResult(V, mplsCpuLookupReqDone_f, &result);
        }

        if (done)
        {
            break;
        }
        sal_task_sleep(1);
    }

    if (!done)
    {
        DRV_IF_ERROR_RETURN(DRV_E_CMD_NOT_DONE);
    }


    valid = 0;
    cmd = DRV_IOR(MplsCpuLookupReq_t, MplsCpuLookupReq_mplsCpuLookupReqValid_f);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &valid));


    pre_info->conflict = GetMplsCpuLookupResult(V, mplsConflict_f, &result);
    pre_info->valid = GetMplsCpuLookupResult(V, mplsLookupResultValid_f, &result);

    /*acording lookup result build prepare information*/
    if (!GetMplsCpuLookupResult(V, mplsConflict_f, &result))
    {
        pre_info->key_index = GetMplsCpuLookupResult(V, mplsResultIndex_f, &result);
    }
    DRV_DBG_OUT("=============================\n");
    DRV_DBG_OUT("Lookup keyinex:%d\n", pre_info->key_index);
    DRV_DBG_OUT("conflict:%d\n", pre_info->conflict);
    DRV_DBG_OUT("valid:%d\n", pre_info->valid);


    return DRV_E_NONE;
}

int32
drv_usw_acc_mpls_prepare(uint8 lchip, drv_acc_in_t* acc_in, drv_cpu_acc_prepare_info_t* pre_info)
{
    uint32 req_valid = 0;
    uint32 tbl_id = 0;
    uint8 cam_num = 0;

    DRV_IF_ERROR_RETURN(drv_usw_get_cam_info(lchip, DRV_ACC_HASH_MODULE_MPLS_HASH, &tbl_id, &cam_num));

    pre_info->tbl_id = acc_in->tbl_id;
    pre_info->key_index = acc_in->index;
    pre_info->cam_num = cam_num;


    /* check valid bit clear */
    DRV_IF_ERROR_RETURN(_drv_usw_acc_hash_get_valid(lchip, DRV_ACC_HASH_MODULE_MPLS_HASH, &req_valid));
    if (req_valid)
    {
        return DRV_E_OCCPANCY;
    }


    if (acc_in->op_type != DRV_ACC_OP_BY_INDEX)
    {
        DRV_IF_ERROR_RETURN(drv_usw_acc_mpls_lookup(lchip, acc_in, pre_info));
    }

    switch(acc_in->type)
    {
        case DRV_ACC_TYPE_ADD:

            pre_info->is_read = 0;
            sal_memcpy(pre_info->data, (uint32*)acc_in->data, sizeof(DsMplsLabelHashKey_m));
            break;

         case DRV_ACC_TYPE_LOOKUP:
            pre_info->is_read = 1;
            break;

         case DRV_ACC_TYPE_ALLOC:
         case DRV_ACC_TYPE_DEL:
            pre_info->is_read = 0;
            break;

        default:
            return DRV_E_INVAILD_TYPE;

    }

    if (pre_info->key_index < cam_num)
    {
        pre_info->is_cam = 1;
        pre_info->cam_table = tbl_id;
    }
    else
    {
        pre_info->is_cam = 0;
    }


    return 0;
}


int32
drv_usw_acc_mpls_process(uint8 lchip, drv_cpu_acc_prepare_info_t* p_info)
{
    uint32 cmdr                        = 0;
    uint32 cmdw                        = 0;

    /*check whether using cam*/
    if (p_info->is_cam)
    {
        /*TBD*/
        /*If software alloc cam using, alloc cam index do not using acc return index*/

        /*If cam not shared, using acc return index, SDK should using this mode, check spec*/
    }

    if (p_info->is_cam)
    {

        cmdr = DRV_IOR(p_info->cam_table, DRV_ENTRY_FLAG);
        if (!p_info->is_read)
        {
            cmdw = DRV_IOW(p_info->cam_table, DRV_ENTRY_FLAG);
            DRV_DBG_OUT("Write %s (%d)\n", TABLE_NAME(lchip, p_info->cam_table), p_info->key_index);
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, p_info->key_index, cmdw, p_info->data));
        }
        else
        {
            cmdr = DRV_IOR(p_info->cam_table, DRV_ENTRY_FLAG);
            DRV_DBG_OUT("Read %s (%d)\n", TABLE_NAME(lchip, p_info->cam_table), p_info->key_index);
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, p_info->key_index, cmdr, p_info->data));
        }
    }
    else
    {

        if (!p_info->is_read)
        {
            cmdw = DRV_IOW(p_info->tbl_id, DRV_ENTRY_FLAG);
            DRV_DBG_OUT("Write %s (%d)\n", TABLE_NAME(lchip, p_info->tbl_id), (p_info->key_index - p_info->cam_num));
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, (p_info->key_index - p_info->cam_num), cmdw, p_info->data));
        }
        else
        {
            cmdr = DRV_IOR(p_info->tbl_id, DRV_ENTRY_FLAG);
            DRV_DBG_OUT("Read %s (%d)\n", TABLE_NAME(lchip, p_info->tbl_id), (p_info->key_index - p_info->cam_num));
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, (p_info->key_index - p_info->cam_num), cmdr, p_info->data));
        }
    }

    return DRV_E_NONE;
}

int32
drv_usw_acc_mpls_result(uint8 lchip, drv_acc_in_t* acc_in, drv_cpu_acc_prepare_info_t* pre_info, drv_acc_out_t* out)
{
    out->is_conflict = pre_info->conflict;
    out->is_hit = pre_info->valid;
    out->key_index = pre_info->key_index;
    out->is_left = pre_info->is_left;
    out->offset = pre_info->is_high;

    if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
    {
        sal_memcpy((uint8*)out->data, (uint8*)pre_info->data, TABLE_ENTRY_SIZE(lchip, pre_info->tbl_id));
    }

    return 0;
}


#define __ACC_GEMPORT__
int32
drv_usw_acc_gemport_lookup(uint8 lchip, drv_acc_in_t* acc_in, drv_cpu_acc_prepare_info_t* pre_info)
{
    uint32 cmd = 0;

    uint32 loop = 0;
    uint32 count = 0;
    uint32 done = 0;
    GemPortCpuLookupReq_m req;
    GemPortCpuLookupResult_m result;

    sal_memset(&req, 0, sizeof(req));
    sal_memset(&result, 0, sizeof(result));

    cmd = DRV_IOW(GemPortCpuLookupResult_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &result));

    SetGemPortCpuLookupReq(V, lookupValid_f, &req, 1);
    SetGemPortCpuLookupReq(A, data_f, &req, acc_in->data);

    cmd = DRV_IOW(GemPortCpuLookupReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &req));

    cmd = DRV_IOR(GemPortCpuLookupResult_t, DRV_ENTRY_FLAG);

    for (loop = 0; (loop < DRV_APP_LOOP_CNT) && !done; loop++)
    {
        if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
        {
            done = 1;
            break;
        }

        for (count = 0; (count < 150) && !done; count++)
        {
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &result));
            done = GetGemPortCpuLookupResult(V, lookupReqDone_f, &result);
        }

        if (done)
        {
            break;
        }
        sal_task_sleep(1);
    }

    if (!done)
    {
        DRV_IF_ERROR_RETURN(DRV_E_CMD_NOT_DONE);
    }

    pre_info->conflict = GetGemPortCpuLookupResult(V, conflict_f, &result);
    pre_info->valid = GetGemPortCpuLookupResult(V, lookupResultValid_f, &result);

    /*acording lookup result build prepare information*/
    if (!GetGemPortCpuLookupResult(V, conflict_f, &result))
    {
        pre_info->key_index = GetGemPortCpuLookupResult(V, resultIndex_f, &result);
    }
    DRV_DBG_OUT("=============================\n");
    DRV_DBG_OUT("Lookup keyinex:%d\n", pre_info->key_index);
    DRV_DBG_OUT("conflict:%d\n", pre_info->conflict);
    DRV_DBG_OUT("valid:%d\n", pre_info->valid);



    return DRV_E_NONE;
}

int32
drv_usw_acc_gemport_prepare(uint8 lchip, drv_acc_in_t* acc_in, drv_cpu_acc_prepare_info_t* pre_info)
{
    uint32 req_valid = 0;
    uint32 tbl_id = 0;
    uint8 cam_num = 0;

    DRV_IF_ERROR_RETURN(drv_usw_get_cam_info(lchip, DRV_ACC_HASH_MODULE_GEMPORT_HASH, &tbl_id, &cam_num));

    pre_info->tbl_id = acc_in->tbl_id;
    pre_info->key_index = acc_in->index;
    pre_info->cam_num = cam_num;


    /* check valid bit clear */
    DRV_IF_ERROR_RETURN(_drv_usw_acc_hash_get_valid(lchip, DRV_ACC_HASH_MODULE_GEMPORT_HASH, &req_valid));
    if (req_valid)
    {
        return DRV_E_OCCPANCY;
    }


    if (acc_in->op_type != DRV_ACC_OP_BY_INDEX)
    {
        DRV_IF_ERROR_RETURN(drv_usw_acc_gemport_lookup(lchip, acc_in, pre_info));
    }

    switch(acc_in->type)
    {
        case DRV_ACC_TYPE_ADD:

            pre_info->is_read = 0;
            sal_memcpy(pre_info->data, (uint32*)acc_in->data, sizeof(DsGemPortHashKey_m));
            break;

         case DRV_ACC_TYPE_LOOKUP:
            pre_info->is_read = 1;
            break;

         case DRV_ACC_TYPE_ALLOC:
         case DRV_ACC_TYPE_DEL:
            pre_info->is_read = 0;
            break;

        default:
            return DRV_E_INVAILD_TYPE;

    }

    if (pre_info->key_index < cam_num)
    {
        pre_info->is_cam = 1;
        pre_info->cam_table = tbl_id;
    }
    else
    {
        pre_info->is_cam = 0;
    }


    return 0;
}


int32
drv_usw_acc_gemport_process(uint8 lchip, drv_cpu_acc_prepare_info_t* p_info)
{
    uint32 cmdr                        = 0;
    uint32 cmdw                        = 0;

    /*check whether using cam*/
    if (p_info->is_cam)
    {
        /*TBD*/
        /*If software alloc cam using, alloc cam index do not using acc return index*/

        /*If cam not shared, using acc return index, SDK should using this mode, check spec*/
    }

    if (p_info->is_cam)
    {

        cmdr = DRV_IOR(p_info->cam_table, DRV_ENTRY_FLAG);
        if (!p_info->is_read)
        {
            cmdw = DRV_IOW(p_info->cam_table, DRV_ENTRY_FLAG);
            DRV_DBG_OUT("Write %s (%d)\n", TABLE_NAME(lchip, p_info->cam_table), p_info->key_index);
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, p_info->key_index, cmdw, p_info->data));
        }
        else
        {
            cmdr = DRV_IOR(p_info->cam_table, DRV_ENTRY_FLAG);
            DRV_DBG_OUT("Read %s (%d)\n", TABLE_NAME(lchip, p_info->cam_table), p_info->key_index);
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, p_info->key_index, cmdr, p_info->data));
        }
    }
    else
    {

        if (!p_info->is_read)
        {
            cmdw = DRV_IOW(p_info->tbl_id, DRV_ENTRY_FLAG);
            DRV_DBG_OUT("Write %s (%d)\n", TABLE_NAME(lchip, p_info->tbl_id), (p_info->key_index - p_info->cam_num));
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, (p_info->key_index - p_info->cam_num), cmdw, p_info->data));
        }
        else
        {
            cmdr = DRV_IOR(p_info->tbl_id, DRV_ENTRY_FLAG);
            DRV_DBG_OUT("Read %s (%d)\n", TABLE_NAME(lchip, p_info->tbl_id), (p_info->key_index - p_info->cam_num));
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, (p_info->key_index - p_info->cam_num), cmdr, p_info->data));
        }
    }

    return DRV_E_NONE;
}

int32
drv_usw_acc_gemport_result(uint8 lchip, drv_acc_in_t* acc_in, drv_cpu_acc_prepare_info_t* pre_info, drv_acc_out_t* out)
{
    out->is_conflict = pre_info->conflict;
    out->is_hit = pre_info->valid;
    out->key_index = pre_info->key_index;
    out->is_left = pre_info->is_left;
    out->offset = pre_info->is_high;

    if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
    {
        sal_memcpy((uint8*)out->data, (uint8*)pre_info->data, TABLE_ENTRY_SIZE(lchip, pre_info->tbl_id));
    }

    return 0;
}




#define __ACC_START__
/*only for fdb acc*/
int32
drv_usw_acc_fdb(uint8 lchip, drv_acc_in_t* acc_in, drv_acc_out_t* acc_out)
{
    uint32 cmd = 0;
    FibHashKeyCpuReq_m fib_req;
    FibHashKeyCpuResult_m fib_result;
    uint32  mem_db_tbl_id = 0;
    uint32  mem_db_index  = 0;
    tbl_entry_t mem_db_tbl_entry;
    uint8   hash_module = 0;


    sal_memset(&fib_result, 0, sizeof(FibHashKeyCpuResult_m));

    FIB_ACC_LOCK(lchip);
    /* check valid bit clear */
    cmd = DRV_IOR(FibHashKeyCpuReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &fib_req), p_drv_master[lchip]->fib_acc_mutex);
    if (GetFibHashKeyCpuReq(V,hashKeyCpuReqValid_f,&fib_req))
    {
        FIB_ACC_UNLOCK(lchip);
        return DRV_E_OCCPANCY;
    }

    sal_memset(&fib_req, 0, sizeof(FibHashKeyCpuReq_m));

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_fdb_prepare(lchip, acc_in, (uint32*)&fib_req),
        p_drv_master[lchip]->fib_acc_mutex);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_host0_process(lchip, (uint32*)&fib_req, (uint32*)&fib_result),
        p_drv_master[lchip]->fib_acc_mutex);
    FIB_ACC_UNLOCK(lchip);
    DRV_IF_ERROR_RETURN(drv_usw_acc_fdb_result(lchip, acc_in, (uint32*)&fib_result, acc_out));

    if (acc_in->type == DRV_ACC_TYPE_ADD)
    {
        hw_mac_addr_t   hw_mac = {0};

        GetDsFibHost0MacHashKey(A, mappedMac_f, acc_in->data, hw_mac);
        /*mcast need config valid and hash type*/
        if(hw_mac[1] & 0x100)
        {
            SetDsFibHost0MacHashKey(V, hashType_f, acc_in->data, 4);/*FIBHOST0PRIMARYHASHTYPE_MAC*/
            SetDsFibHost0MacHashKey(V, valid_f, acc_in->data, 1);
        }

        mem_db_tbl_id = acc_in->tbl_id;
        mem_db_tbl_entry.data_entry = acc_in->data;
        mem_db_tbl_entry.mask_entry = NULL;
        if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
        {
            mem_db_index  = acc_in->index;
        }

        if (acc_in->op_type == DRV_ACC_OP_BY_KEY)
        {
            mem_db_index  = acc_out->key_index;
        }

        drv_usw_acc_get_hash_module_from_tblid(lchip, mem_db_tbl_id, &hash_module);
        if (DRV_ACC_HASH_MODULE_FDB == hash_module)
        {
            DRV_IF_ERROR_RETURN(drv_ser_db_store(lchip, mem_db_tbl_id, mem_db_index, (void*)&mem_db_tbl_entry));
        }
    }

    return DRV_E_NONE;
}

/* for host0 acc*/
int32
drv_usw_acc_host0(uint8 lchip, drv_acc_in_t* acc_in, drv_acc_out_t* acc_out)
{
    uint32 cmd = 0;
    FibHashKeyCpuReq_m fib_req;
    FibHashKeyCpuResult_m fib_result;
    uint32  mem_db_tbl_id = 0;
    uint32  mem_db_index  = 0;
    tbl_entry_t mem_db_tbl_entry;
    uint8   hash_module = 0;

    sal_memset(&fib_result, 0, sizeof(FibHashKeyCpuResult_m));

    FIB_ACC_LOCK(lchip);
    /* check valid bit clear */
    cmd = DRV_IOR(FibHashKeyCpuReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &fib_req), p_drv_master[lchip]->fib_acc_mutex);
    if (GetFibHashKeyCpuReq(V,hashKeyCpuReqValid_f,&fib_req))
    {
        FIB_ACC_UNLOCK(lchip);
        return DRV_E_OCCPANCY;
    }

    sal_memset(&fib_req, 0, sizeof(FibHashKeyCpuReq_m));

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_host0_prepare(lchip, acc_in, (uint32*)&fib_req),
        p_drv_master[lchip]->fib_acc_mutex);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_host0_process(lchip, (uint32*)&fib_req, (uint32*)&fib_result),
        p_drv_master[lchip]->fib_acc_mutex);
    FIB_ACC_UNLOCK(lchip);
    DRV_IF_ERROR_RETURN(drv_usw_acc_host0_result(lchip, acc_in, (uint32*)&fib_result, acc_out));

    /***MEM_DB BACKUP STORE***
    D2:Will get fibhost0 and fdb tbl,but only want fibhost0/fdb tbl.
    TM:Drv-io will get fibhost0 tbl  too.Do not backup here.
    */
    if (DRV_IS_DUET2(lchip))
    {
        if (acc_in->type == DRV_ACC_TYPE_ADD)
        {
            mem_db_tbl_id = acc_in->tbl_id;
            mem_db_tbl_entry.data_entry = acc_in->data;
            mem_db_tbl_entry.mask_entry = NULL;
            if (acc_in->op_type == DRV_ACC_OP_BY_INDEX)
            {
                mem_db_index  = acc_in->index;
            }

            if (acc_in->op_type == DRV_ACC_OP_BY_KEY)
            {
                mem_db_index  = acc_out->key_index;
            }

            drv_usw_acc_get_hash_module_from_tblid(lchip, mem_db_tbl_id, &hash_module);
            if (DRV_ACC_HASH_MODULE_FIB_HOST0 == hash_module)
            {
                DRV_IF_ERROR_RETURN(drv_ser_db_store(lchip, mem_db_tbl_id, mem_db_index, (void*)&mem_db_tbl_entry));
            }
        }
    }

    return DRV_E_NONE;
}

/* for host1/xcoam/flow/userid acc*/
int32
drv_usw_acc_hash(uint8 lchip, uint8 hash_module, drv_acc_in_t* acc_in, drv_acc_out_t* acc_out)
{
    drv_cpu_acc_prepare_info_t pre_info;
    uint32 write_data[12] = { 0 };

    sal_memset(&pre_info, 0, sizeof(drv_cpu_acc_prepare_info_t));
    pre_info.data = write_data;

    CPU_ACC_LOCK(lchip);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_hash_prepare(lchip, hash_module, acc_in, &pre_info, acc_out),
        p_drv_master[lchip]->cpu_acc_mutex);

    if ((!pre_info.conflict) && !((acc_in->type == DRV_ACC_TYPE_ALLOC) && pre_info.valid))
    {
        /* prepare conflict, should do nothing */
        DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_hash_process(lchip, &pre_info), p_drv_master[lchip]->cpu_acc_mutex);
    }

    CPU_ACC_UNLOCK(lchip);
    DRV_IF_ERROR_RETURN(drv_usw_acc_hash_result(lchip, acc_in, &pre_info, acc_out));

    return DRV_E_NONE;
}

/*for mac limit process*/
int32
drv_usw_acc_mac_limit(uint8 lchip, drv_acc_in_t* acc_in, drv_acc_out_t* acc_out)
{
    uint32 cmd = 0;
    FibHashKeyCpuReq_m fib_req;
    FibHashKeyCpuResult_m fib_result;

    sal_memset(&fib_result, 0, sizeof(FibHashKeyCpuResult_m));

    FIB_ACC_LOCK(lchip);
    /* check valid bit clear */
    cmd = DRV_IOR(FibHashKeyCpuReq_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &fib_req),p_drv_master[lchip]->fib_acc_mutex);
    if (GetFibHashKeyCpuReq(V,hashKeyCpuReqValid_f,&fib_req))
    {
        FIB_ACC_UNLOCK(lchip);
        return DRV_E_OCCPANCY;
    }

    sal_memset(&fib_req, 0, sizeof(FibHashKeyCpuReq_m));

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_mac_limit_prepare(lchip, acc_in, (uint32*)&fib_req),
        p_drv_master[lchip]->fib_acc_mutex);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_host0_process(lchip, (uint32*)&fib_req, (uint32*)&fib_result),
        p_drv_master[lchip]->fib_acc_mutex);
    FIB_ACC_UNLOCK(lchip);
    DRV_IF_ERROR_RETURN(drv_usw_acc_mac_limit_result(lchip, acc_in, (uint32*)&fib_result, acc_out));

    return DRV_E_NONE;
}

/*only for ipfix acc*/
int32
drv_usw_acc_ipfix(uint8 lchip, drv_acc_in_t* acc_in, drv_acc_out_t* acc_out)
{
    uint32 cmd = 0;
    uint32 req_tbl_id = 0;
    IpfixHashAdCpuReq_m ipfix_req;
    IpfixHashAdCpuResult_m ipfix_result;

    sal_memset(&ipfix_result, 0, sizeof(IpfixHashAdCpuResult_m));
	sal_memset(&ipfix_req, 0, sizeof(IpfixHashAdCpuReq_m));

    IPFIX_ACC_LOCK(lchip);
    /* check valid bit clear */
    if (DRV_IS_TSINGMA(lchip))
    {
        switch(acc_in->tbl_id)
        {
        case DsIpfixL2HashKey0_t:
        case DsIpfixL2L3HashKey0_t:
        case DsIpfixL3Ipv4HashKey0_t:
        case DsIpfixL3Ipv6HashKey0_t:
        case DsIpfixL3MplsHashKey0_t:
        case DsIpfixSessionRecord0_t:
            acc_in->dir = 0;
            req_tbl_id = IpfixHashAdCpuReq0_t;
            break;
        case DsIpfixL2HashKey1_t:
        case DsIpfixL2L3HashKey1_t:
        case DsIpfixL3Ipv4HashKey1_t:
        case DsIpfixL3Ipv6HashKey1_t:
        case DsIpfixL3MplsHashKey1_t:
        case DsIpfixSessionRecord1_t:
            acc_in->dir = 1;
            req_tbl_id = IpfixHashAdCpuReq1_t;
            break;

        default:
            acc_in->dir = 0;
            req_tbl_id = IpfixHashAdCpuReq0_t;
            break;
        }


    }
    else
    {
        req_tbl_id = IpfixHashAdCpuReq_t;
    }

    cmd = DRV_IOR(req_tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(0, 0, cmd, &ipfix_req), p_drv_master[lchip]->ipfix_acc_mutex);
    if (GetIpfixHashAdCpuReq(V, cpuReqValid_f, &ipfix_req))
    {
        IPFIX_ACC_UNLOCK(lchip);
        return DRV_E_OCCPANCY;
    }

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_ipfix_prepare(lchip, acc_in, (uint32*)&ipfix_req),
        p_drv_master[lchip]->ipfix_acc_mutex);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_ipfix_process(lchip, acc_in->dir, (uint32*)&ipfix_req, (uint32*)&ipfix_result),
        p_drv_master[lchip]->ipfix_acc_mutex);
    IPFIX_ACC_UNLOCK(lchip);
    DRV_IF_ERROR_RETURN(drv_usw_acc_ipfix_result(lchip, acc_in, (uint32*)&ipfix_result, acc_out));

    return DRV_E_NONE;
}


/* for cid acc*/
int32
drv_usw_acc_cid(uint8 lchip, drv_acc_in_t* acc_in, drv_acc_out_t* acc_out)
{
    drv_cpu_acc_prepare_info_t pre_info;
    uint32 write_data[12] = { 0 };

    sal_memset(&pre_info, 0, sizeof(drv_cpu_acc_prepare_info_t));
    pre_info.data = write_data;

    CID_ACC_LOCK(lchip);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_cid_prepare(lchip, acc_in, &pre_info),
        p_drv_master[lchip]->cid_acc_mutex);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_cid_process(lchip, &pre_info),
        p_drv_master[lchip]->cid_acc_mutex);
    CID_ACC_UNLOCK(lchip);
    DRV_IF_ERROR_RETURN(drv_usw_acc_cid_result(lchip, acc_in, &pre_info, acc_out));

    return DRV_E_NONE;
}


/* for mpls acc*/
int32
drv_usw_acc_mpls(uint8 lchip, drv_acc_in_t* acc_in, drv_acc_out_t* acc_out)
{
    drv_cpu_acc_prepare_info_t pre_info;
    uint32 write_data[12] = { 0 };

    sal_memset(&pre_info, 0, sizeof(drv_cpu_acc_prepare_info_t));
    pre_info.data = write_data;

    MPLS_ACC_LOCK(lchip);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_mpls_prepare(lchip, acc_in, &pre_info),
                                    p_drv_master[lchip]->mpls_acc_mutex);

    if ((!pre_info.conflict) && !((acc_in->type == DRV_ACC_TYPE_ALLOC) && pre_info.valid))
    {
        /* prepare conflict, should do nothing */
        DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_mpls_process(lchip, &pre_info),
                                        p_drv_master[lchip]->mpls_acc_mutex);
    }

    MPLS_ACC_UNLOCK(lchip);
    DRV_IF_ERROR_RETURN(drv_usw_acc_mpls_result(lchip, acc_in, &pre_info, acc_out));

    return DRV_E_NONE;
}


/* for mpls acc*/
int32
drv_usw_acc_gemport(uint8 lchip, drv_acc_in_t* acc_in, drv_acc_out_t* acc_out)
{
    drv_cpu_acc_prepare_info_t pre_info;
    uint32 write_data[12] = { 0 };

    sal_memset(&pre_info, 0, sizeof(drv_cpu_acc_prepare_info_t));
    pre_info.data = write_data;

    GEMPORT_ACC_LOCK(lchip);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_gemport_prepare(lchip, acc_in, &pre_info),
                                    p_drv_master[lchip]->gemport_acc_mutex);

    if ((!pre_info.conflict) && !((acc_in->type == DRV_ACC_TYPE_ALLOC) && pre_info.valid))
    {
        DRV_IF_ERROR_RETURN_WITH_UNLOCK(drv_usw_acc_gemport_process(lchip, &pre_info),
                                        p_drv_master[lchip]->gemport_acc_mutex);
    }
    GEMPORT_ACC_UNLOCK(lchip);
    DRV_IF_ERROR_RETURN(drv_usw_acc_gemport_result(lchip, acc_in, &pre_info, acc_out));

    return DRV_E_NONE;
}



/*drv layer acc interface*/
int32
drv_usw_acc(uint8 lchip, void* in, void* out)
{
    drv_acc_in_t* p_acc = NULL;
    uint8 hash_module = 0;

    DRV_PTR_VALID_CHECK(in);
    DRV_PTR_VALID_CHECK(out);

    p_acc = (drv_acc_in_t*)in;

    DRV_IF_ERROR_RETURN(drv_usw_acc_get_hash_module_from_tblid(lchip, p_acc->tbl_id, &hash_module));

    if (hash_module == DRV_ACC_HASH_MODULE_FDB)
    {
        if ((p_acc->type == DRV_ACC_TYPE_LOOKUP) && (p_acc->op_type == DRV_ACC_OP_BY_INDEX))
        {
            DRV_IF_ERROR_RETURN(drv_usw_acc_host0(lchip, in, out));
        }
        else if ((p_acc->type == DRV_ACC_TYPE_CALC) && (p_acc->op_type == DRV_ACC_OP_BY_KEY))
        {
            if (DRV_IS_DUET2(lchip))
            {
                DRV_IF_ERROR_RETURN(_drv_usw_host0_calc_fdb_index(lchip, in, out));
            }
            else
            {
                DRV_IF_ERROR_RETURN(_drv_usw_host0_calc_fdb_index_tsingma(lchip, in, out));
            }
        }
        else
        {
            DRV_IF_ERROR_RETURN(drv_usw_acc_fdb(lchip, in, out));
        }
    }
    else if (hash_module == DRV_ACC_HASH_MODULE_FIB_HOST0)
    {
        if (DRV_IS_DUET2(lchip))
        {
            DRV_IF_ERROR_RETURN(drv_usw_acc_host0(lchip, in, out));
        }
        else
        {
            DRV_IF_ERROR_RETURN(drv_usw_acc_hash(lchip, hash_module, in, out));
        }
    }
    else if ((hash_module == DRV_ACC_HASH_MODULE_FIB_HOST1) || (hash_module <= DRV_ACC_HASH_MODULE_EGRESS_XC) ||
            (hash_module == DRV_ACC_HASH_MODULE_FLOW) || (hash_module == DRV_ACC_HASH_MODULE_USERID))
    {
        DRV_IF_ERROR_RETURN(drv_usw_acc_hash(lchip, hash_module, in, out));
    }
    else if (hash_module == DRV_ACC_HASH_MODULE_MAC_LIMIT)
    {
        DRV_IF_ERROR_RETURN(drv_usw_acc_mac_limit(lchip, in, out));
    }
    else if (hash_module == DRV_ACC_HASH_MODULE_IPFIX)
    {
        DRV_IF_ERROR_RETURN(drv_usw_acc_ipfix(lchip, in, out));
    }
    else if (hash_module == DRV_ACC_HASH_MODULE_CID)
    {
        DRV_IF_ERROR_RETURN(drv_usw_acc_cid(lchip, in, out));
    }
    else if (hash_module == DRV_ACC_HASH_MODULE_AGING)
    {
        DRV_IF_ERROR_RETURN(drv_usw_acc_aging(lchip, in, out));
    }
    else if(hash_module == DRV_ACC_HASH_MODULE_MPLS_HASH)
    {
        DRV_IF_ERROR_RETURN(drv_usw_acc_mpls(lchip, in, out));
    }
    else if(hash_module == DRV_ACC_HASH_MODULE_GEMPORT_HASH)
    {
        DRV_IF_ERROR_RETURN(drv_usw_acc_gemport(lchip, in, out));
    }
    else
    {
        return DRV_E_INVAILD_TYPE;
    }

    return DRV_E_NONE;
}


int32
drv_usw_ftm_get_entry_size(uint8 lchip, uint32 table_id, uint32* entry_size)
{
    *entry_size = TABLE_ENTRY_SIZE(lchip, table_id);

    return DRV_E_NONE;
}

