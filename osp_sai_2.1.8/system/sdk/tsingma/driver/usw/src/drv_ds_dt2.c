#ifdef DUET2

#include "usw/include/drv_common.h"
#include "usw/include/drv_enum.h"
#include "usw/include/drv_ftm.h"
#include "usw/include/drv_ser.h"
#include "usw/include/drv_chip_ctrl.h"



#undef DRV_DEF_C
#undef DRV_DEF_M
#undef DRV_DEF_D
#undef DRV_DEF_F
#undef DRV_DEF_E
#undef DRV_DEF_SDK_D
#undef DRV_DEF_SDK_F

#ifdef DRV_DEF_C
	#error DRV_DEF_C has been defined
#endif

#ifdef DRV_DEF_M
	#error DRV_DEF_M has been defined
#endif

#ifdef DRV_DEF_D
	#error DRV_DEF_D has been defined
#endif

#ifdef DRV_DEF_F
	#error DRV_DEF_F has been defined
#endif

#ifdef DRV_DEF_E
	#error DRV_DEF_E has been defined
#endif

#ifdef DRV_DEF_SDK_D
	#error DRV_DEF_SDK_D has been defined
#endif

#ifdef DRV_DEF_SDK_F
	#error DRV_DEF_SDK_F has been defined
#endif

extern tables_info_t drv_tm_tbls_list[];

 /*DS ADDR*/
#define CTC_DS_ADDR(ModuleName, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, ...) \
   static addrs_t RegName##_tbl_addrs_dt2[] = {__VA_ARGS__};

 /*DS LIST*/
#define CTC_DS_INFO(ModuleName, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, ...) \
   { \
      #ModuleName, \
      #RegName, \
      SliceType, \
      OpType, \
      Entry, \
      Word*4, \
      DB,     \
      ECC,    \
      Bus,    \
      Stats,  \
      sizeof(RegName##_tbl_addrs_dt2)/sizeof(addrs_t), \
      sizeof(RegName##_tbl_fields_dt2)/sizeof(fields_t), \
      RegName##_tbl_addrs_dt2, \
      RegName##_tbl_fields_dt2, \
   },

#define CTC_DS_INFO_E() \
   { \
      "Invalid", \
      "Invalid", \
      0, \
      0, \
      0, \
      0, \
      0, \
      0, \
      0, \
      0, \
      0, \
      0, \
      NULL, \
      NULL, \
   },

 /*Field Addr*/
#define CTC_FIELD_ADDR(ModuleName, RegName, FieldName, FullName, Bits, ...) \
  static segs_t RegName##_##FieldName##_tbl_segs_dt2[] = {__VA_ARGS__};

 /*Field Info*/
#define CTC_FIELD_INFO(ModuleName, RegName, FieldName, FullName, Bits, ...) \
   { \
      #FullName, \
	  RegName##_##FieldName##_f,\
      Bits, \
      sizeof(RegName##_##FieldName##_tbl_segs_dt2) / sizeof(segs_t), \
      RegName##_##FieldName##_tbl_segs_dt2, \
   },

 /*DS Field List Seperator*/
#define CTC_DS_SEPERATOR_INFO(ModuleName, RegName, SliceType, OpType, Entry, Word, ...) \
 }; \
 static fields_t RegName##_tbl_fields_dt2[] = {


#define DRV_DEF_C(MaxInstNum, MaxEntry, MaxWord, MaxBits,MaxStartPos,MaxSegSize)

 /* Segment Info*/
#define DRV_DEF_M(ModuleName, InstNum)
#ifdef DRV_DS_LITE
#define DRV_DEF_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, ...)
#define DRV_DEF_F(ModuleName, InstNum, RegName, FieldName, FullName,Bits, ...)
#else
#define DRV_DEF_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, ...)
#define DRV_DEF_F(ModuleName, InstNum, RegName, FieldName, FullName,Bits, ...) \
        CTC_FIELD_ADDR(ModuleName, RegName, FieldName, FullName, Bits, __VA_ARGS__)
#endif
#define DRV_DEF_E()
#define DRV_DEF_SDK_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, ...)
#define DRV_DEF_SDK_F(ModuleName, InstNum, RegName, FieldName, FullName,Bits, ...) \
        CTC_FIELD_ADDR(ModuleName, RegName, FieldName, FullName, Bits, __VA_ARGS__)
#include "usw/include/drv_ds_dt2.h"
#undef DRV_DEF_M
#undef DRV_DEF_D
#undef DRV_DEF_F
#undef DRV_DEF_E
#undef DRV_DEF_SDK_D
#undef DRV_DEF_SDK_F

 /* Field Info*/
#define DRV_DEF_M(ModuleName, InstNum)
#ifdef DRV_DS_LITE
#define DRV_DEF_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, ...)
#define DRV_DEF_F(ModuleName, InstNum, RegName, FieldName, FullName,Bits, ...)

#else
#define DRV_DEF_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, ...) \
        CTC_DS_SEPERATOR_INFO(ModuleName, RegName, SliceType, OpType, Entry, Word, __VA_ARGS__)
#define DRV_DEF_F(ModuleName, InstNum, RegName, FieldName,FullName, Bits, ...) \
        CTC_FIELD_INFO(ModuleName, RegName, FieldName, FullName, Bits, __VA_ARGS__)
#endif
#define DRV_DEF_E()
#define DRV_DEF_SDK_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, ...) \
        CTC_DS_SEPERATOR_INFO(ModuleName, RegName, SliceType, OpType, Entry, Word, __VA_ARGS__)
#define DRV_DEF_SDK_F(ModuleName, InstNum, RegName, FieldName,FullName, Bits, ...) \
        CTC_FIELD_INFO(ModuleName, RegName, FieldName, FullName, Bits, __VA_ARGS__)
fields_t dt2_fields_1st = {NULL,0,0,0,NULL
#include "usw/include/drv_ds_dt2.h"
};

#undef DRV_DEF_M
#undef DRV_DEF_D
#undef DRV_DEF_F
#undef DRV_DEF_E
#undef DRV_DEF_SDK_D
#undef DRV_DEF_SDK_F

 /* DS Address*/
#define DRV_DEF_M(ModuleName, InstNum)
#ifdef DRV_DS_LITE
#define DRV_DEF_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, ...)
#define DRV_DEF_F(ModuleName, InstNum, RegName, FieldName, Bits, ...)
#else
#define DRV_DEF_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, ...) \
        CTC_DS_ADDR(ModuleName, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, __VA_ARGS__)
#define DRV_DEF_F(ModuleName, InstNum, RegName, FieldName,  Bits, ...)
#endif
#define DRV_DEF_SDK_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, ...) \
        CTC_DS_ADDR(ModuleName, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, __VA_ARGS__)
#define DRV_DEF_SDK_F(ModuleName, InstNum, RegName, FieldName, Bits, ...)
#define DRV_DEF_E()
#include "usw/include/drv_ds_dt2.h"
#undef DRV_DEF_M
#undef DRV_DEF_D
#undef DRV_DEF_F
#undef DRV_DEF_E
#undef DRV_DEF_SDK_D
#undef DRV_DEF_SDK_F

 /* DS List*/
#define DRV_DEF_M(ModuleName, InstNum)
#ifdef DRV_DS_LITE
#define DRV_DEF_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, ...)
#define DRV_DEF_F(ModuleName, InstNum, RegName, FieldName, Bits, ...)
#else
#define DRV_DEF_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, ...) \
        CTC_DS_INFO(ModuleName, RegName, SliceType, OpType, Entry, Word, __VA_ARGS__)
#define DRV_DEF_F(ModuleName, InstNum, RegName, FieldName, Bits, ...)
#endif
#define DRV_DEF_E() \
        CTC_DS_INFO_E()
#define DRV_DEF_SDK_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, ...) \
        CTC_DS_INFO(ModuleName, RegName, SliceType, OpType, Entry, Word, __VA_ARGS__)
#define DRV_DEF_SDK_F(ModuleName, InstNum, RegName, FieldName, Bits, ...)
tables_info_t drv_dt2_tbls_list[] = {
#include "usw/include/drv_ds_dt2.h"
};

/*ECC Interrupt*/
#define DRV_ECC_F(...)  {__VA_ARGS__},



#define MCU0_MUTEX0_CPU_DATA   0x00080080
#define MCU0_MUTEX0_CPU_MASK   0x00080084
#define MCU0_MUTEX1_CPU_DATA   0x00080088
#define MCU0_MUTEX1_CPU_MASK   0x0008008C

#define MCU1_MUTEX0_CPU_DATA   0x00040080
#define MCU1_MUTEX0_CPU_MASK   0x00040084
#define MCU1_MUTEX1_CPU_DATA   0x00040088
#define MCU1_MUTEX1_CPU_MASK   0x0004008C



uint32 drv_tm_enum[DRV_ENUM_MAX];

drv_mem_t drv_dt2_mem[DRV_FTM_MAX_ID];

drv_ecc_intr_tbl_t drv_ecc_dt2_err_intr_tbl[] =
{
#include "usw/include/drv_ecc_intr_d2.h"
    {MaxTblId_t,0,0,0,0,0,0,0,0}
};

drv_ecc_sbe_cnt_t drv_ecc_dt2_sbe_cnt[] =
{
    {BufRetrvFirstFragRam_t,NULL,BufRetrvParityStatus_t,BufRetrvParityStatus_bufRetrvFirstFragRamSbeCnt_f},
    {BufRetrvLinkListRam_t,NULL,BufRetrvParityStatus_t,BufRetrvParityStatus_bufRetrvLinkListRamSbeCnt_f},
    {BufRetrvMsgParkMem_t,NULL,BufRetrvParityStatus_t,BufRetrvParityStatus_bufRetrvMsgParkMemSbeCnt_f},
    {BufRetrvPktMsgMem_t,NULL,BufRetrvParityStatus_t,BufRetrvParityStatus_bufRetrvPktMsgMemSbeCnt_f},
    {BufStoreChanInfo_t,NULL,BufStoreParityStatus_t,BufStoreParityStatus_bufStoreChanInfoSbeCnt_f},
    {BufStoreFreeListRam_t,NULL,BufStoreParityStatus_t,BufStoreParityStatus_bufStoreFreeListRamSbeCnt_f},
    {DmaDescCache_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaDescCacheSbeCnt_f},
    {DmaInfoMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaInfoMemSbeCnt_f},
    {DmaPktRxMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaPktRxMemSbeCnt_f},
    {DmaPktTxFifo_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaPktTxFifoSbeCnt_f},
    {DmaRegRdMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaRegRdMemSbeCnt_f},
    {DmaRegWrMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaRegWrMemSbeCnt_f},
    {DmaUserRegMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaUserRegMemSbeCnt_f},
    {DmaWrReqDataFifo_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaWrReqDataFifoSbeCnt_f},
    {DsAclVlanActionProfile_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsAclVlanActionProfileSbeCnt_f},
    {DsAging_t,NULL,DsAgingParityStatus_t,DsAgingParityStatus_dsAgingSbeCnt_f},
    {DsAgingStatusFib_t,NULL,DsAgingParityStatus_t,DsAgingParityStatus_dsAgingStatusFibSbeCnt_f},
    {DsAgingStatusTcam_t,NULL,DsAgingParityStatus_t,DsAgingParityStatus_dsAgingStatusTcamSbeCnt_f},
    {DsBidiPimGroup_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_dsBidiPimGroupSbeCnt_f},
    {DsBufRetrvExcp_t,NULL,BufRetrvParityStatus_t,BufRetrvParityStatus_dsBufRetrvExcpSbeCnt_f},
    {DsCapwapFrag_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsCapwapFragSbeCnt_f},
    {DsCategoryIdPairHashLeftAd_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsCategoryIdPairHashLeftAdSbeCnt_f},
    {DsCategoryIdPairHashLeftKey_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsCategoryIdPairHashLeftKeySbeCnt_f},
    {DsCategoryIdPairHashRightAd_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsCategoryIdPairHashRightAdSbeCnt_f},
    {DsCategoryIdPairHashRightKey_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsCategoryIdPairHashRightKeySbeCnt_f},
    {DsCategoryIdPairTcamAd_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsCategoryIdPairTcamAdSbeCnt_f},
    {DsCFlexDstChannelBlockMask_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsCFlexDstChannelBlockMaskSbeCnt_f},
    {DsCFlexSrcPortBlockMask_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsCFlexSrcPortBlockMaskSbeCnt_f},
    {DsChannelizeIngFc_t,NULL,NetRxParityStatus_t,NetRxParityStatus_dsChannelizeIngFcSbeCnt_f},
    {DsChannelizeMode_t,NULL,NetRxParityStatus_t,NetRxParityStatus_dsChannelizeModeSbeCnt_f},
    {DsDestEthLmProfile_t,NULL,EpeAclOamParityStatus_t,EpeAclOamParityStatus_dsDestEthLmProfileSbeCnt_f},
    {DsDestInterface_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestInterfaceSbeCnt_f},
    {DsDestInterfaceProfile_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestInterfaceProfileSbeCnt_f},
    {DsDestMapProfileMc_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_dsDestMapProfileMcSbeCnt_f},
    {DsDestMapProfileUc_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsDestMapProfileUcSbeCnt_f},
    {DsDestPort_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestPortSbeCnt_f},
    {DsDestPortChannelMap_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsDestPortChannelMapSbeCnt_f},
    {DsDestStpState_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestStpStateSbeCnt_f},
    {DsDestVlan_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestVlanSbeCnt_f},
    {DsDestVlanProfile_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestVlanProfileSbeCnt_f},
    {DsDestVlanStatus_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestVlanStatusSbeCnt_f},
    {DsDot1AeAesKey_t,NULL,MacSecEngineParityStatus_t,MacSecEngineParityStatus_dsDot1AeAesKeySbeCnt_f},
    {DsDot1AeSci_t,NULL,MacSecEngineParityStatus_t,MacSecEngineParityStatus_dsDot1AeSciSbeCnt_f},
    {DsEcmpGroup_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsEcmpGroupSbeCnt_f},
    {DsEcmpMember_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsEcmpMemberSbeCnt_f},
    {DsEcnMappingAction_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsEcnMappingActionSbeCnt_f},
    {DsEgressDiscardStats_t,NULL,EpeHdrEditParityStatus_t,EpeHdrEditParityStatus_dsEgressDiscardStatsSbeCnt_f},
    {DsEgressDot1AeSci_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsEgressDot1AeSciSbeCnt_f},
    {DsEgressDtls_t,NULL,CapwapProcParityStatus_t,CapwapProcParityStatus_dsEgressDtlsSbeCnt_f},
    {DsEgressIpfixConfig_t,NULL,FlowAccParityStatus_t,FlowAccParityStatus_dsEgressIpfixConfigSbeCnt_f},
    {DsEgressIpfixSamplingCount_t,NULL,FlowAccParityStatus_t,FlowAccParityStatus_dsEgressIpfixSamplingCountSbeCnt_f},
    {DsEgressPortMac_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsEgressPortMacSbeCnt_f},
    {DsEgressRouterMac_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsEgressRouterMacSbeCnt_f},
    {DsEgressVlanRangeProfile_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsEgressVlanRangeProfileSbeCnt_f},
    {DsEgressVsi_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsEgressVsiSbeCnt_f},
    {DsElephantFlowState_t,NULL,EfdParityStatus_t,EfdParityStatus_dsElephantFlowStateSbeCnt_f},
    {DsEpePolicing0Config_t,NULL,PolicingEpeDebugStats_t,PolicingEpeDebugStats_dsEpePolicing0ConfigSbeCnt_f},
    {DsEpePolicing0CountX_t,NULL,PolicingEpeDebugStats_t,PolicingEpeDebugStats_dsEpePolicing0CountXSbeCnt_f},
    {DsEpePolicing0CountY_t,NULL,PolicingEpeDebugStats_t,PolicingEpeDebugStats_dsEpePolicing0CountYSbeCnt_f},
    {DsEpePolicing1Config_t,NULL,PolicingEpeDebugStats_t,PolicingEpeDebugStats_dsEpePolicing1ConfigSbeCnt_f},
    {DsEpePolicing1CountX_t,NULL,PolicingEpeDebugStats_t,PolicingEpeDebugStats_dsEpePolicing1CountXSbeCnt_f},
    {DsEpePolicing1CountY_t,NULL,PolicingEpeDebugStats_t,PolicingEpeDebugStats_dsEpePolicing1CountYSbeCnt_f},
    {DsErmAqmPortThrd_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsErmAqmPortThrdSbeCnt_f},
    {DsErmAqmQueueCfg_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsErmAqmQueueCfgSbeCnt_f},
    {DsErmAqmQueueStatus_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsErmAqmQueueStatusSbeCnt_f},
    {DsErmPortLimitedThrdProfile_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsErmPortLimitedThrdProfileSbeCnt_f},
    {DsErmQueueCfg_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsErmQueueCfgSbeCnt_f},
    {DsErmQueueLimitedThrdProfile_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsErmQueueLimitedThrdProfileSbeCnt_f},
    {DsErmResrcMonPortMax_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsErmResrcMonPortMaxSbeCnt_f},
    {DsGlbDestPort_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsGlbDestPortSbeCnt_f},
    {DsIngressIpfixConfig_t,NULL,FlowAccParityStatus_t,FlowAccParityStatus_dsIngressIpfixConfigSbeCnt_f},
    {DsIngressIpfixSamplingCount_t,NULL,FlowAccParityStatus_t,FlowAccParityStatus_dsIngressIpfixSamplingCountSbeCnt_f},
    {DsIpeCoPPConfig_t,NULL,CoppDebugStats_t,CoppDebugStats_dsIpeCoPPConfigSbeCnt_f},
    {DsIpeCoPPCountX_t,NULL,CoppDebugStats_t,CoppDebugStats_dsIpeCoPPCountXSbeCnt_f},
    {DsIpeCoPPCountY_t,NULL,CoppDebugStats_t,CoppDebugStats_dsIpeCoPPCountYSbeCnt_f},
    {DsIpePhbMutationCosMap_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsIpePhbMutationCosMapSbeCnt_f},
    {DsIpePhbMutationDscpMap_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsIpePhbMutationDscpMapSbeCnt_f},
    {DsIpePolicing0Config_t,NULL,PolicingIpeDebugStats_t,PolicingIpeDebugStats_dsIpePolicing0ConfigSbeCnt_f},
    {DsIpePolicing0CountX_t,NULL,PolicingIpeDebugStats_t,PolicingIpeDebugStats_dsIpePolicing0CountXSbeCnt_f},
    {DsIpePolicing0CountY_t,NULL,PolicingIpeDebugStats_t,PolicingIpeDebugStats_dsIpePolicing0CountYSbeCnt_f},
    {DsIpePolicing0ProfileX_t,NULL,PolicingIpeDebugStats_t,PolicingIpeDebugStats_dsIpePolicing0ProfileXSbeCnt_f},
    {DsIpePolicing0ProfileY_t,NULL,PolicingIpeDebugStats_t,PolicingIpeDebugStats_dsIpePolicing0ProfileYSbeCnt_f},
    {DsIpePolicing1Config_t,NULL,PolicingIpeDebugStats_t,PolicingIpeDebugStats_dsIpePolicing1ConfigSbeCnt_f},
    {DsIpePolicing1CountX_t,NULL,PolicingIpeDebugStats_t,PolicingIpeDebugStats_dsIpePolicing1CountXSbeCnt_f},
    {DsIpePolicing1CountY_t,NULL,PolicingIpeDebugStats_t,PolicingIpeDebugStats_dsIpePolicing1CountYSbeCnt_f},
    {DsIpePolicing1ProfileX_t,NULL,PolicingIpeDebugStats_t,PolicingIpeDebugStats_dsIpePolicing1ProfileXSbeCnt_f},
    {DsIpePolicing1ProfileY_t,NULL,PolicingIpeDebugStats_t,PolicingIpeDebugStats_dsIpePolicing1ProfileYSbeCnt_f},
    {DsIpePolicingPolicy_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsIpePolicingPolicySbeCnt_f},
    {DsIpeStormCtl0Config_t,NULL,StormCtlDebugStats_t,StormCtlDebugStats_dsIpeStormCtl0ConfigSbeCnt_f},
    {DsIpeStormCtl0CountX_t,NULL,StormCtlDebugStats_t,StormCtlDebugStats_dsIpeStormCtl0CountXSbeCnt_f},
    {DsIpeStormCtl0CountY_t,NULL,StormCtlDebugStats_t,StormCtlDebugStats_dsIpeStormCtl0CountYSbeCnt_f},
    {DsIpeStormCtl0ProfileX_t,NULL,StormCtlDebugStats_t,StormCtlDebugStats_dsIpeStormCtl0ProfileXSbeCnt_f},
    {DsIpeStormCtl0ProfileY_t,NULL,StormCtlDebugStats_t,StormCtlDebugStats_dsIpeStormCtl0ProfileYSbeCnt_f},
    {DsIpeStormCtl1Config_t,NULL,StormCtlDebugStats_t,StormCtlDebugStats_dsIpeStormCtl1ConfigSbeCnt_f},
    {DsIpeStormCtl1CountX_t,NULL,StormCtlDebugStats_t,StormCtlDebugStats_dsIpeStormCtl1CountXSbeCnt_f},
    {DsIpeStormCtl1CountY_t,NULL,StormCtlDebugStats_t,StormCtlDebugStats_dsIpeStormCtl1CountYSbeCnt_f},
    {DsIpeStormCtl1ProfileX_t,NULL,StormCtlDebugStats_t,StormCtlDebugStats_dsIpeStormCtl1ProfileXSbeCnt_f},
    {DsIpeStormCtl1ProfileY_t,NULL,StormCtlDebugStats_t,StormCtlDebugStats_dsIpeStormCtl1ProfileYSbeCnt_f},
    {DsIpv6NatPrefix_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsIpv6NatPrefixSbeCnt_f},
    {DsIrmPortCnt_t,NULL,BufStoreParityStatus_t,BufStoreParityStatus_dsIrmPortCntSbeCnt_f},
    {DsIrmPortTcCnt_t,NULL,BufStoreParityStatus_t,BufStoreParityStatus_dsIrmPortTcCntSbeCnt_f},
    {DsL2Edit6WOuter_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsL2Edit6WOuterSbeCnt_f},
    {DsL3Edit6W3rd_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsL3Edit6W3rdSbeCnt_f},
    {DsLatencyMonCnt_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsLatencyMonCntSbeCnt_f},
    {DsLatencyWatermark_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsLatencyWatermarkSbeCnt_f},
    {DsLinkAggregateChannelMember_t,NULL,LinkAggParityStatus_t,LinkAggParityStatus_dsLinkAggregateChannelMemberSbeCnt_f},
    {DsLinkAggregateMember_t,NULL,LinkAggParityStatus_t,LinkAggParityStatus_dsLinkAggregateMemberSbeCnt_f},
    {DsLinkAggregateMemberSet_t,NULL,LinkAggParityStatus_t,LinkAggParityStatus_dsLinkAggregateMemberSetSbeCnt_f},
    {DsLinkAggregationPortChannelMap_t,NULL,LinkAggParityStatus_t,LinkAggParityStatus_dsLinkAggregationPortChannelMapSbeCnt_f},
    {DsMacLimitCount_t,NULL,FibAccParityStatus_t,FibAccParityStatus_dsMacLimitCountSbeCnt_f},
    {DsMacLimitThreshold_t,NULL,FibAccParityStatus_t,FibAccParityStatus_dsMacLimitThresholdSbeCnt_f},
    {DsMetFifoExcp_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_dsMetFifoExcpSbeCnt_f},
    {DsMetLinkAggregateBitmap_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_dsMetLinkAggregateBitmapSbeCnt_f},
    {DsMetLinkAggregatePort_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_dsMetLinkAggregatePortSbeCnt_f},
    {DsMsgFreePtr_t,NULL,QMgrMsgStoreParityStatus_t,QMgrMsgStoreParityStatus_dsMsgFreePtrSbeCnt_f},
    {DsMsgUsedList_t,NULL,QMgrMsgStoreParityStatus_t,QMgrMsgStoreParityStatus_dsMsgUsedListSbeCnt_f},
    {DsPhyPort_t,NULL,IpeHdrAdjParityStatus_t,IpeHdrAdjParityStatus_dsPhyPortSbeCnt_f},
    {DsPhyPortExt_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsPhyPortExtSbeCnt_f},
    {DsPortBlockMask_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsPortBlockMaskSbeCnt_f},
    {DsPortIsolation_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsPortIsolationSbeCnt_f},
    {DsPortLinkAgg_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsPortLinkAggSbeCnt_f},
    {DsQcn_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsQcnSbeCnt_f},
    {DsQcnPortMac_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsQcnPortMacSbeCnt_f},
    {DsQMgrBaseSchMap_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrBaseSchMap1SbeCnt_f},
    {DsQMgrExtSchMap_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrExtSchMap1SbeCnt_f},
    {DsQueueMapTcamData_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsQueueMapTcamDataSbeCnt_f},
    {DsRouterMac_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsRouterMacSbeCnt_f},
    {DsRpf_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_dsRpfSbeCnt_f},
    {DsSrcChannel_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsSrcChannelSbeCnt_f},
    {DsSrcEthLmProfile_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_dsSrcEthLmProfileSbeCnt_f},
    {DsSrcInterface_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcInterfaceSbeCnt_f},
    {DsSrcInterfaceProfile_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcInterfaceProfileSbeCnt_f},
    {DsSrcPort_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcPortSbeCnt_f},
    {DsSrcStpState_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcStpStateSbeCnt_f},
    {DsSrcVlan_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcVlanSbeCnt_f},
    {DsSrcVlanProfile_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcVlanProfileSbeCnt_f},
    {DsSrcVlanStatus_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcVlanStatusSbeCnt_f},
    {DsStats_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsSbeCnt_f},
    {DsStatsEgressACL0_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsEgressACL0SbeCnt_f},
    {DsStatsEgressGlobal0_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsEgressGlobal0SbeCnt_f},
    {DsStatsEgressGlobal1_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsEgressGlobal1SbeCnt_f},
    {DsStatsEgressGlobal2_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsEgressGlobal2SbeCnt_f},
    {DsStatsEgressGlobal3_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsEgressGlobal3SbeCnt_f},
    {DsStatsIngressACL0_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressACL0SbeCnt_f},
    {DsStatsIngressACL1_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressACL1SbeCnt_f},
    {DsStatsIngressACL2_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressACL2SbeCnt_f},
    {DsStatsIngressACL3_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressACL3SbeCnt_f},
    {DsStatsIngressGlobal0_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressGlobal0SbeCnt_f},
    {DsStatsIngressGlobal1_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressGlobal1SbeCnt_f},
    {DsStatsIngressGlobal2_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressGlobal2SbeCnt_f},
    {DsStatsIngressGlobal3_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressGlobal3SbeCnt_f},
    {DsStatsQueue_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsQueueSbeCnt_f},
    {DsVlan2Ptr_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsVlan2PtrSbeCnt_f},
    {DsVlanActionProfile_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsVlanActionProfileSbeCnt_f},
    {DsVlanRangeProfile_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsVlanRangeProfileSbeCnt_f},
    {DsVlanTagBitMap_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsVlanTagBitMapSbeCnt_f},
    {DsVlanXlateDefault_t,NULL,EgrOamHashParityStatus_t,EgrOamHashParityStatus_dsVlanXlateDefaultSbeCnt_f},
    {DsVrf_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_dsVrfSbeCnt_f},
    {DsVsi_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_dsVsiSbeCnt_f},
    {EpeClaEopInfoFifo_t,NULL,EpeAclOamParityStatus_t,EpeAclOamParityStatus_epeClaEopInfoFifoSbeCnt_f},
    {EpeClaPolicingSopInfoFifo_t,NULL,EpeAclOamParityStatus_t,EpeAclOamParityStatus_epeClaPolicingSopInfoRamSbeCnt_f},
    {EpeHdrAdjDataFifo_t,NULL,EpeHdrAdjParityStatus_t,EpeHdrAdjParityStatus_epeHdrAdjDataFifoSbeCnt_f},
    {EpeHdrAdjEopMsgFifo_t,NULL,EpeHdrAdjParityStatus_t,EpeHdrAdjParityStatus_epeHdrAdjEopMsgFifoSbeCnt_f},
    {EpeHdrEditPktCtlFifo_t,NULL,EpeHdrEditParityStatus_t,EpeHdrEditParityStatus_epeHdrEditPktCtlFifoSbeCnt_f},
    {IpeFwdExcepGroupMap_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_ipeFwdExcepGroupMapSbeCnt_f},
    {IpePhbDscpMap_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_ipePhbDscpMapSbeCnt_f},
    {MaxTblId_t,"shareKeyRam10",DynamicKeyParityStatus_t,DynamicKeyParityStatus_shareKeyRam10SbeCnt_f},
    {MaxTblId_t,"shareKeyRam9",DynamicKeyParityStatus_t,DynamicKeyParityStatus_shareKeyRam9SbeCnt_f},
    {MaxTblId_t,"shareKeyRam8",DynamicKeyParityStatus_t,DynamicKeyParityStatus_shareKeyRam8SbeCnt_f},
    {MaxTblId_t,"shareKeyRam7",DynamicKeyParityStatus_t,DynamicKeyParityStatus_shareKeyRam7SbeCnt_f},
    {MaxTblId_t,"shareKeyRam6",DynamicKeyParityStatus_t,DynamicKeyParityStatus_shareKeyRam6SbeCnt_f},
    {MaxTblId_t,"shareKeyRam5",DynamicKeyParityStatus_t,DynamicKeyParityStatus_shareKeyRam5SbeCnt_f},
    {MaxTblId_t,"shareKeyRam4",DynamicKeyParityStatus_t,DynamicKeyParityStatus_shareKeyRam4SbeCnt_f},
    {MaxTblId_t,"shareKeyRam3",DynamicKeyParityStatus_t,DynamicKeyParityStatus_shareKeyRam3SbeCnt_f},
    {MaxTblId_t,"shareKeyRam2",DynamicKeyParityStatus_t,DynamicKeyParityStatus_shareKeyRam2SbeCnt_f},
    {MaxTblId_t,"shareKeyRam1",DynamicKeyParityStatus_t,DynamicKeyParityStatus_shareKeyRam1SbeCnt_f},
    {MaxTblId_t,"shareKeyRam0",DynamicKeyParityStatus_t,DynamicKeyParityStatus_shareKeyRam0SbeCnt_f},
    {MaxTblId_t,"lpmTcamAd5",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd5SbeCnt_f},
    {MaxTblId_t,"lpmTcamAd4",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd4SbeCnt_f},
    {MaxTblId_t,"lpmTcamAd3",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd3SbeCnt_f},
    {MaxTblId_t,"lpmTcamAd2",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd2SbeCnt_f},
    {MaxTblId_t,"lpmTcamAd1",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd1SbeCnt_f},
    {MaxTblId_t,"lpmTcamAd0",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd0SbeCnt_f},
    {MaxTblId_t,"shareAdRam4",DynamicAdParityStatus_t,DynamicAdParityStatus_shareAdRam4SbeCnt_f},
    {MaxTblId_t,"shareAdRam3",DynamicAdParityStatus_t,DynamicAdParityStatus_shareAdRam3SbeCnt_f},
    {MaxTblId_t,"shareAdRam2",DynamicAdParityStatus_t,DynamicAdParityStatus_shareAdRam2SbeCnt_f},
    {MaxTblId_t,"shareAdRam1",DynamicAdParityStatus_t,DynamicAdParityStatus_shareAdRam1SbeCnt_f},
    {MaxTblId_t,"shareAdRam0",DynamicAdParityStatus_t,DynamicAdParityStatus_shareAdRam0SbeCnt_f},
    {MaxTblId_t,"shareEditRam5",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam5SbeCnt_f},
    {MaxTblId_t,"shareEditRam4",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam4SbeCnt_f},
    {MaxTblId_t,"shareEditRam3",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam3SbeCnt_f},
    {MaxTblId_t,"shareEditRam2",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam2SbeCnt_f},
    {MaxTblId_t,"shareEditRam1",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam1SbeCnt_f},
    {MaxTblId_t,"shareEditRam0",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam0SbeCnt_f},
    {MaxTblId_t,"flowTcamShareAd2",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamShareAd2SbeCnt_f},
    {MaxTblId_t,"flowTcamShareAd1",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamShareAd1SbeCnt_f},
    {MaxTblId_t,"flowTcamShareAd0",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamShareAd0SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeAclAd7",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeAclAd7SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeAclAd6",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeAclAd6SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeAclAd5",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeAclAd5SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeAclAd4",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeAclAd4SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeAclAd3",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeAclAd3SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeAclAd2",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeAclAd2SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeAclAd1",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeAclAd1SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeAclAd0",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeAclAd0SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeUserIdAd1",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeUserIdAd1SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeUserIdAd0",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeUserIdAd0SbeCnt_f},
    {MaxTblId_t,"flowTcamEpeAclAd2",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamEpeAclAd2SbeCnt_f},
    {MaxTblId_t,"flowTcamEpeAclAd1",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamEpeAclAd1SbeCnt_f},
    {MaxTblId_t,"flowTcamEpeAclAd0",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamEpeAclAd0SbeCnt_f},
     /*{McuSupDataMem0_t,NULL,McuSupStats0_t,McuSupStats0_mcuSupDataMemSbeCnt_f},*/
     /*{McuSupDataMem0_t,NULL,McuSupStats1_t,McuSupStats1_mcuSupDataMemSbeCnt_f},*/
    {McuSupProgMem1_t,NULL,McuSupStats0_t,McuSupStats0_mcuSupProgMemSbeCnt_f},
    {McuSupProgMem1_t,NULL,McuSupStats1_t,McuSupStats1_mcuSupProgMemSbeCnt_f},
    {MetBrRcdFifo_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metBrRcdFifoSbeCnt_f},
    {MetEnqRcdFifo_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metEnqRcdFifoSbeCnt_f},
    {MetMcastTrackFifo_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metMcastTrackFifoSbeCnt_f},
    {MetMsgMem_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metMsgMemSbeCnt_f},
    {MetRcdMem_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metRcdMemSbeCnt_f},
    {MetUcastHiRcdFifo_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metUcastHiRcdFifoSbeCnt_f},
    {NetTxDsIbLppInfo_t,NULL,NetTxParityStatus_t,NetTxParityStatus_netTxDsIbLppInfoSbeCnt_f},
    {NetTxPktHdr0_t,NULL,NetTxParityStatus_t,NetTxParityStatus_netTxPktHdr0SbeCnt_f},
    {NetTxPktHdr1_t,NULL,NetTxParityStatus_t,NetTxParityStatus_netTxPktHdr1SbeCnt_f},
    {NetTxPktHdr2_t,NULL,NetTxParityStatus_t,NetTxParityStatus_netTxPktHdr2SbeCnt_f},
    {NetTxPktHdr3_t,NULL,NetTxParityStatus_t,NetTxParityStatus_netTxPktHdr3SbeCnt_f},
    {OamHashAclInputFifo_t,NULL,EgrOamHashParityStatus_t,EgrOamHashParityStatus_oamHashAclInputFifoSbeCnt_f},
    {OamHashLkupMgrInputFifo_t,NULL,EgrOamHashParityStatus_t,EgrOamHashParityStatus_oamHashLkupMgrInputFifoSbeCnt_f},
    {OamParserPktFifo_t,NULL,OamParserParityStatus_t,OamParserParityStatus_oamParserPktFifoSbeCnt_f},
    {MaxTblId_t,NULL,MaxTblId_t,0}
};


uint16 drv_ecc_dt2_scan_tcam_tbl[][5] =
{
    {DRV_FTM_TCAM_KEY0,DsAclQosMacKey160Egr0_t,DsAclQosMacKey160Egr1_t,DsAclQosMacKey160Egr2_t,MaxTblId_t},
    {DRV_FTM_TCAM_KEY0,DsAclQosMacKey160Ing0_t,DsAclQosMacKey160Ing1_t,DsAclQosMacKey160Ing2_t,DsAclQosMacKey160Ing3_t},
    {DRV_FTM_TCAM_KEY0,DsAclQosMacKey160Ing4_t,DsAclQosMacKey160Ing5_t,DsAclQosMacKey160Ing6_t,DsAclQosMacKey160Ing7_t},
    {DRV_FTM_TCAM_KEY0,DsUserId0TcamKey160_t,DsUserId1TcamKey160_t,MaxTblId_t,MaxTblId_t},
    {DRV_FTM_LPM_TCAM_KEY0,DsLpmTcamIpv4HalfKey_t,MaxTblId_t ,MaxTblId_t,MaxTblId_t},
    {DRV_FTM_CID_TCAM,DsCategoryIdPairTcamKey_t,MaxTblId_t,MaxTblId_t,MaxTblId_t},
    {DRV_FTM_QUEUE_TCAM,DsQueueMapTcamKey_t,MaxTblId_t,MaxTblId_t,MaxTblId_t},
    {MaxTblId_t,MaxTblId_t,MaxTblId_t,MaxTblId_t,MaxTblId_t}
};


#define DRV_MEM_REG(name, id, num, addr3w, addr6w, addr12w) \
    do{\
        drv_dt2_mem[id].entry_num = num;\
        drv_dt2_mem[id].addr_3w = addr3w;\
        drv_dt2_mem[id].addr_6w = addr6w;\
        drv_dt2_mem[id].addr_12w = addr12w;\
     }while(0);\

int32
drv_mem_init_dt2(uint8 lchip)
{
    DRV_MEM_REG("KeyRam0"             , DRV_FTM_SRAM0, 16384, 0x6000000, 0x6800000, 0x7000000);
    DRV_MEM_REG("KeyRam1"             , DRV_FTM_SRAM1, 16384, 0x6080000, 0x6880000, 0x7080000);
    DRV_MEM_REG("KeyRam2"             , DRV_FTM_SRAM2, 16384, 0x6100000, 0x6900000, 0x7100000);
    DRV_MEM_REG("KeyRam3"             , DRV_FTM_SRAM3, 8192, 0x6180000, 0x6980000, 0x7180000);
    DRV_MEM_REG("KeyRam4"             , DRV_FTM_SRAM4, 8192, 0x6200000, 0x6a00000, 0x7200000);
    DRV_MEM_REG("KeyRam5"             , DRV_FTM_SRAM5, 8192, 0x6280000, 0x6a80000, 0x7280000);
    DRV_MEM_REG("KeyRam6"             , DRV_FTM_SRAM6, 8192, 0x6300000, 0x6b00000, 0x7300000);
    DRV_MEM_REG("KeyRam7"             , DRV_FTM_SRAM7, 8192, 0x6380000, 0x6b80000, 0x7380000);
    DRV_MEM_REG("KeyRam8"             , DRV_FTM_SRAM8, 4096, 0x6400000, 0x6c00000, 0x7400000);
    DRV_MEM_REG("KeyRam9"             , DRV_FTM_SRAM9, 4096, 0x6480000, 0x6c80000, 0x7480000);
    DRV_MEM_REG("KeyRam10"            , DRV_FTM_SRAM10, 2048, 0x6500000, 0x6d00000, 0x7500000);
    DRV_MEM_REG("AdRam0"              , DRV_FTM_SRAM11, 16384, 0x3800000, 0x3400000, 0x3000000);
    DRV_MEM_REG("AdRam1"              , DRV_FTM_SRAM12, 8192, 0x3880000, 0x3480000, 0x3080000);
    DRV_MEM_REG("AdRam2"              , DRV_FTM_SRAM13, 8192, 0x3900000, 0x3500000, 0x3100000);
    DRV_MEM_REG("AdRam3"              , DRV_FTM_SRAM14, 8192, 0x3980000, 0x3580000, 0x3180000);
    DRV_MEM_REG("AdRam4"              , DRV_FTM_SRAM15, 8192, 0x3a00000, 0x3600000, 0x3200000);
    DRV_MEM_REG("EditRam0"            , DRV_FTM_SRAM16, 8192, 0x1a00000, 0x1c00000, 0x1800000);
    DRV_MEM_REG("EditRam1"            , DRV_FTM_SRAM17, 8192, 0x1a40000, 0x1c40000, 0x1840000);
    DRV_MEM_REG("EditRam2"            , DRV_FTM_SRAM18, 8192, 0x1a80000, 0x1c80000, 0x1880000);
    DRV_MEM_REG("EditRam3"            , DRV_FTM_SRAM19, 8192, 0x1ac0000, 0x1cc0000, 0x18c0000);
    DRV_MEM_REG("EditRam4"            , DRV_FTM_SRAM20, 4096, 0x1b00000, 0x1d00000, 0x1900000);
    DRV_MEM_REG("EditRam5"            , DRV_FTM_SRAM21, 4096, 0x1b40000, 0x1d40000, 0x1940000);


    DRV_MEM_REG("Tcam key0"           , DRV_FTM_TCAM_KEY0, 1024, 0x1200000, 0xa0400000, 0x0);
    DRV_MEM_REG("Tcam key1"           , DRV_FTM_TCAM_KEY1, 1024, 0x1210000, 0xa0408000, 0x0);
    DRV_MEM_REG("Tcam key2"           , DRV_FTM_TCAM_KEY2, 1024, 0x1220000, 0xa0410000, 0x0);
    DRV_MEM_REG("Tcam key3"           , DRV_FTM_TCAM_KEY3, 512, 0x1230000, 0xa0418000, 0x0);
    DRV_MEM_REG("Tcam key4"           , DRV_FTM_TCAM_KEY4, 512, 0x1240000, 0xa0420000, 0x0);
    DRV_MEM_REG("Tcam key5"           , DRV_FTM_TCAM_KEY5, 512, 0x1250000, 0xa0428000, 0x0);
    DRV_MEM_REG("Tcam key6"           , DRV_FTM_TCAM_KEY6, 512, 0x1260000, 0xa0430000, 0x0);
    DRV_MEM_REG("Tcam key7"           , DRV_FTM_TCAM_KEY7, 512, 0x1270000, 0xa0438000, 0x0);
    DRV_MEM_REG("Tcam key8"           , DRV_FTM_TCAM_KEY8, 512, 0x1280000, 0xa0440000, 0x0);
    DRV_MEM_REG("Tcam key9"           , DRV_FTM_TCAM_KEY9, 512, 0x1290000, 0xa0448000, 0x0);
    DRV_MEM_REG("Tcam key10"          , DRV_FTM_TCAM_KEY10, 1024, 0x12a0000, 0xa0450000, 0x0);
    DRV_MEM_REG("Tcam key11"          , DRV_FTM_TCAM_KEY11, 1024, 0x12b0000, 0xa0458000, 0x0);
    DRV_MEM_REG("Tcam key12"          , DRV_FTM_TCAM_KEY12, 1024, 0x12c0000, 0xa0460000, 0x0);
    DRV_MEM_REG("Tcam key13"          , DRV_FTM_TCAM_KEY13, 2048, 0x12d0000, 0xa0468000, 0x0);
    DRV_MEM_REG("Tcam key14"          , DRV_FTM_TCAM_KEY14, 2048, 0x12e0000, 0xa0470000, 0x0);
    DRV_MEM_REG("Tcam key15"          , DRV_FTM_TCAM_KEY15, 2048, 0x12f0000, 0xa0478000, 0x0);

    DRV_MEM_REG("Tcam AD0"            , DRV_FTM_TCAM_AD0, 1024, 0x1300000, 0x1300000, 0x1300000);
    DRV_MEM_REG("Tcam AD1"            , DRV_FTM_TCAM_AD1, 1024, 0x1308000, 0x1308000, 0x1308000);
    DRV_MEM_REG("Tcam AD2"            , DRV_FTM_TCAM_AD2, 1024, 0x1310000, 0x1310000, 0x1310000);
    DRV_MEM_REG("Tcam AD3"            , DRV_FTM_TCAM_AD3, 512, 0x1318000, 0x1318000, 0x1318000);
    DRV_MEM_REG("Tcam AD4"            , DRV_FTM_TCAM_AD4, 512, 0x1320000, 0x1320000, 0x1320000);
    DRV_MEM_REG("Tcam AD5"            , DRV_FTM_TCAM_AD5, 512, 0x1328000, 0x1328000, 0x1328000);
    DRV_MEM_REG("Tcam AD6"            , DRV_FTM_TCAM_AD6, 512, 0x1330000, 0x1330000, 0x1330000);
    DRV_MEM_REG("Tcam AD7"            , DRV_FTM_TCAM_AD7, 512, 0x1338000, 0x1338000, 0x1338000);
    DRV_MEM_REG("Tcam AD8"            , DRV_FTM_TCAM_AD8, 512, 0x1340000, 0x1340000, 0x1340000);
    DRV_MEM_REG("Tcam AD9"            , DRV_FTM_TCAM_AD9, 512, 0x1348000, 0x1348000, 0x1348000);
    DRV_MEM_REG("Tcam AD10"           , DRV_FTM_TCAM_AD10, 1024, 0x1350000, 0x1350000, 0x1350000);
    DRV_MEM_REG("Tcam AD11"           , DRV_FTM_TCAM_AD11, 1024, 0x1358000, 0x1358000, 0x1358000);
    DRV_MEM_REG("Tcam AD12"           , DRV_FTM_TCAM_AD12, 1024, 0x1360000, 0x1360000, 0x1360000);
    DRV_MEM_REG("Tcam AD13"           , DRV_FTM_TCAM_AD13, 2048, 0x1368000, 0x1368000, 0x1368000);
    DRV_MEM_REG("Tcam AD14"           , DRV_FTM_TCAM_AD14, 2048, 0x1370000, 0x1370000, 0x1370000);
    DRV_MEM_REG("Tcam AD15"           , DRV_FTM_TCAM_AD15, 2048, 0x1378000, 0x1378000, 0x1378000);

    DRV_MEM_REG("LPM Tcam key0"       , DRV_FTM_LPM_TCAM_KEY0, 512, 0x2100000, 0xa2800000, 0x0);
    DRV_MEM_REG("LPM Tcam key1"       , DRV_FTM_LPM_TCAM_KEY1, 512, 0x2102000, 0xa2801000, 0x0);
    DRV_MEM_REG("LPM Tcam key2"       , DRV_FTM_LPM_TCAM_KEY2, 512, 0x2104000, 0xa2802000, 0x0);
    DRV_MEM_REG("LPM Tcam key3"       , DRV_FTM_LPM_TCAM_KEY3, 512, 0x2106000, 0xa2803000, 0x0);
    DRV_MEM_REG("LPM Tcam key4"       , DRV_FTM_LPM_TCAM_KEY4, 512, 0x2110000, 0xa2808000, 0x0);
    DRV_MEM_REG("LPM Tcam key5"       , DRV_FTM_LPM_TCAM_KEY5, 512, 0x2112000, 0xa2809000, 0x0);
    DRV_MEM_REG("LPM Tcam key6"       , DRV_FTM_LPM_TCAM_KEY6, 512, 0x2114000, 0xa280a000, 0x0);
    DRV_MEM_REG("LPM Tcam key7"       , DRV_FTM_LPM_TCAM_KEY7, 512, 0x2116000, 0xa280b000, 0x0);
    DRV_MEM_REG("LPM Tcam key8"       , DRV_FTM_LPM_TCAM_KEY8, 1024, 0x2120000, 0xa2810000, 0x0);
    DRV_MEM_REG("LPM Tcam key9"       , DRV_FTM_LPM_TCAM_KEY9, 1024, 0x2124000, 0xa2812000, 0x0);
    DRV_MEM_REG("LPM Tcam key10"      , DRV_FTM_LPM_TCAM_KEY10, 1024, 0x2128000, 0xa2814000, 0x0);
    DRV_MEM_REG("LPM Tcam key11"      , DRV_FTM_LPM_TCAM_KEY11, 1024, 0x212c000, 0xa2816000, 0x0);

    DRV_MEM_REG("LPM Tcam Ad0"        , DRV_FTM_LPM_TCAM_AD0, 512, 0x2140000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad1"        , DRV_FTM_LPM_TCAM_AD1, 512, 0x2141000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad2"        , DRV_FTM_LPM_TCAM_AD2, 512, 0x2142000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad3"        , DRV_FTM_LPM_TCAM_AD3, 512, 0x2143000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad4"        , DRV_FTM_LPM_TCAM_AD4, 512, 0x2148000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad5"        , DRV_FTM_LPM_TCAM_AD5, 512, 0x2149000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad6"        , DRV_FTM_LPM_TCAM_AD6, 512, 0x214a000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad7"        , DRV_FTM_LPM_TCAM_AD7, 512, 0x214b000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad8"        , DRV_FTM_LPM_TCAM_AD8, 1024, 0x2150000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad9"        , DRV_FTM_LPM_TCAM_AD9, 1024, 0x2152000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad10"       , DRV_FTM_LPM_TCAM_AD10, 1024, 0x2154000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad11"       , DRV_FTM_LPM_TCAM_AD11, 1024, 0x2156000, 0x0, 0x0);

    DRV_MEM_REG("Queue Tcam"         , DRV_FTM_QUEUE_TCAM, 256, 0x00640000, 0xb1000000, 0x0);
    DRV_MEM_REG("Cid Tcam"           , DRV_FTM_CID_TCAM, 128, 0x0093f400, 0xb0000000, 0x0);

    return 0;
}





/*Diff between Tsingma and D2*/
int32
sys_duet2_ftm_get_edram_bitmap(uint8 lchip,
                                     uint8 sram_type,
                                     uint32* bit)
{
    switch (sram_type)
    {
        /* SelEdram  {edram0, edram3, edram6, edram7} Done */
        case DRV_FTM_SRAM_TBL_LPM_LKP_KEY0:
        bit[DRV_FTM_SRAM0] = 0;
        bit[DRV_FTM_SRAM3] = 1;
        bit[DRV_FTM_SRAM6] = 2;
        bit[DRV_FTM_SRAM7] = 3;
        break;

        /* SelEdram  {edram0, edram3, edram6, edram7} Done */
        case DRV_FTM_SRAM_TBL_LPM_LKP_KEY1:
        bit[DRV_FTM_SRAM0] = 0;
        bit[DRV_FTM_SRAM3] = 1;
        bit[DRV_FTM_SRAM6] = 2;
        bit[DRV_FTM_SRAM7] = 3;
        break;

        /* SelEdram  {edram0, edram1, edram2, edram3, edram4, edram5 } Done*/
        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:
        bit[DRV_FTM_SRAM0] = 0;
        bit[DRV_FTM_SRAM1] = 1;
        bit[DRV_FTM_SRAM2] = 2;
        bit[DRV_FTM_SRAM3] = 3;
        bit[DRV_FTM_SRAM4] = 4;
        bit[DRV_FTM_SRAM5] = 5;
        break;

        /* SelEdram  {edram11, edram12, edram13, edram14, edram15} Done*/
    case DRV_FTM_SRAM_TBL_FLOW_AD:
        bit[DRV_FTM_SRAM11]   = 0;
        bit[DRV_FTM_SRAM12]   = 1;
        bit[DRV_FTM_SRAM13]   = 2;
        bit[DRV_FTM_SRAM14]   = 3;
        bit[DRV_FTM_SRAM15]   = 4;
        break;

        /* SelEdram  {edram0, edram5} Done*/
        case DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY:
        bit[DRV_FTM_SRAM0] = 0;
        bit[DRV_FTM_SRAM5] = 1;
        break;

        /* SelEdram  {edram11, edram12} Done*/
    case DRV_FTM_SRAM_TBL_IPFIX_AD:
        bit[DRV_FTM_SRAM11] = 0;
        bit[DRV_FTM_SRAM12] = 1;
        break;

        /* SelEdram  {edram11, edram12, edram13, edram14, edram15} Done*/
        case DRV_FTM_SRAM_TBL_DSMAC_AD:
        bit[DRV_FTM_SRAM11]   = 0;
        bit[DRV_FTM_SRAM12]   = 1;
        bit[DRV_FTM_SRAM13]   = 2;
        bit[DRV_FTM_SRAM14]   = 3;
        bit[DRV_FTM_SRAM15]   = 4;
        break;

        /* SelEdram  {edram11, edram12, edram13, edram14, edram15} Done*/
        case DRV_FTM_SRAM_TBL_DSIP_AD:
        bit[DRV_FTM_SRAM11]   = 0;
        bit[DRV_FTM_SRAM12]   = 1;
        bit[DRV_FTM_SRAM13]   = 2;
        bit[DRV_FTM_SRAM14]   = 3;
        bit[DRV_FTM_SRAM15]   = 4;
        break;

        /* SelEdram  {edram18, edram19, edram20, edram21 } Done*/
    case DRV_FTM_SRAM_TBL_EDIT:
        bit[DRV_FTM_SRAM18] = 0;
        bit[DRV_FTM_SRAM19] = 1;
        bit[DRV_FTM_SRAM20] = 2;
        bit[DRV_FTM_SRAM21] = 3;
        break;

        /* SelEdram  {edram16, edram18, edram19, edram20, edram21 } Done*/
        case DRV_FTM_SRAM_TBL_NEXTHOP:
        bit[DRV_FTM_SRAM16] = 0;
        bit[DRV_FTM_SRAM18] = 1;
        bit[DRV_FTM_SRAM19] = 2;
        bit[DRV_FTM_SRAM20] = 3;
        bit[DRV_FTM_SRAM21] = 4;
        break;

        /* SelEdram  {edram16, edram18, edram19, edram20, edram21 } Done*/
        case DRV_FTM_SRAM_TBL_MET:
        bit[DRV_FTM_SRAM16] = 0;
        bit[DRV_FTM_SRAM18] = 1;
        bit[DRV_FTM_SRAM19] = 2;
        bit[DRV_FTM_SRAM20] = 3;
        bit[DRV_FTM_SRAM21] = 4;
        break;

        /* SelEdram  {edram17, edram18, edram19, edram20, edram21 } Done*/
        case DRV_FTM_SRAM_TBL_FWD:
        bit[DRV_FTM_SRAM17] = 0;
        bit[DRV_FTM_SRAM18] = 1;
        bit[DRV_FTM_SRAM19] = 2;
        bit[DRV_FTM_SRAM20] = 3;
        bit[DRV_FTM_SRAM21] = 4;
        break;


        /* SelEdram  {edram11, edram12, edram13, edram14, edram15} Done*/
        case DRV_FTM_SRAM_TBL_USERID_AD:
        bit[DRV_FTM_SRAM11]   = 0;
        bit[DRV_FTM_SRAM12]   = 1;
        bit[DRV_FTM_SRAM13]   = 2;
        bit[DRV_FTM_SRAM14]   = 3;
        bit[DRV_FTM_SRAM15]   = 4;
        break;

         /* SelEdram  {edram1, edram4} Done*/
        case DRV_FTM_SRAM_TBL_OAM_LM:
        bit[DRV_FTM_SRAM1] = 0;
        bit[DRV_FTM_SRAM4] = 1;
        break;

         /* SelEdram  {edram1, edram4} Done*/
        case DRV_FTM_SRAM_TBL_OAM_MEP:
        bit[DRV_FTM_SRAM1] = 0;
        bit[DRV_FTM_SRAM4] = 1;
        break;

         /* SelEdram  {edram1, edram4} Done*/
        case DRV_FTM_SRAM_TBL_OAM_MA:
        bit[DRV_FTM_SRAM1] = 0;
        bit[DRV_FTM_SRAM4] = 1;
        break;

         /* SelEdram  {edram1, edram4} Done*/
        case DRV_FTM_SRAM_TBL_OAM_MA_NAME:
        bit[DRV_FTM_SRAM1] = 0;
        bit[DRV_FTM_SRAM4] = 1;
        break;


        /* NONE */
        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
        bit[DRV_FTM_SRAM6] = 0;
        bit[DRV_FTM_SRAM7] = 1;
        bit[DRV_FTM_SRAM8] = 2;
        bit[DRV_FTM_SRAM9] = 3;
        bit[DRV_FTM_SRAM10] = 4;
        break;

        /* NONE */
    case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
        bit[DRV_FTM_SRAM0] = 0;
        bit[DRV_FTM_SRAM1] = 1;
        bit[DRV_FTM_SRAM2] = 2;
        bit[DRV_FTM_SRAM3] = 3;
        bit[DRV_FTM_SRAM4] = 4;
        break;

         /* NONE */
        case DRV_FTM_SRAM_TBL_OAM_APS:
        bit[DRV_FTM_SRAM10] = 0;
        break;

        /* NONE */
        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
        bit[DRV_FTM_SRAM2] = 0;
        bit[DRV_FTM_SRAM5] = 1;
        break;

        /* NONE */
        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
        bit[DRV_FTM_SRAM6] = 0;
        bit[DRV_FTM_SRAM7] = 1;
        bit[DRV_FTM_SRAM8] = 2;
        bit[DRV_FTM_SRAM9] = 3;
        bit[DRV_FTM_SRAM10] = 4;
        break;

        default:
           break;
    }

    return DRV_E_NONE;
}



/*Diff between Tsingma and D2*/
int32
sys_duet2_ftm_get_hash_type(uint8 lchip,
                            uint8 sram_type,
                            uint32 mem_id,
                            uint8 couple,
                            uint32 *poly)
{
    uint8 extra = 0;

    *poly = 0;

    switch (sram_type)
    {
        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:
        /*Host0lkup ctl
         {HASH_CRC, 0x00000005, 11},
         {HASH_CRC, 0x00000017, 11},
         {HASH_CRC, 0x0000002b, 11},
         {HASH_CRC, 0x00000053, 12},
         {HASH_CRC, 0x00000099, 12},
         {HASH_CRC, 0x00000107, 12},
         {HASH_CRC, 0x0000001b, 13},
         {HASH_CRC, 0x00000027, 13},
         {HASH_CRC, 0x000000c3, 13},
         {HASH_XOR, 16        , 0},

         mem   {RAM0,RAM1,RAM2,RAM3,RAM4}
         size  :32k  32k  32k  32k  8k
         couple:64k  64k  64k  64k  16k
         DRV_FTM_SRAM0  32K/4=8K  2, 6
         DRV_FTM_SRAM1  32K/4=8K  3, 7
         DRV_FTM_SRAM2  32K/4=8K  4, 8
         DRV_FTM_SRAM3  32K/4=8K  5, 9
         DRV_FTM_SRAM4  08K/4=2K  0, 1
        */
            switch(mem_id)
            {
            case DRV_FTM_SRAM0:
                *poly = couple ? 6: 3;
                break;
            case DRV_FTM_SRAM1:
                *poly = couple ? 6: 3;
                break;
            case DRV_FTM_SRAM2:
                *poly = couple ? 6: 3;
                break;
            case DRV_FTM_SRAM3:
                *poly = couple ? 3: 0;
                break;
            case DRV_FTM_SRAM4:
                *poly = couple ? 3: 0;
                break;
            case DRV_FTM_SRAM5:
                *poly = couple ? 3: 0;
                break;

            }

            break;

        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
        /*Host1lkup ctl
         {HASH_CRC, 0x00000011, 9},
         {HASH_CRC, 0x0000001b, 10},
         {HASH_CRC, 0x00000027, 10},
         {HASH_CRC, 0x00000005, 11},
         {HASH_CRC, 0x00000017, 11},
         {HASH_CRC, 0x0000002b, 11},
         {HASH_CRC, 0x00000053, 12},
         {HASH_CRC, 0x00000099, 12},
         {HASH_CRC, 0x00000107, 12},
         {HASH_XOR, 16        , 0},

         mem   {RAM0,RAM5,RAM6}
         size  :32k  16k  16k
         couple:64k  32k  32k
         DRV_FTM_SRAM0  32K/4=8K  2, 4
         DRV_FTM_SRAM5  16K/4=4K  0, 2
         DRV_FTM_SRAM6  16K/4=4K  1, 3
         */
            switch(mem_id)
            {
            case DRV_FTM_SRAM6:
                *poly = couple ? 6: 3;
                break;
            case DRV_FTM_SRAM7:
                *poly = couple ? 6: 3;
                break;
            case DRV_FTM_SRAM8:
                *poly = couple ? 3: 1;
                break;
            case DRV_FTM_SRAM9:
                *poly = couple ? 3: 1;
                break;
            case DRV_FTM_SRAM10:
                *poly = couple ? 1: 0;
                break;
            }
            break;

        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
        /*Flow lkup ctl
         {HASH_CRC, 0x00000005, 11},
         {HASH_CRC, 0x00000017, 11},
         {HASH_CRC, 0x0000002b, 11},
         {HASH_CRC, 0x00000053, 12},
         {HASH_CRC, 0x00000099, 12},
         {HASH_CRC, 0x00000107, 12},
         {HASH_CRC, 0x0000001b, 13},
         {HASH_CRC, 0x00000027, 13},
         {HASH_CRC, 0x000000c3, 13},
         {HASH_XOR, 16        , 0},

         mem   {RAM0,RAM1,RAM2,RAM3,RAM4}
         size  :32k  32k  32k  32k  8k
         couple:64k  64k  64k  64k  16k

         DRV_FTM_SRAM0  32K/4=8K  2, 6
         DRV_FTM_SRAM1  32K/4=8K  3, 7
         DRV_FTM_SRAM2  32K/4=8K  4, 8
         DRV_FTM_SRAM3  32K/4=8K  5, 9
         DRV_FTM_SRAM4  08K/4=2K  0, 1
        */
            switch(mem_id)
            {
            case DRV_FTM_SRAM0:
                *poly = couple ? 6: 3;
                break;
            case DRV_FTM_SRAM1:
                *poly = couple ? 6: 3;
                break;
            case DRV_FTM_SRAM2:
                *poly = couple ? 6: 3;
                break;
            case DRV_FTM_SRAM3:
                *poly = couple ? 3: 0;
                break;
            case DRV_FTM_SRAM4:
                *poly = couple ? 3: 0;
                break;
            case DRV_FTM_SRAM5:
                *poly = couple ? 3: 0;
                break;
            }
            break;

        case DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY:
        /*
         {HASH_CRC, 0x0000001b, 10},
         {HASH_CRC, 0x00000027, 10},
         {HASH_CRC, 0x00000005, 11},
         {HASH_CRC, 0x00000017, 11},
         {HASH_CRC, 0x00000053, 12},
         {HASH_CRC, 0x00000107, 12},
     */
            switch(mem_id)
            {
            case DRV_FTM_SRAM0:
                *poly = couple ? 4: 2;
                break;
            case DRV_FTM_SRAM5:
                *poly = couple ? 2: 0;
                break;

            }
            break;

        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
        /*userid  lkup ctl
         {HASH_CRC, 0x00000011, 9},
         {HASH_CRC, 0x0000001b, 10},
         {HASH_CRC, 0x00000027, 10},
         {HASH_CRC, 0x00000005, 11},
         {HASH_CRC, 0x00000017, 11},
         {HASH_CRC, 0x0000002b, 11},
         {HASH_CRC, 0x00000053, 12},
         {HASH_CRC, 0x00000099, 12},
         {HASH_CRC, 0x00000107, 12},
         {HASH_XOR, 16        , 0},

         mem   {RAM2,RAM11,RAM3,RAM12}
         size  :32k  8k    32k  8k
         couple:64k  16k   64k  16k

         DRV_FTM_SRAM2   32K/4=8K  4, 6
         DRV_FTM_SRAM3   32K/4=8K  5, 7
         DRV_FTM_SRAM11  08K/4=2K  0, 2
         DRV_FTM_SRAM12  08K/4=2K  1, 3
        */
            switch(mem_id)
            {
            case DRV_FTM_SRAM6:
                *poly = couple ? 6: 3;
                break;
            case DRV_FTM_SRAM7:
                *poly = couple ? 6: 3;
                break;
            case DRV_FTM_SRAM8:
                *poly = couple ? 3: 1;
                break;
            case DRV_FTM_SRAM9:
                *poly = couple ? 3: 1;
                break;
            case DRV_FTM_SRAM10:
                *poly = couple ? 1: 0;
                break;
            }
            break;

        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
        /*XcOamUserid  lkup ctl
         {HASH_CRC, 0x00000005, 11},
         {HASH_CRC, 0x00000053, 12},
         {HASH_CRC, 0x0000001b, 13},
         {HASH_XOR, 16        , 0},

         mem   {RAM17,RAM7,RAM8, RAM2}
         size  :32k  8k    16k   16K
         couple:64k  16k   32k   32k

         DRV_FTM_SRAM17   32K/4=8K  1, 3
         DRV_FTM_SRAM7    32K/4=8K  2, 4
         DRV_FTM_SRAM8    16K/4=4K  0, 1
        */

            switch(mem_id)
            {
            case DRV_FTM_SRAM2:
                *poly = couple ?(extra?3:2): (extra?1:1);
                break;
            case DRV_FTM_SRAM5:
                *poly = couple ? (extra?4:1): (extra?2:0);
                break;
            }
            break;

    }
    return DRV_E_NONE;
}

int32 drv_duet2_get_mcu_lock_id(uint8 lchip, tbls_id_t tbl_id, uint8* p_mcu_id, uint32* p_lock_id)
{
    uint8  mcu_id  = 0xff;
    uint32 lock_id = DRV_MCU_LOCK_NONE;

    if((NULL == p_lock_id) || (NULL == p_mcu_id))
    {
        return DRV_E_INVALID_PTR;
    }

    if ((NetTxWriteEn_t == tbl_id)
        || ((tbl_id >= Sgmac0TxCfg0_t) && (tbl_id <= Sgmac0TxCfg15_t))
        || ((tbl_id >= Sgmac1TxCfg0_t) && (tbl_id <= Sgmac1TxCfg15_t))
        || ((tbl_id >= Sgmac2TxCfg0_t) && (tbl_id <= Sgmac2TxCfg15_t))
        || ((tbl_id >= Sgmac3TxCfg0_t) && (tbl_id <= Sgmac3TxCfg15_t))
        || ((tbl_id >= SharedMii0Cfg0_t) && (tbl_id <= SharedMii0Cfg15_t))
        || ((tbl_id >= SharedMii1Cfg0_t) && (tbl_id <= SharedMii1Cfg15_t))
        || ((tbl_id >= SharedMii2Cfg0_t) && (tbl_id <= SharedMii2Cfg15_t))
        || ((tbl_id >= SharedMii3Cfg0_t) && (tbl_id <= SharedMii3Cfg15_t)))
    {
        lock_id = DRV_MCU_LOCK_WA_CFG;
        mcu_id  = 0;
    }

    *p_mcu_id  = mcu_id;
    *p_lock_id = lock_id;

    return DRV_E_NONE;
}

int32 drv_duet2_get_mcu_addr(uint8 mcu_id, uint32* mutex_data_addr, uint32* mutex_mask_addr)
{
    if((NULL == mutex_data_addr) || (NULL == mutex_mask_addr))
    {
        return DRV_E_INVALID_PTR;
    }
    if(0 != mcu_id)
    {
        return DRV_E_EXCEED_MAX_SIZE;
    }
    *mutex_data_addr = MCU0_MUTEX0_CPU_DATA;
    *mutex_mask_addr = MCU0_MUTEX0_CPU_MASK;
    return DRV_E_NONE;
}

int32
drv_duet2_chip_read_hss15g(uint8 lchip, uint8 hssid, uint32 addr, uint16* p_data)
{
    int32 ret = 0;
    HsMacroPrtReq0_m hss_req;
    HsMacroPrtAck0_m hss_ack;
    uint32 cmd = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;
    HsMacroPrtReqId0_m req_id;

    DRV_PTR_VALID_CHECK(p_data);
    DRV_INIT_CHECK(lchip);

    if (hssid > DRV_HSS15G_MACRO_NUM - 1)
    {
        return DRV_E_EXCEED_MAX_SIZE;
    }

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    sal_memset(&hss_req, 0, sizeof(HsMacroPrtReq0_m));
    sal_memset(&hss_ack, 0, sizeof(HsMacroPrtAck0_m));
    sal_memset(&req_id, 0, sizeof(HsMacroPrtReqId0_m));

    /* 1. write reqId to hss15g access */
    SetHsMacroPrtReqId0(V, reqId_f, &req_id, 16);
    cmd = DRV_IOW((HsMacroPrtReqId0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &req_id);

    cmd = DRV_IOR((HsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);
    if (GetHsMacroPrtReq0(V, reqPrtValid_f, &hss_req))
    {
        return DRV_E_OCCPANCY;
    }

    /* 2. Write request address, Trigger read commend */
    SetHsMacroPrtReq0(V, reqPrtAddr_f, &hss_req, addr);
    SetHsMacroPrtReq0(V, reqPrtIsRead_f , &hss_req, 1);
    SetHsMacroPrtReq0(V, reqPrtValid_f , &hss_req, 1);
    cmd = DRV_IOW((HsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);
    if (ret < 0)
    {
        return DRV_E_ACCESS_HSS12G_FAIL;
    }


    if (0 == SDK_WORK_PLATFORM)
    {
        /* 3. Read hssReadDataValid register, and check whether current accssing is done */
        cmd = DRV_IOR((HsMacroPrtAck0_t+hssid), DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &hss_ack);
        if (ret < 0)
        {
            return DRV_E_ACCESS_HSS12G_FAIL;
        }

        while (!GetHsMacroPrtAck0(V, ackPrtValid_f, &hss_ack) && (--timeout))
        {
            if (GetHsMacroPrtAck0(V, ackPrtValid_f, &hss_ack))
            {
                break;
            }
        }

        if (!GetHsMacroPrtAck0(V, ackPrtValid_f, &hss_ack))
        {
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
    }

    /* 4. Read hssReadData registers */
    *p_data = GetHsMacroPrtAck0(V, ackPrtReadData_f, &hss_ack);

    return DRV_E_NONE;
}

/**
 @brief access hss15g control register
*/
int32
drv_duet2_chip_write_hss15g(uint8 lchip, uint8 hssid, uint32 addr, uint16 data)
{
    int32 ret = 0;
    HsMacroPrtReq0_m hss_req;
    HsMacroPrtAck0_m hss_ack;
    HsMacroPrtReqId0_m req_id;
    uint32 cmd = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;
    uint32 tmp_val32 = 0;

    DRV_INIT_CHECK(lchip);

    if (hssid > DRV_HSS15G_MACRO_NUM - 1)
    {
        return DRV_E_EXCEED_MAX_SIZE;
    }

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    sal_memset(&hss_req, 0, sizeof(HsMacroPrtReq0_m));
    sal_memset(&hss_ack, 0, sizeof(HsMacroPrtAck0_m));
    sal_memset(&req_id, 0, sizeof(HsMacroPrtReqId0_m));

    /* 1. write reqId to hss15g access */
    tmp_val32 = 16;
    DRV_IOW_FIELD(lchip, (HsMacroPrtReqId0_t+hssid), HsMacroPrtReqId0_reqId_f, &tmp_val32, &req_id);

    cmd = DRV_IOW((HsMacroPrtReqId0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &req_id);

    cmd = DRV_IOR((HsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);
    if (GetHsMacroPrtReq0(V, reqPrtValid_f, &hss_req))
    {
        return DRV_E_OCCPANCY;
    }

    /* 2. Write request addr and data, trigger write operation*/
    tmp_val32 = (uint32)addr;
    DRV_IOW_FIELD(lchip, (HsMacroPrtReq0_t+hssid), HsMacroPrtReq0_reqPrtAddr_f, &tmp_val32, &hss_req);
    tmp_val32 = (uint32)data;
    DRV_IOW_FIELD(lchip, (HsMacroPrtReq0_t+hssid), HsMacroPrtReq0_reqPrtWriteData_f, &tmp_val32, &hss_req);
    tmp_val32 = 0;
    DRV_IOW_FIELD(lchip, (HsMacroPrtReq0_t+hssid), HsMacroPrtReq0_reqPrtIsRead_f, &tmp_val32, &hss_req);
    tmp_val32 = 1;
    DRV_IOW_FIELD(lchip, (HsMacroPrtReq0_t+hssid), HsMacroPrtReq0_reqPrtValid_f, &tmp_val32, &hss_req);
    cmd = DRV_IOW((HsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);
    if (ret < 0)
    {
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    if (0 == SDK_WORK_PLATFORM)
    {
        /* 3. Read hssReadDataValid register, and check whether current accssing is done */
        cmd = DRV_IOR((HsMacroPrtAck0_t+hssid), DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &hss_ack);
        if (ret < 0)
        {
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
        while (!GetHsMacroPrtAck0(V, ackPrtValid_f, &hss_ack) && (--timeout))
        {
            if (GetHsMacroPrtAck0(V, ackPrtValid_f, &hss_ack))
            {
                break;
            }
        }

        if (!GetHsMacroPrtAck0(V, ackPrtValid_f, &hss_ack))
        {
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
    }

    return DRV_E_NONE;
}

/**
 @brief access hss28g control register
*/
int32
drv_duet2_chip_read_hss28g(uint8 lchip, uint8 hssid, uint32 addr, uint16* p_data)
{
    int32 ret = 0;
    CsMacroPrtReq0_m hss_req;
    CsMacroPrtAck0_m hss_ack;
    CsMacroPrtReqId0_m req_id;
    uint32 cmd = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;

    DRV_PTR_VALID_CHECK(p_data);
    DRV_INIT_CHECK(lchip);

    if (hssid > DRV_HSS28G_MACRO_NUM - 1)
    {
        return DRV_E_EXCEED_MAX_SIZE;
    }

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    sal_memset(&hss_req, 0, sizeof(CsMacroPrtReq0_m));
    sal_memset(&hss_ack, 0, sizeof(CsMacroPrtAck0_m));
    sal_memset(&req_id, 0, sizeof(CsMacroPrtReqId0_m));

    /* 1. write reqId to hss28g access */
    SetCsMacroPrtReqId0(V, reqId_f, &req_id, 8);
    cmd = DRV_IOW((CsMacroPrtReqId0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &req_id);

    cmd = DRV_IOR((CsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);
    if (GetCsMacroPrtReq0(V, reqPrtValid_f, &hss_req))
    {
        return DRV_E_OCCPANCY;
    }

    /* 2. Write request address, Trigger read commend */
    SetCsMacroPrtReq0(V, reqPrtAddr_f, &hss_req, addr);
    SetCsMacroPrtReq0(V, reqPrtIsRead_f , &hss_req, 1);
    SetCsMacroPrtReq0(V, reqPrtValid_f , &hss_req, 1);
    cmd = DRV_IOW((CsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);
    if (ret < 0)
    {
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    if (0 == SDK_WORK_PLATFORM)
    {
        /* 3. Read hssReadDataValid register, and check whether current accssing is done */
        cmd = DRV_IOR((CsMacroPrtAck0_t+hssid), DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &hss_ack);
        if (ret < 0)
        {
            return DRV_E_ACCESS_HSS12G_FAIL;
        }

        while (!GetCsMacroPrtAck0(V, ackPrtValid_f, &hss_ack) && (--timeout))
        {
            if (GetCsMacroPrtAck0(V, ackPrtValid_f, &hss_ack))
            {
                break;
            }
        }

        if (!GetCsMacroPrtAck0(V, ackPrtValid_f, &hss_ack))
        {
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
    }

    /* 4. Read hssReadData registers */
    *p_data = GetCsMacroPrtAck0(V, ackPrtReadData_f, &hss_ack);

    return DRV_E_NONE;
}

/**
 @brief access hss28g control register
*/
int32
drv_duet2_chip_write_hss28g(uint8 lchip, uint8 hssid, uint32 addr, uint16 data)
{
    int32 ret = 0;
    CsMacroPrtReq0_m hss_req;
    CsMacroPrtAck0_m hss_ack;
    CsMacroPrtReqId0_m req_id;
    uint32 cmd = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;
    uint32 tmp_val32 = 0;

    DRV_INIT_CHECK(lchip);

    if (hssid > DRV_HSS28G_MACRO_NUM - 1)
    {
        return DRV_E_EXCEED_MAX_SIZE;
    }

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    sal_memset(&hss_req, 0, sizeof(CsMacroPrtReq0_m));
    sal_memset(&hss_ack, 0, sizeof(CsMacroPrtAck0_m));
    sal_memset(&req_id, 0, sizeof(CsMacroPrtReqId0_m));

    /* 1. write reqId to hss28g access */
    tmp_val32 = 8;
    DRV_IOW_FIELD(lchip, (CsMacroPrtReqId0_t+hssid), CsMacroPrtReqId0_reqId_f, &tmp_val32, &req_id);
    cmd = DRV_IOW((CsMacroPrtReqId0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &req_id);

    cmd = DRV_IOR((CsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);

    if (GetCsMacroPrtReq0(V, reqPrtValid_f, &hss_req))
    {
        return DRV_E_OCCPANCY;
    }

    /* 2. Write request addr and data */
    tmp_val32 = (uint32)addr;
    DRV_IOW_FIELD(lchip, (CsMacroPrtReq0_t+hssid), CsMacroPrtReq0_reqPrtAddr_f, &tmp_val32, &hss_req);
    tmp_val32 = (uint32)data;
    DRV_IOW_FIELD(lchip, (CsMacroPrtReq0_t+hssid), CsMacroPrtReq0_reqPrtWriteData_f, &tmp_val32, &hss_req);
    tmp_val32 = 0;
    DRV_IOW_FIELD(lchip, (CsMacroPrtReq0_t+hssid), CsMacroPrtReq0_reqPrtIsRead_f, &tmp_val32, &hss_req);
    tmp_val32 = 1;
    DRV_IOW_FIELD(lchip, (CsMacroPrtReq0_t+hssid), CsMacroPrtReq0_reqPrtValid_f, &tmp_val32, &hss_req);
    cmd = DRV_IOW((CsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);

    if (ret < 0)
    {
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    if (0 == SDK_WORK_PLATFORM)
    {
        /* 3. Read hssReadDataValid register, and check whether current accssing is done */
        cmd = DRV_IOR((CsMacroPrtAck0_t+hssid), DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &hss_ack);
        if (ret < 0)
        {
            return DRV_E_ACCESS_HSS12G_FAIL;
        }

        while (!GetCsMacroPrtAck0(V, ackPrtValid_f, &hss_ack) && (--timeout))
        {
            if (GetCsMacroPrtAck0(V, ackPrtValid_f, &hss_ack))
            {
                break;
            }
        }

        if (!GetCsMacroPrtAck0(V, ackPrtValid_f, &hss_ack))
        {
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
    }

    return DRV_E_NONE;
}

int32
drv_enum_init_duet2(uint8 lchip)
{
    DRV_ENUM(DRV_TCAMKEYTYPE_MACKEY_160)          = 0x0;
    DRV_ENUM(DRV_TCAMKEYTYPE_L3KEY_160)             = 0x1;
    DRV_ENUM(DRV_TCAMKEYTYPE_L3KEY_320)             = 0x2;
    DRV_ENUM(DRV_TCAMKEYTYPE_IPV6KEY_320)          = 0x3;
    DRV_ENUM(DRV_TCAMKEYTYPE_IPV6KEY_640)          = 0x4;
    DRV_ENUM(DRV_TCAMKEYTYPE_MACL3KEY_320)       = 0x5;
    DRV_ENUM(DRV_TCAMKEYTYPE_MACL3KEY_640)        = 0x6;
    DRV_ENUM(DRV_TCAMKEYTYPE_MACIPV6KEY_640)     = 0x7;
    DRV_ENUM(DRV_TCAMKEYTYPE_CIDKEY_160)            = 0x8;
    DRV_ENUM(DRV_TCAMKEYTYPE_SHORTKEY_80)         = 0x9;
    DRV_ENUM(DRV_TCAMKEYTYPE_FORWARDKEY_320)   = 0xa;
    DRV_ENUM(DRV_TCAMKEYTYPE_FORWARDKEY_640)   = 0xb;
    DRV_ENUM(DRV_TCAMKEYTYPE_COPPKEY_320)         = 0xc;
    DRV_ENUM(DRV_TCAMKEYTYPE_COPPKEY_640)         = 0xd;
    DRV_ENUM(DRV_TCAMKEYTYPE_UDFKEY_320)           = 0xe;

    DRV_ENUM(DRV_SCL_KEY_TYPE_MACKEY160)            = 0x0;
    DRV_ENUM(DRV_SCL_KEY_TYPE_MACL3KEY320)         = 0x2;
    DRV_ENUM(DRV_SCL_KEY_TYPE_L3KEY160)               = 0x1;
    DRV_ENUM(DRV_SCL_KEY_TYPE_IPV6KEY320)            = 0x1;
    DRV_ENUM(DRV_SCL_KEY_TYPE_MACIPV6KEY640)      = 0x2;
    DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT) = 0x3;
    DRV_ENUM(DRV_SCL_KEY_TYPE_UDFKEY160)             = 0x6;
    DRV_ENUM(DRV_SCL_KEY_TYPE_UDFKEY320)             = 0x4;
    DRV_ENUM(DRV_SCL_KEY_TYPE_MASK)                    = 0x3;


    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_REGULAR_PORT)      = 0x0;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITHOUT_TUNNEL)    = 0x6;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2)           = 0x7;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV4)  = 0x8;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV6)  = 0x9;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_IPV4)         = 0xa;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_IPV6)         = 0xb;


    DRV_ENUM(DRV_DMA_PACKET_RX0_CHAN_ID) = 0;
    DRV_ENUM(DRV_DMA_PACKET_RX1_CHAN_ID) = 1;
    DRV_ENUM(DRV_DMA_PACKET_RX2_CHAN_ID) = 2;
    DRV_ENUM(DRV_DMA_PACKET_RX3_CHAN_ID) = 3;
    DRV_ENUM(DRV_DMA_PACKET_TX0_CHAN_ID) = 4;
    DRV_ENUM(DRV_DMA_PACKET_TX1_CHAN_ID) = 5;
    DRV_ENUM(DRV_DMA_TBL_WR_CHAN_ID)     = 6;
    DRV_ENUM(DRV_DMA_TBL_RD_CHAN_ID)     = 7;
    DRV_ENUM(DRV_DMA_PORT_STATS_CHAN_ID) = 8;
    DRV_ENUM(DRV_DMA_FLOW_STATS_CHAN_ID) = 9;
    DRV_ENUM(DRV_DMA_REG_MAX_CHAN_ID)    = 10;
    DRV_ENUM(DRV_DMA_LEARNING_CHAN_ID)   = 11;
    DRV_ENUM(DRV_DMA_HASHKEY_CHAN_ID)    = 12;
    DRV_ENUM(DRV_DMA_IPFIX_CHAN_ID)      = 13;
    DRV_ENUM(DRV_DMA_SDC_CHAN_ID)        = 14;
    DRV_ENUM(DRV_DMA_MONITOR_CHAN_ID)    = 15;

    /*Unsed channel*/
    DRV_ENUM(DRV_DMA_PACKET_TX2_CHAN_ID) = 30;
    DRV_ENUM(DRV_DMA_PACKET_TX3_CHAN_ID) = 30;
    DRV_ENUM(DRV_DMA_TBL_RD1_CHAN_ID)    = 30;
    DRV_ENUM(DRV_DMA_TBL_RD2_CHAN_ID)    = 30;
    DRV_ENUM(DRV_DMA_TCAM_SCAN_DESC_NUM) = 33;

    DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA) = 0x33;
    DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2) = 0x35;
    return 0;
}

int32
drv_tbls_list_init_duet2(uint8 lchip)
{

    p_drv_master[lchip]->p_tbl_ext_info = (tables_ext_info_t*)mem_malloc(MEM_SYSTEM_MODULE, MaxTblId_t*sizeof(tables_ext_info_t));
    if(!p_drv_master[lchip]->p_tbl_ext_info)
    {
        return -1;
    }

    sal_memset(p_drv_master[lchip]->p_tbl_ext_info, 0, MaxTblId_t*sizeof(tables_ext_info_t));
    p_drv_master[lchip]->p_tbl_info = drv_dt2_tbls_list;

    p_drv_master[lchip]->p_mem_info = drv_dt2_mem;
    drv_mem_init_dt2(lchip);

    p_drv_master[lchip]->drv_ecc_data.p_intr_tbl = drv_ecc_dt2_err_intr_tbl;
    p_drv_master[lchip]->drv_ecc_data.p_sbe_cnt  = drv_ecc_dt2_sbe_cnt;
    p_drv_master[lchip]->drv_ecc_data.p_scan_tcam_tbl = drv_ecc_dt2_scan_tcam_tbl;

    p_drv_master[lchip]->drv_mem_get_edram_bitmap = sys_duet2_ftm_get_edram_bitmap;
    p_drv_master[lchip]->drv_mem_get_hash_type = sys_duet2_ftm_get_hash_type;

    p_drv_master[lchip]->drv_get_mcu_lock_id = drv_duet2_get_mcu_lock_id;
    p_drv_master[lchip]->drv_get_mcu_addr = drv_duet2_get_mcu_addr;

    p_drv_master[lchip]->drv_chip_read_hss15g = drv_duet2_chip_read_hss15g;
    p_drv_master[lchip]->drv_chip_write_hss15g = drv_duet2_chip_write_hss15g;
    p_drv_master[lchip]->drv_chip_read_hss28g = drv_duet2_chip_read_hss28g;
    p_drv_master[lchip]->drv_chip_write_hss28g = drv_duet2_chip_write_hss28g;

    p_drv_master[lchip]->p_enum_value = drv_tm_enum;
    drv_enum_init_duet2(lchip);

    return 0;
}

int32
drv_tbls_list_deinit_duet2(uint8 lchip)
{
    if(p_drv_master[lchip]->p_tbl_ext_info)
    {
        mem_free(p_drv_master[lchip]->p_tbl_ext_info);
    }
    return 0;
}


#undef DRV_DEF_M
#undef DRV_DEF_D
#undef DRV_DEF_F
#undef DRV_DEF_C
#undef DRV_DEF_E
#undef DRV_DEF_SDK_F
#undef DRV_DEF_SDK_D


#endif
