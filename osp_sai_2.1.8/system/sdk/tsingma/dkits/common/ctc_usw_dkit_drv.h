#ifndef _CTC_GOLDENGATE_DKIT_DRV_H_
#define _CTC_GOLDENGATE_DKIT_DRV_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "sal.h"

/* module data stucture */
struct dkit_modules_s
{
    char        *master_name;
    char        *module_name;
};
typedef struct dkit_modules_s dkit_modules_t;

/* module id check macro define */
enum dkit_modules_id_e
{
   BufRetrv_mod,                                                  /*       0 */
   BufStore_mod,                                                  /*       1 */
   CgPcs_mod,                                                     /*       2 */
   Cgmac_mod,                                                     /*       3 */
   DSANDTCAM_mod,                                                 /*       4 */
   DmaCtl_mod,                                                    /*       5 */
   DsAging_mod,                                                   /*       6 */
   DynamicDsAd_mod,                                               /*       7 */
   DynamicDsHash_mod,                                             /*       8 */
   DynamicDsShare_mod,                                            /*       9 */
   EgrOamHash_mod,                                                /*      10 */
   EpeAclOam_mod,                                                 /*      11 */
   EpeHdrAdj_mod,                                                 /*      12 */
   EpeHdrEdit_mod,                                                /*      13 */
   EpeHdrProc_mod,                                                /*      14 */
   EpeNextHop_mod,                                                /*      15 */
   FibAcc_mod,                                                    /*      16 */
   FibEngine_mod,                                                 /*      17 */
   FlowAcc_mod,                                                   /*      18 */
   FlowHash_mod,                                                  /*      19 */
   FlowTcam_mod,                                                  /*      20 */
   GlobalStats_mod,                                               /*      21 */
   Hss15GUnitWrapper_mod,                                         /*      22 */
   Hss28GUnitWrapper_mod,                                         /*      23 */
   HssAnethWrapper_mod,                                           /*      24 */
   I2CMaster_mod,                                                 /*      25 */
   IpeForward_mod,                                                /*      26 */
   IpeHdrAdj_mod,                                                 /*      27 */
   IpeIntfMap_mod,                                                /*      28 */
   IpeLkupMgr_mod,                                                /*      29 */
   IpePktProc_mod,                                                /*      30 */
   LpmTcamIp_mod,                                                 /*      31 */
   LpmTcamNat_mod,                                                /*      32 */
   MacLed_mod,                                                    /*      33 */
   Mdio_mod,                                                      /*      34 */
   MetFifo_mod,                                                   /*      35 */
   NetRx_mod,                                                     /*      36 */
   NetTx_mod,                                                     /*      37 */
   OamAutoGenPkt_mod,                                             /*      38 */
   OamFwd_mod,                                                    /*      39 */
   OamParser_mod,                                                 /*      40 */
   OamProc_mod,                                                   /*      41 */
   OobFc_mod,                                                     /*      42 */
   Parser_mod,                                                    /*      43 */
   PbCtl_mod,                                                     /*      44 */
   PcieIntf_mod,                                                  /*      45 */
   PktProcDs_mod,                                                 /*      46 */
   Policing_mod,                                                  /*      47 */
   QMgrDeqSlice_mod,                                              /*      48 */
   QMgrEnq_mod,                                                   /*      49 */
   QMgrMsgStore_mod,                                              /*      50 */
   RefTrigger_mod,                                                /*      51 */
   RlmAdCtl_mod,                                                  /*      52 */
   RlmBrPbCtl_mod,                                                /*      53 */
   RlmBsCtl_mod,                                                  /*      54 */
   RlmCsCtl_mod,                                                  /*      55 */
   RlmDeqCtl_mod,                                                 /*      56 */
   RlmEnqCtl_mod,                                                 /*      57 */
   RlmEpeCtl_mod,                                                 /*      58 */
   RlmHashCtl_mod,                                                /*      59 */
   RlmHsCtl_mod,                                                  /*      60 */
   RlmIpeCtl_mod,                                                 /*      61 */
   RlmNetCtl_mod,                                                 /*      62 */
   RlmShareDsCtl_mod,                                             /*      63 */
   RlmShareTcamCtl_mod,                                           /*      64 */
   ShareDlb_mod,                                                  /*      65 */
   ShareEfd_mod,                                                  /*      66 */
   ShareStormCtl_mod,                                             /*      67 */
   SharedPcs_mod,                                                 /*      68 */
   SupClockTree_mod,                                              /*      69 */
   SupCtl_mod,                                                    /*      70 */
   SupMacro_mod,                                                  /*      71 */
   TsEngineRef_mod,                                               /*      72 */
   TsEngine_mod,                                                  /*      73 */
   UserIdHash_mod,                                                /*      74 */
   Xqmac_mod,                                                     /*      75 */
   _DynamicDsAd_mod,                                              /*      76 */
   _DynamicDsHash_mod,                                            /*      77 */
   _DynamicDsShare_mod,                                           /*      78 */
   _FlowTcam_mod,                                                 /*      79 */
   _LpmTcamIp_mod,                                                /*      80 */
   _LpmTcamNat_mod,                                               /*      81 */
   _SdkCmodelBus_mod,                                             /*      82 */
   Mod_IPE,                                                      /*      83 */
   Mod_EPE,                                                      /*      84 */
   Mod_BSR,                                                      /*      85 */
   Mod_SHARE,                                                    /*      86 */
   Mod_MISC,                                                     /*      87 */
   Mod_RLM,                                                      /*      88 */
   Mod_MAC,                                                      /*      89 */
   Mod_OAM,                                                      /*      90 */
   Mod_ALL,                                                      /*      91 */

   MaxModId_m,
};
typedef enum dkit_modules_id_e dkit_mod_id_t;


#ifdef __cplusplus
}
#endif

#endif


