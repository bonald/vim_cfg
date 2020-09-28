/**
 @file drv_io.c

 @date 2010-02-26

 @version v5.1

 The file contains all driver I/O interface realization
*/
#include "drv_api.h"
#include "usw/include/drv_io.h"
#include "usw/include/drv_error.h"
#include "usw/include/drv_chip_ctrl.h"
#include "usw/include/drv_common.h"
#include "usw/include/drv_ser_db.h"
#include "usw/include/drv_ser.h"
#include "dal.h"

#define DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM                DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY0)
#define DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM                DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY1)
#define DRV_LPM_TCAM_KEY2_MAX_ENTRY_NUM                DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY2)
#define DRV_LPM_TCAM_KEY3_MAX_ENTRY_NUM                DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY3)
#define DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM                DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY4)
#define DRV_LPM_TCAM_KEY5_MAX_ENTRY_NUM                DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY5)
#define DRV_LPM_TCAM_KEY6_MAX_ENTRY_NUM                DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY6)
#define DRV_LPM_TCAM_KEY7_MAX_ENTRY_NUM                DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY7)
#define DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM                DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY8)
#define DRV_LPM_TCAM_KEY9_MAX_ENTRY_NUM                DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY9)
#define DRV_LPM_TCAM_KEY10_MAX_ENTRY_NUM               DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY10)
#define DRV_LPM_TCAM_KEY11_MAX_ENTRY_NUM               DRV_MEM_ENTRY_NUM(lchip, DRV_FTM_LPM_TCAM_KEY11)

#define DS_QUEUE_MAP_TCAM_KEY_BYTES                    ((DRV_IS_DUET2(lchip) ? 8 : sizeof(DsQueueMapTcamKey_m)))



drv_io_callback_fun_t g_chip_io_func =
{
    drv_sram_tbl_read:drv_usw_chip_sram_tbl_read,
    drv_sram_tbl_write:drv_usw_chip_sram_tbl_write,
    drv_tcam_tbl_read:drv_usw_chip_tcam_tbl_read,
    drv_tcam_tbl_write:drv_usw_chip_tcam_tbl_write,
    drv_tcam_tbl_remove:drv_usw_chip_tcam_tbl_remove
};

#define DRV_TIME_OUT  1000    /* Time out setting */

/* Cpu access Tcam control operation type */
enum flow_tcam_operation_type_t
{
    CPU_ACCESS_REQ_READ_X = 0,
    CPU_ACCESS_REQ_READ_Y = 1,
    CPU_ACCESS_REQ_WRITE_DATA_MASK = 2,
    CPU_ACCESS_REQ_INVALID_ENTRY = 3,
};
typedef enum flow_tcam_operation_type_t flow_tcam_op_tp_e;

enum fib_tcam_operation_type_t
{
    FIB_CPU_ACCESS_REQ_READ_X = 0,
    FIB_CPU_ACCESS_REQ_READ_Y = 1,
    FIB_CPU_ACCESS_REQ_WRITE_DATA_MASK = 2,
    FIB_CPU_ACCESS_REQ_INVALID_ENTRY = 3,
};
typedef enum fib_tcam_operation_type_t fib_tcam_op_tp_e;

/* Tcam memory type */
enum tcam_mem_type
{
    DRV_INT_TCAM_RECORD_DATA = 0,
    DRV_INT_TCAM_RECORD_MASK = 1,
    DRV_INT_TCAM_RECORD_REG = 2,
    DRV_INT_LPM_TCAM_RECORD_DATA = 3,
    DRV_INT_LPM_TCAM_RECORD_MASK = 4,
    DRV_INT_LPM_TCAM_RECORD_REG = 5,
};
typedef enum tcam_mem_type tcam_mem_type_e;


/* Tcam/Hash/Tbl/Reg I/O operation mutex control */
#define FLOW_TCAM_LOCK(lchip)         sal_mutex_lock(p_drv_master[lchip]->p_flow_tcam_mutex)
#define FLOW_TCAM_UNLOCK(lchip)       sal_mutex_unlock(p_drv_master[lchip]->p_flow_tcam_mutex)
#define LPM_IP_TCAM_LOCK(lchip)       sal_mutex_lock(p_drv_master[lchip]->p_lpm_ip_tcam_mutex)
#define LPM_IP_TCAM_UNLOCK(lchip)     sal_mutex_unlock(p_drv_master[lchip]->p_lpm_ip_tcam_mutex)
#define LPM_NAT_TCAM_LOCK(lchip)      sal_mutex_lock(p_drv_master[lchip]->p_lpm_nat_tcam_mutex)
#define LPM_NAT_TCAM_UNLOCK(lchip)    sal_mutex_unlock(p_drv_master[lchip]->p_lpm_nat_tcam_mutex)
#define DRV_ENTRY_LOCK(lchip)         sal_mutex_lock(p_drv_master[lchip]->p_entry_mutex)
#define DRV_ENTRY_UNLOCK(lchip)       sal_mutex_unlock(p_drv_master[lchip]->p_entry_mutex)

#if (SDK_WORK_PLATFORM == 0)
static uint32 parser_tbl_id_list[3][23] =
        {
            {
               ParserUdfCam_t                           ,
               ParserUdfCamResult_t                     ,
               ParserDebugStats_t                       ,
               ParserEthernetCtl_t                      ,
               ParserIpChecksumCtl_t                    ,
               ParserIpCtl_t                            ,
               ParserL3Ctl_t                            ,
               ParserLayer2ProtocolCam_t                ,
               ParserLayer2ProtocolCamValid_t           ,
               ParserLayer3FlexCtl_t                    ,
               ParserLayer3HashCtl_t                    ,
               ParserLayer3ProtocolCam_t                ,
               ParserLayer3ProtocolCamValid_t           ,
               ParserLayer3ProtocolCtl_t                ,
               ParserLayer4AchCtl_t                     ,
               ParserLayer4AppCtl_t                     ,
               ParserLayer4FlexCtl_t                    ,
               ParserMplsCtl_t                          ,
               ParserPacketTypeMap_t                    ,
               ParserPbbCtl_t                           ,
               ParserTrillCtl_t                         ,
               ParserReserved_t                         ,
               ParserRangeOpCtl_t
            },
            {
               ParserUdfCam1_t                           ,
               ParserUdfCamResult1_t                     ,
               ParserDebugStats1_t                       ,
               ParserEthernetCtl1_t                      ,
               ParserIpChecksumCtl_t                    ,
               ParserIpCtl1_t                            ,
               ParserL3Ctl1_t                            ,
               ParserLayer2ProtocolCam1_t                ,
               ParserLayer2ProtocolCamValid1_t           ,
               ParserLayer3FlexCtl1_t                    ,
               ParserLayer3HashCtl1_t                    ,
               ParserLayer3ProtocolCam1_t                ,
               ParserLayer3ProtocolCamValid1_t           ,
               ParserLayer3ProtocolCtl1_t                ,
               ParserLayer4AchCtl1_t                     ,
               ParserLayer4AppCtl1_t                     ,
               ParserLayer4FlexCtl1_t                    ,
               ParserMplsCtl1_t                          ,
               ParserPacketTypeMap1_t                    ,
               ParserPbbCtl1_t                           ,
               ParserTrillCtl1_t                         ,
               ParserReserved1_t                         ,
               ParserRangeOpCtl1_t
            },
            {
               ParserUdfCam2_t                           ,
               ParserUdfCamResult2_t                     ,
               ParserDebugStats2_t                       ,
               ParserEthernetCtl2_t                      ,
               ParserIpChecksumCtl2_t                    ,
               ParserIpCtl2_t                            ,
               ParserL3Ctl2_t                            ,
               ParserLayer2ProtocolCam2_t                ,
               ParserLayer2ProtocolCamValid2_t           ,
               ParserLayer3FlexCtl2_t                    ,
               ParserLayer3HashCtl2_t                    ,
               ParserLayer3ProtocolCam2_t                ,
               ParserLayer3ProtocolCamValid2_t           ,
               ParserLayer3ProtocolCtl2_t                ,
               ParserLayer4AchCtl2_t                     ,
               ParserLayer4AppCtl2_t                     ,
               ParserLayer4FlexCtl2_t                    ,
               ParserMplsCtl2_t                          ,
               ParserPacketTypeMap2_t                    ,
               ParserPbbCtl2_t                           ,
               ParserTrillCtl2_t                         ,
               ParserReserved2_t                         ,
               ParserRangeOpCtl2_t
            }
        };
#endif

uint8  g_usw_burst_en = FALSE; /* default use burst io */

extern dup_address_offset_type_t duplicate_addr_type;

/**
 @brief Init embeded and external tcam mutex
*/
int32
drv_usw_chip_tcam_mutex_init(uint8 lchip)
{
    int32 ret;

    ret = sal_mutex_create(&p_drv_master[lchip]->p_flow_tcam_mutex);
    if (ret || (!p_drv_master[lchip]->p_flow_tcam_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    ret = sal_mutex_create(&p_drv_master[lchip]->p_lpm_ip_tcam_mutex);
    if (ret || (!p_drv_master[lchip]->p_lpm_ip_tcam_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    ret = sal_mutex_create(&p_drv_master[lchip]->p_lpm_nat_tcam_mutex);
    if (ret || (!p_drv_master[lchip]->p_lpm_nat_tcam_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    ret = sal_mutex_create(&p_drv_master[lchip]->p_entry_mutex);
    if (ret || (!p_drv_master[lchip]->p_entry_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    ret = sal_mutex_create(&p_drv_master[lchip]->p_mep_mutex);
    if (ret || (!p_drv_master[lchip]->p_mep_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    return DRV_E_NONE;
}

int32
drv_usw_chip_pci_mutex_init(uint8 lchip)
{
    int32 ret;

#ifndef TX_PACKET_USE_SPINLOCK
    ret = sal_mutex_create(&p_drv_master[lchip]->p_pci_mutex);
#else
    ret = sal_spinlock_create((sal_spinlock_t**)&p_drv_master[lchip]->p_pci_mutex);
#endif
    if (ret || (!p_drv_master[lchip]->p_pci_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    return DRV_E_NONE;
}

int32
drv_usw_chip_i2c_mutex_init(uint8 lchip)
{
    int32 ret;

#ifndef TX_PACKET_USE_SPINLOCK
    ret = sal_mutex_create(&p_drv_master[lchip]->p_i2c_mutex);
#else
    ret = sal_spinlock_create((sal_spinlock_t**)&p_drv_master[lchip]->p_i2c_mutex);
#endif
    if (ret || (!p_drv_master[lchip]->p_i2c_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    return DRV_E_NONE;
}

int32
drv_usw_chip_hss_mutex_init(uint8 lchip)
{
    int32 ret;

    ret = sal_mutex_create(&p_drv_master[lchip]->p_hss_mutex);
    if (ret || (!p_drv_master[lchip]->p_hss_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    return DRV_E_NONE;
}

int32
drv_usw_chip_acc_mutex_init(uint8 lchip)
{
    int32 ret;

    ret = sal_mutex_create(&p_drv_master[lchip]->fib_acc_mutex);
    if (ret || (!p_drv_master[lchip]->fib_acc_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    ret = sal_mutex_create(&p_drv_master[lchip]->cpu_acc_mutex);
    if (ret || (!p_drv_master[lchip]->cpu_acc_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    ret = sal_mutex_create(&p_drv_master[lchip]->ipfix_acc_mutex);
    if (ret || (!p_drv_master[lchip]->ipfix_acc_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    ret = sal_mutex_create(&p_drv_master[lchip]->cid_acc_mutex);
    if (ret || (!p_drv_master[lchip]->cid_acc_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    ret = sal_mutex_create(&p_drv_master[lchip]->mpls_acc_mutex);
    if (ret || (!p_drv_master[lchip]->mpls_acc_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    ret = sal_mutex_create(&p_drv_master[lchip]->gemport_acc_mutex);
    if (ret || (!p_drv_master[lchip]->gemport_acc_mutex))
    {
        return DRV_E_FAIL_CREATE_MUTEX;
    }

    return DRV_E_NONE;
}

#if 0
uint32
drv_usw_get_tcam_ad_addr_offset(uint8 block_num)
{
    uint32 hw_base_addr = 0;

    switch(block_num)
    {
        case 0:
            hw_base_addr = DRV_TCAM_AD0_BASE_4W;
            break;
        case 1:
            hw_base_addr = DRV_TCAM_AD1_BASE_4W;
            break;
        case 2:
            hw_base_addr = DRV_TCAM_AD2_BASE_4W;
            break;
        case 3:
            hw_base_addr = DRV_TCAM_AD3_BASE_4W;
            break;
        case 4:
            hw_base_addr = DRV_TCAM_AD4_BASE_4W;
            break;
        case 5:
            hw_base_addr = DRV_TCAM_AD5_BASE_4W;
            break;
        default:
            hw_base_addr = DRV_TCAM_AD6_BASE_4W;
            break;
    }

    return hw_base_addr;

}

uint32
drv_usw_get_tcam_addr_offset(uint8 block_num, uint8 is_mask)
{
    uint32 hw_base_addr = 0;

    switch(block_num)
    {
        case 0:
            hw_base_addr = is_mask?DRV_TCAM_MASK0_BASE_4W : DRV_TCAM_KEY0_BASE_4W;
            break;
        case 1:
            hw_base_addr = is_mask?DRV_TCAM_MASK1_BASE_4W : DRV_TCAM_KEY1_BASE_4W;
            break;
        case 2:
            hw_base_addr = is_mask?DRV_TCAM_MASK2_BASE_4W : DRV_TCAM_KEY2_BASE_4W;
            break;
        case 3:
            hw_base_addr = is_mask?DRV_TCAM_MASK3_BASE_4W : DRV_TCAM_KEY3_BASE_4W;
            break;
        case 4:
            hw_base_addr = is_mask?DRV_TCAM_MASK4_BASE_4W : DRV_TCAM_KEY4_BASE_4W;
            break;
        case 5:
            hw_base_addr = is_mask?DRV_TCAM_MASK5_BASE_4W : DRV_TCAM_KEY5_BASE_4W;
            break;
        default:
            hw_base_addr = is_mask?DRV_TCAM_MASK6_BASE_4W : DRV_TCAM_KEY6_BASE_4W;
            break;
    }

    return hw_base_addr;

}
#endif

/**
 @brief Real sram direct write operation I/O
*/
int32
drv_usw_chip_write_sram_entry(uint8 lchip, uintptr addr,
                                uint32* data, int32 len)
{
    int32 i = 0;
    int32 tmp = 0;
    uint32 length = 0;
    uint8 burst_op = 0;
    uint8 io_times = 0;
    uint8 word_num = 0;

    DRV_PTR_VALID_CHECK(data);

    /* the length must be 4*n(n=1,2,3...) */
    if (tmp & 0x3)
    {
        return DRV_E_ILLEGAL_LENGTH;
    }

    /*for burst io*/
    burst_op = addr&0x3;
    if (burst_op >= 3)
    {
        return DRV_E_INVAILD_TYPE;
    }

    tmp = (len >> 2);
    word_num = (burst_op)?16:1;
    io_times = tmp/word_num + ((tmp%word_num)?1:0);

    DRV_ENTRY_LOCK(lchip);

    /*for burst write,no mask, max length is 16dword at one times*/
    for (i = 0; i < io_times; i++)
    {
        if (burst_op)
        {
            length = (tmp>=16)?16:tmp;
        }
        else
        {
            length = 1;
        }

        DRV_IF_ERROR_RETURN_WITH_UNLOCK(
        drv_usw_chip_write_ext(lchip, addr, data+i*word_num, length),
        p_drv_master[lchip]->p_entry_mutex);

        addr += 4*length;
        if ((tmp > 16) && burst_op)
        {
            tmp = tmp - 16;
        }
    }

    DRV_ENTRY_UNLOCK(lchip);

    return DRV_E_NONE;
}

/**
 @brief Real sram direct read operation I/O
*/
int32
drv_usw_chip_read_sram_entry(uint8 lchip, uintptr addr, uint32* data, int32 len)
{
    int32 i = 0;
    int32 tmp = 0;
    int32 length = 0;
    uint8 burst_op = 0;
    uint8 io_times = 0;
    uint8 word_num = 0;

    tmp = len;

    DRV_PTR_VALID_CHECK(data);

    /* the length must be 4*n(n=1,2,3...) */
    if (tmp & 0x3)
    {
        return DRV_E_ILLEGAL_LENGTH;
    }

    /*for burst io*/
    burst_op = addr&0x3;
    if (burst_op >= 3)
    {
        return DRV_E_INVAILD_TYPE;
    }

    tmp = (len >> 2);
    word_num = (burst_op)?16:1;
    io_times = tmp/word_num + ((tmp%word_num)?1:0);

    DRV_ENTRY_LOCK(lchip);

    /*for burst write,no mask, max length is 16dword at one times*/
    for (i = 0; i < io_times; i++)
    {
        if (burst_op)
        {
            length = (tmp>=16)?16:tmp;
        }
        else
        {
            length = 1;
        }

        DRV_IF_ERROR_RETURN_WITH_UNLOCK(
        drv_usw_chip_read_ext(lchip, addr, data+i*word_num, length),
        p_drv_master[lchip]->p_entry_mutex);

        addr += 4*length;
        if ((tmp > 16) && burst_op)
        {
            tmp = tmp - 16;
        }
    }

    DRV_ENTRY_UNLOCK(lchip);
    return DRV_E_NONE;
}

/**
 @brief convert embeded tcam content from X/Y format to data/mask format
*/
int32
drv_usw_chip_convert_tcam_dump_content(uint8 lchip, uint32 tcam_entry_width, uint32 *data, uint32 *mask, uint8* p_empty)
{
     /*#define TCAM_ENTRY_WIDTH 80*/
    uint32 bit_pos = 0;
    uint32 index = 0;
    uint32 bit_offset = 0;

    if ((0 == SDK_WORK_PLATFORM) && (1 == SDK_WORK_ENV))
    {
        return DRV_E_NONE;
    }

    /* for IBM embeded tcam, mask bit = 1 denote compare,
       self-define the return mask bit = 1 denote compare
       to unify the embeded and external tcam read interface's return value;

       X    Y    Read_Data    Read_Mask   Return_Data   Return_Mask
       0    0       X            0            0            0
       0    1       1            1            1            1
       1    0       0            1            0            1
       1    1       X            X            0            0
       X=1, Y=1: No Write but read.  */

    /* data[1] -- [64,80]; data[2] -- [32,63]; data[3] -- [0,31] */
    for (bit_pos = 0; bit_pos < tcam_entry_width; bit_pos++)
    {
        index = bit_pos / 32;
        bit_offset = bit_pos % 32;

        if ((!IS_BIT_SET(mask[index], bit_offset))
            && IS_BIT_SET(data[index], bit_offset))    /* X = 0; Y = 1 */
        {
            SET_BIT(mask[index], bit_offset);      /* Return_Data = 1; Return_Mask = 1 */
        }
        else if (IS_BIT_SET(mask[index], bit_offset)
            && (!IS_BIT_SET(data[index], bit_offset))) /* X = 1; Y = 0 */
        {
            CLEAR_BIT(data[index], bit_offset);    /* Return_Data = 0; Return_Mask = 1 */
            SET_BIT(mask[index], bit_offset);
        }
        else if ((!IS_BIT_SET(mask[index], bit_offset))
            && (!IS_BIT_SET(data[index], bit_offset))) /* X = 0; Y = 0 */
        {
            continue;                              /* Return_Data = X(not cared); Return_Mask = 0 */
            /* Here data = X = 0, self-defined */
        }
        else                                           /* X = 1; Y = 1 */
        {
            *p_empty |= 1;
            /* Here data = 0, mask = 0, self-defined */
            CLEAR_BIT(data[index], bit_offset);    /* Return_Data = 0; Return_Mask = 0 */
            CLEAR_BIT(mask[index], bit_offset);
             /*DRV_DBG_INFO("ERROR! Read un-set Tcam entry!\n");*/
        }
    }

    return DRV_E_NONE;
}


/**
 @brief Real embeded LPM tcam read operation I/O
*/
STATIC int32
_drv_usw_chip_read_lpm_tcam_ip_entry(uint8 lchip, uint32 blknum, uint32 index,
                             uint32* data, uint32* mask)
{
    uint32 lpm_tcam_tcam_mem[LPM_TCAM_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

     /*drv_get_lchip_base(&chip_base);*/

    LPM_IP_TCAM_LOCK(lchip_offset);

   /*
    RTL Flow tcam address:

    based on 80 bits Key & Mask

    [15:0]: entry address, maximum memory 512*368 = 1k*184 = 2k*92 = 4k*46

    [17:16]: memory ID, total number is 3

    Table name: LpmTcamTcamMem

    LpmTcamTcamMem allocate 16B, blknum need to right shift 4 bits, that's [13:12] as blknum, [11:0] as local index
    */

    if (DRV_IS_DUET2(lchip))
    {
        idx = (blknum << (12)) | index;
    }
    else
    {
        idx = (blknum << (13)) | index;
    }

    cmd = DRV_IOR(LpmTcamTcamMem_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, lpm_tcam_tcam_mem), p_drv_master[lchip]->p_lpm_ip_tcam_mutex);

    LPM_IP_TCAM_UNLOCK(lchip_offset);

    DRV_IOR_FIELD(lchip, LpmTcamTcamMem_t, LpmTcamTcamMem_tcamEntryValid_f,
                          &entry_valid, lpm_tcam_tcam_mem);

    if(!entry_valid)
    {
         /*DRV_DBG_INFO("\nTcam Memory is invalid!\n");*/
         /*return DRV_E_INVALID_TCAM_TYPE;*/
    }

    /*clear valid bit*/
    entry_valid = 0;
    DRV_IOW_FIELD(lchip, LpmTcamTcamMem_t, LpmTcamTcamMem_tcamEntryValid_f, &entry_valid, lpm_tcam_tcam_mem);

     /*
    Read Tcam MASK field is X, DATA field is Y
    */

    sal_memcpy((uint8*)data,(uint8*)lpm_tcam_tcam_mem,DRV_LPM_KEY_BYTES_PER_ENTRY);

    sal_memcpy((uint8*)mask, (uint8*)lpm_tcam_tcam_mem + DRV_LPM_KEY_BYTES_PER_ENTRY, DRV_LPM_KEY_BYTES_PER_ENTRY);

    return ret;

#ifdef NEVER
    uint32 lpm_tcam_ip_cpu_req_ctl[LPM_TCAM_IP_CPU_REQ_CTL_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint32 cpu_req = 0, cpu_req_type = 0, cpu_index = 0;
    uint32 cpu_read_data_valid = 0, time_out = 0;

    DRV_PTR_VALID_CHECK(data);

    cpu_index = index;
    cpu_req = TRUE;
    if (DRV_INT_LPM_TCAM_RECORD_MASK == type)
    {
        cpu_req_type = FIB_CPU_ACCESS_REQ_READ_Y;
    }
    else if (DRV_INT_LPM_TCAM_RECORD_DATA == type)
    {
        cpu_req_type = FIB_CPU_ACCESS_REQ_READ_X;
    }
    else
    {
        DRV_DBG_INFO("\nLPM Embeded Tcam Memory Type is useless!\n");
        return DRV_E_INVALID_TCAM_TYPE;
    }

    DRV_IOW_FIELD(lchip, LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuReq_f,
                          &cpu_req, lpm_tcam_ip_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuReqType_f,
                          &cpu_req_type, lpm_tcam_ip_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuIndex_f,
                          &cpu_index, lpm_tcam_ip_cpu_req_ctl);
    cmd = DRV_IOW(LpmTcamIpCpuReqCtl_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, lpm_tcam_ip_cpu_req_ctl);

    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    while (TRUE)
    {
        cmd = DRV_IOR(LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuReadDataValid_f);
        ret = DRV_IOCTL(lchip, 0, cmd, &cpu_read_data_valid);
        if (ret < DRV_E_NONE)
        {
            return ret;
        }

        if (cpu_read_data_valid)
        {
            break;
        }

        if ((time_out ++) > DRV_TIME_OUT)
        {
            return DRV_E_TIME_OUT;
        }

    }

    cmd = DRV_IOR(LpmTcamIpCpuRdData_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, data);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    cpu_read_data_valid = FALSE;

    cmd = DRV_IOW(LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuReadDataValid_f);
    ret = DRV_IOCTL(lchip, 0, cmd, &cpu_read_data_valid);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }
#endif
    return DRV_E_NONE;
}

/**
 @brief Real embeded tcam remove operation I/O
*/

int32
drv_usw_chip_remove_flow_tcam_entry(uint8 lchip, uint32 blknum, uint32 tcam_index)
{
    uint32 flow_tcam_tcam_mem[FLOW_TCAM_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

     /*drv_get_lchip_base(&chip_base);*/

    FLOW_TCAM_LOCK(lchip_offset);

    /*
    RTL Flow tcam address:

    based on 80 bits Key & Mask

    [15:0]: entry address, maximum memory 512*320 = 1k*160 = 2k*80

    [20:16]: memory ID, total number is 17

    Table name: FlowTcamTcamMem

    FlowTcamTcamMem allocate 32B, blknum need to right shift 5 bits, that's [15:11] as blknum, [10:0] as local index
    */

    idx = (blknum<< (11)) |tcam_index;

    cmd = DRV_IOW(FlowTcamTcamMem_t, DRV_ENTRY_FLAG);

    entry_valid = 0;
    DRV_IOW_FIELD(lchip, FlowTcamTcamMem_t, FlowTcamTcamMem_tcamEntryValid_f,
                          &entry_valid, flow_tcam_tcam_mem);

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, flow_tcam_tcam_mem), p_drv_master[lchip]->p_flow_tcam_mutex);

    FLOW_TCAM_UNLOCK(lchip_offset);

    return ret;

#ifdef NEVER
    uint32 flow_tcam_cpu_req_ctl[FLOW_TCAM_CPU_REQ_CTL_BYTES/4] = {0};
    int32 ret = DRV_E_NONE;
    uint32 cmd = 0, mask = 0;
    uint32 time_out = 0;
    tbl_entry_t tbl_entry;
    uint32 cpu_req = 0, cpu_req_type = 0, cpu_index = 0, block_num = 0;

    cpu_index = tcam_index;
    cpu_req_type = CPU_ACCESS_REQ_INVALID_ENTRY;
    cpu_req = FALSE;
    block_num = blknum;

    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuReq_f,
                          &cpu_req, flow_tcam_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuReqType_f,
                          &cpu_req_type, flow_tcam_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuIndex_f,
                          &cpu_index, flow_tcam_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuBlkNum_f,
                          &block_num, flow_tcam_cpu_req_ctl);


    if (g_tcam_dma)
    {
        if (tcam_dma_cb)
        {
            ret =  tcam_dma_cb(lchip, flow_tcam_cpu_req_ctl[0]);
            if (ret < DRV_E_NONE)
            {
                return ret;
            }
        }
    }
    else
    {
        cmd = DRV_IOW(FlowTcamCpuReqCtl_t, DRV_ENTRY_FLAG);
        ret = drv_ioctl(lchip, 0, cmd, flow_tcam_cpu_req_ctl);
        if (ret < DRV_E_NONE)
        {
            return ret;
        }
    }

    while (TRUE)
    {
        cpu_req = TRUE;
        cmd = DRV_IOR(FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuReq_f);
        ret = drv_ioctl(lchip, 0, cmd, &cpu_req);
        if (ret < DRV_E_NONE)
        {
            return ret;
        }

        if (!cpu_req)
        {
            break;
        }

        if ((time_out ++) > DRV_TIME_OUT)
        {
            return DRV_E_TIME_OUT;
        }
    }

    tbl_entry.data_entry = NULL;
    tbl_entry.mask_entry = &mask;
    drv_ecc_recover_store(lchip, DRV_ECC_MEM_TCAM_KEY0 + blknum,
                          tcam_index, &tbl_entry);
#endif
    return DRV_E_NONE;
}

/**
 @brief Real embeded tcam write operation I/O (data&mask write)
*/

int32
drv_usw_chip_write_flow_tcam_data_mask(uint8 lchip, uint32 blknum, uint32 index,
                                   uint32* data, uint32* mask)
{
    uint32 flow_tcam_tcam_mem[FLOW_TCAM_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

     /*drv_get_lchip_base(&chip_base);*/

    FLOW_TCAM_LOCK(lchip_offset);

    /*
    RTL Flow tcam address:

    based on 80 bits Key & Mask

    [15:0]: entry address, maximum memory 512*320 = 1k*160 = 2k*80

    [20:16]: memory ID, total number is 17

    Table name: FlowTcamTcamMem

    FlowTcamTcamMem allocate 32B, blknum need to right shift 5 bits, that's [15:11] as blknum, [10:0] as local index
    */

    idx = (blknum<< (11)) |index;

    cmd = DRV_IOW(FlowTcamTcamMem_t, DRV_ENTRY_FLAG);

    entry_valid = 1;
    DRV_IOW_FIELD(lchip, FlowTcamTcamMem_t, FlowTcamTcamMem_tcamEntryValid_f,
                          &entry_valid, flow_tcam_tcam_mem);

    sal_memcpy((uint8*)flow_tcam_tcam_mem,(uint8*)data, DRV_BYTES_PER_ENTRY);

    sal_memcpy((uint8*)flow_tcam_tcam_mem + DRV_BYTES_PER_ENTRY, (uint8*)mask, DRV_BYTES_PER_ENTRY);

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, flow_tcam_tcam_mem), p_drv_master[lchip]->p_flow_tcam_mutex);

    FLOW_TCAM_UNLOCK(lchip_offset);

    return ret;

#ifdef NEVER
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint32 flow_tcam_cpu_req_ctl[FLOW_TCAM_CPU_REQ_CTL_BYTES/4] = {0};
    uint32 flow_tcam_cpu_wr_data[FLOW_TCAM_CPU_WR_DATA_BYTES/4] = {0};
    uint32 cpu_req = 0, cpu_req_type = 0, cpu_index = 0, block_num = 0;
    uint32  time_out = 0;
    tbl_entry_t tbl_entry;

    DRV_PTR_VALID_CHECK(data);
    DRV_PTR_VALID_CHECK(mask);

    /* Attention that we MUST write the mask before writing corresponding data!! */
    /* because the real store in TCAM is X/Y , and the TCAM access write data and mask at the same time, */
    /* so we should write mask at first, then tcam data. */
    /* X = ~data & mask ; Y = data & mask */

    sal_memcpy((uint8*)flow_tcam_cpu_wr_data,(uint8*)data, DRV_BYTES_PER_ENTRY*2);

    sal_memcpy((uint8*)flow_tcam_cpu_wr_data + DRV_BYTES_PER_ENTRY * 2, (uint8*)mask, DRV_BYTES_PER_ENTRY*2);

    cmd = DRV_IOW(FlowTcamCpuWrData_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, flow_tcam_cpu_wr_data);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    cpu_index = index;
    cpu_req_type = CPU_ACCESS_REQ_WRITE_DATA_MASK;
    cpu_req = FALSE;
    block_num = blknum;

    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuReq_f,
                          &cpu_req, flow_tcam_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuReqType_f,
                          &cpu_req_type, flow_tcam_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuIndex_f,
                          &cpu_index, flow_tcam_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuBlkNum_f,
                          &block_num, flow_tcam_cpu_req_ctl);

    cmd = DRV_IOW(FlowTcamCpuReqCtl_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, flow_tcam_cpu_req_ctl);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    while (TRUE)
    {
        cpu_req = TRUE;
        cmd = DRV_IOR(FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuReq_f);
        ret = DRV_IOCTL(lchip, 0, cmd, &cpu_req);
        if (ret < DRV_E_NONE)
        {
            return ret;
        }

        if (!cpu_req)
        {
            break;
        }

        if ((time_out ++) > DRV_TIME_OUT)
        {
            return DRV_E_TIME_OUT;
        }
    }

    tbl_entry.data_entry = data;
    tbl_entry.mask_entry = mask;
     /*-drv_ecc_recover_store(lchip, DRV_ECC_MEM_TCAM_KEY0 + blknum, index, &tbl_entry);*/
#endif
    return DRV_E_NONE;
}

/**
 @brief Real embeded tcam read operation I/O
*/
STATIC int32
_drv_usw_chip_read_flow_tcam_entry(uint8 lchip, uint32 blknum, uint32 index,
                             uint32* data, uint32* mask)
{
    uint32 flow_tcam_tcam_mem[FLOW_TCAM_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

     /*drv_get_lchip_base(&chip_base);*/

    FLOW_TCAM_LOCK(lchip_offset);

    /*
    RTL Flow tcam address:

    based on 80 bits Key & Mask

    [15:0]: entry address, maximum memory 512*320 = 1k*160 = 2k*80

    [20:16]: memory ID, total number is 17

    Table name: FlowTcamTcamMem

    FlowTcamTcamMem allocate 32B, blknum need to right shift 5 bits, that's [15:11] as blknum, [10:0] as local index
    */

    idx = (blknum<< (11)) |index;

    cmd = DRV_IOR(FlowTcamTcamMem_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, flow_tcam_tcam_mem), p_drv_master[lchip]->p_flow_tcam_mutex);

    FLOW_TCAM_UNLOCK(lchip_offset);

    DRV_IOR_FIELD(lchip, FlowTcamTcamMem_t, FlowTcamTcamMem_tcamEntryValid_f,
                          &entry_valid, flow_tcam_tcam_mem);

    if(!entry_valid)
    {
         /*-DRV_DBG_INFO("\nFlow Tcam Memory is invalid!, idx:%d\n", idx);*/
         /*-return DRV_E_INVALID_TCAM_TYPE;*/
    }

    /*clear valid bit*/
    entry_valid = 0;
    DRV_IOW_FIELD(lchip, FlowTcamTcamMem_t, FlowTcamTcamMem_tcamEntryValid_f, &entry_valid, flow_tcam_tcam_mem);

    /*
    Read Tcam MASK field is X, DATA field is Y
    */

    sal_memcpy((uint8*)data,(uint8*)flow_tcam_tcam_mem,DRV_BYTES_PER_ENTRY);

    sal_memcpy((uint8*)mask, (uint8*)flow_tcam_tcam_mem + DRV_BYTES_PER_ENTRY, DRV_BYTES_PER_ENTRY);


    return ret;


#ifdef NEVER
    uint32 flow_tcam_cpu_req_ctl[FLOW_TCAM_CPU_REQ_CTL_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint32 cpu_req = 0, cpu_req_type = 0, cpu_index = 0, block_num = 0;
    uint32 cpu_read_data_valid = 0, time_out = 0;
    uint32 lchip_offset = lchip - drv_init_chip_info.drv_init_lchip_base;


    DRV_PTR_VALID_CHECK(data);

/*
    if (index >= DRV_INT_TCAM_KEY_MAX_ENTRY_NUM)
    {
        DRV_DBG_INFO("\nEmbeded Tcam's index range is 0~(8k-1), read intTcam entry fail!\n");
        return DRV_E_INVALID_TCAM_INDEX;
    }
*/
    if (DRV_INT_TCAM_RECORD_MASK == type)
    {
        cpu_req_type = CPU_ACCESS_REQ_READ_Y;
    }
    else if (DRV_INT_TCAM_RECORD_DATA == type)
    {
        cpu_req_type = CPU_ACCESS_REQ_READ_X;
    }
    else
    {
        DRV_DBG_INFO("\nEmbeded Tcam Memory Type is useless!\n");
        return DRV_E_INVALID_TCAM_TYPE;
    }
    cpu_req = TRUE;
    cpu_index = index;
    block_num = blknum;

    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuReq_f,
                          &cpu_req, flow_tcam_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuReqType_f,
                          &cpu_req_type, flow_tcam_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuIndex_f,
                          &cpu_index, flow_tcam_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuBlkNum_f,
                          &block_num, flow_tcam_cpu_req_ctl);

    cmd = DRV_IOW(FlowTcamCpuReqCtl_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, flow_tcam_cpu_req_ctl);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    while (TRUE)
    {
        cmd = DRV_IOR(FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuReadDataValid_f);
        ret = DRV_IOCTL(lchip, 0, cmd, &cpu_read_data_valid);
        if (ret < DRV_E_NONE)
        {
            return ret;
        }

        if (cpu_read_data_valid)
        {
            break;
        }

        if ((time_out ++) > DRV_TIME_OUT)
        {
            return DRV_E_TIME_OUT;
        }
    }

    cmd = DRV_IOR(FlowTcamCpuRdData_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, data);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    cpu_read_data_valid = FALSE;

    cmd = DRV_IOW(FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_tcamCpuReadDataValid_f);
    ret = DRV_IOCTL(lchip, 0, cmd, &cpu_read_data_valid);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }
#endif
    return DRV_E_NONE;
}


int32
drv_usw_chip_write_enque_tcam_data_mask(uint8 lchip, uint32 index,
                                   uint32* data, uint32* mask)
{
    uint32 qmgr_enq_tcam_mem[Q_MGR_ENQ_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

     /*drv_get_lchip_base(&chip_base);*/

    FLOW_TCAM_LOCK(lchip_offset);

    idx = index;

    cmd = DRV_IOW(QMgrEnqTcamMem_t, DRV_ENTRY_FLAG);

    sal_memcpy((uint8*)qmgr_enq_tcam_mem,(uint8*)data, DS_QUEUE_MAP_TCAM_KEY_BYTES);

    sal_memcpy((uint8*)qmgr_enq_tcam_mem + DS_QUEUE_MAP_TCAM_KEY_BYTES, (uint8*)mask, DS_QUEUE_MAP_TCAM_KEY_BYTES);

    entry_valid = 1;
    DRV_IOW_FIELD(lchip, QMgrEnqTcamMem_t, QMgrEnqTcamMem_tcamEntryValid_f,
                          &entry_valid, qmgr_enq_tcam_mem);

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, qmgr_enq_tcam_mem), p_drv_master[lchip]->p_flow_tcam_mutex);

    FLOW_TCAM_UNLOCK(lchip_offset);

    return ret;
}

int32
_drv_usw_chip_read_enque_tcam_entry(uint8 lchip, uint32 index,
                             uint32* data, uint32* mask)
{
    uint32 qmgr_enq_tcam_mem[Q_MGR_ENQ_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

     /*drv_get_lchip_base(&chip_base);*/

    FLOW_TCAM_LOCK(lchip_offset);

    idx = index;

    cmd = DRV_IOR(QMgrEnqTcamMem_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, qmgr_enq_tcam_mem), p_drv_master[lchip]->p_flow_tcam_mutex);

    FLOW_TCAM_UNLOCK(lchip_offset);

    DRV_IOR_FIELD(lchip, QMgrEnqTcamMem_t, QMgrEnqTcamMem_tcamEntryValid_f,
                          &entry_valid, qmgr_enq_tcam_mem);

    if(!entry_valid)
    {
         /*DRV_DBG_INFO("\nEnq Tcam Memory is invalid!\n");*/
         /*return DRV_E_INVALID_TCAM_TYPE;*/
    }

    /*clear valid bit*/
    entry_valid = 0;
    DRV_IOW_FIELD(lchip, QMgrEnqTcamMem_t, QMgrEnqTcamMem_tcamEntryValid_f, &entry_valid, qmgr_enq_tcam_mem);

     /*
    Read Tcam MASK field is X, DATA field is Y
    */

    sal_memcpy((uint8*)data, (uint8*)qmgr_enq_tcam_mem,DS_QUEUE_MAP_TCAM_KEY_BYTES);

    sal_memcpy((uint8*)mask,(uint8*)qmgr_enq_tcam_mem + DS_QUEUE_MAP_TCAM_KEY_BYTES, DS_QUEUE_MAP_TCAM_KEY_BYTES);

    return ret;
}


int32
drv_usw_chip_remove_enque_tcam_entry(uint8 lchip, uint32 tcam_index)
{
    uint32 flow_tcam_tcam_mem[FLOW_TCAM_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

     /*drv_get_lchip_base(&chip_base);*/

    FLOW_TCAM_LOCK(lchip_offset);

    idx = tcam_index;

    cmd = DRV_IOW(QMgrEnqTcamMem_t, DRV_ENTRY_FLAG);

    entry_valid = 0;
    DRV_IOW_FIELD(lchip, QMgrEnqTcamMem_t, QMgrEnqTcamMem_tcamEntryValid_f,
                          &entry_valid, flow_tcam_tcam_mem);

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, flow_tcam_tcam_mem), p_drv_master[lchip]->p_flow_tcam_mutex);

    FLOW_TCAM_UNLOCK(lchip_offset);

    return ret;
}

int32
drv_usw_chip_write_cid_tcam_data_mask(uint8 lchip, uint32 index,
                                   uint32* data, uint32* mask)
{
    uint32 ipe_cid_tcam_mem[IPE_CID_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

     /*drv_get_lchip_base(&chip_base);*/

    FLOW_TCAM_LOCK(lchip_offset);

    idx = index;

    cmd = DRV_IOW(IpeCidTcamMem_t, DRV_ENTRY_FLAG);

    sal_memcpy((uint8*)ipe_cid_tcam_mem,(uint8*)data, DS_CATEGORY_ID_PAIR_TCAM_LOOKUP_KEY_BYTES);

    sal_memcpy((uint8*)ipe_cid_tcam_mem + DS_CATEGORY_ID_PAIR_TCAM_LOOKUP_KEY_BYTES, (uint8*)mask, DS_CATEGORY_ID_PAIR_TCAM_LOOKUP_KEY_BYTES);

    entry_valid = 1;
    DRV_IOW_FIELD(lchip, IpeCidTcamMem_t, IpeCidTcamMem_tcamEntryValid_f,
                          &entry_valid, ipe_cid_tcam_mem);

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, ipe_cid_tcam_mem), p_drv_master[lchip]->p_flow_tcam_mutex);

    FLOW_TCAM_UNLOCK(lchip_offset);

    return ret;
}

STATIC int32
_drv_usw_chip_read_cid_tcam_entry(uint8 lchip, uint32 index,
                             uint32* data, uint32* mask)
{
    uint32 ipe_cid_tcam_mem[IPE_CID_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

     /*drv_get_lchip_base(&chip_base);*/

    FLOW_TCAM_LOCK(lchip_offset);

    idx = index;

    cmd = DRV_IOR(IpeCidTcamMem_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, ipe_cid_tcam_mem), p_drv_master[lchip]->p_flow_tcam_mutex);

    FLOW_TCAM_UNLOCK(lchip_offset);

    DRV_IOR_FIELD(lchip, IpeCidTcamMem_t, IpeCidTcamMem_tcamEntryValid_f,
                          &entry_valid, ipe_cid_tcam_mem);

    if(!entry_valid)
    {
         /*DRV_DBG_INFO("\nCid Tcam Memory is invalid!\n");*/
         /*return DRV_E_INVALID_TCAM_TYPE;*/
    }

    /*clear valid bit*/
    entry_valid = 0;
    DRV_IOW_FIELD(lchip, IpeCidTcamMem_t, IpeCidTcamMem_tcamEntryValid_f, &entry_valid, ipe_cid_tcam_mem);

     /*
    Read Tcam MASK field is X, DATA field is Y
    */

    sal_memcpy((uint8*)data, (uint8*)ipe_cid_tcam_mem,DS_CATEGORY_ID_PAIR_TCAM_LOOKUP_KEY_BYTES);

    sal_memcpy((uint8*)mask,(uint8*)ipe_cid_tcam_mem + DS_CATEGORY_ID_PAIR_TCAM_LOOKUP_KEY_BYTES, DS_CATEGORY_ID_PAIR_TCAM_LOOKUP_KEY_BYTES);


    return ret;
}


int32
drv_usw_chip_remove_cid_tcam_entry(uint8 lchip, uint32 tcam_index)
{
    uint32 ipe_cid_tcam_mem[IPE_CID_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

     /*drv_get_lchip_base(&chip_base);*/

    FLOW_TCAM_LOCK(lchip_offset);

    idx = tcam_index;

    cmd = DRV_IOW(IpeCidTcamMem_t, DRV_ENTRY_FLAG);

    entry_valid = 0;
    DRV_IOW_FIELD(lchip, IpeCidTcamMem_t, IpeCidTcamMem_tcamEntryValid_f,
                          &entry_valid, ipe_cid_tcam_mem);

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, ipe_cid_tcam_mem), p_drv_master[lchip]->p_flow_tcam_mutex);

    FLOW_TCAM_UNLOCK(lchip_offset);

    return ret;
}


#ifdef NEVER
/**
 @brief Real embeded tcam ad read operation I/O
*/
STATIC int32
_drv_usw__chip_read_flow_tcam_ad_entry(uint8 lchip, uint32 blknum, uint32 index, uint32* data)
{
     /*uint32 flow_tcam_ad_mem[FLOW_TCAM_AD_MEM_BYTES/4] = {0};*/
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;

    DRV_PTR_VALID_CHECK(data);

     /*drv_get_lchip_base(&chip_base);*/

    FLOW_TCAM_LOCK(lchip_offset);

    idx = (blknum<< 15) |index;

    cmd = DRV_IOR(FlowTcamAdMem_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, data), p_drv_master[lchip]->p_flow_tcam_mutex);

    FLOW_TCAM_UNLOCK(lchip_offset);

    return ret;
#ifdef NEVER
    uint32 flow_tcam_cpu_req_ctl[FLOW_TCAM_CPU_REQ_CTL_BYTES/4] = {0};
     /*uint32 flow_tcam_ad_mem[FLOW_TCAM_AD_MEM_BYTES/4] = {0};*/
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;

    DRV_PTR_VALID_CHECK(data);

    drv_get_lchip_base(&chip_base)

    FLOW_TCAM_LOCK(lchip_offset);

    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_adCpublkNum_f,
                          &blknum, flow_tcam_cpu_req_ctl);

    cmd = DRV_IOW(FlowTcamCpuReqCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, 0, cmd, flow_tcam_cpu_req_ctl),
    p_flow_tcam_mutex[lchip_offset]);

    cmd = DRV_IOR(FlowTcamAdMem_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, index, cmd, data),
    p_flow_tcam_mutex[lchip_offset]);

    FLOW_TCAM_UNLOCK(lchip_offset);

    return ret;
#endif
}
#endif

/**
 @brief Real embeded LPM tcam remove operation I/O
*/

int32
drv_usw_chip_remove_lpm_tcam_ip_entry(uint8 lchip, uint32 blknum, uint32 tcam_index)
{
    uint32 lpm_tcam_tcam_mem[LPM_TCAM_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

     /*drv_get_lchip_base(&chip_base);*/

    LPM_IP_TCAM_LOCK(lchip_offset);

   /*
    RTL Flow tcam address:

    based on 80 bits Key & Mask

    [15:0]: entry address, maximum memory 512*368 = 1k*184 = 2k*92 = 4k*46

    [17:16]: memory ID, total number is 3

    Table name: LpmTcamTcamMem

    LpmTcamTcamMem allocate 16B, blknum need to right shift 4 bits, that's [13:12] as blknum, [11:0] as local index
    */

    if (DRV_IS_DUET2(lchip))
    {
        idx = (blknum << (12)) | tcam_index;
    }
    else
    {
        idx = (blknum << (13)) | tcam_index;
    }


    cmd = DRV_IOW(LpmTcamTcamMem_t, DRV_ENTRY_FLAG);


    entry_valid = 0;
    DRV_IOW_FIELD(lchip, LpmTcamTcamMem_t, LpmTcamTcamMem_tcamEntryValid_f,
                          &entry_valid, lpm_tcam_tcam_mem);

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, lpm_tcam_tcam_mem), p_drv_master[lchip]->p_lpm_ip_tcam_mutex);

    LPM_IP_TCAM_UNLOCK(lchip_offset);

    return ret;

#ifdef NEVER
    uint32 lpm_tcam_ip_cpu_req_ctl[LPM_TCAM_IP_CPU_REQ_CTL_BYTES/4] = {0};
    int32 ret = DRV_E_NONE;
    uint32 cmd = 0;
    uint32 time_out = 0;
    uint32 mask = 0;
    tbl_entry_t tbl_entry;
    uint32 cpu_req = 0, cpu_req_type = 0, cpu_index = 0;

    cpu_index = tcam_index;
    cpu_req_type = FIB_CPU_ACCESS_REQ_INVALID_ENTRY;
    cpu_req = TRUE;

    DRV_IOW_FIELD(lchip, LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuReq_f,
                          &cpu_req, lpm_tcam_ip_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuReqType_f,
                          &cpu_req_type, lpm_tcam_ip_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuIndex_f,
                          &cpu_index, lpm_tcam_ip_cpu_req_ctl);

    cmd = DRV_IOW(LpmTcamIpCpuReqCtl_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, lpm_tcam_ip_cpu_req_ctl);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    while (TRUE)
    {
        cpu_req = TRUE;
        cmd = DRV_IOR(LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuReq_f);
        ret = DRV_IOCTL(lchip, 0, cmd, &cpu_req);
        if (ret < DRV_E_NONE)
        {
            return ret;
        }

        if (!cpu_req)
        {
            break;
        }

        if ((time_out ++) > DRV_TIME_OUT)
        {
            return DRV_E_TIME_OUT;
        }
    }

    tbl_entry.data_entry = NULL;
    tbl_entry.mask_entry = &mask;
    drv_ecc_recover_store(lchip, DRV_ECC_MEM_TCAM_KEY7, tcam_index, &tbl_entry);
#endif
    return DRV_E_NONE;
}

#ifdef NEVER
/**
 @brief Real embeded LPM tcam remove operation I/O
*/
STATIC int32
_drv_usw__chip_remove_lpm_tcam_nat_entry(uint8 lchip, uint32 tcam_index)
{

    uint32 lpm_tcam_nat_cpu_req_ctl[LPM_TCAM_NAT_CPU_REQ_CTL_BYTES/4] = {0};
    int32 ret = DRV_E_NONE;
    uint32 mask = 0;
    uint32 cmd = 0;
    uint32 time_out = 0;
    tbl_entry_t tbl_entry;
    uint32 cpu_req = 0, cpu_req_type = 0, cpu_index = 0;

    cpu_index = tcam_index;
    cpu_req_type = FIB_CPU_ACCESS_REQ_INVALID_ENTRY;
    cpu_req = TRUE;

    DRV_IOW_FIELD(lchip, LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuReq_f,
                          &cpu_req, lpm_tcam_nat_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuReqType_f,
                          &cpu_req_type, lpm_tcam_nat_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuIndex_f,
                          &cpu_index, lpm_tcam_nat_cpu_req_ctl);

    cmd = DRV_IOW(LpmTcamNatCpuReqCtl2_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, lpm_tcam_nat_cpu_req_ctl);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    while (TRUE)
    {
        cpu_req = TRUE;
        cmd = DRV_IOR(LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuReq_f);
        ret = DRV_IOCTL(lchip, 0, cmd, &cpu_req);
        if (ret < DRV_E_NONE)
        {
            return ret;
        }

        if (!cpu_req)
        {
            break;
        }

        if ((time_out ++) > DRV_TIME_OUT)
        {
            return DRV_E_TIME_OUT;
        }
    }

    tbl_entry.data_entry = NULL;
    tbl_entry.mask_entry = &mask;
     /*-drv_ecc_recover_store(lchip, DRV_ECC_MEM_TCAM_KEY8, tcam_index, &tbl_entry);*/

    return DRV_E_NONE;
}
#endif
#ifdef NEVER
/**
 @brief Real embeded LPM tcam read operation I/O
*/
STATIC int32
_drv_usw__chip_read_lpm_tcam_nat_entry(uint8 lchip, uint32 index,
                             uint32* data, tcam_mem_type_e type)
{

    uint32 lpm_tcam_nat_cpu_req_ctl[LPM_TCAM_NAT_CPU_REQ_CTL_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint32 cpu_req = 0, cpu_req_type = 0, cpu_index = 0;
    uint32 cpu_read_data_valid = 0, time_out = 0;
    uint32 lchip_offset = lchip - drv_init_chip_info.drv_init_lchip_base;


    DRV_PTR_VALID_CHECK(data);

    cpu_index = index;
    cpu_req = TRUE;
    if (DRV_INT_LPM_TCAM_RECORD_MASK == type)
    {
        cpu_req_type = FIB_CPU_ACCESS_REQ_READ_Y;
    }
    else if (DRV_INT_LPM_TCAM_RECORD_DATA == type)
    {
        cpu_req_type = FIB_CPU_ACCESS_REQ_READ_X;
    }
    else
    {
        DRV_DBG_INFO("\nLPM Embeded Tcam Memory Type is useless!\n");
        return DRV_E_INVALID_TCAM_TYPE;
    }

    DRV_IOW_FIELD(lchip, LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuReq_f,
                          &cpu_req, lpm_tcam_nat_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuReqType_f,
                          &cpu_req_type, lpm_tcam_nat_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuIndex_f,
                          &cpu_index, lpm_tcam_nat_cpu_req_ctl);

    cmd = DRV_IOW(LpmTcamNatCpuReqCtl2_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, lpm_tcam_nat_cpu_req_ctl);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    while (TRUE)
    {
        cmd = DRV_IOR(LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuReadDataValid_f);
        ret = DRV_IOCTL(lchip, 0, cmd, &cpu_read_data_valid);
        if (ret < DRV_E_NONE)
        {
            return ret;
        }

        if (cpu_read_data_valid)
        {
            break;
        }

        if ((time_out ++) > DRV_TIME_OUT)
        {
            return DRV_E_TIME_OUT;
        }
    }

    cmd = DRV_IOR(LpmTcamNatCpuRdData2_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, data);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    cpu_read_data_valid = FALSE;

    cmd = DRV_IOW(LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuReadDataValid_f);
    ret = DRV_IOCTL(lchip, 0, cmd, &cpu_read_data_valid);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }


    return DRV_E_NONE;
}
#endif
/**
 @brief Real embeded LPM tcam write operation I/O (data&mask write)
*/

int32
drv_usw_chip_write_lpm_tcam_ip_data_mask(uint8 lchip, uint32 blknum, uint32 index,
                                   uint32* data, uint32* mask)
{
    uint32 lpm_tcam_tcam_mem[LPM_TCAM_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint8 chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

     /*drv_get_lchip_base(&chip_base);*/

    LPM_IP_TCAM_LOCK(lchip_offset);

   /*
    RTL Flow tcam address:

    based on 80 bits Key & Mask

    [15:0]: entry address, maximum memory 512*368 = 1k*184 = 2k*92 = 4k*46

    [17:16]: memory ID, total number is 3

    Table name: LpmTcamTcamMem

    LpmTcamTcamMem allocate 16B, blknum need to right shift 4 bits, that's [13:12] as blknum, [11:0] as local index
    */

    if (DRV_IS_DUET2(lchip))
    {
        idx = (blknum << (12)) | index;
    }
    else
    {
        idx = (blknum << (13)) | index;
    }

    cmd = DRV_IOW(LpmTcamTcamMem_t, DRV_ENTRY_FLAG);

    sal_memcpy((uint8*)lpm_tcam_tcam_mem,(uint8*)data, DRV_LPM_KEY_BYTES_PER_ENTRY);

    sal_memcpy((uint8*)lpm_tcam_tcam_mem + DRV_LPM_KEY_BYTES_PER_ENTRY, (uint8*)mask, DRV_LPM_KEY_BYTES_PER_ENTRY);

    entry_valid = 1;
    DRV_IOW_FIELD(lchip, LpmTcamTcamMem_t, LpmTcamTcamMem_tcamEntryValid_f,
                          &entry_valid, lpm_tcam_tcam_mem);

    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, lpm_tcam_tcam_mem), p_drv_master[lchip]->p_lpm_ip_tcam_mutex);

    LPM_IP_TCAM_UNLOCK(lchip_offset);

    return ret;

#ifdef NEVER
    uint32 lpm_tcam_ip_cpu_req_ctl[LPM_TCAM_IP_CPU_REQ_CTL_BYTES/4] = {0};
    uint32 lpm_tcam_ip_cpu_wr_data[LPM_TCAM_IP_CPU_WR_DATA_BYTES/4] = {0};
    uint32 cpu_req = 0, cpu_req_type = 0, cpu_index = 0;
    uint32 cmd = 0;
    uint32 time_out = 0;
    int32 ret = DRV_E_NONE;
    uint32 lchip_offset = lchip - drv_init_chip_info.drv_init_lchip_base;
    tbl_entry_t tbl_entry;

    DRV_PTR_VALID_CHECK(data);
    DRV_PTR_VALID_CHECK(mask);

    /* Attention that we MUST write the mask before writing corresponding data!! */
    /* because the real store in TCAM is X/Y , and the TCAM access write data and mask at the same time, */
    /* so we should write mask at first, then tcam data. */
    /* X = ~data & mask ; Y = data & mask */
    sal_memcpy((uint8*)lpm_tcam_ip_cpu_wr_data,(uint8*)data, DRV_LPM_KEY_BYTES_PER_ENTRY);

    sal_memcpy((uint8*)lpm_tcam_ip_cpu_wr_data + DRV_LPM_KEY_BYTES_PER_ENTRY, (uint8*)mask, DRV_LPM_KEY_BYTES_PER_ENTRY);

    cmd = DRV_IOW(LpmTcamIpCpuWrData_t, DRV_ENTRY_FLAG);
    ret = drv_ioctl(lchip, 0, cmd, lpm_tcam_ip_cpu_wr_data);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    cpu_index = index;
    cpu_req_type = FIB_CPU_ACCESS_REQ_WRITE_DATA_MASK;
    cpu_req = TRUE;

    DRV_IOW_FIELD(lchip, LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuReq_f,
                          &cpu_req, lpm_tcam_ip_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuReqType_f,
                          &cpu_req_type, lpm_tcam_ip_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuIndex_f,
                          &cpu_index, lpm_tcam_ip_cpu_req_ctl);

    cmd = DRV_IOW(LpmTcamIpCpuReqCtl_t, DRV_ENTRY_FLAG);
    ret = drv_ioctl(lchip, 0, cmd, lpm_tcam_ip_cpu_req_ctl);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    while (TRUE)
    {
        cpu_req = TRUE;
        cmd = DRV_IOR(LpmTcamIpCpuReqCtl_t, LpmTcamIpCpuReqCtl_tcamIpCpuReq_f);
        ret = drv_ioctl(lchip, 0, cmd, &cpu_req);
        if (ret < DRV_E_NONE)
        {
            return ret;
        }

        if (!cpu_req)
        {
            break;
        }

        if ((time_out ++) > DRV_TIME_OUT)
        {
            return DRV_E_TIME_OUT;
        }
    }

    tbl_entry.data_entry = data;
    tbl_entry.mask_entry = mask;
    drv_ecc_recover_store(lchip, DRV_ECC_MEM_TCAM_KEY7, index, &tbl_entry);
#endif
    return DRV_E_NONE;
}
#ifdef NEVER
/**
 @brief Real embeded LPM tcam write operation I/O (data&mask write)
*/
STATIC int32
_drv_usw__chip_write_lpm_tcam_nat_data_mask(uint8 lchip, uint32 index,
                                   uint32* data, uint32* mask)
{

    uint32 lpm_tcam_nat_cpu_req_ctl[LPM_TCAM_NAT_CPU_REQ_CTL_BYTES/4] = {0};
    uint32 lpm_tcam_nat_cpu_wr_data[LPM_TCAM_NAT_CPU_WR_DATA_BYTES/4] = {0};
    uint32 cpu_req = 0, cpu_req_type = 0, cpu_index = 0;
    uint32 cmd = 0;
    uint32 time_out = 0;
    int32 ret = DRV_E_NONE;
    uint32 lchip_offset = lchip - drv_init_chip_info.drv_init_lchip_base;

    DRV_PTR_VALID_CHECK(data);
    DRV_PTR_VALID_CHECK(mask);

    /* Attention that we MUST write the mask before writing corresponding data!! */
    /* because the real store in TCAM is X/Y , and the TCAM access write data and mask at the same time, */
    /* so we should write mask at first, then tcam data. */
    /* X = ~data & mask ; Y = data & mask */
    sal_memcpy((uint8*)lpm_tcam_nat_cpu_wr_data,(uint8*)data, DRV_BYTES_PER_ENTRY*2);

    sal_memcpy((uint8*)lpm_tcam_nat_cpu_wr_data + DRV_BYTES_PER_ENTRY*2, (uint8*)mask, DRV_BYTES_PER_ENTRY*2);

    cmd = DRV_IOW(LpmTcamNatCpuWrData2_t, DRV_ENTRY_FLAG);
    ret = drv_ioctl(lchip, 0, cmd, lpm_tcam_nat_cpu_wr_data);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    cpu_index = index;
    cpu_req_type = FIB_CPU_ACCESS_REQ_WRITE_DATA_MASK;
    cpu_req = TRUE;

    DRV_IOW_FIELD(lchip, LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuReq_f,
                          &cpu_req, lpm_tcam_nat_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuReqType_f,
                          &cpu_req_type, lpm_tcam_nat_cpu_req_ctl);
    DRV_IOW_FIELD(lchip, LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuIndex_f,
                          &cpu_index, lpm_tcam_nat_cpu_req_ctl);

    cmd = DRV_IOW(LpmTcamNatCpuReqCtl2_t, DRV_ENTRY_FLAG);
    ret = drv_ioctl(lchip, 0, cmd, lpm_tcam_nat_cpu_req_ctl);
    if (ret < DRV_E_NONE)
    {
        return ret;
    }

    while (TRUE)
    {
        cpu_req = TRUE;
        cmd = DRV_IOR(LpmTcamNatCpuReqCtl2_t, LpmTcamNatCpuReqCtl2_tcamNatCpuReq_f);
        ret = drv_ioctl(lchip, 0, cmd, &cpu_req);
        if (ret < DRV_E_NONE)
        {
            return ret;
        }

        if (!cpu_req)
        {
            break;
        }

        if ((time_out ++) > DRV_TIME_OUT)
        {
            return DRV_E_TIME_OUT;
        }
    }

    tbl_entry.data_entry = data;
    tbl_entry.mask_entry = mask;
    drv_ecc_recover_store(lchip, DRV_ECC_MEM_TCAM_KEY8, index, &tbl_entry);

    return DRV_E_NONE;
}
#endif

#ifdef NEW_DRV
STATIC uint32
_drv_usw_chip_tcam_write_check_match(uint32* p_r_data, uint32* p_w_data, uint32* p_r_mask, uint32* p_w_mask, uint32 num)
{
    uint32 i = 0;

    for (i = 0; i < num; i++)
    {
#if 0
        DRV_DBG_INFO("Table(%d), Index: %d, No.%d r_data:0x%x, r_mask:0x%x, w_data:0x%x, w_mask:0x%x\n",
                  tbl_id,index, i,p_r_data_tmp[i], p_r_mask_tmp[i], p_w_data_tmp[i], p_w_mask_tmp[i]);
#endif

        if (((p_r_data[i])&(p_r_mask[i])) != ((p_w_data[i])&(p_w_mask[i])))
        {

            /* DRV_DBG_INFO("Not Match!!!\n");*/
           p_r_data[i] = 0;
           p_r_mask[i] = 0;
           return  FALSE;
        }
    }

    return TRUE;

}
#endif
/**
 @brief Real embeded tcam ad write operation I/O
*/
int32
drv_usw_chip_write_flow_tcam_ad_entry(uint8 lchip, uint32 blknum, uint32 index, uint32* data)
{
#ifdef NEVER
    uint32 flow_tcam_cpu_req_ctl[FLOW_TCAM_CPU_REQ_CTL_BYTES/4] = {0};
    uint32 flow_tcam_ad_mem[FLOW_TCAM_AD_MEM_BYTES/4] = {0};
    uint32 cmd;
    int32 ret = DRV_E_NONE;
    uint32 lchip_offset = lchip + drv_init_chip_info.drv_init_lchip_base;

    DRV_PTR_VALID_CHECK(data);

    FLOW_TCAM_LOCK(lchip);

    DRV_IOW_FIELD(lchip, FlowTcamCpuReqCtl_t, FlowTcamCpuReqCtl_adCpublkNum_f,
                          &blknum, flow_tcam_cpu_req_ctl);

    cmd = DRV_IOW(FlowTcamCpuReqCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    drv_ioctl(lchip, 0, cmd, flow_tcam_cpu_req_ctl),
    p_flow_tcam_mutex[lchip]);

    sal_memcpy((uint8*)flow_tcam_ad_mem,(uint8*)data, DRV_BYTES_PER_ENTRY*2);


    cmd = DRV_IOW(FlowTcamAdMem_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    drv_ioctl(lchip, index, cmd, flow_tcam_ad_mem),
    p_flow_tcam_mutex[lchip]);

    FLOW_TCAM_UNLOCK(lchip);

    return ret;
#endif
    return 0;
}

/**
 @brief get flow tcam blknum and local index
*/
int32
drv_usw_chip_flow_tcam_get_blknum_index(uint8 lchip, tbls_id_t tbl_id, uint32 index,
                                            uint32 *blknum, uint32 *local_idx, uint32 *is_sec_half)
{
    uint8 addr_offset = 0;
    uint32 blk_id = 0;
    uint32 map_index = 0;

    *is_sec_half = 0;

    if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM)
    {
        for (blk_id = 0; blk_id < MAX_NOR_TCAM_NUM; blk_id++)
        {
            DRV_IF_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));

            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(lchip, tbl_id, blk_id)) && (index <= TCAM_END_INDEX(lchip, tbl_id, blk_id)))
            {
                map_index = index - TCAM_START_INDEX(lchip, tbl_id, blk_id);

                *blknum = blk_id;
                *local_idx = map_index;

                break;
            }
        }
    }
    else if(drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM_AD)
    {
        for (blk_id = 0; blk_id < MAX_NOR_TCAM_NUM; blk_id++)
        {
            DRV_IF_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));

            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(lchip, tbl_id, blk_id)) && (index <= TCAM_END_INDEX(lchip, tbl_id, blk_id)))
            {
                map_index = index - TCAM_START_INDEX(lchip, tbl_id, blk_id);

                *blknum = blk_id;
                *local_idx = map_index;

                break;
            }
        }
    }
    else
    {
        DRV_DBG_INFO("\nInvalid table id %d when get flow tcam block number and index!\n", tbl_id);
        return DRV_E_INVALID_TBL;
    }

    return DRV_E_NONE;
}

int32
drv_usw_chip_lpm_tcam_get_blknum_index(uint8 lchip, tbls_id_t tbl_id, uint32 index,
                                            uint32 *blknum, uint32 *local_idx)
{
    uint8 addr_offset = 0;
    uint32 blk_id = 0;
    uint32 map_index = 0;

    if ((drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_IP) || (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_NAT))
    {
        for (blk_id = 0; blk_id < MAX_LPM_TCAM_NUM; blk_id++)
        {
            DRV_IF_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));

            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(lchip, tbl_id, blk_id)) && (index <= TCAM_END_INDEX(lchip, tbl_id, blk_id)))
            {
                map_index = index - TCAM_START_INDEX(lchip, tbl_id, blk_id);

                *blknum = blk_id;
                *local_idx = map_index * (TCAM_KEY_SIZE(lchip, tbl_id)/DRV_LPM_KEY_BYTES_PER_ENTRY);
                break;
            }
        }
    }
    else if((drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM_LPM_AD) || (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM_NAT_AD))
    {
        for (blk_id = 0; blk_id < MAX_LPM_TCAM_NUM; blk_id++)
        {
            DRV_IF_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));

            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(lchip, tbl_id, blk_id)) && (index <= TCAM_END_INDEX(lchip, tbl_id, blk_id)))
            {
                map_index = index - TCAM_START_INDEX(lchip, tbl_id, blk_id);

                *blknum = blk_id;
                *local_idx = map_index * (TCAM_KEY_SIZE(lchip, tbl_id)/DRV_LPM_KEY_BYTES_PER_ENTRY);

                break;
            }
        }
    }
    else
    {
        DRV_DBG_INFO("\nInvalid table id %d when get flow tcam block number and index!\n", tbl_id);
        return DRV_E_INVALID_TBL;
    }

    /* Model use 12 lpm tcam block, need to map to 3 block rtl lpm tcam number */
    if(*blknum == 1)
    {
        *blknum = 0;
        *local_idx += DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM;
    }
    else  if(*blknum == 2)
    {
        *blknum = 0;
        *local_idx += DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM;
    }
    else  if(*blknum == 3)
    {
        *blknum = 0;
        *local_idx += DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY2_MAX_ENTRY_NUM;
    }
    else  if(*blknum == 4)
    {
        *blknum = 1;
    }
    else  if(*blknum == 5)
    {
        *blknum = 1;
        *local_idx += DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM;
    }
    else  if(*blknum == 6)
    {
        *blknum = 1;
        *local_idx += DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY5_MAX_ENTRY_NUM;
    }
    else  if(*blknum == 7)
    {
        *blknum = 1;
        *local_idx += DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY5_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY6_MAX_ENTRY_NUM;
    }
    else  if(*blknum == 8)
    {
        *blknum = 2;
    }
    else  if(*blknum == 9)
    {
        *blknum = 2;
        *local_idx += DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM;
    }
    else  if(*blknum == 10)
    {
        *blknum = 2;
        *local_idx += DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY9_MAX_ENTRY_NUM;
    }
    else  if(*blknum == 11)
    {
        *blknum = 2;
        *local_idx += DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY9_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY10_MAX_ENTRY_NUM;
    }

    return DRV_E_NONE;
}

/**
 @brief The function read table data from a sram memory location
*/
int32
drv_usw_chip_sram_tbl_read(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32* data)
{
    #define DYN_SRAM_ENTRY_BYTE 16
    uint32 start_data_addr = 0, entry_size = 0, max_index_num = 0;
    int32 ret = DRV_E_NONE;
     /*uint32 blknum = 0, local_idx = 0,is_sec_half = 0;*/
    uint32 burst_op = 0;
    drv_access_type_t acc_type;

    DRV_PTR_VALID_CHECK(data);
     /*DRV_CHIP_ID_VALID_CHECK(lchip);*/
    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);

    drv_get_access_type(lchip, &acc_type);
    /* check if the index num exceed the max index num of the tbl */
    max_index_num = TABLE_MAX_INDEX(lchip, tbl_id);
    entry_size = TABLE_ENTRY_SIZE(lchip, tbl_id);

    DRV_IF_ERROR_RETURN(drv_usw_table_get_hw_addr(lchip, tbl_id, index, &start_data_addr, FALSE));

    if (max_index_num <= index)
    {
        DRV_DBG_INFO("ERROR! (drv_usw_read_sram_tbl): chip-0x%x, tbl-0x%x, index-0x%x exceeds the max_index-0x%x.\n",
                    lchip, tbl_id, index, max_index_num);
        return DRV_E_INVALID_TBL;
    }

    burst_op = !(TABLE_OP_TYPE(lchip, tbl_id)&Op_DisBurst) && p_drv_master[lchip]->burst_en;

    start_data_addr = start_data_addr + burst_op*BURST_IO_READ;

    ret = drv_usw_chip_read_sram_entry(lchip, start_data_addr, data, entry_size);

    if (ret!= DRV_E_NONE)
    {
        DRV_DBG_INFO("tbl_id:%d, index:%d, start_data_addr:%08x , ret:%d\n",tbl_id, index,start_data_addr, ret);
    }

    return ret;
}

/**
 @brief The function write table data to a sram memory location
*/
int32
drv_usw_chip_sram_tbl_write(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32* data)
{
    uint32 start_data_addr = 0, entry_size = 0, max_index_num = 0;
    int32 ret = DRV_E_NONE;
    #if (SDK_WORK_PLATFORM == 0)
    uint32 hw_addr_size_per_idx = 0;
    #endif
    uint32 burst_op = 0;
    tbl_entry_t ecc_tbl_entry;
    tbls_id_t ecc_tbl_id;
    uint32 ecc_index;

    drv_access_type_t acc_type;
    DRV_PTR_VALID_CHECK(data);
    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);

    ecc_tbl_entry.data_entry = data;
    ecc_tbl_entry.mask_entry = NULL;
    ecc_tbl_id = tbl_id;
    ecc_index = index;

    drv_get_access_type(lchip, &acc_type);
    /* check if the index num exceed the max index num of the tbl */
    max_index_num = TABLE_MAX_INDEX(lchip, tbl_id);
    entry_size = TABLE_ENTRY_SIZE(lchip, tbl_id);

    DRV_IF_ERROR_RETURN(drv_usw_table_get_hw_addr(lchip, tbl_id, index, &start_data_addr, FALSE));

    burst_op = !(TABLE_OP_TYPE(lchip, tbl_id)&Op_DisBurst) && p_drv_master[lchip]->burst_en;

    /*for burst io*/
    start_data_addr = start_data_addr +burst_op*BURST_IO_WRITE;

    #if (SDK_WORK_PLATFORM == 0)
    if(TABLE_EXT_INFO_PTR(lchip, tbl_id) && (TABLE_EXT_TYPE(lchip, tbl_id) == 1))  /*parser table*/
    {
        uint8 tbl_offset;
        for(tbl_offset=0; tbl_offset < (sizeof(parser_tbl_id_list[0])/sizeof(uint32)); tbl_offset++)
        {
            if(tbl_id == parser_tbl_id_list[0][tbl_offset])
            {
                break;
            }
        }

        if(tbl_offset == sizeof(parser_tbl_id_list[0])/sizeof(uint32))
        {
            DRV_DBG_INFO("ERROR! (drv_usw_write_sram_tbl): chip-0x%x, tbl-0x%x, index-0x%x is not an parser table\n",
                    lchip, tbl_id, index);
            return DRV_E_INVALID_TBL;
        }

        DRV_IF_ERROR_RETURN(drv_usw_table_consum_hw_addr_size_per_index(lchip, tbl_id, &hw_addr_size_per_idx));

        start_data_addr = TABLE_DATA_BASE(lchip, parser_tbl_id_list[2][tbl_offset], 0) + index *hw_addr_size_per_idx*DRV_BYTES_PER_WORD+burst_op*BURST_IO_WRITE;
        ret = drv_usw_chip_write_sram_entry(lchip, start_data_addr, data, entry_size);

        start_data_addr = TABLE_DATA_BASE(lchip, parser_tbl_id_list[1][tbl_offset], 0) + index *hw_addr_size_per_idx*DRV_BYTES_PER_WORD+burst_op*BURST_IO_WRITE;
        ret = drv_usw_chip_write_sram_entry(lchip, start_data_addr, data, entry_size);

        start_data_addr = TABLE_DATA_BASE(lchip, parser_tbl_id_list[0][tbl_offset], 0) + index *hw_addr_size_per_idx*DRV_BYTES_PER_WORD+burst_op*BURST_IO_WRITE;
        ret = drv_usw_chip_write_sram_entry(lchip, start_data_addr, data, entry_size);

        if (ret)
        {
            return ret;
        }

        drv_ser_db_store(lchip, ecc_tbl_id, ecc_index, (void*)&ecc_tbl_entry);
        return ret;
    }

    if(TABLE_EXT_INFO_PTR(lchip, tbl_id) && (TABLE_EXT_TYPE(lchip, tbl_id) == 2))  /*80bit acl ad table*/
    {
        uint32 data_rd[6] = {0};

        /*Process as 160bits*/
        DRV_IF_ERROR_RETURN(drv_usw_table_get_hw_addr(lchip, tbl_id, (index&0xFFFFFFFE), &start_data_addr, FALSE));

        ret = drv_usw_chip_read_sram_entry(lchip, start_data_addr, data_rd, entry_size*2);
        if (ret)
        {
            return ret;
        }

        if (index&0x1)
        {
            /*high 80bits*/
            sal_memcpy((uint8*)&data_rd[3], (uint8*)data, 12);
        }
        else
        {
            /*low 80bits*/
            sal_memcpy((uint8*)&data_rd[0], (uint8*)data, 12);
        }

        ret = drv_usw_chip_write_sram_entry(lchip, start_data_addr, data_rd, entry_size*2);
        if (ret)
        {
            return ret;
        }

        drv_ser_db_store(lchip, ecc_tbl_id, ecc_index, (void*)&ecc_tbl_entry);
        return ret;
    }

    #endif

    if (max_index_num <= index)
    {
        DRV_DBG_INFO("ERROR! (drv_usw_write_sram_tbl): chip-0x%x, tbl-0x%x, index-0x%x exceeds the max_index-0x%x.\n",
                    lchip, tbl_id, index, max_index_num);
        return DRV_E_INVALID_TBL;
    }

    ret = drv_usw_chip_write_sram_entry(lchip, start_data_addr, data, entry_size);

    if (ret!= DRV_E_NONE)
    {
        DRV_DBG_INFO("tbl_id:%d, index:%d, start_data_addr:%08x \n",tbl_id, index,start_data_addr);
        return ret;
    }

    drv_ser_db_store(lchip, ecc_tbl_id, ecc_index, (void*)&ecc_tbl_entry);
    return ret;
}


/**
 @brief read tcam interface (include operate model and real tcam)
*/
int32
drv_usw_chip_tcam_tbl_read(uint8 lchip, tbls_id_t tbl_id, uint32 index, tbl_entry_t *entry)
{
    uint32* mask = NULL;
    uint32* data = NULL;
    uint32 key_size = TCAM_KEY_SIZE(lchip, tbl_id);
    uint32 entry_num_each_idx = 0, entry_idx = 0;
    bool is_lpm_tcam = FALSE;
    uint8 empty = 0;
    uint32 blknum = 0, local_idx = 0, is_sec_half = 0;
    tbl_entry_t tbl_entry;

    DRV_PTR_VALID_CHECK(entry);
    DRV_PTR_VALID_CHECK(entry->data_entry);
    DRV_PTR_VALID_CHECK(entry->mask_entry);
    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);

    if ((drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_TCAM)
        && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_IP)
        && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_NAT)
        && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_STATIC_TCAM_KEY))
    {
        DRV_DBG_INFO("@@ ERROR! When read operation, INVALID Tcam key TblID! TblID: %d, file:%s line:%d function:%s\n",
                      tbl_id, __FILE__, __LINE__, __FUNCTION__);
        return DRV_E_INVALID_TBL;
    }

    if (TABLE_MAX_INDEX(lchip, tbl_id) <= index)
    {
        DRV_DBG_INFO("ERROR (drv_usw_chip_tcam_tbl_read): chip-0x%x, tbl-%d, index-0x%x exceeds the max_index-0x%x.\n",
                        lchip, tbl_id, index, TABLE_MAX_INDEX(lchip, tbl_id));
        return DRV_E_INVALID_TBL;
    }

    if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_STATIC_TCAM_KEY)
    {
        entry_num_each_idx = 1;
        local_idx = index;
    }
    else if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM)
    {
        /* flow tcam w/r per 80 bit */
        entry_num_each_idx = key_size / DRV_BYTES_PER_ENTRY;

        drv_usw_chip_flow_tcam_get_blknum_index(lchip, tbl_id, index, &blknum, &local_idx, &is_sec_half);
        is_lpm_tcam = 0;

        local_idx = local_idx*(key_size/DRV_BYTES_PER_ENTRY);
        /* bit 12 indicate if it is in high half TCAM in double size mode */
         /*local_idx = (is_sec_half << 12) | local_idx;*/
    }
    else if((drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_IP) || (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_NAT))
    { /*SDK Modify {*/
        drv_usw_chip_lpm_tcam_get_blknum_index(lchip, tbl_id, index, &blknum, &local_idx);
        is_lpm_tcam = TRUE;
        entry_num_each_idx = key_size / DRV_LPM_KEY_BYTES_PER_ENTRY;
    }

    data = entry->data_entry;
    mask = entry->mask_entry;

    for (entry_idx = 0; entry_idx < entry_num_each_idx; ++entry_idx)
    {
        empty = 0;
        /* read real tcam address */

        if (tbl_id == DsQueueMapTcamKey_t)
        {

            DRV_IF_ERROR_RETURN(_drv_usw_chip_read_enque_tcam_entry(lchip, local_idx,
            data + entry_idx*2, mask + entry_idx*2));

            drv_usw_chip_convert_tcam_dump_content(lchip, TABLE_ENTRY_SIZE(lchip, tbl_id)*8,
            data + entry_idx*2,
            mask + entry_idx*2, &empty);

            tbl_entry.data_entry = data + entry_idx*DRV_WORDS_PER_ENTRY;
            tbl_entry.mask_entry = mask + entry_idx*DRV_WORDS_PER_ENTRY;
            drv_ser_check_correct_tcam(lchip, DRV_FTM_QUEUE_TCAM, local_idx, (void*)&tbl_entry, empty);
        }
        else if (tbl_id == DsCategoryIdPairTcamKey_t)
        {

            DRV_IF_ERROR_RETURN(_drv_usw_chip_read_cid_tcam_entry(lchip, local_idx,
            data + entry_idx*1, mask + entry_idx*1));

            drv_usw_chip_convert_tcam_dump_content(lchip, TABLE_ENTRY_SIZE(lchip, tbl_id)*8,
            data + entry_idx*1,
            mask + entry_idx*1, &empty);

            tbl_entry.data_entry = data + entry_idx*DRV_WORDS_PER_ENTRY;
            tbl_entry.mask_entry = mask + entry_idx*DRV_WORDS_PER_ENTRY;
            drv_ser_check_correct_tcam(lchip, DRV_FTM_CID_TCAM, local_idx, (void*)&tbl_entry, empty);
        }
        else if (!is_lpm_tcam)
        {
             /*FLOW_TCAM_LOCK(lchip_offset);*/

            DRV_IF_ERROR_RETURN(_drv_usw_chip_read_flow_tcam_entry(lchip, blknum, local_idx,
            data + entry_idx*DRV_WORDS_PER_ENTRY, mask + entry_idx*DRV_WORDS_PER_ENTRY));

            drv_usw_chip_convert_tcam_dump_content(lchip, DRV_BYTES_PER_ENTRY*8,
            data + entry_idx*DRV_WORDS_PER_ENTRY,
            mask + entry_idx*DRV_WORDS_PER_ENTRY, &empty);

            tbl_entry.data_entry = data + entry_idx*DRV_WORDS_PER_ENTRY;
            tbl_entry.mask_entry = mask + entry_idx*DRV_WORDS_PER_ENTRY;
            drv_ser_check_correct_tcam(lchip, DRV_FTM_TCAM_KEY0 + blknum, local_idx, (void*)&tbl_entry, empty);

             /*FLOW_TCAM_UNLOCK(lchip_offset);*/
        }
        else
        {
             /*if(is_lpm_tcam_ip)*/
            {
                 /*LPM_IP_TCAM_LOCK(lchip_offset);*/

                DRV_IF_ERROR_RETURN(_drv_usw_chip_read_lpm_tcam_ip_entry(lchip, blknum, local_idx,
                data + entry_idx*DRV_LPM_WORDS_PER_ENTRY,
                mask + entry_idx*DRV_LPM_WORDS_PER_ENTRY));

                /*
                DRV_IF_ERROR_RETURN_WITH_UNLOCK(
                    _drv_chip_read_lpm_tcam_ip_entry(lchip, local_idx,
                    mask + entry_idx*DRV_LPM_WORDS_PER_ENTRY,
                    DRV_INT_LPM_TCAM_RECORD_MASK),
                    p_drv_master[lchip_offset]->p_lpm_ip_tcam_mutex);

                DRV_IF_ERROR_RETURN_WITH_UNLOCK(
                    _drv_chip_read_lpm_tcam_ip_entry(lchip, local_idx,
                    data + entry_idx*DRV_LPM_WORDS_PER_ENTRY,
                    DRV_INT_LPM_TCAM_RECORD_DATA),
                    p_drv_master[lchip_offset]->p_lpm_ip_tcam_mutex);
                */

                drv_usw_chip_convert_tcam_dump_content(lchip,
                DRV_LPM_KEY_BYTES_PER_ENTRY*8,
                   data + entry_idx*DRV_LPM_WORDS_PER_ENTRY,
                mask + entry_idx*DRV_LPM_WORDS_PER_ENTRY, &empty);

                tbl_entry.data_entry = data + entry_idx*DRV_LPM_WORDS_PER_ENTRY;
                tbl_entry.mask_entry = mask + entry_idx*DRV_LPM_WORDS_PER_ENTRY;
                drv_ser_check_correct_tcam(lchip, DRV_FTM_LPM_TCAM_KEY0 + blknum, local_idx, (void*)&tbl_entry, empty);

                 /*LPM_IP_TCAM_UNLOCK(lchip_offset);*/
            }
        }

        local_idx++;
    }

    return DRV_E_NONE;
}

/**
 @brief write tcam interface (include operate model and real tcam)
*/
int32
drv_usw_chip_tcam_tbl_write(uint8 lchip, tbls_id_t tbl_id, uint32 index, tbl_entry_t* entry)
{
    uint32* mask = NULL;
    uint32* data = NULL;
    uint32 key_size = TCAM_KEY_SIZE(lchip, tbl_id);
     /*uint32 tcam_asic_data_base = 0, tcam_asic_mask_base = 0;*/
    uint32 entry_num_each_idx = 0, entry_idx = 0;
     /*uint32 index_by_tcam = 0;*/
    bool is_lpm_tcam = FALSE;
     /*bool is_lpm_tcam_ip = FALSE;*/
    uint32 blknum = 0, local_idx = 0, is_sec_half = 0;
    tbl_entry_t ecc_tbl_entry;

    DRV_PTR_VALID_CHECK(entry);
    DRV_PTR_VALID_CHECK(entry->data_entry);
    DRV_PTR_VALID_CHECK(entry->mask_entry);
    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);

    if ((drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_TCAM)
        && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_IP)
        && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_NAT)
        && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_STATIC_TCAM_KEY))
    {
        DRV_DBG_INFO("@@ ERROR! When write operation, INVALID Tcam key TblID! TblID: %d, file:%s line:%d function:%s\n",
                      tbl_id, __FILE__, __LINE__, __FUNCTION__);
        return DRV_E_INVALID_TBL;
    }

    if (TABLE_MAX_INDEX(lchip, tbl_id) <= index)
    {
        DRV_DBG_INFO("ERROR (drv_usw_chip_tcam_tbl_write): chip-0x%x, tbl-%d, index-0x%x exceeds the max_index-0x%x.\n",
                        lchip, tbl_id, index, TABLE_MAX_INDEX(lchip, tbl_id));
        return DRV_E_INVALID_TBL;
    }

    if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_STATIC_TCAM_KEY)
    {
        entry_num_each_idx = 1;
        local_idx = index;
    }
    else if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM)
    {
        /* flow tcam w/r per 80 bit */
        entry_num_each_idx = key_size / (DRV_BYTES_PER_ENTRY);

        drv_usw_chip_flow_tcam_get_blknum_index(lchip, tbl_id, index, &blknum, &local_idx, &is_sec_half);
        is_lpm_tcam = FALSE;

        local_idx = local_idx*entry_num_each_idx;
        /* bit 12 indicate if it is in high half TCAM in double size mode */
         /*local_idx = (is_sec_half << 12) | local_idx;*/
    }
    else if((drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_IP) || (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_NAT))
    {
        drv_usw_chip_lpm_tcam_get_blknum_index(lchip, tbl_id, index, &blknum, &local_idx);
        is_lpm_tcam = TRUE;
        entry_num_each_idx = key_size / DRV_LPM_KEY_BYTES_PER_ENTRY;
    }



    data = entry->data_entry;
    mask = entry->mask_entry;

    for (entry_idx = 0; entry_idx < entry_num_each_idx; ++entry_idx)
    {
         /*uint32 data_tmp[DRV_LPM_WORDS_PER_ENTRY*4] = {0};*/
         /*uint32 mask_tmp[DRV_LPM_WORDS_PER_ENTRY*4] = {0};*/
         /*uint32 *p_r_data_tmp = data_tmp;*/
         /*uint32 *p_r_mask_tmp = mask_tmp;*/
         /*uint32 *p_w_data_tmp = 0;*/
         /*uint32 *p_w_mask_tmp = 0;*/
         /*uint32 word = 0;*/

        /* write real tcam address */
        /*TsEngineRefRc_m ts_ref_rc;*/
        /* uint32 cmd = DRV_IOR(TsEngineRefRc_t, DRV_ENTRY_FLAG);*/
        /*uint32 ts_s;
        uint64 ts_ns;
        uint64 total_ns = 0;
        uint32 ts_array[2] = {0};*/

        /*DRV_IOCTL(lchip, 0, cmd, &ts_ref_rc);*/
        /*ts_s = GetTsEngineRefRc(V, rcSecond_f, &ts_ref_rc);
        ts_ns = GetTsEngineRefRc(V, rcNs_f, &ts_ref_rc);*/

        /*total ns is 60 bits in dma desc, so need to truncate*/
        /*total_ns = (ts_s*1000*1000*1000+ts_ns);
        ts_array[0] = (total_ns)&0xFFFFFFFF;
        ts_array[1] = (total_ns>>32)&0x3FFFFFFF;*/

        if (tbl_id == DsQueueMapTcamKey_t)
        {

            DRV_IF_ERROR_RETURN(drv_usw_chip_write_enque_tcam_data_mask(lchip, local_idx,
            data + entry_idx*2,
            mask + entry_idx*2));
            ecc_tbl_entry.data_entry = (data + entry_idx*2);
            ecc_tbl_entry.mask_entry = (mask + entry_idx*2);
            DRV_IF_ERROR_RETURN(drv_ser_db_store(lchip, DRV_FTM_QUEUE_TCAM, local_idx, (void*)&ecc_tbl_entry));
        }
        else if (tbl_id == DsCategoryIdPairTcamKey_t)
        {

            DRV_IF_ERROR_RETURN(drv_usw_chip_write_cid_tcam_data_mask(lchip, local_idx,
            data + entry_idx*1,
            mask + entry_idx*1));

            ecc_tbl_entry.data_entry = (data + entry_idx*1);
            ecc_tbl_entry.mask_entry = (mask + entry_idx*1);
            DRV_IF_ERROR_RETURN(drv_ser_db_store(lchip, DRV_FTM_CID_TCAM, local_idx, (void*)&ecc_tbl_entry));
        }
        else if (!is_lpm_tcam)
        {
             /*FLOW_TCAM_LOCK(lchip_offset);*/

            DRV_IF_ERROR_RETURN(drv_usw_chip_write_flow_tcam_data_mask(lchip, blknum, local_idx,
            data + entry_idx*DRV_WORDS_PER_ENTRY,
            mask + entry_idx*DRV_WORDS_PER_ENTRY));

            ecc_tbl_entry.data_entry = (data + entry_idx*DRV_WORDS_PER_ENTRY);
            ecc_tbl_entry.mask_entry = (mask + entry_idx*DRV_WORDS_PER_ENTRY);
            DRV_IF_ERROR_RETURN(drv_ser_db_store(lchip, DRV_FTM_TCAM_KEY0 + blknum, local_idx, (void*)&ecc_tbl_entry));
             /*FLOW_TCAM_UNLOCK(lchip_offset);*/
        }
        else
        {
            DRV_IF_ERROR_RETURN(drv_usw_chip_write_lpm_tcam_ip_data_mask(lchip, blknum, local_idx,
                    data + entry_idx*DRV_LPM_WORDS_PER_ENTRY, mask + entry_idx*DRV_LPM_WORDS_PER_ENTRY));

            ecc_tbl_entry.data_entry = (data + entry_idx*DRV_LPM_WORDS_PER_ENTRY);
            ecc_tbl_entry.mask_entry = (mask + entry_idx*DRV_LPM_WORDS_PER_ENTRY);
            DRV_IF_ERROR_RETURN(drv_ser_db_store(lchip, DRV_FTM_LPM_TCAM_KEY0 + blknum, local_idx, (void*)&ecc_tbl_entry));
        }

        local_idx++;
    }

    return DRV_E_NONE;
}

/**
 @brief remove tcam entry interface (include operate model and real tcam)
*/
int32
drv_usw_chip_tcam_tbl_remove(uint8 lchip, tbls_id_t tbl_id, uint32 index)
{
    uint32 entry_idx = 0, entry_num_each_idx = 0;
    uint32 key_size = TCAM_KEY_SIZE(lchip, tbl_id);
    bool is_lpm_tcam = FALSE;
     /*bool is_lpm_tcam_ip = FALSE;*/
    uint32 blknum = 0, local_idx = 0, is_sec_half = 0;
    uint32 ecc_mask = 0;
    tbl_entry_t ecc_tbl_entry;

    if ((drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_TCAM)
        && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_IP)
        && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_LPM_TCAM_NAT)
        && (drv_usw_get_table_type(lchip, tbl_id) != DRV_TABLE_TYPE_STATIC_TCAM_KEY))
    {
        DRV_DBG_INFO("@@ ERROR! When read operation, INVALID Tcam key TblID! TblID: %d, file:%s line:%d function:%s\n",
                      tbl_id, __FILE__, __LINE__, __FUNCTION__);
        return DRV_E_INVALID_TBL;
    }

    if (TABLE_MAX_INDEX(lchip, tbl_id) <= index)
    {
        DRV_DBG_INFO("ERROR (drv_usw_chip_tcam_tbl_read): chip-0x%x, tbl-%d, index-0x%x exceeds the max_index-0x%x.\n",
                        lchip, tbl_id, index, TABLE_MAX_INDEX(lchip, tbl_id));
        return DRV_E_INVALID_TBL;
    }

    if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_STATIC_TCAM_KEY)
    {
        entry_num_each_idx = 1;
        local_idx = index;
    }
    else if (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM)
    {
        /* flow tcam w/r per 80 bit */
        entry_num_each_idx = key_size / (DRV_BYTES_PER_ENTRY);

        drv_usw_chip_flow_tcam_get_blknum_index(lchip, tbl_id, index, &blknum, &local_idx, &is_sec_half);
        is_lpm_tcam = 0;

        local_idx = local_idx*(key_size/DRV_BYTES_PER_ENTRY);
        /* bit 12 indicate if it is in high half TCAM in double size mode */
         /*local_idx = (is_sec_half << 12) | local_idx;*/
    }
    else if((drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_IP) || (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_NAT))
    {
        drv_usw_chip_lpm_tcam_get_blknum_index(lchip, tbl_id, index, &blknum, &local_idx);
        is_lpm_tcam = TRUE;
        entry_num_each_idx = key_size / DRV_LPM_KEY_BYTES_PER_ENTRY;

    }

    for (entry_idx = 0; entry_idx < entry_num_each_idx; ++entry_idx)
    {
        /* remove tcam entry on emulation board */
        if (tbl_id == DsQueueMapTcamKey_t)
        {
            DRV_IF_ERROR_RETURN(drv_usw_chip_remove_enque_tcam_entry(lchip, local_idx));

            ecc_tbl_entry.data_entry = NULL;
            ecc_tbl_entry.mask_entry = &ecc_mask;
            DRV_IF_ERROR_RETURN(drv_ser_db_store(lchip, DRV_FTM_QUEUE_TCAM, local_idx, (void*)&ecc_tbl_entry));
        }
        else if (tbl_id == DsCategoryIdPairTcamKey_t)
        {
            DRV_IF_ERROR_RETURN(drv_usw_chip_remove_cid_tcam_entry(lchip, local_idx));

            ecc_tbl_entry.data_entry = NULL;
            ecc_tbl_entry.mask_entry = &ecc_mask;
            DRV_IF_ERROR_RETURN(drv_ser_db_store(lchip, DRV_FTM_CID_TCAM, local_idx, (void*)&ecc_tbl_entry));
        }
        else if (!is_lpm_tcam)
        {
            DRV_IF_ERROR_RETURN(drv_usw_chip_remove_flow_tcam_entry(lchip, blknum, local_idx));

            ecc_tbl_entry.data_entry = NULL;
            ecc_tbl_entry.mask_entry = &ecc_mask;
            DRV_IF_ERROR_RETURN(drv_ser_db_store(lchip, DRV_FTM_TCAM_KEY0 + blknum, local_idx, (void*)&ecc_tbl_entry));
        }
        else
        {
            DRV_IF_ERROR_RETURN(drv_usw_chip_remove_lpm_tcam_ip_entry(lchip, blknum, local_idx));

            ecc_tbl_entry.data_entry = NULL;
            ecc_tbl_entry.mask_entry = &ecc_mask;
            DRV_IF_ERROR_RETURN(drv_ser_db_store(lchip, DRV_FTM_LPM_TCAM_KEY0 + blknum, local_idx, (void*)&ecc_tbl_entry));
        }

        local_idx++;
    }

    return DRV_E_NONE;
}

int32
drv_chip_common_init(uint8 lchip)
{
    if (!p_drv_master[lchip])
    {
        return DRV_E_INIT_FAILED;
    }

#ifdef CHIP_AGENT
    p_drv_master[lchip]->burst_en = 0;

#else
    p_drv_master[lchip]->burst_en = 1;

#endif

    drv_install_api(lchip, &g_chip_io_func);

    /* initialize tcam I/O mutex */
    DRV_IF_ERROR_RETURN(drv_usw_chip_tcam_mutex_init(lchip));

    /* initialize pci I/O mutex */
    DRV_IF_ERROR_RETURN(drv_usw_chip_pci_mutex_init(lchip));

    /* initialize i2c I/O mutex */
    DRV_IF_ERROR_RETURN(drv_usw_chip_i2c_mutex_init(lchip));

    /* initialize Hss I/O mutex */
    DRV_IF_ERROR_RETURN(drv_usw_chip_hss_mutex_init(lchip));

    /* initialize Acc I/O mutex */
    DRV_IF_ERROR_RETURN(drv_usw_chip_acc_mutex_init(lchip));

    /* */
#ifdef DUET2
{
    extern dal_op_t g_dal_op;
    uint32 value = 0;
    g_dal_op.pci_read(lchip, 0x4c, &value);
    value = value | (1<<28);
    g_dal_op.pci_write(lchip, 0x4c, value);
}
#endif
    return DRV_E_NONE;
}
