/****************************************************************************
* $Id$
*  The header file of the sysenv api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2019-01-16
* Reason        : First Create.
****************************************************************************/
#ifndef __SYSENV_API_H__
#define __SYSENV_API_H__

#include "glb_sysenv_define.h"
#include "sysenv_drv.h"

struct sysenv_para_s
{
    uint16 len;              /* sysenv data length */
    uint16 offset;           /* the offset of the sysenv internal address */
    uint8* p_val;              /* pointer of the  buffur */
};
typedef struct sysenv_para_s sysenv_para_t;

enum sysenv_error_e
{
    SYSENV_SUCCESS = 0,
    SYSENV_E_ERROR = -999,
    SYSENV_E_NO_MEMORY,
    SYSENV_E_NOT_INIT,
    SYSENV_E_INIT_FAILED,
    SYSENV_E_TIMEOUT,    
    SYSENV_E_READ,    
    SYSENV_E_WRITE,
    SYSENV_E_INVALID_PARAM,
    SYSENV_E_INVALID_PTR,
    SYSENV_E_INVALID_INDEX,    
    SYSENV_E_INVALID_LENGTH,
    SYSENV_E_INVALID_CHIP,
};

#define SYSENV_ITEM_CHK(SYSENV_INFO_PTR)                                                   \
do {                                                                        \
    if (((SYSENV_INFO_PTR->base_addr) == 0)&&((SYSENV_INFO_PTR->sysenv_idx) == 0))                                                     \
    {                                                                       \
        DRV_LOG_ERR("%s sysenv item not init", __FUNCTION__);                       \
        return -1;                                                          \
    }                                                                       \
}while(0)

int32 sysenv_write(uint32_t idx, sysenv_para_t* p_para);
int32 sysenv_read(uint32_t idx, sysenv_para_t* p_para);
int32 sysenv_set_tcam_profile(uint32  mode, sysenv_info_t* p_sysenv_info);
int32 sysenv_get_tcam_profile(uint32* p_mode, sysenv_info_t* p_sysenv_info);
int32 sysenv_get_oem_info(uint8* p_oem_info, sysenv_info_t *p_sysenv_info);
int32 sysenv_set_oem_info(uint8* p_oem_info, sysenv_info_t *p_sysenv_info);
int32 sysenv_get_port_info(uint8* p_port_info, sysenv_info_t *p_sysenv_info);
int32 sysenv_set_port_info(uint8* p_port_info, sysenv_info_t *p_sysenv_info);
int32 sysenv_set_bootcmd(sysenv_info_t* p_sysenv_info,char* bootcmd);
int32 sysenv_get_bootcmd(sysenv_info_t* p_sysenv_info,char* bootcmd);
int32 sysenv_set_sw_ver(sysenv_info_t *p_sysenv_info, uint8* p_sw_ver, uint8 sw_ver_len);
int32 sysenv_close(uint32 idx);
int32 sysenv_init(sysenv_chip_info_t *sysenv_chip_info, uint32 num);

#endif /* __SYSENV_API_H__ */
