#ifndef _SYS_USW_PARSER_IO_H_
#define _SYS_USW_PARSER_IO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "sal.h"
#include "ctc_parser.h"
#include "drv_api.h"

extern int32
sys_usw_parser_io_set_parser_pbb_ctl(uint8 lchip, ParserPbbCtl_m* p_ctl);

extern int32
sys_usw_parser_io_get_parser_pbb_ctl(uint8 lchip, ParserPbbCtl_m* p_ctl);

extern int32
sys_usw_parser_io_set_parser_packet_type(uint8 lchip, ParserPacketTypeMap_m* p_map);

extern int32
sys_usw_parser_io_set_parser_ethernet_ctl(uint8 lchip, ParserEthernetCtl_m* p_ctl);

extern int32
sys_usw_parser_io_get_parser_ethernet_ctl(uint8 lchip, ParserEthernetCtl_m* p_ctl);

extern int32
sys_usw_parser_io_set_cvlan_tpid(uint8 lchip, uint32 tpid);

extern int32
sys_usw_parser_io_get_cvlan_tpid(uint8 lchip, uint32* p_tpid);

extern int32
sys_usw_parser_io_set_max_length(uint8 lchip, uint32 len);

extern int32
sys_usw_parser_io_get_max_length(uint8 lchip, uint32* p_len);

extern int32
sys_usw_parser_io_set_i_tag_tpid(uint8 lchip, uint32 tpid);

extern int32
sys_usw_parser_io_get_i_tag_tpid(uint8 lchip, uint32* p_tpid);

extern int32
sys_usw_parser_io_set_cn_tag_tpid(uint8 lchip, uint32 tpid);

extern int32
sys_usw_parser_io_get_cn_tag_tpid(uint8 lchip, uint32* p_tpid);

extern int32
sys_usw_parser_io_set_bvlan_tpid(uint8 lchip, uint32 tpid);

extern int32
sys_usw_parser_io_get_bvlan_tpid(uint8 lchip, uint32* p_tpid);

extern int32
sys_usw_parser_io_set_vlan_parsing_num(uint8 lchip, uint32 num);

extern int32
sys_usw_parser_io_get_vlan_parsing_num(uint8 lchip, uint32* p_num);

extern int32
sys_usw_parser_io_set_svlan_tpid0(uint8 lchip, uint32 tpid);

extern int32
sys_usw_parser_io_get_svlan_tpid0(uint8 lchip, uint32* p_tpid);

extern int32
sys_usw_parser_io_set_svlan_tpid1(uint8 lchip, uint32 tpid);

extern int32
sys_usw_parser_io_get_svlan_tpid1(uint8 lchip, uint32* p_tpid);

extern int32
sys_usw_parser_io_set_svlan_tpid2(uint8 lchip, uint32 tpid);

extern int32
sys_usw_parser_io_get_svlan_tpid2(uint8 lchip, uint32* p_tpid);

extern int32
sys_usw_parser_io_set_svlan_tpid3(uint8 lchip, uint32 tpid);

extern int32
sys_usw_parser_io_get_svlan_tpid3(uint8 lchip, uint32* p_tpid);

extern int32
sys_usw_parser_io_set_evb_tpid(uint8 lchip, uint32 tpid);

extern int32
sys_usw_parser_io_get_evb_tpid(uint8 lchip, uint32* p_tpid);

extern int32
sys_usw_parser_io_set_parser_layer2_protocol_cam_valid(uint8 lchip, ParserLayer2ProtocolCamValid_m* p_valid);

extern int32
sys_usw_parser_io_get_parser_layer2_protocol_cam_valid(uint8 lchip, ParserLayer2ProtocolCamValid_m* p_valid);

extern int32
sys_usw_parser_io_set_parser_layer2_protocol_cam(uint8 lchip, ParserLayer2ProtocolCam_m* p_cam);

extern int32
sys_usw_parser_io_get_parser_layer2_protocol_cam(uint8 lchip, ParserLayer2ProtocolCam_m* p_cam);

extern int32
sys_usw_parser_io_set_parser_ip_ctl(uint8 lchip, ParserIpCtl_m* p_ctl);

extern int32
sys_usw_parser_io_get_parser_ip_ctl(uint8 lchip, ParserIpCtl_m* p_ctl);

extern int32
sys_usw_parser_io_set_parser_mpls_ctl(uint8 lchip, ParserMplsCtl_m* p_ctl);

extern int32
sys_usw_parser_io_get_parser_mpls_ctl(uint8 lchip, ParserMplsCtl_m* p_ctl);

extern int32
sys_usw_parser_io_set_parser_trill_ctl(uint8 lchip, ParserTrillCtl_m* p_ctl);

extern int32
sys_usw_parser_io_get_parser_trill_ctl(uint8 lchip, ParserTrillCtl_m* p_ctl);

extern int32
sys_usw_parser_io_set_parser_l3_hash_ctl_entry(uint8 lchip, ParserLayer3HashCtl_m* p_parser_l3_hash_ctl);

extern int32
sys_usw_parser_io_get_parser_l3_hash_ctl_entry(uint8 lchip, ParserLayer3HashCtl_m* p_parser_l3_hash_ctl);

extern int32
sys_usw_parser_io_set_parser_layer3_type_ctl(uint8 lchip, ParserL3Ctl_m* p_ctl);

extern int32
sys_usw_parser_io_get_parser_layer3_type_ctl(uint8 lchip, ParserL3Ctl_m* p_ctl);

extern int32
sys_usw_parser_io_set_parser_layer3_protocol_cam_valid(uint8 lchip, ParserLayer3ProtocolCamValid_m* p_valid);

extern int32
sys_usw_parser_io_get_parser_layer3_protocol_cam_valid(uint8 lchip, ParserLayer3ProtocolCamValid_m* p_valid);

extern int32
sys_usw_parser_io_set_parser_layer3_protocol_cam(uint8 lchip, ParserLayer3ProtocolCam_m* p_cam);

extern int32
sys_usw_parser_io_get_parser_layer3_protocol_cam(uint8 lchip, ParserLayer3ProtocolCam_m* p_cam);

extern int32
sys_usw_parser_io_set_parser_layer3_protocol_ctl(uint8 lchip, ParserLayer3ProtocolCtl_m* p_ctl);

extern int32
sys_usw_parser_io_set_parser_layer4_flex_ctl(uint8 lchip, ParserLayer4FlexCtl_m* p_ctl);

extern int32
sys_usw_parser_io_get_parser_layer4_flex_ctl(uint8 lchip, ParserLayer4FlexCtl_m* p_ctl);

extern int32
sys_usw_parser_io_set_parser_layer4_app_ctl(uint8 lchip, ParserLayer4AppCtl_m* p_ctl);

extern int32
sys_usw_parser_io_get_parser_layer4_app_ctl(uint8 lchip, ParserLayer4AppCtl_m* p_ctl);

extern int32
sys_usw_parser_io_set_parser_layer4_ach_ctl(uint8 lchip, ParserLayer4AchCtl_m* p_ctl);

extern int32
sys_usw_parser_io_set_parser_outer_hash_ctl(uint8 lchip, IpeUserIdHashCtl_m* p_ctl);

extern int32
sys_usw_parser_io_get_parser_outer_hash_ctl(uint8 lchip, IpeUserIdHashCtl_m* p_ctl);

extern int32
sys_usw_parser_io_set_parser_inner_hash_ctl(uint8 lchip, IpeAclGenHashKeyCtl_m* p_ctl);

extern int32
sys_usw_parser_io_get_parser_inner_hash_ctl(uint8 lchip, IpeAclGenHashKeyCtl_m* p_ctl);
extern int32
sys_usw_parser_io_set_parser_udf_cam(uint8 lchip, ParserUdfCam_m* p_cam,uint8 index);

extern int32
sys_usw_parser_io_get_parser_udf_cam(uint8 lchip, ParserUdfCam_m* p_cam,uint8 index);

extern int32
sys_usw_parser_io_set_parser_udf_cam_result(uint8 lchip, ParserUdfCamResult_m* p_result,uint8 index);

extern int32
sys_usw_parser_io_get_parser_udf_cam_result(uint8 lchip, ParserUdfCamResult_m* p_result,uint8 index);

#ifdef __cplusplus
}
#endif

#endif

