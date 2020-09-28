chipName=$1

:>`pwd`/ds.txt
echo "/**
 @file drv_tbl_macro.h

 @date 2010-3-10

 @version v2.0

*/

#ifndef _DRV_TBL_MACRO_H
#define _DRV_TBL_MACRO_H
#ifdef __cplusplus
extern \"C\" {
#endif

/****************************************************
                                   example
*****************************************************/

/*set filed value of specific table, which value is const uint32
--------------------------------------------------
SetDsNextHop4W(V, u1_g3_macDaMcastMode,     &dsnh4w,   0);
 -------------------------------------------------
*/

/*set filed addr of specific table, which value is point such as mac addr,ipv6 addr
--------------------------------------------------
SetDsNextHop4W(A, u1_g3_macDa,         &dsnh4w,   mac_da);
 -------------------------------------------------
*/

/* get filed value of specific table, which value is const uint32, this value will be return
--------------------------------------------------
   psys->fid = GetDsFibHost0MacHashKey(V, vsiId, p_dskey);
 -------------------------------------------------
*/

/* get filed addr of specific table, which value is point such as mac addr,ipv6 addr
--------------------------------------------------
   GetDsFibHost0MacHashKey(mappedMac, p_dskey, psys->mac);
 -------------------------------------------------
*/
" >> ds.txt
grep "DRV_DEF_D" ../driver/${chipName}/include/drv_ds.h | awk -F , '{sub(/ /,"",$3); printf ("#define Set%s(X, ...)        DRV_SET_FLD(X, %s, ##__VA_ARGS__)\n#define Get%s(X, ...)        DRV_GET_FLD(X, %s, ##__VA_ARGS__)\n",$3,$3,$3,$3)}' >> ds.txt

echo "
#ifdef __cplusplus
}
#endif

#endif

" >> ds.txt

cp ds.txt ../driver/${chipName}/include/drv_tbl_macro.h
