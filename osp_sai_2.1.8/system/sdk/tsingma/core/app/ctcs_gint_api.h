
#ifndef _GINT_API_H
#define _GINT_API_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(TSINGMA)

#include "sal.h"

enum ctc_gint_vlan_action_type_s {
    CTC_GINT_VLAN_ACTION_NONE,       
    CTC_GINT_VLAN_ACTION_ADD,         
    CTC_GINT_VLAN_ACTION_DEL,         
    CTC_GINT_VLAN_ACTION_REPLACE,     
    CTC_GINT_VLAN_ACTION_MAX
};
typedef enum ctc_gint_vlan_action_type_s ctc_gint_vlan_action_type_t;


struct ctc_gint_vlan_action_s {

    uint8  svid_action; /**< refer to ctc_gint_vlan_action_type_t*/
    uint8  scos_action; /**< refer to ctc_gint_vlan_action_type_t*/  
    uint8  scfi_action; /**< refer to ctc_gint_vlan_action_type_t*/ 

    uint8  cvid_action; /**< refer to ctc_gint_vlan_action_type_t*/  
    uint8  ccos_action; /**< refer to ctc_gint_vlan_action_type_t*/
    uint8  ccfi_action; /**< refer to ctc_gint_vlan_action_type_t*/  

    uint16 new_svid;   
    uint16 new_scos;    
    uint16 new_scfi;  

    uint16 new_cvid;   
    uint16 new_ccos;   
    uint16 new_ccfi;  
};
typedef struct ctc_gint_vlan_action_s ctc_gint_vlan_action_t;



struct ctc_gint_storm_restrict_s
{
    uint32  dsl_id;
    uint8   type; /**< 0-boardcast; 1-muticast; 2-unknown*/
    
    uint32  cir;  /**< Committed Information Rate (unit :kbps/pps),
                       CIR = 0 is CIR Fail; cir = CTC_MAX_UINT32_VALUE is CIR pass*/
    uint32  cbs;  /**< Committed Burst Size, (unit :kb/pkt),
                        if set CTC_QOS_SHP_TOKE_THRD_DEFAULT,
                        default value will be set */
    uint8   enable;
}; 
typedef struct ctc_gint_storm_restrict_s  ctc_gint_storm_restrict_t;

struct ctc_gint_mac_limit_s
{
    uint32  dsl_id;     /**<[in]*/
    uint32  limit_cnt;  /**<[out]    it means the logic_port fdb learning count now*/
    uint32  limit_num;  /**<[in/out] threshold, 0 means disable mac limit*/
    uint8   enable;     /**<[in/out] function enable/disable*/
}; 
typedef struct ctc_gint_mac_limit_s  ctc_gint_mac_limit_t;


struct ctc_gint_uni_s
{
    uint32 dsl_id;    /**< [in] xDSL Port ID*/  
    uint32 sid;       /**< [in] stream id*/
    uint32 gport;     /**< [in] gint port, scope 0-63*/

    uint16 logic_port; /*[out]*/
}; 
typedef struct ctc_gint_uni_s  ctc_gint_uni_t;

struct ctc_gint_nni_s
{
    uint32 gport;    /**< [in] gint port, scope 0-63*/
    
    uint16 logic_port;/**< [out]*/
}; 
typedef struct ctc_gint_nni_s  ctc_gint_nni_t;

struct ctc_gint_service_s
{

    uint32 dsl_id;      /**< [in]*/
    uint16 svlan;       /**< [in]*/
    uint16 cvlan;       /**< [in]*/    

    ctc_gint_vlan_action_t igs_vlan_action;/**< [in]*/
    ctc_gint_vlan_action_t egs_vlan_action;/**< [in]*/

    uint16 logic_port;/**< [out]*/
}; 
typedef struct ctc_gint_service_s  ctc_gint_service_t;

enum ctc_gint_upd_action_s 
{
    CTC_GINT_UPD_ACTION_IGS_POLICER = 0x00000001,
    CTC_GINT_UPD_ACTION_IGS_STATS   = 0x00000002,  
    CTC_GINT_UPD_ACTION_EGS_POLICER = 0x00000004,
    CTC_GINT_UPD_ACTION_EGS_STATS   = 0x00000008,
   
    CTC_GINT_ACTION_MAX
};
typedef enum ctc_gint_upd_action_s ctc_gint_upd_action_t;

struct ctc_gint_upd_s
{
    uint32 dsl_id;        /**< [in]*/

    uint32 flag;          /**< [in] refer to ctc_gint_upd_action_t*/  
    uint16 igs_policer_id;/**< [in] 0-0x7fff, 0 means remove*/ 
    uint32 igs_stats_id;  /**< [in] 0 means remove*/
    uint16 egs_policer_id;/**< [in] 0-0x7fff, 0 means remove*/  
    uint32 egs_stats_id;  /**< [in] 0 means remove*/
}; 
typedef struct ctc_gint_upd_s  ctc_gint_upd_t;


#define _GINT_SECURITY_  ""
extern int32 ctcs_gint_set_port_isolation(uint8 lchip, uint32 dsl_id, uint8 enable);
extern int32 ctcs_gint_get_port_isolation(uint8 lchip, uint32 dsl_id, uint8* enable);
extern int32 ctcs_gint_set_storm_restrict(uint8 lchip, ctc_gint_storm_restrict_t* p_cfg);
extern int32 ctcs_gint_get_storm_restrict(uint8 lchip, ctc_gint_storm_restrict_t* p_cfg);
extern int32 ctcs_gint_set_mac_limit(uint8 lchip, ctc_gint_mac_limit_t* p_cfg);
extern int32 ctcs_gint_get_mac_limit(uint8 lchip, ctc_gint_mac_limit_t* p_cfg);

#define _GINT_CFG_ ""
extern int32 ctcs_gint_create_uni(uint8 lchip, ctc_gint_uni_t* p_cfg);
extern int32 ctcs_gint_destroy_uni(uint8 lchip, ctc_gint_uni_t* p_cfg);
extern int32 ctcs_gint_create_nni(uint8 lchip, ctc_gint_nni_t* p_cfg);
extern int32 ctcs_gint_destroy_nni(uint8 lchip, ctc_gint_nni_t* p_cfg);
extern int32 ctcs_gint_add_service(uint8 lchip, ctc_gint_service_t* p_cfg);
extern int32 ctcs_gint_remove_service(uint8 lchip, ctc_gint_service_t* p_cfg);
extern int32 ctcs_gint_update(uint8 lchip, ctc_gint_upd_t* p_cfg);
extern int32 ctcs_gint_init(uint8 lchip);
extern int32 ctcs_gint_deinit(uint8 lchip);

#endif

#ifdef __cplusplus
}
#endif

#endif


