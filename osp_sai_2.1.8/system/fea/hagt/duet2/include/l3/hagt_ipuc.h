/****************************************************************************
 * hagt_ipuc.h :  Macro, data structure for hal agent ipuc  module
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision :   R0.01
 * Author   :   ychen
 * Date     :   2010-9-9
 ****************************************************************************/
#ifndef __HAGT_IPUC_H__
#define __HAGT_IPUC_H__

/***************************************************************************************************
 * Name         : hagt_ipuc_init 
 * Purpose      : init hagt ipuc module, malloc module memory and init       
 * Input        : pst_master: hagt master           
 * Output       : N/A              
 * Return       : HAGT_E_XXXX
 * Note         : N/A
***************************************************************************************************/
extern int32
hagt_ipuc_init (hagt_master_t* pst_master);
#endif /*__HAGT_IPUC_H__*/

