#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_npm.c

 @date 2016-04-20

 @version v1.0

 The file apply APIs to config npm
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_error.h"
#include "ctc_npm.h"

#include "sys_usw_npm.h"
#include "sys_usw_common.h"

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/

/**
 @brief  Set npm parameter info

 @param[in] lchip    local chip id

 @param[in] p_cfg    npm parameter include packet format and generator parameter

 @remark This function set the npm parameter.

 @return CTC_E_XXX

*/
int32
ctc_usw_npm_set_config(uint8 lchip_id, ctc_npm_cfg_t* p_cfg)
{
    uint8 lchip = lchip_id;
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_NPM);
    LCHIP_CHECK(lchip_id);

    CTC_ERROR_RETURN(sys_usw_npm_set_config(lchip_id, p_cfg));

    return CTC_E_NONE;
}

/**
 @brief  Set npm transmit enable

 @param[in] lchip    local chip id

 @param[in] session_id    npm session id

 @param[in] enable   enable or disable transmit

 @remark This function set packet transmit enable or disable.

 @return CTC_E_XXX

*/
int32
ctc_usw_npm_set_transmit_en(uint8 lchip_id, uint8 session_id, uint8 enable)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_NPM);
    LCHIP_CHECK(lchip_id);

    CTC_ERROR_RETURN(sys_usw_npm_set_transmit_en(lchip_id, session_id, enable));

    return CTC_E_NONE;
}
/**
 @brief  Get npm stats

 @param[in] lchip    local chip id

 @param[in] session_id    npm session id

 @param[out] p_stats   npm stats

 @remark This function get stats of session npm.

 @return CTC_E_XXX

*/
int32
ctc_usw_npm_get_stats(uint8 lchip_id, uint8 session_id, ctc_npm_stats_t* p_stats)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_NPM);
    LCHIP_CHECK(lchip_id);

    CTC_ERROR_RETURN(sys_usw_npm_get_stats(lchip_id, session_id, p_stats));

    return CTC_E_NONE;
}

/**
 @brief  Get npm stats

 @param[in] lchip    local chip id

 @param[in] session_id    npm session id

 @param[out] p_stats   npm stats

 @remark This function get stats of session npm.

 @return CTC_E_XXX

*/
int32
ctc_usw_npm_clear_stats(uint8 lchip_id, uint8 session_id)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_NPM);
    LCHIP_CHECK(lchip_id);

    CTC_ERROR_RETURN(sys_usw_npm_clear_stats(lchip_id, session_id));

    return CTC_E_NONE;
}


/**
 @brief  Set global config of npm

 @param[in] lchip    local chip id

 @param[in] p_npm    global parameter

 @remark This function set the global config of npm.

 @return CTC_E_XXX

*/
int32
ctc_usw_npm_set_global_config(uint8 lchip_id, ctc_npm_global_cfg_t* p_npm)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_NPM);
    LCHIP_CHECK(lchip_id);

    CTC_ERROR_RETURN(sys_usw_npm_set_global_config(lchip_id, p_npm));

    return CTC_E_NONE;
}

/**
 @brief  Get global config of npm

 @param[in] lchip    local chip id

 @param[in] p_npm    global parameter

 @remark This function get the global config of npm.

 @return CTC_E_XXX

*/
int32
ctc_usw_npm_get_global_config(uint8 lchip_id, ctc_npm_global_cfg_t* p_npm)
{
    uint8 lchip = lchip_id;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_NPM);
    LCHIP_CHECK(lchip_id);

    CTC_ERROR_RETURN(sys_usw_npm_get_global_config(lchip_id, p_npm));

    return CTC_E_NONE;
}

/**
 @brief Initialize NPM module
*/
int32
ctc_usw_npm_init(uint8 lchip, void* npm_global_cfg)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_npm_init(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief deinit for NPM module
*/
int32
ctc_usw_npm_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_npm_deinit(lchip));
    }

    return CTC_E_NONE;
}

#endif

