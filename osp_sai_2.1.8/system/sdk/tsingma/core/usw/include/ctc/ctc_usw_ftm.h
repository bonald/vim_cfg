/**
    @file ctc_usw_ftm.h

    @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

    @date 2012-10-19

    @version v2.0


  Greatbelt FTM technology support flexible table sizes targeting different application scenario.
  All FTM tables are dynamic which reside in one or more physical memory block(s).
  The size of the table is decided based on a given profile and  memory is allocated at system initialization
  As a highlight of Greatbelt, majority of the FTM memory uses eDRAM technology
  achieving higher density with less power per bit.
\p
    Greatbelt FTM engine incorporates the following physical memories:
\p
\d	eDRAM: 20Mbits of eDRAM, organized as 8 independent accessible blocks.
    All eDRAMs are ECC protected. Effective storage excluding ECC are 224Kx80bits
\d	eTCAM: 640Kbits of eTCAM organized as 8Kx80bits, support error protection
\d	eSRAM: 640Kbits of eSRAM, used as for storage space for eTCAM lookup associated data

\p
  we supply 10 profile apply for different application scenario.
\p
 (1)Default,      (2)Enterprise ,     (3)Bridge,           (4)IPv4 Routing (Host), (5)Pv4 Routing (LPM),
\p
  6)IPv6 Routing, (7)Ethernet Access, (8)MPLS L2VPN(VPWS), (9) VPLS L2VPN(VPLS),   (10)MPLS L3VPN
\p
 If more profiles or modification to the profiles are necessary, please contact centec

\p
\t |Profile        | 1   | 2    |  3   |  4    |  5    |  6   |  7    | 8    | 9   |  10
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----
\t |MAC            | 32K | 64K  |  128K|  32K  |  8K   |  8K  |  64K  | 32K  | 64K |  8K
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----
\t |IPv4-Host      | 8K  | 16K  |  512 |  32K  |  4K   |  0K  |  512  | 1K   | 1K  |  8K
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----
\t |IPv4-LPM       | 16K | 16K  |  512 |  8K   |  64K  |  16K |  512  | 1K   | 1K  |  16K
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----
\t |IPv6           | 512 | 0K   |  0K  |  0K   |  0K   |  8K  |  0K   | 0K   | 0K  |  0K
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----
\t |IPv4-Multicast | 512 | 1K   |  512 |  1K   |  1K   |  256K|  512K | 0K   | 0K  |  1K
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----
\t |IPv6-Multicast | 256 | 0K   |  0K  |  0K   |  0K   |  512K|  0K   | 0K   | 0K  |  0K
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----
\t |ACL            | 1K  | 1.25K|  1.5K|  1.25K|  1.25K|  512 |  1.5K | 1.5K | 1.5K|  1.25K
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----
\t |OAM            | 1K  | 0K   |  0K  |  0K   |  0K   |  0K  |  4K   | 2K   | 1K  |  2K
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----
\t |Service-ACL    | 4K  | 4K   |  8K  |  8K   |  8K   |  4K  |  16K  | 16K  | 8K  |  8K
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----
\t |MPLS-Label     | 4K  | 0K   |  0K  |  0K   |  0K   |  0K  |  0K   | 8K   | 8K  |  8K
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----
\t |Statistics     | 16K | 16K  |  16K |  16K  |  16K  |  16K |  16K  | 16K  | 16K |  16K
\t |---------------|-----|------|------|-------|-------|------|-------|------|-----|----

*/

#ifndef _CTC_USW_FTM_H
#define _CTC_USW_FTM_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
 @addtogroup allocation FTM
 @{
*/

/**
 @brief Profile information

 @param[in] lchip    local chip id

 @param[in] ctc_profile_info  allocation profile information

 @remark[D2.TM] profile_type was assgint to select the profile type, refer to ctc_ftm_profile_type_t

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ftm_mem_alloc(uint8 lchip, ctc_ftm_profile_info_t* ctc_profile_info);

/**
 @brief Get hash polynomial

 @param[in] lchip    local chip id

 @param[in] hash_poly  hash polynomial info

 @remark[D2.TM] Get the current hash polynomial and the hash polynomial capability of the specific memory id

 @return CTC_E_XXX

*/

extern int32
ctc_usw_ftm_get_hash_poly(uint8 lchip, ctc_ftm_hash_poly_t* hash_poly);

/**
 @brief Set hash polynomial

 @param[in] lchip    local chip id

 @param[in] hash_poly  hash polynomial info

 @remark[D2.TM] Set a new hash polynomial to the specific memory id

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ftm_set_hash_poly(uint8 lchip, ctc_ftm_hash_poly_t* hash_poly);

/**
 @brief set spec Profile

 @param[in] lchip    local chip id

 @param[in] spec_type   spec type

 @param[in] value   profile value\

 @remark[D2.TM] Set chip capability with spec_type and value

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ftm_set_profile_specs(uint8 lchip, uint32 spec_type, uint32 value);

/**
 @brief get spec Profile

 @param[in] lchip    local chip id

 @param[in] spec_type   spec type

 @param[out] value   got the profile value

 @remark[D2.TM] Get capability with spec_type

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ftm_get_profile_specs(uint8 lchip, uint32 spec_type, uint32* value);

/**
 @brief Free Ftm Memory

 @param[in] lchip    local chip id

 @remark[D2.TM] deinit the ftm resource

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ftm_mem_free(uint8 lchip);

/**
 @brief Change memory profile

 @param[in] lchip    local chip id

 @param[in] p_profile    change profile information

 @remark[TM] Change memory profile according to user parameter

 @return CTC_E_XXX

*/
extern int32
ctc_usw_ftm_mem_change(uint8 lchip, ctc_ftm_change_profile_t* p_profile);
/**@} end of @addtogroup allocation FTM  */

#ifdef __cplusplus
}
#endif

#endif

