
/**
  @file drv_acc.h

  @date 2010-02-26

  @version v5.1

  The file implement driver acc IOCTL defines and macros
*/

#ifndef _DRV_APP_H_
#define _DRV_APP_H_
#ifdef __cplusplus
extern "C" {
#endif





#define  _DRV_CPU_ACC_M_
#define  _DRV_FIB_ACC_M_
#define  _DRV_IPFIX_ACC_M_
#define HASH_KEY_LENGTH_MODE_SINGLE      0
#define HASH_KEY_LENGTH_MODE_DOUBLE      1
#define HASH_KEY_LENGTH_MODE_QUAD        2

#define FIB_HOST0_CAM_NUM          ( 32 )
#define FIB_HOST1_CAM_NUM          ( 32 )
#define FLOW_HASH_CAM_NUM          ( 32 )
#define USER_ID_HASH_CAM_NUM       ( 32 )
#define MPLS_HASH_CAM_NUM        ( 64 )
#define XC_OAM_HASH_CAM_NUM        ( 128 )
#define DRV_GET_ACC_TYPE()

enum drv_acc_hash_module_e
{
    DRV_ACC_HASH_MODULE_FDB,
    DRV_ACC_HASH_MODULE_FIB_HOST0,
    DRV_ACC_HASH_MODULE_FIB_HOST1,
    DRV_ACC_HASH_MODULE_EGRESS_XC,
    DRV_ACC_HASH_MODULE_FLOW,
    DRV_ACC_HASH_MODULE_USERID,
    DRV_ACC_HASH_MODULE_MAC_LIMIT,
    DRV_ACC_HASH_MODULE_IPFIX,
    DRV_ACC_HASH_MODULE_CID,
    DRV_ACC_HASH_MODULE_AGING,
    DRV_ACC_HASH_MODULE_FIB_HASH, /*TM*/
    DRV_ACC_HASH_MODULE_MPLS_HASH, /*TM*/
    DRV_ACC_HASH_MODULE_GEMPORT_HASH, /*TM*/


    DRV_ACC_HASH_MODULE_NUM,
};
typedef enum drv_acc_hash_module_e drv_acc_hash_module_t;

enum drv_acc_flag_e
{
    DRV_ACC_ENTRY_DYNAMIC,
    DRV_ACC_ENTRY_STATIC,
    DRV_ACC_UPDATE_MAC_LIMIT_CNT,             /* for port/vlan/dynamic mac limit*/
    DRV_ACC_STATIC_MAC_LIMIT_CNT,             /* static fdb do mac limit */
    DRV_ACC_AGING_EN,                         /* used for add fdb by key */
    DRV_ACC_OVERWRITE_EN,                     /* used for add fdb by key and mac limit*/
    DRV_ACC_MAC_LIMIT_NEGATIVE,               /* used for update mac limit count */
    DRV_ACC_MAC_LIMIT_STATUS,                 /* used for update mac limit status */
    DRV_ACC_USE_LOGIC_PORT,
    DRV_ACC_ENTRY_UCAST,                      /* used for dump ucast entry */
    DRV_ACC_ENTRY_MCAST,                      /* used for dump mcast entry */
    DRV_ACC_ENTRY_PENDING,                    /* used for dump pending entry */
    DRV_ACC_MAX_FLAG
};
typedef enum drv_acc_flag_e drv_acc_flag_t;

enum drv_acc_op_type_e
{
    DRV_ACC_OP_BY_INDEX,     /* used for acc add/del/lookup by index */
    DRV_ACC_OP_BY_KEY,       /* used for acc add/del/lookup by key */
    DRV_ACC_OP_BY_MAC,       /* used for dump/flush */
    DRV_ACC_OP_BY_PORT,      /* used for dump/flush */
    DRV_ACC_OP_BY_VLAN,      /* used for dump/flush */
    DRV_ACC_OP_BY_PORT_VLAN, /* used for dump/flush */
    DRV_ACC_OP_BY_PORT_MAC,  /* used for dump/flush */
    DRV_ACC_OP_BY_ALL,       /* used for dump/flush */

    DRV_ACC_FLAG_NUM
};
typedef enum drv_acc_op_type_e drv_acc_op_type_t;

enum drv_acc_type_e
{
    DRV_ACC_TYPE_ADD,               /* Add key action for all acc type */
    DRV_ACC_TYPE_DEL,               /* Del key action for all acc type */
    DRV_ACC_TYPE_LOOKUP,            /* Lookup key action for all acc type */
    DRV_ACC_TYPE_DUMP,              /* dump key action for FDB acc type */
    DRV_ACC_TYPE_FLUSH,             /* Flush key action for FDB acc type */
    DRV_ACC_TYPE_ALLOC,             /* used only for userid/xcoam/flowhash */
    DRV_ACC_TYPE_UPDATE,
    DRV_ACC_TYPE_CALC,              /* Calculate key index for all acc type */
    DRV_ACC_MAX_TYPE
};
typedef enum drv_acc_type_e drv_acc_type_t;

struct drv_acc_in_s
{
    uint32      tbl_id;          /* acc operation table id */
    uint8       type;            /* refer to drv_acc_type_t */
    uint8       op_type;         /* refer to drv_acc_op_type_t */
    uint8       dir;             /* for ipfix ingress/egres*/
    uint8       level;
    uint32      flag;           /* refer to drv_acc_flag_t */
    uint32      index;          /* for op_type=DRV_ACC_OP_BY_INDEX means key index, for dump means start index */
    uint32      query_end_index;/* dump end key index and this key is excluded */
    uint32      query_cnt;      /* dump using, max number for one query */
    uint16      gport;
    uint8       timer_index;    /* used for add fdb by key */
    uint8       offset;         /* used for cid acc, only add by index will use, indicate bucket offset*/
    uint32      limit_cnt;      /* increase/decrease number or new count,
                                   if DRV_ACC_MAC_LIMIT_STATUS is set, means status: discard or not discard*/

    void*       data;
};
typedef struct drv_acc_in_s drv_acc_in_t;

struct drv_acc_out_s
{
    uint32      key_index;      /* by index */
    uint32      ad_index;       /* only used for lookup by key */

    uint8       is_full;        /* for add by key and mac limit means reach mac limit threshold,
                                   for dump means dma full*/
    uint8       is_conflict;
    uint8       is_hit;
    uint8       is_pending;

    uint8       is_end;
    uint8       is_left;        /*only used for cid acc*/
    uint8       offset;         /*only used for cid acc*/
    uint8       rsv;

    uint32      query_cnt;
    uint32      data[40]; /*10*4*/
};
typedef struct drv_acc_out_s drv_acc_out_t;

#define __NEW_ACC_END__

/* add for fib acc start */

typedef uint32 drv_fib_acc_fib_data_t[6];
typedef uint32 drv_fib_acc_rw_data_t[20];
typedef uint32 drv_ipfix_acc_fib_data_t[6];
typedef uint32 drv_ipfix_acc_rw_data_t[20];
typedef uint32 max_ds_t[32];

#define FIB_ACC_LOCK(lchip)    \
  if(p_drv_master[lchip]->fib_acc_mutex) {sal_mutex_lock(p_drv_master[lchip]->fib_acc_mutex);}
#define FIB_ACC_UNLOCK(lchip)  \
  if(p_drv_master[lchip]->fib_acc_mutex) {sal_mutex_unlock(p_drv_master[lchip]->fib_acc_mutex);}

extern int32
drv_usw_acc(uint8 lchip, void* in, void* out);

extern int32
drv_usw_set_dynamic_ram_couple_mode(uint8 lchip, uint32 couple_mode);

extern int32
drv_usw_ftm_get_entry_num(uint8 lchip, uint32 table_id, uint32* entry_num);

extern int32
drv_usw_ftm_get_entry_size(uint8 lchip, uint32 table_id, uint32* entry_size);

extern int32
drv_usw_acc_get_hash_module_from_tblid(uint8 lchip, uint32 tbl_id, uint8* p_module);

extern int32
drv_usw_get_cam_info(uint8 lchip, uint8 hash_module, uint32* tbl_id, uint8* num);

#ifdef __cplusplus
}
#endif
#endif /*end of _DRV_ACC_H*/
