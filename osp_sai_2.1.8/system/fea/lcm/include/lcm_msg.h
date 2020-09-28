#ifndef __LCM_STACKTLK_H__
#define __LCM_STACKTLK_H__

#include "glb_hw_define.h"
#include "LcmMsg.h"

int32 lcm_msg_tx_lcChsmKeepaliveAck(void);

int32 lcm_msg_tx_lcPortInfo(glb_port_t* p_port, int tx_attribute);
int32 lcm_msg_tx_lcFiberAttach(glb_port_t *p_port, int tx_attribute);
int32 lcm_msg_tx_lcFiberDeAttach(glb_port_t *p_port,int tx_attribute);
int32 lcm_msg_tx_lcFiberUpdate(glb_port_t *p_port, int tx_attribute);
int32 lcm_msg_tx_lcUptimeGetAck(int tx_attribute);
int32 lcm_msg_tx_lcTmprThreshold(int32 low, int32 high, int32 crit, int tx_attribute);
int32 lcm_msg_tx_lcOemInfo(int tx_attribute);
int32 lcm_msg_tx_lcChsmTmprGetAck(int tx_attribute);
int32 lcm_msg_tx_lcchsmGetFanStatusAck(int tx_attribute);
int32 lcm_msg_tx_lcchsmGetPsuStatusAck(int tx_attribute);
int32 lcm_msg_tx_lcchsmGetVcmCfgAck(int tx_attribute);
int32 lcm_msg_tx_lcchsmGetVcmStatusAck(int tx_attribute);
int32 lcm_msg_tx_lcChsmVCT(uint32 port_id, glb_port_cable_info_t* p_cable_info, int tx_attribute);
int32 lcm_msg_tx_lcPortSpeedMode(glb_port_t* p_port, int tx_attribute);
int32 lcm_msg_tx_lcRebootInfo(int tx_attribute);
int32 lcm_msg_tx_lcchsmReset(int tx_attribute);
int32 lcm_msg_tx_lcchsmDyingGasp(int tx_attribute);
int32 lcm_msg_tx_lcOnline(int tx_attribute);

int32 lcm_msg_rx_chsmLcAttachAck(LcmMsg_t* msg);

int32 lcm_msg_rx_chsmReloadLc(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmTmprThreshold(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmOemInfoSet(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmOemInfoGet(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmPortInfoSet(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmLedSet(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmTmprGet(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmGetFanStatus(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmGetPsuStatus(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmGetVcmCfg(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmGetVcmStatus(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmResetRebootInfo(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmSetVctMod(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmConfigSpeedMode(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmUpdatebootrom(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmUpdateEpld(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmLoadPhyFirmware(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmConfigbootcmd(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmMgmtCfgSet(LcmMsg_t *msg);
int32 lcm_msg_rx_chsmFanSpeedRate(LcmMsg_t *msg);
#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
int32 lcm_msg_rx_chsmSmartCfgSet(LcmMsg_t *msg);
#endif
int32 lcm_msg_rx_chsmSetSwVer(LcmMsg_t *msg);

#endif
