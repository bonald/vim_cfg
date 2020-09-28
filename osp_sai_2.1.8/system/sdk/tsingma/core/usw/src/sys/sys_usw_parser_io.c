#include "ctc_error.h"
#include "sys_usw_parser.h"
#include "sys_usw_parser_io.h"
#include "sys_usw_packet.h"
#include "drv_api.h"
/**
@brief The function is to set PARSER_PBB_CTL whole entry
*/
int32
sys_usw_parser_io_set_parser_pbb_ctl(uint8 lchip, ParserPbbCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(ParserPbbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_PBB_CTL whole entry
*/
int32
sys_usw_parser_io_get_parser_pbb_ctl(uint8 lchip, ParserPbbCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOR(ParserPbbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_PACKET_TYPE_TABLE whole entry
*/
int32
sys_usw_parser_io_set_parser_packet_type(uint8 lchip, ParserPacketTypeMap_m* p_map)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_map);
    cmd = DRV_IOW(ParserPacketTypeMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_map));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_ETHERNET_CTL whole entry
*/
int32
sys_usw_parser_io_set_parser_ethernet_ctl(uint8 lchip, ParserEthernetCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(ParserEthernetCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_ETHERNET_CTL whole entry
*/
int32
sys_usw_parser_io_get_parser_ethernet_ctl(uint8 lchip, ParserEthernetCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOR(ParserEthernetCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to set cvlan_tpid
*/
int32
sys_usw_parser_io_set_cvlan_tpid(uint8 lchip, uint32 tpid)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(ParserEthernetCtl_t, ParserEthernetCtl_cvlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(EpeL2TpidCtl_t, EpeL2TpidCtl_cvlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_cvlanTagTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(OamParserEtherCtl_t, OamParserEtherCtl_cvlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to get cvlan_tpid
*/
int32
sys_usw_parser_io_get_cvlan_tpid(uint8 lchip, uint32* p_tpid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_tpid);
    cmd = DRV_IOR(ParserEthernetCtl_t, ParserEthernetCtl_cvlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to set max_length_field
*/
int32
sys_usw_parser_io_set_max_length(uint8 lchip, uint32 len)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(ParserEthernetCtl_t, ParserEthernetCtl_maxLengthField_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &len));

    return CTC_E_NONE;
}

/**
@brief The function is to get max_length_field
*/
int32
sys_usw_parser_io_get_max_length(uint8 lchip, uint32* p_len)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_len);
    cmd = DRV_IOR(ParserEthernetCtl_t, ParserEthernetCtl_maxLengthField_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_len));

    return CTC_E_NONE;
}

/**
@brief The function is to set i_tag_tpid
*/
int32
sys_usw_parser_io_set_i_tag_tpid(uint8 lchip, uint32 tpid)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(EpeL2TpidCtl_t, EpeL2TpidCtl_iTagTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to get i_tag_tpid
*/
int32
sys_usw_parser_io_get_i_tag_tpid(uint8 lchip, uint32* p_tpid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_tpid);
    cmd = DRV_IOR(EpeL2TpidCtl_t, EpeL2TpidCtl_iTagTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to set cn_tag_tpid
*/
int32
sys_usw_parser_io_set_cn_tag_tpid(uint8 lchip, uint32 tpid)
{
    uint32 cmd = 0;

    if (!DRV_FLD_IS_EXISIT(ParserEthernetCtl_t, ParserEthernetCtl_cnTpid_f))
    {
        return CTC_E_NOT_SUPPORT;
    }

    cmd = DRV_IOW(ParserEthernetCtl_t, ParserEthernetCtl_cnTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to get cn_tag_tpid
*/
int32
sys_usw_parser_io_get_cn_tag_tpid(uint8 lchip, uint32* p_tpid)
{
    uint32 cmd = 0;
    CTC_PTR_VALID_CHECK(p_tpid);
    cmd = DRV_IOR(ParserEthernetCtl_t, ParserEthernetCtl_cnTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_tpid));
    return CTC_E_NONE;
}

/**
@brief The function is to set evb tpid
*/
int32
sys_usw_parser_io_set_evb_tpid(uint8 lchip, uint32 tpid)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(IpeMuxHeaderAdjustCtl_t, IpeMuxHeaderAdjustCtl_evbTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_evbTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

/*TBD confirm for usw*/
#if 0
    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_evbTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));
#endif
    return CTC_E_NONE;
}

/**
@brief The function is to get evb tpid
*/
int32
sys_usw_parser_io_get_evb_tpid(uint8 lchip, uint32* p_tpid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_tpid);
    cmd = DRV_IOR(IpeMuxHeaderAdjustCtl_t, IpeMuxHeaderAdjustCtl_evbTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to set bvlan_tpid
*/
int32
sys_usw_parser_io_set_bvlan_tpid(uint8 lchip, uint32 tpid)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(EpeL2TpidCtl_t, EpeL2TpidCtl_bvlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));
#if 0
    cmd = DRV_IOW(OamParserEtherCtl_t, OamParserEtherCtl_bvlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));
#endif
    return CTC_E_NONE;
}

/**
@brief The function is to get bvlan_tpid
*/
int32
sys_usw_parser_io_get_bvlan_tpid(uint8 lchip, uint32* p_tpid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_tpid);
    cmd = DRV_IOR(EpeL2TpidCtl_t, EpeL2TpidCtl_bvlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to set vlan_parsing_num
*/
int32
sys_usw_parser_io_set_vlan_parsing_num(uint8 lchip, uint32 num)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(ParserEthernetCtl_t, ParserEthernetCtl_vlanParsingNum_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &num));

    return CTC_E_NONE;
}

/**
@brief The function is to get vlan_parsing_num
*/
int32
sys_usw_parser_io_get_vlan_parsing_num(uint8 lchip, uint32* p_num)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_num);
    cmd = DRV_IOR(ParserEthernetCtl_t, ParserEthernetCtl_vlanParsingNum_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_num));

    return CTC_E_NONE;
}

/**
@brief The function is to set svlan_tpid0
*/
int32
sys_usw_parser_io_set_svlan_tpid0(uint8 lchip, uint32 tpid)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(ParserEthernetCtl_t, ParserEthernetCtl_array_0_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(EpeL2TpidCtl_t, EpeL2TpidCtl_array_0_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_array_0_svlanTagTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(OamParserEtherCtl_t, OamParserEtherCtl_gTpid_0_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    CTC_ERROR_RETURN(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_TPID, 0, tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to get svlan_tpid0
*/
int32
sys_usw_parser_io_get_svlan_tpid0(uint8 lchip, uint32* p_tpid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_tpid);
    cmd = DRV_IOR(ParserEthernetCtl_t, ParserEthernetCtl_array_0_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to set svlan_tpid1
*/
int32
sys_usw_parser_io_set_svlan_tpid1(uint8 lchip, uint32 tpid)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(ParserEthernetCtl_t, ParserEthernetCtl_array_1_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(EpeL2TpidCtl_t, EpeL2TpidCtl_array_1_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_array_1_svlanTagTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(OamParserEtherCtl_t, OamParserEtherCtl_gTpid_1_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    CTC_ERROR_RETURN(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_TPID, 1, tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to get svlan_tpid1
*/
int32
sys_usw_parser_io_get_svlan_tpid1(uint8 lchip, uint32* p_tpid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_tpid);
    cmd = DRV_IOR(ParserEthernetCtl_t, ParserEthernetCtl_array_1_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to set svlan_tpid2
*/
int32
sys_usw_parser_io_set_svlan_tpid2(uint8 lchip, uint32 tpid)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(ParserEthernetCtl_t, ParserEthernetCtl_array_2_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(EpeL2TpidCtl_t, EpeL2TpidCtl_array_2_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_array_2_svlanTagTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(OamParserEtherCtl_t, OamParserEtherCtl_gTpid_2_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    CTC_ERROR_RETURN(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_TPID, 2, tpid));
    return CTC_E_NONE;
}

/**
@brief The function is to get svlan_tpid2
*/
int32
sys_usw_parser_io_get_svlan_tpid2(uint8 lchip, uint32* p_tpid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_tpid);
    cmd = DRV_IOR(ParserEthernetCtl_t, ParserEthernetCtl_array_2_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to set svlan_tpid3
*/
int32
sys_usw_parser_io_set_svlan_tpid3(uint8 lchip, uint32 tpid)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(ParserEthernetCtl_t, ParserEthernetCtl_array_3_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(EpeL2TpidCtl_t, EpeL2TpidCtl_array_3_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_array_3_svlanTagTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    cmd = DRV_IOW(OamParserEtherCtl_t, OamParserEtherCtl_gTpid_3_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tpid));

    CTC_ERROR_RETURN(sys_usw_packet_tx_set_property(lchip, SYS_PKT_TX_TYPE_TPID, 3, tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to get svlan_tpid3
*/
int32
sys_usw_parser_io_get_svlan_tpid3(uint8 lchip, uint32* p_tpid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_tpid);
    cmd = DRV_IOR(ParserEthernetCtl_t, ParserEthernetCtl_array_3_svlanTpid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_tpid));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_LAYER2_PROTOCOL_CAM_VALID whole entry
*/
int32
sys_usw_parser_io_set_parser_layer2_protocol_cam_valid(uint8 lchip, ParserLayer2ProtocolCamValid_m* p_valid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_valid);
    cmd = DRV_IOW(ParserLayer2ProtocolCamValid_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_valid));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_LAYER2_PROTOCOL_CAM_VALID whole entry
*/
int32
sys_usw_parser_io_get_parser_layer2_protocol_cam_valid(uint8 lchip,
                                                              ParserLayer2ProtocolCamValid_m* p_valid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_valid);
    cmd = DRV_IOR(ParserLayer2ProtocolCamValid_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_valid));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_LAYER2_PROTOCOL_CAM whole entry
*/
int32
sys_usw_parser_io_set_parser_layer2_protocol_cam(uint8 lchip, ParserLayer2ProtocolCam_m* p_cam)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_cam);
    cmd = DRV_IOW(ParserLayer2ProtocolCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_cam));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_LAYER2_PROTOCOL_CAM whole entry
*/
int32
sys_usw_parser_io_get_parser_layer2_protocol_cam(uint8 lchip, ParserLayer2ProtocolCam_m* p_cam)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_cam);
    cmd = DRV_IOR(ParserLayer2ProtocolCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_cam));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_MPLS_CTL whole entry
*/
int32
sys_usw_parser_io_set_parser_ip_ctl(uint8 lchip, ParserIpCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(ParserIpCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_MPLS_CTL whole entry
*/
int32
sys_usw_parser_io_get_parser_ip_ctl(uint8 lchip, ParserIpCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOR(ParserIpCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_MPLS_CTL whole entry
*/
int32
sys_usw_parser_io_set_parser_mpls_ctl(uint8 lchip, ParserMplsCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(ParserMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_MPLS_CTL whole entry
*/
int32
sys_usw_parser_io_get_parser_mpls_ctl(uint8 lchip, ParserMplsCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOR(ParserMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_TRILL_CTL whole entry
*/
int32
sys_usw_parser_io_set_parser_trill_ctl(uint8 lchip, ParserTrillCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(ParserTrillCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_TRILL_CTL whole entry
*/
int32
sys_usw_parser_io_get_parser_trill_ctl(uint8 lchip, ParserTrillCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOR(ParserTrillCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_L3_HASH_CTL whole entry
*/
int32
sys_usw_parser_io_set_parser_l3_hash_ctl_entry(uint8 lchip, ParserLayer3HashCtl_m* p_parser_l3_hash_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_parser_l3_hash_ctl);
    cmd = DRV_IOW(ParserLayer3HashCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_parser_l3_hash_ctl));
    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_L3_HASH_CTL whole entry
*/
int32
sys_usw_parser_io_get_parser_l3_hash_ctl_entry(uint8 lchip, ParserLayer3HashCtl_m* p_parser_l3_hash_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_parser_l3_hash_ctl);
    cmd = DRV_IOR(ParserLayer3HashCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_parser_l3_hash_ctl));
    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_L3_CTL whole entry
*/
int32
sys_usw_parser_io_set_parser_layer3_type_ctl(uint8 lchip, ParserL3Ctl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(ParserL3Ctl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_L3_CTL whole entry
*/
int32
sys_usw_parser_io_get_parser_layer3_type_ctl(uint8 lchip, ParserL3Ctl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOR(ParserL3Ctl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_LAYER3_PROTOCOL_CAM_VALID whole entry
*/
int32
sys_usw_parser_io_set_parser_layer3_protocol_cam_valid(uint8 lchip,
                                                              ParserLayer3ProtocolCamValid_m* p_valid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_valid);
    cmd = DRV_IOW(ParserLayer3ProtocolCamValid_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_valid));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_LAYER3_PROTOCOL_CAM_VALID whole entry
*/
int32
sys_usw_parser_io_get_parser_layer3_protocol_cam_valid(uint8 lchip,
                                                              ParserLayer3ProtocolCamValid_m* p_valid)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_valid);
    cmd = DRV_IOR(ParserLayer3ProtocolCamValid_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_valid));
    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_LAYER3_PROTOCOL_CAM whole entry
*/
int32
sys_usw_parser_io_set_parser_layer3_protocol_cam(uint8 lchip,
                                                        ParserLayer3ProtocolCam_m* p_cam)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_cam);
    cmd = DRV_IOW(ParserLayer3ProtocolCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_cam));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_LAYER3_PROTOCOL_CAM whole entry
*/
int32
sys_usw_parser_io_get_parser_layer3_protocol_cam(uint8 lchip,
                                                        ParserLayer3ProtocolCam_m* p_cam)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_cam);
    cmd = DRV_IOR(ParserLayer3ProtocolCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_cam));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_LAYER3_PROTOCOL_CTL whole entry
*/
int32
sys_usw_parser_io_set_parser_layer3_protocol_ctl(uint8 lchip,
                                                        ParserLayer3ProtocolCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(ParserLayer3ProtocolCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));
    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_LAYER4_FLEX_CTL whole entry
*/
int32
sys_usw_parser_io_set_parser_layer4_flex_ctl(uint8 lchip, ParserLayer4FlexCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(ParserLayer4FlexCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_LAYER4_FLEX_CTL whole entry
*/
int32
sys_usw_parser_io_get_parser_layer4_flex_ctl(uint8 lchip, ParserLayer4FlexCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOR(ParserLayer4FlexCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_LAYER4_APP_CTL whole entry
*/
int32
sys_usw_parser_io_set_parser_layer4_app_ctl(uint8 lchip, ParserLayer4AppCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(ParserLayer4AppCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to get PARSER_LAYER4_APP_CTL whole entry
*/
int32
sys_usw_parser_io_get_parser_layer4_app_ctl(uint8 lchip, ParserLayer4AppCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOR(ParserLayer4AppCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to set PARSER_LAYER4_ACH_CTL whole entry
*/
int32
sys_usw_parser_io_set_parser_layer4_ach_ctl(uint8 lchip, ParserLayer4AchCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(ParserLayer4AchCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to set IpeUserIdHashCtl whole entry
*/
int32
sys_usw_parser_io_set_parser_outer_hash_ctl(uint8 lchip, IpeUserIdHashCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(IpeUserIdHashCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to get IpeUserIdHashCtl whole entry
*/
int32
sys_usw_parser_io_get_parser_outer_hash_ctl(uint8 lchip, IpeUserIdHashCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOR(IpeUserIdHashCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to set IpeAclGenHashKeyCtl whole entry
*/
int32
sys_usw_parser_io_set_parser_inner_hash_ctl(uint8 lchip, IpeAclGenHashKeyCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOW(IpeAclGenHashKeyCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}

/**
@brief The function is to get IpeAclGenHashKeyCtl whole entry
*/
int32
sys_usw_parser_io_get_parser_inner_hash_ctl(uint8 lchip, IpeAclGenHashKeyCtl_m* p_ctl)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_ctl);
    cmd = DRV_IOR(IpeAclGenHashKeyCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_ctl));

    return CTC_E_NONE;
}
/**
@brief The function is to set ParserUdfCamResult whole entry
*/
int32
sys_usw_parser_io_set_parser_udf_cam(uint8 lchip, ParserUdfCam_m* p_cam,uint8 index)
{
    uint32 cmd = 0;
    CTC_PTR_VALID_CHECK(p_cam);
    cmd = DRV_IOW(ParserUdfCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_cam));
    return CTC_E_NONE;
}

/**
@brief The function is to get ParserUdfCam whole entry
*/
int32
sys_usw_parser_io_get_parser_udf_cam(uint8 lchip, ParserUdfCam_m* p_cam,uint8 index)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_cam);
    cmd = DRV_IOR(ParserUdfCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_cam));
    return CTC_E_NONE;
}

/**
@brief The function is to set ParserUdfCamResult whole entry
*/
int32
sys_usw_parser_io_set_parser_udf_cam_result(uint8 lchip, ParserUdfCamResult_m* p_result,uint8 index)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_result);
    cmd = DRV_IOW(ParserUdfCamResult_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_result));
    return CTC_E_NONE;
}

/**
@brief The function is to get ParserUdfCamResult whole entry
*/
int32
sys_usw_parser_io_get_parser_udf_cam_result(uint8 lchip, ParserUdfCamResult_m* p_result,uint8 index)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_result);
    cmd = DRV_IOR(ParserUdfCamResult_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_result));
    return CTC_E_NONE;
}
