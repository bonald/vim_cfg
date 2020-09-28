#if !defined (__SAI_SAI_H_)
#define __SAI_SAI_H_

#include "sai.h"
#include "saiswitch.h"

#define QUOTEME_(x) #x                        /* add "" to x */
#define QUOTEME(x)  QUOTEME_(x)

typedef enum{
    UNINITIALIZED = 0,
    INITIALIZED,
}sai_initialize_status_t;

typedef struct ctc_sai_api_reg_info_ ctc_sai_api_reg_info_t;

typedef struct _timestamp_ether_s {
    uint32_t    enable;
    uint16_t    type;
    sai_mac_t   dmac;
    sai_mac_t   smac;
}timestamp_ether_t;

typedef struct ctc_sai_sai_sys_info_
{
    ctc_linklist_t*             api_queue;          /* list -> ctc_sai_api_reg_info_t */
    sai_initialize_status_t     sai_init_status;
    sai_switch_notification_t   sai_switch_notification_table;
    uint8_t                         log_level[SAI_API_MAX];  /* sai_api_t count */

    timestamp_ether_t               timestamp;
}ctc_sai_sai_sys_info_t;

typedef sai_status_t (*init_mode_fn)(ctc_sai_api_reg_info_t*, void*);
typedef sai_status_t (*exit_mode_fn)(ctc_sai_api_reg_info_t*, void*);

struct ctc_sai_api_reg_info_
{
    sai_api_t       id;
    init_mode_fn    init_func;
    exit_mode_fn    exit_func;
    void*           api_method_table;
    void*           private_data;
    sai_initialize_status_t     init_status;
};

ctc_sai_sai_sys_info_t*
ctc_sai_get_sys_info(void);

sai_status_t
api_reg_register_fn(
    _In_  ctc_sai_api_reg_info_t*);

sai_status_t
api_reg_unregister_fn(
    _In_  ctc_sai_api_reg_info_t*);

typedef sai_status_t (*api_reg_traverse_fn)(ctc_sai_api_reg_info_t*,void*);

sai_status_t
api_reg_traverse(
    _In_  api_reg_traverse_fn,
    _In_  void *);

#endif
