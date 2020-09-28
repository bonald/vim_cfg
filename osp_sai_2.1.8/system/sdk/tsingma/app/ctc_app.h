/**
 @file ctc_app.h

 @date 2010-3-2

 @version v2.0

 This file define the isr APIs
*/

#ifndef _CTC_SMP_COMMON_H
#define _CTC_SMP_COMMON_H
#ifdef __cplusplus
extern "C" {
#endif

#define CTC_APP_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(app, app, APP_SMP, level, FMT, ##__VA_ARGS__); \
    } while (0)


struct ctc_app_master_s
{
    uint8 ctcs_api_en;
    uint8 lchip_num;
    uint8 wb_enable;
    uint8 wb_reloading;
    uint8 gchip_id[CTC_MAX_LOCAL_CHIP_NUM];
    char* mem_profile;
};
typedef struct ctc_app_master_s ctc_app_master_t;

extern ctc_app_master_t g_ctc_app_master;

int32 ctc_app_get_lchip_id(uint8 gchip, uint8* lchip);

#ifdef __cplusplus
}
#endif

#endif
