#ifndef __LLDP_API_H__
#define __LLDP_API_H__

#define LLDP_POWER_CLASS_UN             0    
#define LLDP_POWER_CLASS_0              1    
#define LLDP_POWER_CLASS_1              2    
#define LLDP_POWER_CLASS_2              3  
#define LLDP_POWER_CLASS_3              4 
#define LLDP_POWER_CLASS_4              5 
#define LLDP_POWER_CLASS_MAX            6 

#define LLDP_AUTONEGO_CAP_BOTHER                           0
#define LLDP_AUTONEGO_CAP_B10BASET                         1
#define LLDP_AUTONEGO_CAP_B10BASETFD                       2
#define LLDP_AUTONEGO_CAP_B100BASET4                       3
#define LLDP_AUTONEGO_CAP_B100BASETX                       4
#define LLDP_AUTONEGO_CAP_B100BASETXFD                     5
#define LLDP_AUTONEGO_CAP_B100BASET2                       6
#define LLDP_AUTONEGO_CAP_B100BASET2FD                     7
#define LLDP_AUTONEGO_CAP_BFDXPAUSE                        8
#define LLDP_AUTONEGO_CAP_BFDXAPAUSE                       9
#define LLDP_AUTONEGO_CAP_BFDXSPAUSE                       10
#define LLDP_AUTONEGO_CAP_BFDXBPAUSE                       11
#define LLDP_AUTONEGO_CAP_B1000BASEX                       12
#define LLDP_AUTONEGO_CAP_B1000BASEXFD                     13
#define LLDP_AUTONEGO_CAP_B1000BASET                       14
#define LLDP_AUTONEGO_CAP_B1000BASETFD                     15
#define LLDP_AUTONEGO_CAP_MAX                              16

#define LLDP_DOT3_MAU_TYPE_AUI             1
#define LLDP_DOT3_MAU_TYPE_10Base5         2
#define LLDP_DOT3_MAU_TYPE_Foirl           3 
#define LLDP_DOT3_MAU_TYPE_10Base2         4
#define LLDP_DOT3_MAU_TYPE_10BaseT         5
#define LLDP_DOT3_MAU_TYPE_10BaseFP        6
#define LLDP_DOT3_MAU_TYPE_10BaseFB        7
#define LLDP_DOT3_MAU_TYPE_10BaseFL        8
#define LLDP_DOT3_MAU_TYPE_10Broad36       9
#define LLDP_DOT3_MAU_TYPE_10BaseTHD      10
#define LLDP_DOT3_MAU_TYPE_10BaseTFD      11
#define LLDP_DOT3_MAU_TYPE_10BaseFLHD     12
#define LLDP_DOT3_MAU_TYPE_10BaseFLFD     13
#define LLDP_DOT3_MAU_TYPE_100BaseT4      14
#define LLDP_DOT3_MAU_TYPE_100BaseTXHD    15
#define LLDP_DOT3_MAU_TYPE_100BaseTXFD    16
#define LLDP_DOT3_MAU_TYPE_100BaseFXHD    17
#define LLDP_DOT3_MAU_TYPE_100BaseFXFD    18
#define LLDP_DOT3_MAU_TYPE_100BaseT2HD    19
#define LLDP_DOT3_MAU_TYPE_100BaseT2FD    20
#define LLDP_DOT3_MAU_TYPE_1000BaseXHD    21 
#define LLDP_DOT3_MAU_TYPE_1000BaseXFD    22
#define LLDP_DOT3_MAU_TYPE_1000BaseLXHD   23
#define LLDP_DOT3_MAU_TYPE_1000BaseLXFD   24
#define LLDP_DOT3_MAU_TYPE_1000BaseSXHD   25
#define LLDP_DOT3_MAU_TYPE_1000BaseSXFD   26
#define LLDP_DOT3_MAU_TYPE_1000BaseCXHD   27
#define LLDP_DOT3_MAU_TYPE_1000BaseCXFD   28
#define LLDP_DOT3_MAU_TYPE_1000BaseTHD    29
#define LLDP_DOT3_MAU_TYPE_1000BaseTFD    30
#define LLDP_DOT3_MAU_TYPE_10GigBaseX     31
#define LLDP_DOT3_MAU_TYPE_10GigBaseLX4   32
#define LLDP_DOT3_MAU_TYPE_10GigBaseR     33
#define LLDP_DOT3_MAU_TYPE_10GigBaseER    34
#define LLDP_DOT3_MAU_TYPE_10GigBaseLR    35
#define LLDP_DOT3_MAU_TYPE_10GigBaseSR    36
#define LLDP_DOT3_MAU_TYPE_10GigBaseW     37
#define LLDP_DOT3_MAU_TYPE_10GigBaseEW    38
#define LLDP_DOT3_MAU_TYPE_10GigBaseLW    39
#define LLDP_DOT3_MAU_TYPE_10GigBaseSW    40
#define LLDP_DOT3_MAU_TYPE_10GigBaseCX4   41
#define LLDP_DOT3_MAU_TYPE_2BaseTL        42
#define LLDP_DOT3_MAU_TYPE_10PassTS       43
#define LLDP_DOT3_MAU_TYPE_100BaseBX10D   44  
#define LLDP_DOT3_MAU_TYPE_100BaseBX10U   45
#define LLDP_DOT3_MAU_TYPE_100BaseLX10    46
#define LLDP_DOT3_MAU_TYPE_1000BaseBX10D  47 
#define LLDP_DOT3_MAU_TYPE_1000BaseBX10U  48
#define LLDP_DOT3_MAU_TYPE_1000BaseLX10   49
#define LLDP_DOT3_MAU_TYPE_1000BasePX10D  50
#define LLDP_DOT3_MAU_TYPE_1000BasePX10U  51
#define LLDP_DOT3_MAU_TYPE_1000BasePX20D  52
#define LLDP_DOT3_MAU_TYPE_1000BasePX20U  53
#define LLDP_DOT3_MAU_TYPE_MAX 54

char * lldp_auto_neg_cap_to_name(uint32 i);

char * lldp_auto_neg_cap_to_descr(uint32 i);

char * lldp_mau_type_to_name(uint32 i);

char * lldp_mau_type_to_descr(uint32 i);

lldp_protocol_identity_t*
lldp_get_protocol_identity (uint32 proto);


uint8
lldp_get_protocol_id (uint8 *protocol_payload, uint8 protocol_id_len);

int32 
lldp_autoneg_support_get (tbl_interface_t* p_db_if);

int32 
lldp_autoneg_capability_get (tbl_interface_t* p_db_if);

int32
lldp_oper_mau_type_get (tbl_interface_t* p_db_if);

char*
lldp_chassis_id_val2str(char *str, uint32 str_len, lldp_chassis_id_t *p_chassis_id);
char*
lldp_port_id_val2str(char *str, uint32 str_len, lldp_port_id_t *p_port_id);

#endif /* __LLDP_API_H__ */

