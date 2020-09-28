/**
 @file sys_duet2_datapath.h

 @date 2018-09-12

 @version v1.0

 The file define APIs and types use in sys layer
*/
#ifndef _SYS_DUET2_DATAPATH_H
#define _SYS_DUET2_DATAPATH_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_chip.h"
#include "ctc_const.h"
#include "ctc_vector.h"
#include "sys_usw_dmps.h"
#include "sys_usw_common.h"
#include "sys_usw_qos_api.h"


/***************************************************************
 *
 * Macro definition
 *
 ***************************************************************/

/***************************************************************
 *
 * Structure definition
 *
 ***************************************************************/

/***************************************************************
 *
 * Function declaration
 *
 ***************************************************************/
int32 sys_duet2_datapath_init(uint8 lchip, ctc_datapath_global_cfg_t* p_datapath_cfg);

int32 sys_duet2_serdes_set_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);

int32 sys_duet2_serdes_get_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);

int32 sys_duet2_serdes_set_mode(uint8 lchip, ctc_chip_serdes_info_t* p_serdes_info);


#ifdef __cplusplus
}
#endif

#endif

