/**
 @file ctc_usw_parser.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-12-15

 @version v2.0

 Parser module is used to parser layer2, layer3, layer4 packet type and header information. Each packet type has its header format.
 According to layer2 header protocol(Ethertype) in layer2 ETHERNETv2 packet header, can get layer3 packet type. Such as ethertype 0x0800
 can get layer3 packet type is ipv4. And from layer3 header protocol in layer3 packet header, can get layer4 packet type.
 Support to parser:
 \p
    layer2(ppp, eth_v2, eth_sap, eth_snap),
 \p
    layer3(ipv4, ipv6, mpls, mplsmcast, arp, pbb_cmac, ethernet_oam, slow_ptl, fcoe, trill, ptp, udf),
 \p
    layer4(tcp, udp, gre, ach_oam, pbb_itag_oam, ipinip, v6inip, ipinv6, v6inv6, icmp, igmp, rdp , sctp, dccp, udf).
*/

#ifndef CTC_USW_PARSER_H_
#define CTC_USW_PARSER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_parser.h"

/**********************************************************************************
                      Define API function interfaces
***********************************************************************************/
/**
@addtogroup parser PARSER
@{
*/

/**
 @addtogroup parser_l2 PARSER_L2
 @{
*/

/**
 @brief Set tpid value with specified type

 @param[in] lchip    local chip id

 @param[in] type tpid_type CTC_PARSER_L2_TPID_XXX

 @param[in] tpid tpid value

 @remark[D2.TM]  Set tpid value with specified type:
\p
                Use CTC_PARSER_L2_TPID_CVLAN_TPID  to set cvlan tpid
\p
                Use CTC_PARSER_L2_TPID_ITAG_TPID to set itag tpid
\p
                Use CTC_PARSER_L2_TPID_BLVAN_TPID to set bvlan tpid
\p
                Use CTC_PARSER_L2_TPID_SVLAN_TPID_0 to set svlan tpid
\p
                Use CTC_PARSER_L2_TPID_CNTAG_TPID to set cntag tpid
\p
                Use CTC_PARSER_L2_TPID_EVB_TPID to set evb tpid

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_set_tpid(uint8 lchip, ctc_parser_l2_tpid_t type, uint16 tpid);

/**
 @brief Get tpid with specified type

 @param[in] lchip    local chip id

 @param[in] type tpid_type CTC_PARSER_L2_TPID_XXX

 @param[out] tpid tpid value

 @remark[D2.TM]  Get tpid value with specified type:
\p
                Use CTC_PARSER_L2_TPID_CVLAN_TPID to get cvlan tpid
\p
                Use CTC_PARSER_L2_TPID_ITAG_TPID to get itag tpid
\p
                Use CTC_PARSER_L2_TPID_BLVAN_TPID to get bvlan tpid
\p
                Use CTC_PARSER_L2_TPID_SVLAN_TPID_0 to get svlan tpid
\p
                Use CTC_PARSER_L2_TPID_CNTAG_TPID to get cntag tpid
\p
                Use CTC_PARSER_L2_TPID_EVB_TPID to get evb tpid

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_get_tpid(uint8 lchip, ctc_parser_l2_tpid_t type, uint16* tpid);

/**
 @brief Set max_length,based on the value differentiate to denote type or length field in ethernet frame

 @param[in] lchip    local chip id

 @param[in] max_length max_length value

 @remark[D2.TM]  Set max length,maximum length field to differentiate type or length field in ethernet frame.
\p
                When type or length field value is less than max_length,the field stands for length,else stands for type.
\p
                SDK initialize max length to 1536.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_set_max_length_field(uint8 lchip, uint16 max_length);

/**
 @brief Get max_length value

 @param[in] lchip    local chip id

 @param[out] max_length max_length value

 @remark[D2.TM]  Get max length,maximum length field to differentiate type or length
                When type or length field value is less than max_length,the field stands for
                length,else stands for type.
\p
                SDK initialize max length to 1536.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_get_max_length_filed(uint8 lchip, uint16* max_length);

/**
 @brief Set vlan parser num

 @param[in] lchip    local chip id

 @param[in] vlan_num vlan parser num

 @remark[D2.TM]  Set vlan parser num:0-2
\p
                Control the number of vlans to be parserd, up to 2 vlans can be parsered in the parser.
\p
                Chip can support parser four types of vlan tag:svlan tag+ cvlan tag,svlan tag,cvlan tag,untag.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_set_vlan_parser_num(uint8 lchip, uint8 vlan_num);

/**
 @brief Get vlan parser num

 @param[in] lchip    local chip id

 @param[out] vlan_num returned value

 @remark[D2.TM]  Get vlan parser num:0-2
\p
                Control how many vlan to be parsed, up to 2 vlans can be parsered in the parser.
\p
                Chip can support parser four types of vlan tag:svlan tag+ cvlan tag,svlan tag,cvlan tag,untag.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_get_vlan_parser_num(uint8 lchip, uint8* vlan_num);

/**
 @brief Set pbb header info

 @param[in] lchip    local chip id

 @param[in] pbb_header  pbb header parser info

 @remark[D2.TM]  Set pbb header info:nca value enable,outer vlan is cvlan,vlan parsing num.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_set_pbb_header(uint8 lchip, ctc_parser_pbb_header_t* pbb_header);

/**
 @brief Get pbb header info

 @param[in] lchip    local chip id

 @param[out] pbb_header  pbb header parser info

 @remark[D2.TM]  Get pbb header info:nca value enable,outer vlan is cvlan,vlan parsing num.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_get_pbb_header(uint8 lchip, ctc_parser_pbb_header_t* pbb_header);

/**
 @brief Add l2type,can add a new l2type,addition offset for the type,can get the layer3 type

 @param[in] lchip    local chip id

 @param[in] index  the entry index (The range :0~3)

 @param[in] entry  the entry

 @remark[D2.TM]  Map a new layer3 type according to isEth, isPPP, isSAP, l2Type, l2HeaderProtocol.
                Only layer3 type:13,14,15 can be configed by user.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_map_l3_type(uint8 lchip, uint8 index, ctc_parser_l2_protocol_entry_t* entry);

/**
 @brief Remove entry based on the index

 @param[in] lchip    local chip id

 @param[in] index  the entry index(The range :0~3)

 @remark[D2.TM]  Set the entry invalid based on the index to remove mapped layer3 type.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_unmap_l3_type(uint8 lchip, uint8 index);

/**
 @brief enable or disable parser layer3 type

 @param[in] lchip    local chip id

 @param[in] l3_type  the layer3 type in ctc_parser_l3_type_t need to enable or disable, 2-15

 @param[in] enable  enable parser layer3 type

 @remark[D2.TM]  Enable or disable parser layer3 type in ctc_parser_l3_type_t, 2-15, from  CTC_PARSER_L3_TYPE_IPV4
                to CTC_PARSER_L3_TYPE_RSV_USER_FLEXL3

 @return SDK_E_XXX

*/
extern int32
ctc_usw_parser_enable_l3_type(uint8 lchip, ctc_parser_l3_type_t l3_type, bool enable);

/**@} end of @addtogroup   parser_l2 PARSER_L2*/

/**
 @addtogroup parser_l3 PARSER_L3
 @{
*/

/**
 @brief Set trill header info

 @param[in] lchip    local chip id

 @param[in] trill_header  trill header parser info

 @remark[D2.TM]  Set trill header info:inner vlan tag tpid,rbridge channel ether type.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_set_trill_header(uint8 lchip, ctc_parser_trill_header_t* trill_header);

/**
 @brief Get trill header info

 @param[in] lchip    local chip id

 @param[out] trill_header  trill header parser info

 @remark[D2.TM]  Get trill header info:inner vlan tag tpid,rbridge channel ether type.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_get_trill_header(uint8 lchip, ctc_parser_trill_header_t* trill_header);

/**
 @brief Add l3type,can add a new l3type,addition offset for the type,can get the layer4 type

 @param[in] lchip    local chip id

 @param[in] index  the entry index(The range :0~3)

 @param[in] entry  the entry

 @remark[D2.TM]  Map a new layer4 type according to l3_type, l3_type_mask, isSAP, l3_header_protocol, l3_header_protocol_mask,addition_offset.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_map_l4_type(uint8 lchip, uint8 index, ctc_parser_l3_protocol_entry_t* entry);

/**
 @brief Remove entry based on the index

 @param[in] lchip    local chip id

 @param[in] index  the entry index(The range :0~3)

 @remark[D2.TM]  Set the entry invalid based on the index to remove mapped layer4 type.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_unmap_l4_type(uint8 lchip, uint8 index);

/**@} end of @addtogroup   parser_l3 PARSER_L3*/

/**
 @addtogroup parser_l4 PARSER_L4
 @{
*/

/**
 @brief Set layer4 flex header

 @param[in] lchip    local chip id

 @param[in] l4flex_header  l4 flex header info

 @remark[D2.TM]  Set layer4 flex header info:byte selection to put as layer4 dest port,layer4 min length.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_set_l4_flex_header(uint8 lchip, ctc_parser_l4flex_header_t* l4flex_header);

/**
 @brief Get layer4 flex header

 @param[in] lchip    local chip id

 @param[in] l4flex_header  l4 flex header info

 @remark[D2.TM]  Get layer4 flex header info:byte selection to put as layer4 dest port,layer4 min length.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_get_l4_flex_header(uint8 lchip, ctc_parser_l4flex_header_t* l4flex_header);

/**
 @brief Set layer4 app control

 @param[in] lchip    local chip id

 @param[in] l4app_ctl  l4 app ctl info

 @remark[D2.TM]  Set layer4 app control info:ptp enable,ntp enable,bfd enable,apwap enable,wlan packet roaming state.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_set_l4_app_ctl(uint8 lchip, ctc_parser_l4app_ctl_t* l4app_ctl);

/**
 @brief Get layer4 app control

 @param[in] lchip    local chip id

 @param[out] l4app_ctl  l4 app ctl info

 @remark[D2.TM]  Get layer4 app control info:ptp enable,ntp enable,bfd enable,apwap enable,wlan packet roaming state.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_get_l4_app_ctl(uint8 lchip, ctc_parser_l4app_ctl_t* l4app_ctl);

/**
 @brief Set ecmp hash control field

 @param[in] lchip    local chip id

 @param[in] hash_ctl  the field of ecmp hash computing

 @remark[D2.TM]  Set members of ecmp hash key included by hash_type_bitmap and flags.
          It can be fields in layer2,layer3 ip,layer4,pbb,mpls,fcoe,trill packet header.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_set_ecmp_hash_field(uint8 lchip, ctc_parser_ecmp_hash_ctl_t* hash_ctl);

/**
 @brief Get hash control field

 @param[in] lchip    local chip id

 @param[out] hash_ctl  the field of hash computing

 @remark[D2.TM]  Get members of ecmp hash key included by hash_type_bitmap and flags.
          It can be fields in layer2,layer3 ip,layer4,pbb,mpls,fcoe,trill packet header.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_get_ecmp_hash_field(uint8 lchip, ctc_parser_ecmp_hash_ctl_t* hash_ctl);

/**
 @brief Set efd hash control field

 @param[in] lchip    local chip id

 @param[in] hash_ctl  the filed of efd hash computing

 @remark[D2.TM]  Set members of efd hash key included by hash_type_bitmap and flags.
          It can be fields in layer2,layer3 ip,layer4 packet header.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_set_efd_hash_field(uint8 lchip, ctc_parser_efd_hash_ctl_t* hash_ctl);

/**
 @brief Get efd hash control field

 @param[in] lchip    local chip id

 @param[out] hash_ctl  the filed of hash computing

 @remark[D2.TM]  Get members of efd hash key included by hash_type_bitmap and flags.
          It can be fields in layer2,layer3 ip,layer4 packet header.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_get_efd_hash_field(uint8 lchip, ctc_parser_efd_hash_ctl_t* hash_ctl);

/**
 @brief Set parser global config info

 @param[in] lchip    local chip id

 @param[in] global_cfg  the field of parser global config info

 @remark[D2.TM]  Set parser global config info:
\p
\d                ecmp_hash_type: default use xor hash algorithm, if set, use crc
\d                linkagg_hash_type: default use xor hash algorithm, if set, use crc
\d                small_frag_offset:ipv4 small fragment offset, 0~3,means 0,8,16,24 bytes of small fragment length,default 0

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_set_global_cfg(uint8 lchip, ctc_parser_global_cfg_t* global_cfg);

/**
 @brief Get parser global config info

 @param[in] lchip    local chip id

 @param[out] global_cfg  the field of parser global config info

 @remark[D2.TM]  Get parser global config info:
\p
\d                ecmp_hash_type: default use xor hash algorithm, if set, use crc
\d                linkagg_hash_type: default use xor hash algorithm, if set, use crc
\d                small_frag_offset:ipv4 small fragment offset, 0~3,means 0,8,16,24 bytes of small fragment length,default 0

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_get_global_cfg(uint8 lchip, ctc_parser_global_cfg_t* global_cfg);

/**@} end of @addtogroup   parser_l4 PARSER_L4*/

/**
 @brief Init parser module

 @param[in] lchip    local chip id

 @param[in] parser_global_cfg  Data of the mpls initialization

 @remark[D2.TM]  Initialize the parser module  registers and global variable, including cvlan_tpid,svlan_tpid0,max_length_field,vlan_parsing_num,etc.
                This function has to be called before any other functions.
\p
                Ipv4 small fragment offset and hash algorithm type can set by user according to the parameter parser_global_cfg.
\p
                Ipv4 small fragment offset is 0~3,means 0,8,16,24 bytes of small fragment length.
\p
                Hash algorithm type 0 is xor, else is crc.
\p
                When parser_global_cfg is NULL will use default global config to initialize sdk.
\p
                By default:
\p
                Ipv4 small fragment offset is 0
\p
                Hash algorithm type is 0, use xor

 @return CTC_E_XXX

*/
extern int32
ctc_usw_parser_init(uint8 lchip, void* parser_global_cfg);

/**
 @brief De-Initialize parser module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the parser configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_parser_deinit(uint8 lchip);

/**@} end of @addtogroup   parser PARSER*/

#ifdef __cplusplus
}
#endif

#endif

