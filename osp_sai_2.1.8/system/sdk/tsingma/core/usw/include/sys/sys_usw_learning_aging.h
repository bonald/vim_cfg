/**
 @file sys_usw_learning_aging.h

 @date 2010-3-16

 @version v2.0

---file comments----
*/

#ifndef _SYS_USW_LEARNING_AGING_H
#define _SYS_USW_LEARNING_AGING_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_learning_aging.h"
#include "ctc_interrupt.h"
#include "ctc_register.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
enum sys_aging_domain_type_e
{
    SYS_AGING_DOMAIN_TCAM,
    SYS_AGING_DOMAIN_MAC_HASH,
    SYS_AGING_DOMAIN_FLOW_HASH,
    SYS_AGING_DOMAIN_IP_HASH,
    SYS_AGING_DOMAIN_HOST1,
    SYS_AGING_DOMAIN_NUM
};
typedef enum sys_aging_domain_type_e sys_aging_domain_type_t;

#define  SYS_AGING_TIMER_INDEX_MAC      1     /*for normal fdb aging*/
#define  SYS_AGING_TIMER_INDEX_RSV  2         /*for RTL Bug 106537*/
#define  SYS_AGING_TIMER_INDEX_PENDING  3     /*for pending fdb aging*/


struct sys_learning_aging_data_s
{
    mac_addr_t mac;

    uint16  fid;
    uint16  gport;
    uint16  cvlan;
    uint16  svlan;
    uint8   ether_oam_md_level;
    uint8   is_aging;
    uint8   is_ether_oam;
    uint8   is_logic_port;
    uint8   is_hw;
    uint32  key_index;
    uint32  ad_index;
};
typedef struct sys_learning_aging_data_s sys_learning_aging_data_t;

struct sys_learning_aging_info_s
{
    uint32 entry_num;
    sys_learning_aging_data_t* data;
};
typedef struct sys_learning_aging_info_s sys_learning_aging_info_t;

typedef int32 (* sys_learning_aging_fn_t)(sys_learning_aging_info_t* info);

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

/***************************************
 Learning module's sys interfaces
*****************************************/
extern int32
sys_usw_learning_set_action(uint8 lchip, ctc_learning_action_info_t* p_learning_action);

extern int32
sys_usw_learning_get_action(uint8 lchip, ctc_learning_action_info_t* p_learning_action);

/***************************************
 Aging module's sys interfaces
*****************************************/
extern int32
sys_usw_aging_set_property(uint8 lchip, ctc_aging_table_type_t tbl_type, ctc_aging_prop_t aging_prop, uint32 value);

extern int32
sys_usw_aging_get_property(uint8 lchip, ctc_aging_table_type_t tbl_type, ctc_aging_prop_t aging_prop, uint32* value);

extern int32
sys_usw_aging_set_aging_status(uint8 lchip, sys_aging_domain_type_t domain_type, uint32 key_index, uint8 timer, uint8 status);

extern int32
sys_usw_aging_get_aging_index_status(uint8 lchip, uint32 fifo_ptr, ctc_aging_status_t* age_status);


/***************************************
 Init interfaces
*****************************************/
extern int32
sys_usw_aging_get_aging_ptr(uint8 lchip, uint8 domain_type, uint32 key_index, uint32* aging_ptr);

extern int32
sys_usw_learning_aging_init(uint8 lchip, ctc_learn_aging_global_cfg_t* cfg);

extern int32
sys_usw_learning_aging_deinit(uint8 lchip);

extern int32
sys_usw_learning_aging_register_cb2(uint8 lchip, sys_learning_aging_fn_t cb);

extern int32
sys_usw_learning_aging_sync_data(uint8 lchip, void* p_dma_info);
/**
for  cli
**/
int32
sys_usw_aging_get_aging_status(uint8 lchip, sys_aging_domain_type_t domain_type, uint32 key_index, uint8* hit);

int32
sys_usw_learning_aging_set_hw_sync(uint8 lchip, uint8 b_sync);

extern int32
sys_usw_aging_set_aging_status_by_ptr(uint8 lchip, uint32 domain_type, uint32 aging_ptr, bool enable);

int32
sys_usw_learning_aging_get_cb(uint8 lchip, void **cb, void** user_data, uint8 type);

int32
sys_usw_aging_set_aging_timer(uint8 lchip, uint32 key_index, uint8 timer);

int32
sys_usw_learning_aging_deinit(uint8 lchip);

extern int32
sys_usw_aging_get_aging_timer(uint8 lchip, uint8 domain_type, uint32 key_index, uint8* p_timer);

extern int32
sys_usw_learning_aging_dump_db(uint8 lchip, sal_file_t dump_db_fp,ctc_global_dump_db_t* p_dump_param);

#ifdef __cplusplus
}
#endif

#endif /* !_SYS_USW_LEARNING_AGING_H */

