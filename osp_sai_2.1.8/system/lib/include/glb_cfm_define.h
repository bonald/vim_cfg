/****************************************************************************
 *  Centec SyncE related global MACRO, ENUM, Date Structure defines file
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      :  R0.01
 * Author        :  Weihao Li
 * Date          :  2019-05-29
 * Reason        :  First Create.
 ****************************************************************************/
#ifndef __GLB_CFM_DEFINE_H__
#define __GLB_CFM_DEFINE_H__

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#define GLB_CFM_MODE_DOT1AG             0
#define GLB_CFM_MODE_Y1731              1

/* IEEE 802.1ag 2007, Table 21-19 Maintenance Domain Name Format */
#define  CFM_MD_IEEE_802_1_RESERVED                0
#define  CFM_NO_MD_NAME                                       1
#define  CFM_DNS_NAME                                            2
#define  CFM_MAC_ADDR_TWO_OCTET_INTEGER     3
#define  CFM_MD_CHAR_STRING                                4
#define  CFM_MD_TYPE_Y1731_ICC_BASED             32

#define CFM_DOWN 0
#define CFM_UP   1


#endif /*!__GLB_CFM_DEFINE_H__*/

