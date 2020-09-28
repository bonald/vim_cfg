/**
 @file ctc_usw_interrupt.c

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-10-23

 @version v2.0

 This file define sys functions

*/

/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "dal.h"
#include "ctc_error.h"
#include "ctc_macro.h"
#include "ctc_interrupt.h"
#include "ctc_usw_interrupt.h"
#include "sys_usw_chip.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_interrupt_priv.h"
#include "sys_usw_dmps.h"

#include "drv_api.h"
#include "usw/include/drv_chip_ctrl.h"

#include "usw/include/drv_common.h"
/****************************************************************************
*
* Defines and Macros
*
*****************************************************************************/
typedef struct sys_intr_irq_info_s
{
    uint8 irq;
    uint8 irq_idx;
    uint8 prio;
    uint8 group[SYS_USW_MAX_INTR_GROUP] ;
    uint8 group_count;
}
sys_intr_irq_info_t;

#define SYS_INTR_LOW_FATAL_MAX  9
#define SYS_INTR_LOW_NORMAL_MAX  66
#define SYS_INTR_INIT_CHECK(lchip)           \
    do                                    \
    {                                    \
        if(!g_usw_intr_master[lchip].init_done)  \
        {                                \
            SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
       \
        }                                \
    }while(0)

extern int32
sys_usw_intr_isr_sup_fatal(uint8 lchip, uint32 intr, void* p_data);

extern int32
sys_usw_intr_isr_sup_normal(uint8 lchip, uint32 intr, void* p_data);
extern int32
sys_usw_interrupt_reset_hw_cb(uint8 lchip, void* p_user_data);
/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/
sys_intr_master_t g_usw_intr_master[CTC_MAX_LOCAL_CHIP_NUM];

extern dal_op_t g_dal_op;

typedef struct sys_intr_sup_s
{
    uint16  bit_offset;
    uint16  bit_count;
    uint32 tbl_id;
}
sys_intr_sup_t;
sys_intr_sup_t g_intr_sup_info[SYS_INTR_MAX] =
{
    /*  0 */ {  3,   21,    SupIntrFatal_t},
    /*  1 */ {  0,   19,    SupIntrNormal_t},
    /*  2 */ {  0,   1,     SupIntrFunc_t},
    /*  3 */ {  1,   1,     SupIntrFunc_t},
    /*  4 */ {  2,   1,     SupIntrFunc_t},
    /*  5 */ {  3,   1,     SupIntrFunc_t},
    /*  6 */ {  4,   1,     SupIntrFunc_t},
    /*  7 */ {  5,   1,     SupIntrFunc_t},
    /*  8 */ {  6,   1,     SupIntrFunc_t},
    /*  9 */ {  7,   1,     SupIntrFunc_t},
    /* 10 */ {  8,   1,     SupIntrFunc_t},
    /* 11 */ {  9,   1,     SupIntrFunc_t},
    /* 12 */ {  10,   1,     SupIntrFunc_t},
    /* 13 */ {  11,   1,     SupIntrFunc_t},
    /* 14 */ {  12,   1,     SupIntrFunc_t},
    /* 15 */ {  13,   1,     SupIntrFunc_t},
    /* 16 */ {  14,   1,     SupIntrFunc_t},
    /* 17 */ {  15,   1,     SupIntrFunc_t},
    /* 18 */ {  16,   1,     SupIntrFunc_t},
    /* 19 */ {  17,   1,     SupIntrFunc_t},
    /* 20 */ {  18,   1,     SupIntrFunc_t},
    /* 21 */ {  19,   1,     SupIntrFunc_t},
    /* 22 */ {  20,   1,     SupIntrFunc_t},
    /* 23 */ {  21,  1,     SupIntrFunc_t},
    /* 24 */ {  22,  1,     SupIntrFunc_t},
    /* 25 */ {  23,  1,      SupIntrFunc_t},
    /* 26 */ {  24,  1,     SupIntrFunc_t},
    /* 27 */ {  25,  1,      SupIntrFunc_t},
    /* 28 */ {  26,  1,     SupIntrFunc_t},
    /* 29 */ {  27,  1,      SupIntrFunc_t},

};

#define SYS_INTR_FUNC_MAX (SYS_INTR_MAX-2)
uint32 g_intr_func_reg[SYS_INTR_FUNC_MAX] =
{
    0,         /*PTP TS Capture*/
    0,         /*PTP TOD Pluse In*/
    0,         /*PTP TOD Code In Rdy*/
    0,         /*PTP Sync Pluse In*/
    0,         /*PTP Sync Code In Rdy*/

    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    0,
    0,

    RlmHsCtlInterruptFunc_t,
    RlmCsCtlInterruptFunc_t,
    0,
    0,
    PcieIntfInterruptFunc_t,
    DmaCtlIntrFunc_t,
    McuSupInterruptFunc0_t,
    McuSupInterruptFunc1_t
};


/* sub registers for SYS_INTR_REG_SUP_FATAL & SYS_INTR_REG_SUP_NORMAL*/
static uint32 g_intr_sub_reg_sup_abnormal[2][SYS_INTR_SUB_NORMAL_MAX] =
{
    {
         0 ,                              /*reserved 0*/
         0 ,                              /*reserved 1*/
         0 ,                              /*reserved 2*/
         RlmNetCtlInterruptFatal_t ,          /*3*/
         RlmEpeRxCtlInterruptFatal_t ,        /*4*/
         RlmEpeTxCtlInterruptFatal_t ,        /*5*/
         RlmIpeRxCtlInterruptFatal_t ,        /*6*/
         RlmIpeTxCtlInterruptFatal_t ,        /*7*/
         RlmCapwapCtlInterruptFatal_t ,       /*8*/
         RlmSecurityCtlInterruptFatal_t ,     /*9*/
         RlmKeyCtlInterruptFatal_t ,          /*10*/
         RlmTcamCtlInterruptFatal_t ,         /*11*/
         RlmBsrCtlInterruptFatal_t ,          /*12*/
         RlmQMgrCtlInterruptFatal_t ,         /*13*/
         RlmAdCtlInterruptFatal_t,            /*14*/
         PcieIntfInterruptFatal_t ,           /*15*/
         DmaCtlInterruptFatal_t ,             /*16*/
         0,
         0,
         0,
    },
    {
        RlmHsCtlInterruptNormal_t,     /*0*/
        RlmCsCtlInterruptNormal_t,     /*1*/
        RlmMacCtlInterruptNormal_t,   /*2*/
        RlmNetCtlInterruptNormal_t,   /*3*/
        RlmEpeRxCtlInterruptNormal_t,    /*4*/
        RlmEpeTxCtlInterruptNormal_t,    /*5*/
        RlmIpeRxCtlInterruptNormal_t,    /*6*/
        RlmIpeTxCtlInterruptNormal_t,    /*7*/
        RlmCapwapCtlInterruptNormal_t,     /*8*/
        RlmSecurityCtlInterruptNormal_t,     /*9*/
        RlmKeyCtlInterruptNormal_t,     /*10*/
        RlmTcamCtlInterruptNormal_t,     /*11*/
        RlmBsrCtlInterruptNormal_t,      /*12*/
        RlmQMgrCtlInterruptNormal_t,   /*13*/
        RlmAdCtlInterruptNormal_t,  /*14*/
        PcieIntfInterruptNormal_t,   /*15*/
        DmaCtlInterruptNormal_t,   /*16*/
#ifdef EMULATION_ENV
#else
        McuSupInterruptNormal0_t,   /*17*/
        McuSupInterruptNormal1_t,    /*18*/
        I2CMasterInterruptNormal0_t,       /*19*/
        I2CMasterInterruptNormal1_t,     /*20*/
#endif
    }
};
/* low level registers for SYS_INTR_GG_REG_SUP_FATAL, pcie and dma have no low level */
static uint32 g_intr_low_reg_sub_fatal_dt2[SYS_INTR_SUB_FATAL_MAX][SYS_INTR_LOW_FATAL_MAX] =
{
    /*0*/{0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*1*/{0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*2*/{0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*3*/{NetRxInterruptFatal_t, NetTxInterruptFatal_t,  0, 0, 0, 0, 0, 0, 0},
    /*4*/{EpeHdrAdjInterruptFatal_t,  EpeNextHopInterruptFatal_t, EpeHdrProcInterruptFatal_t, 0,0, 0, 0, 0, 0},
    /*5*/{EpeAclOamInterruptFatal_t,  EpeHdrEditInterruptFatal_t,  EpeScheduleInterruptFatal_t, PolicingEpeInterruptFatal_t, 0,0,0,0,0},
    /*6*/{IpeHdrAdjInterruptFatal_t,  IpeIntfMapInterruptFatal_t,  IpeLkupMgrInterruptFatal_t,  EfdInterruptFatal_t, GlobalStatsInterruptFatal_t, 0, 0, 0, 0},
    /*7*/{IpePktProcInterruptFatal_t,  IpeFwdInterruptFatal_t,  DlbInterruptFatal_t,  PolicingIpeInterruptFatal_t,  StormCtlInterruptFatal_t, CoppInterruptFatal_t, 0, 0, 0},
    /*8*/{CapwapCipherInterruptFatal_t, CapwapProcInterruptFatal_t,  0, 0, 0, 0, 0, 0, 0},
    /*9*/{MacSecEngineInterruptFatal_t, 0, 0, 0, 0, 0, 0, 0, 0},
    /*10*/{FibHashInterruptFatal_t,FlowHashInterruptFatal_t,IpfixHashInterruptFatal_t,UserIdHashInterruptFatal_t,FibEngineInterruptFatal_t,FibAccInterruptFatal_t,EgrOamHashInterruptFatal_t,DynamicKeyInterruptFatal_t,DsAgingInterruptFatal_t},
    /*11*/{FlowTcamInterruptFatal_t, DynamicEditInterruptFatal_t, 0, 0, 0, 0, 0, 0, 0},
    /*12*/{BufStoreInterruptFatal_t, MetFifoInterruptFatal_t, OamFwdInterruptFatal_t, OamParserInterruptFatal_t,  OamProcInterruptFatal_t,  PbCtlInterruptFatal_t,  BufRetrvInterruptFatal_t, TsEngineInterruptFatal_t,  0},
    /*13*/{QMgrEnqInterruptFatal_t, QMgrMsgStoreInterruptFatal_t, QMgrDeqInterruptFatal_t, 0,0,0,0,0,0},
    /*14*/{DynamicAdInterruptFatal_t, FlowAccInterruptFatal_t, LpmTcamInterruptFatal_t,0,0,0,0,0,0},
    {0,},
    {0,},
};


uint32 g_intr_low_reg_sub_fatal_tm[SYS_INTR_SUB_FATAL_MAX][SYS_INTR_LOW_FATAL_MAX] =
{
    /*0*/{0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*1*/{0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*2*/{0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*3*/{NetRxInterruptFatal_t, NetTxInterruptFatal_t,  0, 0, 0, 0, 0, 0, 0},
    /*4*/{EpeHdrAdjInterruptFatal_t,  EpeNextHopInterruptFatal_t, EpeHdrProcInterruptFatal_t, 0,0, 0, 0, 0, 0},
    /*5*/{EpeAclOamInterruptFatal_t,  EpeHdrEditInterruptFatal_t,  EpeScheduleInterruptFatal_t, PolicingEpeInterruptFatal_t, IpfixHashInterruptFatal1_t, FlowAccInterruptFatal1_t,0,0,0},
    /*6*/{IpeHdrAdjInterruptFatal_t,  IpeIntfMapInterruptFatal_t,  IpeLkupMgrInterruptFatal_t,  EfdInterruptFatal_t, GlobalStatsInterruptFatal_t, MplsHashInterruptFatal_t, 0, 0, 0},
    /*7*/{IpePktProcInterruptFatal_t,  IpeFwdInterruptFatal_t,  DlbInterruptFatal_t,  PolicingIpeInterruptFatal_t,  StormCtlInterruptFatal_t, CoppInterruptFatal_t, FlowAccInterruptFatal0_t, IpfixHashInterruptFatal0_t, IpeAclInterruptFatal_t},
    /*8*/{CapwapCipherInterruptFatal_t, CapwapProcInterruptFatal_t,  0, 0, 0, 0, 0, 0, 0},
    /*9*/{MacSecEngineInterruptFatal_t, 0, 0, 0, 0, 0, 0, 0, 0},
    /*10*/{FibHashInterruptFatal_t,FlowHashInterruptFatal_t,DsAgingInterruptFatal_t, UserIdHashInterruptFatal_t,FibEngineInterruptFatal_t,FibAccInterruptFatal_t,EgrOamHashInterruptFatal_t,DynamicKeyInterruptFatal_t,0},
    /*11*/{ FlowTcamInterruptFatal_t, DynamicEditInterruptFatal_t, 0, 0, 0, 0,0,0,0},
    /*12*/{BufStoreInterruptFatal_t, MetFifoInterruptFatal_t, OamFwdInterruptFatal_t, OamParserInterruptFatal_t,  OamProcInterruptFatal_t,  PbCtlInterruptFatal_t,  BufRetrvInterruptFatal_t, TsEngineInterruptFatal_t,  0},
    /*13*/{QMgrEnqInterruptFatal_t, QMgrMsgStoreInterruptFatal_t, QMgrDeqInterruptFatal_t, 0,0,0,0,0,0},
    /*14*/{DynamicAdInterruptFatal_t, LpmTcamInterruptFatal_t,0,0, 0,0,0,0,0},
    {0,},
    {0,},
};


/* low level registers for SYS_INTR_SUP_NORMAL*/
static uint32 g_intr_low_reg_sub_normal_dt2[SYS_INTR_SUB_NORMAL_MAX][SYS_INTR_LOW_NORMAL_MAX] =
{
    /*0*/{SharedMiiInterruptNormal0_t, SharedMiiInterruptNormal1_t, SharedMiiInterruptNormal2_t, SharedMiiInterruptNormal3_t,
          SharedMiiInterruptNormal4_t, SharedMiiInterruptNormal5_t, SharedMiiInterruptNormal6_t, SharedMiiInterruptNormal7_t,
          SharedMiiInterruptNormal8_t, SharedMiiInterruptNormal9_t, SharedMiiInterruptNormal10_t, SharedMiiInterruptNormal11_t,

          SharedPcsInterruptNormal0_t, SharedPcsInterruptNormal1_t, SharedPcsInterruptNormal2_t, SharedPcsInterruptNormal3_t,
          SharedPcsInterruptNormal4_t, SharedPcsInterruptNormal5_t,

          /*bit 18 ~ 29*/
          QsgmiiPcsInterruptNormal0_t, QsgmiiPcsInterruptNormal1_t,
          QsgmiiPcsInterruptNormal2_t, QsgmiiPcsInterruptNormal3_t, QsgmiiPcsInterruptNormal4_t, QsgmiiPcsInterruptNormal5_t,
          QsgmiiPcsInterruptNormal6_t, QsgmiiPcsInterruptNormal7_t,  QsgmiiPcsInterruptNormal8_t,  QsgmiiPcsInterruptNormal9_t,
          QsgmiiPcsInterruptNormal10_t, QsgmiiPcsInterruptNormal11_t,

          /*bit 30~41*/
          UsxgmiiPcsInterruptNormal0_t, UsxgmiiPcsInterruptNormal1_t,
          UsxgmiiPcsInterruptNormal2_t, UsxgmiiPcsInterruptNormal3_t, UsxgmiiPcsInterruptNormal4_t, UsxgmiiPcsInterruptNormal5_t,
          UsxgmiiPcsInterruptNormal6_t, UsxgmiiPcsInterruptNormal7_t, UsxgmiiPcsInterruptNormal8_t, UsxgmiiPcsInterruptNormal9_t,
          UsxgmiiPcsInterruptNormal10_t, UsxgmiiPcsInterruptNormal11_t,

          /*serdes Id 0~23, bit 42~65*/
          AnethInterruptNormal37_t, AnethInterruptNormal34_t, AnethInterruptNormal31_t, AnethInterruptNormal28_t,
          AnethInterruptNormal25_t, AnethInterruptNormal22_t, AnethInterruptNormal19_t, AnethInterruptNormal16_t,
          AnethInterruptNormal38_t, AnethInterruptNormal35_t, AnethInterruptNormal32_t, AnethInterruptNormal29_t,
          AnethInterruptNormal26_t, AnethInterruptNormal23_t, AnethInterruptNormal20_t, AnethInterruptNormal17_t,
          AnethInterruptNormal39_t, AnethInterruptNormal36_t, AnethInterruptNormal33_t, AnethInterruptNormal30_t,
          AnethInterruptNormal27_t, AnethInterruptNormal24_t, AnethInterruptNormal21_t, AnethInterruptNormal18_t,
          },
    /*1*/{ SharedMiiInterruptNormal12_t, SharedMiiInterruptNormal13_t, SharedMiiInterruptNormal14_t, SharedMiiInterruptNormal15_t,
           SharedPcsInterruptNormal6_t, SharedPcsInterruptNormal7_t, SharedPcsInterruptNormal8_t, SharedPcsInterruptNormal9_t,

           /*serdes Id 24~39, bit 8~23*/
           AnethInterruptNormal12_t, AnethInterruptNormal8_t, AnethInterruptNormal4_t, AnethInterruptNormal0_t,
           AnethInterruptNormal13_t, AnethInterruptNormal9_t, AnethInterruptNormal5_t, AnethInterruptNormal1_t,
           AnethInterruptNormal14_t, AnethInterruptNormal10_t, AnethInterruptNormal6_t, AnethInterruptNormal2_t,
           AnethInterruptNormal15_t, AnethInterruptNormal11_t, AnethInterruptNormal7_t, AnethInterruptNormal3_t,

          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0, 0, 0, 0, 0, 0, 0, 0, 0,0},
    /*2*/{QuadSgmacInterruptNormal0_t, QuadSgmacInterruptNormal1_t, QuadSgmacInterruptNormal2_t, QuadSgmacInterruptNormal3_t, QuadSgmacInterruptNormal4_t, QuadSgmacInterruptNormal5_t, QuadSgmacInterruptNormal6_t, QuadSgmacInterruptNormal7_t,
          QuadSgmacInterruptNormal8_t,  QuadSgmacInterruptNormal9_t, QuadSgmacInterruptNormal10_t,  QuadSgmacInterruptNormal11_t, QuadSgmacInterruptNormal12_t,  QuadSgmacInterruptNormal13_t,  QuadSgmacInterruptNormal14_t, QuadSgmacInterruptNormal15_t,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*3*/{NetRxInterruptNormal_t, NetTxInterruptNormal_t,  0,0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0},
    /*4*/{EpeHdrAdjInterruptNormal_t,  EpeHdrProcInterruptNormal_t, EpeNextHopInterruptNormal_t,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*5*/{EpeAclOamInterruptNormal_t, EpeHdrEditInterruptNormal_t, EpeScheduleInterruptNormal_t, PolicingEpeInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*6*/{IpeHdrAdjInterruptNormal_t, IpeIntfMapInterruptNormal_t, EfdInterruptNormal_t, GlobalStatsInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*7*/{IpePktProcInterruptNormal_t, IpeFwdInterruptNormal_t, DlbInterruptNormal_t, PolicingIpeInterruptNormal_t, StormCtlInterruptNormal_t, LinkAggInterruptNomal_t, CoppInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*8*/{CapwapCipherInterruptNormal_t, CapwapProcInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*9*/{MacSecEngineInterruptNormal_t,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*10*/{EgrOamHashInterruptNormal_t, DynamicKeyInterruptNormal_t, DsAgingInterruptNormal_t, FibAccInterruptNormal_t, FibEngineInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*11*/{FlowTcamInterruptNormal_t,DynamicEditInterruptNormal_t, LpmTcamInterruptNormal_t, DynamicAdInterruptNormal_t, FlowAccInterruptNormal_t,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*12*/{BufStoreInterruptNormal_t, MetFifoInterruptNormal_t, OamFwdInterruptNormal_t, OamProcInterruptNormal_t, OamAutoGenPktInterruptNormal_t, PbCtlInterruptNormal_t, BufRetrvInterruptNormal_t,
           TsEngineInterruptNormal_t, OobFcInterruptNormal_t, LedInterruptNormal0_t, LedInterruptNormal0_t, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*13*/{QMgrEnqInterruptNormal_t, QMgrDeqInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*14*/{DynamicAdInterruptNormal_t, FlowAccInterruptNormal_t, LpmTcamInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0},
};


/* low level registers for SYS_INTR_SUP_NORMAL*/
 uint32 g_intr_low_reg_sub_normal_tm[SYS_INTR_SUB_NORMAL_MAX][SYS_INTR_LOW_NORMAL_MAX] =
{
    /*0*/{Hss12GUnitWrapperInterruptNormal0_t , Hss12GUnitWrapperInterruptNormal1_t, Hss12GUnitWrapperInterruptNormal2_t, 0,
          0, 0, 0, 0,
          0, 0, 0, 0,

          0, 0, 0, 0,
          0, 0,

          /*bit 18 ~ 29*/
          0, 0,
          0, 0, 0, 0,
          0, 0,  0,  0,
          0, 0,

          /*bit 30~41*/
          0, 0,
          0, 0, 0, 0,
          0, 0, 0, 0,
          0, 0,

          /*serdes Id 0~23, bit 42~65*/
          0, 0, 0, 0,
          0, 0, 0, 0,
          0, 0, 0, 0,
          0, 0, 0, 0,
          0, 0, 0, 0,
          0, 0, 0, 0,
          },
    /*1*/{ Hss28GUnitWrapperInterruptNormal_t, 0 , 0, 0,
           0, 0, 0, 0,

           /*serdes Id 24~39, bit 8~23*/
           0, 0, 0, 0,
           0, 0, 0, 0,
           0, 0, 0, 0,
           0, 0, 0, 0,

          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0, 0, 0, 0, 0, 0, 0, 0, 0,0},
    /*2*/{QuadSgmacInterruptNormal0_t, QuadSgmacInterruptNormal1_t, QuadSgmacInterruptNormal2_t, QuadSgmacInterruptNormal3_t, QuadSgmacInterruptNormal4_t, QuadSgmacInterruptNormal5_t, QuadSgmacInterruptNormal6_t, QuadSgmacInterruptNormal7_t,
          QuadSgmacInterruptNormal8_t,  QuadSgmacInterruptNormal9_t, QuadSgmacInterruptNormal10_t,  QuadSgmacInterruptNormal11_t, QuadSgmacInterruptNormal12_t,  QuadSgmacInterruptNormal13_t,  QuadSgmacInterruptNormal14_t, QuadSgmacInterruptNormal15_t,
          QuadSgmacInterruptNormal16_t, QuadSgmacInterruptNormal17_t, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*3*/{NetRxInterruptNormal_t, NetTxInterruptNormal_t,  0,0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0},
    /*4*/{EpeHdrAdjInterruptNormal_t,  EpeHdrProcInterruptNormal_t, EpeNextHopInterruptNormal_t,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*5*/{EpeAclOamInterruptNormal_t, EpeHdrEditInterruptNormal_t, EpeScheduleInterruptNormal_t, PolicingEpeInterruptNormal_t, FlowAccInterruptNormal1_t, IpfixHashInterruptNormal1_t, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*6*/{IpeHdrAdjInterruptNormal_t, IpeIntfMapInterruptNormal_t, EfdInterruptNormal_t, GlobalStatsInterruptNormal_t, MplsHashInterruptNormal_t, IpeLkupMgrInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*7*/{IpePktProcInterruptNormal_t, IpeFwdInterruptNormal_t, DlbInterruptNormal_t, PolicingIpeInterruptNormal_t, StormCtlInterruptNormal_t, LinkAggInterruptNomal_t, CoppInterruptNormal_t, FlowAccInterruptNormal0_t, IpfixHashInterruptNormal0_t, IpeAclInterruptNormal_t, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*8*/{CapwapCipherInterruptNormal_t, CapwapProcInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*9*/{MacSecEngineInterruptNormal_t,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*10*/{EgrOamHashInterruptNormal_t, DynamicKeyInterruptNormal_t, DsAgingInterruptNormal_t, FibAccInterruptNormal_t, FibEngineInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*11*/{FlowTcamInterruptNormal_t,DynamicEditInterruptNormal_t, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*12*/{BufStoreInterruptNormal_t, MetFifoInterruptNormal_t, OamFwdInterruptNormal_t, OamProcInterruptNormal_t, OamAutoGenPktInterruptNormal_t, PbCtlInterruptNormal_t, BufRetrvInterruptNormal_t,
           TsEngineInterruptNormal_t, OobFcInterruptNormal_t, LedInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*13*/{QMgrEnqInterruptNormal_t, QMgrDeqInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*14*/{DynamicAdInterruptNormal_t,  LpmTcamInterruptNormal_t, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0},
};

static uint32 *g_intr_low_reg_sub_normal[CTC_MAX_LOCAL_CHIP_NUM][SYS_INTR_SUB_NORMAL_MAX] = {{NULL}};
static uint32 *g_intr_low_reg_sub_fatal[CTC_MAX_LOCAL_CHIP_NUM][SYS_INTR_SUB_FATAL_MAX] = {{NULL}};


/* interrupt register type in D2 */
typedef enum sys_interrupt_reg_type_e
{
    SYS_INTR_REG_TYPE_ABNORMAL = 0,
    SYS_INTR_REG_TYPE_FUNC,
    SYS_INTR_REG_TYPE_MAX
} sys_interrupt_reg_type_t;

/****************************************************************************
*
* Function
*
*****************************************************************************/
extern int32 drv_get_host_type(uint8 lchip);
/****************************************************************************
*
* Function
*
*****************************************************************************/

/**
 @brief mapping from ctc parameter to sys level
*/
STATIC int32
_sys_usw_interrupt_type_mapping(uint8 lchip, ctc_intr_type_t* p_type, sys_intr_type_t* p_sys_intr)
{
    int32 ret = CTC_E_NONE;

    switch(p_type->intr)
    {
        case CTC_INTR_CHIP_FATAL:
            p_sys_intr->intr = SYS_INTR_CHIP_FATAL;
            p_sys_intr->sub_intr = p_type->sub_intr;
            p_sys_intr->low_intr = p_type->low_intr;
            break;
        case CTC_INTR_CHIP_NORMAL:
            p_sys_intr->intr = SYS_INTR_CHIP_NORMAL;
            p_sys_intr->sub_intr = p_type->sub_intr;
            p_sys_intr->low_intr = p_type->low_intr;
            break;

        /* ptp */
        case CTC_INTR_FUNC_PTP_TS_CAPTURE:
            p_sys_intr->intr = SYS_INTR_FUNC_PTP_TS_CAPTURE_FIFO;
            p_sys_intr->slice_id = 0;
            break;
        case CTC_INTR_FUNC_PTP_TOD_PULSE_IN:
            p_sys_intr->intr = SYS_INTR_FUNC_PTP_TOD_PULSE_IN;
            p_sys_intr->slice_id = 0;
            break;
        case CTC_INTR_FUNC_PTP_TOD_CODE_IN_RDY:
            p_sys_intr->intr = SYS_INTR_FUNC_PTP_TOD_CODE_IN_RDY;
            p_sys_intr->slice_id = 0;
            break;
        case CTC_INTR_FUNC_PTP_SYNC_PULSE_IN:
            p_sys_intr->intr = SYS_INTR_FUNC_PTP_SYNC_PULSE_IN;
            p_sys_intr->slice_id = 0;
            break;
        case CTC_INTR_FUNC_PTP_SYNC_CODE_IN_RDY:
            p_sys_intr->intr = SYS_INTR_FUNC_PTP_SYNC_CODE_IN_RDY;
            p_sys_intr->slice_id = 0;
            break;

        case CTC_INTR_FUNC_OAM_DEFECT_CACHE:
            p_sys_intr->intr = SYS_INTR_FUNC_OAM_DEFECT_CACHE;
            break;

        case CTC_INTR_FUNC_MDIO_CHANGE:
            p_sys_intr->intr = SYS_INTR_FUNC_MDIO_XG_CHANGE_0;
            p_sys_intr->sub_intr = 0;
            p_sys_intr->slice_id = 0;
            break;

        case CTC_INTR_FUNC_EXTRAL_GPIO_CHANGE:
            p_sys_intr->intr = SYS_INTR_GPIO;
            p_sys_intr->sub_intr = 0;
            p_sys_intr->slice_id = 0;
            break;
#if 0
        case CTC_INTR_FUNC_MDIO_1G_CHANGE: /*for 1g*/
            p_sys_intr->intr = SYS_INTR_GG_FUNC_MDIO_CHANGE_0;
            p_sys_intr->sub_intr = 0;
            p_sys_intr->slice_id = 2;
            break;
#endif

        /* linkagg failover */
        case CTC_INTR_FUNC_LINKAGG_FAILOVER:
            p_sys_intr->intr = SYS_INTR_FUNC_CHAN_LINKDOWN_SCAN;
            p_sys_intr->slice_id = 0;
            break;

        /* aps failover */
        case CTC_INTR_FUNC_APS_FAILOVER:
            p_sys_intr->intr = SYS_INTR_FUNC_MET_LINK_SCAN_DONE;
            p_sys_intr->slice_id = 0;
            break;

        /* flow stats */
        case CTC_INTR_FUNC_STATS_FIFO:
            p_sys_intr->intr = SYS_INTR_FUNC_STATS_STATUS_ADDR;
            p_sys_intr->slice_id = 0;
            break;

        /* DMA */
        case CTC_INTR_DMA_FUNC:
            p_sys_intr->intr = SYS_INTR_DMA;
            p_sys_intr->sub_intr = p_type->sub_intr;
            p_sys_intr->slice_id = 0;
            break;

        case CTC_INTR_FUNC_LINK_CHAGNE:
            p_sys_intr->intr = SYS_INTR_PCS_LINK_31_0;
            break;
        case CTC_INTR_FUNC_IPFIX_OVERFLOW:
            p_sys_intr->intr =  SYS_INTR_FUNC_IPFIX_USEAGE_OVERFLOW;
            break;

        case CTC_INTR_FUNC_STMCTL_STATE:
            p_sys_intr->intr =  SYS_INTR_FUNC_STMCTL_STATE;
            break;

        default:
            ret = CTC_E_INVALID_PARAM;
            break;
    }

    return ret;
}

#if ((SDK_WORK_PLATFORM == 1) || ((0 == SDK_WORK_PLATFORM) && (1 == SDK_WORK_ENV)))  /* simulation */
int32
_sys_usw_interrupt_model_sim_value(uint8 lchip, int32 tbl_id, uint32 intr, uint32* bmp, uint32 enable)
{
    uint32 cmd = 0;
    uint32 value_set[CTC_INTR_STAT_SIZE];
    uint32 value_reset[CTC_INTR_STAT_SIZE];
    uint32 intr_vec[CTC_INTR_STAT_SIZE];

    /*
    if (drv_io_is_asic())
    {
        return 0;
    }
    */

    CTC_BMP_INIT(value_set);
    CTC_BMP_INIT(value_reset);

    if (enable)
    {
        /* trigger/set */
    }
    else
    {
        /* clear */

        /* 1. read value */
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_VAL_SET, cmd, value_set));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_VAL_RESET, cmd, value_reset));

        /* 2. do reset action */
        value_reset[0] &= bmp[0];
        value_reset[1] &= bmp[1];
        value_reset[2] &= bmp[2];

        value_set[0] &= ~value_reset[0];
        value_set[1] &= ~value_reset[1];
        value_set[2] &= ~value_reset[2];

        /* 3. write value */
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_VAL_SET, cmd, value_set));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_VAL_RESET, cmd, value_reset));
    }

    if (intr < SYS_INTR_MAX)
    {
        CTC_BMP_INIT(intr_vec);
        cmd = DRV_IOR(SupCtlIntrVec_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &intr_vec);
        CTC_BMP_UNSET(intr_vec, intr);
        cmd = DRV_IOW(SupCtlIntrVec_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &intr_vec);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_model_sim_mask(uint8 lchip, int32 tbl_id, uint32 bit_offset, uint32 bit_count, uint32 enable)
{
    uint32 cmd = 0;
    uint32 i = 0;
    uint32 mask[CTC_INTR_STAT_SIZE];
    uint32 mask_set[CTC_INTR_STAT_SIZE];
    uint32 mask_reset[CTC_INTR_STAT_SIZE];

    /*
    if (drv_io_is_asic())
    {
        return 0;
    }
    */
    CTC_BMP_INIT(mask);
    CTC_BMP_INIT(mask_set);
    CTC_BMP_INIT(mask_reset);

    if (enable)
    {
        /* 1. read value */
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, mask_set));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, mask_reset));

        /* 2. do reset action */
        if (CTC_INTR_ALL != bit_offset)
        {
            for (i = 0; i < bit_count; i++)
            {
                CTC_BMP_SET(mask, bit_offset+i);
            }
            mask_reset[0] &= mask[0];
            mask_reset[1] &= mask[1];
            mask_reset[2] &= mask[2];
        }

        mask_set[0] &= ~mask_reset[0];
        mask_set[1] &= ~mask_reset[1];
        mask_set[2] &= ~mask_reset[2];

        /* 3. write value */
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, mask_set));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, mask_set));
    }
    else
    {
        /* 1. read value from set */
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, mask_set));

        /* 2. sync value to reset */
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, mask_set));
    }

    return CTC_E_NONE;
}

#endif /* (SDK_WORK_PLATFORM == 1) */

char*
_sys_usw_interrupt_index_str(uint8 lchip, uint32 index)
{
    switch (index)
    {
    case INTR_INDEX_VAL_SET:
        return "ValueSet";

    case INTR_INDEX_VAL_RESET:
        return "ValueReset";

    case INTR_INDEX_MASK_SET:
        return "MaskSet";

    case INTR_INDEX_MASK_RESET:
        return "MaskReset";

    default:
        return "Invalid";
    }
}

STATIC int32
_sys_usw_interrupt_get_status_common(uint8 lchip, uint32 tbl_id, uint32* p_bmp)
{
    uint32 cmd = 0;
    uint32 value[CTC_INTR_STAT_SIZE] = {0};
    uint32 mask[CTC_INTR_STAT_SIZE] = {0};
    uint8 index = 0;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_VAL_SET, cmd, value));
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, mask));

    for(index = 0;index < CTC_INTR_STAT_SIZE;++index)
    {
        p_bmp[index] = value[index] & (~mask[index]);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_set_en_common(uint8 lchip, uint32 bit_offset, uint32 bit_count, uint32 tbl_id, uint32 enable)
{
    uint32 cmd = 0;
    uint32 i = 0;
    uint32 index = 0;
    uint32 mask[CTC_INTR_STAT_SIZE];

    CTC_BMP_INIT(mask);

    index = (enable) ? INTR_INDEX_MASK_RESET : INTR_INDEX_MASK_SET;

#if ((SDK_WORK_PLATFORM == 1) || ((0 == SDK_WORK_PLATFORM) && (1 == SDK_WORK_ENV)))  /* simulation */
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, mask));
#endif

    for (i = 0; i < bit_count; i++)
    {
        CTC_BMP_SET(mask, bit_offset+i);
    }

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, mask));

#if ((SDK_WORK_PLATFORM == 1) || ((0 == SDK_WORK_PLATFORM) && (1 == SDK_WORK_ENV)))  /* simulation */
    _sys_usw_interrupt_model_sim_mask(lchip, tbl_id, bit_offset, bit_count, enable);
#endif

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_clear_status_common(uint8 lchip, uint32 intr, uint32* bmp, uint32 tbl_id)
{
    uint32 cmd = 0;
    uint32 index = INTR_INDEX_VAL_RESET;

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, bmp));

#if ((SDK_WORK_PLATFORM == 1) || ((0 == SDK_WORK_PLATFORM) && (1 == SDK_WORK_ENV)))  /* simulation */
    _sys_usw_interrupt_model_sim_value(lchip, tbl_id, intr, bmp, FALSE);
#endif

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_clear_status_abnormal(uint8 lchip, sys_intr_type_t* p_type, uint32* p_bmp)
{
    uint32 tbl_id = 0;
    uint32 sub_tbl_id = 0;
    uint32 low_tbl_id = 0;


    if(p_type->intr != SYS_INTR_CHIP_FATAL && p_type->intr != SYS_INTR_CHIP_NORMAL)
    {
        return CTC_E_INVALID_PARAM;
    }

    if(p_type->sub_intr != INVG)
    {
        sub_tbl_id = g_intr_sub_reg_sup_abnormal[p_type->intr][p_type->sub_intr];

        if(sub_tbl_id != 0)
        {
            if(p_type->low_intr != INVG) /*only clear low level intr status*/
            {
                if(p_type->intr == SYS_INTR_CHIP_FATAL  && (p_type->sub_intr < SYS_INTR_SUB_FATAL_PCIE_INTF) )
                {
                    low_tbl_id = g_intr_low_reg_sub_fatal[lchip][p_type->sub_intr][p_type->low_intr];
                }
                else if( p_type->intr == SYS_INTR_CHIP_NORMAL && (p_type->sub_intr < SYS_INTR_SUB_FATAL_PCIE_INTF))
                {
                    low_tbl_id = g_intr_low_reg_sub_normal[lchip][p_type->sub_intr][p_type->low_intr];
                }

                if(low_tbl_id != 0)
                {
                    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"clear status: low-table-name:%s\n",TABLE_NAME(lchip, low_tbl_id));
                    CTC_ERROR_RETURN(_sys_usw_interrupt_clear_status_common(lchip, p_type->intr, p_bmp, low_tbl_id));
                }
            }
            else /*only clear sub level intr status*/
            {
                SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"clear status: sub-table-name:%s\n",TABLE_NAME(lchip, sub_tbl_id));
                CTC_ERROR_RETURN(_sys_usw_interrupt_clear_status_common(lchip, p_type->intr, p_bmp, sub_tbl_id));
            }
        }
    }
    else
    {
        tbl_id = g_intr_sup_info[p_type->intr].tbl_id; /*clear sup level intr status*/
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"clear status: sup-table-name:%s\n",TABLE_NAME(lchip, tbl_id));
        CTC_ERROR_RETURN(_sys_usw_interrupt_clear_status_common(lchip, p_type->intr, p_bmp, tbl_id));
    }
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_interrupt_get_status_abnormal(uint8 lchip, sys_intr_type_t* p_type, uint32* p_bmp)
{
    uint32 tbl_id = 0;
    uint32 sub_tbl_id = 0;
    uint32 low_tbl_id = 0;

    if(p_type->intr != SYS_INTR_CHIP_FATAL && p_type->intr != SYS_INTR_CHIP_NORMAL)
    {
        return CTC_E_INVALID_PARAM;
    }

    if(p_type->sub_intr != INVG)
    {

        sub_tbl_id = g_intr_sub_reg_sup_abnormal[p_type->intr][p_type->sub_intr];
        if(sub_tbl_id != 0 )
        {
            if(p_type->low_intr != INVG) /*only get low level intr status*/
            {
                if((p_type->intr == SYS_INTR_CHIP_FATAL) && (p_type->sub_intr < SYS_INTR_SUB_FATAL_MAX))
                {
                    low_tbl_id = g_intr_low_reg_sub_fatal[lchip][p_type->sub_intr][p_type->low_intr];
                }
                else if((p_type->intr == SYS_INTR_CHIP_NORMAL) && (p_type->sub_intr < SYS_INTR_SUB_NORMAL_MAX))
                {
                    low_tbl_id = g_intr_low_reg_sub_normal[lchip][p_type->sub_intr][p_type->low_intr];
                }

                if(low_tbl_id != 0)
                {
                    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"get status: low-table-name:%s\n",TABLE_NAME(lchip, low_tbl_id));
                    CTC_ERROR_RETURN(_sys_usw_interrupt_get_status_common(lchip, low_tbl_id, p_bmp));
                }
            }
            else   /*get sub level init status*/
            {
                SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"get status: sub-table-name:%s\n",TABLE_NAME(lchip, sub_tbl_id));
                CTC_ERROR_RETURN(_sys_usw_interrupt_get_status_common(lchip, sub_tbl_id, p_bmp));
            }
        }
    }
    else
    {
        tbl_id = g_intr_sup_info[p_type->intr].tbl_id; /*get sup level intr status*/
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"get status: sup-table-name:%s\n",TABLE_NAME(lchip, tbl_id));
        CTC_ERROR_RETURN(_sys_usw_interrupt_get_status_common(lchip, tbl_id, p_bmp));
    }
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_interrupt_set_en_abnormal(uint8 lchip, sys_intr_type_t* p_type, uint32 enable)
{
    int32 tbl_id = -1;
    int32 sub_tbl_id = -1;

    if(p_type->intr != SYS_INTR_CHIP_FATAL && p_type->intr != SYS_INTR_CHIP_NORMAL)
    {
        return CTC_E_INVALID_PARAM;
    }

    if(p_type->sub_intr != INVG)
    {
        if(p_type->low_intr != INVG)
        {

            sub_tbl_id = g_intr_sub_reg_sup_abnormal[p_type->intr][p_type->sub_intr];
            if(sub_tbl_id != 0 && (p_type->low_intr < TABLE_FIELD_NUM(lchip, sub_tbl_id)))
            {
                SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"set enable: sub-table-name:%s\n",TABLE_NAME(lchip, sub_tbl_id));
                CTC_ERROR_RETURN(_sys_usw_interrupt_set_en_common(lchip, p_type->low_intr, 1,sub_tbl_id, enable));
            }
        }
        else
        {
            tbl_id = g_intr_sup_info[p_type->intr].tbl_id;
            if(tbl_id != 0 && (p_type->sub_intr < TABLE_FIELD_NUM(lchip, tbl_id)))
            {
                SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"set enable: sup-table-name:%s\n",TABLE_NAME(lchip, tbl_id));
                CTC_ERROR_RETURN(_sys_usw_interrupt_set_en_common(lchip, p_type->sub_intr, 1,tbl_id, enable));
            }
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_clear_status_func(uint8 lchip, sys_intr_type_t* p_type, uint32* p_bmp)
{
    uint32 tbl_id = 0;
    uint32 sub_tbl_id = 0;
    uint32 bmp[CTC_INTR_STAT_SIZE] = {0};
    uint8 bit_offset = 0;
    if(p_type->intr == SYS_INTR_CHIP_FATAL || p_type->intr == SYS_INTR_CHIP_NORMAL)
    {
        return CTC_E_INVALID_PARAM;
    }

    tbl_id = g_intr_sup_info[p_type->intr].tbl_id;
    bit_offset = g_intr_sup_info[p_type->intr].bit_offset;

    sub_tbl_id = g_intr_func_reg[(p_type->intr-2)];
    if(sub_tbl_id != 0)
    {
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"clear status: sub-table-name:%s\n",TABLE_NAME(lchip, sub_tbl_id));
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"bitmap: 0x%x\n",*p_bmp);
        CTC_ERROR_RETURN(_sys_usw_interrupt_clear_status_common(lchip,p_type->intr,p_bmp,sub_tbl_id));
    }

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"clear status: sup-table-name:%s\n",TABLE_NAME(lchip, tbl_id));
    /*clear supCtlFunc*/
    bmp[0] = 1 << bit_offset;
    CTC_ERROR_RETURN(_sys_usw_interrupt_clear_status_common(lchip,p_type->intr,bmp,tbl_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_get_status_func(uint8 lchip, sys_intr_type_t* p_type, uint32* p_bmp)
{
    uint32 tbl_id = 0;
    uint32 sub_tbl_id = 0;
    uint32 bmp[CTC_INTR_STAT_SIZE] = {0};
    int8 bit_offset = 0;
    if(p_type->intr == SYS_INTR_CHIP_FATAL || p_type->intr == SYS_INTR_CHIP_NORMAL)
    {
        return CTC_E_INVALID_PARAM;
    }

    tbl_id = g_intr_sup_info[p_type->intr].tbl_id;
    bit_offset = g_intr_sup_info[p_type->intr].bit_offset;
    sub_tbl_id = g_intr_func_reg[(p_type->intr-2)];

    if(sub_tbl_id != 0)
    {
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"get status: sub-table-name:%s\n",TABLE_NAME(lchip, sub_tbl_id));
        _sys_usw_interrupt_get_status_common(lchip,sub_tbl_id,p_bmp);
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"bitmap: 0x%x\n",*p_bmp);
    }
    else
    {
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"get status: sup-table-name:%s\n",TABLE_NAME(lchip, tbl_id));
        _sys_usw_interrupt_get_status_common(lchip,tbl_id,&bmp[0]);
        *p_bmp = (bmp[0] >> bit_offset) & 0x01;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_set_en_func(uint8 lchip, sys_intr_type_t* p_type, uint32 enable)
{
    uint32 tbl_id = 0;
    uint8 bit_offset = 0;
    if(p_type->intr == SYS_INTR_CHIP_FATAL || p_type->intr == SYS_INTR_CHIP_NORMAL)
    {
        return CTC_E_INVALID_PARAM;
    }

    if(p_type->sub_intr != INVG)
    {
        tbl_id = g_intr_func_reg[(p_type->intr-2)];
        if(tbl_id != -1)
        {
            SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"set enable: table-name:%s\n",TABLE_NAME(lchip, tbl_id));
            _sys_usw_interrupt_set_en_common(lchip,p_type->sub_intr,1,tbl_id,enable);
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        tbl_id = g_intr_sup_info[p_type->intr].tbl_id;
        bit_offset = g_intr_sup_info[p_type->intr].bit_offset;
        CTC_ERROR_RETURN(_sys_usw_interrupt_set_en_common(lchip,bit_offset,1,tbl_id,enable));
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_set_sup_op(uint8 lchip, uint8 grp_id, uint8 act_idx)
{
    uint32 cmd = 0;
    SupIntrGenCtl_m sup_ctl;
    uint32 value = 0;

    value = (1 << grp_id);

    sal_memset(&sup_ctl, 0, sizeof(sup_ctl));
    if (INTR_INDEX_VAL_SET == act_idx)
    {
        SetSupIntrGenCtl(V, supIntrValueSet_f, &sup_ctl, value);
    }
    else if (INTR_INDEX_VAL_RESET == act_idx)
    {
        SetSupIntrGenCtl(V, supIntrValueClr_f, &sup_ctl, value);
    }
    else if (INTR_INDEX_MASK_SET == act_idx)
    {
        SetSupIntrGenCtl(V, supIntrMaskSet_f, &sup_ctl, value);
    }
    else
    {
        SetSupIntrGenCtl(V, supIntrMaskClr_f, &sup_ctl, value);

        /*When release msi interrup, clear Pcie0IntrLogCtl_pcie0IntrLogVec_f*/
        if (g_usw_intr_master[lchip].intr_mode == 1)
        {
            value = 0;
            cmd = DRV_IOR(Pcie0IntrLogCtl_t, Pcie0IntrLogCtl_pcie0IntrLogVec_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            CTC_BIT_UNSET(value, grp_id);
            cmd = DRV_IOW(Pcie0IntrLogCtl_t, Pcie0IntrLogCtl_pcie0IntrLogVec_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            cmd = DRV_IOR(PcieIntrLogCtl_t, PcieIntrLogCtl_pcieIntrLogVec_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            CTC_BIT_UNSET(value, grp_id);
            cmd = DRV_IOW(PcieIntrLogCtl_t, PcieIntrLogCtl_pcieIntrLogVec_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        }

    }

    cmd = DRV_IOW(SupIntrGenCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sup_ctl));

    return CTC_E_NONE;
}
/*used to check interrupt whether is event interrupt*/
STATIC uint32
_sys_usw_interrupt_check_event_intr(uint8 lchip, sys_intr_type_t* p_type)
{
    uint32 is_event = 0;

    switch (p_type->intr)
    {
        case  SYS_INTR_PCS_LINK_31_0:
        case  SYS_INTR_PCS_LINK_47_32:
        case  SYS_INTR_FUNC_CHAN_LINKDOWN_SCAN:
        case  SYS_INTR_DMA:
        case  SYS_INTR_FUNC_IPFIX_USEAGE_OVERFLOW:
        case  SYS_INTR_FUNC_MDIO_CHANGE_0:
        case  SYS_INTR_FUNC_MDIO_CHANGE_1:
        case  SYS_INTR_FUNC_MDIO_XG_CHANGE_0:
        case  SYS_INTR_FUNC_MDIO_XG_CHANGE_1:
        case  SYS_INTR_GPIO:
            is_event = 1;
            break;
        default:
            is_event = 0;
            break;
    }

    return ((is_event)?TRUE:FALSE);
}

STATIC int32
_sys_usw_interrupt_set_mapping(uint8 lchip, sys_intr_group_t* p_group, bool enable)
{
    ds_t map_ctl;
    uint32 cmd = 0;
    uint32 intr_bmp = 0;

    cmd = DRV_IOR(SupIntrMapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &map_ctl));

    switch (p_group->irq_idx)
    {
        case 0:
            intr_bmp = GetSupIntrMapCtl(V, supIntrMap0_f, &map_ctl);
            intr_bmp = (enable)?(intr_bmp|p_group->intr_bmp):(intr_bmp&(~p_group->intr_bmp));
            SetSupIntrMapCtl(V, supIntrMap0_f, &map_ctl, intr_bmp);
            break;

        case 1:
            intr_bmp = GetSupIntrMapCtl(V, supIntrMap1_f, &map_ctl);
            intr_bmp = (enable)?(intr_bmp|p_group->intr_bmp):(intr_bmp&(~p_group->intr_bmp));
            SetSupIntrMapCtl(V, supIntrMap1_f, &map_ctl, intr_bmp);
            break;

        case 2:
            intr_bmp = GetSupIntrMapCtl(V, supIntrMap2_f, &map_ctl);
            intr_bmp = (enable)?(intr_bmp|p_group->intr_bmp):(intr_bmp&(~p_group->intr_bmp));
            SetSupIntrMapCtl(V, supIntrMap2_f, &map_ctl, intr_bmp);
            break;

        case 3:
            intr_bmp = GetSupIntrMapCtl(V, supIntrMap3_f, &map_ctl);
            intr_bmp = (enable)?(intr_bmp|p_group->intr_bmp):(intr_bmp&(~p_group->intr_bmp));
            SetSupIntrMapCtl(V, supIntrMap3_f, &map_ctl, intr_bmp);
            break;

        default:
            break;
    }

    cmd = DRV_IOW(SupIntrMapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &map_ctl));

    return CTC_E_NONE;
}

STATIC int
_sys_usw_interrupt_get_group_id(uint8 lchip, uint32 irq, uint8* group_cnt, uint8* group_arry)
{
    uint8  i   = 0;
    uint8  cnt = 0;
    uint8  group_id = 0;

    /* get irq's group info */
    for(i=0; i<g_usw_intr_master[lchip].group_count; i++)
    {
        if(irq == g_usw_intr_master[lchip].group[i].irq)
        {
            /* get which group occur interrupt */
            group_id = i;
            *(group_arry+cnt) = group_id;
            cnt++;
        }
    }

    *group_cnt = cnt;

    return CTC_E_NONE;
}
/* get which chip occur intrrrupt by IRQ */
STATIC int
_sys_usw_interrupt_get_chip(uint32 irq, uint8 lchip_num, uint8* chip_array,uint8 *valid_chips)
{
    uint8 lchip_idx = 0;
    uint8 group_idx = 0;
    uint8 valid_idx = 0;

    /* get which chip occur intrrupt, and get the group_id */
    for (lchip_idx=0; lchip_idx<lchip_num; lchip_idx++)
    {

        if(sys_usw_chip_check_active(lchip_idx)
            || !g_usw_intr_master[lchip_idx].init_done )
        {
            continue;
        }

        for(group_idx=0; group_idx<g_usw_intr_master[lchip_idx].group_count; group_idx++)
        {
            if(irq == g_usw_intr_master[lchip_idx].group[group_idx].irq)
            {
                *(chip_array+valid_idx) = lchip_idx;
                valid_idx++;
                break;
            }
        }
    }
   *valid_chips = valid_idx;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_dispatch_func_pcs_link(uint8 lchip, uint32 intr, uint32* status)
{
    uint8 bit = 0;
    uint8 cnt = 0;
	uint32 tbl_id = 0;
    uint32 sub_tbl_id = 0;
	uint32 bmp[CTC_INTR_STAT_SIZE] = {0};
    sys_intr_t* p_intr = NULL;

    if (intr == SYS_INTR_PCS_LINK_31_0)
    {
         cnt = 3;
		 tbl_id = Hss12GUnitWrapperInterruptFunc0_t;
    }
    else
    {
         cnt = 1;
		 tbl_id = Hss28GUnitWrapperInterruptFunc_t;
    }


   p_intr = &(g_usw_intr_master[lchip].intr[intr]);

    for (bit = 0; bit < cnt; bit++)
    {
        if (!CTC_IS_BIT_SET(status[0], bit))
        {
            continue;
        }

		sub_tbl_id = tbl_id+ bit;

        _sys_usw_interrupt_get_status_common(lchip, sub_tbl_id, bmp);

        _sys_usw_interrupt_clear_status_common(lchip, intr, bmp, sub_tbl_id);

        p_intr->isr(lchip, sub_tbl_id, bmp);

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_dispatch_func(uint8 lchip, uint32 intr)
{
    sys_intr_type_t type;
    sys_intr_t* p_intr = NULL;
    uint32 status[CTC_INTR_STAT_SIZE] = {0};
    uint8 is_event = 0;

    LCHIP_CHECK(lchip);

    sal_memset(&type, 0, sizeof(type));
    type.intr = intr;
    type.sub_intr = INVG;
    type.low_intr = INVG;

    /*get intr status*/
    sys_usw_interrupt_get_status(lchip, &type, status);

    if(status[0] == 0 && status[1] == 0 && status[2] == 0)
    {
        /*clear SupIntrFunc*/
        CTC_ERROR_RETURN(sys_usw_interrupt_clear_status(lchip, &type, status));
        return CTC_E_NONE;
    }
    is_event = _sys_usw_interrupt_check_event_intr(lchip, &type);
    if (is_event)
    {
        CTC_ERROR_RETURN(sys_usw_interrupt_clear_status(lchip, &type, status));
    }

    p_intr = &(g_usw_intr_master[lchip].intr[type.intr]);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"intr: %d, count: %d\n", type.intr, p_intr->occur_count);
    if (p_intr->valid)
    {
        p_intr->occur_count++;
        if (p_intr->isr)
        {
            SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Dispatch interrupt intr: %d, count: %d\n", type.intr, p_intr->occur_count);

            if (!DRV_IS_DUET2(lchip) &&
                (type.intr == SYS_INTR_PCS_LINK_31_0 || type.intr == SYS_INTR_PCS_LINK_47_32))
            {
                _sys_usw_interrupt_dispatch_func_pcs_link(lchip, type.intr, status);
            }
            else
            {
                p_intr->isr(lchip, type.intr, status);
            }
        }
    }

    if (!is_event)
    {
        CTC_ERROR_RETURN(sys_usw_interrupt_clear_status(lchip, &type, status));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_dispatch_abnormal(uint8 lchip, uint32 intr)
{
    sys_intr_type_t type;
    sys_intr_t* p_intr = NULL;
    uint32 status[CTC_INTR_STAT_SIZE] = {0};

    LCHIP_CHECK(lchip);

    type.intr = intr;
    type.sub_intr = INVG;
    type.low_intr = INVG;

    /*get intr status*/
    CTC_ERROR_RETURN(sys_usw_interrupt_get_status(lchip, &type, status));

    if(status[0] == 0)
    {
        return CTC_E_NONE;
    }

    p_intr = &(g_usw_intr_master[lchip].intr[type.intr]);
    if (p_intr->valid)
    {
        p_intr->occur_count++;
        if (p_intr->isr)
        {
            SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Dispatch interrupt intr: %d, count: %d\n", type.intr, p_intr->occur_count);
            p_intr->isr(lchip, type.intr, status);
        }
    }

    CTC_ERROR_RETURN(sys_usw_interrupt_clear_status(lchip, &type, status));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_second_dispatch(uint8 lchip, sys_intr_group_t* p_group)
{
    _sys_usw_interrupt_set_mapping(lchip, p_group, FALSE);

    /*for DMA specific*/
    _sys_usw_interrupt_dispatch_func(lchip, SYS_INTR_DMA);

    CTC_ERROR_RETURN(sal_sem_give(p_group->p_sync_sem));
    return CTC_E_NONE;
}
/**
 @brief Dispatch interrupt of a group
*/
STATIC void
_sys_usw_interrupt_dispatch(void* p_data)
{
    uint8 lchip = 0;
    uint8 occur_intr_lchip[CTC_MAX_LOCAL_CHIP_NUM] = {0};
    uint32 irq = *((uint32*)p_data);
    sys_intr_group_t* p_group = NULL;
    uint32 cmd = 0;
    uint32 status[CTC_INTR_STAT_SIZE];
    uint8 lchip_idx = 0;
    uint8 lchip_num = 0;
    uint8 valid_chips = 0;
    uint8 group_cnt = 0;
    uint8  occur_intr_group_id[SYS_USW_MAX_INTR_GROUP] = {0};
    uint8 i = 0;
    sys_intr_type_t type;
    uint8 intr_idx = 0;
    uint8 irq_index = 0;
    uint8 max_intr_idx = 0;

    sal_memset(&type,0,sizeof(type));

    /* get lchip num */
    lchip_num = sys_usw_get_local_chip_num();

    /* init lchip=0xff, 0xff is invalid value */
    for (lchip_idx=0; lchip_idx<CTC_MAX_LOCAL_CHIP_NUM; lchip_idx++)
    {
        occur_intr_lchip[lchip_idx] = 0xff;
    }
    /* get which chip occur intrrupt, and get the group_id */
    _sys_usw_interrupt_get_chip(irq, lchip_num, occur_intr_lchip,&valid_chips);

    /* dispatch by chip */
    for(lchip_idx=0; lchip_idx<valid_chips; lchip_idx++)
    {
        lchip = occur_intr_lchip[lchip_idx];

        if(sys_usw_chip_check_active(lchip))
        {
           continue;
        }

        INTR_LOCK;
        if(!g_usw_intr_master[lchip].init_done)
        {
           INTR_UNLOCK;
            continue;
        }
        /* get which group coour interrupt */
        _sys_usw_interrupt_get_group_id(lchip, irq, &group_cnt, occur_intr_group_id);

        /*mask IRQ*/
        p_group = &(g_usw_intr_master[lchip].group[occur_intr_group_id[0]]);
        irq_index = p_group->irq_idx;
        _sys_usw_interrupt_set_sup_op(lchip, p_group->irq_idx, INTR_INDEX_MASK_SET);

#ifdef EMULATION_ENV

        /*special for emulation, need scan all group */
        for (i=0;i<g_usw_intr_master[lchip].group_count;++i)
        {
            p_group = &(g_usw_intr_master[lchip].group[i]);

#else
        for(i=0;i<group_cnt;++i)
        {
            p_group = &(g_usw_intr_master[lchip].group[occur_intr_group_id[i]]);
#endif
            /* 1. get sup interrupt status */
            cmd = DRV_IOR(SupCtlIntrVec_t, DRV_ENTRY_FLAG);
            (DRV_IOCTL(lchip, INTR_INDEX_VAL_SET, cmd, &status));

            /* get groups mask */
            status[0] = (status[0] & p_group->intr_bmp);
            if(0 == status[0])
            {
                continue;
            }
            p_group->occur_count++;

            SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Dispatch interrupt group: %d, IRQ: %d\n", p_group->group, p_group->irq);

            if(!p_group->p_sync_task)
            {
                if (status[0] & CTC_INTR_T2B(SYS_INTR_CHIP_FATAL))
                {
                    _sys_usw_interrupt_dispatch_abnormal(lchip, SYS_INTR_CHIP_FATAL);
                }

                if (status[0] & CTC_INTR_T2B(SYS_INTR_CHIP_NORMAL))
                {
                    _sys_usw_interrupt_dispatch_abnormal(lchip, SYS_INTR_CHIP_NORMAL);
                }

                /*Process sup level function interrupt*/
                max_intr_idx = DRV_IS_DUET2(lchip)?SYS_INTR_DMA:SYS_INTR_GPIO;
                for(intr_idx = SYS_INTR_FUNC_PTP_TS_CAPTURE_FIFO; intr_idx <= max_intr_idx; ++intr_idx)
                {
                    if(status[0] & CTC_INTR_T2B(intr_idx))
                    {
                        _sys_usw_interrupt_dispatch_func(lchip, intr_idx);
                    }
                }
            }
            else if(p_group->p_sync_task) /* second dispatch */
            {
                _sys_usw_interrupt_second_dispatch(lchip, p_group);
            }
        }
        /* before release cpu interrupt, reset chip interrupt */
        _sys_usw_interrupt_set_sup_op(lchip, irq_index, INTR_INDEX_VAL_RESET);
        INTR_UNLOCK;
    }

    if (g_dal_op.interrupt_set_en)
    {
        g_dal_op.interrupt_set_en(irq, TRUE);
    }

    for(lchip_idx=0; lchip_idx<valid_chips; lchip_idx++)
    {
        lchip = occur_intr_lchip[lchip_idx];
        INTR_LOCK;
        if(!g_usw_intr_master[lchip].init_done )
        {
           INTR_UNLOCK;
            continue;
        }
        p_group = &(g_usw_intr_master[lchip].group[occur_intr_group_id[0]]);
        /* mask sup interrupt */
       _sys_usw_interrupt_set_sup_op(lchip, p_group->irq_idx, INTR_INDEX_MASK_RESET);
       INTR_UNLOCK;
    }

    return;

}

STATIC int32
_sys_usw_interrupt_release_fatal(uint8 lchip, sys_intr_type_t* p_type, uint32 enable)
{
    uint32 tbl_id = 0;
    uint32 tbl_id_low = 0;
    uint32 cmd = 0;
    int32 ret = CTC_E_NONE;
    uint32 value_a = 0;
    uint32 mask[CTC_INTR_STAT_SIZE];

    CTC_BMP_INIT(mask);
    SYS_INTETRUPT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* relase mask of all fatal interrupt */
    INTR_SUB_TYPE_CHECK(p_type->sub_intr, SYS_INTR_SUB_FATAL_MAX);
    INTR_SUB_TYPE_CHECK(p_type->low_intr, SYS_INTR_LOW_FATAL_MAX);

    /* now only support mask */
    if (!enable)
    {
        ret = CTC_E_NOT_SUPPORT;
        return ret;
    }

    tbl_id = g_intr_sub_reg_sup_abnormal[SYS_INTR_CHIP_FATAL][p_type->sub_intr];

    if(p_type->sub_intr > 2 && p_type->sub_intr < SYS_INTR_SUB_FATAL_PCIE_INTF)
    {
        tbl_id_low =  g_intr_low_reg_sub_fatal[lchip][p_type->sub_intr][p_type->low_intr];
    }

    if(tbl_id_low != 0)
    {
        /* clear mask of low-level interrupt */
        cmd = DRV_IOR(tbl_id_low, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, mask));
        cmd = DRV_IOW(tbl_id_low, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, mask));
    }

    if(tbl_id != 0)
    {
        /* clear mask of sub-level interrupt */
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, mask));
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, mask));
    }

    /* clear mask of sup-level interrupt */
    value_a = (1<<p_type->sub_intr);
    cmd = DRV_IOW(SupIntrFatal_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, &value_a));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_set_interrupt_en(uint8 lchip, uint32 value)
{
    uint32 cmd = 0;
    SupIntrCtl_m intr_ctl;

    cmd = DRV_IOR(SupIntrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &intr_ctl));
    if (value)
    {
        SetSupIntrCtl(V, funcIntrEn_f, &intr_ctl, 1);
        SetSupIntrCtl(V, interruptEn_f, &intr_ctl, 1);
        if(g_usw_intr_master[lchip].intr_mode)
        {
            SetSupIntrCtl(V, padIntrEn_f, &intr_ctl, 0);
            SetSupIntrCtl(V, pcie0IntrEn_f, &intr_ctl, 1);
            SetSupIntrCtl(V, pcieIntrEn_f, &intr_ctl, 1);
        }
        else
        {
            SetSupIntrCtl(V, padIntrEn_f, &intr_ctl, 1);
            SetSupIntrCtl(V, pcie0IntrEn_f, &intr_ctl, 0);
            SetSupIntrCtl(V, pcieIntrEn_f, &intr_ctl, 0);
        }
    }
    else
    {
        SetSupIntrCtl(V, funcIntrEn_f, &intr_ctl, 0);
        SetSupIntrCtl(V, interruptEn_f, &intr_ctl, 0);
        SetSupIntrCtl(V, padIntrEn_f, &intr_ctl, 0);
        SetSupIntrCtl(V, pcie0IntrEn_f, &intr_ctl, 0);
        SetSupIntrCtl(V, pcieIntrEn_f, &intr_ctl, 0);
    }
    cmd = DRV_IOW(SupIntrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &intr_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_release_normal(uint8 lchip, sys_intr_type_t* p_type, uint32 enable)
{
    uint32 tbl_id = 0;
    uint32 tbl_id_low = 0;
    uint32 cmd = 0;
    int32 ret = CTC_E_NONE;
    uint32 value_a = 0;
    uint32 mask[CTC_INTR_STAT_SIZE] = {0};

    CTC_BMP_INIT(mask);
    SYS_INTETRUPT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* relase mask of all fatal interrupt */
    INTR_SUB_TYPE_CHECK(p_type->sub_intr, SYS_INTR_SUB_NORMAL_MAX);
    INTR_SUB_TYPE_CHECK(p_type->low_intr, SYS_INTR_LOW_NORMAL_MAX);

    /* now only support mask */
    if (!enable)
    {
        ret = CTC_E_NOT_SUPPORT;
        return ret;
    }
    tbl_id = g_intr_sub_reg_sup_abnormal[SYS_INTR_CHIP_NORMAL][p_type->sub_intr];
    tbl_id_low =  g_intr_low_reg_sub_normal[lchip][p_type->sub_intr][p_type->low_intr];

    if(tbl_id_low != 0 && p_type->sub_intr > 2)
    {
        /* clear mask of low-level interrupt */
        cmd = DRV_IOR(tbl_id_low, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, mask));
        cmd = DRV_IOW(tbl_id_low, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, mask));
    }

    if(tbl_id != 0 && tbl_id != RlmCsCtlInterruptNormal_t && tbl_id != RlmHsCtlInterruptNormal_t)
    {
        /* clear mask of sub-level interrupt */
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, mask));

        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, mask));
    }

    /* clear mask of sup-level interrupt */
    value_a = (1<<p_type->sub_intr);
    cmd = DRV_IOW(SupIntrNormal_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, &value_a));

    /* clear mask of sub-level interrupt */
    mask[0] = 0xc000f080;
    mask[1] = 0x07;

    cmd = DRV_IOW(DmaCtlInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, mask);
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_interrupt_init_reg(uint8 lchip)
{
    sys_intr_group_t* p_group = NULL;
    ds_t map_ctl;
    SupIntrGenCtl_m gen_ctl;
    uint32 cmd = 0;
    uint32 i = 0;
    uint32 j = 0;
    uint32 mask_set = 0;
    uint32 unmask_set = 0;
    uint32 value_set = 0;
    uint8 step = 0;
    sys_intr_type_t type;
    drv_work_platform_type_t platform_type;
    uint32 lp_status[2] = {0};
    host_type_t byte_order;
    Pcie0IntrGenCtl_m pcie_intr_ctl;
    uint32 value[4] = {0};
    RlmHsCtlInterruptNormal_m hs_normal;
    RlmCsCtlInterruptNormal_m cs_normal;
    PcieIntfInterruptNormal_m pcie_normal;
    NetTxInterruptNormal_m    nettx_normal;
    QuadSgmacInterruptNormal0_m quadsgmac_normal;
    Hss12GUnitWrapperInterruptFunc0_m hss_12g;
    Hss28GUnitWrapperInterruptFunc_m hss_28g;

    SYS_INTETRUPT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(map_ctl, 0, sizeof(map_ctl));
    sal_memset(&quadsgmac_normal, 0, sizeof(QuadSgmacInterruptNormal0_m));
    sal_memset(&gen_ctl, 0, sizeof(gen_ctl));

    mask_set = 0xFF;

    CTC_ERROR_RETURN(drv_get_platform_type(lchip, &platform_type));

    for (i = 0; i < SYS_USW_MAX_INTR_GROUP; i++)
    {
        p_group = &(g_usw_intr_master[lchip].group[i]);
        if (p_group->group < 0)
        {
            continue;
        }

        if (p_group->group >= 0)
        {
            CTC_BIT_UNSET(mask_set, p_group->irq_idx);
        }

        switch (p_group->irq_idx)
        {
        case 0:
            value_set = GetSupIntrMapCtl(V, supIntrMap0_f, &map_ctl);
            value_set |= p_group->intr_bmp;
            SetSupIntrMapCtl(V, supIntrMap0_f, &map_ctl, value_set);
            break;

        case 1:
            value_set = GetSupIntrMapCtl(V, supIntrMap1_f, &map_ctl);
            value_set |= p_group->intr_bmp;
            SetSupIntrMapCtl(V, supIntrMap1_f, &map_ctl, value_set);
            break;

        case 2:
            value_set = GetSupIntrMapCtl(V, supIntrMap2_f, &map_ctl);
            value_set |= p_group->intr_bmp;
            SetSupIntrMapCtl(V, supIntrMap2_f, &map_ctl, value_set);
            break;

        case 3:
            value_set = GetSupIntrMapCtl(V, supIntrMap3_f, &map_ctl);
            value_set |= p_group->intr_bmp;
            SetSupIntrMapCtl(V, supIntrMap3_f, &map_ctl, value_set);
            break;

        default:
            break;
        }
    }

    unmask_set = (~mask_set)&0xff;

    SetSupIntrGenCtl(V, supIntrMaskSet_f, &gen_ctl, mask_set);
    SetSupIntrGenCtl(V, supIntrMaskClr_f, &gen_ctl, unmask_set);


    cmd = DRV_IOW(SupIntrMapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &map_ctl));

    cmd = DRV_IOW(SupIntrGenCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &gen_ctl));

    /* enable msi byte order for little endian CPU */
    if(g_usw_intr_master[lchip].intr_mode)
    {
        byte_order = drv_get_host_type(lchip);
        if (byte_order == HOST_LE)
        {
            cmd = DRV_IOR(Pcie0IntrGenCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcie_intr_ctl));

            SetPcie0IntrGenCtl(V, cfgMsiByteOrder_f, &pcie_intr_ctl, 1);

            cmd = DRV_IOW(Pcie0IntrGenCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &pcie_intr_ctl));
        }
    }

    /* release fatal interrupt mask */
    for (i = 0; i < SYS_INTR_SUB_FATAL_MAX; i++)
    {
        for (j = 0; j < SYS_INTR_LOW_FATAL_MAX; j++)
        {
            type.intr = SYS_INTR_CHIP_FATAL;
            type.sub_intr = i;
            type.low_intr = j;
            _sys_usw_interrupt_release_fatal(lchip, &type, 1);
        }
    }

    /* release normal interrupt mask */
    for (i = 0; i < SYS_INTR_SUB_NORMAL_MAX; i++)
    {
        for (j = 0; j < SYS_INTR_LOW_NORMAL_MAX; j++)
        {
            type.intr = SYS_INTR_CHIP_NORMAL;
            type.sub_intr = i;
            type.low_intr = j;
            _sys_usw_interrupt_release_normal(lchip, &type, 1);
        }
    }

#define SYS_DATAPATH_HSS_STEP(serdes_id, step)                     \
if(((serdes_id) >= 24) && ((serdes_id) <= 39))                \
{                                                              \
    step = ((serdes_id)/4 - 6) + (3 - (serdes_id)%4)*4;       \
}                                                              \
else                                                           \
{                                                              \
    step = (serdes_id)/8 + (7 - (serdes_id)%8)*3 + 16;        \
}

    sal_memset(&hs_normal, 0, sizeof(hs_normal));
    sal_memset(&cs_normal, 0, sizeof(cs_normal));
    sal_memset(&hss_12g, 0, sizeof(hss_12g));
    sal_memset(&hss_28g, 0, sizeof(hss_28g));
    for (i = 0; i< 256; i++)
    {
        uint8 serdes_idx;
        AnethInterruptNormal0_m aneth_normal;
        sys_dmps_serdes_info_t  serdes_info;

        sal_memset(&serdes_info, 0, sizeof(sys_dmps_serdes_info_t));
        CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, i, SYS_DMPS_PORT_PROP_SERDES, (void *)&serdes_info));

        if(serdes_info.serdes_mode == 0xFF)
        {
            continue;
        }

        for(serdes_idx=0; serdes_idx < serdes_info.serdes_num; serdes_idx++)
        {
            if (DRV_IS_DUET2(lchip))
            {
                uint8 offset;
                SYS_DATAPATH_HSS_STEP(serdes_info.serdes_id[serdes_idx], offset)
                sal_memset(&aneth_normal, 0xFF, sizeof(aneth_normal));
                cmd = DRV_IOW(AnethInterruptNormal0_t+offset, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, &aneth_normal);

                sal_memset(&aneth_normal, 0, sizeof(aneth_normal));
                cmd = DRV_IOW(AnethInterruptNormal0_t+offset, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, INTR_INDEX_VAL_RESET, cmd, &aneth_normal);

                if((serdes_info.serdes_id[serdes_idx] >= 24) && (serdes_info.serdes_id[serdes_idx] <= 39))
                {
                    CTC_BMP_SET((uint32*)&cs_normal, serdes_info.serdes_id[serdes_idx]-16);
                }
                else
                {
                    CTC_BMP_SET((uint32*)&hs_normal, serdes_info.serdes_id[serdes_idx]+42);
                }
            }
            else
            {
                uint8 tbl_offset = 0;
                uint8 field_offset = 0;

                if(serdes_info.serdes_id[serdes_idx] < 24)
                {
                    tbl_offset = (serdes_info.serdes_id[serdes_idx])/8;
                    field_offset = (serdes_info.serdes_id[serdes_idx])%8;
                    CTC_BMP_SET((uint32*)&hss_12g, 64+6*field_offset);
                    CTC_BMP_SET((uint32*)&hss_12g, 65+6*field_offset);
                    CTC_BMP_SET((uint32*)&hss_12g, 66+6*field_offset);
                }
                else
                {
                    tbl_offset = (serdes_info.serdes_id[serdes_idx]-24)/8;
                    field_offset = (serdes_info.serdes_id[serdes_idx]-24)%8;
                    CTC_BMP_SET((uint32*)&hss_28g, 16+3*field_offset);
                    CTC_BMP_SET((uint32*)&hss_28g, 17+3*field_offset);
                    CTC_BMP_SET((uint32*)&hss_28g, 18+3*field_offset);
                }
                cmd = DRV_IOW(Hss12GUnitWrapperInterruptFunc0_t+tbl_offset, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, &hss_12g);
                cmd = DRV_IOW(Hss28GUnitWrapperInterruptFunc_t+tbl_offset, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, &hss_28g);
            }
        }
    }

    cmd = DRV_IOW(RlmCsCtlInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, &cs_normal);
    cmd = DRV_IOW(RlmHsCtlInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, &hs_normal);

    /* release function interrupt mask*/
    cmd = DRV_IOR(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, &value_set);
    cmd = DRV_IOW(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, &value_set);



    cmd = DRV_IOR(PcieIntfInterruptFunc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, &value_set);
    cmd = DRV_IOW(PcieIntfInterruptFunc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, &value_set);

    cmd = DRV_IOR(RlmCsCtlInterruptFunc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, value);
    cmd = DRV_IOW(RlmCsCtlInterruptFunc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, value);

    cmd = DRV_IOR(RlmHsCtlInterruptFunc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, value);
    cmd = DRV_IOW(RlmHsCtlInterruptFunc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, value);

    cmd = DRV_IOR(SupIntrFunc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, &value_set);
    value_set = value_set & (~(1<<6));
    cmd = DRV_IOW(SupIntrFunc_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, &value_set);

    value_set = 1;
    cmd = DRV_IOW(BufRetrvInterruptNormal_t, BufRetrvInterruptNormal_dsBufRetrvExcpEccError_f);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, &value_set);

    sal_memset(&nettx_normal, 0, sizeof(NetTxInterruptNormal_m));
    SetNetTxInterruptNormal(V, rxNoEopErrorIntr0_f, &nettx_normal, 1);
    SetNetTxInterruptNormal(V, rxNoEopErrorIntr1_f, &nettx_normal, 1);
    SetNetTxInterruptNormal(V, rxNoEopErrorIntr2_f, &nettx_normal, 1);
    SetNetTxInterruptNormal(V, rxNoEopErrorIntr3_f, &nettx_normal, 1);
    SetNetTxInterruptNormal(V, rxNoSopErrorIntr0_f, &nettx_normal, 1);
    SetNetTxInterruptNormal(V, rxNoSopErrorIntr1_f, &nettx_normal, 1);
    SetNetTxInterruptNormal(V, rxNoSopErrorIntr2_f, &nettx_normal, 1);
    SetNetTxInterruptNormal(V, rxNoSopErrorIntr3_f, &nettx_normal, 1);
    cmd = DRV_IOW(NetTxInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, &nettx_normal);

    /*mask dma0AxiRdSizeError for RTL bug 110000*/
    if(DRV_IS_TSINGMA(lchip))
    {
        DmaCtlInterruptNormal_m dma_normal;
        sal_memset(&dma_normal, 0, sizeof(dma_normal));
        SetDmaCtlInterruptNormal(V, dma0AxiRdSizeError_f, &dma_normal, 1);
        SetDmaCtlInterruptNormal(V, dma1AxiRdSizeError_f, &dma_normal, 1);
        SetDmaCtlInterruptNormal(V, dma2AxiRdSizeError_f, &dma_normal, 1);
        cmd = DRV_IOW(DmaCtlInterruptNormal_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, &dma_normal);
    }

    cmd = DRV_IOR(PcieIntfInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, &pcie_normal);
    SetPcieIntfInterruptNormal(V, hssRxAEyeQualityIntr_f, &pcie_normal, 1);
    SetPcieIntfInterruptNormal(V, hssRxBEyeQualityIntr_f, &pcie_normal, 1);
    cmd = DRV_IOW(PcieIntfInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, &pcie_normal);

    /*clear some interrupt */
    cmd = DRV_IOR(Pcie0IpStatus_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, lp_status);
    SetPcie0IpStatus(V, pcie0TldlpErrorRec_f,lp_status, 0);
    cmd = DRV_IOW(Pcie0IpStatus_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, lp_status);

    cmd = DRV_IOR(PcieIntfInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &value_set);
    cmd = DRV_IOW(PcieIntfInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 1, cmd, &value_set);

    cmd = DRV_IOR(PcieIntfInterruptFatal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &value_set);
    cmd = DRV_IOW(PcieIntfInterruptFatal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 1, cmd, &value_set);


    cmd = DRV_IOR(RlmHsCtlInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &hs_normal);
    cmd = DRV_IOW(RlmHsCtlInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 1, cmd, &hs_normal);


    cmd = DRV_IOR(RlmCsCtlInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &cs_normal);
    cmd = DRV_IOW(RlmCsCtlInterruptNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 1, cmd, &cs_normal);

    cmd = DRV_IOR(SupIntrNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &value);
    cmd = DRV_IOW(SupIntrNormal_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 1, cmd, &value);
    /* clear mask pfc normal interrupt*/
    step = QuadSgmacInterruptNormal0_intSgmac1RxPauseLockDetect_f - QuadSgmacInterruptNormal0_intSgmac0RxPauseLockDetect_f;
    for(i = 0; i <= 15; i++)
    {
        for(j = 0; j <= 3; j++)
        {
            SetQuadSgmacInterruptNormal0(V, intSgmac0RxPauseLockDetect_f+j*step, &quadsgmac_normal, 1);
        }
        cmd = DRV_IOW(QuadSgmacInterruptNormal0_t + i, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, &quadsgmac_normal);
    }
    CTC_ERROR_RETURN(_sys_usw_set_interrupt_en(lchip, 1));

    return CTC_E_NONE;
}
#define __ADD_FOR_SECOND_DISPATCH__
STATIC int32
_sys_usw_interrupt_get_irq_info(uint8 lchip, uint8* irq_cnt, sys_intr_irq_info_t* irq_array, uint8 intr_mode)
{
    uint8 i = 0;
    uint8 cnt = 0;
    uint8 tmp_cnt = 0;
    uint8 sort_flag = 1;
    sys_intr_irq_info_t tmp_info;
    sys_intr_irq_info_t per_group_irq_info[SYS_USW_MAX_INTR_GROUP];

    sal_memset(&tmp_info, 0, sizeof(tmp_info));
    sal_memset(per_group_irq_info, 0, sizeof(per_group_irq_info));
    cnt = g_usw_intr_master[lchip].group_count;

    /*for legacy interrupt,only support one irq, so,all the interrupt mappin to one irq*/
    if(intr_mode == 2)
    {
        for(i=0; i<cnt; ++i)
        {
            g_usw_intr_master[lchip].group[i].irq_idx = 0;
            g_usw_intr_master[lchip].group[i].irq = g_usw_intr_master[lchip].group[0].irq;
        }
    }

    for(i=0; i<cnt; ++i)
    {
        per_group_irq_info[i].irq = g_usw_intr_master[lchip].group[i].irq;
        per_group_irq_info[i].irq_idx = g_usw_intr_master[lchip].group[i].irq_idx;
        /*per_group_irq_info's index is group, so only use per_group_irq_info[i].group[0]*/
        per_group_irq_info[i].group[0] = g_usw_intr_master[lchip].group[i].group;
        per_group_irq_info[i].prio = g_usw_intr_master[lchip].group[i].prio;
    }

    /*sort according irq value*/
    tmp_cnt = cnt;
    while(sort_flag)
    {
        sort_flag = 0;
        for(i=1;i<tmp_cnt;++i)
        {
            if(per_group_irq_info[i-1].irq_idx > per_group_irq_info[i].irq_idx)
            {
                sal_memcpy(&tmp_info,per_group_irq_info+i-1,sizeof(sys_intr_irq_info_t));
                sal_memcpy(per_group_irq_info+i-1,per_group_irq_info+i,sizeof(sys_intr_irq_info_t));
                sal_memcpy(per_group_irq_info+i,&tmp_info,sizeof(sys_intr_irq_info_t));

                sort_flag = 1;
            }
        }
        tmp_cnt--;
    }

    /*init the first irq info*/
    sal_memcpy(irq_array,per_group_irq_info,sizeof(sys_intr_irq_info_t));
    irq_array[0].group_count = 1;
    *irq_cnt = 1;

    /*get the highest prio and group count for per irq*/
    for(i=1; i<cnt; i++)
    {
        if(per_group_irq_info[i-1].irq_idx != per_group_irq_info[i].irq_idx)
        {
            (*irq_cnt)++;
            irq_array[*irq_cnt-1].irq = per_group_irq_info[i].irq;
            irq_array[*irq_cnt-1].irq_idx = per_group_irq_info[i].irq_idx;
            irq_array[*irq_cnt-1].prio = per_group_irq_info[i].prio;
            irq_array[*irq_cnt-1].group_count = 1;
            irq_array[*irq_cnt-1].group[0] = per_group_irq_info[i].group[0];
        }
        else
        {
            if(irq_array[*irq_cnt-1].prio > per_group_irq_info[i].prio)
            {
                irq_array[*irq_cnt-1].prio = per_group_irq_info[i].prio;
            }

            irq_array[*irq_cnt-1].group_count++;
            irq_array[*irq_cnt-1].group[(irq_array[*irq_cnt-1].group_count-1)] = per_group_irq_info[i].group[0];
        }
    }

    return CTC_E_NONE;
}

STATIC void
_sys_usw_interrupt_sync_thread(void* param)
{
    uint8 lchip = 0;
    int32 ret = 0;
    int32 prio = 0;
    uint32 cmd = 0;
    uint32 status[CTC_INTR_STAT_SIZE];
    uint8 intr_idx = 0;
    sys_intr_group_t* p_group = (sys_intr_group_t*)param;
    sys_intr_type_t type;

    prio = p_group->prio;
    lchip = p_group->lchip;
    sal_task_set_priority(prio);
    sal_memset(&type,0,sizeof(type));
    while(1)
    {
        ret = sal_sem_take(p_group->p_sync_sem, SAL_SEM_FOREVER);
        if (0 != ret)
        {
            continue;
        }
        SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);
        INTR_LOCK;
        if( !g_usw_intr_master[lchip].init_done)
        { /*the chip is inactive, thread should be exit*/
           INTR_UNLOCK;
           return ;
        }

        /* 1. get sup interrupt status */
        cmd = DRV_IOR(SupCtlIntrVec_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, INTR_INDEX_VAL_SET, cmd, &status);

        /* 2. get groups mask */
        status[0] = (status[0] & p_group->intr_bmp);

        /* 3. process interrupt */
        if (status[0] & CTC_INTR_T2B(SYS_INTR_CHIP_FATAL))
        {
            _sys_usw_interrupt_dispatch_abnormal(lchip, SYS_INTR_CHIP_FATAL);
        }

        if (status[0] & CTC_INTR_T2B(SYS_INTR_CHIP_NORMAL))
        {
            _sys_usw_interrupt_dispatch_abnormal(lchip, SYS_INTR_CHIP_NORMAL);
        }

        /*Process sup level function interrupt*/
        for(intr_idx = SYS_INTR_FUNC_PTP_TS_CAPTURE_FIFO; intr_idx <= SYS_INTR_DMA; ++intr_idx)
        {
            if(status[0] & CTC_INTR_T2B(intr_idx))
            {
                _sys_usw_interrupt_dispatch_func(lchip, intr_idx);
            }
        }

        /* 4. mask sup interrupt */
        _sys_usw_interrupt_set_mapping(lchip, p_group, TRUE);

        INTR_UNLOCK;
    }

    return;
}

STATIC int32
_sys_usw_interrupt_init_thread(uint8 lchip, sys_intr_group_t* p_group)
{
    int32 ret = 0;
    char buffer[32];
    uint8 group_id = 0;
    uint16 prio = 0;
    uint64 cpu_mask = 0;

    group_id = p_group->group;
    prio     = p_group->prio;
    sal_sprintf(buffer, "interrupt_group%d", group_id);

    cpu_mask = sys_usw_chip_get_affinity(lchip, 0);
    ret = sys_usw_task_create(lchip,&p_group->p_sync_task, buffer,
                           SAL_DEF_TASK_STACK_SIZE, prio,SAL_TASK_TYPE_INTERRUPT, cpu_mask, _sys_usw_interrupt_sync_thread, (void*)p_group);
    if (ret < 0)
    {
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;

    }

    sal_memcpy(p_group->thread_desc, buffer, sizeof(buffer));

    return CTC_E_NONE;
}

#define __DEBUG_SHOW__
STATIC int32
_sys_usw_interrupt_dump_intr(uint8 lchip, sys_intr_t* p_intr)
{
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9d %-9d %-9d %-10p %-10s\n", p_intr->group, p_intr->intr, p_intr->occur_count, p_intr->isr, p_intr->desc);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_dump_group(uint8 lchip, sys_intr_group_t* p_group, uint8 is_detail)
{
    sys_intr_t* p_intr = NULL;
    uint32 i = 0;

    if (p_group->group < 0)
    {
        return CTC_E_NONE;
    }

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Group %u: \n", p_group->group);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-30s:%-6u \n", "--Group IRQ", p_group->irq);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-30s:%-6u \n", "--ISR Priority", p_group->prio);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-30s:%-6u \n", "--Occur Count", p_group->occur_count);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-30s:%-6u \n", "--Interrupt Count", p_group->intr_count);

    if (is_detail)
    {
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n%-9s %-9s %-9s %-10s %-10s\n", "Group", "Interrupt", "Count", "ISR", "Desc");
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"----------------------------------------------------\n");

        for (i = 0; i < CTC_UINT32_BITS; i++)
        {
            if (CTC_IS_BIT_SET(p_group->intr_bmp, i))
            {
                p_intr = &(g_usw_intr_master[lchip].intr[i]);
                if (p_intr->valid)
                {
                    _sys_usw_interrupt_dump_intr(lchip, p_intr);
                }
            }
        }
    }

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");

    return CTC_E_NONE;
}

int32
_sys_usw_interrupt_dump_group_reg(uint8 lchip)
{
    ds_t map_ctl;
    ds_t gen_ctl;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 value = 0;

    if (!g_usw_intr_master[lchip].init_done)
    {
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;

    }

    sal_memset(&map_ctl, 0, sizeof(map_ctl));
    sal_memset(&gen_ctl, 0, sizeof(gen_ctl));

    tbl_id = SupIntrMapCtl_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &map_ctl));
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Group - Interrupt Mapping reg (%d, %s)\n", tbl_id, TABLE_NAME(lchip, tbl_id));
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %-8s\n", "Index", "Value");
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"--------------------------------------------------------\n");
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %08X\n", "Valid", 0x3FFFFFFF);

    GetSupIntrMapCtl(A, supIntrMap0_f, &map_ctl, &value);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %08X\n", "Group0", value);
    GetSupIntrMapCtl(A, supIntrMap1_f, &map_ctl, &value);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %08X\n", "Group1", value);
    GetSupIntrMapCtl(A, supIntrMap2_f, &map_ctl, &value);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %08X\n", "Group2", value);
    GetSupIntrMapCtl(A, supIntrMap3_f, &map_ctl, &value);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %08X\n", "Group3", value);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");

    tbl_id = SupIntrGenCtl_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &gen_ctl));
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Group reg (%d, %s)\n", tbl_id, TABLE_NAME(lchip, tbl_id));
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %-8s\n", "Index", "Value");
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"--------------------------------------------------------\n");
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %08X\n", "Valid", 0xFF);
    GetSupIntrGenCtl(A, supIntrValueSet_f, &gen_ctl, &value);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %08X\n", "ValueSet", value);
    GetSupIntrGenCtl(A, supIntrValueClr_f, &gen_ctl, &value);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %08X\n", "ValueReset", value);
    GetSupIntrGenCtl(A, supIntrMaskSet_f, &gen_ctl, &value);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %08X\n", "MaskSet", value);
    GetSupIntrGenCtl(A, supIntrMaskClr_f, &gen_ctl, &value);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %08X\n", "MaskReset", value);

    /* TODO : Pcie0IntrLogCtl_t / Pcie1IntrLogCtl_t / SupIntrCtl_t / SupCtlVec_t */

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_interrupt_dump_one_reg(uint8 lchip, uint32 tbl_id)
{
    uint32 cmd = 0;
    uint32 index = 0;
    uint32 value[INTR_INDEX_MAX][CTC_INTR_STAT_SIZE];
    uint32 valid[CTC_INTR_STAT_SIZE];
    tables_info_t* p_tbl = NULL;
    fields_t* p_fld = NULL;
    uint32 fld_id = 0;
    uint32 i = 0;
    uint32 bits = 0;
    uint32 addr_offset = 0;

    sal_memset(valid, 0, sizeof(valid));
    sal_memset(value, 0, sizeof(value));

     /*SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"reg (%d, %s) desc %s intr [%d, %d]\n", tbl_id, TABLE_NAME(lchip, tbl_id), p_reg->desc,*/
      /*   p_reg->intr_start, (p_reg->intr_start+ p_reg->intr_count - 1));*/

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_VAL_SET, cmd, value[INTR_INDEX_VAL_SET]));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_VAL_RESET, cmd, value[INTR_INDEX_VAL_RESET]));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_SET, cmd, value[INTR_INDEX_MASK_SET]));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, INTR_INDEX_MASK_RESET, cmd, value[INTR_INDEX_MASK_RESET]));

    p_tbl = TABLE_INFO_PTR(lchip, tbl_id);

    for (fld_id = 0; fld_id < p_tbl->field_num; fld_id++)
    {
        p_fld = &(p_tbl->ptr_fields[fld_id]);
        if (p_fld->seg_num)
        {
            for (i = 0; i < p_fld->ptr_seg[0].bits; i++)
            {
                CTC_BIT_SET(valid[p_fld->ptr_seg[0].word_offset], (p_fld->ptr_seg[0].start + i));
                bits++;
            }
        }
    }

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %-8s   %s\n", "Index", "Address", "Value");
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"--------------------------------------------------------\n");

    for (i = 0; i < TABLE_ENTRY_SIZE(lchip, tbl_id)/4; i++)
    {
        /* Valid */
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %-8d   %08X\n", "Valid", bits, valid[i]);

        /* Value */
        for (index = 0; index < INTR_INDEX_MAX; index++)
        {
            SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %08X   %08X\n", _sys_usw_interrupt_index_str(lchip, index),
                          (uint32)(TABLE_DATA_BASE(lchip, tbl_id, addr_offset) + (i * 16) + (index * 4)),
                          value[index][i]);
        }

        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    }

    return CTC_E_NONE;
}
/**
 @brief Display interrupt register values in chip
 type 1 : group
 type 2 : fatal
 type 3 : normal
 type 4 : func
 type 5 : pcie
 type 6 : dma-func
 type 7 : all
*/
int32
sys_usw_interrupt_dump_reg(uint8 lchip, uint32 type)
{
    int32 tbl_id = 0;
    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);
    if(type == 0 || type > 7)
    {
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid type: %d\n", type);
        return CTC_E_INVALID_PARAM;
    }
    if (type == 1 || type == 7)
    {
        _sys_usw_interrupt_dump_group_reg(lchip);
    }

    if (type == 2 || type == 7)
    {
        tbl_id = g_intr_sup_info[SYS_INTR_CHIP_FATAL].tbl_id;
        _sys_usw_interrupt_dump_one_reg(lchip, tbl_id);
    }

    if (type == 3 || type == 7)
    {
        tbl_id = g_intr_sup_info[SYS_INTR_CHIP_NORMAL].tbl_id;
        _sys_usw_interrupt_dump_one_reg(lchip, tbl_id);
    }

    if (type == 4 || type == 7)
    {
        tbl_id = g_intr_func_reg[SYS_INTR_PCS_LINK_31_0-2];
        _sys_usw_interrupt_dump_one_reg(lchip, tbl_id);
        tbl_id = g_intr_func_reg[SYS_INTR_PCS_LINK_47_32-2];
        _sys_usw_interrupt_dump_one_reg(lchip, tbl_id);
    }

    if (type == 5 || type == 7)
    {
        tbl_id = g_intr_func_reg[SYS_INTR_PCIE_BURST_DONE-2];
        _sys_usw_interrupt_dump_one_reg(lchip, tbl_id);
    }

    if (type == 6 || type == 7)
    {
        tbl_id = g_intr_func_reg[SYS_INTR_DMA-2];
        _sys_usw_interrupt_dump_one_reg(lchip, tbl_id);
    }

    return CTC_E_NONE;
}

/**
 @brief Display interrupt database values in software
*/
int32
sys_usw_interrupt_dump_db(uint8 lchip)
{
    uint32 unbind = FALSE;
    uint32 i = 0;

    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Using %s global configuration\n", g_usw_intr_master[lchip].is_default ? "default" : "parameter");
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Group Count:      %d\n", g_usw_intr_master[lchip].group_count);
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Interrupt Count:  %d\n", g_usw_intr_master[lchip].intr_count);

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Registered Interrupts:\n");

    for (i = 0; i < SYS_USW_MAX_INTR_GROUP; i++)
    {
        _sys_usw_interrupt_dump_group(lchip, &(g_usw_intr_master[lchip].group[i]), 1);
    }

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Unregistered Interrupts:\n");

    for (i = 0; i < SYS_INTR_MAX_INTR; i++)
    {
        if (!g_usw_intr_master[lchip].intr[i].valid)
        {
            if (!unbind)
            {
                unbind = TRUE;
                SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-9s %-9s %-9s %-10s %-10s\n", "Group", "Interrupt", "Count", "ISR", "Desc");
                SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"----------------------------------------------------\n");
            }

            if (INVG != g_usw_intr_master[lchip].intr[i].intr)
            {
                _sys_usw_interrupt_dump_intr(lchip, &(g_usw_intr_master[lchip].intr[i]));
            }
        }
    }

    if (!unbind)
    {
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"None\n");
    }

    return CTC_E_NONE;
}

int32
sys_usw_interrupt_parser_type(uint8 lchip, ctc_intr_type_t type)
{

    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);

    if (type.intr == SYS_INTR_CHIP_FATAL)
    {
        if(type.sub_intr >= SYS_INTR_SUB_FATAL_MAX || g_intr_sub_reg_sup_abnormal[type.intr][type.sub_intr] == 0)
        {
            return CTC_E_NONE;
        }

        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Fatal Interrupr:\n    sub-intr-table:%s\n", TABLE_NAME(lchip, g_intr_sub_reg_sup_abnormal[type.intr][type.sub_intr]));

        /*print low intr table*/
        if (type.low_intr >= SYS_INTR_LOW_FATAL_MAX || g_intr_low_reg_sub_fatal[lchip][type.sub_intr][type.low_intr] == 0)
        {
            return CTC_E_NONE;
        }

        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"    low-intr-table:%s\n", TABLE_NAME(lchip, g_intr_low_reg_sub_fatal[lchip][type.sub_intr][type.low_intr]));
    }
    else if (type.intr == SYS_INTR_CHIP_NORMAL)
    {
        if(type.sub_intr >= SYS_INTR_SUB_NORMAL_MAX || g_intr_sub_reg_sup_abnormal[type.intr][type.sub_intr] == 0)
        {
            return CTC_E_NONE;
        }

        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Normal Interrupr:\n    sub-intr-table:%s\n", TABLE_NAME(lchip, g_intr_sub_reg_sup_abnormal[type.intr][type.sub_intr]));

        /*print low intr table*/
        if (type.low_intr >= SYS_INTR_LOW_NORMAL_MAX || g_intr_low_reg_sub_normal[lchip][type.sub_intr][type.low_intr] == 0)
        {
            return CTC_E_NONE;
        }

        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"    low-intr-table:%s\n", TABLE_NAME(lchip, g_intr_low_reg_sub_normal[lchip][type.sub_intr][type.low_intr]));
    }
    else
    {
        SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

int32
sys_usw_interrupt_show_status(uint8 lchip)
{
    uint8 i = 0;

    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n-------------------------Work Mode---------------------\n");

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-30s:%-6s \n", "Interrupt Mode", g_usw_intr_master[lchip].intr_mode?"Msi":"Pin");

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------Group Status---------------------\n");

    for (i = 0; i < SYS_USW_MAX_INTR_GROUP; i++)
    {
        _sys_usw_interrupt_dump_group(lchip, &(g_usw_intr_master[lchip].group[i]), 0);
    }

    return CTC_E_NONE;
}

#define __INTERRUPT_INTERFACE__
/**
 @brief Clear interrupt status
*/
int32
sys_usw_interrupt_clear_status(uint8 lchip, sys_intr_type_t* p_type, uint32* p_bmp)
{
    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);
    CTC_MAX_VALUE_CHECK(p_type->intr, SYS_INTR_DMA);
    if(p_type->intr == SYS_INTR_CHIP_FATAL|| p_type->intr == SYS_INTR_CHIP_NORMAL)
    {
        return _sys_usw_interrupt_clear_status_abnormal(lchip, p_type, p_bmp);
    }
    else
    {
        return _sys_usw_interrupt_clear_status_func(lchip, p_type, p_bmp);
    }

    return CTC_E_NONE;
}

/**
 @brief Get interrupt status
 Notice: Sdk sys level for get interrupt status should use this interface
*/
int32
sys_usw_interrupt_get_status(uint8 lchip, sys_intr_type_t* p_type, uint32* p_bmp)
{
    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);
    CTC_MAX_VALUE_CHECK(p_type->intr, SYS_INTR_DMA);
    if(p_type->intr == SYS_INTR_CHIP_FATAL|| p_type->intr == SYS_INTR_CHIP_NORMAL)
    {
        return _sys_usw_interrupt_get_status_abnormal(lchip, p_type, p_bmp);
    }
    else
    {
        return _sys_usw_interrupt_get_status_func(lchip, p_type, p_bmp);
    }

    return CTC_E_NONE;
}

/**
 @brief Set interrupt enable
 Notice: Sdk sys level for enable interrupt should use this interface
*/
int32
sys_usw_interrupt_set_en(uint8 lchip, sys_intr_type_t* p_type, uint32 enable)
{
    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);
    CTC_MAX_VALUE_CHECK(p_type->intr, SYS_INTR_DMA);
    if(p_type->intr == SYS_INTR_CHIP_FATAL|| p_type->intr == SYS_INTR_CHIP_NORMAL)
    {
        return _sys_usw_interrupt_set_en_abnormal(lchip, p_type, enable);
    }
    else
    {
        return _sys_usw_interrupt_set_en_func(lchip, p_type, enable);
    }

    return CTC_E_NONE;

}
int32
sys_usw_interrupt_set_group_en(uint8 lchip, uint8 enable)
{
    uint8 i = 0;
    uint32 value = enable ? INTR_INDEX_MASK_RESET : INTR_INDEX_MASK_SET;

    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);
    for(i = 0; i < g_usw_intr_master[lchip].group_count; i++)
    {
        _sys_usw_interrupt_set_sup_op(lchip, g_usw_intr_master[lchip].group[i].irq_idx, value);
    }

    return CTC_E_NONE;
}

int32
sys_usw_interrupt_get_group_en(uint8 lchip, uint8* enable)
{
    uint8 i = 0;
    uint32 value = 0;
    SupIntrGenCtl_m intr_mask;
    uint32 cmd = 0;
    uint8 group_en = 0;
    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);

    sal_memset(&intr_mask, 0, sizeof(SupIntrGenCtl_m));
    cmd = DRV_IOR(SupIntrGenCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &intr_mask));

    value = GetSupIntrGenCtl(V, supIntrMaskSet_f, &intr_mask);

    for(i = 0; i < g_usw_intr_master[lchip].group_count; i++)
    {
        if (!(value & (1<<i)))
        {
            group_en = 1;
            break;
        }
    }

    *enable = group_en;

    return CTC_E_NONE;
}
/**
 @brief Register event callback function
*/
int32
sys_usw_interrupt_register_event_cb(uint8 lchip, ctc_interrupt_event_t event, CTC_INTERRUPT_EVENT_FUNC cb)
{
    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);


    /* check */
    switch (event)
    {
        case CTC_EVENT_PTP_TX_TS_CAPUTRE:
            return CTC_E_NOT_SUPPORT;
        case CTC_EVENT_ECC_ERROR:
            CTC_ERROR_RETURN(drv_ser_set_cfg(lchip, DRV_SER_CFG_TYPE_ECC_EVENT_CB,cb));
            break;
        default:
            break;
    }

    g_usw_intr_master[lchip].event_cb[event] = cb;

    return CTC_E_NONE;
}

/**
 @brief get event callback function
*/
int32
sys_usw_interrupt_get_event_cb(uint8 lchip, ctc_interrupt_event_t event, CTC_INTERRUPT_EVENT_FUNC* p_cb)
{
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cb);

    SYS_INTR_INIT_CHECK(lchip);

    *p_cb = g_usw_intr_master[lchip].event_cb[event] ;

    return CTC_E_NONE;
}

/**
 @brief Other module should call this function to register isr
*/
int32
sys_usw_interrupt_register_isr(uint8 lchip, sys_interrupt_type_t type, CTC_INTERRUPT_FUNC cb)
{
    sys_intr_t* p_intr = NULL;

    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);

    p_intr = &(g_usw_intr_master[lchip].intr[type]);
    if (p_intr->valid && !p_intr->isr)
    {
        p_intr->isr = cb;
    }

    return CTC_E_NONE;
}

int32
sys_usw_interrupt_get_info(uint8 lchip, sys_intr_type_t* p_ctc_type, uint32 intr_bit, uint32* p_intr_tbl, uint8* p_action_type, uint8* p_ecc_or_parity)
{
    drv_ecc_intr_tbl_t tbl_db_info;
    LCHIP_CHECK(lchip);
    SYS_INTR_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_ctc_type);
    CTC_PTR_VALID_CHECK(p_intr_tbl);
    CTC_PTR_VALID_CHECK(p_action_type);
    CTC_PTR_VALID_CHECK(p_ecc_or_parity);

    *p_ecc_or_parity = 0;

    if (CTC_INTR_CHIP_NORMAL == p_ctc_type->intr)
    {
        if(p_ctc_type->sub_intr != INVG)
        {
            if(p_ctc_type->low_intr != INVG)
            {
                *p_intr_tbl = g_intr_low_reg_sub_normal[lchip][p_ctc_type->sub_intr][p_ctc_type->low_intr];
            }
            else
            {
                *p_intr_tbl = g_intr_sub_reg_sup_abnormal[p_ctc_type->intr][p_ctc_type->sub_intr];
            }
        }
    }
    else if (CTC_INTR_CHIP_FATAL == p_ctc_type->intr)
    {
        if(p_ctc_type->sub_intr != INVG)
        {
            if(p_ctc_type->low_intr != INVG)
            {
                *p_intr_tbl = g_intr_low_reg_sub_fatal[lchip][p_ctc_type->sub_intr][p_ctc_type->low_intr];
            }
            else
            {
                *p_intr_tbl = g_intr_sub_reg_sup_abnormal[p_ctc_type->intr][p_ctc_type->sub_intr];
            }
        }
    }

    if (MaxTblId_t == *p_intr_tbl)
    {
        return CTC_E_NONE;
    }

    tbl_db_info.intr_tblid = *p_intr_tbl;
    tbl_db_info.intr_bit = intr_bit;

    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"table id:%d, filed  id: %d\n", *p_intr_tbl, intr_bit);
    drv_ser_get_cfg(lchip,DRV_SER_CFG_TYPE_ECC_INTERRUPT_INFO, &tbl_db_info);
    *p_action_type = tbl_db_info.action;
    *p_ecc_or_parity = tbl_db_info.tbl_type;

    return CTC_E_NONE;
}

int32
sys_usw_interrupt_proc_ecc_or_parity(uint8 lchip, uint32 intr_tbl, uint32 intr_bit)
{
    drv_ecc_intr_param_t intr_param;

    uint32 bmp[CTC_INTR_STAT_SIZE] = {0};

    sal_memset(&intr_param, 0, sizeof(drv_ecc_intr_param_t));

    CTC_BMP_SET(bmp, intr_bit);
    intr_param.p_bmp = bmp;
    intr_param.intr_tblid = intr_tbl;
    intr_param.total_bit_num = CTC_INTR_STAT_SIZE * CTC_UINT32_BITS;
    intr_param.valid_bit_count = 1;;
    CTC_ERROR_RETURN(drv_ser_restore(lchip, &intr_param));
    return CTC_E_NONE;
}

/**
 @brief Initialize interrupt module
*/
int32
sys_usw_interrupt_init(uint8 lchip, void* p_global_cfg)
{
    ctc_intr_global_cfg_t* p_global_cfg_param = (ctc_intr_global_cfg_t*)p_global_cfg;
    static sys_intr_global_cfg_t intr_cfg;
    uint32 group_count = 0;
    uint32 intr_count = 0;
    uintptr i = 0;
    uint32 j = 0;
    uint32 intr = 0;
    int32 group_id = 0;
    sys_intr_group_t* p_group = NULL;
    uint8 irq_count = 0;
    sys_intr_irq_info_t irq_array[SYS_USW_MAX_INTR_GROUP];
    sys_intr_t* p_intr = NULL;
    int32 ret = CTC_E_NONE;
    ctc_intr_mapping_t* p_intr_mapping = NULL;
    uint32 intr_idx = 0;
    static ctc_intr_type_t ctc_intr;
    static sys_intr_type_t sys_intr;
    uint8 irq_idx = 0;

    if (g_usw_intr_master[lchip].init_done)
    {
        return CTC_E_NONE;
    }

    sal_memset(&intr_cfg, 0, sizeof(intr_cfg));
    sal_memset(&ctc_intr, 0, sizeof(ctc_intr_type_t));
    sal_memset(&irq_array, 0, sizeof(irq_array));

    for (i = 0; i < SYS_INTR_SUB_NORMAL_MAX; i++)
    {
        g_intr_low_reg_sub_normal[lchip][i] = DRV_IS_DUET2(lchip)? g_intr_low_reg_sub_normal_dt2[i] : g_intr_low_reg_sub_normal_tm[i];
    }

    for (i = 0; i < SYS_INTR_SUB_FATAL_MAX; i++)
    {
        g_intr_low_reg_sub_fatal[lchip][i] = DRV_IS_DUET2(lchip)? g_intr_low_reg_sub_fatal_dt2[i] : g_intr_low_reg_sub_fatal_tm[i];
    }

    /* get default interrupt cfg */
    CTC_ERROR_RETURN(sys_usw_interrupt_get_default_global_cfg(lchip, &intr_cfg));
    /* 1. get global configuration */
    if (p_global_cfg_param)
    {
        CTC_PTR_VALID_CHECK(p_global_cfg_param->p_group);
        CTC_PTR_VALID_CHECK(p_global_cfg_param->p_intr);

        group_count = p_global_cfg_param->group_count;
        intr_count = SYS_INTR_MAX;

        if (group_count > SYS_USW_MAX_INTR_GROUP)
        {
            return CTC_E_INVALID_PARAM;
        }

        /* mapping user define interrupt group */
        sal_memset(intr_cfg.group, 0, SYS_USW_MAX_INTR_GROUP*sizeof(ctc_intr_group_t));
        sal_memcpy(intr_cfg.group, p_global_cfg_param->p_group, group_count*sizeof(ctc_intr_group_t));

        /* init intr_cfg.intr[].group */
        for (intr_idx = SYS_INTR_CHIP_FATAL; intr_idx < SYS_INTR_MAX; intr_idx++)
        {
            intr_cfg.intr[intr_idx].group = INVG;
        }

        /* mapping user define interrupt */
        for (intr_idx = 0; intr_idx < p_global_cfg_param->intr_count; intr_idx++)
        {
            p_intr_mapping = (ctc_intr_mapping_t*)(p_global_cfg_param->p_intr+intr_idx);
            ctc_intr.intr = p_intr_mapping->intr;
            sal_memset(&sys_intr, 0, sizeof(sys_intr_type_t));
            ret = _sys_usw_interrupt_type_mapping(lchip, &ctc_intr, &sys_intr);
            /* interrupt is not used */
            if (ret < 0)
            {
                continue;
            }

            if (sys_intr.intr == SYS_INTR_PCS_LINK_31_0)
            {
                intr_cfg.intr[SYS_INTR_PCS_LINK_31_0].group = p_intr_mapping->group;
                intr_cfg.intr[SYS_INTR_PCS_LINK_47_32].group = p_intr_mapping->group;
            }
            else if(sys_intr.intr == SYS_INTR_FUNC_MDIO_XG_CHANGE_0)
            {
                if (DRV_IS_DUET2(lchip))
                {
                    intr_cfg.intr[SYS_INTR_FUNC_MDIO_CHANGE_0].group = p_intr_mapping->group;
                    intr_cfg.intr[SYS_INTR_FUNC_MDIO_CHANGE_1].group = p_intr_mapping->group;
                }

                intr_cfg.intr[SYS_INTR_FUNC_MDIO_XG_CHANGE_0].group = p_intr_mapping->group;
                intr_cfg.intr[SYS_INTR_FUNC_MDIO_XG_CHANGE_1].group = p_intr_mapping->group;
            }
            else if(sys_intr.intr == SYS_INTR_FUNC_IPFIX_USEAGE_OVERFLOW)
            {
                if (!DRV_IS_DUET2(lchip))
                {
                    intr_cfg.intr[SYS_INTR_FUNC_EPE_IPFIX_USEAGE_OVERFLOW].group = p_intr_mapping->group;
                }

                intr_cfg.intr[SYS_INTR_FUNC_IPFIX_USEAGE_OVERFLOW].group = p_intr_mapping->group;
            }
            else if((!DRV_IS_DUET2(lchip)) && sys_intr.intr == SYS_INTR_FUNC_STMCTL_STATE)
            {
               intr_cfg.intr[SYS_INTR_FUNC_STMCTL_STATE].group = p_intr_mapping->group;
            }
            else
            {
                intr_cfg.intr[sys_intr.intr].group = p_intr_mapping->group;
            }

            if (sys_intr.slice_id == 2)
            {
                intr_cfg.intr[sys_intr.intr+1].group = p_intr_mapping->group;
            }
        }

        intr_cfg.group_count = group_count;
        intr_cfg.intr_count = intr_count;

        /* get interrupt mode */
        intr_cfg.intr_mode = p_global_cfg_param->intr_mode;
    }
    intr_cfg.intr[SYS_INTR_FUNC_STATS_STATUS_ADDR].group = INVG;
    sal_memset(&g_usw_intr_master[lchip], 0, sizeof(sys_intr_master_t));

    /* 3. create mutex for intr module */
    ret = sal_mutex_create(&(g_usw_intr_master[lchip].p_intr_mutex));
    if (ret || (!g_usw_intr_master[lchip].p_intr_mutex))
    {
        ret = CTC_E_NO_MEMORY;
        return ret;
    }

    /* 4. init interrupt & group field of master */
    for (intr = 0; intr < SYS_INTR_MAX_INTR; intr++)
    {
        g_usw_intr_master[lchip].intr[intr].group = INVG;
        g_usw_intr_master[lchip].intr[intr].intr = INVG;
    }

    for (group_id = 0; group_id < SYS_USW_MAX_INTR_GROUP; group_id++)
    {
        g_usw_intr_master[lchip].group[group_id].group = INVG;
    }

    /* 5. init interrupt field of master */
    for (i = 0; i < intr_cfg.intr_count; i++)
    {
        intr = intr_cfg.intr[i].intr;

        p_intr = &(g_usw_intr_master[lchip].intr[intr]);
        p_intr->group = intr_cfg.intr[i].group;
        p_intr->intr = intr_cfg.intr[i].intr; /*sys_usw_intr_default[i].intr*/
        p_intr->isr = intr_cfg.intr[i].isr;   /*sys_usw_intr_default[i].isr*/
        sal_strncpy(p_intr->desc, intr_cfg.intr[i].desc, CTC_INTR_DESC_LEN);
    }

    for (i = 0; i < intr_cfg.group_count; i++)
    {
        irq_idx = intr_cfg.group[i].group;
        if (irq_idx >= SYS_USW_MAX_INTR_GROUP )
        {
            continue;
        }
        p_group = &(g_usw_intr_master[lchip].group[i]);
        p_group->group = i;
        p_group->irq = intr_cfg.group[i].irq;
        p_group->prio = intr_cfg.group[i].prio;
        p_group->lchip = lchip;
        p_group->irq_idx = irq_idx;
        sal_strncpy(p_group->desc, intr_cfg.group[i].desc, CTC_INTR_DESC_LEN);
        p_group->intr_count = 0;

        /* register interrupts to group */
        for (j = 0; j < SYS_INTR_MAX_INTR; j++)
        {
            if (g_usw_intr_master[lchip].intr[j].group == p_group->group)
            {
                g_usw_intr_master[lchip].intr[j].valid = TRUE;
                p_group->intr_count++;
                CTC_BIT_SET(p_group->intr_bmp, g_usw_intr_master[lchip].intr[j].intr);
            }
        }

        g_usw_intr_master[lchip].group_count++;
    }

    /* 6. init other field of master */
    g_usw_intr_master[lchip].group_count = intr_cfg.group_count;
    g_usw_intr_master[lchip].is_default = (p_global_cfg_param) ? FALSE : TRUE;
    g_usw_intr_master[lchip].intr_count = intr_cfg.intr_count;
    g_usw_intr_master[lchip].intr_mode = intr_cfg.intr_mode;

    /* get irq info */
    _sys_usw_interrupt_get_irq_info(lchip, &irq_count, irq_array, intr_cfg.intr_mode);
    g_usw_intr_master[lchip].irq_count = irq_count;

    /* 7. init other field of master */
    CTC_ERROR_RETURN(_sys_usw_interrupt_init_reg(lchip));

    /* for msi interrupt, enable msi capbility */
    if (intr_cfg.intr_mode == 1 )
    {
        if (g_dal_op.interrupt_set_msi_cap)
        {
            ret = g_dal_op.interrupt_set_msi_cap(lchip, TRUE, g_usw_intr_master[lchip].irq_count);
            if (ret)
            {
                SYS_INTETRUPT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "enable msi failed!!\n");
                return ret;
            }
        }

    }

    if((intr_cfg.intr_mode > 0 && g_dal_op.interrupt_get_irq)
        || (g_dal_op.soc_active))
    {
        uint16 irqs[8] = {0};
        uint8  irq_num = 0;

        ret = g_dal_op.interrupt_get_irq(lchip, intr_cfg.intr_mode, irqs, &irq_num);
        if (ret)
        {
            SYS_INTETRUPT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "get irq failed!!\n");
            return ret;
        }

        /* update per group's irq */
        for (i = 0; i < g_usw_intr_master[lchip].group_count; i++)
        {
            irq_idx = g_usw_intr_master[lchip].group[i].irq_idx;
            g_usw_intr_master[lchip].group[i].irq = irqs[irq_idx];
        }

        /*update real irq value*/
        for(i=0; i<irq_count; i++)
        {
            irq_array[i].irq = irqs[i];
        }
    }

    /* init thread per group. but if the irq have only one group, should not init thread */
    for(i = 0; i < irq_count; i++)
    {
        if(irq_array[i].group_count <= 1)
        {
            continue;
        }
        for(j = 0; j < irq_array[i].group_count; j++)
        {
            p_group = &g_usw_intr_master[lchip].group[(irq_array[i].group[j])];
            if(!p_group->p_sync_sem)
            {
                ret = sal_sem_create(&(p_group->p_sync_sem), 0);
                if (ret < 0)
                {
                    SYS_INTR_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			        return CTC_E_NOT_INIT;

                }
                CTC_ERROR_RETURN(_sys_usw_interrupt_init_thread(lchip, p_group));
            }
        }
    }

    /* 8. call DAL function to register IRQ */
    if (g_dal_op.interrupt_register)
    {
        for (i = 0; i < irq_count; i++)
        {
            ret = g_dal_op.interrupt_register(irq_array[i].irq, irq_array[i].prio, _sys_usw_interrupt_dispatch, (void*)(uintptr)(irq_array[i].irq_idx));
            if (ret < 0)
            {
                SYS_INTETRUPT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "register interrupt failed!! irq:%d \n", p_group->irq);
                 /*return CTC_E_INTR_REGISTER_FAIL;*/
            }
        }
    }

    /* clear dma intr for msi */
    if(intr_cfg.intr_mode == 1)
    {
        if (DRV_IS_DUET2(lchip))
        {
            Pcie0IntrLogCtl_m pcie_log_ctl;
            uint32 cmd = DRV_IOR(Pcie0IntrLogCtl_t, DRV_ENTRY_FLAG);
            uint32 value_set =0;

            sal_memset(&pcie_log_ctl, 0, sizeof(pcie_log_ctl));
            DRV_IOCTL(lchip, 0, cmd, &pcie_log_ctl);

            if((GetPcie0IntrLogCtl(V, pcie0IntrLogVec_f, &pcie_log_ctl))&&(GetPcie0IntrLogCtl(V, pcie0IntrReqVec_f, &pcie_log_ctl)))
            {
                /* clear dma fun intr */
                cmd = DRV_IOR(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &value_set);
                cmd = DRV_IOW(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 1, cmd, &value_set);

                cmd = DRV_IOR(SupIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &value_set);
                cmd = DRV_IOW(SupIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 1, cmd, &value_set);

                /* clear log, trigger intr */
                SetPcie0IntrLogCtl(V, pcie0IntrLogVec_f, &pcie_log_ctl, 0);
                cmd = DRV_IOW(Pcie0IntrLogCtl_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &pcie_log_ctl);
            }
        }
        else
        {
            PcieIntrLogCtl_m pcie_log_ctl;
            uint32 cmd = DRV_IOR(PcieIntrLogCtl_t, DRV_ENTRY_FLAG);
            uint32 value_set =0;

            sal_memset(&pcie_log_ctl, 0, sizeof(pcie_log_ctl));
            DRV_IOCTL(lchip, 0, cmd, &pcie_log_ctl);

            if((GetPcieIntrLogCtl(V, pcieIntrLogVec_f, &pcie_log_ctl))||(GetPcieIntrLogCtl(V, pcieIntrReqVec_f, &pcie_log_ctl)))
            {
                /* clear dma fun intr */
                cmd = DRV_IOR(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &value_set);
                cmd = DRV_IOW(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 1, cmd, &value_set);

                cmd = DRV_IOR(SupIntrFatal_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &value_set);
                cmd = DRV_IOW(SupIntrFatal_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 1, cmd, &value_set);

                cmd = DRV_IOR(SupIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &value_set);
                cmd = DRV_IOW(SupIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 1, cmd, &value_set);

                /* clear log, trigger intr */
                SetPcieIntrLogCtl(V, pcieIntrLogVec_f, &pcie_log_ctl, 0);
                SetPcieIntrLogCtl(V, pcieIntrReqVec_f, &pcie_log_ctl, 0);
                cmd = DRV_IOW(PcieIntrLogCtl_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &pcie_log_ctl);

                cmd = DRV_IOR(SupIntrGenCtl_t, SupIntrGenCtl_supIntrValueSet_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_set));
                cmd = DRV_IOW(SupIntrGenCtl_t, SupIntrGenCtl_supIntrValueClr_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_set));
            }
        }
    }
    g_usw_intr_master[lchip].init_done = 1;
    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_CHIP_FATAL, sys_usw_intr_isr_sup_fatal));
    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_CHIP_NORMAL, sys_usw_intr_isr_sup_normal));

    drv_ser_register_hw_reset_cb(lchip, DRV_SER_HW_RESET_CB_TYPE_INTERRUPT, sys_usw_interrupt_reset_hw_cb);

    return CTC_E_NONE;

}

int32
sys_usw_interrupt_deinit(uint8 lchip)
{
    uint32 i = 0;
    sys_intr_group_t* p_group = NULL;
    int32 ret = 0;
    uint8 irq_cnt = 0;
    sys_intr_irq_info_t irq_array[SYS_USW_MAX_INTR_GROUP];

    LCHIP_CHECK(lchip);
    if (!g_usw_intr_master[lchip].init_done)
    {
        return CTC_E_NONE;
    }
    INTR_LOCK;
    g_usw_intr_master[lchip].init_done = 0;
    INTR_UNLOCK;
    /*mask group interrupt*/
    sys_usw_interrupt_set_group_en(lchip, FALSE);

    for (i=0; i< g_usw_intr_master[lchip].group_count; i++)
    {
        p_group = &(g_usw_intr_master[lchip].group[i]);
        if (NULL != p_group->p_sync_sem)
        {
            sal_sem_give(p_group->p_sync_sem);
            sal_task_destroy(p_group->p_sync_task);
            sal_sem_destroy(p_group->p_sync_sem);
        }
    }

    sal_memset(irq_array,0,sizeof(irq_array));
    /* free interrupt */
    if (g_dal_op.interrupt_unregister)
    {
        _sys_usw_interrupt_get_irq_info(lchip, &irq_cnt, irq_array, g_usw_intr_master[lchip].intr_mode);

        for (i = 0; i < irq_cnt; i++)
        {
            ret = g_dal_op.interrupt_unregister(irq_array[i].irq);
            if (ret < 0)
            {
                INTR_UNLOCK;
                return ret;
            }
        }

        /* for msi interrupt */
        if(1 == g_usw_intr_master[lchip].intr_mode)
        {
            if (g_dal_op.interrupt_set_msi_cap)
            {
                ret = g_dal_op.interrupt_set_msi_cap(lchip, FALSE, irq_cnt);
                if (ret)
                {
                    INTR_UNLOCK;
                    SYS_INTETRUPT_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "disable msi failed!!\n");
                    return ret;
                }
            }
        }
    }
    sal_mutex_destroy(g_usw_intr_master[lchip].p_intr_mutex);
    sal_memset(&g_usw_intr_master[lchip], 0, sizeof(sys_intr_master_t));
    return CTC_E_NONE;
}

int32
sys_usw_interrupt_reset_hw_cb(uint8 lchip, void* p_user_data)
{
    LCHIP_CHECK(lchip);
    if (!g_usw_intr_master[lchip].init_done)
    {
        return CTC_E_NONE;
    }

    INTR_LOCK;
    _sys_usw_interrupt_init_reg(lchip);

    /* clear dma intr for msi */
    if(g_usw_intr_master[lchip].intr_mode == 1)
    {
        if (DRV_IS_DUET2(lchip))
        {
            Pcie0IntrLogCtl_m pcie_log_ctl;
            uint32 cmd = DRV_IOR(Pcie0IntrLogCtl_t, DRV_ENTRY_FLAG);
            uint32 value_set =0;

            sal_memset(&pcie_log_ctl, 0, sizeof(pcie_log_ctl));
            DRV_IOCTL(lchip, 0, cmd, &pcie_log_ctl);

            if((GetPcie0IntrLogCtl(V, pcie0IntrLogVec_f, &pcie_log_ctl))&&(GetPcie0IntrLogCtl(V, pcie0IntrReqVec_f, &pcie_log_ctl)))
            {
                /* clear dma fun intr */
                cmd = DRV_IOR(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &value_set);
                cmd = DRV_IOW(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 1, cmd, &value_set);

                /* clear log, trigger intr */
                SetPcie0IntrLogCtl(V, pcie0IntrLogVec_f, &pcie_log_ctl, 0);
                cmd = DRV_IOW(Pcie0IntrLogCtl_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &pcie_log_ctl);
            }
        }
        else
        {
            PcieIntrLogCtl_m pcie_log_ctl;
            uint32 cmd = DRV_IOR(PcieIntrLogCtl_t, DRV_ENTRY_FLAG);
            uint32 value_set =0;

            sal_memset(&pcie_log_ctl, 0, sizeof(pcie_log_ctl));
            DRV_IOCTL(lchip, 0, cmd, &pcie_log_ctl);

            if((GetPcieIntrLogCtl(V, pcieIntrLogVec_f, &pcie_log_ctl))&&(GetPcieIntrLogCtl(V, pcieIntrReqVec_f, &pcie_log_ctl)))
            {
                /* clear dma fun intr */
                cmd = DRV_IOR(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &value_set);
                cmd = DRV_IOW(DmaCtlIntrFunc_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 1, cmd, &value_set);

                /* clear log, trigger intr */
                SetPcieIntrLogCtl(V, pcieIntrLogVec_f, &pcie_log_ctl, 0);
                SetPcieIntrLogCtl(V, pcieIntrReqVec_f, &pcie_log_ctl, 0);
                cmd = DRV_IOW(PcieIntrLogCtl_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, 0, cmd, &pcie_log_ctl);
            }
        }
    }

    INTR_UNLOCK;
    return CTC_E_NONE;
}
