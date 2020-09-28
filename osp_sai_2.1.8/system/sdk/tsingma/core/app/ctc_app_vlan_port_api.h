/**
 @file ctc_app_vlan_port_api.h

 @date 2017-07-12

 @version v5.0

 The file defines vlan port api
*/
#ifndef _CTC_APP_VLAN_PORT_API_H_
#define _CTC_APP_VLAN_PORT_API_H_
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"
#include "common/include/ctc_mix.h"
#include "common/include/ctc_error.h"

#include "api/include/ctc_api.h"
#include "api/include/ctcs_api.h"


/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/*virtual device max num*/
#define CTC_APP_VLAN_PORT_MAX_VDEV_NUM   2

enum ctc_app_gem_port_update_type_s {
    CTC_APP_GEM_PORT_UPDATE_ISOLATE,             /**< [GB.D2] Isolate between gem port */
    CTC_APP_GEM_PORT_UPDATE_BIND_VLAN_PORT,      /**< [GB.D2.TM] Bind   vlan port*/
    CTC_APP_GEM_PORT_UPDATE_UNBIND_VLAN_PORT,    /**< [GB.D2.TM] Unbind vlan port */
    CTC_APP_GEM_PORT_UPDATE_IGS_POLICER,         /**< [GB.D2.TM] ingress Policer */
    CTC_APP_GEM_PORT_UPDATE_EGS_POLICER,         /**< [GB.D2.TM] egress Policer */
    CTC_APP_GEM_PORT_UPDATE_PASS_THROUGH_EN,     /**< [GB.D2.TM] Pass through enable */
    CTC_APP_GEM_PORT_UPDATE_IGS_STATS,           /**< [D2] Ingress stats */
    CTC_APP_GEM_PORT_UPDATE_EGS_STATS,           /**< [D2] Egress stats */
    CTC_APP_GEM_PORT_UPDATE_STATION_MOVE_ACTION, /**< [D2] Station move action*/
    CTC_APP_GEM_PORT_UPDATE_MAX
};
typedef enum ctc_app_gem_port_update_type_s ctc_app_gem_port_update_type_t;

enum ctc_app_vlan_port_update_type_s {
    CTC_APP_VLAN_PORT_UPDATE_NONE,
    CTC_APP_VLAN_PORT_UPDATE_IGS_POLICER,         /**< [GB.D2.TM] ingress Policer */
    CTC_APP_VLAN_PORT_UPDATE_EGS_POLICER,         /**< [GB.D2.TM] egress Policer */
    CTC_APP_VLAN_PORT_UPDATE_IGS_STATS,           /**< [D2] ingress Stats */
    CTC_APP_VLAN_PORT_UPDATE_EGS_STATS,           /**< [D2] egress Stats */
    CTC_APP_VLAN_PORT_UPDATE_MAX
};
typedef enum ctc_app_vlan_port_update_type_s ctc_app_vlan_port_update_type_t;


enum ctc_app_vlan_port_match_s {
    CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN,   /**< [GB.D2.TM] Match port + tunnel + svlan + cvlan*/
    CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN,          /**< [GB.D2.TM] Match port + svlan + cvlan*/
    CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX,          /**< [GB.D2.TM] Match port + tunnel + flexible*/
    CTC_APP_VLAN_PORT_MATCH_PORT_FLEX,                 /**< [GB.D2.TM] Match port + flexible*/
    CTC_APP_VLAN_PORT_MATCH_MCAST,               /**< [D2] Match None, for mcast need do qinq, return nhid to caller*/

    CTC_APP_VLAN_PORT_MATCH_MAX
};
typedef enum ctc_app_vlan_port_match_s ctc_app_vlan_port_match_t;


enum ctc_app_vlan_action_s {
    CTC_APP_VLAN_ACTION_NONE,        /**< [GB.D2.TM] Vlan action none */
    CTC_APP_VLAN_ACTION_ADD,         /**< [GB.D2.TM] Add new */
    CTC_APP_VLAN_ACTION_DEL,         /**< [GB.D2.TM] Del */
    CTC_APP_VLAN_ACTION_REPLACE,     /**< [GB.D2.TM] Replace */
    CTC_APP_VLAN_ACTION_MAX
};
typedef enum ctc_app_vlan_action_s ctc_app_vlan_action_t;


struct ctc_app_vlan_action_set_s {

    ctc_app_vlan_action_t svid;   /**< [GB.D2.TM] Svlan vlan id action*/
    ctc_app_vlan_action_t scos;   /**< [GB.D2.TM] svlan cos  action*/
    ctc_app_vlan_action_t scfi;   /**< [GB.D2.TM] Svlan cfi action*/

    ctc_app_vlan_action_t cvid;   /**< [GB.D2.TM] Cvlan vlan id action */
    ctc_app_vlan_action_t ccos;   /**< [GB.D2.TM] Cvlan cos action */
    ctc_app_vlan_action_t ccfi;   /**< [GB.D2.TM] Cvlan cfi action */

    uint16 new_svid;    /**< [GB.D2.TM] New svlan vlan id*/
    uint16 new_scos;    /**< [GB.D2.TM] New svlan cos*/
    uint16 new_scfi;    /**< [GB.D2.TM] New svlan cfi*/

    uint16 new_cvid;   /**< [GB.D2.TM] New cvlan vlan id*/
    uint16 new_ccos;   /**< [GB.D2.TM] New cvlan cos*/
    uint16 new_ccfi;   /**< [GB.D2.TM] New cvlan cfi*/

};
typedef struct ctc_app_vlan_action_set_s ctc_app_vlan_action_set_t;


struct ctc_app_vlan_port_s {

    uint32 vlan_port_id;                /**< [GB.D2.TM]  [out] Entry identification */

    ctc_app_vlan_port_match_t criteria; /**< [GB.D2.TM]  [in] Match criteria*/
    uint32 port;                        /**< [GB.D2.TM]  [in] Incoming port */
    uint32 match_tunnel_value;          /**< [GB.D2.TM]  [in] Tunnel LLID Value */
    uint16 match_svlan;                 /**< [GB.D2.TM]  [in] Svlan to match */
    uint16 match_cvlan;                 /**< [GB.D2.TM]  [in] Cvlan to match */
    uint16 match_svlan_end;             /**< [GB.D2.TM]  [in] Svlan range to match */
    uint16 match_cvlan_end;             /**< [GB.D2.TM]  [in] Cvlan range to match */

    ctc_acl_key_t flex_key;             /**< [GB.D2.TM]  [in] flex key to match */
    ctc_l2_mcast_addr_t l2mc_addr;                /**< [D2] [in] known mcast group to match */

    ctc_app_vlan_action_set_t ingress_vlan_action_set;  /**< [GB.D2.TM]  [in] Ingress vlan action */
    ctc_app_vlan_action_set_t egress_vlan_action_set;   /**< [GB.D2.TM]  [in] Egress vlan action */

    uint32 logic_port;         /**< [GB.D2.TM] [out] Logic port which used for acl */
    uint32 flex_nhid;          /**< [GB.D2.TM] [out] Used for flexible vlan edit, acl redirect nhid*/
    uint32 pon_flex_nhid;      /**< [GB.D2] [out] Pon downlink, Used for flexible vlan edit, acl redirect nhid*/

    /*update*/
    uint8  update_type;        /**< [GB.D2.TM] [in/update] refer to ctc_app_gem_port_update_type_t */
    uint16 ingress_policer_id; /**< [GB.D2.TM] [in/update] ingress policer id */
    uint16 egress_policer_id;  /**< [GB.D2.TM] [in/update] egress policer id */
    uint8  egress_cos_idx;     /**< [D2]    [in/update] egress cos hbwp policer index*/
    uint32 ingress_stats_id;   /**< [D2]    [in/update] ingress stats id */
    uint32 egress_stats_id;    /**< [D2]    [in/update] egress stats id */

};
typedef struct ctc_app_vlan_port_s ctc_app_vlan_port_t;


struct ctc_app_gem_port_s {

    uint32 port;                /**< [GB.D2.TM] [in]  Incomming port*/
    uint32 tunnel_value;        /**< [GB.D2.TM] [in]  GEM port vlan*/

    /*update*/
    uint8  update_type;         /**< [GB.D2.TM] [in/update] refer to ctc_app_gem_port_update_type_t */
    uint8  isolation_en;        /**< [GB.D2] [in/update] Isolate  between each GEM*/
    uint8  pass_trough_en;      /**< [TM]    [in/update] Pass though enable for current Gem */
    uint32 vlan_port_id;        /**< [GB.D2.TM] [in/update] Vlan port id, used for bind/unbind vlan port*/
    uint16 ingress_policer_id;  /**< [GB.D2.TM] [in/update] ingress policer id */
    uint16 egress_policer_id;   /**< [GB.D2.TM] [in/update] egress policer id */
    uint8  egress_cos_idx;      /**< [D2]    [in/update] egress cos index*/
    uint8  station_move_action; /**< [D2]    [in/update] station move action,refer to ctc_port_station_move_action_type_t*/
    uint32 ingress_stats_id;    /**< [D2] [in/update] ingress stats id */
    uint32 egress_stats_id;     /**< [D2] [in/update] egress stats id */

    uint32 logic_port;          /**< [GB.D2.TM] [out] Logic port which used for acl key and policer id*/
    uint32 ga_gport;            /**< [GB.D2] [out] Gem port associate gport, used for enable acl*/
};
typedef struct ctc_app_gem_port_s ctc_app_gem_port_t;

struct ctc_app_uni_s {
    uint32 vdev_id;            /**< [GB.D2.TM] [in] virtual device Identification */
    uint32 port;               /**< [GB.D2.TM] [in] uni port*/
    uint32 mc_nhid;            /**< [GB.D2.TM] [out] for l2 mcast add member*/
    uint32 bc_nhid;            /**< [GB.D2.TM] [out] for l2 bcast add member*/
    uint32 associate_port;     /**< [GB.D2] [out] associate gport*/

};
typedef struct ctc_app_uni_s ctc_app_uni_t;

struct ctc_app_nni_s {
    uint32 vdev_id;            /**< [GB.D2.TM] [in] virtual device Identification */
    uint32 port;               /**< [GB.D2.TM] [in] nni port which is uplink port*/
    uint8 rx_en;               /**< [GB.D2.TM] [in] receive enable */
    uint8 rsv[3];
    uint32 logic_port;         /**< [GB.D2.TM] [out] Logic port which used for learning*/
};
typedef struct ctc_app_nni_s ctc_app_nni_t;

struct ctc_app_vlan_port_fid_s
{
  uint16 vdev_id;              /**< [GB.D2.TM] [in] virtual device Identification */
  uint16 pkt_svlan;            /**< [GB.D2.TM] [in] svlan id in packet */
  uint16 pkt_cvlan;            /**< [GB.D2.TM] [in] cvlan id in packet */
  uint16 is_flex;              /**< [GB.D2.TM] [in] flex qinq */
  uint16 fid;                  /**< [GB.D2.TM] [out] forwarding id */
  uint16 rsv;
};
typedef struct ctc_app_vlan_port_fid_s ctc_app_vlan_port_fid_t;

enum ctc_app_vlan_port_mac_learn_limit_type_e {
    CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_GEM_PORT,    /**< [D2.TM] Learn limit for gem port*/
    CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_FID,         /**< [D2.TM] Learn limit for vlan*/
    CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_VDEV,        /**< [TM] Learn limit for virtual device*/
    CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_SYSTEM,      /**< [TM] Learn limit for system*/

    CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_MAX
};
typedef enum ctc_app_vlan_port_mac_learn_limit_type_e ctc_app_vlan_port_learn_limit_type_t;

struct ctc_app_vlan_port_mac_learn_limit_s
{
    ctc_app_vlan_port_learn_limit_type_t limit_type;        /**< [D2.TM] Learn limit type must be ctc_app_vlan_port_mac_learn_limit_type_t*/
    uint32 gport;                                           /**< [D2.TM] Global source port when limit_type is CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_GEM_PORT*/
    uint32 tunnel_value;                                    /**< [D2.TM] GEM port vlan  when limit_type is CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_GEM_PORT*/
    uint16 fid;                                             /**< [D2.TM] fid ID when limit_type is CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_FID*/
    uint32 limit_count;                                     /**< [D2.TM] mac limit count*/
    uint32 limit_num;                                       /**< [D2.TM] mac limit threshold, max num means disable mac limit*/
    uint32 limit_action;                                    /**< [D2.TM] action when reach threshold, refer to ctc_maclimit_action_t*/
};
typedef struct ctc_app_vlan_port_mac_learn_limit_s ctc_app_vlan_port_mac_learn_limit_t;

enum ctc_app_vlan_global_cfg_type_e {
    CTC_APP_VLAN_CFG_UNI_OUTER_ISOLATE,         /**< [GB.D2.TM] Isolate between uni, If value equal 1, enable */
    CTC_APP_VLAN_CFG_UNI_INNER_ISOLATE,         /**< [GB.D2.TM] Isolate inner uni, If value equal 1, enable */
    CTC_APP_VLAN_CFG_UNKNOWN_MCAST_DROP,        /**< [GB.D2.TM] Unknown mcast drop, If value equal 1, enable */
    CTC_APP_VLAN_CFG_MAC_LEARN_LIMIT,           /**< [D2.TM] Learn limit, refer to ctc_app_vlan_port_learn_limit_t */
    CTC_APP_VLAN_CFG_VLAN_GLOBAL_VDEV_EN,       /**< [TM] If the vlan enable global vdev, the uplink will add global vedv_vlan */
    CTC_APP_VLAN_CFG_VLAN_ISOLATE,              /**< [D2] Specified vlan isolation between ONUs. If value equal 1, isolation enable, else isolation disable */
    CTC_APP_VLAN_CFG_MAX
};
typedef enum ctc_app_vlan_global_cfg_type_e ctc_app_vlan_global_cfg_type_t;

struct ctc_app_global_cfg_s
{
    uint16 vdev_id;          /**< [GB.D2.TM] virtual device Identification */
    uint8 cfg_type;          /**< [GB.D2.TM] Configure type  refer to ctc_app_vlan_global_cfg_type_t*/
    uint32 value;            /**< [GB.D2.TM] Configure value*/
    uint16 vlan_id;          /**< [D2.TM] Vlan id */
    ctc_app_vlan_port_mac_learn_limit_t mac_learn; /**< [D2.TM] Configure app vlan port learn limit */
};
typedef struct ctc_app_global_cfg_s ctc_app_global_cfg_t;

enum ctc_app_vlan_port_flag_e {
    CTC_APP_VLAN_PORT_FLAG_MULTI_NNI_EN = 1U << 0,      /**< [GB.D2] If set, support multi-nni port */
    CTC_APP_VLAN_PORT_FLAG_STATS_EN  = 1U << 1,         /**< [D2] If set, gemport and vlan port can bind stats*/

    CTC_APP_VLAN_PORT_FLAG_MAX
};
typedef enum ctc_app_vlan_port_flag_e ctc_app_vlan_port_flag_t;

struct ctc_app_vlan_port_init_cfg_s {
    uint16 vdev_num;            /**< [GB.D2.TM] virtual device num, default is 1 */
    uint16 vdev_base_vlan;      /**< [TM] mux vlan appand to the packet  represent vdev id */
    uint16 mcast_tunnel_vlan;      /**< [GB.D2] mcast gem port, 0 is default(4094) */
    uint16 bcast_tunnel_vlan;      /**< [GB.D2] bcast gem port, 0 is default(4095) */
    uint32 flag;                /**< [GB.D2] init flag, refer to ctc_app_vlan_port_flag_t */
};
typedef struct ctc_app_vlan_port_init_cfg_s ctc_app_vlan_port_init_cfg_t;

struct ctc_app_vlan_port_api_s
{
    int32(*ctc_app_vlan_port_init)(uint8 lchip, void* p_param);
    int32(*ctc_app_vlan_port_get_nni)(uint8 lchip, ctc_app_nni_t* p_nni);
    int32(*ctc_app_vlan_port_destory_nni)(uint8 lchip, ctc_app_nni_t* p_nni);
    int32(*ctc_app_vlan_port_create_nni)(uint8 lchip, ctc_app_nni_t* p_nni);
    int32(*ctc_app_vlan_port_update_nni)(uint8 lchip, ctc_app_nni_t* p_nni);
    int32(*ctc_app_vlan_port_update)(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port);
    int32(*ctc_app_vlan_port_get)(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port);
    int32(*ctc_app_vlan_port_destory)(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port);
    int32(*ctc_app_vlan_port_create)(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port);
    int32(*ctc_app_vlan_port_update_gem_port)(uint8 lchip, ctc_app_gem_port_t* p_gem_port);
    int32(*ctc_app_vlan_port_get_gem_port)(uint8 lchip, ctc_app_gem_port_t* p_gem_port);
    int32(*ctc_app_vlan_port_destory_gem_port)(uint8 lchip, ctc_app_gem_port_t* p_gem_port);
    int32(*ctc_app_vlan_port_create_gem_port)(uint8 lchip, ctc_app_gem_port_t* p_gem_port);
    int32(*ctc_app_vlan_port_get_uni)(uint8 lchip, ctc_app_uni_t* p_uni);
    int32(*ctc_app_vlan_port_destory_uni)(uint8 lchip, ctc_app_uni_t* p_uni);
    int32(*ctc_app_vlan_port_create_uni)(uint8 lchip, ctc_app_uni_t* p_uni);
    int32(*ctc_app_vlan_port_get_by_logic_port)(uint8 lchip, uint32 logic_port, ctc_app_vlan_port_t* p_vlan_port);
    int32(*ctc_app_vlan_port_get_global_cfg)(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg);
    int32(*ctc_app_vlan_port_set_global_cfg)(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg);
    int32(*ctc_app_vlan_port_get_fid_mapping_info)(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);
    int32(*ctc_app_vlan_port_alloc_fid)(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);
    int32(*ctc_app_vlan_port_free_fid)(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);
};
typedef struct ctc_app_vlan_port_api_s ctc_app_vlan_port_api_t;

/**********************************************************************************
                      Define API function interfaces
 ***********************************************************************************/


/**
 @brief Set vlan port by logic port

 @param[in] lchip    local chip id

 @param[out] p_glb_cfg    point to ctc_app_global_cfg_t

 @remark  Set global ctl

 @return CTC_E_XXX

*/

extern int32
ctc_app_vlan_port_set_global_cfg(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg);


/**
 @brief Get vlan port by logic port

 @param[in] lchip    local chip id

 @param[out] p_glb_cfg    point to ctc_app_global_cfg_t

 @remark  Set global ctl

 @return CTC_E_XXX

*/

extern int32
ctc_app_vlan_port_get_global_cfg(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg);

/**
 @brief Get vlan port by logic port

 @param[in] lchip    local chip id

 @param[in] logic_port    logic port

 @param[out] p_vlan_port    point to p_vlan_port

 @remark  Get vlan port by logic port

 @return CTC_E_XXX

*/

extern int32
ctc_app_vlan_port_get_by_logic_port(uint8 lchip, uint32 logic_port, ctc_app_vlan_port_t* p_vlan_port);


/**
 @brief Create uni port

 @param[in] lchip    local chip id

 @param[in/out] p_uni    point to ctc_app_nni_t

 @remark  Create uni downlin port

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_create_uni(uint8 lchip, ctc_app_uni_t* p_uni);

/**
 @brief Destroy uni port

 @param[in] lchip    local chip id

 @param[in] p_uni    point to ctc_app_nni_t

 @remark  Destroy uni downlin port

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_destory_uni(uint8 lchip, ctc_app_uni_t* p_uni);



/**
 @brief Get uni port

 @param[in] lchip    local chip id

 @param[in/out] p_uni    point to ctc_app_nni_t

 @remark  Get uni downlin port

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_get_uni(uint8 lchip, ctc_app_uni_t* p_uni);




/**
 @brief Create gem port which mapping tunnel value to logic port and gem_gport

 @param[in] lchip    local chip id

 @param[in/out] p_gem_port  point to ctc_app_gem_port_t

 @remark  Mapping tunnel value to logic port and gem_gport

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_create_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port);

/**
 @brief Destroy gem port which cancel mapping tunnel value to logic port and gem_gport

 @param[in] lchip    local chip id

 @param[in] p_gem_port  point to ctc_app_gem_port_t

 @remark  Cancel mapping tunnel value to logic port and gem_gport

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_destory_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port);



/**
 @brief Get gem port which cancel mapping tunnel value to logic port and gem_gport

 @param[in] lchip    local chip id

 @param[in/out] p_gem_port  point to ctc_app_gem_port_t

 @remark  Get mapping tunnel value to logic port and gem_gport

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_get_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port);



/**
 @brief Update em port property

 @param[in] lchip    local chip id

 @param[in] p_gem_port  point to ctc_app_gem_port_t

 @remark  Update gem port property

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_update_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port);


/**
 @brief Create vlan port which mapping vlan and service

 @param[in] lchip    local chip id

 @param[in/out] p_vlan_port point to ctc_app_vlan_port_t

 @remark  Mapping vlan and service

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_create(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port);

/**
 @brief Destroy vlan port which cancel mapping vlan and service

 @param[in] lchip    local chip id

 @param[in] p_vlan_port point to ctc_app_vlan_port_t

 @remark  Cancel mapping vlan and service

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_destory(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port);


/**
 @brief Get vlan port which cancel mapping vlan and service

 @param[in] lchip    local chip id

 @param[in/out] p_vlan_port point to ctc_app_vlan_port_t

 @remark  Get mapping vlan and service

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_get(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port);


/**
 @brief Update vlan port which cancel mapping vlan and service

 @param[in] lchip    local chip id

 @param[in/out] p_vlan_port point to ctc_app_vlan_port_t

 @remark  Update mapping vlan and service

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_update(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port);


/**
 @brief Create nni uplink port

 @param[in] lchip    local chip id

 @param[in/out] p_nni    point to ctc_app_nni_t

 @remark  Create nni uplink port

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_create_nni(uint8 lchip, ctc_app_nni_t* p_nni);

/**
 @brief Destroy nni uplink port

 @param[in] lchip    local chip id

 @param[in] p_nni    point to ctc_app_nni_t

 @remark  Destroy nni uplnk port

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_destory_nni(uint8 lchip, ctc_app_nni_t* p_nni);


/**
 @brief Update nni port property

 @param[in] lchip    local chip id

 @param[in] p_uni  point to ctc_app_nni_t

 @remark  Update nni port property

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_update_nni(uint8 lchip, ctc_app_nni_t* p_uni);

/**
 @brief Get nni uplink port

 @param[in] lchip    local chip id

 @param[in/out] p_nni    point to ctc_app_nni_t

 @remark  Get nni uplnk port

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_get_nni(uint8 lchip, ctc_app_nni_t* p_nni);


/**
 @brief Get fid mapping info

 @param[in] lchip    local chip id

 @param[in/out] p_port_fid    point to ctc_app_vlan_port_fid_t

 @remark  Get fid with svlan and cvlan

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_get_fid_mapping_info(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);

/**
 @brief Alloc fid with svlan cvlan vdev_id

 @param[in] lchip    local chip id

 @param[in/out] p_port_fid    point to ctc_app_vlan_port_fid_t

 @remark  Alloc fid with svlan cvlan and vdev_id

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_alloc_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);

/**
 @brief Free fid with svlan cvlan vdev_id

 @param[in] lchip    local chip id

 @param[in/out] p_port_fid    point to ctc_app_vlan_port_fid_t

 @remark  free fid with svlan cvlan and vdev_id

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_free_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid);

/**
 @brief vlan port ini

 @param[in] lchip    local chip id

 @param[in] p_param    void

 @remark  init vlan port

 @return CTC_E_XXX

*/
extern int32
ctc_app_vlan_port_init(uint8 lchip, void* p_param);


#ifdef __cplusplus
}
#endif

#endif  /* _CTC_APP_VLAN_PORT_API_H_*/



