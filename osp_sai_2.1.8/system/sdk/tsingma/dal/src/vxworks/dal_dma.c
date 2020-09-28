/****************************************************************************
 * dma_core.c    Core of asic dma implementation. This file must can be
 *               compiled in linux kernel mode, linux user mode and vxworks.
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Zhu Jian
 * Date:         2012-03-06.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "sal.h"
#include "dal_user.h"
#include "dal_mpool.h"

/****************************************************************************
 *
* Defines and Macros
*
****************************************************************************/
#define CTC_HW_DEV_NAME     "/dev/linux_dal"
#define CTC_HW_DEV_MAJOR    100

#define CTC_GET_DMA_INFO       _IO(CTC_HW_DEV_MAJOR, 10)

typedef struct cmdpara_dma_info_s
{
    unsigned int phy_base;
    unsigned int size;
} cmdpara_dma_info_t;


/****************************************************************************
 *
* Global and Declarations
*
****************************************************************************/
uint32* dma_virt_base = NULL;
uint32 dma_phy_base = 0;
uint32 dma_size = 0;

/*
static int32 mem_fd = -1;
*/

/****************************************************************************
 *
* Functions
*
****************************************************************************/

/**
 @brief This function is to convert logic address to physical address

 @param[in] laddr logical address

 @return physical address

*/
unsigned int
dma_l2p(void* laddr)
{
    unsigned int phy_base = (unsigned int)dma_phy_base;
    unsigned int virt_base = (unsigned int)(dma_virt_base);
    unsigned int addr = (unsigned int)(laddr);
    unsigned int p;

    p = phy_base + (addr - virt_base);

    return p;
}

/**
 @brief This function is to convert physical address to logic address

 @param[in] paddr physical address

 @return logical address

*/
unsigned int*
dma_p2l(unsigned int paddr)
{
    unsigned int phy_base = (unsigned int)dma_phy_base;
    unsigned int virt_base = (unsigned int)(dma_virt_base);
    unsigned int l;

    l = virt_base + (paddr - phy_base);

    return (unsigned int*)l;
}

/**
 @brief This function is to init dma memory and open device node

 @param[in] NULL

 @return CTC_E_XXX

*/
int
dma_init()
{
    int ret = 0;

#if 0
    cmdpara_dma_info_t dma_info;
    int devfd = dal_get_devfd();
    if (devfd < 0)
    {
        DAL_DEBUG_OUT("evfd is not init");
        return -1;
    }

    ioctl(devfd, CTC_GET_DMA_INFO, (long)&dma_info);
    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC | O_DSYNC | O_RSYNC)) < 0)
    {
        DAL_DEBUG_OUT("open /dev/mem: ");
        return -1;
    }

    sleep(2);

    dma_phy_base = dma_info.phy_base;
    dma_virt_base = (unsigned int*)mmap(NULL, dma_info.size, PROT_READ | PROT_WRITE, MAP_SHARED,
                                        mem_fd, dma_info.phy_base);
    dma_size = dma_info.size;
    DAL_DEBUG_OUT("dma zone base %p. phy base %x, size %x \n", dma_virt_base, dma_info.phy_base, dma_info.size);

    sal_memset(dma_virt_base, 0, dma_size);

    mpool_init((void*)dma_virt_base, dma_size);
     /*dma_cache_init();*/

    DAL_DEBUG_OUT("dma zone init pass!!!!\n");
#endif
    return ret;
}

