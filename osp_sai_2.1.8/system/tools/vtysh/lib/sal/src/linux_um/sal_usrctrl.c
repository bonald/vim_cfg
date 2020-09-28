/****************************************************************************
 * ctckal_usrctrl.c :        user control module source code
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History :
 * Revision       :         R0.01
 * Author         :         Bo Xiong
 * Date           :         2005-7-28
 * Reason         :         First Create
 ****************************************************************************/
#include <stropts.h>
#include "sal.h"

/* todo: block comments */
#define READ_BAY			1
#define WRITE_BAY			0

/*****************************************************************************
 * typedef
 *****************************************************************************/
/* bay */
struct cmdpara_bay_s
{
    uint32_t     chip_id;
    uint32_t	 fpga_id;
    uint32_t     reg_addr;
    uint32_t     value;
};
typedef struct cmdpara_bay_s cmdpara_bay_t;

/*****************************************************************************
 * macros
 *****************************************************************************/

/*****************************************************************************
 * global variables
 *****************************************************************************/
static int32_t ctc_chip_io_fd;

#ifndef BAY_ACCESS_BY_IOCTL
extern int pci_io_write_bay(uint32 bay_id, uint32 reg_offset, uint32 value);
extern int pci_io_read_bay(uint32 bay_id, uint32 reg_offset, uint32 p_value);
#endif


/*****************************************************************************
 * exported functions
 *****************************************************************************/
int32_t 
ctckal_usrctrl_init(void)
{
#if _GLB_UML_SYSTEM_
    (void)ctc_chip_io_fd;
    return 0;
#else
    ctc_chip_io_fd = open("/dev/asic_allctrl",O_RDWR);
    if(ctc_chip_io_fd < 0)
    {
        return -1;        
    }
    return ctc_chip_io_fd;
#endif     
}

