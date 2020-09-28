/**
 @file ctc_sgmac.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-7-28

 @version v2.0

 This file contains all port related data structure, enum, macro and proto.

*/

#ifndef _CTC_SGMAC_H
#define _CTC_SGMAC_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
 @defgroup sgmac SGMAC
 @{
*/
#define CTC_HUMBER_DS_VPLS_PORT_ENTRY_NUM   32                                      /**<vpls port entry>*/
#define CTC_HUMBER_MAX_VPLS_SRC_PORT        (CTC_HUMBER_DS_VPLS_PORT_ENTRY_NUM * 256) /**<vpls port entry>*/

/**
 @brief define sgmac register base
*/
enum ctc_sgmac_reg_base_e
{
    CTC_SGMAC_INGRESS_DVP_NH_BASE = 0,      /**< nextHop base for SGMAC VPWS/VPLS >*/
    CTC_SGMAC_INGRESS_DVP_MET_BASE,         /**< VPWS/VPLS multicast MET base >*/
    CTC_SGMAC_INGRESS_DEFAULT_MET_PTR,      /**< if defaultMetValid set this is the MET pointer else this is default MET base >*/
    CTC_SGMAC_INGRESS_L2_MET_BASE,          /**< L2 multicast MET base >*/
    CTC_SGMAC_INGRESS_L3_MET_BASE,          /**< L3 multicast MET base >*/

    CTC_SGMAC_EGRESS_SGMAC0_L2_MET_BASE,    /**< l2 MET base >*/
    CTC_SGMAC_EGRESS_SGMAC0_L3_MET_BASE,    /**< l3 MET base >*/
    CTC_SGMAC_EGRESS_SGMAC0_DVP_MET_BASE,   /**< VPLS MET base >*/

    CTC_SGMAC_EGRESS_SGMAC1_L2_MET_BASE,    /**< l2 MET base >*/
    CTC_SGMAC_EGRESS_SGMAC1_L3_MET_BASE,    /**< l3 MET base >*/
    CTC_SGMAC_EGRESS_SGMAC1_DVP_MET_BASE,   /**< VPLS MET base >*/

    CTC_SGMAC_EGRESS_SGMAC2_L2_MET_BASE,    /**< VPLS MET base >*/
    CTC_SGMAC_EGRESS_SGMAC2_L3_MET_BASE,    /**< l3 MET base >*/
    CTC_SGMAC_EGRESS_SGMAC2_DVP_MET_BASE,   /**< l2 MET base >*/

    CTC_SGMAC_EGRESS_SGMAC3_L2_MET_BASE,    /**< l2 MET base >*/
    CTC_SGMAC_EGRESS_SGMAC3_L3_MET_BASE,    /**< l3 MET base >*/
    CTC_SGMAC_EGRESS_SGMAC3_DVP_MET_BASE    /**< VPLS MET base >*/
};
typedef enum ctc_sgmac_reg_base_e ctc_sgmac_reg_base_t;

struct ctc_sgmac_s
{
    uint8 version;              /**<sgmac version*/
    uint8 trunk_num;            /**<sgmac trunk group number*/
    uint8 stacked_chip[2];      /**<remote gchip_id connect with trunk group*/
};
typedef struct ctc_sgmac_s ctc_sgmac_t;

/**@} end of @defgroup sgmac  */
#ifdef __cplusplus
}
#endif

#endif

