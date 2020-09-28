#if (FEATURE_MODE == 0)
/**
    @file ctc_usw_wlan.h

    @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

    @date 2017-10-19

    @version v2.0

With the popularity of wireless devices, centralized WLAN management and access security become more important.
The centralized controller is known as an Access controller. The access point associated with it are known as Wireless Termination Point(WTP).
The AC may be a layer2 or a layer3 device and is often used to control/configure multiple WTPs as well as mange the mobility of the wireless station.
The AC also serves a point of aggregation in the data plane.
Control And Provisioning of wireless Access Point(CAPWAP) is a protocol of The Internet Engineer Task Force(TETF) .
It defines UDP tunnels to standardize the contol and data plane communication between the WTP and the AC.Duet2 support CAPWAP tunnel encapsulation and decapsulation,
dtls encryption and decryption along with the associated fragmentation and reassembly of packets. It also supports mobility management for wireless station.
For the station roams between AC(HA-FA),traffic for the roamed-in station must be redirected to its Home AC through CAPWAP tunnels.
This module contains the API designed to support implementation of WLAN. It is including function as follows:
\p

\d  CAPWAP tunnel encapsulation and decapsulation

\d  CAPWAP control and data packets dtls encrypt/decrypt

\d  802.3 translate 802.11, 802.11 translate 802.3

\d  CAPWAP frag and reseamble

\d  UDP-lite

\d  Multicast

\d  802.11Qos, not support 802.11 frag and reseamble

*/


#ifndef _CTC_USW_WLAN_H
#define _CTC_USW_WLAN_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_wlan.h"

/**
 @brief Init wlan module

 @param[in] lchip    local chip id

 @param[in] p_glb_param  point to init paramete

 @remark[D2.TM]
  init wlan module
 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_init(uint8 lchip, ctc_wlan_global_cfg_t* p_glb_param);

/**
 @brief deinit wlan module

 @param[in] lchip    local chip id

 @remark[D2.TM]
  deinit wlan module
 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_deinit(uint8 lchip);

/**
 @brief add wlan tunnel decapsulation function

 @param[in] lchip    local chip id

 @param[in] p_tunnel_param  the property of decap

 @remark[D2.TM]  terminating the WLAN tunnel, default is ipsa,
         ipda, l4_port should be set.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_add_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param);

/**
 @brief remove wlan tunnel decapsulation function

 @param[in] lchip    local chip id

 @param[in] p_tunnel_param  the property of decap


 @remark[D2.TM]  remove the specific overlay tunnel


 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_remove_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param);

/**
 @brief update wlan tunnel decapsulation function

 @param[in] lchip    local chip id

 @param[in] p_tunnel_param  the property of decap


 @remark[D2.TM]  update the specific overlay tunnel


 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_update_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param);

/**
 @brief add the wlan client function

 @param[in] lchip    local chip id

 @param[in] p_client_param the client property, refer to ctc_wlan_client_t

 @remark[D2.TM] add a client for a specific tunnel.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_add_client(uint8 lchip, ctc_wlan_client_t* p_client_param);

/**
 @brief remove the wlan client function

 @param[in] lchip    local chip id

 @param[in] p_client_param the client property, refer to ctc_wlan_client_t

 @remark[D2.TM] remove the specific client

 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_remove_client(uint8 lchip, ctc_wlan_client_t* p_client_param);

/**
 @brief upate the wlan client function

 @param[in] lchip    local chip id

 @param[in] p_client_param  the client property, refer to ctc_wlan_client_t

 @remark[D2.TM]  update the specific client

 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_update_client(uint8 lchip, ctc_wlan_client_t* p_client_param);

/**
 @brief set crypt key for decrypt or encrypt

 @param[in] lchip    local chip id

 @param[in] p_crypt_param the crypt info, refer to ctc_wlan_key_t

 @remark[D2.TM] set the specific authentication key info for decrypt or encrypt

 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_set_crypt(uint8 lchip, ctc_wlan_crypt_t* p_crypt_param);

/**
 @brief get the specific crypt info

 @param[in] lchip    local chip id

 @param[in] p_crypt_param the key info, refer to ctc_wlan_key_t

 @remark[D2.TM] get the specific authentication key info

 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_get_crypt(uint8 lchip, ctc_wlan_crypt_t* p_crypt_param);

/**
 @brief set the wlan global config function

 @param[in] lchip    local chip id

 @param[in] p_glb_param the global config, refer to ctc_wlan_global_cfg_t

 @remark[D2.TM] set the global wlan config

 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_set_global_cfg(uint8 lchip, ctc_wlan_global_cfg_t* p_glb_param);

/**
 @brief get the wlan global config function

 @param[in] lchip    local chip id

 @param[in] p_glb_param the global config, refer to ctc_wlan_global_cfg

 @remark[D2.TM] get the global wlan config

 @return CTC_E_XXX

*/
extern int32
ctc_usw_wlan_get_global_cfg(uint8 lchip, ctc_wlan_global_cfg_t* p_glb_param);
#ifdef __cplusplus
}
#endif

#endif /*end of _CTC_USW_WLAN_H*/

#endif

