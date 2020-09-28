
/**
  @file drv_api.h

  @date 2010-02-26

  @version v5.1

  The file implement driver acc IOCTL defines and macros
*/

#ifndef _DRV_API_H_
#define _DRV_API_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"
#include "usw/include/drv_tbl_macro.h"
#include "usw/include/drv_enum.h"
#include "usw/include/drv_error.h"
#include "usw/include/drv_app.h"
#include "usw/include/drv_ftm.h"
#include "usw/include/drv_ser.h"
#include "usw/include/drv_ser_db.h"

#define MAX_ENTRY_WORD 128    /**< define table/reg entry's max words */
#define MAX_ENTRY_BYTE 128   /**< define table/reg entry's max bytes */



/**********************************************************************************
              Define Typedef, define and Data Structure
***********************************************************************************/
#define DRV_IOC_READ          1U
#define DRV_IOC_WRITE         2U
#define DRV_IOC_REMOVE        3U
#define DRV_IOC_DIR_BITS      2
#define DRV_IOC_MEMID_BITS    13
#define DRV_IOC_FIELDID_BITS  16
#define DRV_ENTRY_FLAG        0x1FFF   /* when fieldid equals this value, it represent
                                          the operation is applied to the whole entry */
#define DRV_IOC_DIR_MASK     ((1 << DRV_IOC_DIR_BITS)-1)
#define DRV_IOC_MEMID_MASK   ((1 << DRV_IOC_MEMID_BITS)-1)
#define DRV_IOC_FIELDID_MASK ((1 << DRV_IOC_FIELDID_BITS)-1)
#define DRV_IOC_FIELDID_SHIFT 0
#define DRV_IOC_MEMID_SHIFT  (DRV_IOC_FIELDID_SHIFT + DRV_IOC_FIELDID_BITS)
#define DRV_IOC_DIR_SHIFT    (DRV_IOC_MEMID_SHIFT + DRV_IOC_MEMID_BITS)
#define DRV_IOC_OP(cmd)      (((cmd) >> DRV_IOC_DIR_SHIFT)&DRV_IOC_DIR_MASK)
#define DRV_IOC_MEMID(cmd)   (((cmd) >> DRV_IOC_MEMID_SHIFT)&DRV_IOC_MEMID_MASK)
#define DRV_IOC_FIELDID(cmd) (((cmd) >> DRV_IOC_FIELDID_SHIFT)&DRV_IOC_FIELDID_MASK)
#define DRV_IOC(dir, memid, fieldid) \
    (((dir) << DRV_IOC_DIR_SHIFT) | \
    ((memid) << DRV_IOC_MEMID_SHIFT) | \
    ((fieldid) << DRV_IOC_FIELDID_SHIFT))
#define DRV_IOCTL           drv_ioctl_api
#define DRV_IOR(memid, fieldid) DRV_IOC(DRV_IOC_READ, (memid), (fieldid))
#define DRV_IOW(memid, fieldid) DRV_IOC(DRV_IOC_WRITE, (memid), (fieldid))
#define DRV_IOD(memid, fieldid) DRV_IOC(DRV_IOC_REMOVE, (memid), (fieldid))

#define DRV_TCAM_TBL_REMOVE(lchip, tbl, index) \
    do\
    {\
        int retv = 0;\
        uint32 cmd = 0;  \
        cmd = DRV_IOD(tbl, DRV_ENTRY_FLAG);  \
        retv = FTM_TCAM_IOCTL(lchip, index, cmd, &cmd);\
        if (retv < 0)\
        {\
            return(retv); \
        }\
    }\
    while(0)

#define DRV_IOW_FIELD(lchip, memid, fieldid, value, ptr) \
    do\
    {\
        drv_set_field(lchip, memid, fieldid, ptr, value);\
    }\
    while(0)

#define DRV_IOR_FIELD(lchip, memid, fieldid, value, ptr) \
    do\
    {\
        drv_get_field(lchip, memid, fieldid, ptr, value);\
    }\
    while(0)

#define DRV_SET_FIELD_ADDR  drv_set_field
#define DRV_GET_FIELD       drv_get_field
#define DRV_SET_FIELD_VALUE(lchip, tbl_id, field_id, entry, value) \
    {                                                       \
        uint32 vl = value;                                  \
        drv_set_field(lchip, tbl_id, field_id, entry, &vl);        \
    }
#define DRV_SET_FIELD_V(lchip, tbl_id, field_id, entry, value) \
    {                                                       \
        uint32 vl = value;                                  \
        drv_set_field(lchip, tbl_id, field_id, entry, &vl);        \
    }
#define DRV_SET_FIELD_A(lchip, t,f,d,v)    drv_set_field(lchip, t,f,d,(uint32*)v)
#define DRV_GET_FIELD_V             drv_get_field_value
#define DRV_GET_FIELD_A(lchip, t,f,d,v)    drv_get_field(lchip, t,f,d,(uint32*)v)

#define DRV_SET_FLD(X, T, f, ...)         DRV_SET_FIELD_ ## X(lchip, T ##_t, T ## _ ## f,  ##__VA_ARGS__)
#define DRV_GET_FLD(X, T, f, ...)        DRV_GET_FIELD_ ## X(lchip, T ##_t, T ## _ ## f,  ##__VA_ARGS__)
#define DRV_FLD_IS_EXISIT(tbl_id, field_id) \
    ( DRV_E_NONE == drv_field_support_check(lchip, tbl_id, field_id) )

/* Tcam data mask storage structure */
struct tbl_entry_s
{
    uint32* data_entry;
    uint32* mask_entry;
};
typedef struct tbl_entry_s tbl_entry_t;

enum drv_work_platform_type_e
{
    HW_PLATFORM = 0,
    SW_SIM_PLATFORM = 1,
    MAX_WORK_PLATFORM = 2,
};
typedef enum drv_work_platform_type_e drv_work_platform_type_t;

enum drv_access_type_e
{
    DRV_PCI_ACCESS,
    DRV_I2C_ACCESS,

    DRV_MAX_ACCESS_TYPE
};
typedef enum drv_access_type_e drv_access_type_t;

/**
 @brief define the byte order
*/
enum host_type_e
{
    HOST_LE = 0,     /**< little edian */
    HOST_BE = 1,     /**< big edian */
};
typedef enum host_type_e host_type_t;

enum drv_table_property_e
{
    DRV_TABLE_PROP_TYPE,
    DRV_TABLE_PROP_HW_ADDR,
    DRV_TABLE_PROP_GET_NAME,
    DRV_TABLE_PROP_BITMAP,
    DRV_TABLE_PROP_FIELD_NUM,

    MAX_DRV_TABLE_PROP
};
typedef enum drv_table_property_e drv_table_property_t;

/* table extended property type */
enum drv_table_type_s
{
    DRV_TABLE_TYPE_NORMAL = 0,
    DRV_TABLE_TYPE_TCAM,
    DRV_TABLE_TYPE_TCAM_AD,
    DRV_TABLE_TYPE_TCAM_LPM_AD,
    DRV_TABLE_TYPE_TCAM_NAT_AD,
    DRV_TABLE_TYPE_LPM_TCAM_IP,
    DRV_TABLE_TYPE_LPM_TCAM_NAT,
    DRV_TABLE_TYPE_STATIC_TCAM_KEY,
    DRV_TABLE_TYPE_DESC,
    DRV_TABLE_TYPE_DBG,
    DRV_TABLE_TYPE_DYNAMIC,
    DRV_TABLE_TYPE_MIXED,
    DRV_TABLE_TYPE_INVALID,
};
typedef enum drv_table_type_s drv_table_type_t;




struct drv_io_callback_fun_s
{
    int32(*drv_sram_tbl_read)(uint8, tbls_id_t, uint32, uint32*);
    int32(*drv_sram_tbl_write)(uint8, tbls_id_t, uint32, uint32*);
    int32(*drv_tcam_tbl_read)(uint8, tbls_id_t, uint32, tbl_entry_t*);
    int32(*drv_tcam_tbl_write)(uint8, tbls_id_t, uint32, tbl_entry_t*);
    int32(*drv_tcam_tbl_remove)(uint8, tbls_id_t, uint32);
};
typedef struct drv_io_callback_fun_s drv_io_callback_fun_t;



enum drv_agent_cb_e
{
    DRV_AGENT_CB_SET_OAM_DEFECT,
    DRV_AGENT_CB_SET_PKT_RX,
    DRV_AGENT_CB_SET_PKT_TX,
    DRV_AGENT_CB_SET_DMA_RX,
    DRV_AGENT_CB_SET_DMA_DUMP,
    DRV_AGENT_CB_GET_LEARN,
    DRV_AGENT_CB_GET_AGING,
    DRV_AGENT_CB_GET_IPFIX,
    DRV_AGENT_CB_GET_MONITOR,
    DRV_AGENT_CB_GET_OAM_DEFECT,
    DRV_AGENT_CB_GET_DMA_DUMP,
    DRV_AGENT_CB_IPFIX_EXPORT,
    DRV_AGENT_CB_PKT_TX,
    DRV_AGENT_CB_PKT_RX,
    DRV_AGENT_CB_MAX

};

/**
 @brief define drv chip agent mode
*/
enum drv_chip_agent_mode_e
{
    DRV_CHIP_AGT_MODE_NONE = 0,
    DRV_CHIP_AGT_MODE_CLIENT,
    DRV_CHIP_AGT_MODE_SERVER,
    DRV_CHIP_AGT_MODE_MAX
};
typedef enum drv_chip_agent_mode_e drv_chip_agent_mode_t;

enum drv_chip_type_e
{
    DRV_DUET2,
    DRV_TSINGMA,
    DRV_TSINGMA2,
};
typedef enum drv_chip_type_e drv_chip_type_t;

#define DRV_IS_DUET2(lchip) drv_is_chip(lchip, DRV_DUET2)
#define DRV_IS_TSINGMA(lchip) drv_is_chip(lchip, DRV_TSINGMA)
#define DRV_IS_TM2(lchip) drv_is_chip(lchip, DRV_TSINGMA2)

extern int32  drv_field_support_check(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id);
extern uint8  drv_is_chip(uint8 lchip, drv_chip_type_t chip_type);

extern int32 drv_set_access_type(uint8 lchip, drv_access_type_t access_type);
extern int32 drv_get_gchip_id(uint8 lchip, uint8* gchip_id);
extern int32 drv_set_gchip_id(uint8 lchip, uint8 gchip_id);
extern int32 drv_usw_agent_register_cb(uint8 type, void* cb);
extern int32 drv_get_chip_agent_mode(void);
extern int32 drv_get_access_type(uint8 lchip, drv_access_type_t* p_access_type);
extern int32 drv_get_platform_type(uint8 lchip, drv_work_platform_type_t *plaform_type);
extern int32 drv_get_host_type (uint8 lchip);
extern int32 drv_get_table_property(uint8 lchip,uint8 type, tbls_id_t tbl_id, uint32 index, void* value);
extern int32 drv_get_field(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id, void* ds, uint32* value);
extern int32 drv_set_field(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id, void* ds, uint32 *value);
extern uint32 drv_get_field_value(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id,  void* ds);
extern int32 drv_install_api(uint8 lchip, drv_io_callback_fun_t* cb);
extern int32 drv_ioctl_api(uint8 lchip, int32 index, uint32 cmd, void* val);
extern int32 drv_acc_api(uint8 lchip, void* in, void* out);
extern int32 drv_set_warmboot_status(uint8 lchip, uint32 wb_status);
extern int32 drv_init(uint8 lchip, uint8 base);
extern int32 drv_deinit(uint8 lchip, uint8 base);

#ifdef __cplusplus
}
#endif
#endif /*end of _DRV_API_H*/
