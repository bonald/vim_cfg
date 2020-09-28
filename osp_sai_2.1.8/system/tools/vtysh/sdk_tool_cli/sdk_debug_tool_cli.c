
//#include <zebra.h>
#include "command.h"
#include "vtysh.h"

CTC_CLI(ctc_cli_dbg_tool_show_bsr_BufRetrv,
   ctc_cli_dbg_tool_show_bsr_BufRetrv_cmd,
   "show statistic (bsr) (bufretrv) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "bsr status",
   "show BufRetrv statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_bsr_BufStore,
   ctc_cli_dbg_tool_show_bsr_BufStore_cmd,
   "show statistic (bsr) (bufstore) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "bsr status",
   "show BufStore statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_mac_CpuMac,
   ctc_cli_dbg_tool_show_mac_CpuMac_cmd,
   "show statistic (mac) (cpumac) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "mac status",
   "show CpuMac statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_Ds_TCAM,
   ctc_cli_dbg_tool_show_share_Ds_TCAM_cmd,
   "show statistic (share) (ds&tcam) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show Ds&TCAM statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_DsAging,
   ctc_cli_dbg_tool_show_share_DsAging_cmd,
   "show statistic (share) (dsaging) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show DsAging statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_DynamicDs,
   ctc_cli_dbg_tool_show_share_DynamicDs_cmd,
   "show statistic (share) (dynamicds) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show DynamicDs statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_epe_ELoop,
   ctc_cli_dbg_tool_show_epe_ELoop_cmd,
   "show statistic (epe) (eloop) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "epe status",
   "show ELoop statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_epe_EpeAclQos,
   ctc_cli_dbg_tool_show_epe_EpeAclQos_cmd,
   "show statistic (epe) (epeaclqos) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "epe status",
   "show EpeAclQos statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_epe_EpeCla,
   ctc_cli_dbg_tool_show_epe_EpeCla_cmd,
   "show statistic (epe) (epecla) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "epe status",
   "show EpeCla statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_epe_EpeHdrAdj,
   ctc_cli_dbg_tool_show_epe_EpeHdrAdj_cmd,
   "show statistic (epe) (epehdradj) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "epe status",
   "show EpeHdrAdj statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_epe_EpeHdrEdit,
   ctc_cli_dbg_tool_show_epe_EpeHdrEdit_cmd,
   "show statistic (epe) (epehdredit) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "epe status",
   "show EpeHdrEdit statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_epe_EpeHdrProc,
   ctc_cli_dbg_tool_show_epe_EpeHdrProc_cmd,
   "show statistic (epe) (epehdrproc) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "epe status",
   "show EpeHdrProc statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_epe_EpeNextHop,
   ctc_cli_dbg_tool_show_epe_EpeNextHop_cmd,
   "show statistic (epe) (epenexthop) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "epe status",
   "show EpeNextHop statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_epe_EpeOam,
   ctc_cli_dbg_tool_show_epe_EpeOam_cmd,
   "show statistic (epe) (epeoam) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "epe status",
   "show EpeOam statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_GlobalStats,
   ctc_cli_dbg_tool_show_misc_GlobalStats_cmd,
   "show statistic (misc) (globalstats) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show GlobalStats statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_GreatbeltSup,
   ctc_cli_dbg_tool_show_misc_GreatbeltSup_cmd,
   "show statistic (misc) (greatbeltsup) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show GreatbeltSup statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_HashDs,
   ctc_cli_dbg_tool_show_share_HashDs_cmd,
   "show statistic (share) (hashds) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show HashDs statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_I2CMaster,
   ctc_cli_dbg_tool_show_misc_I2CMaster_cmd,
   "show statistic (misc) (i2cmaster) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show I2CMaster statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_IntLkIntf,
   ctc_cli_dbg_tool_show_misc_IntLkIntf_cmd,
   "show statistic (misc) (intlkintf) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show IntLkIntf statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_ipe_IpeFib,
   ctc_cli_dbg_tool_show_ipe_IpeFib_cmd,
   "show statistic (ipe) (ipefib) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "ipe status",
   "show IpeFib statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_ipe_IpeForward,
   ctc_cli_dbg_tool_show_ipe_IpeForward_cmd,
   "show statistic (ipe) (ipeforward) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "ipe status",
   "show IpeForward statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_ipe_IpeHdrAdj,
   ctc_cli_dbg_tool_show_ipe_IpeHdrAdj_cmd,
   "show statistic (ipe) (ipehdradj) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "ipe status",
   "show IpeHdrAdj statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_ipe_IpeIntfMap,
   ctc_cli_dbg_tool_show_ipe_IpeIntfMap_cmd,
   "show statistic (ipe) (ipeintfmap) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "ipe status",
   "show IpeIntfMap statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_ipe_IpeLkupMgr,
   ctc_cli_dbg_tool_show_ipe_IpeLkupMgr_cmd,
   "show statistic (ipe) (ipelkupmgr) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "ipe status",
   "show IpeLkupMgr statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_ipe_IpePktProc,
   ctc_cli_dbg_tool_show_ipe_IpePktProc_cmd,
   "show statistic (ipe) (ipepktproc) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "ipe status",
   "show IpePktProc statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_MacLedDriver,
   ctc_cli_dbg_tool_show_misc_MacLedDriver_cmd,
   "show statistic (misc) (macleddriver) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show MacLedDriver statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_mac_MacMux,
   ctc_cli_dbg_tool_show_mac_MacMux_cmd,
   "show statistic (mac) (macmux) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "mac status",
   "show MacMux statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_Mdio,
   ctc_cli_dbg_tool_show_misc_Mdio_cmd,
   "show statistic (misc) (mdio) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show Mdio statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_bsr_MetFifo,
   ctc_cli_dbg_tool_show_bsr_MetFifo_cmd,
   "show statistic (bsr) (metfifo) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "bsr status",
   "show MetFifo statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_ipe_NetRx,
   ctc_cli_dbg_tool_show_ipe_NetRx_cmd,
   "show statistic (ipe) (netrx) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "ipe status",
   "show NetRx statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_epe_NetTx,
   ctc_cli_dbg_tool_show_epe_NetTx_cmd,
   "show statistic (epe) (nettx) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "epe status",
   "show NetTx statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_oam_OamAutoGenPkt,
   ctc_cli_dbg_tool_show_oam_OamAutoGenPkt_cmd,
   "show statistic (oam) (oamautogenpkt) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "oam status",
   "show OamAutoGenPkt statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_oam_OamFwd,
   ctc_cli_dbg_tool_show_oam_OamFwd_cmd,
   "show statistic (oam) (oamfwd) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "oam status",
   "show OamFwd statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_oam_OamParser,
   ctc_cli_dbg_tool_show_oam_OamParser_cmd,
   "show statistic (oam) (oamparser) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "oam status",
   "show OamParser statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_oam_OamProc,
   ctc_cli_dbg_tool_show_oam_OamProc_cmd,
   "show statistic (oam) (oamproc) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "oam status",
   "show OamProc statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_OobFc,
   ctc_cli_dbg_tool_show_misc_OobFc_cmd,
   "show statistic (misc) (oobfc) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show OobFc statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_Parser,
   ctc_cli_dbg_tool_show_share_Parser_cmd,
   "show statistic (share) (parser) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show Parser statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_PbCtl,
   ctc_cli_dbg_tool_show_misc_PbCtl_cmd,
   "show statistic (misc) (pbctl) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show PbCtl statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_PciExpCore,
   ctc_cli_dbg_tool_show_misc_PciExpCore_cmd,
   "show statistic (misc) (pciexpcore) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show PciExpCore statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_Policing,
   ctc_cli_dbg_tool_show_share_Policing_cmd,
   "show statistic (share) (policing) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show Policing statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_PtpEngine,
   ctc_cli_dbg_tool_show_misc_PtpEngine_cmd,
   "show statistic (misc) (ptpengine) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show PtpEngine statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_PtpFrc,
   ctc_cli_dbg_tool_show_misc_PtpFrc_cmd,
   "show statistic (misc) (ptpfrc) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show PtpFrc statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_bsr_QMgrDeq,
   ctc_cli_dbg_tool_show_bsr_QMgrDeq_cmd,
   "show statistic (bsr) (qmgrdeq) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "bsr status",
   "show QMgrDeq statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_bsr_QMgrEnq,
   ctc_cli_dbg_tool_show_bsr_QMgrEnq_cmd,
   "show statistic (bsr) (qmgrenq) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "bsr status",
   "show QMgrEnq statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_bsr_QMgrQueEntry,
   ctc_cli_dbg_tool_show_bsr_QMgrQueEntry_cmd,
   "show statistic (bsr) (qmgrqueentry) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "bsr status",
   "show QMgrQueEntry statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_Qsgmii,
   ctc_cli_dbg_tool_show_misc_Qsgmii_cmd,
   "show statistic (misc) (qsgmii) (index INDEX) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show Qsgmii statistic",
   "Index",
   "<0-11>",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_mac_QuadMac,
   ctc_cli_dbg_tool_show_mac_QuadMac_cmd,
   "show statistic (mac) (quadmac) (index INDEX) (sub-index SUB_INDEX|) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "mac status",
   "show QuadMac statistic",
   "QuadMac index",
   "<0-11>",
   "QuadMac internal sub index",
   "<0-3>",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_misc_QuadPcs,
   ctc_cli_dbg_tool_show_misc_QuadPcs_cmd,
   "show statistic (misc) (quadpcs) (index INDEX) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "misc status",
   "show QuadPcs statistic",
   "Index",
   "<0-5>",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_mac_Sgmac,
   ctc_cli_dbg_tool_show_mac_Sgmac_cmd,
   "show statistic (mac) (sgmac) (index INDEX) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "mac status",
   "show Sgmac statistic",
   "Index",
   "<0-11>",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_SharedDs,
   ctc_cli_dbg_tool_show_share_SharedDs_cmd,
   "show statistic (share) (sharedds) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show SharedDs statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_TcamCtlInt,
   ctc_cli_dbg_tool_show_share_TcamCtlInt_cmd,
   "show statistic (share) (tcamctlint) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show TcamCtlInt statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_TcamDs,
   ctc_cli_dbg_tool_show_share_TcamDs_cmd,
   "show statistic (share) (tcamds) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show TcamDs statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_DsFib,
   ctc_cli_dbg_tool_show_share_DsFib_cmd,
   "show statistic (share) (dsfib) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show DsFib statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_DynamicDsMem,
   ctc_cli_dbg_tool_show_share_DynamicDsMem_cmd,
   "show statistic (share) (dynamicdsmem) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show DynamicDsMem statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_bsr_FwdMsg,
   ctc_cli_dbg_tool_show_bsr_FwdMsg_cmd,
   "show statistic (bsr) (fwdmsg) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "bsr status",
   "show FwdMsg statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_HashDsMem,
   ctc_cli_dbg_tool_show_share_HashDsMem_cmd,
   "show statistic (share) (hashdsmem) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show HashDsMem statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_share_TcamDsMem,
   ctc_cli_dbg_tool_show_share_TcamDsMem_cmd,
   "show statistic (share) (tcamdsmem) (lchip CHIP_ID|)",
   "show chip statistic info cmd",
   "ipe/bsr/mac/misc/oam/share status info",
   "share status",
   "show TcamDsMem statistic",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_show_bus,
   ctc_cli_dbg_tool_show_bus_cmd,
   "show bus ((ipe learning2fwd) |(ipe pkt-process2fwd) |(ipe hdr-adjust2inf-mapper) |(ipe parser2inf-mapper) |(ipe inf-mapper2lkp-mgr) |(ipe parser2lkp-mgr) |(ipe pkt-process-ack2oam) |(ipe inf-mapper2oam) |(ipe lkp-mgr2oam) |(ipe inf-mapper2pkt-process) |(ipe scl2pkt-process) |(ipe lkp-mgr2pkt-process) |(bsr bufretrieve2pkt-buffer-track) |(bsr ipe-hdr2metfifo) |(bsr bufstore2metfifo) |(bsr bufretrieve2metfifo) |(bsr qmgr2metfifo) |(bsr enqueue-msg) |(bsr dequeue-msg2enqueue-msg) |(epe hdr-process2acl-qos) |(epe tcam-ack2acl) |(epe classfiy2policer) |(epe bufretrieve2hdr-adjust) |(epe hdr-process2hdr-edit) |(epe oam2hdr-edit) |(epe hdr-adjust2hdr-process) |(epe nh-mapper2hdr-process) |(epe payload-process2layer3-edit) |(epe hdr-adjust2nh-mapper) |(epe parser2nh-mapper) |(epe hdr-process2oam) |(epe classfiy2oam) |(fib lkpmgr-lkp-key-input) |(fib pkt-process-lkp-key-output) |(fib fib-ad-output) |(fib track-fifo) |(lpm lkp-mgr-key-input) |(lpm conflict-tcam-key-input) |(lpm hash-key-output) |(lpm conflict-tcam-key-output) |(lpm pipeline-request-fifo) |(lpm ad-output) |(lpm track) |(tcam fifo) |(oam hdr-edit) | (oam rx-proc)) index INDEX (lchip CHIP_ID|)",
   "show chip info cmd",
   "bus info",
   "ipe bus",
   "bus name",
   "ipe bus",
   "bus name",
   "ipe bus",
   "bus name",
   "ipe bus",
   "bus name",
   "ipe bus",
   "bus name",
   "ipe bus",
   "bus name",
   "ipe bus",
   "bus name",
   "ipe bus",
   "bus name",
   "ipe bus",
   "bus name",
   "ipe bus",
   "bus name",
   "ipe bus",
   "bus name",
   "ipe bus",
   "bus name",
   "bsr bus",
   "bus name",
   "bsr bus",
   "bus name",
   "bsr bus",
   "bus name",
   "bsr bus",
   "bus name",
   "bsr bus",
   "bus name",
   "bsr bus",
   "bus name",
   "bsr bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "epe bus",
   "bus name",
   "fib bus",
   "bus name",
   "fib bus",
   "bus name",
   "fib bus",
   "bus name",
   "fib bus",
   "bus name",
   "lpm bus",
   "bus name",
   "lpm bus",
   "bus name",
   "lpm bus",
   "bus name",
   "lpm bus",
   "bus name",
   "lpm bus",
   "bus name",
   "lpm bus",
   "bus name",
   "lpm bus",
   "bus name",
   "tcam bus",
   "bus name",
   "oam bus",
   "bus name",
   "oam bus",
   "bus name",
   "bus index",
   "index num",
   "Chip id on linecard",
   "<0-1>")
{
    return CLI_SUCCESS; 
}

CTC_CLI(ctc_cli_dbg_tool_exec_cfg,
ctc_cli_dbg_tool_exec_cfg_cmd,
"execute chip CHIP_ID FILE",
"execute cfg cmd",
"chip",
"chip id value",
"cfg File name")
{
    return CLI_SUCCESS; 
}

int32
sdk_debug_tool_cli_init(uint8 cli_tree_mode)
{
    install_element(1, &ctc_cli_dbg_tool_show_bsr_BufRetrv_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_bsr_BufStore_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_mac_CpuMac_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_Ds_TCAM_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_DsAging_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_DynamicDs_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_epe_ELoop_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_epe_EpeAclQos_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_epe_EpeCla_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_epe_EpeHdrAdj_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_epe_EpeHdrEdit_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_epe_EpeHdrProc_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_epe_EpeNextHop_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_epe_EpeOam_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_GlobalStats_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_GreatbeltSup_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_HashDs_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_I2CMaster_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_IntLkIntf_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_ipe_IpeFib_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_ipe_IpeForward_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_ipe_IpeHdrAdj_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_ipe_IpeIntfMap_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_ipe_IpeLkupMgr_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_ipe_IpePktProc_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_MacLedDriver_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_mac_MacMux_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_Mdio_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_bsr_MetFifo_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_ipe_NetRx_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_epe_NetTx_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_oam_OamAutoGenPkt_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_oam_OamFwd_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_oam_OamParser_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_oam_OamProc_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_OobFc_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_Parser_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_PbCtl_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_PciExpCore_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_Policing_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_PtpEngine_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_PtpFrc_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_bsr_QMgrDeq_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_bsr_QMgrEnq_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_bsr_QMgrQueEntry_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_Qsgmii_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_mac_QuadMac_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_misc_QuadPcs_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_mac_Sgmac_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_SharedDs_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_TcamCtlInt_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_TcamDs_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_DsFib_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_DynamicDsMem_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_bsr_FwdMsg_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_HashDsMem_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_share_TcamDsMem_cmd);
    install_element(1, &ctc_cli_dbg_tool_show_bus_cmd);
    install_element(1, &ctc_cli_dbg_tool_exec_cfg_cmd);
    
    return CLI_SUCCESS;
}

