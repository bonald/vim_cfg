#include "sal.h"
#include "ctc_cli.h"
#include "usw/include/drv_enum.h"
#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "usw/include/drv_chip_ctrl.h"
#include "ctc_dkit.h"
#include "ctc_dkit_common.h"
#include "ctc_usw_dkit.h"

extern ctc_dkit_master_t* g_usw_dkit_master[CTC_DKITS_MAX_LOCAL_CHIP_NUM];
#define SYS_DKIT_TSINGMA_SENSOR_TIMEOUT 1000
#define SYS_DKIT_TSINGMA_TEMP_TABLE_NUM 166

STATIC int32
_ctc_tm_dkit_get_temp_with_code(uint8 lchip, uint32 temp_code, uint32* p_temp_val)
{
    uint16 temp_mapping_tbl[SYS_DKIT_TSINGMA_TEMP_TABLE_NUM] = {
        804, 801, 798, 795, 792, 790, 787, 784, 781, 778, 775, 772, 769, 766, 763, 761, 758, 755, 752, 749, /*-40~-21*/
        746, 743, 740, 737, 734, 731, 728, 725, 722, 719, 717, 714, 711, 708, 705, 702, 699, 696, 693, 690, /*-20~-1*/
        687, 684, 681, 678, 675, 672, 669, 666, 663, 660, 658, 655, 652, 649, 646, 643, 640, 637, 634, 631, /*0~19*/
        628, 625, 622, 619, 616, 613, 610, 607, 604, 601, 599, 596, 593, 590, 587, 584, 581, 578, 575, 572, /*20~39*/
        569, 566, 563, 560, 557, 554, 551, 548, 545, 542, 540, 537, 534, 531, 528, 525, 522, 519, 516, 513, /*40~59*/
        510, 507, 504, 501, 498, 495, 492, 489, 486, 483, 481, 478, 475, 472, 469, 466, 463, 460, 457, 454, /*60~79*/
        451, 448, 445, 442, 439, 436, 433, 430, 427, 424, 421, 418, 415, 412, 409, 406, 403, 400, 397, 394, /*80~99*/
        391, 388, 385, 382, 379, 376, 373, 370, 367, 364, 361, 358, 355, 352, 349, 346, 343, 340, 337, 334, /*100~119*/
        331, 328, 325, 322, 329, 316};                                                                      /*120~125*/
    uint8 index = 0;

    for (index = 0; index< (SYS_DKIT_TSINGMA_TEMP_TABLE_NUM-1); index++)
    {
        if ((temp_code <= temp_mapping_tbl[index]) && (temp_code > temp_mapping_tbl[index+1]))
        {
            break;
        }
    }

    if (index < 39)
    {
        *p_temp_val = 40 - index + (1 << 31);
    }
    else
    {
        *p_temp_val = index - 40;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_tm_dkit_monitor_sensor(uint8 lchip, ctc_dkit_monitor_sensor_t type, uint32* p_value)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 timeout = SYS_DKIT_TSINGMA_SENSOR_TIMEOUT;
    uint32 index = 0;

    DKITS_PTR_VALID_CHECK(p_value);
    if ((CTC_DKIT_MONITOR_SENSOR_TEMP != type) && (CTC_DKIT_MONITOR_SENSOR_VOL != type))
    {
        return CLI_ERROR;
    }

    /*config RCLK_EN=1,RAVG_SEL,RCLK_DIV*/
    /*mask_write tbl-reg OmcMem 0xf offset 0x0 0x00001000 0x00001000*/
    index = 0xf;
    DKITS_BIT_SET(field_val, 12);
    field_val = 0x310c7;
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));

    /*config RTHMC_RST=1*/
    /*mask_write tbl-reg OmcMem 0x10 offset 0x0 0x00000010 0x00000010*/
    index = 0x10;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));
    DKITS_BIT_SET(field_val, 4);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));

    /*wait RTHMC_RST=1*/
    /*read tbl-reg OmcMem 0x10 offset 0x0*/
    timeout = SYS_DKIT_TSINGMA_SENSOR_TIMEOUT;
    index = 0x10;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    while(timeout)
    {
        timeout--;
        (DRV_IOCTL(lchip, index, cmd, &field_val));
        if (0 == DKITS_IS_BIT_SET(field_val, 4))
        {
            break;
        }
        sal_task_sleep(1);
    }
    if (0 == timeout)
    {
        return CLI_ERROR;
    }

    /*config ENBIAS=1£¬ENVR=1£¬ENAD=1*/
    /*mask_write tbl-reg OmcMem 0x11 offset 0x0 0x02000007 0x03000007*/
    index = 0x11;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));
    DKITS_BIT_SET(field_val, 0);
    DKITS_BIT_SET(field_val, 1);
    DKITS_BIT_SET(field_val, 2);
    DKITS_BIT_UNSET(field_val, 24);
    DKITS_BIT_SET(field_val, 25);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));

    sal_task_sleep(10);

    /*set RTHM_MODE=1,RSAMPLE_DONE_INTEN=1,RDUMMY_THMRD=1,RV_SAMPLE_EN=1*/
    /*mask_write tbl-reg OmcMem 0x10 offset 0x0 0x00000203 0x00000003*/
    /*mask_write tbl-reg OmcMem 0x8 offset 0x0 0x00000004 0x00000004*/
    /*mask_write tbl-reg OmcMem 0x12 offset 0x0 0x00000001 0x00000001*/
    index = 0x10;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));
    DKITS_BIT_SET(field_val, 0);
    DKITS_BIT_SET(field_val, 1);
    DKITS_BIT_SET(field_val, 9);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));
    index = 0x8;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));
    DKITS_BIT_SET(field_val, 2);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));
    index = 0x12;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));
    DKITS_BIT_SET(field_val, 0);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));

    if (CTC_DKIT_MONITOR_SENSOR_TEMP == type)
    {
        /*mask_write tbl-reg OmcMem 0x10 offset 0x0 0x00000001 0x00000001*/
        index = 0x10;
        cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
        (DRV_IOCTL(lchip, index, cmd, &field_val));
        DKITS_BIT_SET(field_val, 0);
        cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
        (DRV_IOCTL(lchip, index, cmd, &field_val));
    }
    else
    {
        /*mask_write tbl-reg OmcMem 0x10 offset 0x0 0x00000000 0x00000001*/
        index = 0x10;
        cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
        (DRV_IOCTL(lchip, index, cmd, &field_val));
        DKITS_BIT_UNSET(field_val, 0);
        cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
        (DRV_IOCTL(lchip, index, cmd, &field_val));
    }
    sal_task_sleep(10);

    /*mask_write tbl-reg OmcMem 0x12 offset 0x0 0x00000001 0x00000001*/
    index = 0x12;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));
    DKITS_BIT_SET(field_val, 0);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    (DRV_IOCTL(lchip, index, cmd, &field_val));


    /*Wait((mask_read tbl-reg OmcMem 0xa offset 0x0 0x00000004) =1)*/
    index = 0xa;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    timeout = SYS_DKIT_TSINGMA_SENSOR_TIMEOUT;
    while(timeout)
    {
        timeout--;
        (DRV_IOCTL(lchip, index, cmd, &field_val));
        if (DKITS_IS_BIT_SET(field_val, 2))
        {
            break;
        }
        sal_task_sleep(1);
    }
    if (0 == timeout)
    {
        return CLI_ERROR;
    }

    /*mask_write tbl-reg OmcMem 0x11 offset 0x0 0x00000006 0x00000006*/
    index = 0x11;
    cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
    DRV_IOCTL(lchip, index, cmd, &field_val);
    DKITS_BIT_SET(field_val, 1);
    DKITS_BIT_SET(field_val, 2);
    cmd = DRV_IOW(OmcMem_t, OmcMem_data_f);
    DRV_IOCTL(lchip, index, cmd, &field_val);

    if (CTC_DKIT_MONITOR_SENSOR_TEMP == type)
    {
        /*read-reg OmcMem 0xd offset 0x0*/
        index = 0xd;
        cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
        DRV_IOCTL(lchip, index, cmd, &field_val);
        _ctc_tm_dkit_get_temp_with_code(lchip, field_val, p_value);
    }
    else
    {
        /*read-reg OmcMem 0xc offset 0x0*/
        index = 0xc;
        cmd = DRV_IOR(OmcMem_t, OmcMem_data_f);
        DRV_IOCTL(lchip, index, cmd, &field_val);
        *p_value =  (field_val*1232)/1023;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_tm_dkit_monitor_show_sensor_temperature(uint8 lchip)
{
    uint32 temp_value = 0;
    uint32 ret = 0;

    ret = _ctc_tm_dkit_monitor_sensor(lchip, CTC_DKIT_MONITOR_SENSOR_TEMP, &temp_value);
    if (ret)
    {
        CTC_DKIT_PRINT("Read temperature fail!!!\n");
        return CLI_ERROR;
    }
    CTC_DKIT_PRINT("Temperature is %d C\n", temp_value);

    return CLI_SUCCESS;
}


STATIC int32
_ctc_tm_dkit_monitor_show_sensor_voltage(uint8 lchip)
{
    uint32 vol_value = 0;
    uint32 ret = 0;

    ret = _ctc_tm_dkit_monitor_sensor(lchip, CTC_DKIT_MONITOR_SENSOR_VOL, &vol_value);
    if (ret)
    {
        CTC_DKIT_PRINT("Read voltage fail!!!\n");
        return CLI_ERROR;
    }
    CTC_DKIT_PRINT("Voltage is %d mV\n", vol_value);

    return CLI_SUCCESS;

}

STATIC void
_ctc_tm_dkit_monitor_temperature_handler(void* arg)
{
    uint32 ret = 0;
    sal_time_t tv;
    char* p_time_str = NULL;
    sal_file_t p_file = NULL;
    uint32 temperature = 0;
    ctc_dkit_monitor_para_t* para = (ctc_dkit_monitor_para_t*)arg;
    uint8 lchip = para->lchip;

    if (para->log)
    {
        p_file = sal_fopen(para->str, "wt");
        if (!p_file)
        {
            CTC_DKIT_PRINT("Open file %s fail!!!\n", para->str);
            return;
        }
        sal_fclose(p_file);
        p_file = NULL;
    }

    while (1)
    {
        if (para->log && (NULL == p_file))
        {
            p_file = sal_fopen(para->str, "a");
        }
        /*get systime*/
        sal_time(&tv);
        p_time_str = sal_ctime(&tv);
        ret = _ctc_tm_dkit_monitor_sensor(lchip, CTC_DKIT_MONITOR_SENSOR_TEMP, &temperature);
        if (ret)
        {
            CTC_DKITS_PRINT_FILE(p_file, "Read temperature fail!!\n");
        }
        else if (temperature >= para->temperature)
        {
            CTC_DKITS_PRINT_FILE(p_file, "t = %-4d, %s", temperature, p_time_str);
        }

        if (temperature >= para->power_off_temp) /*power off*/
        {

            CTC_DKITS_PRINT_FILE(p_file, "Power off!!!\n");
            goto END;
        }
        sal_task_sleep(para->interval*1000);

        if(0 == para->enable)
        {
            goto END;
        }
    }

END:
    if (p_file)
    {
        sal_fclose(p_file);
    }
    return;
}

STATIC int32
_ctc_tm_dkit_monitor_temperature(void* p_para)
{
    int ret = 0;
    uint8 lchip = 0;
    uint8 task_id = CTC_DKIT_MONITOR_TASK_TEMPERATURE;
    ctc_dkit_monitor_para_t* p_monitor_para = (ctc_dkit_monitor_para_t*)p_para;
    char buffer[SAL_TASK_MAX_NAME_LEN] = {0};

    DKITS_PTR_VALID_CHECK(p_monitor_para);
    lchip = p_monitor_para->lchip;
    CTC_DKIT_LCHIP_CHECK(lchip);
    DKITS_PTR_VALID_CHECK(g_usw_dkit_master[lchip]);

    if ((p_monitor_para->enable) && (NULL == g_usw_dkit_master[lchip]->monitor_task[task_id].monitor_task))
    {
        if (NULL == g_usw_dkit_master[lchip]->monitor_task[task_id].para)
        {
            g_usw_dkit_master[lchip]->monitor_task[task_id].para
                   = (ctc_dkit_monitor_para_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_dkit_monitor_para_t));
            if (NULL == g_usw_dkit_master[lchip]->monitor_task[task_id].para)
            {
                return CLI_ERROR;
            }
        }
        sal_memcpy(g_usw_dkit_master[lchip]->monitor_task[task_id].para, p_para , sizeof(ctc_dkit_monitor_para_t));

        sal_sprintf(buffer, "Temperature-%d", lchip);
        ret = sal_task_create(&g_usw_dkit_master[lchip]->monitor_task[task_id].monitor_task,
                                                  buffer,
                                                  SAL_DEF_TASK_STACK_SIZE,
                                                  SAL_TASK_PRIO_DEF,
                                                  _ctc_tm_dkit_monitor_temperature_handler,
                                                  g_usw_dkit_master[lchip]->monitor_task[task_id].para);

        if (0 != ret)
        {
            CTC_DKIT_PRINT("Temperature monitor task create fail!\n");
            return CLI_ERROR;
        }
    }
    else if(0 == p_monitor_para->enable)
    {
        if (g_usw_dkit_master[lchip]->monitor_task[task_id].para)
        {
            sal_memset(g_usw_dkit_master[lchip]->monitor_task[task_id].para, 0 ,sizeof(ctc_dkit_monitor_para_t));
        }
        if (g_usw_dkit_master[lchip]->monitor_task[task_id].monitor_task)
        {
            sal_task_destroy(g_usw_dkit_master[lchip]->monitor_task[task_id].monitor_task);
            g_usw_dkit_master[lchip]->monitor_task[task_id].monitor_task = NULL;
        }
    }

    return CLI_SUCCESS;
}

int32
ctc_tm_dkit_monitor_show_sensor_result(void* p_para)
{
    ctc_dkit_monitor_para_t* p_monitor_para = (ctc_dkit_monitor_para_t*)p_para;
    DKITS_PTR_VALID_CHECK(p_para);
    CTC_DKIT_LCHIP_CHECK(p_monitor_para->lchip);
    DRV_INIT_CHECK(p_monitor_para->lchip);

    if (CTC_DKIT_MONITOR_SENSOR_TEMP == p_monitor_para->sensor_mode)
    {
        _ctc_tm_dkit_monitor_show_sensor_temperature(p_monitor_para->lchip);
    }
    else if (CTC_DKIT_MONITOR_SENSOR_VOL == p_monitor_para->sensor_mode)
    {
        _ctc_tm_dkit_monitor_show_sensor_voltage(p_monitor_para->lchip);
    }
    else if(CTC_DKIT_MONITOR_SENSOR_TEMP_NOMITOR == p_monitor_para->sensor_mode)
    {
        _ctc_tm_dkit_monitor_temperature(p_para);
    }

    return CLI_SUCCESS;
}


int32
ctc_tm_dkit_interface_show_pcs_status(uint8 lchip, uint16 mac_id)
{
    return CLI_SUCCESS;
}

