
#ifdef TSINGMA

#include "usw/include/drv_common.h"
#include "usw/include/drv_enum.h"
#include "usw/include/drv_ftm.h"
#include "usw/include/drv_ser.h"

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
   static addrs_t RegName##_tbl_addrs_tm[] = {__VA_ARGS__};

 /*DS LIST*/
#define CTC_DS_INFO(ModuleName, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, ...) \
   { \
      #ModuleName, \
      #RegName, \
      SliceType, \
      OpType, \
      Entry, \
      Word*4, \
      DB,  \
      ECC, \
      Bus,    \
      Stats,  \
      sizeof(RegName##_tbl_addrs_tm)/sizeof(addrs_t), \
      sizeof(RegName##_tbl_fields_tm)/sizeof(fields_t), \
      RegName##_tbl_addrs_tm, \
      RegName##_tbl_fields_tm, \
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
  static segs_t RegName##_##FieldName##_tbl_segs_tm[] = {__VA_ARGS__};

 /*Field Info*/
#define CTC_FIELD_INFO(ModuleName, RegName, FieldName, FullName, Bits, ...) \
   { \
      #FullName, \
	  RegName##_##FieldName##_f,\
      Bits, \
      sizeof(RegName##_##FieldName##_tbl_segs_tm) / sizeof(segs_t), \
      RegName##_##FieldName##_tbl_segs_tm, \
   },

 /*DS Field List Seperator*/
#define CTC_DS_SEPERATOR_INFO(ModuleName, RegName, SliceType, OpType, Entry, Word, ...) \
 }; \
 static fields_t RegName##_tbl_fields_tm[] = {


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
#include "usw/include/drv_ds_tm.h"
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
#define DRV_DEF_F(ModuleName, InstNum, RegName, FieldName, FullName, Bits, ...) \
        CTC_FIELD_INFO(ModuleName, RegName, FieldName, FullName, Bits, __VA_ARGS__)
#endif
#define DRV_DEF_E()
#define DRV_DEF_SDK_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, ...) \
        CTC_DS_SEPERATOR_INFO(ModuleName, RegName, SliceType, OpType, Entry, Word, __VA_ARGS__)
#define DRV_DEF_SDK_F(ModuleName, InstNum, RegName, FieldName,FullName, Bits, ...) \
        CTC_FIELD_INFO(ModuleName, RegName, FieldName, FullName, Bits, __VA_ARGS__)
fields_t tsingma_fields_1st = {NULL,0,0,0,NULL
#include "usw/include/drv_ds_tm.h"
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
#define DRV_DEF_F(ModuleName, InstNum, RegName, FieldName, FullName, Bits, ...)
#endif
#define DRV_DEF_SDK_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, ...) \
        CTC_DS_ADDR(ModuleName, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, __VA_ARGS__)
#define DRV_DEF_SDK_F(ModuleName, InstNum, RegName, FieldName, FullName, Bits, ...)
#define DRV_DEF_E()
#include "usw/include/drv_ds_tm.h"
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
#define DRV_DEF_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, ...) \
        CTC_DS_INFO(ModuleName, RegName, SliceType, OpType, Entry, Word, DB, ECC, Bus, Stats, __VA_ARGS__)
#define DRV_DEF_F(ModuleName, InstNum, RegName, FieldName, FullName, Bits, ...)
#endif
#define DRV_DEF_E() \
        CTC_DS_INFO_E()
#define DRV_DEF_SDK_D(ModuleName, InstNum, RegName, SliceType, OpType, Entry, Word, ...) \
        CTC_DS_INFO(ModuleName, RegName, SliceType, OpType, Entry, Word, __VA_ARGS__)
#define DRV_DEF_SDK_F(ModuleName, InstNum, RegName, FieldName, Bits, ...)
tables_info_t drv_tm_tbls_list[] = {
#include "usw/include/drv_ds_tm.h"
};

/*ECC Interrupt*/
#define DRV_ECC_F(...)    {__VA_ARGS__},



#define TSINGMA_MCU_MUTEX0_CPU_DATA_0 0x004c0080     /*McuMutex0CpuCtl0.mutex0CpuData*/
#define TSINGMA_MCU_MUTEX0_CPU_MASK_0 0x004c0084     /*McuMutex0CpuCtl0.mutex0CpuMask*/
#define TSINGMA_MCU_MUTEX0_CPU_DATA_1 0x00540080     /*McuMutex0CpuCtl1.mutex0CpuData*/
#define TSINGMA_MCU_MUTEX0_CPU_MASK_1 0x00540084     /*McuMutex0CpuCtl1.mutex0CpuMask*/
#define TSINGMA_MCU_MUTEX0_CPU_DATA_2 0x005c0080     /*McuMutex0CpuCtl2.mutex0CpuData*/
#define TSINGMA_MCU_MUTEX0_CPU_MASK_2 0x005c0084     /*McuMutex0CpuCtl2.mutex0CpuMask*/
#define TSINGMA_MCU_MUTEX0_CPU_DATA_3 0x002c0080     /*McuMutex0CpuCtl3.mutex0CpuData*/
#define TSINGMA_MCU_MUTEX0_CPU_MASK_3 0x002c0084     /*McuMutex0CpuCtl3.mutex0CpuMask*/

#define TSINGMA_MCU_MUTEX1_CPU_DATA_0 0x004c0088     /*McuMutex1CpuCtl0.mutex0CpuData*/
#define TSINGMA_MCU_MUTEX1_CPU_MASK_0 0x004c008c     /*McuMutex1CpuCtl0.mutex0CpuMask*/
#define TSINGMA_MCU_MUTEX1_CPU_DATA_1 0x00540088     /*McuMutex1CpuCtl1.mutex0CpuData*/
#define TSINGMA_MCU_MUTEX1_CPU_MASK_1 0x0054008c     /*McuMutex1CpuCtl1.mutex0CpuMask*/
#define TSINGMA_MCU_MUTEX1_CPU_DATA_2 0x005c0088     /*McuMutex1CpuCtl2.mutex0CpuData*/
#define TSINGMA_MCU_MUTEX1_CPU_MASK_2 0x005c008c     /*McuMutex1CpuCtl2.mutex0CpuMask*/
#define TSINGMA_MCU_MUTEX1_CPU_DATA_3 0x002c0088     /*McuMutex1CpuCtl3.mutex0CpuData*/
#define TSINGMA_MCU_MUTEX1_CPU_MASK_3 0x002c008c     /*McuMutex1CpuCtl3.mutex0CpuMask*/



uint32 drv_tm_enum[DRV_ENUM_MAX];

drv_mem_t drv_tm_mem[DRV_FTM_MAX_ID];

drv_ecc_intr_tbl_t drv_ecc_tm_err_intr_tbl[] =
{
#include "usw/include/drv_ecc_intr_tm.h"
    {MaxTblId_t,0,0,0,0,0,0,0,0}
};

drv_ecc_sbe_cnt_t drv_ecc_tm_sbe_cnt[] =
{
    {MaxTblId_t,"bufRetrvReassembleFifo",BufRetrvParityStatus_t,BufRetrvParityStatus_bufRetrvReassembleFifoSbeCnt_f},
    {DsBufRetrvExcp_t,NULL,BufRetrvParityStatus_t,BufRetrvParityStatus_dsBufRetrvExcpSbeCnt_f},
    {MaxTblId_t,"miscIntfDataMem",BufRetrvParityStatus_t,BufRetrvParityStatus_miscIntfDataMemSbeCnt_f},
    {BufRetrvLinkListRam_t,NULL,BufRetrvParityStatus_t,BufRetrvParityStatus_bufRetrvLinkListRamSbeCnt_f},
    {BufRetrvPktMsgMem_t,NULL,BufRetrvParityStatus_t,BufRetrvParityStatus_bufRetrvPktMsgMemSbeCnt_f},
    {BufRetrvFirstFragRam_t,NULL,BufRetrvParityStatus_t,BufRetrvParityStatus_bufRetrvFirstFragRamSbeCnt_f},
    {BufRetrvMsgParkMem_t,NULL,BufRetrvParityStatus_t,BufRetrvParityStatus_bufRetrvMsgParkMemSbeCnt_f},
    {MaxTblId_t,"bufStorePktAbortReqFifo", BufStoreParityStatus_t,BufStoreParityStatus_bufStorePktAbortReqFifoSbeCnt_f},
    {MaxTblId_t,"bufStoreIpeFreePtrFifo",BufStoreParityStatus_t,BufStoreParityStatus_bufStoreIpeFreePtrFifoSbeCnt_f},
    {DsIrmPortCnt_t,NULL,BufStoreParityStatus_t,BufStoreParityStatus_dsIrmPortCntSbeCnt_f},
    {MaxTblId_t,"bsMetFifoReleaseFifo",BufStoreParityStatus_t,BufStoreParityStatus_bsMetFifoReleaseFifoSbeCnt_f},
    {BufStoreFreeListRam_t,NULL,BufStoreParityStatus_t,BufStoreParityStatus_bufStoreFreeListRamSbeCnt_f},
    {DsEgressDtls_t,NULL,CapwapProcParityStatus_t,CapwapProcParityStatus_dsEgressDtlsSbeCnt_f},
    {DsIpeCoPPCountY_t,NULL,CoppParityStatus_t,CoppParityStatus_dsIpeCoPPCountYSbeCnt_f},
    {DsIpeCoPPCountX_t,NULL,CoppParityStatus_t,CoppParityStatus_dsIpeCoPPCountXSbeCnt_f},
    {DsIpeCoPPConfig_t,NULL,CoppParityStatus_t,CoppParityStatus_dsIpeCoPPConfigSbeCnt_f},
    {Dma2AxiMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dma2AxiMemSbeCnt_f},
    {Dma1AxiMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dma1AxiMemSbeCnt_f},
    {Dma0AxiMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dma0AxiMemSbeCnt_f},
    {DmaInfoMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaInfoMemSbeCnt_f},
    {DmaWrReqDataFifo_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaWrReqDataFifoSbeCnt_f},
    {DmaUserRegMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaUserRegMemSbeCnt_f},
    {DmaDescCache_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaDescCacheSbeCnt_f},
    {MaxTblId_t,"dmaPktTxFifo",DmaCtlParityStatus_t,DmaCtlParityStatus_dmaPktTxFifoSbeCnt_f},
    {DmaPktRxMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaPktRxMemSbeCnt_f},
    {DmaRegWrMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaRegWrMemSbeCnt_f},
    {DmaRegRdMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaRegRdMemSbeCnt_f},
    {DmaInfoMem_t,NULL,DmaCtlParityStatus_t,DmaCtlParityStatus_dmaInfoMemSbeCnt_f},
    {DsAgingStatusTcam_t,NULL,DsAgingParityStatus_t,DsAgingParityStatus_dsAgingStatusTcamSbeCnt_f},
    {DsAgingStatusFib_t,NULL,DsAgingParityStatus_t,DsAgingParityStatus_dsAgingStatusFibSbeCnt_f},
    {DsAging_t,NULL,DsAgingParityStatus_t,DsAgingParityStatus_dsAgingSbeCnt_f},
    {DsElephantFlowState_t,NULL,EfdParityStatus_t,EfdParityStatus_dsElephantFlowStateSbeCnt_f},
    {DsVlanXlateDefault_t,NULL,EgrOamHashParityStatus_t,EgrOamHashParityStatus_dsVlanXlateDefaultSbeCnt_f},
    {DsDestEthLmProfile_t,NULL,EpeAclOamParityStatus_t,EpeAclOamParityStatus_dsDestEthLmProfileSbeCnt_f},
    {EpeClaPolicingSopInfoFifo_t,NULL,EpeAclOamParityStatus_t,EpeAclOamParityStatus_epeClaPolicingSopInfoRamSbeCnt_f},
    {EpeClaEopInfoFifo_t,NULL,EpeAclOamParityStatus_t,EpeAclOamParityStatus_epeClaEopInfoFifoSbeCnt_f},
    {EpeHdrAdjEopMsgFifo_t,NULL,EpeHdrAdjParityStatus_t,EpeHdrAdjParityStatus_epeHdrAdjEopMsgFifoSbeCnt_f},
    {EpeHdrAdjDataFifo_t,NULL,EpeHdrAdjParityStatus_t,EpeHdrAdjParityStatus_epeHdrAdjDataFifoSbeCnt_f},
    {DsEgressDiscardStats_t,NULL,EpeHdrEditParityStatus_t,EpeHdrEditParityStatus_dsEgressDiscardStatsSbeCnt_f},
    {EpeHdrEditPktCtlFifo_t,NULL,EpeHdrEditParityStatus_t,EpeHdrEditParityStatus_epeHdrEditPktCtlFifoSbeCnt_f},
    {DsIpv6NatPrefix_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsIpv6NatPrefixSbeCnt_f},
    {DsLatencyWatermark_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsLatencyWatermarkSbeCnt_f},
    {DsLatencyMonCnt_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsLatencyMonCntSbeCnt_f},
    {DsL3Edit6W3rd_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsL3Edit6W3rdSbeCnt_f},
    {DsL2Edit6WOuter_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsL2Edit6WOuterSbeCnt_f},
    {DsCapwapFrag_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsCapwapFragSbeCnt_f},
    {DsEgressVsi_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsEgressVsiSbeCnt_f},
    {DsEgressPortMac_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsEgressPortMacSbeCnt_f},
    {DsPortLinkAgg_t,NULL,EpeHdrProcParityStatus_t,EpeHdrProcParityStatus_dsPortLinkAggSbeCnt_f},
    {DsDestVlanStatus_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestVlanStatusSbeCnt_f},
    {DsDestVlan_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestVlanSbeCnt_f},
    {DsDestVlanProfile_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestVlanProfileSbeCnt_f},
    {DsDestStpState_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestStpStateSbeCnt_f},
    {DsGlbDestPort_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsGlbDestPortSbeCnt_f},
    {MaxTblId_t,"eNHBypassInsideFifo",EpeNextHopParityStatus_t,EpeNextHopParityStatus_eNHBypassInsideFifoSbeCnt_f},
    {DsVlanTagBitMap_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsVlanTagBitMapSbeCnt_f},
    {DsPortIsolation_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsPortIsolationSbeCnt_f},
    {DsEgressVlanRangeProfile_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsEgressVlanRangeProfileSbeCnt_f},
    {DsEgressRouterMac_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsEgressRouterMacSbeCnt_f},
    {DsDestPort_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestPortSbeCnt_f},
    {DsDestInterface_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestInterfaceSbeCnt_f},
    {DsDestInterfaceProfile_t,NULL,EpeNextHopParityStatus_t,EpeNextHopParityStatus_dsDestInterfaceProfileSbeCnt_f},
    {DsMacLimitThreshold_t,NULL,FibAccParityStatus_t,FibAccParityStatus_dsMacLimitThresholdSbeCnt_f},
    {DsMacLimitCount_t,NULL,FibAccParityStatus_t,FibAccParityStatus_dsMacLimitCountSbeCnt_f},
    {FlowAccAdRam0_t,NULL,FlowAccParityStatus0_t,FlowAccParityStatus0_flowAccAdRamSbeCnt_f},
    {DsIpfixSamplingCount0_t,NULL,FlowAccParityStatus0_t,FlowAccParityStatus0_dsIpfixSamplingCountSbeCnt_f},
    {DsIpfixConfig0_t,NULL,FlowAccParityStatus0_t,FlowAccParityStatus0_dsIpfixConfigSbeCnt_f},
    {FlowAccAdRam1_t,NULL,FlowAccParityStatus1_t,FlowAccParityStatus1_flowAccAdRamSbeCnt_f},
    {DsIpfixSamplingCount1_t,NULL,FlowAccParityStatus1_t,FlowAccParityStatus1_dsIpfixSamplingCountSbeCnt_f},
    {DsIpfixConfig1_t,NULL,FlowAccParityStatus1_t,FlowAccParityStatus1_dsIpfixConfigSbeCnt_f},
    {MaxTblId_t,"flowTcamShareAd3",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamShareAd3SbeCnt_f},
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
    {MaxTblId_t,"flowTcamIpeUserIdAd3",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeUserIdAd3SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeUserIdAd2",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeUserIdAd2SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeUserIdAd1",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeUserIdAd1SbeCnt_f},
    {MaxTblId_t,"flowTcamIpeUserIdAd0",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamIpeUserIdAd0SbeCnt_f},
    {MaxTblId_t,"flowTcamEpeAclAd2",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamEpeAclAd2SbeCnt_f},
    {MaxTblId_t,"flowTcamEpeAclAd1",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamEpeAclAd1SbeCnt_f},
    {MaxTblId_t,"flowTcamEpeAclAd0",FlowTcamParityStatus_t,FlowTcamParityStatus_flowTcamEpeAclAd0SbeCnt_f},
    {DsStatsQueue_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsQueueSbeCnt_f},
    {DsStats_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsSbeCnt_f},
    {DsStatsEgressGlobal3_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsEgressGlobal3SbeCnt_f},
    {DsStatsEgressGlobal2_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsEgressGlobal2SbeCnt_f},
    {DsStatsEgressGlobal1_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsEgressGlobal1SbeCnt_f},
    {DsStatsEgressGlobal0_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsEgressGlobal0SbeCnt_f},
    {DsStatsEgressACL0_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsEgressACL0SbeCnt_f},
    {DsStatsIngressGlobal3_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressGlobal3SbeCnt_f},
    {DsStatsIngressGlobal2_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressGlobal2SbeCnt_f},
    {DsStatsIngressGlobal1_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressGlobal1SbeCnt_f},
    {DsStatsIngressGlobal0_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressGlobal0SbeCnt_f},
    {DsStatsIngressACL3_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressACL3SbeCnt_f},
    {DsStatsIngressACL2_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressACL2SbeCnt_f},
    {DsStatsIngressACL1_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressACL1SbeCnt_f},
    {DsStatsIngressACL0_t,NULL,GlobalStatsParityStatus_t,GlobalStatsParityStatus_dsStatsIngressACL0SbeCnt_f},
    {MaxTblId_t,"monHss1McuRamEcc",Hss28GMon_t,Hss28GMon_monHss1McuRamEccSbeCnt_f},
    {MaxTblId_t,"monHss0McuRamEcc",Hss28GMon_t,Hss28GMon_monHss0McuRamEccSbeCnt_f},
    {IpeFwdExcepGroupMap_t,NULL,IpeAclParityStatus_t,IpeAclParityStatus_ipeFwdExcepGroupMapSbeCnt_f},
    {DsSrcChannel_t,NULL,IpeAclParityStatus_t,IpeAclParityStatus_dsSrcChannelSbeCnt_f},
    {DsDestMapProfileUc_t,NULL,IpeAclParityStatus_t,IpeAclParityStatus_dsDestMapProfileUcSbeCnt_f},
    {DsCategoryIdPairHashRightKey_t,NULL,IpeAclParityStatus_t,IpeAclParityStatus_dsCategoryIdPairHashRightKeySbeCnt_f},
    {DsCategoryIdPairHashLeftKey_t,NULL,IpeAclParityStatus_t,IpeAclParityStatus_dsCategoryIdPairHashLeftKeySbeCnt_f},
    {DsCategoryIdPairHashRightAd_t,NULL,IpeAclParityStatus_t,IpeAclParityStatus_dsCategoryIdPairHashRightAdSbeCnt_f},
    {DsCategoryIdPairHashLeftAd_t,NULL,IpeAclParityStatus_t,IpeAclParityStatus_dsCategoryIdPairHashLeftAdSbeCnt_f},
    {DsCategoryIdPairTcamAd_t,NULL,IpeAclParityStatus_t,IpeAclParityStatus_dsCategoryIdPairTcamAdSbeCnt_f},
    {DsAclVlanActionProfile_t,NULL,IpeAclParityStatus_t,IpeAclParityStatus_dsAclVlanActionProfileSbeCnt_f},
    {DsCFlexSrcPortBlockMask_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsCFlexSrcPortBlockMaskSbeCnt_f},
    {DsIpePhbMutationCosMap_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsIpePhbMutationCosMapSbeCnt_f},
    {DsIpePhbMutationDscpMap_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsIpePhbMutationDscpMapSbeCnt_f},
    {DsEcmpMember_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsEcmpMemberSbeCnt_f},
    {DsEcmpGroup_t,NULL,IpeFwdParityStatus_t,IpeFwdParityStatus_dsEcmpGroupSbeCnt_f},
    {DsVlanActionProfile_t,NULL,IpeHdrAdjParityStatus_t,IpeHdrAdjParityStatus_dsVlanActionProfileSbeCnt_f},
    {DsVlanRangeProfile_t,NULL,IpeHdrAdjParityStatus_t,IpeHdrAdjParityStatus_dsVlanRangeProfileSbeCnt_f},
    {DsPhyPortExt_t,NULL,IpeHdrAdjParityStatus_t,IpeHdrAdjParityStatus_dsPhyPortExtSbeCnt_f},
    {DsPhyPort_t,NULL,IpeHdrAdjParityStatus_t,IpeHdrAdjParityStatus_dsPhyPortSbeCnt_f},
    {DsSrcStpState_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcStpStateSbeCnt_f},
    {DsSrcVlanProfile_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcVlanProfileSbeCnt_f},
    {DsSrcVlanStatus_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcVlanStatusSbeCnt_f},
    {DsSrcVlan_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcVlanSbeCnt_f},
    {DsVlan2Ptr_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsVlan2PtrSbeCnt_f},
    {DsSrcInterfaceProfile_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcInterfaceProfileSbeCnt_f},
    {DsSrcInterface_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcInterfaceSbeCnt_f},
    {DsSrcPort_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsSrcPortSbeCnt_f},
    {DsRouterMac_t,NULL,IpeIntfMapParityStatus_t,IpeIntfMapParityStatus_dsRouterMacSbeCnt_f},
    {DsRouterMacInner_t,NULL,IpeLkupMgrParityStatus_t,IpeLkupMgrParityStatus_dsRouterMacInnerSbeCnt_f},
    {DsSrcEthLmProfile_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_dsSrcEthLmProfileSbeCnt_f},
    {IpePhbDscpMap_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_ipePhbDscpMapSbeCnt_f},
    {DsBidiPimGroup_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_dsBidiPimGroupSbeCnt_f},
    {DsRpf_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_dsRpfSbeCnt_f},
    {DsVrf_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_dsVrfSbeCnt_f},
    {DsVsi_t,NULL,IpePktProcParityStatus_t,IpePktProcParityStatus_dsVsiSbeCnt_f},
    {DsPortBasedHashProfile1_t,NULL,LinkAggParityStatus_t,LinkAggParityStatus_dsPortBasedHashProfile1SbeCnt_f},
    {DsSgmacMap_t,NULL,LinkAggParityStatus_t,LinkAggParityStatus_dsSgmacMapSbeCnt_f},
    {DsLinkAggregateChannelMemberSet_t,NULL,LinkAggParityStatus_t,LinkAggParityStatus_dsLinkAggregateChannelMemberSetSbeCnt_f},
    {DsLinkAggregateMemberSet_t,NULL,LinkAggParityStatus_t,LinkAggParityStatus_dsLinkAggregateMemberSetSbeCnt_f},
    {DsLinkAggregationPortChannelMap_t,NULL,LinkAggParityStatus_t,LinkAggParityStatus_dsLinkAggregationPortChannelMapSbeCnt_f},
    {DsLinkAggregateChannelMember_t,NULL,LinkAggParityStatus_t,LinkAggParityStatus_dsLinkAggregateChannelMemberSbeCnt_f},
    {DsLinkAggregateMember_t,NULL,LinkAggParityStatus_t,LinkAggParityStatus_dsLinkAggregateMemberSbeCnt_f},
    {MaxTblId_t,"lpmTcamAd5",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd5SbeCnt_f},
    {MaxTblId_t,"lpmTcamAd4",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd4SbeCnt_f},
    {MaxTblId_t,"lpmTcamAd3",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd3SbeCnt_f},
    {MaxTblId_t,"lpmTcamAd2",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd2SbeCnt_f},
    {MaxTblId_t,"lpmTcamAd1",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd1SbeCnt_f},
    {MaxTblId_t,"lpmTcamAd0",LpmTcamParityStatus_t,LpmTcamParityStatus_lpmTcamAd0SbeCnt_f},
    {DsDot1AePnCheck_t,NULL,MacSecEngineParityStatus_t,MacSecEngineParityStatus_dsDot1AePnCheckSbeCnt_f},
    {DsDot1AeDecryptConfig_t,NULL,MacSecEngineParityStatus_t,MacSecEngineParityStatus_dsDot1AeDecryptConfigSbeCnt_f},
    {DsDot1AeSci_t,NULL,MacSecEngineParityStatus_t,MacSecEngineParityStatus_dsDot1AeSciSbeCnt_f},
    {DsDot1AeAesKey_t,NULL,MacSecEngineParityStatus_t,MacSecEngineParityStatus_dsDot1AeAesKeySbeCnt_f},
    //{MaxTblId_t,"mcuSupDataMem0",McuSupStats0_t,McuSupStats0_mcuSupDataMemSbeCnt_f},
    //{MaxTblId_t,"mcuSupDataMem1",McuSupStats1_t,McuSupStats1_mcuSupDataMemSbeCnt_f},
    //{MaxTblId_t,"mcuSupDataMem2",McuSupStats2_t,McuSupStats2_mcuSupDataMemSbeCnt_f},
    //{MaxTblId_t,"mcuSupDataMem3",McuSupStats3_t,McuSupStats3_mcuSupDataMemSbeCnt_f},
    //{MaxTblId_t,"mcuSupProgMem0",McuSupStats0_t,McuSupStats0_mcuSupProgMemSbeCnt_f},
    //{MaxTblId_t,"mcuSupProgMem1",McuSupStats1_t,McuSupStats1_mcuSupProgMemSbeCnt_f},
    //{MaxTblId_t,"mcuSupProgMem2",McuSupStats2_t,McuSupStats2_mcuSupProgMemSbeCnt_f},
    //{MaxTblId_t,"mcuSupProgMem3",McuSupStats3_t,McuSupStats3_mcuSupProgMemSbeCnt_f},
    {MetMcastHiRcdFifo_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metMcastHiRcdFifoSbeCnt_f},
    {MetMcastLoRcdFifo_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metMcastLoRcdFifoSbeCnt_f},
    {MaxTblId_t,"metMcastLoLoopFifo",MetFifoParityStatus_t,MetFifoParityStatus_metMcastLoLoopFifoSbeCnt_f},
    {MaxTblId_t,"metMcastHiLoopFifo",MetFifoParityStatus_t,MetFifoParityStatus_metMcastHiLoopFifoSbeCnt_f},
    {DsApsBridge_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_dsApsBridgeSbeCnt_f},
    {MaxTblId_t,"metLogReqFifo",MetFifoParityStatus_t,MetFifoParityStatus_metLogReqFifoSbeCnt_f},
    {DsDestMapProfileMc_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_dsDestMapProfileMcSbeCnt_f},
    {MetUcastLoRcdFifo_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metUcastLoRcdFifoSbeCnt_f},
    {MetUcastHiRcdFifo_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metUcastHiRcdFifoSbeCnt_f},
    {DsMetLinkAggregatePort_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_dsMetLinkAggregatePortSbeCnt_f},
    {DsMetLinkAggregateBitmap_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_dsMetLinkAggregateBitmapSbeCnt_f},
    {MetRcdMem_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metRcdMemSbeCnt_f},
    {MetMsgMem_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metMsgMemSbeCnt_f},
    {MetMcastTrackFifo_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metMcastTrackFifoSbeCnt_f},
    {DsMetFifoExcp_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_dsMetFifoExcpSbeCnt_f},
    {MetEnqRcdFifo_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metEnqRcdFifoSbeCnt_f},
    {MetBrRcdFifo_t,NULL,MetFifoParityStatus_t,MetFifoParityStatus_metBrRcdFifoSbeCnt_f},
    {DsMplsHashCamAd_t,NULL,MplsHashParityStatus_t,MplsHashParityStatus_dsMplsHashCamAdSbeCnt_f},
    {MaxTblId_t,"mplsHashAd",MplsHashParityStatus_t,MplsHashParityStatus_mplsHashAdSbeCnt_f},
    {MaxTblId_t,"NetRxFreePtr1Fifo",NetRxParityStatus_t,NetRxParityStatus_netRxFreePtr1FifoSbeCnt_f},
    {MaxTblId_t,"NetRxFreePtr0Fifo",NetRxParityStatus_t,NetRxParityStatus_netRxFreePtr0FifoSbeCnt_f},
    {DsChannelizeMode_t,NULL,NetRxParityStatus_t,NetRxParityStatus_dsChannelizeModeSbeCnt_f},
    {MaxTblId_t,"netRxFreePtr3Fifo",NetRxParityStatus_t,NetRxParityStatus_netRxFreePtr3FifoSbeCnt_f},
    {MaxTblId_t,"netRxFreePtr2Fifo",NetRxParityStatus_t,NetRxParityStatus_netRxFreePtr2FifoSbeCnt_f},
    {NetTxPktHdr3_t,NULL,NetTxParityStatus_t,NetTxParityStatus_netTxPktHdr3SbeCnt_f},
    {NetTxPktHdr2_t,NULL,NetTxParityStatus_t,NetTxParityStatus_netTxPktHdr2SbeCnt_f},
    {NetTxPktHdr1_t,NULL,NetTxParityStatus_t,NetTxParityStatus_netTxPktHdr1SbeCnt_f},
    {NetTxPktHdr0_t,NULL,NetTxParityStatus_t,NetTxParityStatus_netTxPktHdr0SbeCnt_f},
    {OamParserPktFifo_t,NULL,OamParserParityStatus_t,OamParserParityStatus_oamParserPktFifoSbeCnt_f},
    {MaxTblId_t,"pbCtlPktBufRRam",PbCtlParityStatus_t,PbCtlParityStatus_pbCtlPktBufRRamSbeCnt_f},
    {DsEpePolicing1CountY_t,NULL,PolicingEpeParityStatus_t,PolicingEpeParityStatus_dsEpePolicing1CountYSbeCnt_f},
    {DsEpePolicing1CountX_t,NULL,PolicingEpeParityStatus_t,PolicingEpeParityStatus_dsEpePolicing1CountXSbeCnt_f},
    {DsEpePolicing1Config_t,NULL,PolicingEpeParityStatus_t,PolicingEpeParityStatus_dsEpePolicing1ConfigSbeCnt_f},
    {DsEpePolicing0CountY_t,NULL,PolicingEpeParityStatus_t,PolicingEpeParityStatus_dsEpePolicing0CountYSbeCnt_f},
    {DsEpePolicing0CountX_t,NULL,PolicingEpeParityStatus_t,PolicingEpeParityStatus_dsEpePolicing0CountXSbeCnt_f},
    {DsEpePolicing0Config_t,NULL,PolicingEpeParityStatus_t,PolicingEpeParityStatus_dsEpePolicing0ConfigSbeCnt_f},
    {DsIpePolicing1ProfileY_t,NULL,PolicingIpeParityStatus_t,PolicingIpeParityStatus_dsIpePolicing1ProfileYSbeCnt_f},
    {DsIpePolicing1ProfileX_t,NULL,PolicingIpeParityStatus_t,PolicingIpeParityStatus_dsIpePolicing1ProfileXSbeCnt_f},
    {DsIpePolicing1CountY_t,NULL,PolicingIpeParityStatus_t,PolicingIpeParityStatus_dsIpePolicing1CountYSbeCnt_f},
    {DsIpePolicing1CountX_t,NULL,PolicingIpeParityStatus_t,PolicingIpeParityStatus_dsIpePolicing1CountXSbeCnt_f},
    {DsIpePolicing1Config_t,NULL,PolicingIpeParityStatus_t,PolicingIpeParityStatus_dsIpePolicing1ConfigSbeCnt_f},
    {DsIpePolicing0CountY_t,NULL,PolicingIpeParityStatus_t,PolicingIpeParityStatus_dsIpePolicing0CountYSbeCnt_f},
    {DsIpePolicing0CountX_t,NULL,PolicingIpeParityStatus_t,PolicingIpeParityStatus_dsIpePolicing0CountXSbeCnt_f},
    {DsIpePolicing0Config_t,NULL,PolicingIpeParityStatus_t,PolicingIpeParityStatus_dsIpePolicing0ConfigSbeCnt_f},
    {DsIpePolicing0ProfileY_t,NULL,PolicingIpeParityStatus_t,PolicingIpeParityStatus_dsIpePolicing0ProfileYSbeCnt_f},
    {DsIpePolicing0ProfileX_t,NULL,PolicingIpeParityStatus_t,PolicingIpeParityStatus_dsIpePolicing0ProfileXSbeCnt_f},
    {DsQMgrNetBaseQueMeterProfile_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrNetBaseQueMeterProfileSbeCnt_f},
    {DsQMgrNetBaseQueMeterProfId_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrNetBaseQueMeterProfIdSbeCnt_f},
    {DsQMgrExtGrpShpProfId_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrExtGrpShpProfIdSbeCnt_f},
    {DsQMgrExtQueShpProfId_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrExtQueShpProfIdSbeCnt_f},
    {DsQMgrNetQueShpProfile_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrNetQueShpProfileSbeCnt_f},
    {DsQMgrNetQueShpProfId_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrNetQueShpProfIdSbeCnt_f},
    {DsQMgrNetChanShpToken_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrNetChanShpTokenSbeCnt_f},
    {DsQMgrExtGrpShpProfile_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrExtGrpShpProfileSbeCnt_f},
    {DsQMgrExtSchMap_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrExtSchMap1SbeCnt_f},
    {DsQMgrBaseSchMap_t,NULL,QMgrDeqParityStatus_t,QMgrDeqParityStatus_dsQMgrBaseSchMap1SbeCnt_f},
    {DsErmResrcMonPortMax_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsErmResrcMonPortMaxSbeCnt_f},
    {DsErmQueueCfg_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsErmQueueCfgSbeCnt_f},
    {DsPortBlockMask_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsPortBlockMaskSbeCnt_f},
    {DsQueueMapTcamData_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsQueueMapTcamDataSbeCnt_f},
    {DsCFlexDstChannelBlockMask_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsCFlexDstChannelBlockMaskSbeCnt_f},
    {DsErmAqmQueueStatus_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsErmAqmQueueStatusSbeCnt_f},
    {DsErmAqmQueueCfg_t,NULL,QMgrEnqParityStatus_t,QMgrEnqParityStatus_dsErmAqmQueueCfgSbeCnt_f},
    {DsQueueEntry_t,NULL,QMgrMsgStoreParityStatus_t,QMgrMsgStoreParityStatus_dsQueueEntrySbeCnt_f},
    {DsQMsgUsedList_t,NULL,QMgrMsgStoreParityStatus_t,QMgrMsgStoreParityStatus_dsQMsgUsedListSbeCnt_f},
    {DsMsgFreePtr_t,NULL,QMgrMsgStoreParityStatus_t,QMgrMsgStoreParityStatus_dsMsgFreePtrSbeCnt_f},
    {MaxTblId_t,"shareBufRam6RdData" ,ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam6RdDataSbeCnt_f},
    {MaxTblId_t,"shareBufRam5RdData1",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam5RdData1SbeCnt_f},
    {MaxTblId_t,"shareBufRam5RdData0",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam5RdData0SbeCnt_f},
    {MaxTblId_t,"shareBufRam4RdData1",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam4RdData1SbeCnt_f},
    {MaxTblId_t,"shareBufRam4RdData0",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam4RdData0SbeCnt_f},
    {MaxTblId_t,"shareBufRam3RdData1",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam3RdData1SbeCnt_f},
    {MaxTblId_t,"shareBufRam3RdData0",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam3RdData0SbeCnt_f},
    {MaxTblId_t,"shareBufRam2RdData1",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam2RdData1SbeCnt_f},
    {MaxTblId_t,"shareBufRam2RdData0",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam2RdData0SbeCnt_f},
    {MaxTblId_t,"shareBufRam1RdData1",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam1RdData1SbeCnt_f},
    {MaxTblId_t,"shareBufRam1RdData0",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam1RdData0SbeCnt_f},
    {MaxTblId_t,"shareBufRam0RdData1",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam0RdData1SbeCnt_f},
    {MaxTblId_t,"shareBufRam0RdData0",ShareBufferParityStatus_t,ShareBufferParityStatus_shareBufRam0RdData0SbeCnt_f},
    {DsIpeStormCtl0ProfileX_t,NULL,StormCtlParityStatus_t,StormCtlParityStatus_dsIpeStormCtl0ProfileXSbeCnt_f},
    {DsIpeStormCtl0ProfileY_t,NULL,StormCtlParityStatus_t,StormCtlParityStatus_dsIpeStormCtl0ProfileYSbeCnt_f},
    {DsIpeStormCtl0Config_t,NULL,StormCtlParityStatus_t,StormCtlParityStatus_dsIpeStormCtl0ConfigSbeCnt_f},
    {DsIpeStormCtl0CountX_t,NULL,StormCtlParityStatus_t,StormCtlParityStatus_dsIpeStormCtl0CountXSbeCnt_f},
    {DsIpeStormCtl0CountY_t,NULL,StormCtlParityStatus_t,StormCtlParityStatus_dsIpeStormCtl0CountYSbeCnt_f},
    {DsIpeStormCtl1Config_t,NULL,StormCtlParityStatus_t,StormCtlParityStatus_dsIpeStormCtl1ConfigSbeCnt_f},
    {DsIpeStormCtl1CountX_t,NULL,StormCtlParityStatus_t,StormCtlParityStatus_dsIpeStormCtl1CountXSbeCnt_f},
    {DsIpeStormCtl1CountY_t,NULL,StormCtlParityStatus_t,StormCtlParityStatus_dsIpeStormCtl1CountYSbeCnt_f},
    {DsIpeStormCtl1ProfileX_t,NULL,StormCtlParityStatus_t,StormCtlParityStatus_dsIpeStormCtl1ProfileXSbeCnt_f},
    {DsIpeStormCtl1ProfileY_t,NULL,StormCtlParityStatus_t,StormCtlParityStatus_dsIpeStormCtl1ProfileYSbeCnt_f},
    {MaxTblId_t,"shareAdRam5",DynamicAdParityStatus_t,  DynamicAdParityStatus_shareAdRam5SbeCnt_f},
    {MaxTblId_t,"shareAdRam4",DynamicAdParityStatus_t,  DynamicAdParityStatus_shareAdRam4SbeCnt_f},
    {MaxTblId_t,"shareAdRam3",DynamicAdParityStatus_t,  DynamicAdParityStatus_shareAdRam3SbeCnt_f},
    {MaxTblId_t,"shareAdRam2",DynamicAdParityStatus_t,  DynamicAdParityStatus_shareAdRam2SbeCnt_f},
    {MaxTblId_t,"shareAdRam1",DynamicAdParityStatus_t,  DynamicAdParityStatus_shareAdRam1SbeCnt_f},
    {MaxTblId_t,"shareAdRam0",DynamicAdParityStatus_t,  DynamicAdParityStatus_shareAdRam0SbeCnt_f},
    {MaxTblId_t,"shareEditRam5",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam5SbeCnt_f},
    {MaxTblId_t,"shareEditRam4",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam4SbeCnt_f},
    {MaxTblId_t,"shareEditRam3",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam3SbeCnt_f},
    {MaxTblId_t,"shareEditRam2",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam2SbeCnt_f},
    {MaxTblId_t,"shareEditRam1",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam1SbeCnt_f},
    {MaxTblId_t,"shareEditRam0",DynamicEditParityStatus_t,DynamicEditParityStatus_shareEditRam0SbeCnt_f},
    {MaxTblId_t,"shareKeyRam10",DynamicKeyParityStatus_t, DynamicKeyParityStatus_shareKeyRam10SbeCnt_f},
    {MaxTblId_t,"shareKeyRam9",DynamicKeyParityStatus_t, DynamicKeyParityStatus_shareKeyRam9SbeCnt_f},
    {MaxTblId_t,"shareKeyRam8",DynamicKeyParityStatus_t, DynamicKeyParityStatus_shareKeyRam8SbeCnt_f},
    {MaxTblId_t,"shareKeyRam7",DynamicKeyParityStatus_t, DynamicKeyParityStatus_shareKeyRam7SbeCnt_f},
    {MaxTblId_t,"shareKeyRam6",DynamicKeyParityStatus_t, DynamicKeyParityStatus_shareKeyRam6SbeCnt_f},
    {MaxTblId_t,"shareKeyRam5",DynamicKeyParityStatus_t, DynamicKeyParityStatus_shareKeyRam5SbeCnt_f},
    {MaxTblId_t,"shareKeyRam4",DynamicKeyParityStatus_t, DynamicKeyParityStatus_shareKeyRam4SbeCnt_f},
    {MaxTblId_t,"shareKeyRam3",DynamicKeyParityStatus_t, DynamicKeyParityStatus_shareKeyRam3SbeCnt_f},
    {MaxTblId_t,"shareKeyRam2",DynamicKeyParityStatus_t, DynamicKeyParityStatus_shareKeyRam2SbeCnt_f},
    {MaxTblId_t,"shareKeyRam1",DynamicKeyParityStatus_t, DynamicKeyParityStatus_shareKeyRam1SbeCnt_f},
    {MaxTblId_t,"shareKeyRam0",DynamicKeyParityStatus_t, DynamicKeyParityStatus_shareKeyRam0SbeCnt_f},
    {MaxTblId_t,NULL,MaxTblId_t,0}
};


uint16 drv_ecc_tm_scan_tcam_tbl[][5] =
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
        drv_tm_mem[id].entry_num = num;\
        drv_tm_mem[id].addr_3w = addr3w;\
        drv_tm_mem[id].addr_6w = addr6w;\
        drv_tm_mem[id].addr_12w = addr12w;\
     }while(0);\

int32
drv_mem_init_tsingma(uint8 lchip)
{
    DRV_MEM_REG("KeyRam0", DRV_FTM_SRAM0, 16384, 0xa800000, 0xa000000, 0xb000000);
    DRV_MEM_REG("KeyRam1", DRV_FTM_SRAM1, 16384, 0xa880000, 0xa080000, 0xb080000);
    DRV_MEM_REG("KeyRam2", DRV_FTM_SRAM2, 16384, 0xa900000, 0xa100000, 0xb100000);
    DRV_MEM_REG("KeyRam3", DRV_FTM_SRAM3, 8192, 0xa980000, 0xa180000, 0xb180000);
    DRV_MEM_REG("KeyRam4", DRV_FTM_SRAM4, 8192, 0xaa00000, 0xa200000, 0xb200000);
    DRV_MEM_REG("KeyRam5", DRV_FTM_SRAM5, 4096, 0xaa80000, 0xa280000, 0xb280000);
    DRV_MEM_REG("KeyRam6", DRV_FTM_SRAM6, 8192, 0xab00000, 0xa300000, 0xb300000);
    DRV_MEM_REG("KeyRam7", DRV_FTM_SRAM7, 8192, 0xab80000, 0xa380000, 0xb380000);
    DRV_MEM_REG("KeyRam8", DRV_FTM_SRAM8, 8192, 0xac00000, 0xa400000, 0xb400000);
    DRV_MEM_REG("KeyRam9", DRV_FTM_SRAM9, 8192, 0xac80000, 0xa480000, 0xb480000);
    DRV_MEM_REG("KeyRam10", DRV_FTM_SRAM10, 8192, 0xad00000, 0xa500000, 0xb500000);
    DRV_MEM_REG("AdRam0", DRV_FTM_SRAM11, 16384, 0x5800000, 0x5000000, 0x5400000);
    DRV_MEM_REG("AdRam1", DRV_FTM_SRAM12, 8192, 0x5880000, 0x5080000, 0x5480000);
    DRV_MEM_REG("AdRam2", DRV_FTM_SRAM13, 8192, 0x5900000, 0x5100000, 0x5500000);
    DRV_MEM_REG("AdRam3", DRV_FTM_SRAM14, 16384, 0x5980000, 0x5180000, 0x5580000);
    DRV_MEM_REG("AdRam4", DRV_FTM_SRAM15, 8192, 0x5a00000, 0x5200000, 0x5600000);
    DRV_MEM_REG("AdRam5", DRV_FTM_SRAM16, 8192, 0x5a80000, 0x5280000, 0x5680000);
    DRV_MEM_REG("EditRam0", DRV_FTM_SRAM17, 32768, 0x3000000, 0x3400000, 0x3800000);
    DRV_MEM_REG("EditRam1", DRV_FTM_SRAM18, 8192, 0x3080000, 0x3480000, 0x3880000);
    DRV_MEM_REG("EditRam2", DRV_FTM_SRAM19, 8192, 0x3100000, 0x3500000, 0x3900000);
    DRV_MEM_REG("EditRam3", DRV_FTM_SRAM20, 4096, 0x3180000, 0x3580000, 0x3980000);
    DRV_MEM_REG("EditRam4", DRV_FTM_SRAM21, 4096, 0x3200000, 0x3600000, 0x3a00000);
    DRV_MEM_REG("EditRam5", DRV_FTM_SRAM22, 4096, 0x3280000, 0x3680000, 0x3a80000);
    DRV_MEM_REG("SharedRam0", DRV_FTM_SRAM23, 32768, 0x7000000, 0x0, 0x0);
    DRV_MEM_REG("SharedRam1", DRV_FTM_SRAM24, 32768, 0x7100000, 0x0, 0x0);
    DRV_MEM_REG("SharedRam2", DRV_FTM_SRAM25, 32768, 0x7200000, 0x0, 0x0);
    DRV_MEM_REG("SharedRam3", DRV_FTM_SRAM26, 32768, 0x7300000, 0x0, 0x0);
    DRV_MEM_REG("SharedRam4", DRV_FTM_SRAM27, 32768, 0x7400000, 0x0, 0x0);
    DRV_MEM_REG("SharedRam5", DRV_FTM_SRAM28, 32768, 0x7500000, 0, 0x0);
    DRV_MEM_REG("SharedRam6", DRV_FTM_SRAM29, 32768, 0x7600000, 0x7800000, 0x0);
    DRV_MEM_REG("MplsRam0", DRV_FTM_MIXED0, 4096, 0x1d00000, 0x0, 0x0);
    DRV_MEM_REG("MplsRam1", DRV_FTM_MIXED1, 4096, 0x1d10000, 0x0, 0x0);
    DRV_MEM_REG("MplsRam2", DRV_FTM_MIXED2, 8192, 0x1c00000, 0x1c80000, 0x0);
    DRV_MEM_REG("Tcam key0", DRV_FTM_TCAM_KEY0, 1024, 0x2400000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key1", DRV_FTM_TCAM_KEY1, 1024, 0x2410000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key2", DRV_FTM_TCAM_KEY2, 1024, 0x2420000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key3", DRV_FTM_TCAM_KEY3, 1024, 0x2430000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key4", DRV_FTM_TCAM_KEY4, 1024, 0x2440000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key5", DRV_FTM_TCAM_KEY5, 1024, 0x2450000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key6", DRV_FTM_TCAM_KEY6, 1024, 0x2460000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key7", DRV_FTM_TCAM_KEY7, 1024, 0x2470000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key8", DRV_FTM_TCAM_KEY8, 1024, 0x2480000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key9", DRV_FTM_TCAM_KEY9, 1024, 0x2490000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key10", DRV_FTM_TCAM_KEY10, 1024, 0x24a0000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key11", DRV_FTM_TCAM_KEY11, 1024, 0x24b0000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key12", DRV_FTM_TCAM_KEY12, 1024, 0x24c0000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key13", DRV_FTM_TCAM_KEY13, 1024, 0x24d0000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key14", DRV_FTM_TCAM_KEY14, 1024, 0x24e0000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key15", DRV_FTM_TCAM_KEY15, 2048, 0x24f0000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key16", DRV_FTM_TCAM_KEY16, 2048, 0x2500000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key17", DRV_FTM_TCAM_KEY17, 2048, 0x2510000, 0x0, 0x0);
    DRV_MEM_REG("Tcam key18", DRV_FTM_TCAM_KEY18, 2048, 0x2520000, 0x0, 0x0);
    DRV_MEM_REG("Tcam AD0", DRV_FTM_TCAM_AD0, 1024, 0x2600000, 0x2600000, 0x2600000);
    DRV_MEM_REG("Tcam AD1", DRV_FTM_TCAM_AD1, 1024, 0x2608000, 0x2608000, 0x2608000);
    DRV_MEM_REG("Tcam AD2", DRV_FTM_TCAM_AD2, 1024, 0x2610000, 0x2610000, 0x2610000);
    DRV_MEM_REG("Tcam AD3", DRV_FTM_TCAM_AD3, 1024, 0x2618000, 0x2618000, 0x2618000);
    DRV_MEM_REG("Tcam AD4", DRV_FTM_TCAM_AD4, 1024, 0x2620000, 0x2620000, 0x2620000);
    DRV_MEM_REG("Tcam AD5", DRV_FTM_TCAM_AD5, 1024, 0x2628000, 0x2628000, 0x2628000);
    DRV_MEM_REG("Tcam AD6", DRV_FTM_TCAM_AD6, 1024, 0x2630000, 0x2630000, 0x2630000);
    DRV_MEM_REG("Tcam AD7", DRV_FTM_TCAM_AD7, 1024, 0x2638000, 0x2638000, 0x2638000);
    DRV_MEM_REG("Tcam AD8", DRV_FTM_TCAM_AD8, 1024, 0x2640000, 0x2640000, 0x2640000);
    DRV_MEM_REG("Tcam AD9", DRV_FTM_TCAM_AD9, 1024, 0x2648000, 0x2648000, 0x2648000);
    DRV_MEM_REG("Tcam AD10", DRV_FTM_TCAM_AD10, 1024, 0x2650000, 0x2650000, 0x2650000);
    DRV_MEM_REG("Tcam AD11", DRV_FTM_TCAM_AD11, 1024, 0x2658000, 0x2658000, 0x2658000);
    DRV_MEM_REG("Tcam AD12", DRV_FTM_TCAM_AD12, 1024, 0x2660000, 0x2660000, 0x2660000);
    DRV_MEM_REG("Tcam AD13", DRV_FTM_TCAM_AD13, 1024, 0x2668000, 0x2668000, 0x2668000);
    DRV_MEM_REG("Tcam AD14", DRV_FTM_TCAM_AD14, 1024, 0x2670000, 0x2670000, 0x2670000);
    DRV_MEM_REG("Tcam AD15", DRV_FTM_TCAM_AD15, 2048, 0x2678000, 0x2678000, 0x2678000);
    DRV_MEM_REG("Tcam AD16", DRV_FTM_TCAM_AD16, 2048, 0x2680000, 0x2680000, 0x2680000);
    DRV_MEM_REG("Tcam AD17", DRV_FTM_TCAM_AD17, 2048, 0x2688000, 0x2688000, 0x2688000);
    DRV_MEM_REG("Tcam AD18", DRV_FTM_TCAM_AD18, 2048, 0x2690000, 0x2690000, 0x2690000);

    DRV_MEM_REG("LPM Tcam key0" , DRV_FTM_LPM_TCAM_KEY0, 1024, 0x4100000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam key1" , DRV_FTM_LPM_TCAM_KEY1, 1024, 0x4102000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam key2" , DRV_FTM_LPM_TCAM_KEY2, 1024, 0x4104000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam key3" , DRV_FTM_LPM_TCAM_KEY3, 1024, 0x4106000, 0x0, 0x0);

    DRV_MEM_REG("LPM Tcam key4" , DRV_FTM_LPM_TCAM_KEY4, 1024, 0x4120000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam key5" , DRV_FTM_LPM_TCAM_KEY5, 1024, 0x4122000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam key6" , DRV_FTM_LPM_TCAM_KEY6, 1024, 0x4124000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam key7" , DRV_FTM_LPM_TCAM_KEY7, 1024, 0x4126000, 0x0, 0x0);

    DRV_MEM_REG("LPM Tcam key8" , DRV_FTM_LPM_TCAM_KEY8, 2048, 0x4140000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam key9" , DRV_FTM_LPM_TCAM_KEY9, 2048, 0x4148000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam key10" , DRV_FTM_LPM_TCAM_KEY10, 2048, 0x4150000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam key11" , DRV_FTM_LPM_TCAM_KEY11, 2048, 0x4158000, 0x0, 0x0);


    DRV_MEM_REG("LPM Tcam Ad0" , DRV_FTM_LPM_TCAM_AD0, 1024, 0x4180000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad1" , DRV_FTM_LPM_TCAM_AD1, 1024, 0x4182000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad2" , DRV_FTM_LPM_TCAM_AD2, 1024, 0x4184000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad3" , DRV_FTM_LPM_TCAM_AD3, 1024, 0x4186000, 0x0, 0x0);

    DRV_MEM_REG("LPM Tcam Ad4" , DRV_FTM_LPM_TCAM_AD4, 1024, 0x4190000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad5" , DRV_FTM_LPM_TCAM_AD5, 1024, 0x4192000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad6" , DRV_FTM_LPM_TCAM_AD6, 1024, 0x4194000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad7" , DRV_FTM_LPM_TCAM_AD7, 1024, 0x4196000, 0x0, 0x0);

    DRV_MEM_REG("LPM Tcam Ad8" , DRV_FTM_LPM_TCAM_AD8, 2048, 0x41a0000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad9" , DRV_FTM_LPM_TCAM_AD9, 2048, 0x41a4000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad10" , DRV_FTM_LPM_TCAM_AD10, 2048, 0x41a8000, 0x0, 0x0);
    DRV_MEM_REG("LPM Tcam Ad11" , DRV_FTM_LPM_TCAM_AD11, 2048, 0x41ac000, 0x0, 0x0);

    DRV_MEM_REG("Queue Tcam"         , DRV_FTM_QUEUE_TCAM, 256, 0x00644000, 0xb1000000, 0x0);
    DRV_MEM_REG("Cid Tcam"           , DRV_FTM_CID_TCAM, 128, 0x014d3000, 0xb0000000, 0x0);


    return 0;
}



int32
sys_tsingma_ftm_get_edram_bitmap(uint8 lchip,
                                     uint8 sram_type,
                                     uint32* bit)
{

    switch (sram_type)
    {
        /* SelEdram  Done */
        case DRV_FTM_SRAM_TBL_LPM_LKP_KEY0:
        bit[DRV_FTM_SRAM0] = 0;
        bit[DRV_FTM_SRAM3] = 1;
        bit[DRV_FTM_SRAM6]  = 2;
        bit[DRV_FTM_SRAM8]  = 3;
        break;

        /* SelEdram  {edram0, edram3, edram6, edram7} Done */
        case DRV_FTM_SRAM_TBL_LPM_LKP_KEY1:
        bit[DRV_FTM_SRAM1] = 0;
        bit[DRV_FTM_SRAM4] = 1;
        bit[DRV_FTM_SRAM7]  = 2;
        bit[DRV_FTM_SRAM10] = 3;
        break;

        /* SelEdram  Done */
        case DRV_FTM_SRAM_TBL_LPM_LKP_KEY2:
        bit[DRV_FTM_SRAM0] = 0;
        bit[DRV_FTM_SRAM3] = 1;
        bit[DRV_FTM_SRAM8]  = 2;
        break;

        /* SelEdram  {edram0, edram3, edram6, edram7} Done */
        case DRV_FTM_SRAM_TBL_LPM_LKP_KEY3:
        bit[DRV_FTM_SRAM1] = 0;
        bit[DRV_FTM_SRAM4] = 1;
        bit[DRV_FTM_SRAM10] = 2;
        break;


        /* SelEdram  {edram0, edram1, edram2, edram3, edram4, edram5 } Done*/
        case DRV_FTM_SRAM_TBL_MAC_HASH_KEY:
        bit[DRV_FTM_SRAM0] = 0;
        bit[DRV_FTM_SRAM1] = 1;
        bit[DRV_FTM_SRAM2] = 2;
        bit[DRV_FTM_SRAM3] = 3;
        bit[DRV_FTM_SRAM4] = 4;
        bit[DRV_FTM_SRAM5] = 5;

        bit[DRV_FTM_SRAM23] = DRV_USW_FTM_EXTERN_MEM_ID_BASE;
        bit[DRV_FTM_SRAM24] = DRV_USW_FTM_EXTERN_MEM_ID_BASE+1;
        bit[DRV_FTM_SRAM25] = DRV_USW_FTM_EXTERN_MEM_ID_BASE+2;
        bit[DRV_FTM_SRAM26] = DRV_USW_FTM_EXTERN_MEM_ID_BASE+3;

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



        /* SelEdram  {edram11, edram12, edram13, edram14} Done*/
        case DRV_FTM_SRAM_TBL_DSMAC_AD:
        bit[DRV_FTM_SRAM11]   = 0;
        bit[DRV_FTM_SRAM12]   = 1;
        bit[DRV_FTM_SRAM13]   = 2;
        bit[DRV_FTM_SRAM14]   = 3;

        bit[DRV_FTM_SRAM27]   = DRV_USW_FTM_EXTERN_MEM_ID_BASE;
        bit[DRV_FTM_SRAM28]   = DRV_USW_FTM_EXTERN_MEM_ID_BASE+1;

        break;

        /* SelEdram  {edram11, edram12, edram13, edram14, edram16} Done*/
        case DRV_FTM_SRAM_TBL_DSIP_AD:
        bit[DRV_FTM_SRAM11]   = 0;
        bit[DRV_FTM_SRAM12]   = 1;
        bit[DRV_FTM_SRAM13]   = 2;
        bit[DRV_FTM_SRAM14]   = 3;
        bit[DRV_FTM_SRAM16]   = 4;

        bit[DRV_FTM_SRAM27]   = DRV_USW_FTM_EXTERN_MEM_ID_BASE;
        bit[DRV_FTM_SRAM28]   = DRV_USW_FTM_EXTERN_MEM_ID_BASE+1;
        break;


        /* SelEdram  {edram12, edram13, edram14, edram15, edram16} Done*/
        case DRV_FTM_SRAM_TBL_USERID_AD:
        bit[DRV_FTM_SRAM12]   = 0;
        bit[DRV_FTM_SRAM13]   = 1;
        bit[DRV_FTM_SRAM14]   = 2;
        bit[DRV_FTM_SRAM15]   = 3;
        bit[DRV_FTM_SRAM16]   = 4;
        break;


        /* SelEdram  {edram11, edram15, edram16} Done*/
        case DRV_FTM_SRAM_TBL_USERID_AD1:
        bit[DRV_FTM_SRAM11]   = 0;
        bit[DRV_FTM_SRAM15]   = 1;
        bit[DRV_FTM_SRAM16]   = 2;
        break;


        /* SelEdram  {edram16, edram18, edram19, edram21 } Done*/
        case DRV_FTM_SRAM_TBL_NEXTHOP:
        bit[DRV_FTM_SRAM17] = 0;
        bit[DRV_FTM_SRAM18] = 1;
        bit[DRV_FTM_SRAM19] = 2;
        bit[DRV_FTM_SRAM21] = 3;

        bit[DRV_FTM_SRAM29]   = DRV_USW_FTM_EXTERN_MEM_ID_BASE;


        break;

        /* SelEdram  {edram16, edram18, edram19, edram20, edram21 } Done*/
        case DRV_FTM_SRAM_TBL_MET:
        bit[DRV_FTM_SRAM17] = 0;
        bit[DRV_FTM_SRAM18] = 1;
        bit[DRV_FTM_SRAM19] = 2;
        bit[DRV_FTM_SRAM22] = 3;

        break;


        /* SelEdram  {edram18, edram19, edram20, edram21 } Done*/
    case DRV_FTM_SRAM_TBL_EDIT:
        bit[DRV_FTM_SRAM19] = 0;
        bit[DRV_FTM_SRAM21] = 1;
        bit[DRV_FTM_SRAM22] = 2;
        break;



        /* SelEdram  {edram17, edram18, edram19, edram20, edram21 } Done*/
        case DRV_FTM_SRAM_TBL_FWD:
        bit[DRV_FTM_SRAM18] = 0;
        bit[DRV_FTM_SRAM19] = 1;
        bit[DRV_FTM_SRAM20] = 2;
        bit[DRV_FTM_SRAM21] = 3;
        bit[DRV_FTM_SRAM22] = 3;
        break;


         /* SelEdram  {edram1, edram4} Done*/
        case DRV_FTM_SRAM_TBL_OAM_LM:
        bit[DRV_FTM_SRAM2] = 0;
        break;

         /* SelEdram  {edram1, edram4} Done*/
        case DRV_FTM_SRAM_TBL_OAM_MEP:
        bit[DRV_FTM_SRAM2] = 0;
        break;

         /* SelEdram  {edram1, edram4} Done*/
        case DRV_FTM_SRAM_TBL_OAM_MA:
        bit[DRV_FTM_SRAM2] = 0;
        break;

         /* SelEdram  {edram1, edram4} Done*/
        case DRV_FTM_SRAM_TBL_OAM_MA_NAME:
        bit[DRV_FTM_SRAM2] = 0;
        break;


            /* NONE*/
        case DRV_FTM_SRAM_TBL_FLOW_AD:
            bit[DRV_FTM_SRAM11]   = 0;
            break;


        /* NONE */
        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
        bit[DRV_FTM_SRAM9] = 0;
        bit[DRV_FTM_SRAM10] = 1;
        break;

            /* NONE */
        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
            bit[DRV_FTM_SRAM0] = 0;
            break;

        /* NONE */
        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
        bit[DRV_FTM_SRAM3] = 0;
        bit[DRV_FTM_SRAM4] = 1;
        break;

        /* NONE */
        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
        bit[DRV_FTM_SRAM2] = 0;
        bit[DRV_FTM_SRAM6] = 1;
        bit[DRV_FTM_SRAM7] = 2;
        bit[DRV_FTM_SRAM8] = 3;
        bit[DRV_FTM_SRAM9] = 4;
        break;

        /* NONE */
        case DRV_FTM_SRAM_TBL_USERID1_HASH_KEY:
        bit[DRV_FTM_SRAM5] = 0;
        bit[DRV_FTM_SRAM8] = 1;
        bit[DRV_FTM_SRAM9] = 2;
        break;

        /* NONE */
        case DRV_FTM_SRAM_TBL_MPLS_HASH_KEY:
        bit[DRV_FTM_MIXED0] = 0;
        bit[DRV_FTM_MIXED1] = 1;
        break;

        case DRV_FTM_SRAM_TBL_MPLS_HASH_AD:
        bit[DRV_FTM_MIXED2] = 0;
        break;


        /* NONE */
        case DRV_FTM_SRAM_TBL_GEM_PORT:
        bit[DRV_FTM_MIXED2] = 0;
        bit[DRV_FTM_MIXED0] = 1;
        bit[DRV_FTM_MIXED1] = 2;

        break;



        default:
           break;
    }


    return DRV_E_NONE;
}


STATIC int32
sys_tsingma_ftm_get_hash_type(uint8 lchip, uint8 sram_type, uint32 mem_id, uint8 couple, uint32 *poly)
{
    uint8 extra = 0;

    *poly = 0;

    switch (sram_type)
    {
        case DRV_FTM_SRAM_TBL_MAC_HASH_KEY:
        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:
            switch(mem_id)
            {
            case DRV_FTM_SRAM0:
                *poly = couple ? 6: 3;
                break;
            case DRV_FTM_SRAM1:
                *poly = couple ? 7: 4;
                break;
            case DRV_FTM_SRAM2:
                *poly = couple ? 8: 5;
                break;
            case DRV_FTM_SRAM3:
                *poly = couple ? 3: 1;
                break;
            case DRV_FTM_SRAM4:
                *poly = couple ? 4: 2;
                break;
            case DRV_FTM_SRAM5:
                *poly = couple ? 1: 0;
                break;

            case DRV_FTM_SRAM23:
                *poly = couple ? 10: 6;
                break;
            case DRV_FTM_SRAM24:
                *poly = couple ? 11: 7;
                break;
            case DRV_FTM_SRAM25:
                *poly = couple ? 12: 8;
                break;
            case DRV_FTM_SRAM26:
                *poly = couple ? 13: 9;
                break;

            }

            break;

        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY: /*TsingMa no couple, 1R*/
            switch(mem_id)
            {
            case DRV_FTM_SRAM9:
                *poly = couple ? 0: 0;
                break;
            case DRV_FTM_SRAM10:
                *poly = couple ? 1: 1;
                break;
            }
            break;

        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
            switch(mem_id)
            {
            case DRV_FTM_SRAM0:
                *poly = couple ? 0: 0;
                break;

            }
            break;

        case DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY:
            break;

        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
            switch(mem_id)
            {
            case DRV_FTM_SRAM2:
                *poly = couple ? 0: 1;
                break;
            case DRV_FTM_SRAM6:
                *poly = couple ? 2: 2;
                break;
            case DRV_FTM_SRAM7:
                *poly = couple ? 3: 3;
                break;
            case DRV_FTM_SRAM8:
                *poly = couple ? 4: 3;
                break;
            case DRV_FTM_SRAM9:
                *poly = couple ? 5: 4;
                break;
            }
            break;

        case DRV_FTM_SRAM_TBL_USERID1_HASH_KEY:
            switch(mem_id)
            {
            case DRV_FTM_SRAM5:
                *poly = couple ? 3: 0;
                break;
            case DRV_FTM_SRAM8:
                *poly = couple ? 1: 1;
                break;
            case DRV_FTM_SRAM9:
                *poly = couple ? 2: 2;
                break;

            }
            break;

        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:

            switch(mem_id)
            {
            case DRV_FTM_SRAM3:
                *poly = couple ?(extra?3:2): (extra?1:0);
                break;
            case DRV_FTM_SRAM4:
                *poly = couple ? (extra?4:3): (extra?2:1);
                break;
            }
            break;

    }
    return DRV_E_NONE;
}



int32 drv_tsingma_get_mcu_lock_id(uint8 lchip, tbls_id_t tbl_id, uint8* p_mcu_id, uint32* p_lock_id)
{
    uint8  mcu_id  = 0xff;
    uint32 lock_id = DRV_MCU_LOCK_NONE;

    if((NULL == p_lock_id) || (NULL == p_mcu_id))
    {
        return DRV_E_INVALID_PTR;
    }

    switch(tbl_id)
    {
        case SharedPcsSoftRst0_t:
        case SharedPcsSoftRst1_t:
        case QsgmiiPcsSoftRst0_t:
        case QsgmiiPcsSoftRst1_t:
        case QsgmiiPcsSoftRst2_t:
        case QsgmiiPcsSoftRst3_t:
        case QsgmiiPcsSoftRst4_t:
        case QsgmiiPcsSoftRst5_t:
        case QsgmiiPcsSoftRst6_t:
        case QsgmiiPcsSoftRst7_t:
        case UsxgmiiPcsSoftRst0_t:
        case UsxgmiiPcsSoftRst1_t:
        case UsxgmiiPcsSoftRst2_t:
        case UsxgmiiPcsSoftRst3_t:
        case UsxgmiiPcsSoftRst4_t:
        case UsxgmiiPcsSoftRst5_t:
        case UsxgmiiPcsSoftRst6_t:
        case UsxgmiiPcsSoftRst7_t:
            lock_id = DRV_MCU_LOCK_PCS_RESET;
            mcu_id  = 0;
            break;
        case SharedPcsSoftRst2_t:
        case SharedPcsSoftRst3_t:
        case QsgmiiPcsSoftRst8_t:
        case QsgmiiPcsSoftRst9_t:
        case QsgmiiPcsSoftRst10_t:
        case QsgmiiPcsSoftRst11_t:
        case UsxgmiiPcsSoftRst8_t:
        case UsxgmiiPcsSoftRst9_t:
        case UsxgmiiPcsSoftRst10_t:
        case UsxgmiiPcsSoftRst11_t:
            lock_id = DRV_MCU_LOCK_PCS_RESET;
            mcu_id  = 1;
            break;
        case SharedPcsSoftRst4_t:
        case SharedPcsSoftRst5_t:
            lock_id = DRV_MCU_LOCK_PCS_RESET;
            mcu_id  = 2;
            break;
        case SharedPcsSoftRst6_t:
        case SharedPcsSoftRst7_t:
            lock_id = DRV_MCU_LOCK_PCS_RESET;
            mcu_id  = 3;
            break;
        default:
            break;
    }

    *p_mcu_id  = mcu_id;
    *p_lock_id = lock_id;

    return DRV_E_NONE;
}

int32 drv_tsingma_get_mcu_addr(uint8 mcu_id, uint32* mutex_data_addr, uint32* mutex_mask_addr)
{
    if((NULL == mutex_data_addr) || (NULL == mutex_mask_addr))
    {
        return DRV_E_INVALID_PTR;
    }

    switch(mcu_id)
    {
        case 0:
            *mutex_data_addr = TSINGMA_MCU_MUTEX0_CPU_DATA_0;
            *mutex_mask_addr = TSINGMA_MCU_MUTEX0_CPU_MASK_0;
            break;
        case 1:
            *mutex_data_addr = TSINGMA_MCU_MUTEX0_CPU_DATA_1;
            *mutex_mask_addr = TSINGMA_MCU_MUTEX0_CPU_MASK_1;
            break;
        case 2:
            *mutex_data_addr = TSINGMA_MCU_MUTEX0_CPU_DATA_2;
            *mutex_mask_addr = TSINGMA_MCU_MUTEX0_CPU_MASK_2;
            break;
        case 3:
            *mutex_data_addr = TSINGMA_MCU_MUTEX0_CPU_DATA_3;
            *mutex_mask_addr = TSINGMA_MCU_MUTEX0_CPU_MASK_3;
            break;
        default:
            return DRV_E_INVALID_TBL;
    }
    return DRV_E_NONE;
}

int32
drv_tsingma_chip_read_hss15g(uint8 lchip, uint8 hss_id, uint32 addr, uint16* p_data)
{
    uint8 rslt_lock = TRUE;
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 value = 0;
    uint32 timeout = 0x6400;
    uint8 acc_id = (uint8)((addr>>8) & 0xff);
    uint8 addr_raw = (uint8)(addr & 0xff);
    Hss12GRegAccCtl0_m acc_ctl;
    Hss12GRegAccResult0_m acc_rslt;

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    DRV_PTR_VALID_CHECK(p_data);
    DRV_INIT_CHECK(lchip);

    sal_memset(&acc_ctl, 0, sizeof(Hss12GRegAccCtl0_m));
    sal_memset(&acc_rslt, 0, sizeof(Hss12GRegAccResult0_m));
    
    ret = drv_usw_mcu_lock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
    if(DRV_E_NONE != ret)
    {
        DRV_DBG_INFO("Get lock error (read 12G serdes reg)! hss_id=%u, addr=0x%x\n", hss_id, addr);
        return ret;
    }

    /* 1. write Hss12GRegAccCtl to run read action */
    tbl_id = Hss12GRegAccCtl0_t + hss_id;
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, Hss12GRegAccCtl0_hssAccValid_f, &value, &acc_ctl);
    value = acc_id;
    DRV_IOW_FIELD(lchip, tbl_id, Hss12GRegAccCtl0_hssAccId_f, &value, &acc_ctl);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, Hss12GRegAccCtl0_hssAccIsRead_f, &value, &acc_ctl);
    value = addr_raw;
    DRV_IOW_FIELD(lchip, tbl_id, Hss12GRegAccCtl0_hssAccAddr_f, &value, &acc_ctl);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &acc_ctl);
    if (ret < 0)
    {
        drv_usw_mcu_unlock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    /* 2. read Hss12GRegAccResult to get return value */
    value = 0;
    tbl_id = Hss12GRegAccResult0_t + hss_id;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    while(rslt_lock && (--timeout))
    {
        ret = DRV_IOCTL(lchip, 0, cmd, &acc_rslt);
        if (ret < 0)
        {
            drv_usw_mcu_unlock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
        DRV_IOR_FIELD(lchip, tbl_id, Hss12GRegAccResult0_hssAccAck_f, &value, &acc_rslt);
        if(1 == value)
        {
            rslt_lock = FALSE;
        }
    }
    if(TRUE == rslt_lock)
    {
        drv_usw_mcu_unlock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
        return DRV_E_DATAPATH_READ_CHIP_FAIL;
    }
    DRV_IOR_FIELD(lchip, tbl_id, Hss12GRegAccResult0_hssAccAckData_f, &value, &acc_rslt);
    *p_data = (uint16)value;

    /* 3. clear Hss12GRegAccResultx.hssAccAck */
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Hss12GRegAccResult0_hssAccAck_f, &value, &acc_rslt);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &acc_rslt);
    if (ret < 0)
    {
        drv_usw_mcu_unlock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    ret = drv_usw_mcu_unlock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
    if (DRV_E_NONE != ret)
    {
        DRV_DBG_INFO("Get unlock error (read 12G serdes reg)! hss_id=%u, addr=0x%x\n", hss_id, addr);
        return ret;
    }

    return DRV_E_NONE;
}

/**
 @brief access hss15g control register
*/
int32
drv_tsingma_chip_write_hss15g(uint8 lchip, uint8 hss_id, uint32 addr, uint16 data)
{
    uint8 rslt_lock = TRUE;
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 value = 0;
    uint32 timeout = 0x6400;
    uint8 acc_id = (uint8)((addr>>8) & 0xff);
    uint8 addr_raw = (uint8)(addr & 0xff);
    Hss12GRegAccCtl0_m acc_ctl;
    Hss12GRegAccResult0_m acc_rslt;

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    DRV_INIT_CHECK(lchip);

    sal_memset(&acc_ctl, 0, sizeof(Hss12GRegAccCtl0_m));
    sal_memset(&acc_rslt, 0, sizeof(Hss12GRegAccResult0_m));

    ret = drv_usw_mcu_lock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
    if(DRV_E_NONE != ret)
    {
        DRV_DBG_INFO("Get lock error (write 12G serdes reg)! hss_id=%u, addr=0x%x, data=%u\n", hss_id, addr, data);
        return ret;
    }

    /* 1. write Hss12GRegAccCtl to run write action */
    tbl_id = Hss12GRegAccCtl0_t + hss_id;
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, Hss12GRegAccCtl0_hssAccValid_f, &value, &acc_ctl);
    value = acc_id;
    DRV_IOW_FIELD(lchip, tbl_id, Hss12GRegAccCtl0_hssAccId_f, &value, &acc_ctl);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Hss12GRegAccCtl0_hssAccIsRead_f, &value, &acc_ctl);
    value = data;
    DRV_IOW_FIELD(lchip, tbl_id, Hss12GRegAccCtl0_hssAccWdata_f, &value, &acc_ctl);
    value = addr_raw;
    DRV_IOW_FIELD(lchip, tbl_id, Hss12GRegAccCtl0_hssAccAddr_f, &value, &acc_ctl);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &acc_ctl);
    if (ret < 0)
    {
        drv_usw_mcu_unlock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    /* 2. read Hss12GRegAccResult to get return value */
    value = 0;
    tbl_id = Hss12GRegAccResult0_t + hss_id;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    while(rslt_lock && (--timeout))
    {
        ret = DRV_IOCTL(lchip, 0, cmd, &acc_rslt);
        if (ret < 0)
        {
            drv_usw_mcu_unlock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
        DRV_IOR_FIELD(lchip, tbl_id, Hss12GRegAccResult0_hssAccAck_f, &value, &acc_rslt);
        if(1 == value)
        {
            rslt_lock = FALSE;
        }
    }
    if(TRUE == rslt_lock)
    {
        drv_usw_mcu_unlock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
        return DRV_E_DATAPATH_READ_CHIP_FAIL;
    }

    /* 3. clear Hss12GRegAccResultx.hssAccAck */
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Hss12GRegAccResult0_hssAccAck_f, &value, &acc_rslt);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &acc_rslt);
    if (ret < 0)
    {
        drv_usw_mcu_unlock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    ret = drv_usw_mcu_unlock(lchip, DRV_MCU_LOCK_HSS15G_REG, hss_id);
    if (DRV_E_NONE != ret)
    {
        DRV_DBG_INFO("Get unlock error (write 12G serdes reg)! hss_id=%u, addr=0x%x, data=%u\n", hss_id, addr, data);
        return ret;
    }

    return DRV_E_NONE;
}

/**
 @brief access hss28g control register
*/
int32
drv_tsingma_chip_read_hss28g(uint8 lchip, uint8 hss_id, uint32 addr, uint16* p_data)
{
    uint8 rslt_lock = TRUE;
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 value = 0;
    uint32 timeout = 0x6400;
    Hss28GRegAccCtl_m acc_ctl;
    Hss28GRegAccResult_m acc_rslt;

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    DRV_PTR_VALID_CHECK(p_data);
    DRV_INIT_CHECK(lchip);

    sal_memset(&acc_ctl, 0, sizeof(Hss28GRegAccCtl_m));
    sal_memset(&acc_rslt, 0, sizeof(Hss28GRegAccResult_m));

    /* 1. write Hss28GRegAccCtl to run read action */
    tbl_id = Hss28GRegAccCtl_t;
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccCtl_hssAccValid_f, &value, &acc_ctl);
    value = hss_id;
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccCtl_hssAccId_f, &value, &acc_ctl);
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccCtl_hssAccIsRead_f, &value, &acc_ctl);
    value = addr;
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccCtl_hssAccAddr_f, &value, &acc_ctl);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &acc_ctl);
    if (ret < 0)
    {
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    /* 2. read Hss28GRegAccResult to get return value */
    value = 0;
    tbl_id = Hss28GRegAccResult_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    while(rslt_lock && (--timeout))
    {
        ret = DRV_IOCTL(lchip, 0, cmd, &acc_rslt);
        if (ret < 0)
        {
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
        DRV_IOR_FIELD(lchip, tbl_id, Hss28GRegAccResult_hssAccAck_f, &value, &acc_rslt);
        if(1 == value)
        {
            rslt_lock = FALSE;
        }
    }

    DRV_IOR_FIELD(lchip, tbl_id, Hss28GRegAccResult_hssAccAckError_f, &value, &acc_rslt);
    if((1 == value) || (TRUE == rslt_lock))
    {
        return DRV_E_DATAPATH_READ_CHIP_FAIL;
    }
    DRV_IOR_FIELD(lchip, tbl_id, Hss28GRegAccResult_hssAccAckData_f, &value, &acc_rslt);
    *p_data = (uint16)value;

    /* 3. clear Hss12GRegAccResultx.hssAccAck */
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccResult_hssAccAck_f, &value, &acc_rslt);
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccResult_hssAccAckError_f, &value, &acc_rslt);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &acc_rslt);
    if (ret < 0)
    {
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    return DRV_E_NONE;
}

/**
 @brief access hss28g control register
*/
int32
drv_tsingma_chip_write_hss28g(uint8 lchip, uint8 hss_id, uint32 addr, uint16 data)
{
    uint8  rslt_lock = TRUE;
    int32  ret       = 0;
    uint32 cmd       = 0;
    uint32 tbl_id    = 0;
    uint32 value     = 0;
    uint32 timeout   = 0x6400;
    Hss28GRegAccCtl_m acc_ctl;
    Hss28GRegAccResult_m acc_rslt;

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    DRV_INIT_CHECK(lchip);

    sal_memset(&acc_ctl, 0, sizeof(Hss28GRegAccCtl_m));
    sal_memset(&acc_rslt, 0, sizeof(Hss28GRegAccResult_m));

    /* 1. write Hss28GRegAccCtl to run write action */
    tbl_id = Hss28GRegAccCtl_t;
    value = 1;
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccCtl_hssAccValid_f, &value, &acc_ctl);
    value = hss_id;
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccCtl_hssAccId_f, &value, &acc_ctl);
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccCtl_hssAccIsRead_f, &value, &acc_ctl);
    value = data;
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccCtl_hssAccWdata_f, &value, &acc_ctl);
    value = addr;
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccCtl_hssAccAddr_f, &value, &acc_ctl);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &acc_ctl);
    if (ret < 0)
    {
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    /* 2. read Hss28GRegAccResult to get return value */
    value = 0;
    tbl_id = Hss28GRegAccResult_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    while(rslt_lock && (--timeout))
    {
        ret = DRV_IOCTL(lchip, 0, cmd, &acc_rslt);
        if (ret < 0)
        {
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
        DRV_IOR_FIELD(lchip, tbl_id, Hss28GRegAccResult_hssAccAck_f, &value, &acc_rslt);
        if(1 == value)
        {
            rslt_lock = FALSE;
        }
    }
    DRV_IOR_FIELD(lchip, tbl_id, Hss28GRegAccResult_hssAccAckError_f, &value, &acc_rslt);
    if((1 == value) || (TRUE == rslt_lock))
    {
        return DRV_E_DATAPATH_READ_CHIP_FAIL;
    }

    /* 3. clear Hss28GRegAccResultx.hssAccAck */
    value = 0;
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccResult_hssAccAck_f, &value, &acc_rslt);
    DRV_IOW_FIELD(lchip, tbl_id, Hss28GRegAccResult_hssAccAckError_f, &value, &acc_rslt);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &acc_rslt);
    if (ret < 0)
    {
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    return DRV_E_NONE;
}

int32
drv_enum_init_tsingma(uint8 lchip)
{
    DRV_ENUM(DRV_TCAMKEYTYPE_MACKEY_160)          = 0x0;
    DRV_ENUM(DRV_TCAMKEYTYPE_L3KEY_160)             = 0x1;
    DRV_ENUM(DRV_TCAMKEYTYPE_L3KEY_320)             = 0x2;
    DRV_ENUM(DRV_TCAMKEYTYPE_IPV6KEY_320)          = 0x3;
    DRV_ENUM(DRV_TCAMKEYTYPE_IPV6KEY_640)          = 0x4;
    DRV_ENUM(DRV_TCAMKEYTYPE_MACL3KEY_320)       = 0x6;
    DRV_ENUM(DRV_TCAMKEYTYPE_MACL3KEY_640)        = 0xe;
    DRV_ENUM(DRV_TCAMKEYTYPE_MACIPV6KEY_640)     = 0x7;
    DRV_ENUM(DRV_TCAMKEYTYPE_CIDKEY_160)            = 0x8;
    DRV_ENUM(DRV_TCAMKEYTYPE_SHORTKEY_80)         = 0x9;
    DRV_ENUM(DRV_TCAMKEYTYPE_FORWARDKEY_320)   = 0xa;
    DRV_ENUM(DRV_TCAMKEYTYPE_FORWARDKEY_640)   = 0xb;
    DRV_ENUM(DRV_TCAMKEYTYPE_COPPKEY_320)         = 0xc;
    DRV_ENUM(DRV_TCAMKEYTYPE_COPPKEY_640)         = 0xd;
    DRV_ENUM(DRV_TCAMKEYTYPE_UDFKEY_320)           = 0x5;

    DRV_ENUM(DRV_SCL_KEY_TYPE_MACKEY160)            = 0x0;
    DRV_ENUM(DRV_SCL_KEY_TYPE_MACL3KEY320)         = 0x3;
    DRV_ENUM(DRV_SCL_KEY_TYPE_L3KEY160)               = 0x1;
    DRV_ENUM(DRV_SCL_KEY_TYPE_IPV6KEY320)            = 0x2;
    DRV_ENUM(DRV_SCL_KEY_TYPE_MACIPV6KEY640)      = 0x7;
    DRV_ENUM(DRV_SCL_KEY_TYPE_RESOLVE_CONFLICT) = 0x5;
    DRV_ENUM(DRV_SCL_KEY_TYPE_UDFKEY160)             = 0x6;
    DRV_ENUM(DRV_SCL_KEY_TYPE_UDFKEY320)             = 0x4;
    DRV_ENUM(DRV_SCL_KEY_TYPE_MASK)                    = 0x7;


    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_REGULAR_PORT)      = 0x0;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITHOUT_TUNNEL)    = 0x7;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2)           = 0x8;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV4)  = 0xF;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV6)  = 0xF;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_IPV4)         = 0xF;
    DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_IPV6)         = 0xF;

    DRV_ENUM(DRV_DMA_PACKET_RX0_CHAN_ID) = 0;
    DRV_ENUM(DRV_DMA_PACKET_RX1_CHAN_ID) = 1;
    DRV_ENUM(DRV_DMA_PACKET_RX2_CHAN_ID) = 2;
    DRV_ENUM(DRV_DMA_PACKET_RX3_CHAN_ID) = 3;
    DRV_ENUM(DRV_DMA_PACKET_TX0_CHAN_ID) = 4;
    DRV_ENUM(DRV_DMA_PACKET_TX1_CHAN_ID) = 5;
    DRV_ENUM(DRV_DMA_PACKET_TX2_CHAN_ID) = 6;
    DRV_ENUM(DRV_DMA_PACKET_TX3_CHAN_ID) = 7;
    DRV_ENUM(DRV_DMA_TBL_RD_CHAN_ID)     = 8;
    DRV_ENUM(DRV_DMA_PORT_STATS_CHAN_ID) = 9;
    DRV_ENUM(DRV_DMA_FLOW_STATS_CHAN_ID) = 10;
    DRV_ENUM(DRV_DMA_REG_MAX_CHAN_ID)    = 11;
    DRV_ENUM(DRV_DMA_TBL_RD1_CHAN_ID)    = 12;
    DRV_ENUM(DRV_DMA_TBL_RD2_CHAN_ID)    = 13;
    DRV_ENUM(DRV_DMA_TBL_WR_CHAN_ID)     = 14;
    DRV_ENUM(DRV_DMA_LEARNING_CHAN_ID)   = 15;
    DRV_ENUM(DRV_DMA_HASHKEY_CHAN_ID)    = 16;
    DRV_ENUM(DRV_DMA_IPFIX_CHAN_ID)      = 17;
    DRV_ENUM(DRV_DMA_SDC_CHAN_ID)        = 18;
    DRV_ENUM(DRV_DMA_MONITOR_CHAN_ID)    = 19;
    DRV_ENUM(DRV_DMA_TCAM_SCAN_DESC_NUM) = 37;

    DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA) = 0x37;
    DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2) = 0x38;

    return 0;
}

int32
drv_tbls_list_init_tsingma(uint8 lchip)
{

    p_drv_master[lchip]->p_tbl_ext_info = (tables_ext_info_t*)mem_malloc(MEM_SYSTEM_MODULE, MaxTblId_t*sizeof(tables_ext_info_t));
    if(!p_drv_master[lchip]->p_tbl_ext_info)
    {
        return -1;
    }

    sal_memset(p_drv_master[lchip]->p_tbl_ext_info, 0, MaxTblId_t*sizeof(tables_ext_info_t));
    p_drv_master[lchip]->p_tbl_info = drv_tm_tbls_list;

    p_drv_master[lchip]->p_mem_info = drv_tm_mem;

    p_drv_master[lchip]->drv_ecc_data.p_intr_tbl = drv_ecc_tm_err_intr_tbl;
    p_drv_master[lchip]->drv_ecc_data.p_sbe_cnt  = drv_ecc_tm_sbe_cnt;
    p_drv_master[lchip]->drv_ecc_data.p_scan_tcam_tbl = drv_ecc_tm_scan_tcam_tbl;

    drv_mem_init_tsingma(lchip);


    p_drv_master[lchip]->drv_mem_get_edram_bitmap = sys_tsingma_ftm_get_edram_bitmap;
    p_drv_master[lchip]->drv_mem_get_hash_type = sys_tsingma_ftm_get_hash_type;

    p_drv_master[lchip]->drv_get_mcu_lock_id = drv_tsingma_get_mcu_lock_id;
    p_drv_master[lchip]->drv_get_mcu_addr = drv_tsingma_get_mcu_addr;

    p_drv_master[lchip]->drv_chip_read_hss15g = drv_tsingma_chip_read_hss15g;
    p_drv_master[lchip]->drv_chip_write_hss15g = drv_tsingma_chip_write_hss15g;
    p_drv_master[lchip]->drv_chip_read_hss28g = drv_tsingma_chip_read_hss28g;
    p_drv_master[lchip]->drv_chip_write_hss28g = drv_tsingma_chip_write_hss28g;

    p_drv_master[lchip]->p_enum_value = drv_tm_enum;
    drv_enum_init_tsingma(lchip);

    return 0;
}


int32
drv_tbls_list_deinit_tsingma(uint8 lchip)
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



