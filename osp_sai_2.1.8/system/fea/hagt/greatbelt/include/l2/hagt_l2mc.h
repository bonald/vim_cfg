#ifndef __HAGT_L2MC_H__
#define __HAGT_L2MC_H__
/*****************************************************************************************************
 * Name    : hagt_l2mc_init
 * Purpose : init l2mc in hagt 
             call from nsm
 * Input   : pst_hagt_master : pointer of hagt master structure
 * Output  : N/A
 * Return  : HAGT_E_***  - hal agent return type
 * Note    : N/A
******************************************************************************************************/
int32
hagt_l2mc_init(hagt_master_t* pst_hagt_master);

#endif /* __HAGT_L2MC_H__ */
