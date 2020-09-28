/**
 @file dkit_error.h

 @date 2016-05-17

 @version v1.0

 The file contains dkit error code definition
*/

#ifndef _CTC_DKIT_ERROR_H
#define _CTC_DKIT_ERROR_H

/**********************************************************************************
              Define Typedef, define and Data Structure
***********************************************************************************/
enum ctc_dkit_err_e
{
    DKIT_E_NONE = 0,

    DKIT_E_INVALID_PTR = -40000,    /**< Invalid pointer */
    DKIT_E_INVALID_PORT,            /**< Invalid port */
    DKIT_E_INVALID_PARAM,           /**< Invalid parameter */
    DKIT_E_BADID,                   /**< Invalid identifier */
    DKIT_E_INVALID_CHIP_ID,         /**< Invalid chip */
    DKIT_E_INVALID_CONFIG,          /**< Invalid configuration */
    DKIT_E_EXIST,                   /**< Entry exists */
    DKIT_E_NOT_EXIST,               /**< Entry not found */
    DKIT_E_NOT_READY,               /**< Not ready to configuration */
    DKIT_E_IN_USE,                  /**< Already in used */
    DKIT_E_NOT_SUPPORT,             /**< API or some feature is not supported */
    DKIT_E_NO_RESOURCE,             /**< No resource in ASIC */
    DKIT_E_PROFILE_NO_RESOURCE,     /**< Profile no resource in ASIC */
    DKIT_E_NO_MEMORY,               /**< No memory */
    DKIT_E_HASH_CONFLICT,           /**< Hash Conflict */
    DKIT_E_NOT_INIT,                /**< Feature not initialized */
    DKIT_E_INIT_FAIL,               /**< Feature initialize fail */
    DKIT_E_DMA,                     /**< DMA error */
    DKIT_E_HW_TIME_OUT,             /**< HW operation timed out */
    DKIT_E_HW_BUSY,                 /**< HW busy */
    DKIT_E_HW_INVALID_INDEX,        /**< Exceed max table index */
    DKIT_E_HW_NOT_LOCK,             /**< HW not lock */
    DKIT_E_HW_FAIL,                 /**< HW operation fail */
    DKIT_E_VERSION_MISMATCH,        /**< Version mismatch*/
    DKIT_E_PARAM_CONFLICT,          /**< Parameter Conflict*/

    DKIT_E_END = -35000
};
typedef enum ctc_dkit_err_e ctc_dkit_err_t;

extern bool dkits_debug;
#define CTC_DKIT_ERROR_RETURN(op) \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            if (dkits_debug) \
            sal_printf("Error Happened!! Fun:%s()  Line:%d ret:%d\n",__FUNCTION__,__LINE__, rv); \
            return (rv); \
        } \
    }

#define CTC_DKIT_ERROR_GOTO(op, ret, label) \
    { \
        if ((ret = (op)) < 0) \
        { \
            goto label; \
        } \
    }

#define CTC_DKIT_API_ERROR_RETURN(api, lchip, ...) \
{ \
    int32 ret = DKIT_E_NOT_SUPPORT; \
    ret = api? api(lchip, ##__VA_ARGS__) : DKIT_E_NOT_SUPPORT; \
    return ret;\
}

/**********************************************************************************
                      Define API function interfaces
***********************************************************************************/

#endif /*end of _CTC_DKIT_ERROR_H*/

