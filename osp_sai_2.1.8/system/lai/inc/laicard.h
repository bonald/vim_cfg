/****************************************************************************
* $Id$
*  Centec interface information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-11-25
* Reason        : First Create.
****************************************************************************/

#if !defined (__LAICARD_H_)
#define __LAICARD_H_

#include "laitype.h"
#include "laisynce.h"


typedef enum _lai_card_event_t
{
    /** Create a new card*/
    LAI_CARD_EVENT_ADD,

    /** Delete/Invalidate an existing card */
    LAI_CARD_EVENT_DELETE,

} lai_card_event_t;


/**
 * Routine Description:
 *   @brief card event notification
 *
 * Arguments:
 *    @param[in] info- card information
 *    @param[in] data  - array of port events

 * Return Values:
 *    None
 */
typedef void (*lai_card_event_notification_fn)(
    _In_ lai_card_event_t  event,
    _In_ lai_card_info_t* card
    );

/**
 * Routine Description:
 *    @brief get card information
 *
 * Arguments:
 *    @param[in] oid - card oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_card_info_get_fn)(
    _In_ lai_object_id_t oid,
    _Out_ lai_card_info_t* card
    );

/**
 * Routine Description:
 *    @brief set oem information
 *
 * Arguments:
 *    @param[in] oid - card oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_oem_info_set_fn)(
    _In_ lai_object_id_t oid,
    _In_ lai_oem_info_t* oem_info
    );

/**
 * Routine Description:
 *    @brief set oem information
 *
 * Arguments:
 *    @param[in] oid - card oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_sw_version_info_set_fn)(
    _In_ lai_object_id_t oid,
    _In_ lai_sw_version_info_t* sw_ver_info
    );

/**
 * Routine Description:
 *    @brief set card temperature threshold for alarm
 *
 * Arguments:
 *    @param[in] oid - card oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_tmpr_info_set_fn)(
    _In_ lai_object_id_t oid,
    _In_ int32_t low, 
    _In_ int32_t high,
    _In_ int32_t critical
    );


/**
 * Routine Description:
 *    @brief set boot command line to card, for reboot
 *
 * Arguments:
 *    @param[in] oid - card oid, cmd, boot command
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_bootcmd_set_fn)(
    _In_ lai_object_id_t oid,
    _In_ char* cmd
    );

/**
 * Routine Description:
 *    @brief set stm mode to card, for stm profile mode
 *
 * Arguments:
 *    @param[in] oid - card oid, mode, profile mode
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_stmmode_set_fn)(
    _In_ lai_object_id_t oid,
    _In_ uint32_t mode
    );

/**
 * Routine Description:
 *    @brief set ifname info
 *
 * Arguments:
 *    @param[in] oid - card oid
 *    @param[in] ifname - interface name information
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_ifname_info_set_fn)(
    _In_ lai_object_id_t oid,
    _In_ lai_portname_info_t* ifname
    );



/**
 * Routine Description:
 *    @brief get reboot info from card 
 *
 * Arguments:
 *    @param[in] oid - card oid
 *
 * Arguments:
 *    @param[out] reboot_info - reboot information
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_reboot_info_get_fn)(
    _In_ lai_object_id_t oid,
    _In_ lai_reboot_info_t* reboot_info
    );

/**
 * Routine Description:
 *    @brief reset reboot info
 *
 * Arguments:
 *    @param[in] oid - card oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_reboot_info_reset_fn)(
    _In_ lai_object_id_t oid
    );


/**
 * Routine Description:
 *    @brief update epld version
 *
 * Arguments:
 *    @param[in] oid - card oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_update_epld_fn)(
    _In_ lai_object_id_t oid,
    _In_ char* epld_version
    );

/**
 * Routine Description:
 *    @brief update bootrom version
 *
 * Arguments:
 *    @param[in] oid - card oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_update_bootrom_fn)(
    _In_ lai_object_id_t oid,
    _In_ char* bootrom_version
    );

/**
 * Routine Description:
 *    @brief set reboot card
 *
 * Arguments:
 *    @param[in] oid - card oid
 *    @param[in] cmd - reboot command
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
 typedef lai_status_t (*lai_set_bootcmd_fn)(
    _In_ lai_object_id_t oid,
    _In_ char* cmd
    );

/**
 * Routine Description:
 *    @brief update phy firmware version
 *
 * Arguments:
 *    @param[in] oid - card oid
 *    @param[in] phy_port_idx - phy port index
 *    @param[in] phy_firmware_version - phy firmware
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_update_phy_firmware_fn)(
    _In_ lai_object_id_t oid,
    _In_ int32_t phy_port_idx,
    _In_ char* phy_firmware_version
    );

/**
 * Routine Description:
 *    @brief set fan speed for adjust auto
 *
 * Arguments:
 *    @param[in] oid - card oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_fan_speed_set_fn)(
    _In_ lai_object_id_t oid,
    _In_ int32_t bottom, 
    _In_ int32_t low,
    _In_ int32_t high,
    _In_ int32_t full
    );

/**
 * Routine Description:
 *    @brief set poe global enable
 *
 * Arguments:
 *    @param[in] oid - card oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_poe_enable_set_fn)(
    _In_ lai_object_id_t oid,
    _In_ char enable
    );

/**
 * Routine Description:
 *    @brief set poe global power limit
 *
 * Arguments:
 *    @param[in] oid - card oid
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t (*lai_poe_power_max_set_fn)(
    _In_ lai_object_id_t oid,
    _In_ uint32_t power_max
    );

/**
 * Routine Description:
 *    @brief init card
 *
 * Arguments:
 *    @param[in] 
 *
 * Return Values:
 *    @return LAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
typedef lai_status_t(*lai_card_init)();

/**
 * @brief Card methods table retrieved with lai_api_query()
 */
typedef struct _lai_card_api_t
{
    lai_card_info_get_fn             get_card_info;
    lai_oem_info_set_fn              set_oem_info;
    lai_tmpr_info_set_fn             set_tmpr_info;
    lai_bootcmd_set_fn               set_bootcmd;
    lai_stmmode_set_fn               set_stmmode;
    lai_ifname_info_set_fn           set_ifname_info;
    lai_reboot_info_get_fn           get_reboot_info;
    lai_reboot_info_reset_fn         reset_reboot_info;
    lai_update_epld_fn               update_epld;
    lai_update_bootrom_fn            update_bootrom;
    lai_set_bootcmd_fn               reboot;
    lai_update_phy_firmware_fn       update_phy_firmware;
    lai_fan_speed_set_fn             set_fan_speed;
    lai_poe_enable_set_fn            set_poe_enable;
    lai_poe_power_max_set_fn         set_poe_power_max; 
    lai_sw_version_info_set_fn       set_sw_version_info;
    lai_card_init                    init;
} lai_card_api_t;

typedef struct _lai_card_notification_t
{
    lai_card_event_notification_fn          on_card_event;
    lai_fiber_event_notification_fn         on_fiber_event;
    lai_port_event_notification_fn          on_port_event;
    lai_fan_event_notification_fn           on_fan_event;
    lai_psu_event_notification_fn           on_psu_event;
    lai_thermal_event_notification_fn       on_thermal_event;
    lai_synce_dpll_state_notification_fn    on_synce_dpll_state_event;
    lai_vcm_event_notification_fn           on_vcm_event;
    lai_poe_event_notification_fn           on_poe_event;
} lai_card_notification_t;

#endif // __LAICARD_H_




