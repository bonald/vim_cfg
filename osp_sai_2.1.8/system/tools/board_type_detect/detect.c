#ifdef CNOS_OS
#include "sal.h"
#else 
#include "sal_common.h"
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glb_hw_define.h"
#include "glb_tempfile_define.h"
#include "detect.h"


int main(int argc, char *argv[])
{    
    int ret = 0;

#if (defined _CTC_X86_64_)
    ctc_get_emulation_board_info();
    return 0;
#endif
    i2c_open_dev0();
    i2c_open_dev1();
    ret = is_i2c_opened();
    if(ret < 0)
    {
        printf("There is no i2c device can be open! Please check it\n");
        return -1;
    }
    
    if(argc >1)
    {
        if(!sal_strncmp("onie", argv[1], 4))
        {
            ctc_get_platform_type();
            if(g_platform_type)
                printf("This board support ONIE !\n");
            else
                printf("This board unsupport ONIE !\n");
        }
        if(!sal_strncmp("dump", argv[1], 4))
        {
            ctc_detect_info_init(1);
        }
    }
    else
    {
        ret = ctc_get_board_info();
        if(ret < 0)
        {
            printf("Can't get board type!\n");
            return -1;
        }
    }
    return 0;
}


