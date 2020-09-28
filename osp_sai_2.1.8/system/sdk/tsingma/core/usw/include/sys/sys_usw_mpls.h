#if (FEATURE_MODE == 0)
/**
 @file sys_usw_mpls.h

 @date 2010-03-11

 @version v2.0

*/
#ifndef _SYS_USW_MPLS_H
#define _SYS_USW_MPLS_H
#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"

#include "ctc_mpls.h"
#include "ctc_const.h"
#include "ctc_stats.h"
#include "ctc_hash.h"
/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/

enum sys_mpls_ilm_property_type_e
{
    SYS_MPLS_ILM_QOS_DOMAIN,
    SYS_MPLS_ILM_APS_SELECT,
    SYS_MPLS_ILM_OAM_LMEN,
    SYS_MPLS_ILM_OAM_IDX,
    SYS_MPLS_ILM_OAM_CHK,
    SYS_MPLS_ILM_OAM_VCCV_BFD,
    SYS_MPLS_ILM_ROUTE_MAC,
    SYS_MPLS_ILM_LLSP,
    SYS_MPLS_ILM_DENY_LEARNING,
    SYS_MPLS_ILM_ARP_ACTION,
    SYS_MPLS_ILM_QOS_MAP,
    SYS_MPLS_STATS_EN,
    SYS_MPLS_ILM_MAC_LIMIT_DISCARD_EN,
    SYS_MPLS_ILM_MAC_LIMIT_ACTION,
    SYS_MPLS_ILM_STATION_MOVE_DISCARD_EN,
    SYS_MPLS_ILM_METADATA,
    SYS_MPLS_ILM_DCN_ACTION,
	SYS_MPLS_ILM_CID,
    SYS_MPLS_ILM_MAX
};
typedef enum sys_mpls_ilm_property_type_e sys_mpls_ilm_property_type_t;

enum sys_mpls_ilm_phb_mode_e
{
    SYS_MPLS_ILM_PHB_DISABLE,     /*ilm donot mapping pri and color*/
    SYS_MPLS_ILM_PHB_RAW,         /*ilm mapping pri and color directly*/
    SYS_MPLS_ILM_PHB_ELSP,        /*ilm mapping qos domain, pri and color according qos domain phb*/
    SYS_MPLS_ILM_PHB_LLSP,        /*ilm mapping pri directly, color accoding qos domain phb*/

    SYS_MPLS_ILM_PHB_MAX
};
typedef enum sys_mpls_ilm_phb_mode_e sys_mpls_ilm_phb_mode_t;

struct sys_mpls_oam_s
{
    /*key*/
    uint32 label;
    uint8  spaceid;
    /*data*/
    uint16 oam_type;   /*0-vpws oam; 1-tp oam*/
    uint16 oamid;      /*0-vpws oam: l2vpn id ;1-tp oam : mep index*/
    uint32 gport;
};
typedef struct sys_mpls_oam_s sys_mpls_oam_t;

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
extern int32
sys_usw_mpls_init(uint8 lchip, ctc_mpls_init_t* p_mpls_info);

extern int32
sys_usw_mpls_add_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm);
int32
sys_usw_mpls_update_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm);
extern int32
sys_usw_mpls_del_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm);

extern int32
sys_usw_mpls_get_ilm(uint8 lchip, uint32 nh_id[SYS_USW_MAX_ECPN], ctc_mpls_ilm_t * p_mpls_ilm);

extern int32
sys_usw_mpls_add_l2vpn_pw(uint8 lchip, ctc_mpls_l2vpn_pw_t* p_mpls_pw);

extern int32
sys_usw_mpls_del_l2vpn_pw(uint8 lchip, ctc_mpls_l2vpn_pw_t* p_mpls_pw);

extern int32
sys_usw_mpls_set_ilm_property(uint8 lchip, ctc_mpls_property_t* p_mpls_pro_info);

extern int32
sys_usw_mpls_deinit(uint8 lchip);
extern int32
sys_usw_mpls_get_oam_info(uint8 lchip, sys_mpls_oam_t* mpls_oam);
extern int32
sys_usw_mpls_swap_oam_info(uint8 lchip, uint32 label, uint8 spaceid, uint32 new_label, uint8 new_spaceid);
#ifdef __cplusplus
}
#endif

#endif

#endif
