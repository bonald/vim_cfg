/**
 @file sys_usw_linkagg.h

 @date 2009-10-19

 @version v2.0

 The file define APIs and types use in sys layer
*/
#ifndef _SYS_USW_LINKAGG_H
#define _SYS_USW_LINKAGG_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_const.h"
#include "ctc_vector.h"
#include "ctc_common.h"

#include "ctc_linkagg.h"
/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
/* member num in all linkAgg groups */

#define SYS_DLB_DOWN_FRE_RATE  1
#define SYS_DLB_TS_THRES 200
#define SYS_DLB_MAX_PTR 1999
#define SYS_DLB_UPDATE_THRES 10000
#define SYS_DLB_TIMER_INTERVAL 3

/**
 @brief linkagg member number
*/
enum sys_linkagg_mem_num_e
{
    SYS_LINKAGG_MEM_NUM_1 = 24,     /* 24  members in linkAgg group */
    SYS_LINKAGG_MEM_NUM_2 = 32,     /* 32 members in linkAgg group */
    SYS_LINKAGG_MEM_NUM_3 = 64,     /* 64 members in linkAgg group */
    SYS_LINKAGG_MEM_NUM_4 = 128     /* 128 members in linkAgg group */
};
typedef enum sys_linkagg_mem_num_e sys_linkagg_mem_num_t;

/* member port info */
struct sys_linkagg_port_s
{
    uint8 valid;        /* 1: member port is in linkAgg group */
    uint32 gport;       /* member port */
};
typedef struct sys_linkagg_port_s sys_linkagg_port_t;

/* linkAgg group info */
struct sys_linkagg_s
{
    uint16  tid;              /* linkAggId */
    uint8  mode;             /* ctc_linkagg_group_mode_t */
    uint16 member_base;

    uint16  flag;
    uint16  max_member_num;
    uint16  real_member_num;  /*for static failover, real num MUST read table*/
    uint8   ref_cnt;          /*for channel linkagg, bpe cb cascade*/
};
typedef struct sys_linkagg_s sys_linkagg_t;

struct sys_linkagg_master_s
{
    sal_mutex_t* p_linkagg_mutex;
    uint8 bind_gport_disable;                /** If set indicate the gport won't be configured automatically when the member port is added. */
    uint8 linkagg_mode;
    uint16 linkagg_num;                /* linkagg group num */
    uint8 chanagg_grp_num;           /* num of channel agg group*/
    uint8 chanagg_mem_per_grp;   /* num of member in a channel agg group*/
    uint8 opf_type;
	uint8 rsv;
    ctc_vector_t* group_vector;
    ctc_vector_t* chan_group;
    uint32 rr_profile_bmp;
};
typedef struct sys_linkagg_master_s sys_linkagg_master_t;

#define SYS_LINKAGG_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(linkagg, linkagg, LINKAGG_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0);
/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
extern int32
sys_usw_linkagg_init(uint8 lchip, void* linkagg_global_cfg);

extern int32
sys_usw_linkagg_deinit(uint8 lchip);

extern int32
sys_usw_linkagg_create(uint8 lchip, ctc_linkagg_group_t* p_linkagg_grp);

extern int32
sys_usw_linkagg_destroy(uint8 lchip, uint8 tid);

extern int32
sys_usw_linkagg_set_property(uint8 lchip, uint8 tid, ctc_linkagg_property_t linkagg_prop, uint32 value);

extern int32
sys_usw_linkagg_get_property(uint8 lchip, uint8 tid, ctc_linkagg_property_t linkagg_prop, uint32* p_value);

extern int32
sys_usw_linkagg_add_port(uint8 lchip, uint8 tid, uint32 gport);

extern int32
sys_usw_linkagg_remove_port(uint8 lchip, uint8 tid, uint32 gport);

extern int32
sys_usw_linkagg_get_1st_local_port(uint8 lchip, uint8 tid, uint32* p_gport, uint16* local_cnt);

extern int32
sys_usw_linkagg_show_ports(uint8 lchip, uint8 tid, uint32* p_gports, uint16* cnt);

extern int32
sys_usw_linkagg_set_psc(uint8 lchip, ctc_linkagg_psc_t* p_psc);

extern int32
sys_usw_linkagg_get_psc(uint8 lchip, ctc_linkagg_psc_t* p_psc);

extern int32
sys_usw_linkagg_show_all_member(uint8 lchip, uint8 tid);

extern int32
sys_usw_linkagg_get_max_mem_num(uint8 lchip, uint16* max_num);

extern int32
sys_usw_linkagg_replace_ports(uint8 lchip, uint8 tid, uint32* gport, uint16 mem_num);


#ifdef __cplusplus
}
#endif

#endif

