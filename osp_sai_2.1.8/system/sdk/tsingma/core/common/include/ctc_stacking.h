/**
 @file ctc_stacking.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-11

 @version v2.0

   This file contains all stacking related data structure, enum, macro and proto.
*/

#ifndef _CTC_STK_H
#define _CTC_STK_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common/include/ctc_const.h"
#include "common/include/ctc_packet.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/

#define CTC_STK_MAX_TRUNK_ID  63
#define CTC_STK_TRUNK_BMP_NUM   ((CTC_STK_MAX_TRUNK_ID + CTC_UINT32_BITS) / CTC_UINT32_BITS)
/**
 @defgroup stacking STACKING
 @{
*/

/**
 @brief  define stacking trunk load balance mode
*/
enum ctc_stacking_load_mode_e
{
    CTC_STK_LOAD_STATIC,          /**< [GB.GG.D2.TM]stacking uplink trunk use static load balance by packetHash%trunkMembers */
    CTC_STK_LOAD_DYNAMIC,         /**< [GB.GG.D2.TM]stacking uplink trunk use dynamic load balance by monitor traffic from the trunk member */
    CTC_STK_LOAD_STATIC_FAILOVER, /**< [GB.GG.D2.TM]stacking uplink trunk use failover to automatic switch to other member when member is linkdown */
    CTC_STK_LOAD_MODE_MAX
};
typedef enum ctc_stacking_load_mode_e ctc_stacking_load_mode_t;

/**
 @brief  define stacking trunk cloud header type
*/
enum ctc_stacking_hdr_flag_e
{
    CTC_STK_HDR_WITH_NONE,        /**< [GB.GG.D2.TM]stacking without cloud header*/
    CTC_STK_HDR_WITH_L2,          /**< [GB.GG.D2.TM]stacking with L2 cloud header*/
    CTC_STK_HDR_WITH_L2_AND_IPV4, /**< [GB.GG.D2]stacking with L2 + IPv4 cloud header*/
    CTC_STK_HDR_WITH_L2_AND_IPV6, /**< [GB.GG.D2]stacking with L2 + IPv6 cloud header*/
    CTC_STK_HDR_WITH_IPV4,        /**< [GB.GG.D2]stacking with IPv4 cloud header*/
    CTC_STK_HDR_WITH_IPV6         /**< [GB.GG.D2]stacking with IPv6 cloud header*/
};
typedef enum ctc_stacking_hdr_flag_e ctc_stacking_hdr_flag_t;

/**
 @brief  define stacking port configuration data struct
*/
struct ctc_stacking_port_cfg_s
{
    uint32  gport;                         /**< [D2.TM] Gport */
    uint8   direction;                     /**< [D2.TM] direction, refer to ctc_direction_t*/
    uint8   hdr_type;                      /**< [D2.TM] encap cloud header type, refer to ctc_stacking_hdr_flag_t*/
    uint8   enable;                        /**< [D2.TM] 0:diable stacking port property, 1: enable stacking port property */
    uint8   rsv;
};
typedef struct ctc_stacking_port_cfg_s ctc_stacking_port_cfg_t;

/**
 @brief  define stacking property value
*/
enum ctc_stacking_prop_type_e
{
    CTC_STK_PROP_GLOBAL_CONFIG,           /**< [GB.GG.D2.TM]set stacking global confiure, p_value, point to ctc_stacking_glb_cfg_t*/
    CTC_STK_PROP_MCAST_STOP_RCHIP,        /**< [GB.GG.D2.TM]set mcast stop , p_value, point to ctc_stacking_stop_rchip_t*/
    CTC_STK_PROP_BREAK_EN,                /**< [GB.GG.D2.TM]if stacking exist break point, set enable(value = 1) or disable(value = 0)*/
    CTC_STK_PROP_FULL_MESH_EN,            /**< [GB.GG.D2.TM]if stacking topo is fullmesh, set enable(value = 1) or disable(value = 0)*/
    CTC_STK_PROP_MCAST_BREAK_POINT,       /**< [GB.GG.D2.TM]Set mcast break point, use for ring topo, p_value, point to ctc_stacking_mcast_break_point_t*/
    CTC_STK_PROP_PORT_BIND_TRUNK,         /**< [GB.GG.D2.TM] Port bind trunk which used for trunk check, p_value, point to ctc_stacking_bind_trunk_t*/
    CTC_STK_PROP_RCHIP_PORT_EN,           /**< [GB.GG.D2.TM]Remote chip ability, p_value, point to ctc_stacking_rchip_port_t*/
    CTC_STK_PROP_STK_PORT_EN,             /**< [D2.TM] Set stacking port enable, refer to ctc_stacking_port_cfg_t*/
    CTC_STK_PROP_MAX
};
typedef enum ctc_stacking_prop_type_e ctc_stacking_prop_type_t;

/**
 @brief  define stacking cloud header global configure
*/
struct ctc_stacking_hdr_glb_s
{
    uint8  mac_da_chk_en;      /**< [GB.GG.D2.TM]if set, cloud header packet mac da field with be check against port mac or system mac*/
    uint8  ether_type_chk_en;  /**< [GB.GG.D2.TM]if set, cloud header packet ether_type field with be check against ether_type */
    uint8  cos;                /**< [GB.GG.D2.TM]cloud header vlan cos if exsit vlan*/
    uint8  ip_protocol;        /**< [GB.GG.D2.TM]cloud header ip protocol if exsit ip header, default is 255*/

    uint16 vlan_tpid;          /**< [GB.GG.D2.TM]cloud header vlan tpid*/
    uint16 ether_type;         /**< [GB.GG.D2.TM]cloud header ether type*/

    uint8  ip_ttl;             /**< [GB.GG.D2.TM]cloud header ip ttl if exsit ip header, default is 255*/
    uint8  ip_dscp;            /**< [GB.GG.D2.TM]cloud header ip dscp if exsit ip header, default is 63*/
    uint8  udp_en;             /**< [GB.GG.D2.TM]cloud header encap with udp header*/
    uint8  is_ipv4;            /**< [GB.GG.D2.TM]cloud header encap with ipv4 or ipv6*/

    union
    {
        ip_addr_t ipv4;             /**< [GB.GG.D2.TM]cloud header ipv4 sourec address*/
        ipv6_addr_t ipv6;           /**< [GB.GG.D2.TM]cloud header ipv6 sourec address*/
    } ipsa;

    uint16 udp_src_port;            /**< [GB.GG.D2.TM]cloud header udp source port if udp_en*/
    uint16 udp_dest_port;           /**< [GB.GG.D2.TM]cloud header udp destination port if udp_en*/
};
typedef struct ctc_stacking_hdr_glb_s ctc_stacking_hdr_glb_t;


/**
@brief  define stacking fabric global configure
*/
struct ctc_stacking_inter_connect_glb_s
{
    #define CTC_INTER_CONNECT_MEMBER_NUM  4                                   /**< [GB]inter connect member num*/
    uint8  mode;                                                              /**< [GB] Two-Chips inter-connect. 0, stacking mode; 1 interlaken mode*/
    uint16  member_port[CTC_MAX_LOCAL_CHIP_NUM][CTC_INTER_CONNECT_MEMBER_NUM];/**< [GB] Member Port is valid for stacking mode, Port is lport*/
    uint8  member_num;                                                        /**< [GB] Member port num for stacking mode, max is CTC_STACKING_MEMBER_NUM*/
};
typedef struct ctc_stacking_inter_connect_glb_s ctc_stacking_inter_connect_glb_t;


/**
 @brief  define stacking version
*/
enum ctc_stacking_version_e
{
    CTC_STACKING_VERSION_1_0,     /**< [D2.TM] The Version, stackingHeader's length is 32. GB GG and D2 all support the version 1.0*/
    CTC_STACKING_VERSION_2_0,     /**< [D2.TM] The Version, stackingHeader's length is flexible. D2 Support the version 2.0*/
    CTC_STACKING_VERSION_MAX
};
typedef enum ctc_stacking_version_e ctc_stacking_version_t;



enum ctc_stacking_trunk_mode_e
{
    CTC_STACKING_TRUNK_MODE_0 = 0,     /**< [GB.GG.D2.TM] Only support Static mode, trunk max member count is 8*/
    CTC_STACKING_TRUNK_MODE_1,         /**< [GB.GG.D2.TM] Support Static/DLB mode, Static max member count is 32, DLB max member count is 16*/
    CTC_STACKING_TRUNK_MODE_2,         /**< [GB.GG.D2.TM] Support Static/DLB mode, Static max member count is <1-32>, DLB max member count is 16*/
    CTC_STACKING_TRUNK_MODE_MAX
};
typedef enum ctc_stacking_trunk_mode_e ctc_stacking_trunk_mode_t;

/**
 @brief  define stacking global configure
*/
struct ctc_stacking_glb_cfg_s
{
    ctc_stacking_hdr_glb_t hdr_glb;                 /**< [GB.GG.D2.TM]cloud header global configure*/
    ctc_stacking_inter_connect_glb_t connect_glb;   /**< [GB]Two-Chips inter-connect global configure*/
    uint8  fabric_mode;                             /**< [GG.D2.TM]the fabric mode of distributed system, 0:normal mode; 1:spine-leaf mode*/
    uint8  trunk_mode;                              /**< [GG.D2.TM]The stacking global trunk mode, refer to ctc_stacking_trunk_mode_t*/
    uint8  version;                                 /**< [D2.TM] stacking version, refer to ctc_stacking_version_t*/
    uint8  mcast_mode;                              /**< [GB.GG.D2.TM] Mcast mode, 0:add trunk to mcast group automatically; 1:add trunk to mcast group by user*/
};
typedef struct ctc_stacking_glb_cfg_s ctc_stacking_glb_cfg_t;

/**
 @brief  define stacking cloud header data structure
*/
struct ctc_stacking_hdr_encap_s
{
    uint32 hdr_flag;   /**< [GB.GG.D2.TM]encap cloud header type, refer to ctc_stacking_hdr_flag_t*/

    mac_addr_t mac_da; /**< [GB.GG.D2.TM]cloud header mac da if exist l2 header*/
    mac_addr_t mac_sa; /**< [GB.GG.D2.TM]cloud header mac sa if exist l2 header*/

    uint8  vlan_valid; /**< [GB.GG.D2.TM]if set, cloud header will encap vlan when using l2 header*/
    uint8  rsv0;
    uint16 vlan_id;   /**< [GB.GG.D2.TM]cloud header vlan id if exist vlan */

    union
    {
        ip_addr_t ipv4;             /**< [GB.GG.D2.TM]cloud header ipv4 destination address*/
        ipv6_addr_t ipv6;           /**< [GB.GG.D2.TM]cloud header ipv6 destination address*/
    } ipda;

};
typedef struct ctc_stacking_hdr_encap_s ctc_stacking_hdr_encap_t;

/**
 @brief  define stacking extend mode data structure
*/
struct ctc_stacking_extend_s
{
    uint8  gchip;          /**< [GB.GG.D2.TM]local gchip id, only useful when enable extend mode*/
    uint8  enable;         /**< [GB.GG.D2.TM]use gchip for extend mode*/
};
typedef struct ctc_stacking_extend_s ctc_stacking_extend_t;

enum ctc_stacking_trunk_flag_e
{
    CTC_STACKING_TRUNK_MEM_ASCEND_ORDER = 0x00000001,    /** [GB.GG.D2.TM] if set, the member port will be in order,only support in static mode*/
    CTC_STACKING_TRUNK_LB_HASH_OFFSET_VALID = 0x00000002 /*< [TM] load hash offset is high priority */
};
typedef enum ctc_stacking_trunk_flag_e ctc_stacking_trunk_flag_t;

/**
 @brief  define stacking trunk (uplink) data structure
*/
struct ctc_stacking_trunk_s
{

    uint8               trunk_id;             /**< [GB.GG.D2.TM]uplink trunk id <1-63>*/

    uint8               max_mem_cnt;          /**< [GB.GG.D2.TM] trunk group max member count   Support only if trunk_mode == CTC_STACKING_TRUNK_MODE_2 */
    uint8               select_mode;          /**< [D2.TM] unicast path select mode, If set 1,  use src port and remote chip select path, else use remote chip select path*/
    uint8               remote_gchip;         /**< [GB.GG.D2.TM]remote gchip id from uplink trunk id, used for ucast path select*/
    uint32              gport;                /**< [GB.GG.D2.TM]uplink trunk member port used for load balance*/
	uint32              src_gport;            /**< [D2.TM]      packet in port, used for ucast path select when select_mode == 1*/
    uint32              flag;                 /**< [GB.GG.D2.TM] trunk flag, refer to ctc_stacking_trunk_flag_t; */

    uint8               lb_hash_offset;          /**< [TM] select load balance hash offset*/
    uint8               rsv[3];

	ctc_stacking_load_mode_t     load_mode;   /**< [GB.GG.D2.TM]uplink load banlance mode*/
    ctc_stacking_hdr_encap_t     encap_hdr;   /**< [GB.GG.D2.TM]uplink cloud header*/
    ctc_stacking_extend_t        extend;      /**< [GB.GG.D2.TM]useful when internal is stacking connect*/
};
typedef struct ctc_stacking_trunk_s ctc_stacking_trunk_t;

/**
 @brief  define stacking keeplive member type
*/
enum ctc_stacking_keeplive_member_type_e
{
    CTC_STK_KEEPLIVE_MEMBER_TRUNK,           /**< [GB.GG.D2.TM]keepLive member is trunk(path select)*/
    CTC_STK_KEEPLIVE_MEMBER_PORT,            /**< [GB.GG.D2.TM]keepLive member is gport(master cpu port)*/
    CTC_STK_KEEPLIVE_MEMBER_MAX
};
typedef enum ctc_stacking_keeplive_member_type_e ctc_stacking_keeplive_member_type_t;

/**
 @brief  define stacking keeplive group data structure
*/
struct ctc_stacking_keeplive_s
{
    uint16 group_id;      /**< [GB.GG.D2.TM] keeplive group id*/
    uint8  mem_type;      /**< [GB.GG.D2.TM] keeplive member type, refer to ctc_stacking_keeplive_member_type_t*/
    uint8  trunk_id;      /**< [GB.GG.D2.TM] uplink trunk id <1-63>, select one path which keeplive packet go through*/
    uint32 cpu_gport;     /**< [GB.GG.D2.TM] assign the master cpu port which need receive the keeplive packet*/

    ctc_stacking_extend_t extend; /**< [GB.GG.D2.TM] useful when internal is stacking connect*/
    uint32 trunk_bitmap[CTC_STK_TRUNK_BMP_NUM];  /**< [GB.GG.D2.TM] trunk id bitmap*/
    uint8  trunk_bmp_en;                         /**< [GB.GG.D2.TM] if set, means using trunk_bitmap[CTC_STK_TRUNK_BMP_NUM]*/
    uint8  rsv[3];
};
typedef struct ctc_stacking_keeplive_s ctc_stacking_keeplive_t;



/**
 @brief  define stacking keeplive member type
*/
enum ctc_stacking_mcast_profile_type_e
{
    CTC_STK_MCAST_PROFILE_CREATE,           /**< [GB.GG.D2.TM]create mcast profile id*/
    CTC_STK_MCAST_PROFILE_DESTROY,          /**< [GB.GG.D2.TM]destroy mcast profile id*/
    CTC_STK_MCAST_PROFILE_ADD,              /**< [GB.GG.D2.TM]add mcast profile member*/
    CTC_STK_MCAST_PROFILE_REMOVE,           /**< [GB.GG.D2.TM]remove mcast profile member*/
    CTC_STK_MCAST_PROFILE_MAX
};
typedef enum ctc_stacking_mcast_profile_type_e ctc_stacking_mcast_profile_type_t;

/**
 @brief  define stacking keeplive group data structure
*/
struct ctc_stacking_trunk_mcast_profile_s
{
    uint8  type;                  /**< [GB.GG.D2.TM] operation type, refer to ctc_stacking_mcast_profile_type_t*/
    uint8  append_en;             /**< [GB.GG.D2.TM] if append_en == 1, mcast_profile_id will append to default profile
                                                  else  replace default profile*/
    uint16 mcast_profile_id;      /**< [GB.GG.D2.TM] Trunk mcast profile id, if mcast_profile_id == 0, SDK will alloc*/
    uint32 trunk_bitmap[CTC_STK_TRUNK_BMP_NUM];       /**< [GB.GG.D2.TM] uplink trunk id bitmap*/
    ctc_stacking_extend_t extend; /**< [GB.GG.D2.TM] used in multi-chip*/

};
typedef struct ctc_stacking_trunk_mcast_profile_s ctc_stacking_trunk_mcast_profile_t;


/**
 @brief  define stacking keeplive member type
*/
enum ctc_stacking_stop_mode_e
{
    CTC_STK_STOP_MODE_DISCARD_RCHIP_BITMAP = 0,   /**< [GB.GG.D2.TM] discard packets from remote chip bitmap, only support 32 chip, compatible mode*/
    CTC_STK_STOP_MODE_DISCARD_RCHIP,              /**< [D2.TM] discard packets from remote chip*/
    CTC_STK_STOP_MODE_DISCARD_RCHIP_PORT,         /**< [D2.TM] discard packets from a given packet in port and remote chip*/
    CTC_STK_STOP_MODE_BLOCK_RCHIP_PORT,           /**< [D2.TM] block or allow packets from a given packet in port and remote chip to ports*/
    CTC_STK_STOP_MODE_MAX
};
typedef enum ctc_stacking_stop_mode_e ctc_stacking_stop_mode_t;


/**
 @brief  define stacking mcast stop remote chip configure
*/
struct ctc_stacking_stop_rchip_s
{
    uint8 mode;                                /**<[GB.GG.D2.TM] ctc_stacking_stop_mode_t */
    uint32 rchip_bitmap;                       /**<[GB.GG.D2.TM] remote chip bitmap, use in mode CTC_STK_STOP_MODE_DISCARD_RCHIP_BITMAP*/

    uint32 rchip;                              /**<[D2.TM] remote gchip id,  used in mode CTC_STK_STOP_MODE_DISCARD_RCHIP*/
	uint32 src_gport;                          /**<[D2.TM] packet in port,   used in mode CTC_STK_STOP_MODE_DISCARD_RCHIP_PORT and CTC_STK_STOP_MODE_BLOCK_RCHIP_PORT*/

    ctc_port_bitmap_t pbm;                     /**<[D2.TM] bitmap port to block or allow packets from a given packet in port and remote chip, used in mode CTC_STK_STOP_MODE_BLOCK_RCHIP_PORT*/

	uint8 discard;                             /**<[D2.TM] If set , discard packets from a given packet in port and remote chip, used in mode CTC_STK_STOP_MODE_DISCARD_RCHIP and CTC_STK_STOP_MODE_DISCARD_RCHIP_PORT*/

};
typedef struct ctc_stacking_stop_rchip_s ctc_stacking_stop_rchip_t;



/**
 @brief  define stacking mcast break point configure
*/
struct ctc_stacking_mcast_break_point_s
{
    uint8  trunk_id;                        /**<[GB.GG.D2.TM] Trunk id to set point*/
    uint8  enable;                          /**<[GB.GG.D2.TM] If set, trunk port will disable mcast transmit, else enable*/
};
typedef struct ctc_stacking_mcast_break_point_s ctc_stacking_mcast_break_point_t;

/**
 @brief  define stacking remote chip ability
*/
struct ctc_stacking_rchip_port_s
{
    uint8  rchip;                        /**<[GB.GG.D2.TM] Remote  gchip id*/
    ctc_port_bitmap_t pbm;               /**<[GB.GG.D2.TM] Remote chip support port bitmap*/
};
typedef struct ctc_stacking_rchip_port_s ctc_stacking_rchip_port_t;

/**
 @brief  define stacking bind trunk
*/
struct ctc_stacking_bind_trunk_s
{
    uint32  gport;                        /**<[GB.GG.D2.TM] Trunk port*/
    uint8   trunk_id;                     /**<[GB.GG.D2.TM] Trunk id, trunk_id = 0 means unbinding*/
    uint8   rsv[3];
};
typedef struct ctc_stacking_bind_trunk_s ctc_stacking_bind_trunk_t;


/**
 @brief  define stacking property data structure
*/
struct ctc_stacking_property_s
{
    uint32 prop_type;           /**< [GB.GG.D2.TM] configure property type , refer to ctc_stacking_prop_type_t*/
    uint32 value;               /**< [GB.GG.D2.TM] configure property value*/
    void*  p_value;             /**< [GB.GG.D2.TM] configure property complex value*/

    ctc_stacking_glb_cfg_t glb_cfg;  /**< [GB.GG.D2.TM] confiure global configure value*/

	ctc_stacking_extend_t extend;    /**<[GB.GG.D2.TM] useful when internal is stacking connect*/

};
typedef struct ctc_stacking_property_s ctc_stacking_property_t;

/**
 @brief  define cloud header encap data structure
*/
struct ctc_stacking_cloud_hdr_s
{
  ctc_stacking_hdr_encap_t  hdr;    /**< [GB] Cloud header encap, only support CTC_STK_HDR_WITH_L2 */
  uint16    dest_group_id;          /**< [GB] Cloud header encap, dest mcast group id */
  uint32    dest_gport;             /**< [GB] Cloud header encap, dest gport */
  uint32    src_gchip:8;            /**< [GB] Cloud header encap, source gchip id */
  uint32    priority :8;            /**< [GB] Cloud header encap, packet priority */
  uint32    is_mcast :1;            /**< [GB] Indicate dest is mcast group */
  uint32    rsv1     :15;

  ctc_pkt_skb_t          skb;       /**< [GB] Packet TX buffer for cloud header encap */
};
typedef struct ctc_stacking_cloud_hdr_s ctc_stacking_cloud_hdr_t;

/**@} end of @defgroup  stacking STACKING */

#ifdef __cplusplus
}
#endif

#endif

