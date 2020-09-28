/****************************************************************************
*  Centec Bridge information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liwh
* Date          : 2010-07-19 
* Reason        : First Create.
****************************************************************************/
#ifndef __HAGT_BRIDGE_H__
#define __HAGT_BRIDGE_H__

/****************************************************************************
*  
* Function
*
*****************************************************************************/

/*****************************************************************************************************
 * Name    : hagt_bridge_init
 * Purpose : This function used to init hagt bridge
 * Input   : hagt_master_t* p_hagt_master
 * Output  : N/A
 * Return  : hsrv_e_***  - hal server return type
 * Note    : N/A
******************************************************************************************************/
int32 hagt_bridge_init(hagt_master_t* p_hagt_master);

/*******************************************************************************************
 * Name         : hagt_bridge_get_learning_enable
 * Purpose      : This function used to get learning enable
 * Input        : N/A
 * Output       : N/A
 * Return       : hsrv_e_***  - hal server return type
 * Note         : N/A
********************************************************************************************/
bool 
hagt_bridge_get_learning_enable(void);

int32
hagt_bridge_dma_learning(uint8 lchip, void* p_dma_learn, uint8 entry_num);
#endif
