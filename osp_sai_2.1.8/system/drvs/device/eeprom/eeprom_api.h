/****************************************************************************
* $Id$
*  The header file of the eeprom api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-09-26
* Reason        : First Create.
****************************************************************************/
#ifndef __EEPROM_API_H__
#define __EEPROM_API_H__

#include "ctc_i2c.h"
#include "glb_eeprom_define.h"

#define EEPROM_OFFSET_WIDTH 2

/* define the structure including i2c operation paramaters */
struct eeprom_para_s
{
    uint16 len;              /* eeprom data length */
    uint16 offset;           /* the offset of the eeprom internal address */
    uint8* p_val;              /* pointer of the  buffur */
};
typedef struct eeprom_para_s eeprom_para_t;

struct eeprom_diag_result_s
{
   uint8 rd_fail;
   uint8 wr_fail;
   uint8 cmp_fail;
};
typedef struct eeprom_diag_result_s eeprom_diag_result_t;

/*modified by jcao for bug 14259, 2011-05-19*/
#define EEPROM_ITEM_CHK(EEPROM_INFO_PTR)                                                   \
do {                                                                        \
    if (((EEPROM_INFO_PTR->base_addr) == 0)&&((EEPROM_INFO_PTR->eeprom_idx) == 0))                                                     \
    {                                                                       \
        DRV_LOG_ERR("%s eeprom item not init", __FUNCTION__);                       \
        return -1;                                                          \
    }                                                                       \
}while(0)

int32 eeprom_write(uint32_t idx, eeprom_para_t* p_para);
int32 eeprom_read(uint32_t idx, eeprom_para_t* p_para);
int32 eeprom_close(uint32 idx);
int32 eeprom_init(i2c_gen_t* i2c_gen, uint32 num);
int32 eeprom_mgrif_crc_serial_param(int32 param, int32 value);
int32 eeprom_set_serial_no(uint8* p_serial_no, uint8 serial_len);
int32 eeprom_get_serial_no(uint8* p_serial_no, uint8* p_serial_len, eeprom_info_t *p_eeprom_info);
int32 eeprom_get_oem_info(uint8* p_oem_info, eeprom_info_t *p_eeprom_info);
int32 eeprom_set_oem_info(uint8* p_oem_info, eeprom_info_t *p_eeprom_info);
int32 eeprom_set_sw_ver(eeprom_info_t *p_eeprom_info, uint8* p_sw_ver, uint8 sw_ver_len);
int32 eeprom_get_system_mac(uint8* p_buf, eeprom_info_t *p_eeprom_info);
int32 eeprom_set_datapath(uint8 datapath, eeprom_info_t* p_eeprom_info);
int32 eeprom_get_datapath(uint8* datapath, eeprom_info_t* p_eeprom_info);
int32 eeprom_set_tcam_profile(uint32  mode, eeprom_info_t* p_eeprom_info);
int32 eeprom_get_tcam_profile(uint32* p_mode, eeprom_info_t* p_eeprom_info);
int32 eeprom_get_chassis_type(glb_chassis_type_t* chassis_type);
int32 eeprom_set_bootcmd(eeprom_info_t* p_eeprom_info,char* bootcmd);
int32 eeprom_get_bootcmd(eeprom_info_t* p_eeprom_info,char* bootcmd);
int32 eeprom_get_port_info(uint8* p_port_info, eeprom_info_t *p_eeprom_info);
int32 eeprom_set_port_info(uint8* p_port_info, eeprom_info_t *p_eeprom_info);
int32 eeprom_bootup_diag_and_report(eeprom_info_t *p_eeprom_info);

#endif /* __EEPROM_API_H__ */
