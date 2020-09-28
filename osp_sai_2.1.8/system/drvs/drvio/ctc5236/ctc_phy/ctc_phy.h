#ifndef __CTC_PHY_H__
#define __CTC_PHY_H__

#define CTC_PHY_DEV_NAME          "/dev/ctc_phy"
#define CTC_PHY_DEV_MAJOR         102
#define CTC_PHY_MAX_INTR_NUM      2

#define RD_PERI_IRQ_STAT          _IO(CTC_PHY_DEV_MAJOR, 6)
#define CTC_CMD_EN_INTERRUPTS     _IO(CTC_PHY_DEV_MAJOR, 0)

#endif

