
#include "ctc_error.h"
#include "drv_api.h"
#include "ctc_chip.h"
#include "sys_usw_chip.h"

#include "usw/include/drv_common.h"
#include "usw/include/drv_chip_ctrl.h"


#define SYS_CHIP_MEM_BIST_WRITE(lchip, offset, value)                               \
do {                                                                                \
    drv_access_type_t acc_type;                                                     \
    drv_get_access_type(lchip, &acc_type);                                                 \
    switch (acc_type)                                                               \
    {                                                                               \
    case DRV_PCI_ACCESS:                                                            \
        CTC_ERROR_RETURN(drv_usw_pci_write_chip(lchip, offset, 1, &value));         \
        break;                                                                      \
    case DRV_I2C_ACCESS:                                                            \
        CTC_ERROR_RETURN(drv_usw_i2c_write_chip(lchip, offset, value));             \
        break;                                                                      \
    default:                                                                        \
        return DRV_E_INVALID_ACCESS_TYPE;                                           \
    }                                                                               \
}while (0)


int32
sys_duet2_chip_set_mem_bist(uint8 lchip, void* p_val)
{
    uint8 index = 0;
    uint32 cmd = 0;
    uint32 field_val = 0;
    ctc_chip_mem_bist_t* p_value = NULL;

    CTC_PTR_VALID_CHECK(p_val);

    p_value = (ctc_chip_mem_bist_t*)p_val;

    field_val = 0x00008000;
    SYS_CHIP_MEM_BIST_WRITE(lchip, TABLE_DATA_BASE(lchip, SupResetCtl_t, 0), field_val);

    field_val = 0;
    SYS_CHIP_MEM_BIST_WRITE(lchip, TABLE_DATA_BASE(lchip, SysMabistGo_t, 0), field_val);

    /*wait pll lock*/
    cmd = DRV_IOR(PllCoreMon_t, PllCoreMon_monPllCoreLock_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    while (field_val == 0)
    {
        sal_task_sleep(1);
        if ((index++) > 50)
        {
            SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mbist cannot lock\n");
            return CTC_E_HW_FAIL;
        }
        cmd = DRV_IOR(PllCoreMon_t, PllCoreMon_monPllCoreLock_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    /*wait mbist ready*/
    sal_task_sleep(2);
    cmd = DRV_IOR(SysMabistReady_t, SysMabistReady_sysMabistReady_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));


    /*wait mbist ready*/
    sal_task_sleep(2);
    cmd = DRV_IOR(SysMabistReady_t, SysMabistReady_sysMabistReady_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    while (field_val != 0x3FFFF)
    {
        sal_task_sleep(1);
        if ((index++) > 50)
        {
            SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mbist cannot ready\n");
            return CTC_E_HW_FAIL;
        }
        cmd = DRV_IOR(SysMabistReady_t, SysMabistReady_sysMabistReady_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    /*set mbist repair*/
    field_val = 0;
    SYS_CHIP_MEM_BIST_WRITE(lchip, TABLE_DATA_BASE(lchip, SysMabistRepair_t, 0), field_val);

    /*set mbist go*/
    sal_task_sleep(2);
    field_val = 0x3FFFF;
    SYS_CHIP_MEM_BIST_WRITE(lchip, TABLE_DATA_BASE(lchip, SysMabistGo_t, 0), field_val);


    sal_task_sleep(2000);

    /*wait mbist done*/
    cmd = DRV_IOR(SysMabistDone_t, SysMabistDone_sysMabistDone_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    while (field_val != 0x3FFFF)
    {
        sal_task_sleep(1);
        if ((index++) > 50)
        {
            SYS_CHIP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mbist cannot done\n");
            return CTC_E_HW_FAIL;
        }

        cmd = DRV_IOR(SysMabistDone_t, SysMabistDone_sysMabistDone_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    /*wait mbist result*/
    cmd = DRV_IOR(SysMabistFail_t, SysMabistFail_sysMabistFail_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    if (field_val == 0)
    {
        p_value->status = 0;
    }
    else
    {
        p_value->status = 1;
    }

    field_val = 0;
    SYS_CHIP_MEM_BIST_WRITE(lchip, TABLE_DATA_BASE(lchip, SupResetCtl_t, 0), field_val);

    return CTC_E_NONE;

}



