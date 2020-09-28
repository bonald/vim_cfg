/**
 @file ctc_error.c

 @date 2012-08-15

 @version v2.0

 return sdk error description
 auto generated based on head file
*/
#include "sal.h"
#include "ctc_error.h"

uint32 g_mapping_disable = 0;
const char *ctc_get_error_desc(int32 error_code)
{
    static char  error_desc[256+1] = {0};
    error_code = ctc_error_code_mapping(error_code);
    if (error_code == CTC_E_NONE)
    {
        return " ";
    }

    switch(error_code)
    {
        case CTC_E_NONE:  
          return "NO error ";   
        case CTC_E_INVALID_PTR:  
          return "Invalid pointer ";   
        case CTC_E_INVALID_PORT:  
          return "Invalid port ";   
        case CTC_E_INVALID_PARAM:  
          return "Invalid parameter ";   
        case CTC_E_BADID:  
          return "Invalid identifier ";   
        case CTC_E_INVALID_CHIP_ID:  
          return "Invalid chip ";   
        case CTC_E_INVALID_CONFIG:  
          return "Invalid configuration ";   
        case CTC_E_EXIST:  
          return "Entry exists ";   
        case CTC_E_NOT_EXIST:  
          return "Entry not found ";   
        case CTC_E_NOT_READY:  
          return "Not ready to configuration ";   
        case CTC_E_IN_USE:  
          return "Already in used ";   
        case CTC_E_NOT_SUPPORT:  
          return "API or some feature is not supported ";   
        case CTC_E_NO_RESOURCE:  
          return "No resource in ASIC ";   
        case CTC_E_PROFILE_NO_RESOURCE:  
          return "Profile no resource in ASIC ";   
        case CTC_E_NO_MEMORY:  
          return "No memory ";   
        case CTC_E_HASH_CONFLICT:  
          return "Hash Conflict ";   
        case CTC_E_NOT_INIT:  
          return "Feature not initialized ";   
        case CTC_E_INIT_FAIL:  
          return "Feature initialize fail ";   
        case CTC_E_DMA:  
          return "DMA error ";   
        case CTC_E_HW_TIME_OUT:  
          return "HW operation timed out ";   
        case CTC_E_HW_BUSY:  
          return "HW busy ";   
        case CTC_E_HW_INVALID_INDEX:  
          return "Exceed max table index ";   
        case CTC_E_HW_NOT_LOCK:  
          return "HW not lock ";   
        case CTC_E_HW_FAIL:  
          return "HW operation fail ";   
        case CTC_E_VERSION_MISMATCH:  
          return "Version mismatch";   
        case CTC_E_PARAM_CONFLICT:  
          return "Parameter Conflict";   
        case CTC_E_TOO_MANY_FRAGMENT:  
          return "Too many fragment, need Defragmentation";   
        case CTC_E_WB_BUSY:  
          return "Warmboot busy ";   
        case CTC_E_ENTRY_EXIST:  
          return "Entry already exist ";   
        case CTC_E_ENTRY_NOT_EXIST:  
          return "Entry not exist ";   
        case CTC_E_EXCEED_MAX_SIZE:  
          return "Exceed max value ";   
        case CTC_E_EXCEED_MIN_SIZE:  
          return "Under min value ";   
        case CTC_E_INVALID_DIR:  
          return "Invalid direction ";   
        case CTC_E_CANT_CREATE_AVL:  
          return "Cannot create avl tree ";   
        case CTC_E_UNEXPECT:  
          return "Unexpect value/result ";   
        case CTC_E_LESS_ZERO:  
          return "Less zero ";   
        case CTC_E_MEMBER_EXIST:  
          return "Member already exist ";   
        case CTC_E_MEMBER_NOT_EXIST:  
          return "Member not exist ";   
        case CTC_E_INVALID_EXP_VALUE:  
          return "Invalid exp value ";   
        case CTC_E_INVALID_MPLS_LABEL_VALUE:  
          return "Invalid mpls label value ";   
        case CTC_E_INVALID_REG:  
          return "Invalid register id ";   
        case CTC_E_INVALID_TBL:  
          return "Invalid table id ";   
        case CTC_E_GLOBAL_CONFIG_CONFLICT:  
          return "Global config conflict";   
        case CTC_E_FEATURE_NOT_SUPPORT:  
          return "Feature not supported ";   
        case CTC_E_INVALID_TTL:  
          return "Invalid TTL ";   
        case CTC_E_INVALID_MTU_SIZE:  
          return "Invalid mtu size ";   
        case CTC_E_SPOOL_ADD_UPDATE_FAILED:  
          return "Share pool add or update fail ";   
        case CTC_E_SPOOL_REMOVE_FAILED:  
          return "Share pool remove fail";   
        case CTC_E_RPF_SPOOL_ADD_VECTOR_FAILED:  
          return "Share pool add vector failed ";   
        case CTC_E_RPF_SPOOL_REMOVE_VECTOR_FAILED:  
          return "Share pool remove vector failed ";   
        case CTC_E_NO_ROOM_TO_MOVE_ENTRY:  
          return "No room to move entry ";   
        case CTC_E_STP_INVALID_STP_ID:  
          return "Invalid stp id ";   
        case CTC_E_EXCEED_STP_MAX_LPORT:  
          return "Exceed max stp lport ";   
        case CTC_E_INVALID_COS:  
          return "Invalid cos ";   
        case CTC_E_INVALID_FRAMESIZE_INDEX:  
          return "Invalid frame size index ";   
        case CTC_E_INVALID_MIN_FRAMESIZE:  
          return "Invalid min frame size ";   
        case CTC_E_INVALID_MAX_FRAMESIZE:  
          return "Invalid max frame size ";   
        case CTC_E_MAC_NOT_USED:  
          return "MAC is not used ";   
        case CTC_E_TRAININT_FAIL:  
          return "Training fail ";   
        case CTC_E_FAIL_CREATE_MUTEX:  
          return "Create mutex fail";   
        case CTC_E_MUTEX_BUSY:  
          return "Mutex busy ";   
        case CTC_E_INVALID_CHIP_NUM:  
          return "Invalid chip num ";   
        case CTC_E_INVALID_GLOBAL_CHIPID:  
          return "Invalid global chip id ";   
        case CTC_E_INVALID_LOCAL_CHIPID:  
          return "Invalid Local chip id ";   
        case CTC_E_CHIP_IS_LOCAL:  
          return "Chip is local! ";   
        case CTC_E_CHIP_IS_REMOTE:  
          return "Chip is not local! ";   
        case CTC_E_INVALID_GLOBAL_PORT:  
          return "Invalid global port ";   
        case CTC_E_INVALID_LOGIC_PORT:  
          return "Invalid logic port";   
        case CTC_E_MEMBER_PORT_EXIST:  
          return "Port is exist ";   
        case CTC_E_MEMBER_PORT_NOT_EXIST:  
          return "Port is not exist ";   
        case CTC_E_LOCAL_PORT_NOT_EXIST:  
          return "Local phyport is not exist ";   
        case CTC_E_ALREADY_PHY_IF_EXIST:  
          return "Physical interface  already exists the port ";   
        case CTC_E_ALREADY_SUB_IF_EXIST:  
          return "Sub interface  already exists the port ";   
        case CTC_E_PORT_HAS_OTHER_FEATURE:  
          return "Has other feature on this port ";   
        case CTC_E_PORT_FEATURE_MISMATCH:  
          return "Feature on port mismatch ";   
        case CTC_E_PORT_PVLAN_TYPE_NOT_SUPPORT:  
          return "port for this pvlan type can not configure isolated id ";   
        case CTC_E_PORT_HAS_OTHER_RESTRICTION:  
          return "port has another restriction feature ";   
        case CTC_E_INVALID_LOCAL_PORT:  
          return "Invalid localphy port ";   
        case CTC_E_INVALID_PORT_MAC_TYPE:  
          return "Invalid port mac type ";   
        case CTC_E_INVALID_PORT_SPEED_MODE:  
          return "Invalid port speed mode ";   
        case CTC_E_INVALID_PREAMBLE:  
          return "Invalid preamble ";   
        case CTC_E_INVALID_TX_THRESHOLD:  
          return "Invalid transmit threshold ";   
        case CTC_E_CANT_CHANGE_SERDES_MODE:  
          return "Serdes mode can't be changed ";   
        case CTC_E_SERDES_STATUS_NOT_READY:  
          return "Serdes Status is not ready  ";   
        case CTC_E_SERDES_MODE_NOT_SUPPORT:  
          return "Serdes mode is not support  ";   
        case CTC_E_SERDES_PLL_NOT_SUPPORT:  
          return "Serdes pll core divide is not support  ";   
        case CTC_E_SERDES_EYE_TEST_NOT_DONE:  
          return "Eye test not done  ";   
        case CTC_E_CHIP_TIME_OUT:  
          return "Time out ";   
        case CTC_E_CHIP_SCAN_PHY_REG_FULL:  
          return "phy opt register full";   
        case CTC_E_CHIP_DATAPATH_NOT_MATCH:  
          return "datapath not match";   
        case CTC_E_PORT_INVALID_PORT_MAC:  
          return "Invalid Port MAC, high 32 bits must same";   
        case CTC_E_PORT_INVALID_ACL_LABEL:  
          return "Invalid Port ACL label";   
        case CTC_E_PORT_PROP_COLLISION:  
          return "Port property collision ";   
        case CTC_E_PORT_ACL_EN_FIRST:  
          return "Enable ACL on port first ";   
        case CTC_E_CHIP_SWITCH_FAILED:  
          return "dynamic switch failed ";   
        case CTC_E_PORT_MAC_IS_DISABLE:  
          return "mac is not enable ";   
        case CTC_E_PORT_MAC_PCS_ERR:  
          return "Pcs is error ";   
        case CTC_E_INVALID_DEVICE_ID:  
          return "Invalid device id ";   
        case CTC_E_VLAN_EXCEED_MAX_VLANCTL:  
          return "[Vlan] Exceed max vlantag control type ";   
        case CTC_E_VLAN_EXCEED_MAX_PHY_PORT:  
          return "[Vlan] Exceed max phy port ";   
        case CTC_E_VLAN_INVALID_VLAN_ID:  
          return "[Vlan] Invalid vlan id ";   
        case CTC_E_VLAN_INVALID_VLAN_PTR:  
          return "[Vlan] Invalid vlan ptr ";   
        case CTC_E_VLAN_ERROR_MODE:  
          return "[Vlan] API not support in this mode ";   
        case CTC_E_VLAN_INVALID_VLAN_ID_OR_VLAN_PTR:  
          return "[Vlan] Invalid vlan id or vlan ptr ";   
        case CTC_E_VLAN_INVALID_FID_ID:  
          return "[Vlan] Invalid fid id ";   
        case CTC_E_VLAN_INVALID_VRFID:  
          return "[Vlan] Invalid vrfid ";   
        case CTC_E_VLAN_INVALID_PROFILE_ID:  
          return "[Vlan] Invalid profile id ";   
        case CTC_E_VLAN_EXIST:  
          return "[Vlan] Vlan already create  ";   
        case CTC_E_VLAN_NOT_CREATE:  
          return "[Vlan] Vlan not create ";   
        case CTC_E_VLAN_FILTER_PORT:  
          return "[Vlan] Invalid vlan filter port";   
        case CTC_E_VLAN_CLASS_INVALID_TYPE:  
          return "[Vlan] Vlan class invalid type ";   
        case CTC_E_VLAN_CLASS_NOT_ENABLE:  
          return "[Vlan] Vlan class not enable ";   
        case CTC_E_VLAN_MAPPING_NOT_ENABLE:  
          return "[Vlan] Vlan mapping not enable ";   
        case CTC_E_VLAN_MAPPING_INVALID_SERVICE_ID:  
          return "[Vlan] Vlan mapping invalid service id ";   
        case CTC_E_VLAN_MAPPING_RANGE_TYPE_NOT_MATCH:  
          return "[Vlan] Vlan mapping range type not match ";   
        case CTC_E_VLAN_MAPPING_RANGE_GROUP_NOT_MATCH:  
          return "[Vlan] Vlan mapping range group not match ";   
        case CTC_E_VLAN_MAPPING_GET_VLAN_RANGE_FAILED:  
          return "[Vlan] Vlan mapping get range failed";   
        case CTC_E_VLAN_MAPPING_VRANGE_FULL:  
          return "[Vlan] Vlan mapping vlan range full";   
        case CTC_E_VLAN_MAPPING_VRANGE_OVERLAPPED:  
          return "[Vlan] Vlan mapping vlan range overlapped";   
        case CTC_E_VLAN_MAPPING_TAG_APPEND:  
          return "[Vlan] Vlan mapping tag_edit append wrong";   
        case CTC_E_VLAN_MAPPING_TAG_SWAP:  
          return "[Vlan] Vlan mapping tag_edit swap wrong";   
        case CTC_E_VLAN_MAPPING_TAG_OP_NOT_SUPPORT:  
          return "[Vlan] Vlan mapping tag operation not support ";   
        case CTC_E_VLAN_RANGE_NOT_EXIST:  
          return "[Vlan] Vlan range not exist";   
        case CTC_E_VLAN_RANGE_EXIST:  
          return "[Vlan] Vlan range already exist";   
        case CTC_E_VLAN_RANGE_END_GREATER_START:  
          return "[Vlan] End vlan id should greater than start vlan id ";   
        case CTC_E_VLAN_CLASS_PROTOCOL_VLAN_FULL:  
          return "[Vlan] Protocol vlan table is full ";   
        case CTC_E_VLAN_ACL_EN_FIRST:  
          return "[Vlan] Enable ACL on vlan first ";   
        case CTC_E_INVALID_TID:  
          return "[Linkagg] Tid is invalid ";   
        case CTC_E_LINKAGG_NOT_EXIST:  
          return "[Linkagg] Entry is not exist ";   
        case CTC_E_LINKAGG_HAS_EXIST:  
          return "[Linkagg] Entry already exist ";   
        case CTC_E_LINKAGG_DYNAMIC_BALANCE_NOT_SUPPORT:  
          return "[Linkagg] Dynamic balance don't support ";   
        case CTC_E_LINKAGG_INVALID_LINKAGG_MODE:  
          return "[Linkagg] Linkagg mode is invalid ";   
        case CTC_E_CREATE_MEM_CACHE_FAIL:  
          return "Create mem cache fail ";   
        case CTC_E_USRID_INVALID_KEY:  
          return "[Userid] Invalid usrid key ";   
        case CTC_E_USRID_INVALID_LABEL:  
          return "[Userid] Invalid usrid label ";   
        case CTC_E_USRID_INVALID_TYPE:  
          return "[Userid] Invalid usrid type ";   
        case CTC_E_USRID_DISABLE:  
          return "[Userid] Userid disable ";   
        case CTC_E_USRID_ALREADY_ENABLE:  
          return "[Userid] Userid already enable ";   
        case CTC_E_USRID_NO_KEY:  
          return "[Userid] Userid no key ";   
        case CTC_E_SCL_INVALID_KEY:  
          return "[SCL] Invalid scl key ";   
        case CTC_E_SCL_INVALID_LABEL:  
          return "[SCL] Invalid scl label ";   
        case CTC_E_SCL_INVALID_TYPE:  
          return "[SCL] Invalid scl type ";   
        case CTC_E_SCL_DISABLE:  
          return "[SCL] SCL disable ";   
        case CTC_E_SCL_ALREADY_ENABLE:  
          return "[SCL] SCL already enable ";   
        case CTC_E_SCL_NO_KEY:  
          return "[SCL] SCL no key ";   
        case CTC_E_SCL_INVALID_DEFAULT_PORT:  
          return "[SCL] Invalid default port";   
        case CTC_E_SCL_HASH_CONFLICT:  
          return "[SCL] Hash conflict";   
        case CTC_E_SCL_HASH_INSERT_FAILED:  
          return "[SCL] Hash insert fail";   
        case CTC_E_SCL_HASH_CONVERT_FAILED:  
          return "[SCL] Hash covert fail";   
        case CTC_E_SCL_BUILD_AD_INDEX_FAILED:  
          return "[SCL] Build ad index fail";   
        case CTC_E_SCL_NEED_DEBUG:  
          return "[SCL] Need debug";   
        case CTC_E_SCL_GET_KEY_FAILED:  
          return "[SCL] Get key fail";   
        case CTC_E_SCL_KEY_AD_EXIST:  
          return "[SCL] Key ad already exist";   
        case CTC_E_SCL_AD_NO_MEMORY:  
          return "[SCL] Ad no memory";   
        case CTC_E_SCL_BUILD_VLAN_ACTION_INDEX_FAILED:  
          return "[SCL] Build vlan action index fail";   
        case CTC_E_SCL_UNSUPPORT:  
          return "[SCL] No support";   
        case CTC_E_SCL_CANNOT_OVERWRITE:  
          return "[SCL] Cannot overwrite";   
        case CTC_E_SCL_LKP_FAILED:  
          return "[SCL] Lookup fail";   
        case CTC_E_SCL_VLAN_ACTION_INVALID:  
          return "[SCL] Invalid vlan action";   
        case CTC_E_SCL_STATIC_ENTRY:  
          return "[SCL] Static entry";   
        case CTC_E_SCL_UPDATE_FAILED:  
          return "[SCL] Update entry failed";   
        case CTC_E_SCL_INIT:  
          return "[SCL] Init failed ";   
        case CTC_E_SCL_GROUP_PRIORITY:  
          return "[SCL] Invalid group priority ";   
        case CTC_E_SCL_GROUP_TYPE:  
          return "[SCL] Invalid group type ";   
        case CTC_E_SCL_GROUP_ID:  
          return "[SCL] Invalid group id ";   
        case CTC_E_SCL_GROUP_ID_RSVED:  
          return "[SCL] Group id reserved ";   
        case CTC_E_SCL_GROUP_UNEXIST:  
          return "[SCL] Group not exist ";   
        case CTC_E_SCL_GROUP_EXIST:  
          return "[SCL] Group exist ";   
        case CTC_E_SCL_GROUP_NOT_EMPTY:  
          return "[SCL] Group not empty ";   
        case CTC_E_SCL_INVALID_ACTION:  
          return "[SCL] Invalid action ";   
        case CTC_E_SCL_INVALID_KEY_TYPE:  
          return "[SCL] Invalid key type ";   
        case CTC_E_SCL_INVALID_LABEL_TYPE:  
          return "[SCL] Invalid label type ";   
        case CTC_E_SCL_LABEL_NOT_EXIST:  
          return "[SCL] Label not exist ";   
        case CTC_E_SCL_HAVE_ENABLED:  
          return "[SCL] Have enabled ";   
        case CTC_E_SCL_NOT_ENABLED:  
          return "[SCL] Not enabled ";   
        case CTC_E_SCL_LABEL_IN_USE:  
          return "[SCL] Label in use ";   
        case CTC_E_SCL_FLAG_COLLISION:  
          return "[SCL] Flag collision ";   
        case CTC_E_SCL_L4_PORT_RANGE_EXHAUSTED:  
          return "[SCL] L4 port range exhausted ";   
        case CTC_E_SCL_TCP_FLAG_EXHAUSTED:  
          return "[SCL] Tcp flags exhausted ";   
        case CTC_E_SCL_GET_NODES_FAILED:  
          return "[SCL] Get nodes failed ";   
        case CTC_E_SCL_PORT_BITMAP_OVERFLOW:  
          return "[SCL] Port bitmap overflow ";   
        case CTC_E_SCL_GROUP_INFO:  
          return "[SCL] Group info error ";   
        case CTC_E_SCL_GET_BLOCK_INDEX_FAILED:  
          return "[SCL] Get block index failed ";   
        case CTC_E_SCL_ENTRY_INSTALLED:  
          return "[SCL] Entry installed ";   
        case CTC_E_SCL_GET_STATS_FAILED:  
          return "[SCL] Get stats failed ";   
        case CTC_E_SCL_GROUP_NOT_INSTALLED:  
          return "[SCL] Group not installed ";   
        case CTC_E_SCL_SERVICE_ID:  
          return "[SCL] Invalid service id ";   
        case CTC_E_SCL_GLOBAL_CFG_ERROR:  
          return "[SCL] Global config error ";   
        case CTC_E_SCL_STATS_NOT_SUPPORT:  
          return "[SCL] Stats not support on this priority ";   
        case CTC_E_SCL_ADD_TCAM_ENTRY_TO_WRONG_GROUP:  
          return "[SCL] Add tcam entry to wrong group ";   
        case CTC_E_SCL_ADD_HASH_ENTRY_TO_WRONG_GROUP:  
          return "[SCL] Add hash entry to wrong group ";   
        case CTC_E_SCL_HASH_ENTRY_NO_PRIORITY:  
          return "[SCL] Hash entry has no priority ";   
        case CTC_E_SCL_HASH_ENTRY_UNSUPPORT_COPY:  
          return "[SCL] Hash acl entry not support copy ";   
        case CTC_E_SCL_VLAN_EDIT_INVALID:  
          return "[SCL] SCL vlan edit action invalid ";   
        case CTC_E_SCL_KEY_ACTION_TYPE_NOT_MATCH:  
          return "[SCL] SCL Key type Action type not match ";   
        case CTC_E_FDB_WRONG_MAC_ADDR:  
          return "[FDB] Wrong mac address ";   
        case CTC_E_FDB_MAC_FILTERING_ENTRY_EXIST:  
          return "[FDB] Mac filtering entry is already exist ";   
        case CTC_E_FDB_ENTRY_FULL:  
          return "[FDB] Entries have been full ";   
        case CTC_E_FDB_L2MCAST_MEMBER_INVALID:  
          return "[FDB] L2mcast member is invalid ";   
        case CTC_E_FDB_L2MCAST_ADD_MEMBER_FAILED:  
          return "[FDB] L2mcast add member failed ";   
        case CTC_E_FDB_MCAST_ENTRY_EXIST:  
          return "[FDB] Multicast entry with same mac is already exists ";   
        case CTC_E_FDB_DEFAULT_ENTRY_NOT_EXIST:  
          return "[FDB] Default entry does not exist ";   
        case CTC_E_FDB_OPERATION_PAUSE:  
          return "[FDB] Need to next flush ";   
        case CTC_E_FDB_HASH_INSERT_FAILED:  
          return "[FDB] Hash insert failed ";   
        case CTC_E_FDB_HASH_REMOVE_FAILED:  
          return "[FDB] Hash remove failed ";   
        case CTC_E_FDB_AD_INDEX_NOT_EXIST:  
          return "[FDB] Ad index not exist ";   
        case CTC_E_FDB_DEFAULT_ENTRY_NOT_ALLOWED:  
          return "[FDB] Default entry not allowed ";   
        case CTC_E_FDB_INVALID_FDB_TYPE:  
          return "[FDB] Invalid fdb type ";   
        case CTC_E_FDB_NOT_LOCAL_MEMBER:  
          return "[FDB] Member port is not local member ";   
        case CTC_E_FDB_KEY_ALREADY_EXIST:  
          return "[FDB] Key already exist ";   
        case CTC_E_FDB_HASH_CONFLICT:  
          return "[FDB] Hash confict ";   
        case CTC_E_FDB_SOURCE_PORT_TYPE_NOT_MATCH:  
          return "[FDB] Source port type not match ";   
        case CTC_E_FDB_NO_RESOURCE:  
          return "[FDB] No resource ";   
        case CTC_E_FDB_INVALID_FID:  
          return "[FDB] Invalid FID ";   
        case CTC_E_FDB_NO_SW_TABLE_NO_QUARY:  
          return "[FDB] Not support this function when no software table ";   
        case CTC_E_FDB_ONLY_SW_LEARN:  
          return "[FDB] FDB in software aging mode, you can change it in chip_profile.cfg ";   
        case CTC_E_FDB_LOGIC_USE_HW_LEARN_DISABLE:  
          return "[FDB] FDB logic-port use hw-learning disable, you can change it in chip_profile.cfg ";   
        case CTC_E_FDB_NO_SW_TABLE_NO_FLUSH_BY_SW:  
          return "[FDB] Not support software flush when no software table ";   
        case CTC_E_FDB_ONLY_HW_LEARN:  
          return "[FDB] Not support Hw learning to Software learning Switch";   
        case CTC_E_FDB_FIB_DUMP_FAIL:  
          return "[FDB] FIB Dump failed ";   
        case CTC_E_FDB_SECURITY_VIOLATION:  
          return "[FDB] Security violation ";   
        case CTC_E_NH_NOT_INIT:  
          return "[Nexthop] Nexhexthop not init ";   
        case CTC_E_NH_INVALID_NHID:  
          return "[Nexthop] Nexthop ID is invalid value ";   
        case CTC_E_NH_NOT_EXIST:  
          return "[Nexthop] Nexthop not exist ";   
        case CTC_E_NH_EXIST:  
          return "[Nexthop] Nexthop  exist ";   
        case CTC_E_NH_INVALID_DSEDIT_PTR:  
          return "[Nexthop] L2DsEditPtr is invalid value";   
        case CTC_E_NH_INVALID_NH_TYPE:  
          return "[Nexthop] Nexthop type is invalid ";   
        case CTC_E_NH_INVALID_NH_SUB_TYPE:  
          return "[Nexthop] Nexthop sub type is invalid ";   
        case CTC_E_NH_INVALID_VLAN_EDIT_TYPE:  
          return "[Nexthop] Egress Vlan Edit type is invalid ";   
        case CTC_E_NH_INVALID_DSNH_TYPE:  
          return "[Nexthop] DsNexthop table type is invalid ";   
        case CTC_E_NH_VLAN_EDIT_CONFLICT:  
          return "[Nexthop] Configure confilct, if cvlan(svlan) edit type is insert-vlan, svlan(cvlan) edit type should not be edit-none or edit-replace ";   
        case CTC_E_NH_STATS_NOT_EXIST:  
          return "[Nexthop] Nexthop stats not exist ";   
        case CTC_E_NH_STATS_EXIST:  
          return "[Nexthop] Nexthop stats exist ";   
        case CTC_E_NH_INVALID_MARTINI_SEQ_TYPE:  
          return "[Nexthop] Invalid martini seq type ";   
        case CTC_E_NH_SHOULD_USE_DSNH8W:  
          return "[Nexthop] Should use DsNexthop8W table ";   
        case CTC_E_NH_SHOULD_USE_DSNH4W:  
          return "[Nexthop] Should use DsNexthop4W table ";   
        case CTC_E_NH_NHID_NOT_MATCH_NHTYPE:  
          return "[Nexthop] Nexthop type doesn't match ";   
        case CTC_E_NH_GLB_SRAM_IS_INUSE:  
          return "[Nexthop] Global DsNexthop offset or GroupId is used in other nexthop, can't re-use it ";   
        case CTC_E_NH_GLB_SRAM_ISNOT_INUSE:  
          return "[Nexthop] Global DsNexthop offset or GroupId isn't used,  can't free it ";   
        case CTC_E_NH_GLB_SRAM_INDEX_EXCEED:  
          return "[Nexthop] Global DsNexthop offset or GroupId is out of range of offset value";   
        case CTC_E_NH_NO_LABEL:  
          return "[Nexthop] Need at least one mpls label";   
        case CTC_E_NH_ISNT_UNROV:  
          return "[Nexthop] Current nexthop isn't unresolved nexthop ";   
        case CTC_E_NH_IS_UNROV:  
          return "[Nexthop] Current nexthop is unresolved nexthop ";   
        case CTC_E_NH_EXCEED_MAX_DSNH_OFFSET:  
          return "[Nexthop] Exceed the max number of dsnh offset ";   
        case CTC_E_NH_EXCEED_MAX_LOGICAL_REPLI_CNT:  
          return "[Nexthop] Exceed the max logical replicate number ";   
        case CTC_E_NH_MEMBER_IS_UPMEP:  
          return "[Nexthop] Member is upmep ";   
        case CTC_E_NH_EGS_EDIT_CONFLICT_VLAN_AND_MACDA_EDIT:  
          return "[Nexthop] Vlan edit and change MAC DA edit exist conflict ";   
        case CTC_E_NH_EXIST_VC_LABEL:  
          return "[Nexthop] Other nexthop are using already";   
        case CTC_E_NH_NOT_EXIST_TUNNEL_LABEL:  
          return "[Nexthop] Tunnel label not exist";   
        case CTC_E_NH_NHOFFSET_EXHAUST:  
          return "[Nexthop] Nexthop offset is exhausted";   
        case CTC_E_NH_TOO_MANY_LABELS:  
          return "[Nexthop] Nexthop offset is exhausted";   
        case CTC_E_NH_EXCEED_MAX_ECMP_NUM:  
          return "[Nexthop] Exceed the max number of ecmp ";   
        case CTC_E_NH_ECMP_MEM_NOS_EXIST:  
          return "[Nexthop] ECMP member not exist! ";   
        case CTC_E_NH_TUNNEL_GRE_KEY_IS_TOO_BIG:  
          return "[Nexthop] GRE key is too big, it is only 16 bit when enable mtu check ";   
        case CTC_E_NH_ECMP_IN_USE:  
          return "[Nexthop] ECMP already in use, max path can not be reset ";   
        case CTC_E_NH_HR_NH_IN_USE:  
          return "[Nexthop] Host nexthop already in use. ";   
        case CTC_E_NH_CROSS_CHIP_RSPAN_NH_IN_USE:  
          return "[Nexthop] CrossChip RSPAN nexthop already in use. ";   
        case CTC_E_NH_MCAST_MIRROR_NH_IN_USE:  
          return "[Nexthop] Mcast mirror already in use. ";   
        case CTC_E_NH_EXCEED_MCAST_PHY_REP_NUM:  
          return "[Nexthop] Exceed the max physical replicate number . ";   
        case CTC_E_NH_LABEL_IS_MISMATCH_WITH_STATS:  
          return "[Nexthop] When support two tunnel label, can not support stats ";   
        case CTC_E_NH_SUPPORT_PW_APS_UPDATE_BY_INT_ALLOC_DSNH:  
          return "[Nexthop] Only sdk allocate dsnh offset support update form normal pw to aps pw,or reverse";   
        case CTC_E_NH_INVALID_TPID_INDEX:  
          return "[Nexthop] Nexthop svlan tpid index is invalid";   
        case CTC_E_L3IF_VMAC_NOT_EXIST:  
          return "[L3if] Vmac not exist! ";   
        case CTC_E_L3IF_VMAC_INVALID_PREFIX_TYPE:  
          return "[L3if] The prefix_type for vmac is invalid! ";   
        case CTC_E_L3IF_VMAC_ENTRY_EXCEED_MAX_SIZE:  
          return "[L3if] Vmac entry exceed the max size! ";   
        case CTC_E_L3IF_ROUTED_EXCEED_IF_SIZE:  
          return "[L3if] Routed port(Sub interface) entry exceed the max size! ";   
        case CTC_E_L3IF_INVALID_IF_TYPE:  
          return "[L3if] Invalid  L3 interface type ";   
        case CTC_E_L3IF_INVALID_IF_ID:  
          return "[L3if] Invalid interface ID! ";   
        case CTC_E_L3IF_NOT_EXIST:  
          return "[L3if] Not exist! ";   
        case CTC_E_L3IF_EXIST:  
          return "[L3if] Already exist! ";   
        case CTC_E_L3IF_MISMATCH:  
          return "[L3if] Information is mismatch ";   
        case CTC_E_L3IF_NO_ALLOCED_IPUCSA:  
          return "[L3if] No alloced ipucsa ";   
        case CTC_E_L3IF_INVALID_PBR_LABEL:  
          return "[L3if] Invalid L3 interface pbr label! ";   
        case CTC_E_L3IF_INVALID_RPF_CHECK_TYPE:  
          return "[L3if] Invalid RPF check type! ";   
        case CTC_E_L3IF_INVALID_RPF_STRICK_TYPE:  
          return "[L3if] Invalid RPF strick type! ";   
        case CTC_E_L3IF_INVALID_ACL_LABEL:  
          return "[L3if] Invalid acl vlan label! ";   
        case CTC_E_L3IF_VRF_STATS_NOT_EXIST:  
          return "[L3if] VRF stats not exist ";   
        case CTC_E_L3IF_ROUTER_MAC_EXHAUSTED:  
          return "[L3if] Router mac exhausted ";   
        case CTC_E_ACLQOS_ENTRY_NOT_EXIST:  
          return "[AclQos] Entry not exist ";   
        case CTC_E_ACLQOS_ENTRY_EXIST:  
          return "[AclQos] Entry exist ";   
        case CTC_E_ACLQOS_INVALID_ACTION:  
          return "[AclQos] Invalid action ";   
        case CTC_E_ACLQOS_INVALID_KEY_TYPE:  
          return "[AclQos] Invalid key type ";   
        case CTC_E_ACLQOS_INVALID_LABEL_TYPE:  
          return "[AclQos] Invalid label type ";   
        case CTC_E_ACLQOS_LABEL_NOT_EXIST:  
          return "[AclQos] Label not exist ";   
        case CTC_E_ACLQOS_HAVE_ENABLED:  
          return "[AclQos] Have enabled ";   
        case CTC_E_ACLQOS_NOT_ENABLED:  
          return "[AclQos] Not enabled ";   
        case CTC_E_ACLQOS_LABEL_IN_USE:  
          return "[AclQos] Label in use ";   
        case CTC_E_QOS_POLICER_CREATED:  
          return "[Qos] Policer created ";   
        case CTC_E_QOS_POLICER_NOT_EXIST:  
          return "[Qos] Policer not exist ";   
        case CTC_E_QOS_POLICER_NOT_BIND:  
          return "[Qos] Policer not_bind ";   
        case CTC_E_QOS_POLICER_IN_USE:  
          return "[Qos] Policer in_use ";   
        case CTC_E_QOS_POLICER_CIR_GREATER_THAN_PIR:  
          return "[Qos] Policer cir greater than pir ";   
        case CTC_E_QOS_POLICER_CBS_GREATER_THAN_PBS:  
          return "[Qos] Policer cbs greater than pbs ";   
        case CTC_E_QOS_POLICER_SERVICE_POLICER_NOT_ENABLE:  
          return "[Qos] Policer service policer not enable ";   
        case CTC_E_QOS_POLICER_STATS_NOT_ENABLE:  
          return "[Qos] Policer stats not enable ";   
        case CTC_E_QOS_QUEUE_STATS_NOT_ENABLE:  
          return "[Qos] Queue stats not enable ";   
        case CTC_E_QOS_NO_INT_POLICER_ENTRY:  
          return "[Qos] No available internal policer entries ";   
        case CTC_E_QOS_NO_EXT_POLICER_ENTRY:  
          return "[Qos] No available external policer entries ";   
        case CTC_E_QOS_NO_INT_POLICER_PROFILE_ENTRY:  
          return "[Qos] No available internal policer profile entries ";   
        case CTC_E_ACLQOS_DIFFERENT_DIR:  
          return "[AclQos] Different direction ";   
        case CTC_E_ACLQOS_DIFFERENT_TYPE:  
          return "[AclQos] Different type ";   
        case CTC_E_ACLQOS_DIFFERENT_CHIP:  
          return "[AclQos] Different chip ";   
        case CTC_E_ACL_PBR_ECMP_CMP_FAILED:  
          return "[Acl] PBR ECMP entry compare failed, can not do ecmp ";   
        case CTC_E_ACL_PBR_DO_ECMP_FAILED:  
          return "[Acl] Pbr do ecmp failed ";   
        case CTC_E_ACL_PBR_ENTRY_NO_NXTTHOP:  
          return "[Acl] Pbr permit entry, must add fwd-to-nhid action ";   
        case CTC_E_QOS_THIS_PORT_NOT_SUPPORT_POLICER:  
          return "[Qos] This port not support policer ";   
        case CTC_E_QOS_POLICER_NOT_SUPPORT_STATS:  
          return "[Qos] This policer not support stats ";   
        case CTC_E_ACL_INIT:  
          return "[Acl] Init failed ";   
        case CTC_E_ACL_GROUP_PRIORITY:  
          return "[Acl] Invalid group priority ";   
        case CTC_E_ACL_GROUP_TYPE:  
          return "[Acl] Invalid group type ";   
        case CTC_E_ACL_GROUP_ID:  
          return "[Acl] Invalid group id ";   
        case CTC_E_ACL_GROUP_ID_RSVED:  
          return "[Acl] Group id reserved ";   
        case CTC_E_ACL_ENTRY_EXIST:  
          return "[Acl] Entry exist ";   
        case CTC_E_ACL_ENTRY_UNEXIST:  
          return "[Acl] Entry not exist ";   
        case CTC_E_ACL_GROUP_UNEXIST:  
          return "[Acl] Group not exist ";   
        case CTC_E_ACL_GROUP_EXIST:  
          return "[Acl] Group exist ";   
        case CTC_E_ACL_GROUP_NOT_EMPTY:  
          return "[Acl] Group not empty ";   
        case CTC_E_ACL_INVALID_ACTION:  
          return "[Acl] Invalid action ";   
        case CTC_E_ACL_INVALID_KEY_TYPE:  
          return "[Acl] Invalid key type ";   
        case CTC_E_ACL_INVALID_LABEL_TYPE:  
          return "[Acl] Invalid label type ";   
        case CTC_E_ACL_LABEL_NOT_EXIST:  
          return "[Acl] Label not exist ";   
        case CTC_E_ACL_HAVE_ENABLED:  
          return "[Acl] Have enabled ";   
        case CTC_E_ACL_NOT_ENABLED:  
          return "[Acl] Not enabled ";   
        case CTC_E_ACL_LABEL_IN_USE:  
          return "[Acl] Label in use ";   
        case CTC_E_ACL_FLAG_COLLISION:  
          return "[Acl] Flag collision ";   
        case CTC_E_ACL_L4_PORT_RANGE_EXHAUSTED:  
          return "[Acl] L4 port range exhausted ";   
        case CTC_E_ACL_TCP_FLAG_EXHAUSTED:  
          return "[Acl] Tcp flags exhausted ";   
        case CTC_E_ACL_GET_NODES_FAILED:  
          return "[Acl] Get nodes failed ";   
        case CTC_E_ACL_PORT_BITMAP_OVERFLOW:  
          return "[Acl] Port bitmap overflow ";   
        case CTC_E_ACL_GROUP_INFO:  
          return "[Acl] Group info error ";   
        case CTC_E_ACL_ENTRY_INSTALLED:  
          return "[Acl] Entry installed ";   
        case CTC_E_ACL_GET_STATS_FAILED:  
          return "[Acl] Get stats failed ";   
        case CTC_E_ACL_GROUP_NOT_INSTALLED:  
          return "[Acl] Group not installed ";   
        case CTC_E_ACL_SERVICE_ID:  
          return "[Acl] Invalid service id ";   
        case CTC_E_ACL_GLOBAL_CFG_ERROR:  
          return "[Acl] Global config error ";   
        case CTC_E_ACL_STATS_NOT_SUPPORT:  
          return "[Acl] Stats not support on this priority ";   
        case CTC_E_ACL_ADD_TCAM_ENTRY_TO_WRONG_GROUP:  
          return "[Acl] Add tcam entry to wrong group ";   
        case CTC_E_ACL_ADD_HASH_ENTRY_TO_WRONG_GROUP:  
          return "[Acl] Add hash entry to wrong group ";   
        case CTC_E_ACL_HASH_ENTRY_NO_PRIORITY:  
          return "[Acl] Hash entry has no priority ";   
        case CTC_E_ACL_HASH_ENTRY_UNSUPPORT_COPY:  
          return "[Acl] Hash acl entry not support copy ";   
        case CTC_E_ACL_VLAN_EDIT_INVALID:  
          return "[Acl] Acl vlan edit action invalid ";   
        case CTC_E_ACL_INVALID_EGRESS_ACTION:  
          return "[Acl] Acl invalid egress action ";   
        case CTC_E_ACL_NO_ROOM_TO_MOVE_ENTRY:  
          return "[Acl] Acl no room to move entry ";   
        case CTC_E_ACL_HASH_CONFLICT:  
          return "[Acl] Hash conflict";   
        case CTC_E_ACL_IPV4_NO_LAYER3_TYPE:  
          return "[Acl] IPV4 key must configure layer3 type";   
        case CTC_E_STATS_MTU2_LESS_MTU1:  
          return "[Stats] Mtu2 pkt length should greater than mtu1 pkt length ";   
        case CTC_E_STATS_MTU1_GREATER_MTU2:  
          return "[Stats] Mtu1 pkt length should less than mtu2 pkt length ";   
        case CTC_E_STATS_PORT_STATS_NO_TYPE:  
          return "[Stats] Port stats option type is none ";   
        case CTC_E_STATS_PHB_STATS_INVALID:  
          return "[Stats] Do phb/per-port-phb stats in qos module ";   
        case CTC_E_STATS_PORT_NOT_MAP_TO_MAC:  
          return "[Stats] Port not map to mac ";   
        case CTC_E_STATS_PORT_NOT_ENABLE:  
          return "[Stats] Port not enable ";   
        case CTC_E_STATS_MAC_STATS_MODE_ASIC:  
          return "[Stats] Mac stats information from asic ";   
        case CTC_E_STATS_TABLE_NUM_NO_EXPECT:  
          return "[Stats] Stats table entry get from ftm not expect ";   
        case CTC_E_STATS_VLAN_STATS_CONFLICT_WITH_PHB:  
          return "[Stats] Vlan stats is conflict with phb ";   
        case CTC_E_STATS_NOT_ENABLE:  
          return "[Stats] Stats is not enabled ";   
        case CTC_E_STATS_DMA_ENABLE_BUT_SW_DISABLE:  
          return "[Stats] When dma sync is enabled, sw mode must be enabled also ";   
        case CTC_E_STATS_STATSID_EXIST:  
          return "[Stats] Statsid have been alloced ";   
        case CTC_E_STATS_STATSID_TYPE_MISMATCH:  
          return "[Stats] Stats_id type is mismatch ";   
        case CTC_E_STATS_STATSID_ALREADY_IN_USE:  
          return "[Stats] Statsid have been used ";   
        case CTC_E_IPUC_VERSION_DISABLE:  
          return "[IPUC] SDK is not assigned resource for this version ";   
        case CTC_E_IPUC_INVALID_VRF:  
          return "[IPUC] Invalid vrfid ";   
        case CTC_E_IPUC_INVALID_L3IF:  
          return "[IPUC] L3if is not needed ";   
        case CTC_E_IPUC_NEED_L3IF:  
          return "[IPUC] Need l3if ";   
        case CTC_E_IPUC_TUNNEL_INVALID:  
          return "[IPUC] SDK is not assigned resource for tunnel of this IP version ";   
        case CTC_E_IPUC_RPF_NOT_SUPPORT_THIS_MASK_LEN:  
          return "[IPUC] IPv6 rpf check not support mask length 128 ";   
        case CTC_E_IPUC_NAT_NOT_SUPPORT_THIS_MASK_LEN:  
          return "[IPUC] NAPT not support this mask length. for IPV4, only support mask length 32 ";   
        case CTC_E_IPUC_INVALID_TTL_THRESHOLD:  
          return "[IPUC] Invalid TTL threshold ";   
        case CTC_E_IPUC_TUNNEL_RPF_INFO_MISMATCH:  
          return "[IPUC] IP-tunnel RPF info is mismatch with former config ";   
        case CTC_E_IPUC_ADD_FAILED:  
          return "[IPUC] Add ipuc route failed ";   
        case CTC_E_IPUC_TUNNEL_PAYLOAD_TYPE_MISMATCH:  
          return "[IPUC] IP-tunnel payload type is mismatch with grekey ";   
        case CTC_E_IPUC_HASH_CONFLICT:  
          return "[IPUC] Hash confict ";   
        case CTC_E_IPUC_INVALID_ROUTE_FLAG:  
          return "[IPUC] route flag is confict ";   
        case CTC_E_IPMC_GROUP_NOT_EXIST:  
          return "[IPMC] This group does NOT exist ";   
        case CTC_E_IPMC_GROUP_HAS_EXISTED:  
          return "[IPMC] This group has already existed ";   
        case CTC_E_IPMC_INVALID_MASK_LEN:  
          return "[IPMC] Mask length error ";   
        case CTC_E_IPMC_BAD_L3IF_TYPE:  
          return "[IPMC] Unknown l3if type ";   
        case CTC_E_IPMC_NOT_MCAST_ADDRESS:  
          return "[IPMC] Group address is NOT multicast IP address ";   
        case CTC_E_IPMC_EXCEED_GLOBAL_CHIP:  
          return "[IPMC] Target chip exceeds global chip id ";   
        case CTC_E_IPMC_ADD_MEMBER_FAILED:  
          return "[IPMC] Add member failed ";   
        case CTC_E_IPMC_GROUP_IS_NOT_IP_MAC:  
          return "[IPMC] The group is not a IP based l2 mcast group ";   
        case CTC_E_IPMC_GROUP_IS_IP_MAC:  
          return "[IPMC] The group is a IP based l2 mcast group ";   
        case CTC_E_IPMC_INVALID_VRF:  
          return "[IPMC] Invalid vrfid ";   
        case CTC_E_IPMC_GROUP_IS_NOT_INIT:  
          return "[IPMC] The group is not init ";   
        case CTC_E_IPMC_VERSION_DISABLE:  
          return "[IPMC] SDK is not assigned resource for this version ";   
        case CTC_E_IPMC_RPF_CHECK_DISABLE:  
          return "[IPMC] Rpf check disable ";   
        case CTC_E_IPMC_OFFSET_ALLOC_ERROR:  
          return "[IPMC] Offset alloc error ";   
        case CTC_E_IPMC_STATS_NOT_EXIST:  
          return "[IPMC] Stats not exist ";   
        case CTC_E_IPMC_HASH_CONFLICT:  
          return "[IPMC] Ipmc hash conflict ";   
        case CTC_E_PDU_INVALID_ACTION_INDEX:  
          return "[PDU] Invalid action index ";   
        case CTC_E_PDU_INVALID_INDEX:  
          return "[PDU] Invalid index ";   
        case CTC_E_PDU_INVALID_IPDA:  
          return "[PDU] Invalid ipda ";   
        case CTC_E_PDU_INVALID_MACDA:  
          return "[PDU] Invalid macda ";   
        case CTC_E_PDU_INVALID_MACDA_MASK:  
          return "[PDU] Invalid macda mask ";   
        case CTC_E_QUEUE_DROP_PROF_NOT_EXIST:  
          return "[Queue] Drop profile don't exist ";   
        case CTC_E_QUEUE_SHAPE_PROF_NOT_EXIST:  
          return "[Queue] Shape profile don't exist ";   
        case CTC_E_QUEUE_CHANNEL_SHAPE_PROF_EXIST:  
          return "[Queue] Channel shape profile exist ";   
        case CTC_E_QUEUE_CHANNEL_SHAPE_PROF_NOT_EXIST:  
          return "[Queue] Channel shape profile not exist ";   
        case CTC_E_QUEUE_NOT_ENOUGH:  
          return "[Queue] Queue not enough ";   
        case CTC_E_QUEUE_SERVICE_QUEUE_NOT_INITIALIZED:  
          return "[Queue] Service queue not initialized ";   
        case CTC_E_QUEUE_SERVICE_QUEUE_INITIALIZED:  
          return "[Queue] Service queue initialized ";   
        case CTC_E_QUEUE_SERVICE_EXCEED_MAX_COUNT_LIMIT:  
          return "[Queue] The number of services reaches the maximum limit ";   
        case CTC_E_QUEUE_SERVICE_NOT_EXIST:  
          return "[Queue] Service don't exist ";   
        case CTC_E_QUEUE_SERVICE_REGISTERED:  
          return "[Queue] Service already registered ";   
        case CTC_E_QUEUE_SERVICE_EXIST:  
          return "[Queue] Service already exist ";   
        case CTC_E_QUEUE_SERVICE_QUEUE_NO_HASH_ENTRY:  
          return "[Queue] Service queue have no hash entry ";   
        case CTC_E_QUEUE_SERVICE_GROUP_NOT_EXIST:  
          return "[Queue] Service group don't exist ";   
        case CTC_E_QUEUE_INTERNAL_PORT_IN_USE:  
          return "[Queue] Internal port in use ";   
        case CTC_E_QUEUE_NO_FREE_INTERNAL_PORT:  
          return "[Queue] Internal port no free count ";   
        case CTC_E_QUEUE_DEPTH_NOT_EMPTY:  
          return "[Queue] Depth is not empty ";   
        case CTC_E_QUEUE_WDRR_WEIGHT_TOO_BIG:  
          return "[Queue] Wdrr weight is too big ";   
        case CTC_E_QUEUE_INVALID_CONFIG:  
          return "[Queue] Config is invalid  ";   
        case CTC_E_QUEUE_CPU_REASON_NOT_CREATE:  
          return "[CPU]  CPU Reason not create ";   
        case CTC_E_APS_GROUP_EXIST:  
          return "[Aps] Group exist ";   
        case CTC_E_APS_INTERFACE_ERROR:  
          return "[Aps] L3 if and port not match ";   
        case CTC_E_APS_INVALID_GROUP_ID:  
          return "[Aps] Invalid group id ";   
        case CTC_E_APS_INVALID_TYPE:  
          return "[Aps] Invalid select type ";   
        case CTC_E_APS_GROUP_NOT_EXIST:  
          return "[Aps] Group don't exist ";   
        case CTC_E_APS_GROUP_NEXT_APS_EXIST:  
          return "[Aps] Next aps group exist ";   
        case CTC_E_APS_MPLS_TYPE_L3IF_NOT_EXIST:  
          return "[Aps] Mpls aps must configure l3if id ";   
        case CTC_E_APS_RAPS_GROUP_NOT_EXIST:  
          return "[Aps] Raps group don't exist ";   
        case CTC_E_APS_DONT_SUPPORT_2_LEVEL_HW_BASED_APS:  
          return "[Aps] Aps only support hw-based aps for 1 level ";   
        case CTC_E_APS_DONT_SUPPORT_HW_BASED_APS_FOR_LINKAGG:  
          return "[Aps] Aps only support hw-based aps for phyport ";   
        case CTC_E_VPLS_VSI_NOT_CREATE:  
          return "[Vpls] Instance is not created ";   
        case CTC_E_VPLS_INVALID_VPLSPORT_ID:  
          return "[Vpls] Invalid logic port id ";   
        case CTC_E_HW_OP_FAIL:  
          return "[ASIC] Hardware operation failed ";   
        case CTC_E_DRV_FAIL:  
          return "[ASIC] Driver init failed ";   
        case CTC_E_CHECK_HSS_READY_FAIL:  
          return "[ASIC] Hss4G Macro change failed >";   
        case CTC_E_OAM_NOT_INIT:  
          return "[OAM] Module not initialized ";   
        case CTC_E_OAM_MAID_LENGTH_INVALID:  
          return "[OAM] Maid length invalid ";   
        case CTC_E_OAM_MAID_ENTRY_EXIST:  
          return "[OAM] Maid entry exist ";   
        case CTC_E_OAM_MAID_ENTRY_NOT_FOUND:  
          return "[OAM] Maid entry not found ";   
        case CTC_E_OAM_MAID_ENTRY_REF_BY_MEP:  
          return "[OAM] Maid entry used by local mep, can not remove ";   
        case CTC_E_OAM_CHAN_ENTRY_EXIST:  
          return "[OAM] MEP or MIP lookup key exists ";   
        case CTC_E_OAM_CHAN_ENTRY_NOT_FOUND:  
          return "[OAM] MEP or MIP lookup key not found ";   
        case CTC_E_OAM_CHAN_MD_LEVEL_EXIST:  
          return "[OAM] MD level already config other";   
        case CTC_E_OAM_CHAN_MD_LEVEL_NOT_EXIST:  
          return "[OAM] MD level not exist";   
        case CTC_E_OAM_CHAN_ENTRY_NUM_EXCEED:  
          return "[OAM] Lookup key entry full in EVC";   
        case CTC_E_OAM_LM_NUM_RXCEED:  
          return "[OAM] LM entry full in EVC";   
        case CTC_E_OAM_CHAN_LMEP_EXIST:  
          return "[OAM] Local mep already exists";   
        case CTC_E_OAM_CHAN_LMEP_NOT_FOUND:  
          return "[OAM] Local mep not found ";   
        case CTC_E_OAM_CHAN_NOT_UP_DIRECTION:  
          return "[OAM] Lookup channel isn't up mep channel ";   
        case CTC_E_OAM_CHAN_NOT_DOWN_DIRECTION:  
          return "[OAM] Lookup channel isn't down mep channel ";   
        case CTC_E_OAM_TX_GPORT_AND_MASTER_GCHIP_NOT_MATCH:  
          return "[OAM] Linkagg's master gchip does not match with tx_ccm_gport's chip ";   
        case CTC_E_OAM_TX_GPORT_AND_CHAN_GPORT_NOT_MATCH:  
          return "[OAM] Ccm gport does not match with channel gport ";   
        case CTC_E_OAM_RMEP_EXIST:  
          return "[OAM] Remote mep already exists ";   
        case CTC_E_OAM_RMEP_NOT_FOUND:  
          return "[OAM] Remote mep is not found ";   
        case CTC_E_OAM_MEP_LM_NOT_EN:  
          return "[OAM] LM not enable in MEP";   
        case CTC_E_OAM_INVALID_MD_LEVEL:  
          return "[OAM] Md level is invalid";   
        case CTC_E_OAM_INVALID_MEP_ID:  
          return "[OAM] Mep id is invalid";   
        case CTC_E_OAM_INVALID_MEP_CCM_INTERVAL:  
          return "[OAM] Mep ccm interval is invalid ";   
        case CTC_E_OAM_INVALID_MEP_TPID_INDEX:  
          return "[OAM] Mep tpid index is invalid";   
        case CTC_E_OAM_INVALID_MEP_COS:  
          return "[OAM] Mep cos is invalid";   
        case CTC_E_OAM_INVALID_CFG_IN_MEP_TYPE:  
          return "[OAM] Config is invalid in this mep type";   
        case CTC_E_OAM_INVALID_CFG_LM:  
          return "[OAM] Config is invalid lm is Enable";   
        case CTC_E_OAM_INVALID_LM_TYPE:  
          return "[OAM] Lm type is invalid";   
        case CTC_E_OAM_INVALID_LM_COS_TYPE:  
          return "[OAM] Lm cos type is invalid";   
        case CTC_E_OAM_INVALID_CSF_TYPE:  
          return "[OAM] CSF type is invalid";   
        case CTC_E_OAM_NH_OFFSET_EXIST:  
          return "[OAM] Nexthop offset for down mep already exists ";   
        case CTC_E_OAM_NH_OFFSET_NOT_FOUND:  
          return "[OAM] Nexthop offset for down mep is not found ";   
        case CTC_E_OAM_NH_OFFSET_ADD_VECTOR_FAIL:  
          return "[OAM] Add next hop offset for down mep fail ";   
        case CTC_E_OAM_NH_OFFSET_IN_USE:  
          return "[OAM] Nexthop offset for down mep is in use ";   
        case CTC_E_OAM_RMEP_D_LOC_PRESENT:  
          return "[OAM] Some rmep still has dloc defect ";   
        case CTC_E_OAM_ITU_DEFECT_CLEAR_MODE:  
          return "[OAM] Clear the defect fail in itu defect clear mode ";   
        case CTC_E_OAM_MEP_INDEX_VECTOR_ADD_FAIL:  
          return "[OAM] Add mep to index vector fail ";   
        case CTC_E_OAM_INVALID_OPERATION_ON_CPU_MEP:  
          return "[OAM] Invalid operation on mep configured for cpu ";   
        case CTC_E_OAM_INVALID_OPERATION_ON_P2P_MEP:  
          return "[OAM] Invalid operation on p2p mep configured";   
        case CTC_E_OAM_DRIVER_FAIL:  
          return "[OAM] Driver fail ";   
        case CTC_E_OAM_EXCEED_MAX_LOOP_BACK_PORT_NUMBER:  
          return "[OAM] Loop back port number exceed max value ";   
        case CTC_E_OAM_INVALID_GLOBAL_PARAM_TYPE:  
          return "[OAM] Global param type is invalid  ";   
        case CTC_E_OAM_INDEX_EXIST:  
          return "[OAM] Index already exist ";   
        case CTC_E_OAM_INVALID_MA_INDEX:  
          return "[OAM] Ma index is invalid ";   
        case CTC_E_OAM_INVALID_MEP_INDEX:  
          return "[OAM] Oam mep index is invalid  ";   
        case CTC_E_OAM_INVALID_OAM_TYPE:  
          return "[OAM] Oam type is invalid  ";   
        case CTC_E_OAM_EXCEED_MAX_LEVEL_NUM_PER_CHAN:  
          return "[OAM] Exceed max level num per channel";   
        case CTC_E_OAM_BFD_INVALID_STATE:  
          return "[OAM] BFD state is invalid";   
        case CTC_E_OAM_BFD_INVALID_DIAG:  
          return "[OAM] BFD diag is invalid";   
        case CTC_E_OAM_BFD_INVALID_INTERVAL:  
          return "[OAM] BFD interval is invalid";   
        case CTC_E_OAM_BFD_INVALID_DETECT_MULT:  
          return "[OAM] BFD detect mult is invalid";   
        case CTC_E_OAM_BFD_INVALID_UDP_PORT:  
          return "[OAM] BFD Source UDP port is invalid";   
        case CTC_E_OAM_TRPT_NOT_INIT:  
          return "[OAM] Throughput Module not initialized ";   
        case CTC_E_OAM_TRPT_INTERVAL_CONFLICT:  
          return "[OAM] Throughput rate conflict ";   
        case CTC_E_OAM_TRPT_SESSION_EN:  
          return "[OAM] The session is ongoing ";   
        case CTC_E_OAM_TRPT_SESSION_ALREADY_EN:  
          return "[OAM] The session is ongoing ";   
        case CTC_E_OAM_TRPT_SESSION_NOT_CFG:  
          return "[OAM] The session is not config ";   
        case CTC_E_OAM_TRPT_PERIOD_INVALID:  
          return "[OAM] Throughput send packet period is invalid ";   
        case CTC_E_PTP_NOT_INIT:  
          return "[PTP] Module not initialized ";   
        case CTC_E_PTP_EXCEED_MAX_FRACTIONAL_VALUE:  
          return "[PTP] Exceed max ns or fractional ns value, must less than 10^9 ";   
        case CTC_E_PTP_EXCEED_MAX_FFO_VALUE:  
          return "[PTP] Invalid sync interface param ";   
        case CTC_E_PTP_INVALID_SYNC_INTF_PARAM_VALUE:  
          return "[PTP] Invalid sync interface param value ";   
        case CTC_E_PTP_TS_NOT_READY:  
          return "[PTP] Time stamp is not ready ";   
        case CTC_E_PTP_TX_TS_ROLL_OVER_FAILURE:  
          return "[PTP] TX time stamp roll over failure ";   
        case CTC_E_PTP_TS_GET_BUFF_FAIL:  
          return "[PTP] Fail to get ts from buffer ";   
        case CTC_E_PTP_TS_ADD_BUFF_FAIL:  
          return "[PTP] Fail to add ts to buffer ";   
        case CTC_E_PTP_RX_TS_INFORMATION_NOT_READY:  
          return "[PTP] RX TS from sync interface or tod interface is not ready ";   
        case CTC_E_PTP_EXCEED_MAX_DRIFT_VALUE:  
          return "[PTP] Exceed max drift value ";   
        case CTC_E_PTP_TAI_LESS_THAN_GPS:  
          return "[PTP] TAI second is less than gps second ";   
        case CTC_E_PTP_CAN_NOT_SET_TOW:  
          return "[PTP] ToW can not be set at this time ";   
        case CTC_E_PTP_SYNC_SIGNAL_FREQUENCY_OUT_OF_RANGE:  
          return "[PTP] Signal frequency is out of range";   
        case CTC_E_PTP_SYNC_SIGNAL_NOT_VALID:  
          return "[PTP] Invalid signal frequency";   
        case CTC_E_PTP_SYNC_SIGNAL_DUTY_OUT_OF_RANGE:  
          return "[PTP] Signal duty is out of range";   
        case CTC_E_PTP_INVALID_SYNC_INTF_ACCURACY_VALUE:  
          return "[PTP] Invalid sync interface accuracy value";   
        case CTC_E_PTP_INVALID_SYNC_INTF_EPOCH_VALUE:  
          return "[PTP] Invalid sync interface epoch value";   
        case CTC_E_PTP_INVALID_TOD_INTF_PARAM_VALUE:  
          return "[PTP] Invalid tod interface param value";   
        case CTC_E_PTP_INVALID_TOD_INTF_EPOCH_VALUE:  
          return "[PTP] Invalid tod interface epoch value";   
        case CTC_E_PTP_INVALID_TOD_INTF_PPS_STATUS_VALUE:  
          return "[PTP] Invalid tod interface 1PPS status value";   
        case CTC_E_PTP_INVALID_TOD_INTF_CLOCK_SRC_VALUE:  
          return "[PTP] Invalid tod interface clock source value";   
        case CTC_E_PTP_INVALID_CAPTURED_TS_SEQ_ID:  
          return "[PTP] Invalid captured ts sequece id";   
        case CTC_E_PTP_INVALID_CAPTURED_TS_SOURCE:  
          return "[PTP] Invalid captured ts source";   
        case CTC_E_PTP_INVALID_CAPTURED_TYPE:  
          return "[PTP] Invalid captured ts type";   
        case CTC_E_LEARNING_AND_AGING_INVALID_AGING_THRESHOLD:  
          return "[LEARNING_AGING] Invalid aging threshold ";   
        case CTC_E_LEARNING_AND_AGING_INVALID_LEARNING_THRESHOLD:  
          return "[LEARNING_AGING] Invalid learning threshold ";   
        case CTC_E_AGING_INVALID_INTERVAL:  
          return "[LEARNING_AGING] Invalid aging interval ";   
        case CTC_E_INVALID_MAC_FILTER_SIZE:  
          return "[LEARNING_AGING] Invalid mac filter size ";   
        case CTC_E_ALLOCATION_INVALID_ENTRY_SIZE:  
          return "[FTM] Entry size is invalid,must be multiple of 256 (80bit) ";   
        case CTC_E_ALLOCATION_INVALID_KEY_SIZE:  
          return "[FTM] Key size is invalid  ";   
        case CTC_E_ALLOCATION_EXCEED_INT_TCAM_PHYSIZE:  
          return "[FTM] Internal tcam physize exceed ";   
        case CTC_E_ALLOCATION_EXCEED_EXT_TCAM_PHYSIZE:  
          return "[FTM] External tcam physize exceed ";   
        case CTC_E_ALLOCATION_EXCEED_HASH_PHYSIZE:  
          return "[FTM] Hash physize exceed max";   
        case CTC_E_ALLOCATION_INVALID_HASH_PHYSIZE:  
          return "[FTM] Hash physize is invalid ";   
        case CTC_E_ALLOCATION_EXCEEED_SRAM_PHYSIZE:  
          return "[FTM] Sram physize exceed max";   
        case CTC_E_ALLOCATION_EXCEED_MAX_OAM:  
          return "[FTM] Oam num exceed max ";   
        case CTC_E_HASH_MEM_INIT_TIMEOUT:  
          return "[FTM] Hash ram init timeout ";   
        case CTC_E_MIRROR_EXCEED_SESSION:  
          return "[Mirror] Exceed mirror session num ";   
        case CTC_E_MIRROR_INVALID_MIRROR_INFO_TYPE:  
          return "[Mirror] Invalid mirror info type ";   
        case CTC_E_MIRROR_INVALID_MIRROR_LOG_ID:  
          return "[Mirror] Invalid mirror log id ";   
        case CTC_E_MPLS_ENTRY_NOT_SUPPORT_STATS:  
          return "[MPLS] Entry not support statistics ";   
        case CTC_E_MPLS_ENTRY_STATS_EXIST:  
          return "[MPLS] Statistics entry exist ";   
        case CTC_E_MPLS_ENTRY_STATS_NOT_EXIST:  
          return "[MPLS] Statistics entry not exist ";   
        case CTC_E_MPLS_NO_RESOURCE:  
          return "[MPLS] No resource ";   
        case CTC_E_MPLS_SPACE_ERROR:  
          return "[MPLS] Space don't enable ";   
        case CTC_E_MPLS_LABEL_ERROR:  
          return "[MPLS] Label  don't correct ";   
        case CTC_E_MPLS_LABEL_TYPE_ERROR:  
          return "[MPLS] Label Type don't correct ";   
        case CTC_E_MPLS_VRF_ID_ERROR:  
          return "[MPLS] VRF ID don't correct ";   
        case CTC_E_MPLS_SPACE_NO_RESOURCE:  
          return "[MPLS] User allocate num larger than sdk allocate, please modify chip_profile.cfg ";   
        case CTC_E_MPLS_LABEL_RANGE_ERROR:  
          return "[MPLS] User allocate label range error ";   
        case CTC_E_DMA_TX_FAILED:  
          return "[DMA] Packet DMA transmit failed ";   
        case CTC_E_DMA_TABLE_WRITE_FAILED:  
          return "[DMA] Table DMA write failed ";   
        case CTC_E_DMA_TABLE_READ_FAILED:  
          return "[DMA] Table DMA read failed ";   
        case CTC_E_DMA_DESC_INVALID:  
          return "[DMA] Invalid descriptor ";   
        case CTC_E_DMA_INVALID_PKT_LEN:  
          return "[DMA] Invalid length of packet ";   
        case CTC_E_DMA_NOT_INIT:  
          return "[DMA] DMA Module is not init ";   
        case CTC_E_DMA_INVALID_SUB_TYPE:  
          return "[DMA] Invalid DMA sub isr type ";   
        case CTC_E_DMA_INVALID_CHAN_ID:  
          return "[DMA] Invalid DMA channel ID ";   
        case CTC_E_DMA_EXCEED_MAX_DESC_NUM:  
          return "[DMA] Exceed Max Desc num ";   
        case CTC_E_DMA_DESC_NOT_DONE:  
          return "[DMA] Desc not done ";   
        case CTC_E_PARSER_INVALID_L3_TYPE:  
          return "[Parser] Layer3 type is invalid ";   
        case CTC_E_STK_NOT_INIT:  
          return "[STACKING] Module not initialized ";   
        case CTC_E_STK_INVALID_TRUNK_ID:  
          return "[STACKING] Trunk id is invalid ";   
        case CTC_E_STK_TRUNK_NOT_EXIST:  
          return "[STACKING] Trunk not exist ";   
        case CTC_E_STK_TRUNK_HAS_EXIST:  
          return "[STACKING] Trunk have exist ";   
        case CTC_E_STK_TRUNK_EXCEED_MAX_MEMBER_PORT:  
          return "[STACKING] Trunk exceed max member port";   
        case CTC_E_STK_TRUNK_MEMBER_PORT_NOT_EXIST:  
          return "[STACKING] Trunk member port not exist";   
        case CTC_E_STK_TRUNK_MEMBER_PORT_EXIST:  
          return "[STACKING] Trunk member port exist";   
        case CTC_E_STK_TRUNK_MEMBER_PORT_INVALID:  
          return "[STACKING] Trunk member port not valid";   
        case CTC_E_STK_TRUNK_HDR_DSCP_NOT_VALID:  
          return "[STACKING] Trunk header dscp not valid";   
        case CTC_E_STK_TRUNK_HDR_COS_NOT_VALID:  
          return "[STACKING] Trunk header cos not valid";   
        case CTC_E_STK_KEEPLIVE_TWO_MEMBER_AND_ONE_CPU:  
          return "[STACKING] KeepLive only support two member trunk and one cpu port";   
        case CTC_E_SYNCE_NOT_INIT:  
          return "[SYNCE] Module not initialized ";   
        case CTC_E_SYNCE_CLOCK_ID_EXCEED_MAX_VALUE:  
          return "[SYNCE] Clock ID exceed max value ";   
        case CTC_E_SYNCE_DIVIDER_EXCEED_MAX_VALUE:  
          return "[SYNCE] Divider exceed max value ";   
        case CTC_E_SYNCE_INVALID_RECOVERED_PORT:  
          return "[SYNCE] Recovered clock port ID must be integer multiples of 4 ";   
        case CTC_E_SYNCE_NO_SERDES_INFO:  
          return "[SYNCE] Can not get serdes info ";   
        case CTC_E_INTR_NOT_INIT:  
          return "[INTERRUPT] Module not initialized ";   
        case CTC_E_INTR_INVALID_TYPE:  
          return "[INTERRUPT] sup-level type is invalid ";   
        case CTC_E_INTR_INVALID_SUB_TYPE:  
          return "[INTERRUPT] sub-level type is invalid ";   
        case CTC_E_INTR_NOT_SUPPORT:  
          return "[INTERRUPT] operation does not support ";   
        case CTC_E_INTR_INVALID_PARAM:  
          return "[INTERRUPT] parameter is invalid ";   
        case CTC_E_STMCTL_INVALID_UC_MODE:  
          return "[STMCTL] Invalid unicast storm ctl mode ";   
        case CTC_E_STMCTL_INVALID_MC_MODE:  
          return "[STMCTL] Invalid mcast storm ctl mode ";   
        case CTC_E_STMCTL_INVALID_THRESHOLD:  
          return "[STMCTL] Invalid storm ctl threshold ";   
        case CTC_E_STMCTL_INVALID_PKT_TYPE:  
          return "[STMCTL] Invalid storm ctl packet type ";   
        case CTC_E_BPE_NOT_INIT:  
          return "[BPE] Module not initialized ";   
        case CTC_E_BPE_INVALID_PORT_BASE:  
          return "[BPE] Invalid BPE port base ";   
        case CTC_E_BPE_INVALID_VLAN_BASE:  
          return "[BPE] Invalid BPE vlan base ";   
        case CTC_E_BPE_INVALID_BPE_CAPABLE_GPORT:  
          return "[BPE] Invalid BPE capable gport ";   
        case CTC_E_BPE_INVLAID_BPE_MODE:  
          return "[BPE] Invalid BPE mode ";   
        case CTC_E_OVERLAY_TUNNEL_INVALID_TYPE:  
          return "[OVERLAY_TUNNEL] The tunnel type is not correct";   
        case CTC_E_OVERLAY_TUNNEL_INVALID_VN_ID:  
          return "[OVERLAY_TUNNEL] This Virtual subnet ID is out of range or not assigned";   
        case CTC_E_OVERLAY_TUNNEL_INVALID_FID:  
          return "[OVERLAY_TUNNEL] This Virtual FID is out of range";   
        case CTC_E_OVERLAY_TUNNEL_VN_FID_NOT_EXIST:  
          return "[OVERLAY_TUNNEL] This Virtual subnet's FID has not assigned";   
        case CTC_E_OVERLAY_TUNNEL_NO_DA_INDEX_RESOURCE:  
          return "[OVERLAY_TUNNEL] The register to store IP DA index is full";   
        case CTC_E_DATAPATH_HSS_NOT_READY:  
          return "[DATAPATH] Hss can not ready ";   
        case CTC_E_DATAPATH_CLKTREE_CFG_ERROR:  
          return "[DATAPATH] Clktree cfg error ";   
        case CTC_E_DATAPATH_INVALID_PORT_TYPE:  
          return "[DATAPATH] invalid port type ";   
        case CTC_E_DATAPATH_INVALID_CORE_FRE:  
          return "[DATAPATH] invalid core frequency ";   
        case CTC_E_DATAPATH_PLL_NOT_LOCK:  
          return "[DATAPATH] pll cannot lock ";   
        case CTC_E_DATAPATH_PLL_EXIST_LANE:  
          return "[DATAPATH] pll cannot change ";   
        case CTC_E_DATAPATH_MAC_ENABLE:  
          return "[DATAPATH] mac is enable, cannot switch or 3ap training ";   
        case CTC_E_DATAPATH_SWITCH_FAIL:  
          return "[DATAPATH] dynamic switch fail ";   
        case CTC_E_ERROE_CODE_END:  
          return "End error code";   
    default:
      sal_sprintf(error_desc,"Error code:%d",error_code);
      return error_desc;
    }

}

int32
ctc_error_code_mapping(int32 error_code)
{
    if (g_mapping_disable || (error_code < CTC_E_ENTRY_EXIST) || (CTC_E_NONE == error_code))
    {
        return error_code;
    }

    if (error_code >= CTC_E_ERROE_CODE_END)
    {
        switch (error_code)
        {
            case -10000:
                return CTC_E_NO_MEMORY;
            case -9999:
                return CTC_E_HW_BUSY;
            case -9998:
                return CTC_E_HW_TIME_OUT;
            case -9997:
                return CTC_E_HW_INVALID_INDEX;
            default:
                return CTC_E_HW_FAIL;
        }
    }

    switch (error_code)
    {
        case CTC_E_HW_BUSY:
            return CTC_E_HW_BUSY;
        case CTC_E_HW_INVALID_INDEX:
            return CTC_E_HW_INVALID_INDEX;
        case CTC_E_INVALID_PTR:
        case CTC_E_VLAN_INVALID_VLAN_PTR:
            return CTC_E_INVALID_PTR;
        case CTC_E_INVALID_PORT:
        case CTC_E_INVALID_GLOBAL_PORT:
        case CTC_E_INVALID_LOGIC_PORT:
        case CTC_E_INVALID_LOCAL_PORT:
        case CTC_E_VPLS_INVALID_VPLSPORT_ID:
        case CTC_E_STK_TRUNK_MEMBER_PORT_INVALID:
        case CTC_E_VLAN_EXCEED_MAX_PHY_PORT:
        case CTC_E_FDB_NOT_LOCAL_MEMBER:
        case CTC_E_SCL_INVALID_DEFAULT_PORT:
            return CTC_E_INVALID_PORT;
        case CTC_E_INVALID_CHIP_ID:
        case CTC_E_INVALID_GLOBAL_CHIPID:
        case CTC_E_INVALID_LOCAL_CHIPID:
        case CTC_E_CHIP_IS_LOCAL:
        case CTC_E_CHIP_IS_REMOTE:
        case CTC_E_IPMC_EXCEED_GLOBAL_CHIP:
            return CTC_E_INVALID_CHIP_ID;
        case CTC_E_HW_TIME_OUT:
        case CTC_E_HASH_MEM_INIT_TIMEOUT:
        case CTC_E_CHIP_TIME_OUT:
            return CTC_E_HW_TIME_OUT;
        case CTC_E_HW_NOT_LOCK:
        case CTC_E_DATAPATH_PLL_NOT_LOCK:
            return CTC_E_HW_NOT_LOCK;
        case CTC_E_DMA:
        case CTC_E_DMA_TX_FAILED:
        case CTC_E_DMA_TABLE_WRITE_FAILED:
        case CTC_E_DMA_TABLE_READ_FAILED:
        case CTC_E_DMA_EXCEED_MAX_DESC_NUM:
        case CTC_E_DMA_DESC_NOT_DONE:
            return CTC_E_DMA;
        case CTC_E_HW_FAIL:
        case CTC_E_OAM_DRIVER_FAIL:
        case CTC_E_DATAPATH_HSS_NOT_READY:
        case CTC_E_SERDES_STATUS_NOT_READY:
        case CTC_E_SERDES_EYE_TEST_NOT_DONE:
        case CTC_E_CHIP_SCAN_PHY_REG_FULL:
        case CTC_E_CHIP_SWITCH_FAILED:
        case CTC_E_PORT_MAC_PCS_ERR:
        case CTC_E_DATAPATH_SWITCH_FAIL:
        case CTC_E_FDB_FIB_DUMP_FAIL:
        case CTC_E_HW_OP_FAIL:
        case CTC_E_CHECK_HSS_READY_FAIL:
        case CTC_E_PTP_TS_NOT_READY:
        case CTC_E_PTP_TX_TS_ROLL_OVER_FAILURE:
        case CTC_E_PTP_RX_TS_INFORMATION_NOT_READY:
        case CTC_E_TRAININT_FAIL:
            return CTC_E_HW_FAIL;
        case CTC_E_BADID:
        case CTC_E_STP_INVALID_STP_ID:
        case CTC_E_APS_INVALID_GROUP_ID:
        case CTC_E_OVERLAY_TUNNEL_INVALID_VN_ID:
        case CTC_E_OVERLAY_TUNNEL_INVALID_FID:
        case CTC_E_INVALID_DEVICE_ID:
        case CTC_E_VLAN_INVALID_VLAN_ID:
        case CTC_E_VLAN_INVALID_FID_ID:
        case CTC_E_VLAN_INVALID_VRFID:
        case CTC_E_VLAN_MAPPING_INVALID_SERVICE_ID:
        case CTC_E_FDB_INVALID_FID:
        case CTC_E_MIRROR_INVALID_MIRROR_LOG_ID:
        case CTC_E_L3IF_INVALID_IF_ID:
        case CTC_E_IPUC_INVALID_VRF:
        case CTC_E_IPUC_INVALID_L3IF:
        case CTC_E_IPMC_INVALID_VRF:
        case CTC_E_INVALID_TID:
        case CTC_E_SCL_GROUP_ID:
        case CTC_E_SCL_GROUP_ID_RSVED:
        case CTC_E_SCL_SERVICE_ID:
        case CTC_E_ACL_GROUP_ID:
        case CTC_E_ACL_GROUP_ID_RSVED:
        case CTC_E_ACL_SERVICE_ID:
           return CTC_E_BADID;
        case CTC_E_IN_USE:
        case CTC_E_NH_GLB_SRAM_IS_INUSE:
        case CTC_E_NH_GLB_SRAM_ISNOT_INUSE:
        case CTC_E_NH_EXIST_VC_LABEL:
        case CTC_E_NH_ECMP_IN_USE:
        case CTC_E_NH_HR_NH_IN_USE:
        case CTC_E_NH_CROSS_CHIP_RSPAN_NH_IN_USE:
        case CTC_E_NH_MCAST_MIRROR_NH_IN_USE:
        case CTC_E_QUEUE_SERVICE_REGISTERED:
        case CTC_E_QUEUE_INTERNAL_PORT_IN_USE:
        case CTC_E_OAM_MAID_ENTRY_REF_BY_MEP:
        case CTC_E_OAM_NH_OFFSET_IN_USE:
        case CTC_E_STATS_STATSID_ALREADY_IN_USE:
        case CTC_E_SCL_LABEL_IN_USE:
        case CTC_E_ACLQOS_LABEL_IN_USE:
        case CTC_E_ACL_LABEL_IN_USE:
        case CTC_E_QOS_POLICER_IN_USE:
           return CTC_E_IN_USE;
        case CTC_E_NOT_SUPPORT:
        case CTC_E_FEATURE_NOT_SUPPORT:
        case CTC_E_APS_DONT_SUPPORT_HW_BASED_APS_FOR_LINKAGG:
        case CTC_E_PORT_PVLAN_TYPE_NOT_SUPPORT:
        case CTC_E_SERDES_MODE_NOT_SUPPORT:
        case CTC_E_SERDES_PLL_NOT_SUPPORT:
        case CTC_E_MPLS_ENTRY_NOT_SUPPORT_STATS:
        case CTC_E_LINKAGG_DYNAMIC_BALANCE_NOT_SUPPORT:
        case CTC_E_SCL_UNSUPPORT:
        case CTC_E_INTR_NOT_SUPPORT:
        case CTC_E_SCL_STATS_NOT_SUPPORT:
        case CTC_E_SCL_HASH_ENTRY_UNSUPPORT_COPY:
        case CTC_E_QOS_THIS_PORT_NOT_SUPPORT_POLICER:
        case CTC_E_QOS_POLICER_NOT_SUPPORT_STATS:
        case CTC_E_ACL_STATS_NOT_SUPPORT:
        case CTC_E_ACL_HASH_ENTRY_UNSUPPORT_COPY:
           return CTC_E_NOT_SUPPORT;
        case CTC_E_NO_RESOURCE:
        case CTC_E_FDB_NO_RESOURCE:
        case CTC_E_FDB_SECURITY_VIOLATION:
        case CTC_E_NH_NHOFFSET_EXHAUST:
        case CTC_E_QUEUE_NOT_ENOUGH:
        case CTC_E_QUEUE_SERVICE_QUEUE_NO_HASH_ENTRY:
        case CTC_E_QUEUE_NO_FREE_INTERNAL_PORT:
        case CTC_E_OVERLAY_TUNNEL_NO_DA_INDEX_RESOURCE:
        case CTC_E_L3IF_ROUTER_MAC_EXHAUSTED:
        case CTC_E_IPMC_OFFSET_ALLOC_ERROR:
        case CTC_E_USRID_NO_KEY:
        case CTC_E_SCL_NO_KEY:
        case CTC_E_SCL_BUILD_AD_INDEX_FAILED:
        case CTC_E_SCL_AD_NO_MEMORY:
        case CTC_E_SCL_L4_PORT_RANGE_EXHAUSTED:
        case CTC_E_SCL_TCP_FLAG_EXHAUSTED:
        case CTC_E_ACL_L4_PORT_RANGE_EXHAUSTED:
        case CTC_E_ACL_TCP_FLAG_EXHAUSTED:
        case CTC_E_ACL_NO_ROOM_TO_MOVE_ENTRY:
        case CTC_E_VLAN_MAPPING_VRANGE_FULL:
           return CTC_E_NO_RESOURCE;
        case CTC_E_PROFILE_NO_RESOURCE:
           return CTC_E_PROFILE_NO_RESOURCE;
        case CTC_E_NO_MEMORY:
        case CTC_E_CANT_CREATE_AVL:
        case CTC_E_SPOOL_ADD_UPDATE_FAILED:
        case CTC_E_RPF_SPOOL_ADD_VECTOR_FAILED:
        case CTC_E_NO_ROOM_TO_MOVE_ENTRY:
        case CTC_E_FAIL_CREATE_MUTEX:
        case CTC_E_CREATE_MEM_CACHE_FAIL:
        case CTC_E_OAM_MEP_INDEX_VECTOR_ADD_FAIL:
        case CTC_E_SCL_HASH_INSERT_FAILED:
        case CTC_E_PTP_TS_ADD_BUFF_FAIL:
           return CTC_E_NO_MEMORY;
        case CTC_E_HASH_CONFLICT:
        case CTC_E_MPLS_NO_RESOURCE:
        case CTC_E_IPUC_HASH_CONFLICT:
        case CTC_E_IPMC_HASH_CONFLICT:
        case CTC_E_SCL_HASH_CONFLICT:
        case CTC_E_ACL_HASH_CONFLICT:
        case CTC_E_FDB_HASH_CONFLICT:
            return CTC_E_HASH_CONFLICT;
        case CTC_E_NOT_INIT:
        case CTC_E_NH_NOT_INIT:
        case CTC_E_OAM_NOT_INIT:
        case CTC_E_OAM_TRPT_NOT_INIT:
        case CTC_E_BPE_NOT_INIT:
        case CTC_E_STK_NOT_INIT:
        case CTC_E_IPMC_GROUP_IS_NOT_INIT:
        case CTC_E_PTP_NOT_INIT:
        case CTC_E_DMA_NOT_INIT:
        case CTC_E_SYNCE_NOT_INIT:
        case CTC_E_INTR_NOT_INIT:
           return CTC_E_NOT_INIT;
        case CTC_E_INIT_FAIL:
        case CTC_E_SCL_INIT:
        case CTC_E_ACL_INIT:
        case CTC_E_DRV_FAIL:
           return CTC_E_INIT_FAIL;
        case CTC_E_NOT_READY:
        case CTC_E_QUEUE_SERVICE_QUEUE_NOT_INITIALIZED:
        case CTC_E_QUEUE_SERVICE_QUEUE_INITIALIZED:
        case CTC_E_OAM_MEP_LM_NOT_EN:
        case CTC_E_OAM_TRPT_SESSION_EN:
        case CTC_E_OAM_TRPT_SESSION_ALREADY_EN:
        case CTC_E_OAM_TRPT_SESSION_NOT_CFG:
        case CTC_E_PORT_ACL_EN_FIRST:
        case CTC_E_VLAN_CLASS_NOT_ENABLE:
        case CTC_E_VLAN_MAPPING_NOT_ENABLE:
        case CTC_E_VLAN_ACL_EN_FIRST:
        case CTC_E_L3IF_NO_ALLOCED_IPUCSA:
        case CTC_E_STATS_PORT_NOT_ENABLE:
        case CTC_E_STATS_NOT_ENABLE:
        case CTC_E_STATS_DMA_ENABLE_BUT_SW_DISABLE:
        case CTC_E_IPUC_VERSION_DISABLE:
        case CTC_E_IPUC_TUNNEL_INVALID:
        case CTC_E_IPMC_VERSION_DISABLE:
        case CTC_E_IPMC_RPF_CHECK_DISABLE:
        case CTC_E_USRID_DISABLE:
        case CTC_E_USRID_ALREADY_ENABLE:
        case CTC_E_SCL_DISABLE:
        case CTC_E_SCL_ALREADY_ENABLE:
        case CTC_E_SCL_NEED_DEBUG:
        case CTC_E_ACL_GROUP_NOT_EMPTY:
        case CTC_E_SCL_GROUP_NOT_EMPTY:
        case CTC_E_SCL_HAVE_ENABLED:
        case CTC_E_SCL_NOT_ENABLED:
        case CTC_E_ACLQOS_HAVE_ENABLED:
        case CTC_E_ACLQOS_NOT_ENABLED:
        case CTC_E_ACL_HAVE_ENABLED:
        case CTC_E_ACL_NOT_ENABLED:
        case CTC_E_QOS_POLICER_SERVICE_POLICER_NOT_ENABLE:
        case CTC_E_QOS_POLICER_STATS_NOT_ENABLE:
        case CTC_E_QOS_QUEUE_STATS_NOT_ENABLE:
        case CTC_E_SCL_ENTRY_INSTALLED:
        case CTC_E_SCL_GROUP_NOT_INSTALLED:
           return CTC_E_NOT_READY;
        case CTC_E_EXIST:
        case CTC_E_ENTRY_EXIST:
        case CTC_E_MEMBER_EXIST:
        case CTC_E_NH_EXIST:
        case CTC_E_NH_STATS_EXIST:
        case CTC_E_QUEUE_CHANNEL_SHAPE_PROF_EXIST:
        case CTC_E_QUEUE_SERVICE_EXIST:
        case CTC_E_APS_GROUP_EXIST:
        case CTC_E_APS_GROUP_NEXT_APS_EXIST:
        case CTC_E_OAM_MAID_ENTRY_EXIST:
        case CTC_E_OAM_CHAN_ENTRY_EXIST:
        case CTC_E_OAM_CHAN_LMEP_EXIST:
        case CTC_E_OAM_RMEP_EXIST:
        case CTC_E_OAM_NH_OFFSET_EXIST:
        case CTC_E_OAM_INDEX_EXIST:
        case CTC_E_MEMBER_PORT_EXIST:
        case CTC_E_ALREADY_PHY_IF_EXIST:
        case CTC_E_ALREADY_SUB_IF_EXIST:
        case CTC_E_MPLS_ENTRY_STATS_EXIST:
        case CTC_E_STK_TRUNK_HAS_EXIST:
        case CTC_E_STK_TRUNK_MEMBER_PORT_EXIST:
        case CTC_E_DATAPATH_PLL_EXIST_LANE:
        case CTC_E_VLAN_EXIST:
        case CTC_E_VLAN_RANGE_EXIST:
        case CTC_E_FDB_MAC_FILTERING_ENTRY_EXIST:
        case CTC_E_FDB_MCAST_ENTRY_EXIST:
        case CTC_E_FDB_KEY_ALREADY_EXIST:
        case CTC_E_L3IF_EXIST:
        case CTC_E_STATS_STATSID_EXIST:
        case CTC_E_IPMC_GROUP_HAS_EXISTED:
        case CTC_E_LINKAGG_HAS_EXIST:
        case CTC_E_SCL_GROUP_EXIST:
        case CTC_E_ACLQOS_ENTRY_EXIST:
        case CTC_E_ACL_ENTRY_EXIST:
        case CTC_E_ACL_GROUP_EXIST:
        case CTC_E_QOS_POLICER_CREATED:
           return CTC_E_EXIST;
        case CTC_E_NOT_EXIST:
        case CTC_E_QUEUE_CPU_REASON_NOT_CREATE:
        case CTC_E_ENTRY_NOT_EXIST:
        case CTC_E_MEMBER_NOT_EXIST:
        case CTC_E_NH_NOT_EXIST:
        case CTC_E_NH_STATS_NOT_EXIST:
        case CTC_E_NH_NOT_EXIST_TUNNEL_LABEL:
        case CTC_E_NH_ECMP_MEM_NOS_EXIST:
        case CTC_E_QUEUE_DROP_PROF_NOT_EXIST:
        case CTC_E_QUEUE_SHAPE_PROF_NOT_EXIST:
        case CTC_E_QUEUE_CHANNEL_SHAPE_PROF_NOT_EXIST:
        case CTC_E_QUEUE_SERVICE_NOT_EXIST:
        case CTC_E_QUEUE_SERVICE_GROUP_NOT_EXIST:
        case CTC_E_APS_GROUP_NOT_EXIST:
        case CTC_E_APS_RAPS_GROUP_NOT_EXIST:
        case CTC_E_OAM_MAID_ENTRY_NOT_FOUND:
        case CTC_E_OAM_CHAN_ENTRY_NOT_FOUND:
        case CTC_E_OAM_CHAN_MD_LEVEL_NOT_EXIST:
        case CTC_E_OAM_CHAN_LMEP_NOT_FOUND:
        case CTC_E_OAM_RMEP_NOT_FOUND:
        case CTC_E_OAM_NH_OFFSET_NOT_FOUND:
        case CTC_E_OVERLAY_TUNNEL_VN_FID_NOT_EXIST:
        case CTC_E_MEMBER_PORT_NOT_EXIST:
        case CTC_E_LOCAL_PORT_NOT_EXIST:
        case CTC_E_VPLS_VSI_NOT_CREATE:
        case CTC_E_MPLS_ENTRY_STATS_NOT_EXIST:
        case CTC_E_STK_TRUNK_NOT_EXIST:
        case CTC_E_STK_TRUNK_MEMBER_PORT_NOT_EXIST:
        case CTC_E_VLAN_NOT_CREATE:
        case CTC_E_VLAN_RANGE_NOT_EXIST:
        case CTC_E_FDB_DEFAULT_ENTRY_NOT_EXIST:
        case CTC_E_FDB_AD_INDEX_NOT_EXIST:
        case CTC_E_L3IF_VMAC_NOT_EXIST:
        case CTC_E_L3IF_NOT_EXIST:
        case CTC_E_L3IF_VRF_STATS_NOT_EXIST:
        case CTC_E_IPMC_GROUP_NOT_EXIST:
        case CTC_E_IPMC_STATS_NOT_EXIST:
        case CTC_E_LINKAGG_NOT_EXIST:
        case CTC_E_SCL_GROUP_UNEXIST:
        case CTC_E_SCL_LABEL_NOT_EXIST:
        case CTC_E_ACLQOS_ENTRY_NOT_EXIST:
        case CTC_E_ACLQOS_LABEL_NOT_EXIST:
        case CTC_E_QOS_POLICER_NOT_EXIST:
        case CTC_E_ACL_ENTRY_UNEXIST:
        case CTC_E_ACL_GROUP_UNEXIST:
        case CTC_E_ACL_LABEL_NOT_EXIST:
           return CTC_E_NOT_EXIST;
        case CTC_E_INVALID_CONFIG:
        case CTC_E_GLOBAL_CONFIG_CONFLICT:
        case CTC_E_MAC_NOT_USED:
        case CTC_E_NH_INVALID_DSEDIT_PTR:
        case CTC_E_NH_INVALID_NH_TYPE:
        case CTC_E_NH_INVALID_NH_SUB_TYPE:
        case CTC_E_NH_INVALID_VLAN_EDIT_TYPE:
        case CTC_E_NH_INVALID_DSNH_TYPE:
        case CTC_E_NH_VLAN_EDIT_CONFLICT:
        case CTC_E_NH_SHOULD_USE_DSNH8W:
        case CTC_E_NH_SHOULD_USE_DSNH4W:
        case CTC_E_NH_NHID_NOT_MATCH_NHTYPE:
        case CTC_E_NH_NO_LABEL:
        case CTC_E_NH_ISNT_UNROV:
        case CTC_E_NH_IS_UNROV:
        case CTC_E_NH_MEMBER_IS_UPMEP:
        case CTC_E_NH_EGS_EDIT_CONFLICT_VLAN_AND_MACDA_EDIT:
        case CTC_E_NH_LABEL_IS_MISMATCH_WITH_STATS:
        case CTC_E_NH_SUPPORT_PW_APS_UPDATE_BY_INT_ALLOC_DSNH:
        case CTC_E_QUEUE_DEPTH_NOT_EMPTY:
        case CTC_E_QUEUE_INVALID_CONFIG:
        case CTC_E_APS_INTERFACE_ERROR:
        case CTC_E_APS_MPLS_TYPE_L3IF_NOT_EXIST:
        case CTC_E_APS_DONT_SUPPORT_2_LEVEL_HW_BASED_APS:
        case CTC_E_OAM_CHAN_MD_LEVEL_EXIST:
        case CTC_E_OAM_CHAN_ENTRY_NUM_EXCEED:
        case CTC_E_OAM_LM_NUM_RXCEED:
        case CTC_E_OAM_CHAN_NOT_UP_DIRECTION:
        case CTC_E_OAM_CHAN_NOT_DOWN_DIRECTION:
        case CTC_E_OAM_TX_GPORT_AND_MASTER_GCHIP_NOT_MATCH:
        case CTC_E_OAM_TX_GPORT_AND_CHAN_GPORT_NOT_MATCH:
        case CTC_E_OAM_INVALID_CFG_IN_MEP_TYPE:
        case CTC_E_OAM_INVALID_CFG_LM:
        case CTC_E_OAM_NH_OFFSET_ADD_VECTOR_FAIL:
        case CTC_E_OAM_RMEP_D_LOC_PRESENT:
        case CTC_E_OAM_ITU_DEFECT_CLEAR_MODE:
        case CTC_E_OAM_INVALID_OPERATION_ON_CPU_MEP:
        case CTC_E_OAM_INVALID_OPERATION_ON_P2P_MEP:
        case CTC_E_OAM_INVALID_GLOBAL_PARAM_TYPE:
        case CTC_E_OAM_EXCEED_MAX_LEVEL_NUM_PER_CHAN:
        case CTC_E_OAM_TRPT_INTERVAL_CONFLICT:
        case CTC_E_BPE_INVALID_BPE_CAPABLE_GPORT:
        case CTC_E_BPE_INVLAID_BPE_MODE:
        case CTC_E_PORT_HAS_OTHER_FEATURE:
        case CTC_E_PORT_FEATURE_MISMATCH:
        case CTC_E_PORT_HAS_OTHER_RESTRICTION:
        case CTC_E_CHIP_DATAPATH_NOT_MATCH:
        case CTC_E_PORT_PROP_COLLISION:
        case CTC_E_PORT_MAC_IS_DISABLE:
        case CTC_E_MPLS_LABEL_ERROR:
        case CTC_E_STK_TRUNK_EXCEED_MAX_MEMBER_PORT:
        case CTC_E_STK_KEEPLIVE_TWO_MEMBER_AND_ONE_CPU:
        case CTC_E_DATAPATH_INVALID_PORT_TYPE:
        case CTC_E_DATAPATH_MAC_ENABLE:
        case CTC_E_INVALID_PORT_MAC_TYPE:
        case CTC_E_CANT_CHANGE_SERDES_MODE:
        case CTC_E_VLAN_RANGE_END_GREATER_START:
        case CTC_E_VLAN_MAPPING_GET_VLAN_RANGE_FAILED:
        case CTC_E_VLAN_MAPPING_VRANGE_OVERLAPPED:
        case CTC_E_VLAN_MAPPING_TAG_APPEND:
        case CTC_E_VLAN_MAPPING_TAG_SWAP:
        case CTC_E_FDB_L2MCAST_ADD_MEMBER_FAILED:
        case CTC_E_FDB_OPERATION_PAUSE:
        case CTC_E_FDB_HASH_REMOVE_FAILED:
        case CTC_E_FDB_DEFAULT_ENTRY_NOT_ALLOWED:
        case CTC_E_FDB_ONLY_SW_LEARN:
        case CTC_E_FDB_ONLY_HW_LEARN:
        case CTC_E_FDB_LOGIC_USE_HW_LEARN_DISABLE:
        case CTC_E_VLAN_FILTER_PORT:
        case CTC_E_FDB_L2MCAST_MEMBER_INVALID:
        case CTC_E_L3IF_MISMATCH:
        case CTC_E_STATS_PORT_STATS_NO_TYPE:
        case CTC_E_STATS_PHB_STATS_INVALID:
        case CTC_E_STATS_PORT_NOT_MAP_TO_MAC:
        case CTC_E_STATS_MAC_STATS_MODE_ASIC:
        case CTC_E_STATS_TABLE_NUM_NO_EXPECT:
        case CTC_E_STATS_VLAN_STATS_CONFLICT_WITH_PHB:
        case CTC_E_STATS_STATSID_TYPE_MISMATCH:
        case CTC_E_IPUC_NEED_L3IF:
        case CTC_E_IPUC_RPF_NOT_SUPPORT_THIS_MASK_LEN:
        case CTC_E_IPUC_NAT_NOT_SUPPORT_THIS_MASK_LEN:
        case CTC_E_IPUC_TUNNEL_RPF_INFO_MISMATCH:
        case CTC_E_IPUC_TUNNEL_PAYLOAD_TYPE_MISMATCH:
        case CTC_E_IPUC_INVALID_ROUTE_FLAG:
        case CTC_E_IPMC_ADD_MEMBER_FAILED:
        case CTC_E_IPMC_GROUP_IS_NOT_IP_MAC:
        case CTC_E_IPMC_GROUP_IS_IP_MAC:
        case CTC_E_SCL_HASH_CONVERT_FAILED:
        case CTC_E_SCL_GET_KEY_FAILED:
        case CTC_E_SCL_KEY_AD_EXIST:
        case CTC_E_SCL_BUILD_VLAN_ACTION_INDEX_FAILED:
        case CTC_E_SCL_CANNOT_OVERWRITE:
        case CTC_E_SCL_LKP_FAILED:
        case CTC_E_SCL_STATIC_ENTRY:
        case CTC_E_SCL_UPDATE_FAILED:
        case CTC_E_SCL_FLAG_COLLISION:
        case CTC_E_ACL_FLAG_COLLISION:
        case CTC_E_SCL_GET_NODES_FAILED:
        case CTC_E_SCL_GROUP_INFO:
        case CTC_E_SCL_ADD_TCAM_ENTRY_TO_WRONG_GROUP:
        case CTC_E_SCL_ADD_HASH_ENTRY_TO_WRONG_GROUP:
        case CTC_E_SCL_HASH_ENTRY_NO_PRIORITY:
        case CTC_E_SCL_KEY_ACTION_TYPE_NOT_MATCH:
        case CTC_E_QOS_POLICER_CIR_GREATER_THAN_PIR:
        case CTC_E_QOS_POLICER_CBS_GREATER_THAN_PBS:
        case CTC_E_ACLQOS_DIFFERENT_DIR:
        case CTC_E_ACLQOS_DIFFERENT_TYPE:
        case CTC_E_ACLQOS_DIFFERENT_CHIP:
        case CTC_E_PTP_TS_GET_BUFF_FAIL:
        case CTC_E_PTP_CAN_NOT_SET_TOW:
        case CTC_E_SYNCE_INVALID_RECOVERED_PORT:
        case CTC_E_QOS_NO_INT_POLICER_ENTRY:
        case CTC_E_QOS_NO_EXT_POLICER_ENTRY:
        case CTC_E_QOS_NO_INT_POLICER_PROFILE_ENTRY:
        case CTC_E_ACL_PBR_ENTRY_NO_NXTTHOP:
        case CTC_E_ACL_GET_NODES_FAILED:
        case CTC_E_SCL_GLOBAL_CFG_ERROR:
        case CTC_E_QOS_POLICER_NOT_BIND:
        case CTC_E_ACL_GROUP_INFO:
        case CTC_E_ACL_ENTRY_INSTALLED:
        case CTC_E_ACL_GET_STATS_FAILED:
        case CTC_E_ACL_GROUP_NOT_INSTALLED:
        case CTC_E_ACL_GLOBAL_CFG_ERROR:
        case CTC_E_ACL_ADD_TCAM_ENTRY_TO_WRONG_GROUP:
        case CTC_E_ACL_ADD_HASH_ENTRY_TO_WRONG_GROUP:
        case CTC_E_ACL_HASH_ENTRY_NO_PRIORITY:
        case CTC_E_IPUC_ADD_FAILED:
        case CTC_E_SCL_GET_BLOCK_INDEX_FAILED:
        case CTC_E_SCL_GET_STATS_FAILED:
        case CTC_E_ACL_PBR_ECMP_CMP_FAILED:
        case CTC_E_ACL_PBR_DO_ECMP_FAILED:
        case CTC_E_MUTEX_BUSY:
           return CTC_E_INVALID_CONFIG;
        default:
            return CTC_E_INVALID_PARAM;
    }

}

int32
ctc_set_error_code_mapping_en(uint8 enable)
{
    g_mapping_disable = !enable;
    return CTC_E_NONE;
}
