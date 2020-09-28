/**
 @file dal.h

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-4-9

 @version v2.0

*/
#ifndef _DRV_CHIP_CTRL_H_
#define _DRV_CHIP_CTRL_H_
#ifdef __cplusplus
extern "C" {
#endif

#define DRV_PCI_CMD_STATUS  0x0
#define DRV_PCI_ADDR  0x4
#define DRV_PCI_DATA_BUF  0x8
#define DRV_PCI_CHECK_STATUS 0x000e0000

#define DRV_I2C_REQ_READ     0x80
#define DRV_I2C_REQ_WRITE    0x0
#define DRV_I2C_WRDATA_OFFSET  0
#define DRV_I2C_ADDRCMD_OFFSET 4
#define DRV_I2C_RDDATA_OFFSET 5
#define DRV_I2C_STATUS_OFFSET 9

#define DRV_I2C_CHIP_READ    0
#define DRV_I2C_CHIP_WRITE    1
#define DRV_I2C_CHIP_WRDATA_REG_OFFSET  0
#define DRV_I2C_CHIP_ADDR_REG_OFFSET  1
#define DRV_I2C_CHIP_STATUS_REG_OFFSET  3
#define DRV_I2C_CHIP_DATA_REG_OFFSET  2

#define DRV_I2C_RCBUS_WRDATA_REG_OFFSET  0x12
#define DRV_I2C_RCBUS_ADDR_REG_OFFSET  0x11
#define DRV_I2C_RCBUS_CMD_REG_OFFSET  0x13

#define DRV_CMD_TIMEOUT   0x6400

#define DRV_HSS15G_MACRO_NUM   3
#define DRV_HSS28G_MACRO_NUM   4

#define DRV_HSS_ADDR_COMBINE(addr_h, addr_l)   (((addr_h)<<6)|(addr_l))

#define DRV_ACCESS_DAL(func) \
    { \
        int32 rv; \
        if ((rv = (func)) < 0)  \
        {  \
            return rv; \
        } \
    }

#ifndef TX_PACKET_USE_SPINLOCK
#define DRV_ACCESS_DAL_WITH_UNLOCK(func, lock) \
    { \
        int32 rv; \
        if ((rv = (func)) < 0)  \
        {  \
            sal_mutex_unlock(lock); \
            return rv; \
        } \
    }
#else
#define DRV_ACCESS_DAL_WITH_UNLOCK(func, lock) \
    { \
        int32 rv; \
        if ((rv = (func)) < 0)  \
        {  \
            sal_spinlock_unlock((sal_spinlock_t*)lock); \
            return rv; \
        } \
    }
#endif

#define DRV_CONVT_LEN(len, len_convt) \
    { \
        uint8 temp = len; \
        do \
        { \
            temp++; \
        } while ((temp = < 16) && (temp & (temp - 1))); \
        len_convt = temp; \
    }

#if (HOST_IS_LE == 0)
#define DRV_SWAP32(val) (uint32)(val)
#define DRV_SWAP16(val) (uint16)(val)

/* pci cmd struct define */
typedef struct drv_pci_cmd_status_s
{
#if 0
    uint32 reserved_0           : 5;
    uint32 reserved_1           : 5;
    uint32 reserved_2           : 2;
    uint32 cmdStatusError     : 1;
    uint32 cmdStatusTimeout        : 1;
    uint32 cmdError        : 1;
    uint32 regInProc          : 1;
    uint32 reserved_3           : 2;
    uint32 cmdDataLen       : 6;
    uint32 reserved_4           : 1;
    uint32 cmdEntryWords    : 6;
    uint32 cmdReadType      : 1;
#endif
#if 1
    uint32 pcieReqOverlap        : 1;
    uint32 regProcState        : 3;
    uint32 pciePoisoned        : 1;
    uint32 cmdStatusWaitReq   : 1;
    uint32 regProcBusy        : 1;
    uint32 reqProcAckCnt    : 5;
    uint32 reqProcAckError  : 1;
    uint32 reqProcTimeout   : 1;
    uint32 reqProcError     : 1;
    uint32 reqProcDone      : 1;
    uint32 pcieDataError    : 1;
    uint32 pcieReqError     : 1;
    uint32 reserved         : 1;
    uint32 cmdDataLen       : 5;
    uint32 cmdEntryWords    : 4;
    uint32 pcieReqCmdChk    : 3;
    uint32 cmdReadType      : 1;
#endif
} drv_pci_cmd_status_t;

#else

#define DRV_SWAP32(val) \
    ((uint32)( \
         (((uint32)(val) & (uint32)0x000000ffUL) << 24) | \
         (((uint32)(val) & (uint32)0x0000ff00UL) << 8) | \
         (((uint32)(val) & (uint32)0x00ff0000UL) >> 8) | \
         (((uint32)(val) & (uint32)0xff000000UL) >> 24)))

#define DRV_SWAP16(val) \
    ((uint16)( \
         (((uint16)(val) & (uint16)0x00ffU) << 8) | \
         (((uint16)(val) & (uint16)0xff00U) >> 8)))

typedef struct drv_pci_cmd_status_s
{
    uint32 cmdReadType      : 1;
    uint32 pcieReqCmdChk    : 3;
    uint32 cmdEntryWords    : 4;
    uint32 cmdDataLen       : 5;
    uint32 reserved         : 1;
    uint32 pcieReqError     : 1;
    uint32 pcieDataError    : 1;
    uint32 reqProcDone      : 1;
    uint32 reqProcError     : 1;
    uint32 reqProcTimeout   : 1;
    uint32 reqProcAckError  : 1;
    uint32 reqProcAckCnt    : 5;
    uint32 regProcBusy        : 1;
    uint32 cmdStatusWaitReq   : 1;
    uint32 pciePoisoned        : 1;
    uint32 regProcState        : 3;
    uint32 pcieReqOverlap        : 1;

#if 0
    uint32 cmdReadType      : 1;
    uint32 cmdEntryWords    : 6;
    uint32 reserved_4           : 1;
    uint32 cmdDataLen       : 6;
    uint32 reserved_3           : 2;
    uint32 regInProc          : 1;
    uint32 cmdError        : 1;
    uint32 cmdStatusTimeout        : 1;
    uint32 cmdStatusError     : 1;
    uint32 reserved_2           : 2;
    uint32 reserved_1           : 5;
    uint32 reserved_0           : 5;
#endif

} drv_pci_cmd_status_t;
#endif

typedef union drv_pci_cmd_status_u_e
{
    drv_pci_cmd_status_t cmd_status;
    uint32 val;
} drv_pci_cmd_status_u_t;

/*
   hss15g and hss28g using these address to access hssctl,
   Notice for hss28g only using 4 lanes, so EFGH lane is useless for hss28g
*/
enum drv_hss_prt_addr_e
{
    DRV_HSS_TX_LINKA_ADDR = 0,    /*0*/
    DRV_HSS_TX_LINKB_ADDR,         /*1*/
    DRV_HSS_RX_LINKA_ADDR,         /*2*/
    DRV_HSS_RX_LINKB_ADDR,         /*3*/
    DRV_HSS_TX_LINKC_ADDR,         /*4*/
    DRV_HSS_TX_LINKD_ADDR,         /*5*/
    DRV_HSS_RX_LINKC_ADDR,         /*6*/
    DRV_HSS_RX_LINKD_ADDR,         /*7*/
    DRV_HSS_TX_LINKE_ADDR,          /*8*/
    DRV_HSS_TX_LINKF_ADDR,          /*9*/
    DRV_HSS_RX_LINKE_ADDR,          /*10*/
    DRV_HSS_RX_LINKF_ADDR,          /*11*/
    DRV_HSS_TX_LINKG_ADDR,          /*12*/
    DRV_HSS_TX_LINKH_ADDR,          /*13*/
    DRV_HSS_RX_LINKG_ADDR,          /*14*/
    DRV_HSS_RX_LINKH_ADDR,          /*15*/
    DRV_HSS_COMMON_ADDR = 16,
    DRV_HSS_TX_BC_ADDR,               /*17*/
    DRV_HSS_RX_BC_ADDR,               /*18*/

    DRV_HSS_PLLA_ADDR = 20,           /*20*/
    DRV_HSS_PLLB_ADDR                   /*21*/

};
typedef enum drv_hss_prt_addr_e drv_hss_prt_addr_t;

extern int32 drv_usw_chip_i2c_mutex_init(uint8 lchip_offset);
extern int32 drv_usw_chip_pci_mutex_init(uint8 lchip_offset);
extern int32 drv_usw_chip_read(uint8 lchip, uint32 offset, uint32* value);
extern int32 drv_usw_chip_write(uint8 lchip, uint32 offset, uint32 p_value);
extern int32 drv_usw_chip_set_access_type(drv_access_type_t access_type);
extern int32 drv_usw_chip_get_access_type(drv_access_type_t* p_access_type);
extern int32 drv_usw_pci_read_chip(uint8 lchip, uint32 offset, uint32 len, uint32* value);
extern int32 drv_usw_pci_write_chip(uint8 lchip, uint32 offset, uint32 len, uint32* value);
extern int32 drv_usw_i2c_read_chip(uint8 lchip, uint32 offset, uint32* value);
extern int32 drv_usw_i2c_write_chip(uint8 lchip, uint32 offset, uint32 value);
extern int32 drv_usw_i2c_read_local(uint8 lchip, uint32 offset, uint32* value);
extern int32 drv_usw_i2c_write_local(uint8 lchip, uint32 offset, uint32 value);
extern int32 drv_usw_i2c_read_hss6g(uint8 lchip, uint32 offset, uint16* value);
extern int32 drv_usw_i2c_write_hss6g(uint8 lchip, uint32 offset, uint16 value, uint16 mask);
extern int32 drv_usw_chip_write_ext(uint8 lchip, uint32 offset, uint32* p_value, uint32 length);
extern int32 drv_usw_chip_read_ext(uint8 lchip, uint32 offset, uint32* p_value, uint32 length);
extern int32 drv_chip_read_hss15g(uint8 lchip, uint8 hssid, uint32 addr, uint16* p_data);
extern int32 drv_chip_write_hss15g(uint8 lchip, uint8 hssid, uint32 addr, uint16 data);
extern int32 drv_chip_read_hss28g(uint8 lchip, uint8 hssid, uint32 addr, uint16* p_data);
extern int32 drv_chip_write_hss28g(uint8 lchip, uint8 hssid, uint32 addr, uint16 data);
extern int32 drv_chip_read_hss15g_aec_aet(uint8 lchip, uint8 hssid, uint8 lane_id, uint8 is_aet,  uint32 addr, uint16* p_data);
extern int32 drv_chip_write_hss15g_aec_aet(uint8 lchip, uint8 hssid, uint8 lane_id, uint8 is_aet, uint32 addr, uint16 data);
extern int32 drv_chip_read_hss28g_aec_aet(uint8 lchip, uint8 hssid, uint8 lane_id, uint8 is_aet,  uint32 addr, uint16* p_data);
extern int32 drv_chip_write_hss28g_aec_aet(uint8 lchip, uint8 hssid, uint8 lane_id, uint8 is_aet, uint32 addr, uint16 data);

#ifdef __cplusplus
}
#endif

#endif

