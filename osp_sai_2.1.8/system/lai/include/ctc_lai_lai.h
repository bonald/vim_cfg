#if !defined (__LAI_LAI_H_)
#define __LAI_LAI_H_

#include "laiinc.h"


#define QUOTEME_(x) #x                        /* add "" to x */
#define QUOTEME(x)  QUOTEME_(x)


#define SAI_PRINT_CON(fmt, args...)                      \
{                                                            \
    FILE * fp_console = NULL;                                \
    fp_console = fopen("/dev/console", "w+");                \
    fprintf(fp_console, fmt"\n", ##args);                    \
    fclose(fp_console);                                      \
}

#define LAI_LOG_WRN(fmt, arg ...) LCM_LOG_WARN(fmt, ## arg)
#define LAI_LOG_ERR(fmt, arg ...) LCM_LOG_ERR(fmt, ## arg)
#define LAI_LOG_CRI(fmt, arg ...) LCM_LOG_CRIT(fmt, ## arg)

typedef enum{
    UNINITIALIZED = 0,
    INITIALIZED,
}lai_initialize_status_t;


typedef struct _ctc_lai_api_reg_info_ ctc_lai_api_reg_info_t;

typedef lai_status_t (*lai_init_mode_fn)(ctc_lai_api_reg_info_t*, void*);
typedef lai_status_t (*lai_exit_mode_fn)(ctc_lai_api_reg_info_t*, void*);

struct _ctc_lai_api_reg_info_
{
    lai_api_t       id;
    lai_init_mode_fn    init_func;
    lai_exit_mode_fn    exit_func;
    void*           api_method_table;
    lai_initialize_status_t init_status;
};

typedef struct ctc_lai_lai_sys_info_
{
    ctc_lai_api_reg_info_t* api_queue[LAI_API_MAX];          /* list -> ctc_lai_api_reg_info_t */
    lai_initialize_status_t   lai_init_status;
    lai_card_notification_t   lai_card_notification_table;
}ctc_lai_lai_sys_info_t;

ctc_lai_lai_sys_info_t*
ctc_lai_get_sys_info(void);


lai_status_t
api_reg_register_fn(
    _In_  ctc_lai_api_reg_info_t*);

lai_status_t
api_reg_unregister_fn(
    _In_  ctc_lai_api_reg_info_t*);


lai_status_t
lai_api_reg_register_fn(
    _In_  ctc_lai_api_reg_info_t    *preg);

#endif

