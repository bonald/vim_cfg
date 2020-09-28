/**
  @file drv_io.h

  @date 2010-02-26

  @version v5.1

  The file implement driver IOCTL defines and macros
*/

#ifndef _DRV_IO_H_
#define _DRV_IO_H_

#include "usw/include/drv_enum.h"
#include "usw/include/drv_common.h"
#include "drv_api.h"


/**********************************************************************************
              Define Typedef, define and Data Structure
***********************************************************************************/

#define HASH_KEY_LENGTH_MODE_SINGLE      0
#define HASH_KEY_LENGTH_MODE_DOUBLE      1
#define HASH_KEY_LENGTH_MODE_QUAD        2

#define FIB_HOST0_CAM_NUM          ( 32 )
#define FIB_HOST1_CAM_NUM          ( 32 )
#define FLOW_HASH_CAM_NUM          ( 32 )
#define USER_ID_HASH_CAM_NUM       ( 32 )
#define XC_OAM_HASH_CAM_NUM        ( 128 )

#define _DRV_FIB_ACC_M_1

struct addr_array_s
{
    uint8 valid;
    uint32 start_addr;
    uint32 end_addr;
};
typedef struct addr_array_s addr_array_t;

/**********************************************************************************
                      Define API function interfaces
***********************************************************************************/
/**
 @brief remove tcam entry interface according to key id and index
*/
extern int32
drv_usw_chip_sram_tbl_read(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32* data);

extern int32
drv_usw_chip_sram_tbl_write(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32* data);

extern int32
drv_usw_chip_tcam_tbl_read(uint8 lchip_offset, tbls_id_t tbl_id, uint32 index, tbl_entry_t *entry);

extern int32
drv_usw_chip_tcam_tbl_write(uint8 lchip_offset, tbls_id_t tbl_id, uint32 index, tbl_entry_t* entry);

extern int32
drv_usw_chip_tcam_tbl_remove(uint8 lchip_offset, tbls_id_t tbl_id, uint32 index);

extern int32
drv_usw_chip_write_flow_tcam_data_mask(uint8 chip_id, uint32 blknum, uint32 index, uint32* data, uint32* mask);

extern int32
drv_usw_chip_remove_flow_tcam_entry(uint8 chip_id, uint32 blknum, uint32 tcam_index);

extern int32
drv_usw_chip_write_lpm_tcam_ip_data_mask(uint8 chip_id, uint32 blknum, uint32 index, uint32* data, uint32* mask);

extern int32
drv_usw_chip_remove_lpm_tcam_ip_entry(uint8 chip_id, uint32 blknum, uint32 tcam_index);

extern int32
drv_usw_chip_remove_enque_tcam_entry(uint8 chip_id, uint32 tcam_index);

extern int32
drv_usw_chip_write_enque_tcam_data_mask(uint8 chip_id, uint32 index, uint32* data, uint32* mask);

extern int32
drv_usw_chip_remove_cid_tcam_entry(uint8 chip_id, uint32 tcam_index);

extern int32
drv_usw_chip_write_cid_tcam_data_mask(uint8 chip_id, uint32 index, uint32* data, uint32* mask);

extern int32
drv_usw_chip_convert_tcam_dump_content(uint8 lchip, uint32 tcam_entry_width, uint32 *data, uint32 *mask, uint8* p_empty);

#endif /*end of _DRV_IO_H*/

