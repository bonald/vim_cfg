/****************************************************************************
 * sysenv_spiflash.c   spiflash access interface
 *
 * Copyright:    (c)2005 Actus Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       shil
 * Date:         2019-01-16.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h> 
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <mtd/mtd-user.h>

#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_const.h"
#include "ctclib_debug.h"
#include "drv_debug.h"
#include "sysenv_api.h"
#include "sysenv_spiflash.h"


/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/


/****************************************************************************
 *
* Functions  
*
****************************************************************************/

int32 sysenv_spiflash_close(sysenv_handle_t *hdl)
{
    sysenv_spiflash_info_t *phdl_spiflash_info = hdl->sysenv_chip_info.sysenv_gen;
    close(phdl_spiflash_info->dev_fd);
    DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, phdl_spiflash_info);
    hdl->sysenv_chip_info.sysenv_gen = NULL;
    DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, hdl);
    
    return SYSENV_SUCCESS;
}

int32 sysenv_spiflash_erase_block(const sysenv_handle_t *hdl, uint32 start, uint32 block_num)
{
    sysenv_spiflash_info_t *phdl_spiflash_info = hdl->sysenv_chip_info.sysenv_gen;
    struct erase_info_user erase;
    int err = 0;

    erase.start = start & (~(phdl_spiflash_info->block_size - 1));
    erase.length = block_num * phdl_spiflash_info->block_size;

    if (phdl_spiflash_info->dev_fd < 0)
    {
        DRV_LOG_ERR("spiflash device not open!");
        return SYSENV_E_ERROR;
    }
    
    err = ioctl(phdl_spiflash_info->dev_fd, MEMERASE, &erase);
    if (err != 0)
    {
        DRV_LOG_ERR("erase spiflash device failed!");
        return SYSENV_E_ERROR;
    }

    return SYSENV_SUCCESS;
}

int32 sysenv_spiflash_write_block(const sysenv_handle_t *hdl, uint32 start, uint32 block_num, uint8 *buf)
{
    sysenv_spiflash_info_t *phdl_spiflash_info = hdl->sysenv_chip_info.sysenv_gen;
    uint32 offset = start & (~(phdl_spiflash_info->block_size - 1));
    int real_write_len = 0;
    
    if (phdl_spiflash_info->dev_fd < 0)
    {
        DRV_LOG_ERR("spiflash device not open!");
        return SYSENV_E_ERROR;
    }

    if (offset != lseek(phdl_spiflash_info->dev_fd, offset, SEEK_SET))
    {
        DRV_LOG_ERR("lseek failed!");
        return SYSENV_E_ERROR;
    }

    real_write_len = write(phdl_spiflash_info->dev_fd, buf, block_num * phdl_spiflash_info->block_size);
    if (real_write_len != block_num * phdl_spiflash_info->block_size)
    {
        DRV_LOG_ERR("write spiflash device failed!");
        return SYSENV_E_ERROR;
    }

    return SYSENV_SUCCESS;
}

int32 sysenv_spiflash_read_block(const sysenv_handle_t *hdl, uint32 start, uint32 block_num, uint8 *buf)
{
    sysenv_spiflash_info_t *phdl_spiflash_info = hdl->sysenv_chip_info.sysenv_gen;
    uint32 offset = start & (~(phdl_spiflash_info->block_size - 1));
    int real_read_len = 0;
    
    if (phdl_spiflash_info->dev_fd < 0)
    {
        DRV_LOG_ERR("spiflash device not open!");
        return SYSENV_E_ERROR;
    }

    if (offset != lseek(phdl_spiflash_info->dev_fd, offset, SEEK_SET))
    {
        DRV_LOG_ERR("lseek failed!");
        return SYSENV_E_ERROR;
    }

    real_read_len = read(phdl_spiflash_info->dev_fd, buf, block_num * phdl_spiflash_info->block_size);
    if (real_read_len != block_num * phdl_spiflash_info->block_size)
    {
        DRV_LOG_ERR("read spiflash device failed!");
        return SYSENV_E_ERROR;
    }

    return SYSENV_SUCCESS;
}

int32 sysenv_spiflash_read(const sysenv_handle_t *hdl, uint16 offset, uint16 len, uint8 *p_val)
{
    sysenv_spiflash_info_t *phdl_spiflash_info = hdl->sysenv_chip_info.sysenv_gen;
    
    uint8 *tmp_buf = NULL;
    uint32 pos = 0;

    tmp_buf = (uint8 *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SYSENV_INFO, phdl_spiflash_info->block_size * phdl_spiflash_info->block_num);
    if (tmp_buf == NULL)
    {
        DRV_LOG_ERR("malloc buff for read and write spiflash device failed!");
        return SYSENV_E_ERROR;
    }
    memset(tmp_buf, 0, phdl_spiflash_info->block_size * phdl_spiflash_info->block_num);

    if (sysenv_spiflash_read_block(hdl, phdl_spiflash_info->base_offset, phdl_spiflash_info->block_num, tmp_buf) != 0)
    {
        DRV_LOG_ERR("read spiflash device failed!");
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, tmp_buf);
        return SYSENV_E_ERROR;
    }

    for (pos = 0; pos < len; pos++)
        p_val[pos] = tmp_buf[pos+offset];

    DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, tmp_buf);
    return SYSENV_SUCCESS;
}

int32 sysenv_spiflash_write(const sysenv_handle_t *hdl, uint16 offset, uint16 len, uint8 *p_val)
{
    sysenv_spiflash_info_t *phdl_spiflash_info = hdl->sysenv_chip_info.sysenv_gen;
    
    uint8 *tmp_buf = NULL;
    uint32 pos = 0;

    tmp_buf = (uint8 *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SYSENV_INFO, phdl_spiflash_info->block_size * phdl_spiflash_info->block_num);
    if (tmp_buf == NULL)
    {
        DRV_LOG_ERR("malloc buff for read and write spiflash device failed!");
        return SYSENV_E_ERROR;
    }
    memset(tmp_buf, 0, phdl_spiflash_info->block_size * phdl_spiflash_info->block_num);

    if (sysenv_spiflash_read_block(hdl, phdl_spiflash_info->base_offset, phdl_spiflash_info->block_num, tmp_buf) != 0)
    {
        DRV_LOG_ERR("read spiflash device failed!");
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, tmp_buf);
        return SYSENV_E_ERROR;
    }
    
    for (pos = 0; pos < len; pos++)
        tmp_buf[pos+offset] = p_val[pos];

    if (sysenv_spiflash_erase_block(hdl, phdl_spiflash_info->base_offset, phdl_spiflash_info->block_num) != 0)
    {
        DRV_LOG_ERR("erase spiflash device failed!");
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, tmp_buf);
        return SYSENV_E_ERROR;
    }

    if (sysenv_spiflash_write_block(hdl, phdl_spiflash_info->base_offset, phdl_spiflash_info->block_num, tmp_buf) != 0)
    {
        DRV_LOG_ERR("write spiflash device failed!");
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, tmp_buf);
        return SYSENV_E_ERROR;
    }

    DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, tmp_buf);
    return SYSENV_SUCCESS;
}


sysenv_handle_t *sysenv_spiflash_register_driver(sysenv_chip_info_t *sysenv_chip_info)
{
    sysenv_spiflash_info_t *p_spiflash_info = (sysenv_spiflash_info_t *)sysenv_chip_info->sysenv_gen;
    sysenv_spiflash_info_t *phdl_spiflash_info = NULL;

    /* malloc space for spiflash hdl*/
    sysenv_handle_t *hdl = (sysenv_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SYSENV_INFO, sizeof(sysenv_handle_t));
    if(NULL == hdl)
    {
        DRV_LOG_ERR("sysenv alloc spiflash hdl space fail!\n");
        return NULL;
    }
    sal_memset(hdl, 0, sizeof(sysenv_handle_t));

    /* malloc space for spiflash hdl->sysenv_chip_info.sysenv_gen */
    hdl->sysenv_chip_info.sysenv_gen = DRV_MALLOC(CTCLIB_MEM_DRIVER_SYSENV_INFO, sizeof(sysenv_spiflash_info_t));
    if(NULL == hdl->sysenv_chip_info.sysenv_gen)
    {
        DRV_LOG_ERR("sysenv alloc spiflash sysenv_gen space fail!\n");
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, hdl);
        return NULL;
    }
    sal_memset(hdl->sysenv_chip_info.sysenv_gen, 0, sizeof(sysenv_spiflash_info_t));
    phdl_spiflash_info = hdl->sysenv_chip_info.sysenv_gen;

    /* init sysenv_chip_info.sysenv_gen */
    sal_strncpy(phdl_spiflash_info->dev_name, p_spiflash_info->dev_name, 64);
    phdl_spiflash_info->dev_fd = -1;
    phdl_spiflash_info->dev_fd = open(phdl_spiflash_info->dev_name, O_SYNC|O_RDWR);
    if (phdl_spiflash_info->dev_fd < 0)
    {
        DRV_LOG_ERR("sysenv open spiflash device failed!\n");
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, phdl_spiflash_info);
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, hdl);
        return NULL;
    }
    phdl_spiflash_info->base_offset = p_spiflash_info->base_offset;
    phdl_spiflash_info->block_size = p_spiflash_info->block_size;
    phdl_spiflash_info->block_num = p_spiflash_info->block_num;

    /* init sysenv_chip_info.sysenv_chip_type */
    hdl->sysenv_chip_info.sysenv_chip_type = sysenv_chip_info->sysenv_chip_type;

    /* init read write close func */
    hdl->close = sysenv_spiflash_close;
    hdl->read = sysenv_spiflash_read;
    hdl->write = sysenv_spiflash_write;

    return hdl;
}
