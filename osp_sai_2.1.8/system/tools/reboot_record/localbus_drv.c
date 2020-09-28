#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/statfs.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_tempfile_define.h"

#define CTC_NORMAL_EPLD_LOCAL_BUS_ADDR      0xffb00000
#define CTC_LS2F_EPLD_ADDR                  0x1ccc0000



int ctc_localbus_write(uint32 epld_addr, int32 value)
{
    int mem_fd;
    unsigned char *local_bus_base;

    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(mem_fd < 0) {
        printf("open /dev/mem failed");
        return -1;
    }
#ifdef _CTC_LOONGSON_2F_
    local_bus_base = (unsigned char*)mmap(NULL, 0x1000, PROT_READ|PROT_WRITE,
            MAP_SHARED, mem_fd, CTC_LS2F_EPLD_ADDR);/* LS IO for EPLD */
#else
    local_bus_base = (unsigned char*)mmap(NULL, 0x1000, PROT_READ|PROT_WRITE,
            MAP_SHARED, mem_fd, CTC_NORMAL_EPLD_LOCAL_BUS_ADDR);/*PPC IO for EPLD */
#endif
    if(local_bus_base < 0)
    {
        printf("Mmap epld error\n");
        close(mem_fd);
        return -1;
    }

    *((unsigned char *)(local_bus_base + epld_addr)) = value;
    
    munmap(local_bus_base, 0x1000);

    return 0;
}

