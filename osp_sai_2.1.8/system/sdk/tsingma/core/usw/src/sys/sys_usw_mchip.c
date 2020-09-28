/**
 @file sys_usw_mchip.c

 @date 2017-9-4

 @version v1.0

 The file define APIs of chip of sys layer
*/
/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "ctc_error.h"
#include "sys_usw_mchip.h"
#include "dal.h"

sys_usw_mchip_t *p_sys_mchip_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#ifdef DUET2
extern int32 sys_duet2_mchip_init(uint8 lchip);
extern int32 sys_duet2_mchip_deinit(uint8 lchip);
#endif


#ifdef TSINGMA
extern int32 sys_tsingma_mchip_init(uint8 lchip);
extern int32 sys_tsingma_mchip_deinit(uint8 lchip);
#endif


int32
sys_usw_mchip_init(uint8 lchip)
{

    /* 2. allocate interrupt master */
    if (p_sys_mchip_master[lchip])
    {
        return CTC_E_NONE;
    }

    p_sys_mchip_master[lchip] = (sys_usw_mchip_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_usw_mchip_t));
    if (NULL == p_sys_mchip_master[lchip])
    {
        return CTC_E_NO_MEMORY;
    }


#ifdef DUET2
    if (DRV_IS_DUET2(lchip))
    {
        sys_duet2_mchip_init(lchip);
    }
#endif


#ifdef TSINGMA
    if (DRV_IS_TSINGMA(lchip))
    {
        sys_tsingma_mchip_init(lchip);
    }
#endif


    MCHIP_API(lchip)->mchip_cap_init(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_mchip_deinit(uint8 lchip)
{
    uint32 dev_id = 0;
    
    if (!p_sys_mchip_master[lchip])
    {
        return CTC_E_NONE;
    }

    dal_get_chip_dev_id(lchip, &dev_id);
    switch (dev_id)
    {

    #ifdef DUET2
        case DAL_DUET2_DEVICE_ID:
            sys_duet2_mchip_deinit(lchip);
            break;
    #endif

    #ifdef TSINGMA
        case DAL_TSINGMA_DEVICE_ID:
            sys_tsingma_mchip_deinit(lchip);
            break;
    #endif

        default:
            return -1;
            break;
    }

    mem_free(p_sys_mchip_master[lchip]);

    return CTC_E_NONE;
}

