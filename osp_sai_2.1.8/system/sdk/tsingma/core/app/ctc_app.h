/**
 @file ctc_app.h

 @date 2017-6-20

 @version v2.0

 This file define the app common define
*/

#ifndef _APP_API_COMMON_H
#define _APP_API_COMMON_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal_types.h"
#include "ctc_const.h"
#include "ctc_debug.h"

#define CTC_APP_API_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(app, api, APP_SMP, level, FMT, ##__VA_ARGS__); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif
