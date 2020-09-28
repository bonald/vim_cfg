/****************************************************************************
 * lcm_lcshtlk.c:    lcm communicate with lcsh
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       weij
 * Date         :       2010-08-07
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "sal_common.h"
#include "lcm_srv.h"
#include "glb_const.h"
#include "glb_if_define.h"
#include "glb_hw_define.h"
#ifndef _GLB_UML_SYSTEM_
#include "phy_api.h"
#include "clock_api.h"
#include "epld_api.h"
#include "fiber_api.h"
#include "sensor_api.h"
#include "fan_api.h"
#include "power_api.h"
#include "vsc3308_api.h"
#include "eeprom_api.h"
#include "gpio_api.h"
#include "mux_api.h"
#include "i2c_cpm.h"
#include "i2c_smbus.h"
#include "spi_gpio.h"
#endif /* _GLB_UML_SYSTEM_ */
#include "ctc_api.h"
#include "lcm_mgt.h"
#include "lcm_debug.h"
#include "sysenv_api.h"

// TODO: Commented by xgu for compile, 20121120  #include <drv_io.h> 
#include "ctclib_show.h"
// TODO: Commented by xgu for compile, 20121120 #include "sys_humber_ftm.h"
#include <inttypes.h>
/****************************************************************************
 *  
* Global and Declaration
*
*****************************************************************************/
lcm_srv_cb_t lcm_lcsh_srv_cb_tbl[];

extern void *epld_base;
#ifndef _GLB_UML_SYSTEM_
extern int32 cpu_debug_gephy_mdio_read(unsigned int phy_addr, unsigned int reg, unsigned int* value);
extern int32 cpu_debug_gephy_mdio_write(unsigned int phy_addr, unsigned int reg, unsigned int value);
#ifndef CNOS_OS
extern int32 if_build_port_shortname_by_slot_port (char *ifname, uint16 slot_id, uint16 port_id, uint16 sub_port_id);
#endif
#endif
/*********************************************************************
 * Name    : lcm_lcsh_msg_rx_test1
 * Purpose : this function is used to test rcv message form lcsh and return value
 * Input   : 
 * Output  : 
 * Return  : hsrv_e_***  - hal server return type
 * Note    : if the fid is already alloced, it will return HSRV_E_ENTRY_EXIST
*********************************************************************/
int32 
lcm_lcsh_msg_rx_test1(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    int32 ret = 0;
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    lcapi_lcm_lcsh_test1_msg_t lcapi_lcm_lcsh_test1_msg;
        
    sal_memset(&st_lcapi_lcm_lcsh_msg, 0 , sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&lcapi_lcm_lcsh_test1_msg, 0 , sizeof(lcapi_lcm_lcsh_test1_msg));

    lcapi_lcm_lcsh_test1_msg.int_value = 111;
    sal_snprintf(lcapi_lcm_lcsh_test1_msg.str_value, 100, "I get it :)");
    sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, &lcapi_lcm_lcsh_test1_msg, sizeof(lcapi_lcm_lcsh_test1_msg_t));
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_TEST1;
    st_lcapi_lcm_lcsh_msg.msg_len = sizeof(lcapi_lcm_lcsh_test1_msg_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
}
extern int32 asic_gb_gephy_xgmdio_read(uint8 bus, uint8 phy_addr, uint16 reg, uint8 dev, uint16* value);
extern int32 asic_gb_gephy_xgmdio_write(uint8 bus, uint8 phy_addr, uint16 reg, uint8 dev, uint16 value);

/*********************************************************************
 * Name    : lcsh_lcm_msg_rx_read_regs
 * Purpose : this function is used to test rcv message form lcsh and return value
 * Input   : 
 * Output  : 
 * Return  : hsrv_e_***  - hal server return type
 * Note    : if the fid is already alloced, it will return HSRV_E_ENTRY_EXIST
*********************************************************************/
int32 
lcsh_lcm_msg_rx_read_regs(lcm_sess_t* p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    read_regs_reg_t* req_msg;
    reg_values_t ret_val[MAX_DIAG_ENTRY_NUM];
#ifndef _GLB_UML_SYSTEM_
    epld_para_t epld_para;
    fiber_para_t fiber_para;/* added by tongzb for read fiber 2016-07-08 */
    uint8 buf[MAX_DIAG_ENTRY_NUM];
    uint16 buf_u16[MAX_DIAG_ENTRY_NUM];
    eeprom_para_t eeprom_para;
    sysenv_para_t sysenv_para;
#endif
    int ret = 0;

    if(NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess o req\n");
        return LCM_E_INVALID_PARAM;
    }
    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));
#ifdef _GLB_UML_SYSTEM_

    req_msg = (read_regs_reg_t* )(req->msg);

    LCM_LOG_DEBUG(LCM_LCSH, "Recv message: %d %d %d %d\n",
                            req_msg->opcode, req_msg->chip, req_msg->addr, req_msg->count);
    
    ret_val[0].addr = 0x1234;
    ret_val[0].value = 0x5678;
    ret_val[1].addr = 0xabcd;
    ret_val[1].value = 0xdcba;
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_READ_REGS;
    sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t)*2);
    st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t)*2 + LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }

    return ret;
#else
    uint32 port_id;
    uint32 chip;
    uint32 address, device, ret_cnt, ii;
    uint16 tmpval;
    uint32 tmpval_32;
    uint16 tmpval_16;
    uint32 count = 0;
    FILE* fp;
    uint8* tmpbuf;
    //uint8 byte_width;
    uint8 indirect = 0;
    
    ret_cnt = 0;
    req_msg = (read_regs_reg_t* )(req->msg);
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_READ_REGS;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;

    switch(req_msg->opcode) 
    {
        case REG_GEPHY_R:
            port_id = req_msg->chip - 1;
            address = req_msg->addr;
            count = req_msg->count;
            for(ii = 0; ii < count; ii++)
            {
                ret = gephy_reg_read(port_id, (uint8)(address + ii), &tmpval);
                if(ret)
                {
                    break;
                }
                ret_val[ii].addr = address + ii;
                ret_val[ii].value = tmpval;
                ret_cnt++;
            }
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;
            
        case REG_XGPHY_R:
            port_id = req_msg->chip - 1;
            device = (req_msg->addr >> 16) & 0xffff;
            address = (req_msg->addr) & 0xffff;
            count = req_msg->count;
            for(ii = 0; ii < count; ii++)
            {
                ret = xgphy_reg_read(port_id, (uint8)device, (uint16)(address+ii), &tmpval);

                if(ret)
                {
                    break;
                }
                ret_val[ii].addr = address + ii;
                ret_val[ii].value = tmpval;
                ret_cnt++;
            }
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;

        /* added by tongzb for read fiber 2016-07-08 */
        case REG_FIBER_R:
            sal_memset(&fiber_para, 0, sizeof(fiber_para_t));
            port_id =req_msg->chip - 1;
            ret = (req_msg->addr >> 8)&0xff;
            if(ret==3)
            {
                fiber_para.subdev = FIBER_DEV_ADDR3;
            }
            else if(ret == 2)
            {
                fiber_para.subdev = FIBER_DEV_ADDR2;
            }
            else
            {
                fiber_para.subdev = FIBER_DEV_ADDR1;
            }
            fiber_para.offset = (req_msg->addr)&0xff;
            fiber_para.len = req_msg->count > LCAPI_LCM_LCSH_REG_NUM ? LCAPI_LCM_LCSH_REG_NUM : req_msg->count;
            fiber_para.val = buf;
            ret = fiber_read(port_id, &fiber_para);
            if(ret)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                for(ii = 0; ii< fiber_para.len; ii++)
                {
                    ret_val[ii].addr = fiber_para.offset + ii;
                    ret_val[ii].value = fiber_para.val[ii];
                }
                sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * fiber_para.len);
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE + sizeof(reg_values_t) * fiber_para.len;
            }
            break;
         /* added end for read fiber */
        case REG_EPLD_R:
            sal_memset(&epld_para, 0, sizeof(epld_para_t));
            chip = req_msg->chip;
            epld_para.addr = req_msg->addr;
            count = req_msg->count;
            for(ii = 0; ii < count; ii++)
            {
                ret = epld_read(chip, &epld_para);
                if(ret)
                {
                    break;
                }
                ret_val[ii].addr = epld_para.addr++;
                ret_val[ii].value = epld_para.val;
                ret_cnt++;
            }
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
}
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;
            
        case REG_XGPHY_LOAD:
            st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            port_id = req_msg->chip - 1;
#if 1
            tmpbuf = (uint8* )LCM_MALLOC(CTCLIB_MEM_LCM_MODULE, 16);
            fp = sal_fopen(SWITCH_PREFIX"/etc/vsc8488_firmware_mdio.bin", "rb");
            if(fp < 0) 
            {
                LCM_FREE(CTCLIB_MEM_LCM_MODULE, tmpbuf);
                break;
            }
            tmpval_32=0;
            while((count = sal_fread(tmpbuf, 1, 16, fp))>0)
            {
                for(ii=0; ii<count/4; ii++)
                {
                    tmpval_16 = (tmpbuf[ii*4+3]<<8) | tmpbuf[ii*4+2];
                    xgphy_reg_write(port_id, 0x1f, tmpval_32++, tmpval_16);
                    tmpval_16 = (tmpbuf[ii*4+1]<<8) | tmpbuf[ii*4];
                    xgphy_reg_write(port_id, 0x1f, tmpval_32++, tmpval_16);
                }        
            }
            
            while(tmpval_32 < 0x8000)
            {
                xgphy_reg_write(port_id, 0x1f, tmpval_32++, 0);
            }
            st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_OK;
            sal_fclose(fp);
            LCM_FREE(CTCLIB_MEM_LCM_MODULE, tmpbuf);
#else
            LCM_LOG_DEBUG(LCM_LCSH, "QT2X25 start......."); 
            fp = sal_fopen(SWITCH_PREFIX"/etc/qt2x25_firmware_mdio.bin", "rb");
            if(NULL == fp) {
                LCM_LOG_DEBUG(LCM_LCSH,"Open SWITCH_PREFIX/etc/qt2x25_firmware_mdio.bin failed.");
                break;
            }

            tmpbuf = (uint8* )LCM_MALLOC(CTCLIB_MEM_LCM_MODULE, 0x6000);
            if(NULL == tmpbuf)
            {
                sal_fclose(fp);
                LCM_LOG_DEBUG(LCM_LCSH,"QT2x25 load firmware, out of memory");
                break;
            }
            count = sal_fread(tmpbuf, 1, 0x6000, fp);
            LCM_LOG_DEBUG(LCM_LCSH, "QT2x25 load, read %d bytes", count); 
            if(count < 0)
            {
                    LCM_FREE(CTCLIB_MEM_LCM_MODULE, tmpbuf);
                    LCM_LOG_DEBUG(LCM_LCSH,"QT2x25 load, read failed"); 
                    sal_fclose(fp);
                    break;
            }
            
            device = 3;
            for(ii = 0; ii < 0x4000; ii++)
            {
                address = 0x8000 + ii;
                ret = xgphy_reg_write(port_id, device, address, tmpbuf[ii]);
                if(ret)
                {
                    LCM_FREE(CTCLIB_MEM_LCM_MODULE, tmpbuf);
                    sal_fclose(fp);
                    LCM_LOG_DEBUG(LCM_LCSH,"QT2x25 load, write failed"); 
                    break;
                }
            }

            device = 4;
            for(ii = 0x4000; ii < count; ii++)
            {
                address = 0x8000 + ii;
                ret = xgphy_reg_write(port_id, device, address, tmpbuf[ii]);
                if(ret)
                {
                    LCM_FREE(CTCLIB_MEM_LCM_MODULE, tmpbuf);
                    sal_fclose(fp);
                    LCM_LOG_DEBUG(LCM_LCSH,"QT2x25 load, write failed"); 
                    break;
                }
            }
            st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_OK;
            LCM_FREE(CTCLIB_MEM_LCM_MODULE, tmpbuf);
            sal_fclose(fp);
#endif            
            break;

        case REG_SENSOR_R:            
            chip = req_msg->chip;               
            ret = sensor_reg_read(chip, req_msg->addr, buf, req_msg->count);
            /* IIC operate, ret >=0 mean success. jqiu modify.bug 13956. 2011-01-22*/
            if(ret<0)
            {
                ret_cnt = 0;
            }
            else
            {
                ret_val[0].addr = req_msg->addr;
                ret_cnt = 1;
                if(req_msg->count == 1)
                    ret_val[0].value = buf[0];
                else if(req_msg->count == 2)
                    ret_val[0].value = (buf[0]<<8) + buf[1];
                else
                    ret_cnt = 0;                
            }    
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;   
        case REG_SENSOR_TMP_R:
            chip = req_msg->chip;
            ret = sensor_dev_get_temp(chip, req_msg->addr, (int16*)&tmpval_16);
            if(ret != RESULT_OK)
            {
                ret_cnt = 0;
            }
            else
            {
                ret_val[0].addr = req_msg->addr;
                ret_val[0].value = tmpval_16;
                ret_cnt = 1;
            }
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;
        case REG_GPIO_CHIP_R:            
            chip = req_msg->chip;               
            ret = gpio_reg_read(chip, req_msg->addr, buf, req_msg->count);
            if(ret<0)
            {
                ret_cnt = 0;
            }
            else
            {
                ret_val[0].addr = req_msg->addr;
                ret_cnt = 1;
                ret_val[0].value = buf[0];                 
            }    
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;  
        case REG_MUX_CHIP_R:            
            chip = req_msg->chip;               
            ret = mux_reg_read(chip, req_msg->addr, buf, req_msg->count);
            if(ret<0)
            {
                ret_cnt = 0;
            }
            else
            {
                ret_val[0].addr = req_msg->addr;
                ret_cnt = 1;
                ret_val[0].value = buf[0];                 
            }    
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break; 
        case REG_FAN_R:            
            chip = req_msg->chip;               
            ret = fan_reg_read(chip, req_msg->addr, buf);
            /* IIC operate, ret >=0 mean success. jqiu modify.bug 13956. 2011-01-22*/
            if(ret<0)
            {
                ret_cnt = 0;
            }
            else
            {
                ret_val[0].addr = req_msg->addr;
                ret_cnt = 1;
                ret_val[0].value = buf[0];      
            }    
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;   
        case REG_FAN_SPEED_R:            
            chip = req_msg->chip;               
            ret = fan_get_real_speed(chip, buf_u16);
            /* IIC operate, ret >=0 mean success. jqiu modify.bug 13956. 2011-01-22*/
            if(ret<0)
            {
                ret_cnt = 0;
            }
            else
            {
                ret_val[0].addr = 0;
                ret_val[0].value = buf_u16[0]; 
                ret_val[1].addr = 1;
                ret_val[1].value = buf_u16[1]; 
                ret_val[2].addr = 2;
                ret_val[2].value = buf_u16[2]; 
                ret_val[3].addr = 3;
                ret_val[3].value = buf_u16[3]; 
                ret_cnt = 4;
            }    
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;               
        case REG_PSU_R:            
            chip = (req_msg->chip&0xf);
            if(req_msg->chip&0xf0)
            {
                ret = psu_pmbus_read(chip, req_msg->addr, buf, req_msg->count);
            }
            else
            {
                ret = psu_normal_read(chip, req_msg->addr, buf, req_msg->count);
            }
            if(ret >= 0)
            {
                for(ii=0; ii<req_msg->count; ii++)
                {
                    ret_val[ii].addr = req_msg->addr + ii;
                    ret_val[ii].value = buf[ii];
                    ret_cnt ++;
                }
            }
            else
            {
                ret_cnt =0;
            }
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;               
        case REG_VSC3308_R:
            chip = req_msg->chip;
            address = req_msg->addr;
            count = req_msg->count;
            for(ii = 0; ii < count; ii++)
            {
                ret = vsc3308_read(chip, address, &buf[ii]);
                if(ret)
                {
                    break;
                }
                ret_val[ii].addr = address++;
                ret_val[ii].value = buf[ii];
                ret_cnt++;
            }
            
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;
        case REG_EEPROM_R:
            sal_memset(&eeprom_para, 0, sizeof(eeprom_para_t));
            chip = req_msg->chip;
            eeprom_para.offset = req_msg->addr;
            eeprom_para.len = req_msg->count;
            eeprom_para.p_val = LCM_CALLOC(CTCLIB_MEM_LCM_MODULE, eeprom_para.len);
            if (eeprom_para.p_val)
            {
                ret = eeprom_read(chip, &eeprom_para);
                if (ret >=0)
                {
                    for (ii= 0; ii < eeprom_para.len; ii++)
                    {
                        ret_val[ii].addr = eeprom_para.offset + ii;
                        ret_val[ii].value = eeprom_para.p_val[ii];
                        ret_cnt ++;
                    }
                }
                else
                {
                    ret_cnt =0;
                }
                LCM_FREE(CTCLIB_MEM_LCM_MODULE, eeprom_para.p_val);
            }
            else
            {
                ret_cnt =0;
            }       
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;  
        case REG_SYSENV_R:
            sal_memset(&sysenv_para, 0, sizeof(sysenv_para_t));
            chip = req_msg->chip;
            sysenv_para.offset = req_msg->addr;
            sysenv_para.len = req_msg->count;
            sysenv_para.p_val = LCM_CALLOC(CTCLIB_MEM_LCM_MODULE, sysenv_para.len);
            if (sysenv_para.p_val)
            {
                ret = sysenv_read(chip, &sysenv_para);
                if (ret >=0)
                {
                    for (ii= 0; ii < sysenv_para.len; ii++)
                    {
                        ret_val[ii].addr = sysenv_para.offset + ii;
                        ret_val[ii].value = sysenv_para.p_val[ii];
                        ret_cnt ++;
                    }
                }
                else
                {
                    ret_cnt =0;
                }
                LCM_FREE(CTCLIB_MEM_LCM_MODULE, sysenv_para.p_val);
            }
            else
            {
                ret_cnt =0;
            }       
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;  
        case REG_I2C_BRIDGE_R:
            ///TODO::here need add driver
#if 0            
            /*add by guhg,2015-11-27,read i2c reg through i2c bridge*/
            ret = raw_bridge_i2c_read(req_msg->addr_1 & 0xff,(req_msg->chip >> 16) & 0xff,(req_msg->chip >> 8) & 0xff 
                         ,req_msg->chip  & 0xff, req_msg->addr, (req_msg->addr_1>> 8) & 0xff, buf, req_msg->count);
            if(ret >= 0)
            {
                for(ii=0; ii<req_msg->count; ii++)
                {
                    ret_val[ii].addr = req_msg->addr + ii;
                    ret_val[ii].value = buf[ii];
                    ret_cnt ++;
                }
            }
            else
            {
                ret_cnt =0;
            }
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
#endif            
            break; 

        case REG_I2C_R:       
            /* Added by qicx for bug 21474: 2012-11-27 */
            /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
#ifndef _CTC_X86_64_
            ret = raw_i2c_cpm_read((req_msg->addr_1&0xff), (req_msg->chip & 0xff), req_msg->addr, (req_msg->addr_1>>8), 
                            buf, (req_msg->count & 0xff));
#else
            ret = raw_i2c_smbus_read((req_msg->addr_1&0xff), (req_msg->chip & 0xff), req_msg->addr, (req_msg->addr_1>>8), 
                            buf, (req_msg->count & 0xff));
#endif
            if(ret >= 0)
            {
                for(ii=0; ii<req_msg->count; ii++)
                {
                    ret_val[ii].addr = req_msg->addr + ii;
                    ret_val[ii].value = buf[ii];
                    ret_cnt ++;
                }
            }
            else
            {
                ret_cnt =0;
            }
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break; 
        case REG_SPI_R:       
            /* Added by qicx for bug 21474: 2012-11-27 */
#if 1      
            /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
            ret = raw_gpio_spi_read(req_msg->addr, req_msg->addr_1,buf, (req_msg->count & 0xff));
            if(ret >= 0)
            {
                for(ii=0; ii<req_msg->count; ii++)
                {
                    ret_val[ii].addr = req_msg->addr + ii;
                    ret_val[ii].value = buf[ii];
                    ret_cnt ++;
                }
            }
            else
            {
                ret_cnt =0;
            }
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
#endif            
            break;         
            
#if 1	/* chani 20130930 for DPLL */
        case REG_CLOCK_R:
            chip = req_msg->chip;
            address = req_msg->addr;
            count = req_msg->count;
            indirect = req_msg->indirect;
            
            for(ii = 0; ii < count; ii++)
            {
                if (indirect)
                {
                    ret = clock_read_indirect(chip, (uint16)(address + ii), &tmpval_32);
                }
                else
                {
                    ret = clock_read(chip, (uint16)(address + ii), &tmpval_32);
                }
                if(ret)
                {
                    break;
                }
                ret_val[ii].addr = address + ii;
                ret_val[ii].value = tmpval_32;
                ret_cnt++;
            }
            sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
            if(ret_cnt <= 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;   
#endif /* chani 20130930 for DPLL */                  
                      
        default:
            break;
    }
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
#endif  /* _GLB_UML_SYSTEM_ */     
}
/*********************************************************************
 * Name    : lcsh_lcm_msg_rx_write_regs
 * Purpose : this function is write register
 * Input   : 
 * Output  : 
 * Return  : hsrv_e_***  - hal server return type
 * Note    : if the fid is already alloced, it will return HSRV_E_ENTRY_EXIST
*********************************************************************/
int32 
lcsh_lcm_msg_rx_write_regs(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    write_regs_reg_t* req_msg;
    int ret = 0;

    if(NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess o req\n");
        return LCM_E_INVALID_PARAM;
    }

    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));

#ifdef _GLB_UML_SYSTEM_

    req_msg = (write_regs_reg_t* )(req->msg);

    LCM_LOG_DEBUG(LCM_LCSH, "Recv message: %d %d %d %d\n",
                            req_msg->opcode, req_msg->chip, req_msg->addr, req_msg->value[0]);

    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }

    return ret;
#else
    uint32 port_id;
    uint32 chip;
    uint32 address,device;
    uint16 tmpval;
    uint32 tmpval_32;
#ifdef HAVE_PHY_DRV
    uint16 tmpval_16;
    uint32 enable;
#endif /* !HAVE_PHY_DRV */
    epld_para_t epld_para;
    eeprom_para_t eeprom_para;
    sysenv_para_t sysenv_para;
    fiber_para_t fiber_para;
    uint8 tmpval_8;
    //uint8 byte_width;
    uint8 indirect = 0;
    uint8 offset_buf[LCSH_I2C_BUF_LEN];
    uint8 i = 0;
    uint32 ret_cnt = 0;
    reg_values_t ret_val[MAX_DIAG_ENTRY_NUM];
    uint16 cfg_speed[FAN_SPEED_LEVEL_MAX];

    //glb_vtss_phy_recov_clk_t phy_clk_port;
    glb_vtss_phy_clock_conf_t phy_clk_conf;
    
    req_msg = (write_regs_reg_t* )(req->msg);
    switch(req_msg->opcode) 
    {  
        case REG_GEPHY_W:
            port_id = req_msg->chip -1 ;
            address = req_msg->addr;
            tmpval = req_msg->value[0];

            ret = gephy_reg_write(port_id, (uint8)address, tmpval);

            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;
        case REG_XGPHY_W:
            port_id = req_msg->chip - 1;
            device = (req_msg->addr >> 16) & 0xffff;
            address = (req_msg->addr) & 0xffff;
            tmpval = req_msg->value[0];

            ret = xgphy_reg_write(port_id, device, address, tmpval);


            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;     
     
        /* added by tongzb for write fiber 2016-07-08 */
        case REG_FIBER_W:
            sal_memset(&fiber_para, 0, sizeof(fiber_para_t));
            port_id = req_msg->chip - 1;
            ret = (req_msg->addr >> 8)&0xff;
            if(ret==3)
            {
                fiber_para.subdev = FIBER_DEV_ADDR3;
            }
            else if(ret == 2)
            {
                fiber_para.subdev = FIBER_DEV_ADDR2;
            }
            else
            {
                fiber_para.subdev = FIBER_DEV_ADDR1;
            }
            fiber_para.offset = (req_msg->addr)&0xff;
            if(fiber_para.subdev==FIBER_DEV_ADDR3)
            {
                fiber_para.len = 2;
                tmpval = (req_msg->value[0] & 0xffff);
                fiber_para.val = (uint8*)&tmpval;
            }
            else
            {
                fiber_para.len = 1;
                tmpval_8 = (req_msg->value[0] & 0xff);
                fiber_para.val = &tmpval_8;
            }

            ret = fiber_write(port_id, &fiber_para);

            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;
        /*added end for write qsfp */

        case REG_EPLD_W:
            sal_memset(&epld_para, 0, sizeof(epld_para_t));
            chip = req_msg->chip;
            epld_para.addr = req_msg->addr;
            epld_para.val = req_msg->value[0];

            ret = epld_write(chip, &epld_para);

            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break; 

        case REG_SENSOR_W:            
            chip = req_msg->chip;
            tmpval_8 = req_msg->value[0];
            ret = sensor_reg_write(chip, req_msg->addr, &tmpval_8, 1);
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            /* IIC operate, ret >=0 mean success. jqiu modify.bug 13956. 2011-01-22*/
            if(ret<0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;    

        case REG_SENSOR_TMP_W:            
            chip = req_msg->chip;
            ret = sensor_dev_set_temp(chip, req_msg->addr, (int16)req_msg->value[0]);
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;
        case REG_GPIO_CHIP_W:            
            chip = req_msg->chip;
            tmpval_8 = req_msg->value[0];
            ret = gpio_reg_write(chip, req_msg->addr, &tmpval_8, 1);
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret<0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;  
        case REG_MUX_CHIP_W:            
            chip = req_msg->chip;
            tmpval_8 = req_msg->value[0];
            ret = mux_reg_write(chip, req_msg->addr, &tmpval_8, 1);
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret<0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;
        case REG_FAN_W:            
            chip = req_msg->chip;
            tmpval_8 = req_msg->value[0];
            ret = fan_reg_write(chip, req_msg->addr, tmpval_8);
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            /* IIC operate, ret >=0 mean success. jqiu modify.bug 13956. 2011-01-22*/
            if(ret<0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;  
        case REG_FAN_SPEED_W:            
            chip = req_msg->chip;
            cfg_speed[FAN_SPEED_BOTTOM] = req_msg->value[0];
            cfg_speed[FAN_SPEED_LOW] = req_msg->value[1];
            cfg_speed[FAN_SPEED_HIGH] = req_msg->value[2];
            cfg_speed[FAN_SPEED_FULL] = req_msg->value[3]; 
            ret = fan_user_config_speed_rate(chip, cfg_speed, FAN_SPEED_LEVEL_MAX);
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            /* IIC operate, ret >=0 mean success. jqiu modify.bug 13956. 2011-01-22*/
            if(ret<0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;  
        case REG_PSU_W:            
            chip = req_msg->chip;
            tmpval_8 = req_msg->value[0];
            ret = psu_reg_write(chip, req_msg->addr, tmpval_8, 1);
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            /* IIC operate, ret >=0 mean success. jqiu modify.bug 13956. 2011-01-22*/
            if(ret<0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;       
        case REG_VSC3308_W:
            chip = req_msg->chip;
            tmpval_8 = req_msg->value[0];
            ret = vsc3308_write(chip, req_msg->addr, tmpval_8);
            
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret<0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;
        case REG_VSC3308_CONF:
            chip = req_msg->chip;
            ret = vsc3308_config_input_output_pin(chip, (req_msg->addr & 0xff), (req_msg->addr_1 & 0xff));

            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret < 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;
        case REG_PHY_CLK_CONF:
            port_id = req_msg->chip - 1;
            //phy_clk_port = req_msg->addr;
            phy_clk_conf.src = (req_msg->value[0]>>16) & 0xff;
            phy_clk_conf.freq =(req_msg->value[0]>>8) & 0xff; 
            phy_clk_conf.squelch = req_msg->value[0] & 0xff;
            
            ret = phy_cfg_clock(port_id, phy_clk_conf);
            
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;
        case REG_EEPROM_W:
            sal_memset(&eeprom_para, 0, sizeof(eeprom_para_t));
            chip = req_msg->chip;
            eeprom_para.offset = req_msg->addr;
            tmpval_8 = req_msg->value[0];
            eeprom_para.p_val = &tmpval_8;
            eeprom_para.len = 1;

            ret = eeprom_write(chip, &eeprom_para);
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret < 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;      
        case REG_SYSENV_W:
            sal_memset(&sysenv_para, 0, sizeof(sysenv_para_t));
            chip = req_msg->chip;
            sysenv_para.offset = req_msg->addr;
            tmpval_8 = req_msg->value[0];
            sysenv_para.p_val = &tmpval_8;
            sysenv_para.len = 1;

            ret = sysenv_write(chip, &sysenv_para);
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret < 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;      
        case REG_I2C_W:
            /* Added by qicx for bug 21474: 2012-11-27 */
            /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */            
            tmpval_8 = (req_msg->addr_1 >> 8) & 0xff;  /* get value length */
            for(i=0;i<tmpval_8;i++)
            {
                offset_buf[i] = req_msg->value[i];
            }
            
#ifndef _CTC_X86_64_
            ret = raw_i2c_cpm_write((req_msg->addr_1&0xff), (req_msg->chip&0xffff), req_msg->addr, 
                    ((req_msg->addr_1>>16) & 0xff), offset_buf, tmpval_8);
#else
            ret = raw_i2c_smbus_write((req_msg->addr_1&0xff), (req_msg->chip&0xffff), req_msg->addr, 
                    ((req_msg->addr_1>>16) & 0xff), offset_buf, tmpval_8);
#endif
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret < 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break; 
        case REG_I2C_RAW:
            /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */           
            tmpval_8 = (req_msg->addr_1 >> 8) & 0xff;  /* get value length */
            if(req_msg->addr == 1)/* write */
            {
                for(i=0;i<tmpval_8;i++)
                {
                    offset_buf[i] = req_msg->value[i];
                }
            }
            ret = raw_i2c_cpm_transfer((req_msg->addr_1&0xff), (req_msg->chip&0xffff),req_msg->addr, offset_buf,tmpval_8);
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret >= 0)
            {
                if(req_msg->addr == 0)/* read */
                {
                    for(i=0; i<tmpval_8; i++)
                    {
                        ret_val[i].addr = i;
                        ret_val[i].value = offset_buf[i];
                        ret_cnt ++;
                    }
                    if(ret_cnt <= 0)
                    {
                        st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                        st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
                    }
                    else
                    {
                        sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, ret_val, sizeof(reg_values_t) * ret_cnt);
                        st_lcapi_lcm_lcsh_msg.msg_len = sizeof(reg_values_t) * ret_cnt + LCAPI_LCM_LCSH_HEAD_SIZE;
                    }                    
                }
                else
                {
                    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                }
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            
            break; 
        case REG_SPI_W:
#if 1       
            tmpval_8 = req_msg->addr_1& 0xff;  /* get value length */
            ret = raw_gpio_spi_write(req_msg->addr, (req_msg->addr_1>>8)&0xff,req_msg->value[0],tmpval_8);
            
            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret < 0)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
#endif
            break;         
                           
#if 1	/* chani 20130930 for DPLL */
        case REG_CLOCK_W:
            chip = req_msg->chip;
            address = req_msg->addr;
            tmpval_32 = req_msg->value[0];
            indirect = req_msg->indirect;
            
            if (indirect)
            {
                ret = clock_write_indirect(chip, (uint16)address, tmpval_32);
            }
            else
            {
                ret = clock_write(chip, (uint16)address, tmpval_32);
            }

            st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_REGS;
            if(ret)
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
                st_lcapi_lcm_lcsh_msg.errcode = LCAPI_LCM_LCSH_ERR;
            }
            else
            {
                st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
            }
            break;    
#endif /* chani 20130930 for DPLL */              
                     
        default:
            break;
    }
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
#endif  /* _GLB_UML_SYSTEM_ */     
}

/*********************************************************************
 * Name    : lcsh_lcm_msg_rx_get_port_info
 * Purpose : this function get port info
 * Input   : 
 * Output  : 
 * Return  : hsrv_e_***  - hal server return type
 * Note    : if the fid is already alloced, it will return HSRV_E_ENTRY_EXIST
*********************************************************************/
int32 
lcsh_lcm_msg_rx_get_port_info(lcm_sess_t* p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    showport_t* req_msg;
    showport_t st_msg;
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    portdesc_t *port_desc;
    int ret = 0;
    uint16 ii, port_num, counts;

    if(NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess o req\n");
        return LCM_E_INVALID_PARAM;
    }

    /*get local line card*/
    p_card= lcm_mgt_get_card();  
    if(NULL == p_card)
    {
        return -1;
    }

    port_num = p_card->port_num;
    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_msg, 0, sizeof(showport_t));

    req_msg = (showport_t* )(req->msg);
    if(GET_PORT_NUM == req_msg->opcode)
    {
        st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_GET_PORT_INFO;
        st_msg.opcode = GET_PORT_NUM;
        st_msg.port_num = port_num;
        sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, &st_msg, sizeof(showport_t));
        st_lcapi_lcm_lcsh_msg.msg_len = sizeof(showport_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
        if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
        {
            return ret;
        }
    }
    else if(GET_PORT_INFO == req_msg->opcode)
    {
        counts = req_msg->port_num;
        port_desc = (portdesc_t* )LCM_CALLOC(CTCLIB_MEM_LCM_MODULE, sizeof(portdesc_t) * counts);

        if (!port_desc)
        {
            ret = -1;
        }

        for(ii = 0; ii< counts; ii++)
        {
            if((req_msg->offset + ii) >= port_num)
            {
                counts = ii;
                break;
            }
            p_port = p_card->pp_port[req_msg->offset + ii];
            port_desc[ii].admin.up = p_port->port_cfg.enable;
            port_desc[ii].admin.speed= p_port->port_cfg.speed;
            port_desc[ii].admin.duplex = p_port->port_cfg.duplex;
            port_desc[ii].admin.loopback= p_port->port_cfg.loopback;
            port_desc[ii].curstatus.up = p_port->port_status.link_up;
            port_desc[ii].curstatus.speed= p_port->port_status.speed;
            port_desc[ii].curstatus.duplex= p_port->port_status.duplex;
            port_desc[ii].present = p_port->port_state;
            port_desc[ii].portno = p_port->port_idx;  
            port_desc[ii].mac_id = p_port->mac_idx;
            port_desc[ii].phy_type = p_port->phy_type;
            port_desc[ii].is_combo = p_port->is_combo;
        }
        st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_GET_PORT_INFO;
        sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, port_desc, sizeof(portdesc_t)*counts);
        st_lcapi_lcm_lcsh_msg.msg_len = sizeof(portdesc_t)*counts + LCAPI_LCM_LCSH_HEAD_SIZE;
        LCM_FREE(CTCLIB_MEM_LCM_MODULE, port_desc);
        if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
        {
            return ret;
        }
    }
    return ret;
  
}

/*********************************************************************
 * Name    : lcsh_lcm_msg_rx_conport_regs
 * Purpose : this function is used to test rcv message form lcsh and return value
 * Input   : 
 * Output  : 
 * Return  : hsrv_e_***  - hal server return type
 * Note    : if the fid is already alloced, it will return HSRV_E_ENTRY_EXIST
*********************************************************************/
int32 
lcsh_lcm_msg_rx_conport_regs(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    conport_t* con_msg;
    int ret = 0;

    if(NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess o req\n");
        return LCM_E_INVALID_PARAM;
    }

    con_msg = (conport_t* )(req->msg);
    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));

#ifdef _GLB_UML_SYSTEM_
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_CONFIG_PORT;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
#else

    switch(con_msg->opcode)
    {
        case CONFIG_SPEED:
            ret = phy_cfg_speed(con_msg->port_id - 1, con_msg->speed);
            break;
        case CONFIG_DUPLEX:
            ret = phy_cfg_duplex(con_msg->port_id - 1, con_msg->duplex);
            break;
        case CONFIG_ENABLE:
            ret = phy_cfg_enable(con_msg->port_id - 1, con_msg->enable);
            break;
        case CONFIG_LB:
            ret = phy_cfg_loopback(con_msg->port_id - 1, con_msg->loopback);
            break;
        case CONFIG_MEDIA:
            if(con_msg->media_type == GLB_PORT_TYPE_AUTO)
                ret = phy_cfg_medium(con_msg->port_id - 1, GLB_MEDIA_AMS_CU_1000BX);
            else if(con_msg->media_type == GLB_PORT_TYPE_COPPER)
                ret = phy_cfg_medium(con_msg->port_id - 1, GLB_MEDIA_RJ45);
            else
                ret = phy_cfg_medium(con_msg->port_id - 1, GLB_MEDIA_1000BX);
            break;

        default:
            break;
    }
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_CONFIG_PORT;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;

#endif  /* _GLB_UML_SYSTEM_ */     
}

/*********************************************************************
 * Name    : lcm_lcsh_msg_rx_test1
 * Purpose : this function is used to test rcv message form lcsh and return value
 * Input   : 
 * Output  : 
 * Return  : hsrv_e_***  - hal server return type
 * Note    : if the fid is already alloced, it will return HSRV_E_ENTRY_EXIST
*********************************************************************/
int32 
lcm_lcsh_msg_rx_show_transceiver(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
#ifdef _GLB_UML_SYSTEM_

    return LCM_E_SUCCESS;
#else
    int ret = 0;
    uint8 detail;
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    int32 port_id;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ];
    
    if (NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess or req\n");
        return LCM_E_INVALID_PARAM;
    }

    /*get local line card*/
    p_card= lcm_mgt_get_card();  
    if (NULL == p_card)
    {
        return -1;
    }

    detail = req->msg[0];
    sal_memset(&st_lcapi_lcm_lcsh_msg, 0 , sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(port_name, 0 , sizeof(port_name));

    CTCLIB_DUMP_PREPARE();
    
    for(port_id = 0; port_id< p_card->port_num; port_id++)
    {
        p_port = p_card->pp_port[port_id];
        if (p_port && p_port->p_fiber)
        {
            if (p_port->p_host_card)
            {
                /*Fix bug 14686. jqiu 2011-06-15*/
                if_build_port_shortname_by_slot_port(port_name, p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
            }
            else
            {
                sal_snprintf(port_name, 10, "port-%d", p_port->port_idx); 
            }
            fiber_dump_basic_info(port_name, p_port->p_fiber, g_fp_dumpfile);
            if(detail)
            {
                fiber_dump_diag_info(port_name, p_port->p_fiber, g_fp_dumpfile);
            }
        }
    }
    
    CTCLIB_DUMP_CLOSE_FILE();
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_DUMP_TRANSCEIVER_INFO;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
#endif
}

int32 
lcm_lcsh_msg_rx_config_bringup_log(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    int ret = 0;
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    bringuplog_t *bringuplog_msg;

    if(NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess o req\n");
        return LCM_E_INVALID_PARAM;
    }

    bringuplog_msg = (bringuplog_t* )(req->msg);
    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    lcm_bringup_log_set(bringuplog_msg->enable);

    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_CONFIG_BRINGUP_LOG;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
}

int32 
lcm_lcsh_msg_rx_config_lcm_debug_log(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    int ret = 0;
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;

    lcmdebuglog_t *lcmdebuglog_msg;

    if(NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess o req\n");
        return LCM_E_INVALID_PARAM;
    }

    lcmdebuglog_msg = (lcmdebuglog_t* )(req->msg);
    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    if (lcmdebuglog_msg->enable)
    {
        LCM_LOG_DEBUG_ENABLE(lcmdebuglog_msg->debug_type);
    }
    else
    {
        LCM_LOG_DEBUG_DISABLE(lcmdebuglog_msg->debug_type);
    }

    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_CONFIG_LCM_DEBUG_LOG;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
}

int32 
lcm_lcsh_msg_rx_config_drv_debug_log(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    int ret = 0;
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;

    lcmdebuglog_t *lcmdebuglog_msg;

    if(NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess o req\n");
        return LCM_E_INVALID_PARAM;
    }

    lcmdebuglog_msg = (lcmdebuglog_t* )(req->msg);
    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    if (lcmdebuglog_msg->enable)
    {
        DRV_LOG_DEBUG_ENABLE(lcmdebuglog_msg->debug_type);
    }
    else
    {
        DRV_LOG_DEBUG_DISABLE(lcmdebuglog_msg->debug_type);
    }

    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_CONFIG_DRV_DEBUG_LOG;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
}

int32 
lcm_lcsh_msg_rx_read_soc_reg(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    int ret = 0;
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    char cmd[128] = {0};

    read_soc_reg_t *read_soc_reg_msg;

    if(NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess o req\n");
        return LCM_E_INVALID_PARAM;
    }

    read_soc_reg_msg = (read_soc_reg_t* )(req->msg);

    snprintf(cmd, 128, "reg_ops -r" " %"PRIu64 " %"PRIu64 " %"PRIu64 " %"PRIu64, read_soc_reg_msg->baseaddr,
                                 read_soc_reg_msg->offset, read_soc_reg_msg->width, read_soc_reg_msg->count);
    system(cmd);

    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_READ_SOC_REG;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
}

int32 
lcm_lcsh_msg_rx_write_soc_reg(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    int ret = 0;
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    char cmd[128] = {0};

    write_soc_reg_t *write_soc_reg_msg;

    if(NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess o req\n");
        return LCM_E_INVALID_PARAM;
    }

    write_soc_reg_msg = (write_soc_reg_t* )(req->msg);

    snprintf(cmd, 128, "reg_ops -w" " %"PRIu64 " %"PRIu64 " %"PRIu64 " %"PRIu64, write_soc_reg_msg->baseaddr,
                                 write_soc_reg_msg->offset, write_soc_reg_msg->width, write_soc_reg_msg->value);
    system(cmd);

    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_WRITE_SOC_REG;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
}

/* for bug 43927 */
extern int32 lcm_notify_fiber_info(glb_port_t* p_port, lcm_fiber_notify_event_e notify_flag);
int32 
_lcm_msg_rx_change_wavelength(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    int ret = 0;
    fiber_conf_t *fiber_conf;
    glb_card_t* p_card = NULL; 
    glb_port_t* p_port = NULL;
    fiber_info_t *p_fiber = NULL;
    
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    
    if (NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess or req\n");
        return LCM_E_INVALID_PARAM;
    }

    p_card= lcm_mgt_get_card(); 
    if (NULL == p_card)
    {
        return -1;
    }
    fiber_conf = (fiber_conf_t* )(req->msg);
    p_port = p_card->pp_port[fiber_conf->port_id - 1];
    p_fiber = p_port->p_fiber;
    if (NULL == p_port || NULL == p_fiber)
    {
        return -1;
    }
    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
#ifdef _GLB_UML_SYSTEM_
        st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_CHANGE_WAVELENGTH;
        st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
        if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
        {
            return ret;
        }
        return ret;
#else

    switch(fiber_conf->opmode)
    {
        case CONFIG_WAVELENGTH:
            ret = fiber_config_wavelength(fiber_conf->port_id - 1, fiber_conf->value,WAVELENGTH_MODE,p_fiber);
            break;
        case CONFIG_CHANNEL:
            ret = fiber_config_wavelength(fiber_conf->port_id - 1, fiber_conf->value,CHANNEL_MODE,p_fiber);
            break;    
        default:
            break;
    }
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_CHANGE_WAVELENGTH;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        LCM_LOG_ERR("change wavelength fail %d ",ret);
        return ret;
    }
    return ret;
#endif
}

int32 
_lcm_msg_rx_config_prbs(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    conprbs_t* con_msg;
    int ret = 0;
 
    if(NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess o req\n");
        return LCM_E_INVALID_PARAM;
    }

    con_msg = (conprbs_t* )(req->msg);
    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));

#ifdef _GLB_UML_SYSTEM_
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_CONFIG_PRBS;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
#else
    uint8 result;
    switch(con_msg->opcode)
    {
        case CONFIG_PRBS_TX_ENABLE:
            ret = phy_cfg_prbs(con_msg->port_id - 1, 1,con_msg->polynome_type,&result);
            break;
        case CONFIG_PRBS_TX_DISABLE:
            ret = phy_cfg_prbs(con_msg->port_id - 1, 0,con_msg->polynome_type,&result);
            break;
        case CONFIG_PRBS_RX_CHECK:
            ret = phy_cfg_prbs(con_msg->port_id - 1, 2,con_msg->polynome_type,&result);
            break;
        default:
            break;
    }
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_CONFIG_PRBS;
    sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, &result, sizeof(uint8));
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE + sizeof(uint8);
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;

#endif  /* _GLB_UML_SYSTEM_ */     
}

int32 
_lcm_msg_rx_config_timer(lcm_sess_t *p_sess, lcapi_lcm_lcsh_msg_t* req)
{
    lcapi_lcm_lcsh_msg_t st_lcapi_lcm_lcsh_msg;
    contimer_t* con_msg;
    int ret = 0;
 
    if(NULL == p_sess || NULL == req)
    {
        LCM_LOG_ERR("Invalid p_sess o req\n");
        return LCM_E_INVALID_PARAM;
    }

    con_msg = (contimer_t* )(req->msg);
    sal_memset(&st_lcapi_lcm_lcsh_msg, 0, sizeof(lcapi_lcm_lcsh_msg_t));

#ifdef _GLB_UML_SYSTEM_
    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_CONFIG_TIMER;
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE;
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;
#else
    uint8 result;
    
    if(con_msg->opcode == 2)/* get */
    {
        lcm_get_timers_mask(con_msg->timer_type,&result);
    }
    else
    {
        lcm_set_timers_mask(con_msg->timer_type,con_msg->opcode);
    }

    st_lcapi_lcm_lcsh_msg.msg_type = LCAPI_LCM_LCSH_CONFIG_TIMER;
    sal_memcpy(st_lcapi_lcm_lcsh_msg.msg, &result, sizeof(uint8));
    st_lcapi_lcm_lcsh_msg.msg_len = LCAPI_LCM_LCSH_HEAD_SIZE + sizeof(uint8);
    if ((ret = lcm_sess_send(p_sess, &st_lcapi_lcm_lcsh_msg)) < 0)
    {
        return ret;
    }
    return ret;

#endif  /* _GLB_UML_SYSTEM_ */     
}

lcm_srv_cb_t lcm_lcsh_srv_cb_tbl[LCM_SRV_CB_TBL_SZ] = {
    [LCAPI_LCM_LCSH_TEST1]                      = lcm_lcsh_msg_rx_test1,
    [LCAPI_LCM_LCSH_READ_REGS]                  = lcsh_lcm_msg_rx_read_regs,    
    [LCAPI_LCM_LCSH_WRITE_REGS]                 = lcsh_lcm_msg_rx_write_regs,
    [LCAPI_LCM_LCSH_GET_PORT_INFO]              = lcsh_lcm_msg_rx_get_port_info,
    [LCAPI_LCM_LCSH_CONFIG_PORT]                = lcsh_lcm_msg_rx_conport_regs,
    [LCAPI_LCM_LCSH_DUMP_TRANSCEIVER_INFO]      = lcm_lcsh_msg_rx_show_transceiver,
    [LCAPI_LCM_LCSH_CONFIG_BRINGUP_LOG] = lcm_lcsh_msg_rx_config_bringup_log,
    [LCAPI_LCM_LCSH_CONFIG_LCM_DEBUG_LOG] = lcm_lcsh_msg_rx_config_lcm_debug_log,
    [LCAPI_LCM_LCSH_CONFIG_DRV_DEBUG_LOG] = lcm_lcsh_msg_rx_config_drv_debug_log,
    [LCAPI_LCM_LCSH_READ_SOC_REG] = lcm_lcsh_msg_rx_read_soc_reg,
    [LCAPI_LCM_LCSH_WRITE_SOC_REG] = lcm_lcsh_msg_rx_write_soc_reg,
    [LCAPI_LCM_LCSH_CHANGE_WAVELENGTH]          = _lcm_msg_rx_change_wavelength,
    [LCAPI_LCM_LCSH_CONFIG_PRBS]          = _lcm_msg_rx_config_prbs,
    [LCAPI_LCM_LCSH_CONFIG_TIMER]          = _lcm_msg_rx_config_timer,
};

