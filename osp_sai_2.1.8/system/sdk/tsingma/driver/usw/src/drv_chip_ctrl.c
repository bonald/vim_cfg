/**
 @file drv_chip_ctrl.c

 @date 2011-10-09

 @version v4.28.2

 The file performs adapter layer between drv and dal
*/
#include "sal.h"
#include "drv_api.h"
#include "usw/include/drv_chip_ctrl.h"
#include "usw/include/drv_common.h"
#include "usw/include/drv_error.h"
#include "usw/include/drv_io.h"

#include "dal.h"

extern dal_op_t g_dal_op;

#ifndef TX_PACKET_USE_SPINLOCK
#define DRV_PCI_LOCK(lchip_offset)          sal_mutex_lock(p_drv_master[lchip_offset]->p_pci_mutex)
#define DRV_PCI_UNLOCK(lchip_offset)        sal_mutex_unlock(p_drv_master[lchip_offset]->p_pci_mutex)
#define DRV_I2C_LOCK(lchip_offset)          sal_mutex_lock(p_drv_master[lchip_offset]->p_i2c_mutex)
#define DRV_I2C_UNLOCK(lchip_offset)        sal_mutex_unlock(p_drv_master[lchip_offset]->p_i2c_mutex)
#else
#define DRV_PCI_LOCK(lchip_offset)          sal_spinlock_lock((sal_spinlock_t*)p_drv_master[lchip_offset]->p_pci_mutex)
#define DRV_PCI_UNLOCK(lchip_offset)        sal_spinlock_unlock((sal_spinlock_t*)p_drv_master[lchip_offset]->p_pci_mutex)
#define DRV_I2C_LOCK(lchip_offset)          sal_spinlock_lock((sal_spinlock_t*)p_drv_master[lchip_offset]->p_i2c_mutex)
#define DRV_I2C_UNLOCK(lchip_offset)        sal_spinlock_unlock((sal_spinlock_t*)p_drv_master[lchip_offset]->p_i2c_mutex)
#endif

#define DRV_HSS_LOCK(lchip_offset)          sal_mutex_lock(p_drv_master[lchip_offset]->p_hss_mutex)
#define DRV_HSS_UNLOCK(lchip_offset)        sal_mutex_unlock(p_drv_master[lchip_offset]->p_hss_mutex)

/**
 @brief common read interface
*/
int32
drv_usw_chip_read(uint8 lchip, uint32 offset, uint32* p_value)
{
    drv_access_type_t acc_type;

    drv_get_access_type(lchip, &acc_type);

    switch (acc_type)
    {
    case DRV_PCI_ACCESS:
        DRV_IF_ERROR_RETURN(drv_usw_pci_read_chip(lchip, offset, 1, p_value));
        break;

    case DRV_I2C_ACCESS:
        DRV_IF_ERROR_RETURN(drv_usw_i2c_read_chip(lchip, offset, p_value));
        break;

    default:
        return DRV_E_INVALID_ACCESS_TYPE;
    }

    return DRV_E_NONE;
}

/**
 @brief common read interface
*/
int32
drv_usw_chip_read_ext(uint8 lchip, uint32 offset, uint32* p_value, uint32 length)
{
    uint32 index = 0;
    drv_access_type_t acc_type;

    drv_get_access_type(lchip, &acc_type);

    switch (acc_type)
    {
    case DRV_PCI_ACCESS:
        DRV_IF_ERROR_RETURN(drv_usw_pci_read_chip(lchip, offset, length, p_value));
        break;

    case DRV_I2C_ACCESS:
        offset = offset&0xfffffffc;
        for (index = 0; index < length; index++)
        {
            DRV_IF_ERROR_RETURN(drv_usw_i2c_read_chip(lchip, offset+4*index, &(p_value[index])));
        }

        break;

    default:
        return DRV_E_INVALID_ACCESS_TYPE;
    }

    return DRV_E_NONE;
}

/**
 @brief common write interface
*/
int32
drv_usw_chip_write(uint8 lchip, uint32 offset, uint32 value)
{
    drv_access_type_t acc_type;

    if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
    {
        return DRV_E_NONE;
    }

    if (1 == SDK_WORK_PLATFORM)
    {
        return 0;
    }

    drv_get_access_type(lchip, &acc_type);

    switch (acc_type)
    {
    case DRV_PCI_ACCESS:
        DRV_IF_ERROR_RETURN(drv_usw_pci_write_chip(lchip, offset, 1, &value));
        break;

    case DRV_I2C_ACCESS:
        DRV_IF_ERROR_RETURN(drv_usw_i2c_write_chip(lchip, offset, value));
        break;

    default:
        DRV_DBG_INFO("access type is invalid \n");
        return DRV_E_INVALID_ACCESS_TYPE;
    }

    return DRV_E_NONE;
}

/**
 @brief common write interface
*/
int32
drv_usw_chip_write_ext(uint8 lchip, uint32 offset, uint32* p_value, uint32 length)
{
    int32 index = 0;
    drv_access_type_t acc_type;

    drv_get_access_type(lchip, &acc_type);

    switch (acc_type)
    {
    case DRV_PCI_ACCESS:
        DRV_IF_ERROR_RETURN(drv_usw_pci_write_chip(lchip, offset, length, p_value));
        break;

    case DRV_I2C_ACCESS:
        offset = offset&0xfffffffc;
        for (index = 0; index < length; index++)
        {
            DRV_IF_ERROR_RETURN(drv_usw_i2c_write_chip(lchip, (offset+4*index), p_value[index]));
        }

        break;

    default:
        DRV_DBG_INFO("access type is invalid \n");
        return DRV_E_INVALID_ACCESS_TYPE;
    }

    return DRV_E_NONE;
}

/**
 @brief pci read interface
*/
int32
drv_usw_pci_read_chip(uint8 lchip, uint32 offset, uint32 len, uint32* p_value)
{
    drv_pci_cmd_status_u_t cmd_status_u;
    uint32 timeout = DRV_CMD_TIMEOUT;
    uint32 cmd_len = 0;
    int32 ret = 0;
    uint8 index = 0;
    DRV_PTR_VALID_CHECK(p_value);

    /* pcie only have 16 databuf, len must not exceed 16 */
    if ((16 < len) || (0 == len))
    {
        DRV_DBG_INFO("pci read length error! len = %d \n", len);
        return DRV_E_EXCEED_MAX_SIZE;
    }
    /* cmdDataLen must be power of 2 */
    if ((len & (len - 1)) == 0)
    {
        cmd_len = len;
    }
    else
    {
        cmd_len = len;

        do
        {
            cmd_len++;
        }
        while ((cmd_len <= 16) && (cmd_len & (cmd_len - 1)));
    }

    DRV_PCI_LOCK(lchip);

    /* 1. write CmdStatusReg */
    sal_memset(&cmd_status_u, 0, sizeof(drv_pci_cmd_status_u_t));
    cmd_status_u.cmd_status.cmdReadType = 1;
    cmd_status_u.cmd_status.cmdEntryWords = (len==16)?0:len;   /* normal operate only support 1 entry */
    cmd_status_u.cmd_status.cmdDataLen = len;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.pci_write(lchip, DRV_PCI_CMD_STATUS, cmd_status_u.val), p_drv_master[lchip]->p_pci_mutex);
    /* 2. write AddrReg */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.pci_write(lchip, DRV_PCI_ADDR, offset), p_drv_master[lchip]->p_pci_mutex);
    /* 3. polling status and check */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.pci_read(lchip, DRV_PCI_CMD_STATUS, &cmd_status_u.val), p_drv_master[lchip]->p_pci_mutex);
    while (!(cmd_status_u.cmd_status.reqProcDone) && (--timeout))
    {
        ret = g_dal_op.pci_read(lchip, DRV_PCI_CMD_STATUS, &cmd_status_u.val);
        if (ret < 0)
        {
            DRV_DBG_INFO("pci read error! cmd_status = %x, line = %d, offset = 0x%x\n", cmd_status_u.val, __LINE__, offset);
            DRV_PCI_UNLOCK(lchip);
            return ret;
        }
    }
    /* check cmd done */
    if (!(cmd_status_u.cmd_status.reqProcDone))
    {
        DRV_DBG_INFO("pci read error! cmd_status = %x, line = %d\n", cmd_status_u.val, __LINE__);
        DRV_PCI_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }
    /*check pcie read status */
     /*if ((cmd_status_u.val & DRV_PCI_CHECK_STATUS) != 0)*/
    if (cmd_status_u.cmd_status.reqProcError != 0)
    {
        DRV_DBG_INFO("pci read error! cmd_status = %x, line = %d, offset = 0x%x, lchip = %d\n", cmd_status_u.val, __LINE__, offset, lchip);
        DRV_PCI_UNLOCK(lchip);
        return DRV_E_PCI_CMD_ERROR;
    }
    /* AckCnt need not to check */

    /* 4. read data from buffer */
    for (index = 0; index < len; index++)
    {
        DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.pci_read(lchip, DRV_PCI_DATA_BUF + index * 4, &p_value[index]), p_drv_master[lchip]->p_pci_mutex);
    }
    DRV_PCI_UNLOCK(lchip);
    return DRV_E_NONE;
}

/**
 @brief pci write interface
*/
int32
drv_usw_pci_write_chip(uint8 lchip, uint32 offset, uint32 len, uint32* p_value)
{
    drv_pci_cmd_status_u_t cmd_status_u;
    uint32 timeout = DRV_CMD_TIMEOUT;  /* need to be confirmed */
    uint32 cmd_len = 0;
    int32 ret = 0;
    uint8 index = 0;

    DRV_PTR_VALID_CHECK(p_value);

    /* pcie only have 16 databuf, len must not exceed 16 */
    if ((16 < len) || (0 == len))
    {
        DRV_DBG_INFO("pci read length error! len = %d \n", len);
        return DRV_E_EXCEED_MAX_SIZE;
    }

    /* cmdDataLen must be power of 2 */
    if ((len & (len - 1)) == 0)
    {
        cmd_len = len;
    }
    else
    {
        cmd_len = len;

        do
        {
            cmd_len++;
        }
        while ((cmd_len <= 16) && (cmd_len & (cmd_len - 1)));
    }

    DRV_PCI_LOCK(lchip);

    /* 1. write CmdStatusReg */
    sal_memset(&cmd_status_u, 0, sizeof(drv_pci_cmd_status_u_t));
    cmd_status_u.cmd_status.cmdReadType = 0;
    cmd_status_u.cmd_status.cmdEntryWords = (len==16)?0:len;
    cmd_status_u.cmd_status.cmdDataLen = len; /* Notice: for 1 entry op, cmdDatalen eq cmdEntryWords, but for mutil entry, len = cmd_len */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.pci_write(lchip, DRV_PCI_CMD_STATUS, cmd_status_u.val), p_drv_master[lchip]->p_pci_mutex);

    /* 2. write AddrReg */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.pci_write(lchip, DRV_PCI_ADDR, offset), p_drv_master[lchip]->p_pci_mutex);

    /* 3. write data into databuffer */
    for (index = 0; index < len; index++)
    {
        DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.pci_write(lchip, DRV_PCI_DATA_BUF + index * 4, p_value[index]), p_drv_master[lchip]->p_pci_mutex);
    }

    /* 4. polling status and check */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.pci_read(lchip, DRV_PCI_CMD_STATUS, &cmd_status_u.val), p_drv_master[lchip]->p_pci_mutex);

    while (!(cmd_status_u.cmd_status.reqProcDone) && (--timeout))
    {
        ret = g_dal_op.pci_read(lchip, DRV_PCI_CMD_STATUS, &cmd_status_u.val);
        if (ret < 0)
        {
            DRV_DBG_INFO("pci read error! cmd_status = %x, line = %d\n", cmd_status_u.val, __LINE__);
            DRV_PCI_UNLOCK(lchip);
            return ret;
        }
    }

    /* check cmd done */
    if (!(cmd_status_u.cmd_status.reqProcDone))
    {
        DRV_DBG_INFO("pci read error! cmd_status = %x, line = %d\n", cmd_status_u.val, __LINE__);
        DRV_PCI_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }

    /*check pcie read status */
     /*if ((cmd_status_u.val & DRV_PCI_CHECK_STATUS) != 0)*/
    if (cmd_status_u.cmd_status.reqProcError != 0)
    {
        DRV_DBG_INFO("pci read error! cmd_status = %x, line = %d, offset=0x%x\n", cmd_status_u.val, __LINE__, offset);
        DRV_PCI_UNLOCK(lchip);
        return DRV_E_PCI_CMD_ERROR;
    }

    DRV_PCI_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief i2c read tbl/reg interface
*/
int32
drv_usw_i2c_read_chip(uint8 lchip, uint32 offset, uint32* p_value)
{
    int32 ret = 0;
    uint8 i2c_cmd = 0;
    uint32 data_buf = 0;
    uint8  cmd_status = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;

    DRV_PTR_VALID_CHECK(p_value);

    DRV_I2C_LOCK(lchip);

    /* 1. Write request address into WrData registers which will configure ChipAccessReg */
    offset |= DRV_I2C_CHIP_READ;
    offset = DRV_SWAP32(offset);
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_WRDATA_OFFSET, 4, (uint8*)&offset), p_drv_master[lchip]->p_i2c_mutex);

    /* 2. Configure request address and set the request types as write to configure address of ChipAccessReg */
    i2c_cmd = DRV_I2C_CHIP_ADDR_REG_OFFSET | DRV_I2C_REQ_WRITE;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 3. Configure request address and set the request types as read to get status of ChipAccessReg */
    i2c_cmd = DRV_I2C_CHIP_STATUS_REG_OFFSET | DRV_I2C_REQ_READ;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 4. Read RdData to get status of accessing supervisor register */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_RDDATA_OFFSET, 4, (uint8*)&data_buf), p_drv_master[lchip]->p_i2c_mutex);

    data_buf = DRV_SWAP32((uint32)data_buf);
    /* check the second inderect access status */
    while (((data_buf & 0x80000000) != 0x80000000) && (--timeout))
    {
        ret = g_dal_op.i2c_read(lchip, DRV_I2C_RDDATA_OFFSET, 4, (uint8*)&data_buf);
        if (ret < 0)
        {
            DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", data_buf, __LINE__);
            DRV_I2C_UNLOCK(lchip);
            return ret;
        }
        data_buf = DRV_SWAP32((uint32)data_buf);
    }

    /* check timeout */
    if ((data_buf & 0x20000000) != 0)
    {
        DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", data_buf, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }

    /* check cmd status error */
    if ((data_buf & 0x10000000) != 0)
    {
        DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", data_buf, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_I2C_CMD_ERROR;
    }

    /* 5. Configure request address and set the request types as read to get readData of ChipAccessReg */
    i2c_cmd = DRV_I2C_CHIP_DATA_REG_OFFSET | DRV_I2C_REQ_READ;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 6. Check whether current reading status is done, selective step, can omit it if waiting time is enough */
    timeout = DRV_CMD_TIMEOUT;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status), p_drv_master[lchip]->p_i2c_mutex);

    while (!(cmd_status & 0x80) && (--timeout))
    {
        ret = g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status);
        if (ret < 0)
        {
            DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", cmd_status, __LINE__);
            DRV_I2C_UNLOCK(lchip);
            return ret;
        }
    }

    if (!(cmd_status & 0x80))
    {
        DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }

    if ((cmd_status & 0x60) != 0)
    {
        DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_I2C_CMD_ERROR;
    }

    /* 7. Read RdData to get read data of accessing supervisor 0x3010 register */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_RDDATA_OFFSET, 4, (uint8*)&data_buf), p_drv_master[lchip]->p_i2c_mutex);
    *p_value = DRV_SWAP32((uint32)data_buf);

    DRV_I2C_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief i2c write tbl/reg interface
*/
int32
drv_usw_i2c_write_chip(uint8 lchip, uint32 offset, uint32 value)
{
    int32 ret = 0;
    uint8 i2c_cmd = 0;
    uint32 data_buf = 0;
    uint8  cmd_status = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;

    DRV_I2C_LOCK(lchip);

    value = DRV_SWAP32(value);

    /* 1. Write data into WrData registers which will configure ChipAccessReg */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_WRDATA_OFFSET, 4, (uint8*)&value), p_drv_master[lchip]->p_i2c_mutex);

    /* 2. Configure request address and set the request types as write to configure Write Data of ChipAccessReg */
    i2c_cmd = DRV_I2C_CHIP_WRDATA_REG_OFFSET | DRV_I2C_REQ_WRITE;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 3. Write request address into WrData registers which will configure ChipAccessReg */
    offset |= DRV_I2C_CHIP_WRITE;
    offset = DRV_SWAP32(offset);
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_WRDATA_OFFSET, 4, (uint8*)&offset), p_drv_master[lchip]->p_i2c_mutex);

    /* 4. Configure request address and set the request types as write to configure address of ChipAccessReg */
    i2c_cmd = DRV_I2C_CHIP_ADDR_REG_OFFSET | DRV_I2C_REQ_WRITE;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 5. Configure request address and set the request types as read to get status of ChipAccessReg */
    i2c_cmd = DRV_I2C_CHIP_STATUS_REG_OFFSET | DRV_I2C_REQ_READ;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 6. Read RdData to get status of accessing supervisor  register */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_RDDATA_OFFSET, 4, (uint8*)&data_buf), p_drv_master[lchip]->p_i2c_mutex);

    data_buf = DRV_SWAP32((uint32)data_buf);
    /* check the second inderect access status */
    while (((data_buf & 0x80000000) != 0x80000000) && (--timeout))
    {
        ret = g_dal_op.i2c_read(lchip, DRV_I2C_RDDATA_OFFSET, 4, (uint8*)&data_buf);
        if (ret < 0)
        {
            DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", data_buf, __LINE__);
            DRV_I2C_UNLOCK(lchip);
            return ret;
        }
        data_buf = DRV_SWAP32((uint32)data_buf);
    }

    /* check timeout */
    if ((data_buf & 0x80000000) != 0x80000000)
    {
        DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", data_buf, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }

    if ((data_buf & 0x30000000) != 0)
    {
        DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", data_buf, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_I2C_CMD_ERROR;
    }

    /* 7. Check whether current reading Status is done, selective step, can omit it if waiting time is enough */
    timeout = DRV_CMD_TIMEOUT;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status), p_drv_master[lchip]->p_i2c_mutex);

    while (!(cmd_status & 0x80) && (--timeout))
    {
        ret = g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status);
        if (ret < 0)
        {
            DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", cmd_status, __LINE__);
            DRV_I2C_UNLOCK(lchip);
            return ret;
        }
    }

    if (!(cmd_status & 0x80))
    {
        DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }

    if ((cmd_status & 0x60) != 0)
    {
        DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_I2C_CMD_ERROR;
    }

    DRV_I2C_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief i2c read pcie configuration space interface
*/
int32
drv_usw_i2c_read_local(uint8 lchip, uint32 offset, uint32* p_value)
{
    uint8 cmd_status = 0;
    int32 ret = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;
    uint32 data_buf = 0;
    uint8 temp = 0;

    DRV_PTR_VALID_CHECK(p_value);

    DRV_I2C_LOCK(lchip);

    offset = DRV_SWAP32(offset);

    /* 1.Configure request address and set the request types as write */
    offset |= DRV_I2C_REQ_READ;
    temp = (uint8)(offset & 0xff);
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, &temp), p_drv_master[lchip]->p_i2c_mutex);

    /* 2. Check status and read data, selective step, can omit it if waiting time is enough */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status), p_drv_master[lchip]->p_i2c_mutex);

    while (!(cmd_status & 0x80) && (--timeout))
    {
        ret = g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status);
        if (ret < 0)
        {
            DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", cmd_status, __LINE__);
            DRV_I2C_UNLOCK(lchip);
            return ret;
        }
    }

    if (!(cmd_status & 0x80))
    {
        DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }

    if ((cmd_status & 0x60) != 0)
    {
        DRV_DBG_INFO("i2c read error! cmd_status = 0x%x line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_I2C_CMD_ERROR;
    }

    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_RDDATA_OFFSET, 4, (uint8*)&data_buf), p_drv_master[lchip]->p_i2c_mutex);
    *p_value = DRV_SWAP32((uint32)data_buf);

    DRV_I2C_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief i2c write pcie configuration space interface
*/
int32
drv_usw_i2c_write_local(uint8 lchip, uint32 offset, uint32 value)
{
    uint8 cmd_status = 0;
    int32 ret = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;
    uint8 temp = 0;

    DRV_I2C_LOCK(lchip);

    offset = DRV_SWAP32(offset);
    value = DRV_SWAP32(value);

    /* 1. Write configuration value into WrData registers */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_WRDATA_OFFSET, 4, (uint8*)&value), p_drv_master[lchip]->p_i2c_mutex);

    /* 2. Configure request address and set the request types as write to configure address of PCIe Cfg Reg */
    offset |= DRV_I2C_REQ_WRITE;
    temp = (uint8)(offset & 0xff);
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, &temp), p_drv_master[lchip]->p_i2c_mutex);

    /* 3. read status and check */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status), p_drv_master[lchip]->p_i2c_mutex);

    while (!(cmd_status & 0x80) && (--timeout))
    {
        ret = g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status);
        if (ret < 0)
        {
            DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
            DRV_I2C_UNLOCK(lchip);
            return ret;
        }
    }

    if (!(cmd_status & 0x80))
    {
        DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }

    if ((cmd_status & 0x60) != 0)
    {
        DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_I2C_CMD_ERROR;
    }

    DRV_I2C_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief i2c read hss6g configuration space interface
*/
int32
drv_usw_i2c_read_hss6g(uint8 lchip, uint32 offset, uint16* p_value)
{
    uint8 cmd_status = 0;
    int32 ret = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;
    uint32 cmd = 0;
    uint8 i2c_cmd = 0;
    uint16 data_buf = 0;

    DRV_PTR_VALID_CHECK(p_value);

    DRV_I2C_LOCK(lchip);

    offset = DRV_SWAP32(offset);

    /* 1. Write request address into WrData registers which will configure address of RCBusCfg */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_WRDATA_OFFSET, 4, (uint8*)&offset), p_drv_master[lchip]->p_i2c_mutex);

    /* 2. Configure request address and set the request types as write to configure address of RCBusCfg */
    i2c_cmd = DRV_I2C_RCBUS_ADDR_REG_OFFSET | DRV_I2C_REQ_WRITE;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 3. Write request type of RCBusCfg into WrData */
    cmd = 0x02;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_WRDATA_OFFSET, 4, (uint8*)&cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 4. Configure request address and set the request types as write to configure command of RCBusCfg */
    i2c_cmd = DRV_I2C_RCBUS_CMD_REG_OFFSET | DRV_I2C_REQ_WRITE;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 5. Configure request address and set the request types as read to get status and readData of RCBusCfg */
    i2c_cmd = DRV_I2C_RCBUS_CMD_REG_OFFSET | DRV_I2C_REQ_READ;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 6. Check whether current reading status is done, selective step, can omit it if waiting time is enough */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status), p_drv_master[lchip]->p_i2c_mutex);

    while (!(cmd_status & 0x80) && (--timeout))
    {
        ret = g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status);
        if (ret < 0)
        {
            DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
            DRV_I2C_UNLOCK(lchip);
            return ret;
        }
    }

    if (!(cmd_status & 0x80))
    {
        DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }

    if ((cmd_status & 0x60) != 0)
    {
        DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_I2C_CMD_ERROR;
    }

    /* 7. Read RdData to get status and read data of RCBusCfg ,check 2st inderect access status */
    timeout = DRV_CMD_TIMEOUT;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_RDDATA_OFFSET + 3, 1, (uint8*)&cmd_status), p_drv_master[lchip]->p_i2c_mutex);

    while (!(cmd_status & 0x10) && (--timeout))
    {
        ret = g_dal_op.i2c_read(lchip, DRV_I2C_RDDATA_OFFSET + 3, 1, (uint8*)&cmd_status);
        if (ret < 0)
        {
            DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
            DRV_I2C_UNLOCK(lchip);
            return ret;
        }
    }

    if (!(cmd_status & 0x10))
    {
        DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }

    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_RDDATA_OFFSET, 2, (uint8*)&data_buf), p_drv_master[lchip]->p_i2c_mutex);
    data_buf = DRV_SWAP16(data_buf);

    *p_value = (uint16)data_buf;

    DRV_I2C_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief i2c write hss6g configuration space interface
*/
int32
drv_usw_i2c_write_hss6g(uint8 lchip, uint32 offset, uint16 value, uint16 mask)
{
    uint8 i2c_cmd = 0;
    uint32 cmd = 0;
    uint8 cmd_status = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;
    int32 ret = 0;

    DRV_I2C_LOCK(lchip);

    value = DRV_SWAP16(value);
    mask = DRV_SWAP16(mask);
    offset = DRV_SWAP32(offset);

    /* 1. Write mask and data into WrData registers which will configure rcWrMask of RCBusCfg */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_WRDATA_OFFSET, 2, (uint8*)&mask), p_drv_master[lchip]->p_i2c_mutex);
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_WRDATA_OFFSET + 2, 2, (uint8*)&value), p_drv_master[lchip]->p_i2c_mutex);

    /* 2. Configure request address and set the request types as write to configure write data/mask of RCBusCfg */
    i2c_cmd = DRV_I2C_RCBUS_WRDATA_REG_OFFSET | DRV_I2C_REQ_WRITE;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 3. write request address into WrData registers which will configure address of RCBusCfg */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_WRDATA_OFFSET, 4, (uint8*)&offset), p_drv_master[lchip]->p_i2c_mutex);

    /* 4. Configure request address and set the request types as write to configure address of RCBusCfg */
    i2c_cmd = DRV_I2C_RCBUS_ADDR_REG_OFFSET | DRV_I2C_REQ_WRITE;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 5. Write request type of RCBUS into WrData, mask-write */
    cmd = 0x01;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_WRDATA_OFFSET, 4, (uint8*)&cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 6. Configure request address and set the request types as write to configure command of RCBusCfg */
    i2c_cmd = DRV_I2C_RCBUS_CMD_REG_OFFSET | DRV_I2C_REQ_WRITE;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* 7. Read RdData to get status of RCBusCfg */
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status), p_drv_master[lchip]->p_i2c_mutex);

    while (!(cmd_status & 0x80) && (--timeout))
    {
        ret = g_dal_op.i2c_read(lchip, DRV_I2C_STATUS_OFFSET, 1, (uint8*)&cmd_status);
        if (ret < 0)
        {
            DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
            DRV_I2C_UNLOCK(lchip);
            return ret;
        }
    }

    if (!(cmd_status & 0x80))
    {
        DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }

    if ((cmd_status & 0x60) != 0)
    {
        DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_I2C_CMD_ERROR;
    }

    i2c_cmd = DRV_I2C_RCBUS_CMD_REG_OFFSET | DRV_I2C_REQ_READ;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_write(lchip, DRV_I2C_ADDRCMD_OFFSET, 1, (uint8*)&i2c_cmd), p_drv_master[lchip]->p_i2c_mutex);

    /* check 2st inderect access status */
    timeout = DRV_CMD_TIMEOUT;
    DRV_ACCESS_DAL_WITH_UNLOCK(g_dal_op.i2c_read(lchip, DRV_I2C_RDDATA_OFFSET + 3, 1, (uint8*)&cmd_status), p_drv_master[lchip]->p_i2c_mutex);

    while (!(cmd_status & 0x10) && (--timeout))
    {
        ret = g_dal_op.i2c_read(lchip, DRV_I2C_RDDATA_OFFSET + 3, 1, (uint8*)&cmd_status);
        if (ret < 0)
        {
            DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
            DRV_I2C_UNLOCK(lchip);
            return ret;
        }
    }

    if (!(cmd_status & 0x10))
    {
        DRV_DBG_INFO("i2c read error! cmd_status = %d line = %d\n", cmd_status, __LINE__);
        DRV_I2C_UNLOCK(lchip);
        return DRV_E_CMD_NOT_DONE;
    }

    DRV_I2C_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief access hss15g control register
*/
int32
drv_chip_read_hss15g(uint8 lchip, uint8 hssid, uint32 addr, uint16* p_data)
{
    DRV_PTR_VALID_CHECK(p_data);
    DRV_INIT_CHECK(lchip);

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
    {
        return DRV_E_NONE;
    }

    DRV_HSS_LOCK(lchip);
    if (p_drv_master[lchip]->drv_chip_read_hss15g)
    {
        DRV_IF_ERROR_RETURN_WITH_UNLOCK(p_drv_master[lchip]->drv_chip_read_hss15g(lchip, hssid, addr, p_data), p_drv_master[lchip]->p_hss_mutex);
    }
    DRV_HSS_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief access hss15g control register
*/
int32
drv_chip_write_hss15g(uint8 lchip, uint8 hssid, uint32 addr, uint16 data)
{
    DRV_INIT_CHECK(lchip);

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
    {
        return DRV_E_NONE;
    }

    DRV_HSS_LOCK(lchip);
    if (p_drv_master[lchip]->drv_chip_write_hss15g)
    {
        DRV_IF_ERROR_RETURN_WITH_UNLOCK(p_drv_master[lchip]->drv_chip_write_hss15g(lchip, hssid, addr, data), p_drv_master[lchip]->p_hss_mutex);
    }
    DRV_HSS_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief access hss15g control register
*/
int32
drv_chip_read_hss15g_aec_aet(uint8 lchip, uint8 hssid, uint8 lane_id, uint8 is_aet,  uint32 addr, uint16* p_data)
{
    int32 ret = 0;
    HsMacroPrtReq0_m hss_req;
    HsMacroPrtAck0_m hss_ack;
    uint32 cmd = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;
    HsMacroPrtReqId0_m req_id;
    uint8 reqid = 0;

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

    DRV_HSS_LOCK(lchip);

    /* 1. write reqId to hss15g access */
    reqid = is_aet?(8+lane_id):lane_id;
    SetHsMacroPrtReqId0(V, reqId_f, &req_id, reqid);
    cmd = DRV_IOW((HsMacroPrtReqId0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &req_id);

    cmd = DRV_IOR((HsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);
    if (GetHsMacroPrtReq0(V, reqPrtValid_f, &hss_req))
    {
        DRV_HSS_UNLOCK(lchip);
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
        DRV_HSS_UNLOCK(lchip);
        return DRV_E_ACCESS_HSS12G_FAIL;
    }


    if (0 == SDK_WORK_PLATFORM)
    {
        /* 3. Read hssReadDataValid register, and check whether current accssing is done */
        cmd = DRV_IOR((HsMacroPrtAck0_t+hssid), DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &hss_ack);
        if (ret < 0)
        {
            DRV_HSS_UNLOCK(lchip);
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
            DRV_HSS_UNLOCK(lchip);
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
    }

    /* 4. Read hssReadData registers */
    *p_data = GetHsMacroPrtAck0(V, ackPrtReadData_f, &hss_ack);

    DRV_HSS_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief access hss15g control register
*/
int32
drv_chip_write_hss15g_aec_aet(uint8 lchip, uint8 hssid, uint8 lane_id, uint8 is_aet, uint32 addr, uint16 data)
{
    int32 ret = 0;
    HsMacroPrtReq0_m hss_req;
    HsMacroPrtAck0_m hss_ack;
    HsMacroPrtReqId0_m req_id;
    uint32 cmd = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;
    uint8 reqid = 0;
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

    DRV_HSS_LOCK(lchip);

    /* 1. write reqId to hss15g access */
    reqid = is_aet?(8+lane_id):lane_id;

    tmp_val32 = reqid;
    DRV_IOW_FIELD(lchip, (HsMacroPrtReqId0_t+hssid), HsMacroPrtReqId0_reqId_f, &tmp_val32, &req_id);

    cmd = DRV_IOW((HsMacroPrtReqId0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &req_id);

    cmd = DRV_IOR((HsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);

    if (GetHsMacroPrtReq0(V, reqPrtValid_f, &hss_req))
    {
        DRV_HSS_UNLOCK(lchip);
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
        DRV_HSS_UNLOCK(lchip);
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    if (0 == SDK_WORK_PLATFORM)
    {
        /* 3. Read hssReadDataValid register, and check whether current accssing is done */
        cmd = DRV_IOR((HsMacroPrtAck0_t+hssid), DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &hss_ack);
        if (ret < 0)
        {
            DRV_HSS_UNLOCK(lchip);
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
            DRV_HSS_UNLOCK(lchip);
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
    }

    DRV_HSS_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief access hss28g control register
*/
int32
drv_chip_read_hss28g(uint8 lchip, uint8 hssid, uint32 addr, uint16* p_data)
{
    DRV_PTR_VALID_CHECK(p_data);
    DRV_INIT_CHECK(lchip);

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
    {
        return DRV_E_NONE;
    }

    DRV_HSS_LOCK(lchip);
    if (p_drv_master[lchip]->drv_chip_read_hss28g)
    {
        DRV_IF_ERROR_RETURN_WITH_UNLOCK(p_drv_master[lchip]->drv_chip_read_hss28g(lchip, hssid, addr, p_data), p_drv_master[lchip]->p_hss_mutex);
    }
    DRV_HSS_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief access hss28g control register
*/
int32
drv_chip_write_hss28g(uint8 lchip, uint8 hssid, uint32 addr, uint16 data)
{
    DRV_INIT_CHECK(lchip);

    /*uml donot care*/
    if (1 == SDK_WORK_PLATFORM)
    {
        return DRV_E_NONE;
    }

    if (p_drv_master[lchip]->wb_status == DRV_WB_STATUS_RELOADING)
    {
        return DRV_E_NONE;
    }

    DRV_HSS_LOCK(lchip);
    if (p_drv_master[lchip]->drv_chip_write_hss28g)
    {
        DRV_IF_ERROR_RETURN_WITH_UNLOCK(p_drv_master[lchip]->drv_chip_write_hss28g(lchip, hssid, addr, data), p_drv_master[lchip]->p_hss_mutex);
    }
    DRV_HSS_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief access hss28g control register
*/
int32
drv_chip_read_hss28g_aec_aet(uint8 lchip, uint8 hssid, uint8 lane_id, uint8 is_aet,  uint32 addr, uint16* p_data)
{
    int32 ret = 0;
    CsMacroPrtReq0_m hss_req;
    CsMacroPrtAck0_m hss_ack;
    CsMacroPrtReqId0_m req_id;
    uint32 cmd = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;
    uint8 reqid = 0;

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

    DRV_HSS_LOCK(lchip);

    /* 1. write reqId to hss28g access */
    reqid = is_aet?(4+lane_id):lane_id;
    SetHsMacroPrtReqId0(V, reqId_f, &req_id, reqid);
    cmd = DRV_IOW((CsMacroPrtReqId0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &req_id);

    cmd = DRV_IOR((CsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);
    if (GetCsMacroPrtReq0(V, reqPrtValid_f, &hss_req))
    {
        DRV_HSS_UNLOCK(lchip);
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
        DRV_HSS_UNLOCK(lchip);
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    if (0 == SDK_WORK_PLATFORM)
    {
        /* 3. Read hssReadDataValid register, and check whether current accssing is done */
        cmd = DRV_IOR((CsMacroPrtAck0_t+hssid), DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &hss_ack);
        if (ret < 0)
        {
            DRV_HSS_UNLOCK(lchip);
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
            DRV_HSS_UNLOCK(lchip);
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
    }

    /* 4. Read hssReadData registers */
    *p_data = GetCsMacroPrtAck0(V, ackPrtReadData_f, &hss_ack);

    DRV_HSS_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief access hss28g control register
*/
int32
drv_chip_write_hss28g_aec_aet(uint8 lchip, uint8 hssid, uint8 lane_id, uint8 is_aet, uint32 addr, uint16 data)
{
    int32 ret = 0;
    CsMacroPrtReq0_m hss_req;
    CsMacroPrtAck0_m hss_ack;
    CsMacroPrtReqId0_m req_id;
    uint32 cmd = 0;
    uint32 timeout = DRV_CMD_TIMEOUT;
    uint8 reqid = 0;
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

    DRV_HSS_LOCK(lchip);

    /* 1. write reqId to hss28g access */
    reqid = is_aet?(4+lane_id):lane_id;
    SetHsMacroPrtReqId0(V, reqId_f, &req_id, reqid);
    tmp_val32 = reqid;
    DRV_IOW_FIELD(lchip, (CsMacroPrtReqId0_t+hssid), CsMacroPrtReqId0_reqId_f, &tmp_val32, &req_id);
    cmd = DRV_IOW((CsMacroPrtReqId0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &req_id);

    cmd = DRV_IOR((CsMacroPrtReq0_t+hssid), DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &hss_req);

    if (GetCsMacroPrtReq0(V, reqPrtValid_f, &hss_req))
    {
        DRV_HSS_UNLOCK(lchip);
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
        DRV_HSS_UNLOCK(lchip);
        return DRV_E_ACCESS_HSS12G_FAIL;
    }

    if (0 == SDK_WORK_PLATFORM)
    {

        /* 3. Read hssReadDataValid register, and check whether current accssing is done */
        cmd = DRV_IOR((CsMacroPrtAck0_t+hssid), DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &hss_ack);
        if (ret < 0)
        {
            DRV_HSS_UNLOCK(lchip);
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
            DRV_HSS_UNLOCK(lchip);
            return DRV_E_ACCESS_HSS12G_FAIL;
        }
    }
    DRV_HSS_UNLOCK(lchip);

    return DRV_E_NONE;
}

