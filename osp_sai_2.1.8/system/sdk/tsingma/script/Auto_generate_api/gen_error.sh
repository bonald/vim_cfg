echo "************************************************"
echo "auto generate error code "
echo "************************************************"
destfile="../../core/common/src/ctc_error.c"
srcfile="../../core/common/include/ctc_error.h"
echo  '/**'                                                                                                        >$destfile
echo  ' @file ctc_error.c'                                                                                    >>$destfile
echo  ''                                                                                                           >>$destfile
echo  ' @date 2012-08-15'                                                                                          >>$destfile
echo  ''                                                                                                           >>$destfile
echo  ' @version v2.0'                                                                                             >>$destfile
echo  ''                                                                                                           >>$destfile
echo  ' return sdk error description'                                                                              >>$destfile
echo  ' auto generated based on head file'                                                                         >>$destfile
echo  '*/'                                                                                                         >>$destfile
echo  '#include "sal.h"'                                                                                     >>$destfile
echo  '#include "ctc_error.h"'                                                                                     >>$destfile
echo  ''                                                                                                           >>$destfile
echo  'uint32 g_mapping_disable = 0;'                                                                              >>$destfile

if [ "1" == "2" ]
then
echo 'enum ctc_err_old_temp_e'                           >>$destfile
echo '{'                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_ENTRY_EXIST = -30000,'                           >>$destfile
echo '    CTC_E_ENTRY_NOT_EXIST, '                           >>$destfile
echo '    CTC_E_EXCEED_MAX_SIZE,'                           >>$destfile
echo '    CTC_E_EXCEED_MIN_SIZE,'                           >>$destfile
echo '    CTC_E_INVALID_DIR,'                           >>$destfile
echo '    CTC_E_CANT_CREATE_AVL,'                           >>$destfile
echo '    CTC_E_UNEXPECT,'                           >>$destfile
echo '    CTC_E_LESS_ZERO,'                           >>$destfile
echo '    CTC_E_MEMBER_EXIST,'                           >>$destfile
echo '    CTC_E_MEMBER_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_INVALID_EXP_VALUE,'                           >>$destfile
echo '    CTC_E_INVALID_MPLS_LABEL_VALUE,'                           >>$destfile
echo '    CTC_E_INVALID_REG,'                           >>$destfile
echo '    CTC_E_INVALID_TBL,'                           >>$destfile
echo '    CTC_E_GLOBAL_CONFIG_CONFLICT,'                           >>$destfile
echo '    CTC_E_FEATURE_NOT_SUPPORT,'                           >>$destfile
echo '    CTC_E_INVALID_TTL,'                           >>$destfile
echo '    CTC_E_INVALID_MTU_SIZE,'                           >>$destfile
echo '    CTC_E_SPOOL_ADD_UPDATE_FAILED,'                           >>$destfile
echo '    CTC_E_SPOOL_REMOVE_FAILED,'                           >>$destfile
echo '    CTC_E_RPF_SPOOL_ADD_VECTOR_FAILED,'                           >>$destfile
echo '    CTC_E_RPF_SPOOL_REMOVE_VECTOR_FAILED,'                           >>$destfile
echo '    CTC_E_NO_ROOM_TO_MOVE_ENTRY,'                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_STP_INVALID_STP_ID,'                           >>$destfile
echo '    CTC_E_EXCEED_STP_MAX_LPORT,'                           >>$destfile
echo '    CTC_E_INVALID_COS,'                           >>$destfile
echo '    CTC_E_INVALID_FRAMESIZE_INDEX,'                           >>$destfile
echo '    CTC_E_INVALID_MIN_FRAMESIZE,'                           >>$destfile
echo '    CTC_E_INVALID_MAX_FRAMESIZE,'                           >>$destfile
echo '    CTC_E_MAC_NOT_USED,'                           >>$destfile
echo '    CTC_E_TRAININT_FAIL,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_FAIL_CREATE_MUTEX = -29900,'                           >>$destfile
echo '    CTC_E_MUTEX_BUSY,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_INVALID_CHIP_NUM = -29800,'                           >>$destfile
echo '    CTC_E_INVALID_GLOBAL_CHIPID,'                           >>$destfile
echo '    CTC_E_INVALID_LOCAL_CHIPID,'                           >>$destfile
echo '    CTC_E_CHIP_IS_LOCAL,'                           >>$destfile
echo '    CTC_E_CHIP_IS_REMOTE,'                           >>$destfile
echo '    CTC_E_INVALID_GLOBAL_PORT,'                           >>$destfile
echo '    CTC_E_INVALID_LOGIC_PORT,'                           >>$destfile
echo '    CTC_E_MEMBER_PORT_EXIST,'                           >>$destfile
echo '    CTC_E_MEMBER_PORT_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_LOCAL_PORT_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_ALREADY_PHY_IF_EXIST,'                           >>$destfile
echo '    CTC_E_ALREADY_SUB_IF_EXIST,'                           >>$destfile
echo '    CTC_E_PORT_HAS_OTHER_FEATURE,'                           >>$destfile
echo '    CTC_E_PORT_FEATURE_MISMATCH,'                           >>$destfile
echo '    CTC_E_PORT_PVLAN_TYPE_NOT_SUPPORT,'                           >>$destfile
echo '    CTC_E_PORT_HAS_OTHER_RESTRICTION,'                           >>$destfile
echo '    CTC_E_INVALID_LOCAL_PORT,'                           >>$destfile
echo '    CTC_E_INVALID_PORT_MAC_TYPE,'                           >>$destfile
echo '    CTC_E_INVALID_PORT_SPEED_MODE,'                           >>$destfile
echo '    CTC_E_INVALID_PREAMBLE,'                           >>$destfile
echo '    CTC_E_INVALID_TX_THRESHOLD,'                           >>$destfile
echo '    CTC_E_CANT_CHANGE_SERDES_MODE,'                           >>$destfile
echo '    CTC_E_SERDES_STATUS_NOT_READY,'                           >>$destfile
echo '    CTC_E_SERDES_MODE_NOT_SUPPORT,'                           >>$destfile
echo '    CTC_E_SERDES_PLL_NOT_SUPPORT,'                           >>$destfile
echo '    CTC_E_SERDES_EYE_TEST_NOT_DONE,'                           >>$destfile
echo '    CTC_E_CHIP_TIME_OUT,'                           >>$destfile
echo '    CTC_E_CHIP_SCAN_PHY_REG_FULL,'                           >>$destfile
echo '    CTC_E_CHIP_DATAPATH_NOT_MATCH,'                           >>$destfile
echo '    CTC_E_PORT_INVALID_PORT_MAC,'                           >>$destfile
echo '    CTC_E_PORT_INVALID_ACL_LABEL,'                           >>$destfile
echo '    CTC_E_PORT_PROP_COLLISION,'                           >>$destfile
echo '    CTC_E_PORT_ACL_EN_FIRST,'                           >>$destfile
echo '    CTC_E_CHIP_SWITCH_FAILED,'                           >>$destfile
echo '    CTC_E_PORT_MAC_IS_DISABLE,'                           >>$destfile
echo '    CTC_E_PORT_MAC_PCS_ERR,'                           >>$destfile
echo '    CTC_E_INVALID_DEVICE_ID,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_VLAN_EXCEED_MAX_VLANCTL = -29700,'                           >>$destfile
echo '    CTC_E_VLAN_EXCEED_MAX_PHY_PORT,'                           >>$destfile
echo '    CTC_E_VLAN_INVALID_VLAN_ID,'                           >>$destfile
echo '    CTC_E_VLAN_INVALID_VLAN_PTR,'                           >>$destfile
echo '    CTC_E_VLAN_ERROR_MODE,'                           >>$destfile
echo '    CTC_E_VLAN_INVALID_VLAN_ID_OR_VLAN_PTR,'                           >>$destfile
echo '    CTC_E_VLAN_INVALID_FID_ID,'                           >>$destfile
echo '    CTC_E_VLAN_INVALID_VRFID,'                           >>$destfile
echo '    CTC_E_VLAN_INVALID_PROFILE_ID,'                           >>$destfile
echo '    CTC_E_VLAN_EXIST,'                           >>$destfile
echo '    CTC_E_VLAN_NOT_CREATE,'                           >>$destfile
echo '    CTC_E_VLAN_FILTER_PORT,'                           >>$destfile
echo '    CTC_E_VLAN_CLASS_INVALID_TYPE,'                           >>$destfile
echo '    CTC_E_VLAN_CLASS_NOT_ENABLE,'                           >>$destfile
echo '    CTC_E_VLAN_MAPPING_NOT_ENABLE,'                           >>$destfile
echo '    CTC_E_VLAN_MAPPING_INVALID_SERVICE_ID,'                           >>$destfile
echo '    CTC_E_VLAN_MAPPING_RANGE_TYPE_NOT_MATCH,'                           >>$destfile
echo '    CTC_E_VLAN_MAPPING_RANGE_GROUP_NOT_MATCH,'                           >>$destfile
echo '    CTC_E_VLAN_MAPPING_GET_VLAN_RANGE_FAILED,'                           >>$destfile
echo '    CTC_E_VLAN_MAPPING_VRANGE_FULL,'                           >>$destfile
echo '    CTC_E_VLAN_MAPPING_VRANGE_OVERLAPPED,'                           >>$destfile
echo '    CTC_E_VLAN_MAPPING_TAG_APPEND,'                           >>$destfile
echo '    CTC_E_VLAN_MAPPING_TAG_SWAP,'                           >>$destfile
echo '    CTC_E_VLAN_MAPPING_TAG_OP_NOT_SUPPORT,'                           >>$destfile
echo '    CTC_E_VLAN_RANGE_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_VLAN_RANGE_EXIST,'                           >>$destfile
echo '    CTC_E_VLAN_RANGE_END_GREATER_START,'                           >>$destfile
echo '    CTC_E_VLAN_CLASS_PROTOCOL_VLAN_FULL,'                           >>$destfile
echo '    CTC_E_VLAN_ACL_EN_FIRST,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_INVALID_TID = -29600,'                           >>$destfile
echo '    CTC_E_LINKAGG_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_LINKAGG_HAS_EXIST,'                           >>$destfile
echo '    CTC_E_LINKAGG_DYNAMIC_BALANCE_NOT_SUPPORT,'                           >>$destfile
echo '    CTC_E_LINKAGG_INVALID_LINKAGG_MODE,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_CREATE_MEM_CACHE_FAIL = -29500,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_USRID_INVALID_KEY = -29400,'                           >>$destfile
echo '    CTC_E_USRID_INVALID_LABEL,'                           >>$destfile
echo '    CTC_E_USRID_INVALID_TYPE,'                           >>$destfile
echo '    CTC_E_USRID_DISABLE,'                           >>$destfile
echo '    CTC_E_USRID_ALREADY_ENABLE,'                           >>$destfile
echo '    CTC_E_USRID_NO_KEY,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_SCL_INVALID_KEY = -29380,'                           >>$destfile
echo '    CTC_E_SCL_INVALID_LABEL,'                           >>$destfile
echo '    CTC_E_SCL_INVALID_TYPE,'                           >>$destfile
echo '    CTC_E_SCL_DISABLE,'                           >>$destfile
echo '    CTC_E_SCL_ALREADY_ENABLE,'                           >>$destfile
echo '    CTC_E_SCL_NO_KEY,'                           >>$destfile
echo '    CTC_E_SCL_INVALID_DEFAULT_PORT,'                           >>$destfile
echo '    CTC_E_SCL_HASH_CONFLICT,'                           >>$destfile
echo '    CTC_E_SCL_HASH_INSERT_FAILED,'                           >>$destfile
echo '    CTC_E_SCL_HASH_CONVERT_FAILED,'                           >>$destfile
echo '    CTC_E_SCL_BUILD_AD_INDEX_FAILED,'                           >>$destfile
echo '    CTC_E_SCL_NEED_DEBUG,'                           >>$destfile
echo '    CTC_E_SCL_GET_KEY_FAILED,'                           >>$destfile
echo '    CTC_E_SCL_KEY_AD_EXIST,'                           >>$destfile
echo '    CTC_E_SCL_AD_NO_MEMORY,'                           >>$destfile
echo '    CTC_E_SCL_BUILD_VLAN_ACTION_INDEX_FAILED,'                           >>$destfile
echo '    CTC_E_SCL_UNSUPPORT,'                           >>$destfile
echo '    CTC_E_SCL_CANNOT_OVERWRITE,'                           >>$destfile
echo '    CTC_E_SCL_LKP_FAILED,'                           >>$destfile
echo '    CTC_E_SCL_VLAN_ACTION_INVALID,'                           >>$destfile
echo '    CTC_E_SCL_STATIC_ENTRY,'                           >>$destfile
echo '    CTC_E_SCL_UPDATE_FAILED,'                           >>$destfile
echo '    CTC_E_SCL_INIT,'                           >>$destfile
echo '    CTC_E_SCL_GROUP_PRIORITY,'                           >>$destfile
echo '    CTC_E_SCL_GROUP_TYPE,'                           >>$destfile
echo '    CTC_E_SCL_GROUP_ID,'                           >>$destfile
echo '    CTC_E_SCL_GROUP_ID_RSVED,'                           >>$destfile
echo '    CTC_E_SCL_GROUP_UNEXIST,'                           >>$destfile
echo '    CTC_E_SCL_GROUP_EXIST,'                           >>$destfile
echo '    CTC_E_SCL_GROUP_NOT_EMPTY,'                           >>$destfile
echo '    CTC_E_SCL_INVALID_ACTION,'                           >>$destfile
echo '    CTC_E_SCL_INVALID_KEY_TYPE,'                           >>$destfile
echo '    CTC_E_SCL_INVALID_LABEL_TYPE,'                           >>$destfile
echo '    CTC_E_SCL_LABEL_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_SCL_HAVE_ENABLED,'                           >>$destfile
echo '    CTC_E_SCL_NOT_ENABLED,'                           >>$destfile
echo '    CTC_E_SCL_LABEL_IN_USE,'                           >>$destfile
echo '    CTC_E_SCL_FLAG_COLLISION,'                           >>$destfile
echo '    CTC_E_SCL_L4_PORT_RANGE_EXHAUSTED,'                           >>$destfile
echo '    CTC_E_SCL_TCP_FLAG_EXHAUSTED,'                           >>$destfile
echo '    CTC_E_SCL_GET_NODES_FAILED ,'                           >>$destfile
echo '    CTC_E_SCL_PORT_BITMAP_OVERFLOW,'                           >>$destfile
echo '    CTC_E_SCL_GROUP_INFO,'                           >>$destfile
echo '    CTC_E_SCL_GET_BLOCK_INDEX_FAILED,'                           >>$destfile
echo '    CTC_E_SCL_ENTRY_INSTALLED,'                           >>$destfile
echo '    CTC_E_SCL_GET_STATS_FAILED,'                           >>$destfile
echo '    CTC_E_SCL_GROUP_NOT_INSTALLED,'                           >>$destfile
echo '    CTC_E_SCL_SERVICE_ID,'                           >>$destfile
echo '    CTC_E_SCL_GLOBAL_CFG_ERROR,'                           >>$destfile
echo '    CTC_E_SCL_STATS_NOT_SUPPORT,'                           >>$destfile
echo '    CTC_E_SCL_ADD_TCAM_ENTRY_TO_WRONG_GROUP,'                           >>$destfile
echo '    CTC_E_SCL_ADD_HASH_ENTRY_TO_WRONG_GROUP,'                           >>$destfile
echo '    CTC_E_SCL_HASH_ENTRY_NO_PRIORITY,'                           >>$destfile
echo '    CTC_E_SCL_HASH_ENTRY_UNSUPPORT_COPY,'                           >>$destfile
echo '    CTC_E_SCL_VLAN_EDIT_INVALID,'                           >>$destfile
echo '    CTC_E_SCL_KEY_ACTION_TYPE_NOT_MATCH,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_FDB_WRONG_MAC_ADDR = -29300,'                           >>$destfile
echo '    CTC_E_FDB_MAC_FILTERING_ENTRY_EXIST,'                           >>$destfile
echo '    CTC_E_FDB_ENTRY_FULL,'                           >>$destfile
echo '    CTC_E_FDB_L2MCAST_MEMBER_INVALID,'                           >>$destfile
echo '    CTC_E_FDB_L2MCAST_ADD_MEMBER_FAILED,'                           >>$destfile
echo '    CTC_E_FDB_MCAST_ENTRY_EXIST,'                           >>$destfile
echo '    CTC_E_FDB_DEFAULT_ENTRY_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_FDB_OPERATION_PAUSE,'                           >>$destfile
echo '    CTC_E_FDB_HASH_INSERT_FAILED,'                           >>$destfile
echo '    CTC_E_FDB_HASH_REMOVE_FAILED,'                           >>$destfile
echo '    CTC_E_FDB_AD_INDEX_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_FDB_DEFAULT_ENTRY_NOT_ALLOWED,'                           >>$destfile
echo '    CTC_E_FDB_INVALID_FDB_TYPE,'                           >>$destfile
echo '    CTC_E_FDB_NOT_LOCAL_MEMBER,'                           >>$destfile
echo '    CTC_E_FDB_KEY_ALREADY_EXIST,'                           >>$destfile
echo '    CTC_E_FDB_HASH_CONFLICT,'                           >>$destfile
echo '    CTC_E_FDB_SOURCE_PORT_TYPE_NOT_MATCH,'                           >>$destfile
echo '    CTC_E_FDB_NO_RESOURCE,'                           >>$destfile
echo '    CTC_E_FDB_INVALID_FID,'                           >>$destfile
echo '    CTC_E_FDB_NO_SW_TABLE_NO_QUARY,'                           >>$destfile
echo '    CTC_E_FDB_ONLY_SW_LEARN,'                           >>$destfile
echo '    CTC_E_FDB_LOGIC_USE_HW_LEARN_DISABLE,'                           >>$destfile
echo '    CTC_E_FDB_NO_SW_TABLE_NO_FLUSH_BY_SW,'                           >>$destfile
echo '    CTC_E_FDB_ONLY_HW_LEARN,'                           >>$destfile
echo '    CTC_E_FDB_FIB_DUMP_FAIL,'                           >>$destfile
echo '    CTC_E_FDB_SECURITY_VIOLATION,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_NH_NOT_INIT = -29200,'                           >>$destfile
echo '    CTC_E_NH_INVALID_NHID,'                           >>$destfile
echo '    CTC_E_NH_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_NH_EXIST,'                           >>$destfile
echo '    CTC_E_NH_INVALID_DSEDIT_PTR,'                           >>$destfile
echo '    CTC_E_NH_INVALID_NH_TYPE,'                           >>$destfile
echo '    CTC_E_NH_INVALID_NH_SUB_TYPE,'                           >>$destfile
echo '    CTC_E_NH_INVALID_VLAN_EDIT_TYPE,'                           >>$destfile
echo '    CTC_E_NH_INVALID_DSNH_TYPE,'                           >>$destfile
echo '    CTC_E_NH_VLAN_EDIT_CONFLICT,'                           >>$destfile
echo '    CTC_E_NH_STATS_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_NH_STATS_EXIST,'                           >>$destfile
echo '    CTC_E_NH_INVALID_MARTINI_SEQ_TYPE = -29150,'                           >>$destfile
echo '    CTC_E_NH_SHOULD_USE_DSNH8W,'                           >>$destfile
echo '    CTC_E_NH_SHOULD_USE_DSNH4W,'                           >>$destfile
echo '    CTC_E_NH_NHID_NOT_MATCH_NHTYPE,'                           >>$destfile
echo '    CTC_E_NH_GLB_SRAM_IS_INUSE,'                           >>$destfile
echo '    CTC_E_NH_GLB_SRAM_ISNOT_INUSE,'                           >>$destfile
echo '    CTC_E_NH_GLB_SRAM_INDEX_EXCEED,'                           >>$destfile
echo '    CTC_E_NH_NO_LABEL,'                           >>$destfile
echo '    CTC_E_NH_ISNT_UNROV,'                           >>$destfile
echo '    CTC_E_NH_IS_UNROV,'                           >>$destfile
echo '    CTC_E_NH_EXCEED_MAX_DSNH_OFFSET,'                           >>$destfile
echo '    CTC_E_NH_EXCEED_MAX_LOGICAL_REPLI_CNT,'                           >>$destfile
echo '    CTC_E_NH_MEMBER_IS_UPMEP,'                           >>$destfile
echo '    CTC_E_NH_EGS_EDIT_CONFLICT_VLAN_AND_MACDA_EDIT,'                           >>$destfile
echo '    CTC_E_NH_EXIST_VC_LABEL,'                           >>$destfile
echo '    CTC_E_NH_NOT_EXIST_TUNNEL_LABEL,'                           >>$destfile
echo '    CTC_E_NH_NHOFFSET_EXHAUST,'                           >>$destfile
echo '    CTC_E_NH_TOO_MANY_LABELS,'                           >>$destfile
echo '    CTC_E_NH_EXCEED_MAX_ECMP_NUM,'                           >>$destfile
echo '    CTC_E_NH_ECMP_MEM_NOS_EXIST,'                           >>$destfile
echo '    CTC_E_NH_TUNNEL_GRE_KEY_IS_TOO_BIG,'                           >>$destfile
echo '    CTC_E_NH_ECMP_IN_USE,'                           >>$destfile
echo '    CTC_E_NH_HR_NH_IN_USE,'                           >>$destfile
echo '    CTC_E_NH_CROSS_CHIP_RSPAN_NH_IN_USE,'                           >>$destfile
echo '    CTC_E_NH_MCAST_MIRROR_NH_IN_USE,'                           >>$destfile
echo '    CTC_E_NH_EXCEED_MCAST_PHY_REP_NUM,'                           >>$destfile
echo '    CTC_E_NH_LABEL_IS_MISMATCH_WITH_STATS,'                           >>$destfile
echo '    CTC_E_NH_SUPPORT_PW_APS_UPDATE_BY_INT_ALLOC_DSNH,'                           >>$destfile
echo '    CTC_E_NH_INVALID_TPID_INDEX,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_L3IF_VMAC_NOT_EXIST = -29100,'                           >>$destfile
echo '    CTC_E_L3IF_VMAC_INVALID_PREFIX_TYPE,'                           >>$destfile
echo '    CTC_E_L3IF_VMAC_ENTRY_EXCEED_MAX_SIZE,'                           >>$destfile
echo '    CTC_E_L3IF_ROUTED_EXCEED_IF_SIZE,'                           >>$destfile
echo '    CTC_E_L3IF_INVALID_IF_TYPE,'                           >>$destfile
echo '    CTC_E_L3IF_INVALID_IF_ID,'                           >>$destfile
echo '    CTC_E_L3IF_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_L3IF_EXIST,'                           >>$destfile
echo '    CTC_E_L3IF_MISMATCH,'                           >>$destfile
echo '    CTC_E_L3IF_NO_ALLOCED_IPUCSA,'                           >>$destfile
echo '    CTC_E_L3IF_INVALID_PBR_LABEL,'                           >>$destfile
echo '    CTC_E_L3IF_INVALID_RPF_CHECK_TYPE,'                           >>$destfile
echo '    CTC_E_L3IF_INVALID_RPF_STRICK_TYPE,'                           >>$destfile
echo '    CTC_E_L3IF_INVALID_ACL_LABEL,'                           >>$destfile
echo '    CTC_E_L3IF_VRF_STATS_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_L3IF_ROUTER_MAC_EXHAUSTED,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_ACLQOS_ENTRY_NOT_EXIST = -29000,'                           >>$destfile
echo '    CTC_E_ACLQOS_ENTRY_EXIST,'                           >>$destfile
echo '    CTC_E_ACLQOS_INVALID_ACTION,'                           >>$destfile
echo '    CTC_E_ACLQOS_INVALID_KEY_TYPE,'                           >>$destfile
echo '    CTC_E_ACLQOS_INVALID_LABEL_TYPE,'                           >>$destfile
echo '    CTC_E_ACLQOS_LABEL_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_ACLQOS_HAVE_ENABLED,'                           >>$destfile
echo '    CTC_E_ACLQOS_NOT_ENABLED,'                           >>$destfile
echo '    CTC_E_ACLQOS_LABEL_IN_USE,'                           >>$destfile
echo '    CTC_E_QOS_POLICER_CREATED,'                           >>$destfile
echo '    CTC_E_QOS_POLICER_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_QOS_POLICER_NOT_BIND,'                           >>$destfile
echo '    CTC_E_QOS_POLICER_IN_USE,'                           >>$destfile
echo '    CTC_E_QOS_POLICER_CIR_GREATER_THAN_PIR,'                           >>$destfile
echo '    CTC_E_QOS_POLICER_CBS_GREATER_THAN_PBS,'                           >>$destfile
echo '    CTC_E_QOS_POLICER_SERVICE_POLICER_NOT_ENABLE,'                           >>$destfile
echo '    CTC_E_QOS_POLICER_STATS_NOT_ENABLE,'                           >>$destfile
echo '    CTC_E_QOS_QUEUE_STATS_NOT_ENABLE,'                           >>$destfile
echo '    CTC_E_QOS_NO_INT_POLICER_ENTRY,'                           >>$destfile
echo '    CTC_E_QOS_NO_EXT_POLICER_ENTRY,'                           >>$destfile
echo '    CTC_E_QOS_NO_INT_POLICER_PROFILE_ENTRY,'                           >>$destfile
echo '    CTC_E_ACLQOS_DIFFERENT_DIR,'                           >>$destfile
echo '    CTC_E_ACLQOS_DIFFERENT_TYPE,'                           >>$destfile
echo '    CTC_E_ACLQOS_DIFFERENT_CHIP,'                           >>$destfile
echo '    CTC_E_ACL_PBR_ECMP_CMP_FAILED,'                           >>$destfile
echo '    CTC_E_ACL_PBR_DO_ECMP_FAILED,'                           >>$destfile
echo '    CTC_E_ACL_PBR_ENTRY_NO_NXTTHOP,'                           >>$destfile
echo '    CTC_E_QOS_THIS_PORT_NOT_SUPPORT_POLICER,'                           >>$destfile
echo '    CTC_E_QOS_POLICER_NOT_SUPPORT_STATS,'                           >>$destfile
echo '    CTC_E_ACL_INIT,'                           >>$destfile
echo '    CTC_E_ACL_GROUP_PRIORITY,'                           >>$destfile
echo '    CTC_E_ACL_GROUP_TYPE,'                           >>$destfile
echo '    CTC_E_ACL_GROUP_ID,'                           >>$destfile
echo '    CTC_E_ACL_GROUP_ID_RSVED,'                           >>$destfile
echo '    CTC_E_ACL_ENTRY_EXIST,'                           >>$destfile
echo '    CTC_E_ACL_ENTRY_UNEXIST,'                           >>$destfile
echo '    CTC_E_ACL_GROUP_UNEXIST,'                           >>$destfile
echo '    CTC_E_ACL_GROUP_EXIST,'                           >>$destfile
echo '    CTC_E_ACL_GROUP_NOT_EMPTY,'                           >>$destfile
echo '    CTC_E_ACL_INVALID_ACTION,'                           >>$destfile
echo '    CTC_E_ACL_INVALID_KEY_TYPE,'                           >>$destfile
echo '    CTC_E_ACL_INVALID_LABEL_TYPE,'                           >>$destfile
echo '    CTC_E_ACL_LABEL_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_ACL_HAVE_ENABLED,'                           >>$destfile
echo '    CTC_E_ACL_NOT_ENABLED,'                           >>$destfile
echo '    CTC_E_ACL_LABEL_IN_USE,'                           >>$destfile
echo '    CTC_E_ACL_FLAG_COLLISION,'                           >>$destfile
echo '    CTC_E_ACL_L4_PORT_RANGE_EXHAUSTED,'                           >>$destfile
echo '    CTC_E_ACL_TCP_FLAG_EXHAUSTED,'                           >>$destfile
echo '    CTC_E_ACL_GET_NODES_FAILED ,'                           >>$destfile
echo '    CTC_E_ACL_PORT_BITMAP_OVERFLOW,'                           >>$destfile
echo '    CTC_E_ACL_GROUP_INFO,'                           >>$destfile
echo '    CTC_E_ACL_ENTRY_INSTALLED,'                           >>$destfile
echo '    CTC_E_ACL_GET_STATS_FAILED,'                           >>$destfile
echo '    CTC_E_ACL_GROUP_NOT_INSTALLED,'                           >>$destfile
echo '    CTC_E_ACL_SERVICE_ID,'                           >>$destfile
echo '    CTC_E_ACL_GLOBAL_CFG_ERROR,'                           >>$destfile
echo '    CTC_E_ACL_STATS_NOT_SUPPORT,'                           >>$destfile
echo '    CTC_E_ACL_ADD_TCAM_ENTRY_TO_WRONG_GROUP,'                           >>$destfile
echo '    CTC_E_ACL_ADD_HASH_ENTRY_TO_WRONG_GROUP,'                           >>$destfile
echo '    CTC_E_ACL_HASH_ENTRY_NO_PRIORITY,'                           >>$destfile
echo '    CTC_E_ACL_HASH_ENTRY_UNSUPPORT_COPY,'                           >>$destfile
echo '    CTC_E_ACL_VLAN_EDIT_INVALID,'                           >>$destfile
echo '    CTC_E_ACL_INVALID_EGRESS_ACTION,'                           >>$destfile
echo '    CTC_E_ACL_NO_ROOM_TO_MOVE_ENTRY,'                           >>$destfile
echo '    CTC_E_ACL_HASH_CONFLICT,'                           >>$destfile
echo '    CTC_E_ACL_IPV4_NO_LAYER3_TYPE,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_STATS_MTU2_LESS_MTU1 = -28900,'                           >>$destfile
echo '    CTC_E_STATS_MTU1_GREATER_MTU2,'                           >>$destfile
echo '    CTC_E_STATS_PORT_STATS_NO_TYPE,'                           >>$destfile
echo '    CTC_E_STATS_PHB_STATS_INVALID,'                           >>$destfile
echo '    CTC_E_STATS_PORT_NOT_MAP_TO_MAC,'                           >>$destfile
echo '    CTC_E_STATS_PORT_NOT_ENABLE,'                           >>$destfile
echo '    CTC_E_STATS_MAC_STATS_MODE_ASIC,'                           >>$destfile
echo '    CTC_E_STATS_TABLE_NUM_NO_EXPECT,'                           >>$destfile
echo '    CTC_E_STATS_VLAN_STATS_CONFLICT_WITH_PHB,'                           >>$destfile
echo '    CTC_E_STATS_NOT_ENABLE,'                           >>$destfile
echo '    CTC_E_STATS_DMA_ENABLE_BUT_SW_DISABLE,'                           >>$destfile
echo '    CTC_E_STATS_STATSID_EXIST,'                           >>$destfile
echo '    CTC_E_STATS_STATSID_TYPE_MISMATCH,'                           >>$destfile
echo '    CTC_E_STATS_STATSID_ALREADY_IN_USE,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_IPUC_VERSION_DISABLE = -28800,'                           >>$destfile
echo '    CTC_E_IPUC_INVALID_VRF,'                           >>$destfile
echo '    CTC_E_IPUC_INVALID_L3IF,'                           >>$destfile
echo '    CTC_E_IPUC_NEED_L3IF,'                           >>$destfile
echo '    CTC_E_IPUC_TUNNEL_INVALID,'                           >>$destfile
echo '    CTC_E_IPUC_RPF_NOT_SUPPORT_THIS_MASK_LEN,'                           >>$destfile
echo '    CTC_E_IPUC_NAT_NOT_SUPPORT_THIS_MASK_LEN,'                           >>$destfile
echo '    CTC_E_IPUC_INVALID_TTL_THRESHOLD,'                           >>$destfile
echo '    CTC_E_IPUC_TUNNEL_RPF_INFO_MISMATCH,'                           >>$destfile
echo '    CTC_E_IPUC_ADD_FAILED,'                           >>$destfile
echo '    CTC_E_IPUC_TUNNEL_PAYLOAD_TYPE_MISMATCH,'                           >>$destfile
echo '    CTC_E_IPUC_HASH_CONFLICT,'                           >>$destfile
echo '    CTC_E_IPUC_INVALID_ROUTE_FLAG,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_IPMC_GROUP_NOT_EXIST = -28700,'                           >>$destfile
echo '    CTC_E_IPMC_GROUP_HAS_EXISTED,'                           >>$destfile
echo '    CTC_E_IPMC_INVALID_MASK_LEN,'                           >>$destfile
echo '    CTC_E_IPMC_BAD_L3IF_TYPE,'                           >>$destfile
echo '    CTC_E_IPMC_NOT_MCAST_ADDRESS,'                           >>$destfile
echo '    CTC_E_IPMC_EXCEED_GLOBAL_CHIP,'                           >>$destfile
echo '    CTC_E_IPMC_ADD_MEMBER_FAILED,'                           >>$destfile
echo '    CTC_E_IPMC_GROUP_IS_NOT_IP_MAC,'                           >>$destfile
echo '    CTC_E_IPMC_GROUP_IS_IP_MAC,'                           >>$destfile
echo '    CTC_E_IPMC_INVALID_VRF,'                           >>$destfile
echo '    CTC_E_IPMC_GROUP_IS_NOT_INIT,'                           >>$destfile
echo '    CTC_E_IPMC_VERSION_DISABLE,'                           >>$destfile
echo '    CTC_E_IPMC_RPF_CHECK_DISABLE,'                           >>$destfile
echo '    CTC_E_IPMC_OFFSET_ALLOC_ERROR,'                           >>$destfile
echo '    CTC_E_IPMC_STATS_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_IPMC_HASH_CONFLICT,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_PDU_INVALID_ACTION_INDEX = -28600,'                           >>$destfile
echo '    CTC_E_PDU_INVALID_INDEX,'                           >>$destfile
echo '    CTC_E_PDU_INVALID_IPDA,'                           >>$destfile
echo '    CTC_E_PDU_INVALID_MACDA,'                           >>$destfile
echo '    CTC_E_PDU_INVALID_MACDA_MASK,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_QUEUE_DROP_PROF_NOT_EXIST = -28500,'                           >>$destfile
echo '    CTC_E_QUEUE_SHAPE_PROF_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_QUEUE_CHANNEL_SHAPE_PROF_EXIST,'                           >>$destfile
echo '    CTC_E_QUEUE_CHANNEL_SHAPE_PROF_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_QUEUE_NOT_ENOUGH,'                           >>$destfile
echo '    CTC_E_QUEUE_SERVICE_QUEUE_NOT_INITIALIZED,'                           >>$destfile
echo '    CTC_E_QUEUE_SERVICE_QUEUE_INITIALIZED,'                           >>$destfile
echo '    CTC_E_QUEUE_SERVICE_EXCEED_MAX_COUNT_LIMIT,'                           >>$destfile
echo '    CTC_E_QUEUE_SERVICE_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_QUEUE_SERVICE_REGISTERED,'                           >>$destfile
echo '    CTC_E_QUEUE_SERVICE_EXIST,'                           >>$destfile
echo '    CTC_E_QUEUE_SERVICE_QUEUE_NO_HASH_ENTRY,'                           >>$destfile
echo '    CTC_E_QUEUE_SERVICE_GROUP_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_QUEUE_INTERNAL_PORT_IN_USE,'                           >>$destfile
echo '    CTC_E_QUEUE_NO_FREE_INTERNAL_PORT,'                           >>$destfile
echo '    CTC_E_QUEUE_DEPTH_NOT_EMPTY,'                           >>$destfile
echo '    CTC_E_QUEUE_WDRR_WEIGHT_TOO_BIG,'                           >>$destfile
echo '    CTC_E_QUEUE_INVALID_CONFIG,'                           >>$destfile
echo '    CTC_E_QUEUE_CPU_REASON_NOT_CREATE,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_APS_GROUP_EXIST = -28400,'                           >>$destfile
echo '    CTC_E_APS_INTERFACE_ERROR,'                           >>$destfile
echo '    CTC_E_APS_INVALID_GROUP_ID,'                           >>$destfile
echo '    CTC_E_APS_INVALID_TYPE,'                           >>$destfile
echo '    CTC_E_APS_GROUP_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_APS_GROUP_NEXT_APS_EXIST,'                           >>$destfile
echo '    CTC_E_APS_MPLS_TYPE_L3IF_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_APS_RAPS_GROUP_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_APS_DONT_SUPPORT_2_LEVEL_HW_BASED_APS,'                           >>$destfile
echo '    CTC_E_APS_DONT_SUPPORT_HW_BASED_APS_FOR_LINKAGG,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_VPLS_VSI_NOT_CREATE = -28300,'                           >>$destfile
echo '    CTC_E_VPLS_INVALID_VPLSPORT_ID,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_HW_OP_FAIL = -28000,'                           >>$destfile
echo '    CTC_E_DRV_FAIL,'                           >>$destfile
echo '    CTC_E_CHECK_HSS_READY_FAIL,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_OAM_NOT_INIT = -27900,'                           >>$destfile
echo '    CTC_E_OAM_MAID_LENGTH_INVALID,'                           >>$destfile
echo '    CTC_E_OAM_MAID_ENTRY_EXIST,'                           >>$destfile
echo '    CTC_E_OAM_MAID_ENTRY_NOT_FOUND,'                           >>$destfile
echo '    CTC_E_OAM_MAID_ENTRY_REF_BY_MEP,'                           >>$destfile
echo '    CTC_E_OAM_CHAN_ENTRY_EXIST,'                           >>$destfile
echo '    CTC_E_OAM_CHAN_ENTRY_NOT_FOUND,'                           >>$destfile
echo '    CTC_E_OAM_CHAN_MD_LEVEL_EXIST,'                           >>$destfile
echo '    CTC_E_OAM_CHAN_MD_LEVEL_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_OAM_CHAN_ENTRY_NUM_EXCEED,'                           >>$destfile
echo '    CTC_E_OAM_LM_NUM_RXCEED,'                           >>$destfile
echo '    CTC_E_OAM_CHAN_LMEP_EXIST,'                           >>$destfile
echo '    CTC_E_OAM_CHAN_LMEP_NOT_FOUND,'                           >>$destfile
echo '    CTC_E_OAM_CHAN_NOT_UP_DIRECTION,'                           >>$destfile
echo '    CTC_E_OAM_CHAN_NOT_DOWN_DIRECTION,'                           >>$destfile
echo '    CTC_E_OAM_TX_GPORT_AND_MASTER_GCHIP_NOT_MATCH,'                           >>$destfile
echo '    CTC_E_OAM_TX_GPORT_AND_CHAN_GPORT_NOT_MATCH,'                           >>$destfile
echo '    CTC_E_OAM_RMEP_EXIST,'                           >>$destfile
echo '    CTC_E_OAM_RMEP_NOT_FOUND,'                           >>$destfile
echo '    CTC_E_OAM_MEP_LM_NOT_EN,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_MD_LEVEL,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_MEP_ID,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_MEP_CCM_INTERVAL,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_MEP_TPID_INDEX,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_MEP_COS,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_CFG_IN_MEP_TYPE,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_CFG_LM,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_LM_TYPE,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_LM_COS_TYPE,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_CSF_TYPE,'                           >>$destfile
echo '    CTC_E_OAM_NH_OFFSET_EXIST,'                           >>$destfile
echo '    CTC_E_OAM_NH_OFFSET_NOT_FOUND,'                           >>$destfile
echo '    CTC_E_OAM_NH_OFFSET_ADD_VECTOR_FAIL,'                           >>$destfile
echo '    CTC_E_OAM_NH_OFFSET_IN_USE,'                           >>$destfile
echo '    CTC_E_OAM_RMEP_D_LOC_PRESENT,'                           >>$destfile
echo '    CTC_E_OAM_ITU_DEFECT_CLEAR_MODE,'                           >>$destfile
echo '    CTC_E_OAM_MEP_INDEX_VECTOR_ADD_FAIL,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_OPERATION_ON_CPU_MEP,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_OPERATION_ON_P2P_MEP,'                           >>$destfile
echo '    CTC_E_OAM_DRIVER_FAIL,'                           >>$destfile
echo '    CTC_E_OAM_EXCEED_MAX_LOOP_BACK_PORT_NUMBER,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_GLOBAL_PARAM_TYPE,'                           >>$destfile
echo '    CTC_E_OAM_INDEX_EXIST,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_MA_INDEX,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_MEP_INDEX,'                           >>$destfile
echo '    CTC_E_OAM_INVALID_OAM_TYPE,'                           >>$destfile
echo '    CTC_E_OAM_EXCEED_MAX_LEVEL_NUM_PER_CHAN,'                           >>$destfile
echo '    CTC_E_OAM_BFD_INVALID_STATE,'                           >>$destfile
echo '    CTC_E_OAM_BFD_INVALID_DIAG,'                           >>$destfile
echo '    CTC_E_OAM_BFD_INVALID_INTERVAL,'                           >>$destfile
echo '    CTC_E_OAM_BFD_INVALID_DETECT_MULT,'                           >>$destfile
echo '    CTC_E_OAM_BFD_INVALID_UDP_PORT,'                           >>$destfile
echo '    CTC_E_OAM_TRPT_NOT_INIT,'                           >>$destfile
echo '    CTC_E_OAM_TRPT_INTERVAL_CONFLICT,'                           >>$destfile
echo '    CTC_E_OAM_TRPT_SESSION_EN,'                           >>$destfile
echo '    CTC_E_OAM_TRPT_SESSION_ALREADY_EN,'                           >>$destfile
echo '    CTC_E_OAM_TRPT_SESSION_NOT_CFG,'                           >>$destfile
echo '    CTC_E_OAM_TRPT_PERIOD_INVALID,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_PTP_NOT_INIT = -27800,'                           >>$destfile
echo '    CTC_E_PTP_EXCEED_MAX_FRACTIONAL_VALUE,'                           >>$destfile
echo '    CTC_E_PTP_EXCEED_MAX_FFO_VALUE,'                           >>$destfile
echo '    CTC_E_PTP_INVALID_SYNC_INTF_PARAM_VALUE,'                           >>$destfile
echo '    CTC_E_PTP_TS_NOT_READY,'                           >>$destfile
echo '    CTC_E_PTP_TX_TS_ROLL_OVER_FAILURE,'                           >>$destfile
echo '    CTC_E_PTP_TS_GET_BUFF_FAIL,'                           >>$destfile
echo '    CTC_E_PTP_TS_ADD_BUFF_FAIL,'                           >>$destfile
echo '    CTC_E_PTP_RX_TS_INFORMATION_NOT_READY,'                           >>$destfile
echo '    CTC_E_PTP_EXCEED_MAX_DRIFT_VALUE,'                           >>$destfile
echo '    CTC_E_PTP_TAI_LESS_THAN_GPS,'                           >>$destfile
echo '    CTC_E_PTP_CAN_NOT_SET_TOW,'                           >>$destfile
echo '    CTC_E_PTP_SYNC_SIGNAL_FREQUENCY_OUT_OF_RANGE,'                           >>$destfile
echo '    CTC_E_PTP_SYNC_SIGNAL_NOT_VALID,'                           >>$destfile
echo '    CTC_E_PTP_SYNC_SIGNAL_DUTY_OUT_OF_RANGE,'                           >>$destfile
echo '    CTC_E_PTP_INVALID_SYNC_INTF_ACCURACY_VALUE,'                           >>$destfile
echo '    CTC_E_PTP_INVALID_SYNC_INTF_EPOCH_VALUE,'                           >>$destfile
echo '    CTC_E_PTP_INVALID_TOD_INTF_PARAM_VALUE,'                           >>$destfile
echo '    CTC_E_PTP_INVALID_TOD_INTF_EPOCH_VALUE,'                           >>$destfile
echo '    CTC_E_PTP_INVALID_TOD_INTF_PPS_STATUS_VALUE,'                           >>$destfile
echo '    CTC_E_PTP_INVALID_TOD_INTF_CLOCK_SRC_VALUE,'                           >>$destfile
echo '    CTC_E_PTP_INVALID_CAPTURED_TS_SEQ_ID,'                           >>$destfile
echo '    CTC_E_PTP_INVALID_CAPTURED_TS_SOURCE,'                           >>$destfile
echo '    CTC_E_PTP_INVALID_CAPTURED_TYPE,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_LEARNING_AND_AGING_INVALID_AGING_THRESHOLD = -27700,'                           >>$destfile
echo '    CTC_E_LEARNING_AND_AGING_INVALID_LEARNING_THRESHOLD,'                           >>$destfile
echo '    CTC_E_AGING_INVALID_INTERVAL,'                           >>$destfile
echo '    CTC_E_INVALID_MAC_FILTER_SIZE,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_ALLOCATION_INVALID_ENTRY_SIZE = -27600,'                           >>$destfile
echo '    CTC_E_ALLOCATION_INVALID_KEY_SIZE,'                           >>$destfile
echo '    CTC_E_ALLOCATION_EXCEED_INT_TCAM_PHYSIZE,'                           >>$destfile
echo '    CTC_E_ALLOCATION_EXCEED_EXT_TCAM_PHYSIZE,'                           >>$destfile
echo '    CTC_E_ALLOCATION_EXCEED_HASH_PHYSIZE,'                           >>$destfile
echo '    CTC_E_ALLOCATION_INVALID_HASH_PHYSIZE,'                           >>$destfile
echo '    CTC_E_ALLOCATION_EXCEEED_SRAM_PHYSIZE,'                           >>$destfile
echo '    CTC_E_ALLOCATION_EXCEED_MAX_OAM,'                           >>$destfile
echo '    CTC_E_HASH_MEM_INIT_TIMEOUT,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_MIRROR_EXCEED_SESSION = -27500,'                           >>$destfile
echo '    CTC_E_MIRROR_INVALID_MIRROR_INFO_TYPE,'                           >>$destfile
echo '    CTC_E_MIRROR_INVALID_MIRROR_LOG_ID,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_MPLS_ENTRY_NOT_SUPPORT_STATS = -27400,'                           >>$destfile
echo '    CTC_E_MPLS_ENTRY_STATS_EXIST,'                           >>$destfile
echo '    CTC_E_MPLS_ENTRY_STATS_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_MPLS_NO_RESOURCE,'                           >>$destfile
echo '    CTC_E_MPLS_SPACE_ERROR,'                           >>$destfile
echo '    CTC_E_MPLS_LABEL_ERROR,'                           >>$destfile
echo '    CTC_E_MPLS_LABEL_TYPE_ERROR,'                           >>$destfile
echo '    CTC_E_MPLS_VRF_ID_ERROR,'                           >>$destfile
echo '    CTC_E_MPLS_SPACE_NO_RESOURCE,'                           >>$destfile
echo '    CTC_E_MPLS_LABEL_RANGE_ERROR,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_DMA_TX_FAILED = -27200,'                           >>$destfile
echo '    CTC_E_DMA_TABLE_WRITE_FAILED,'                           >>$destfile
echo '    CTC_E_DMA_TABLE_READ_FAILED,'                           >>$destfile
echo '    CTC_E_DMA_DESC_INVALID,'                           >>$destfile
echo '    CTC_E_DMA_INVALID_PKT_LEN,'                           >>$destfile
echo '    CTC_E_DMA_NOT_INIT,'                           >>$destfile
echo '    CTC_E_DMA_INVALID_SUB_TYPE,'                           >>$destfile
echo '    CTC_E_DMA_INVALID_CHAN_ID,'                           >>$destfile
echo '    CTC_E_DMA_EXCEED_MAX_DESC_NUM,'                           >>$destfile
echo '    CTC_E_DMA_DESC_NOT_DONE,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_PARSER_INVALID_L3_TYPE = -27100,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_STK_NOT_INIT = -27000,'                           >>$destfile
echo '    CTC_E_STK_INVALID_TRUNK_ID,'                           >>$destfile
echo '    CTC_E_STK_TRUNK_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_STK_TRUNK_HAS_EXIST,'                           >>$destfile
echo '    CTC_E_STK_TRUNK_EXCEED_MAX_MEMBER_PORT,'                           >>$destfile
echo '    CTC_E_STK_TRUNK_MEMBER_PORT_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_STK_TRUNK_MEMBER_PORT_EXIST,'                           >>$destfile
echo '    CTC_E_STK_TRUNK_MEMBER_PORT_INVALID,'                           >>$destfile
echo '    CTC_E_STK_TRUNK_HDR_DSCP_NOT_VALID,'                           >>$destfile
echo '    CTC_E_STK_TRUNK_HDR_COS_NOT_VALID,'                           >>$destfile
echo '    CTC_E_STK_KEEPLIVE_TWO_MEMBER_AND_ONE_CPU,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_SYNCE_NOT_INIT = -26900,'                           >>$destfile
echo '    CTC_E_SYNCE_CLOCK_ID_EXCEED_MAX_VALUE,'                           >>$destfile
echo '    CTC_E_SYNCE_DIVIDER_EXCEED_MAX_VALUE,'                           >>$destfile
echo '    CTC_E_SYNCE_INVALID_RECOVERED_PORT,'                           >>$destfile
echo '    CTC_E_SYNCE_NO_SERDES_INFO,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_INTR_NOT_INIT = -26800,'                           >>$destfile
echo '    CTC_E_INTR_INVALID_TYPE,'                           >>$destfile
echo '    CTC_E_INTR_INVALID_SUB_TYPE,'                           >>$destfile
echo '    CTC_E_INTR_NOT_SUPPORT,'                           >>$destfile
echo '    CTC_E_INTR_INVALID_PARAM,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_STMCTL_INVALID_UC_MODE = -26700,'                           >>$destfile
echo '    CTC_E_STMCTL_INVALID_MC_MODE,'                           >>$destfile
echo '    CTC_E_STMCTL_INVALID_THRESHOLD,'                           >>$destfile
echo '    CTC_E_STMCTL_INVALID_PKT_TYPE,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_BPE_NOT_INIT = -26600,'                           >>$destfile
echo '    CTC_E_BPE_INVALID_PORT_BASE,'                           >>$destfile
echo '    CTC_E_BPE_INVALID_VLAN_BASE,'                           >>$destfile
echo '    CTC_E_BPE_INVALID_BPE_CAPABLE_GPORT,'                           >>$destfile
echo '    CTC_E_BPE_INVLAID_BPE_MODE,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_OVERLAY_TUNNEL_INVALID_TYPE = -26500,'                           >>$destfile
echo '    CTC_E_OVERLAY_TUNNEL_INVALID_VN_ID,'                           >>$destfile
echo '    CTC_E_OVERLAY_TUNNEL_INVALID_FID,'                           >>$destfile
echo '    CTC_E_OVERLAY_TUNNEL_VN_FID_NOT_EXIST,'                           >>$destfile
echo '    CTC_E_OVERLAY_TUNNEL_NO_DA_INDEX_RESOURCE,'                           >>$destfile
echo ''                           >>$destfile
echo ''                           >>$destfile
echo '    CTC_E_DATAPATH_HSS_NOT_READY = -26400,'                           >>$destfile
echo '    CTC_E_DATAPATH_CLKTREE_CFG_ERROR,'                           >>$destfile
echo '    CTC_E_DATAPATH_INVALID_PORT_TYPE,'                           >>$destfile
echo '    CTC_E_DATAPATH_INVALID_CORE_FRE,'                           >>$destfile
echo '    CTC_E_DATAPATH_PLL_NOT_LOCK,'                           >>$destfile
echo '    CTC_E_DATAPATH_PLL_EXIST_LANE,'                           >>$destfile
echo '    CTC_E_DATAPATH_MAC_ENABLE,'                           >>$destfile
echo '    CTC_E_DATAPATH_SWITCH_FAIL'                           >>$destfile
echo ''                           >>$destfile
echo '};'                           >>$destfile
echo 'typedef enum ctc_err_old_temp_e ctc_err_old_temp_t;'                           >>$destfile
fi
echo  'const char *ctc_get_error_desc(int32 error_code)'                                                    >>$destfile
echo  '{'                                                                                                          >>$destfile
echo  '    static char  error_desc[256+1] = {0};'                                                                     >>$destfile
echo  '    error_code = ctc_error_code_mapping(error_code);'                                                       >>$destfile
echo  '    if (error_code == CTC_E_NONE)'                                                                          >>$destfile
echo  '    {'                                                                                                      >>$destfile
echo  '        return " ";'                                                                                        >>$destfile
echo  '    }'                                                                                                      >>$destfile
echo  ''                                                                                                           >>$destfile
echo  '    switch(error_code)'                                                                                     >>$destfile
echo  '    {'                                                                                                      >>$destfile
sed "/^\s*CTC_E/! d;s/^\s*\(CTC_E_\w*\).*<\s*\(.*\)\s*\*.*$/        case \1:  \n          return \"\2\";   /" $srcfile          >>$destfile
echo  '    default:'                                                                                             >>$destfile
echo  '      sal_sprintf(error_desc,"Error code:%d",error_code);'                                                  >>$destfile
echo  '      return error_desc;'                                                                                   >>$destfile
echo  '    }'                                                                                                      >>$destfile
echo  ''                                                                                                           >>$destfile
echo  '}'                                                                                                          >>$destfile
echo '' >>$destfile
echo 'int32' >>$destfile
echo 'ctc_error_code_mapping(int32 error_code)' >>$destfile
echo '{' >>$destfile
echo '    if (g_mapping_disable || (error_code < CTC_E_ENTRY_EXIST) || (CTC_E_NONE == error_code))' >>$destfile
echo '    {' >>$destfile
echo '        return error_code;' >>$destfile
echo '    }' >>$destfile
echo '' >>$destfile
echo '    if (error_code >= CTC_E_ERROE_CODE_END)' >>$destfile
echo '    {' >>$destfile
echo '        switch (error_code)' >>$destfile
echo '        {' >>$destfile
echo '            case -10000:' >>$destfile
echo '                return CTC_E_NO_MEMORY;' >>$destfile
echo '            case -9999:' >>$destfile
echo '                return CTC_E_HW_BUSY;' >>$destfile
echo '            case -9998:' >>$destfile
echo '                return CTC_E_HW_TIME_OUT;' >>$destfile
echo '            case -9997:' >>$destfile
echo '                return CTC_E_HW_INVALID_INDEX;' >>$destfile
echo '            default:' >>$destfile
echo '                return CTC_E_HW_FAIL;' >>$destfile
echo '        }' >>$destfile
echo '    }' >>$destfile
echo '' >>$destfile
echo '    switch (error_code)' >>$destfile
echo '    {' >>$destfile
echo '        case CTC_E_HW_BUSY:' >>$destfile
echo '            return CTC_E_HW_BUSY;' >>$destfile
echo '        case CTC_E_HW_INVALID_INDEX:' >>$destfile
echo '            return CTC_E_HW_INVALID_INDEX;' >>$destfile
echo '        case CTC_E_INVALID_PTR:' >>$destfile
echo '        case CTC_E_VLAN_INVALID_VLAN_PTR:' >>$destfile
echo '            return CTC_E_INVALID_PTR;' >>$destfile
echo '        case CTC_E_INVALID_PORT:' >>$destfile
echo '        case CTC_E_INVALID_GLOBAL_PORT:' >>$destfile
echo '        case CTC_E_INVALID_LOGIC_PORT:' >>$destfile
echo '        case CTC_E_INVALID_LOCAL_PORT:' >>$destfile
echo '        case CTC_E_VPLS_INVALID_VPLSPORT_ID:' >>$destfile
echo '        case CTC_E_STK_TRUNK_MEMBER_PORT_INVALID:' >>$destfile
echo '        case CTC_E_VLAN_EXCEED_MAX_PHY_PORT:' >>$destfile
echo '        case CTC_E_FDB_NOT_LOCAL_MEMBER:' >>$destfile
echo '        case CTC_E_SCL_INVALID_DEFAULT_PORT:' >>$destfile
echo '            return CTC_E_INVALID_PORT;' >>$destfile
echo '        case CTC_E_INVALID_CHIP_ID:' >>$destfile
echo '        case CTC_E_INVALID_GLOBAL_CHIPID:' >>$destfile
echo '        case CTC_E_INVALID_LOCAL_CHIPID:' >>$destfile
echo '        case CTC_E_CHIP_IS_LOCAL:' >>$destfile
echo '        case CTC_E_CHIP_IS_REMOTE:' >>$destfile
echo '        case CTC_E_IPMC_EXCEED_GLOBAL_CHIP:' >>$destfile
echo '            return CTC_E_INVALID_CHIP_ID;' >>$destfile
echo '        case CTC_E_HW_TIME_OUT:' >>$destfile
echo '        case CTC_E_HASH_MEM_INIT_TIMEOUT:' >>$destfile
echo '        case CTC_E_CHIP_TIME_OUT:' >>$destfile
echo '            return CTC_E_HW_TIME_OUT;' >>$destfile
echo '        case CTC_E_HW_NOT_LOCK:' >>$destfile
echo '        case CTC_E_DATAPATH_PLL_NOT_LOCK:' >>$destfile
echo '            return CTC_E_HW_NOT_LOCK;' >>$destfile
echo '        case CTC_E_DMA:' >>$destfile
echo '        case CTC_E_DMA_TX_FAILED:' >>$destfile
echo '        case CTC_E_DMA_TABLE_WRITE_FAILED:' >>$destfile
echo '        case CTC_E_DMA_TABLE_READ_FAILED:' >>$destfile
echo '        case CTC_E_DMA_EXCEED_MAX_DESC_NUM:' >>$destfile
echo '        case CTC_E_DMA_DESC_NOT_DONE:' >>$destfile
echo '            return CTC_E_DMA;' >>$destfile
echo '        case CTC_E_HW_FAIL:' >>$destfile
echo '        case CTC_E_OAM_DRIVER_FAIL:' >>$destfile
echo '        case CTC_E_DATAPATH_HSS_NOT_READY:' >>$destfile
echo '        case CTC_E_SERDES_STATUS_NOT_READY:' >>$destfile
echo '        case CTC_E_SERDES_EYE_TEST_NOT_DONE:' >>$destfile
echo '        case CTC_E_CHIP_SCAN_PHY_REG_FULL:' >>$destfile
echo '        case CTC_E_CHIP_SWITCH_FAILED:' >>$destfile
echo '        case CTC_E_PORT_MAC_PCS_ERR:' >>$destfile
echo '        case CTC_E_DATAPATH_SWITCH_FAIL:' >>$destfile
echo '        case CTC_E_FDB_FIB_DUMP_FAIL:' >>$destfile
echo '        case CTC_E_HW_OP_FAIL:' >>$destfile
echo '        case CTC_E_CHECK_HSS_READY_FAIL:' >>$destfile
echo '        case CTC_E_PTP_TS_NOT_READY:' >>$destfile
echo '        case CTC_E_PTP_TX_TS_ROLL_OVER_FAILURE:' >>$destfile
echo '        case CTC_E_PTP_RX_TS_INFORMATION_NOT_READY:' >>$destfile
echo '        case CTC_E_TRAININT_FAIL:' >>$destfile
echo '            return CTC_E_HW_FAIL;' >>$destfile
echo '        case CTC_E_BADID:' >>$destfile
echo '        case CTC_E_STP_INVALID_STP_ID:' >>$destfile
echo '        case CTC_E_APS_INVALID_GROUP_ID:' >>$destfile
echo '        case CTC_E_OVERLAY_TUNNEL_INVALID_VN_ID:' >>$destfile
echo '        case CTC_E_OVERLAY_TUNNEL_INVALID_FID:' >>$destfile
echo '        case CTC_E_INVALID_DEVICE_ID:' >>$destfile
echo '        case CTC_E_VLAN_INVALID_VLAN_ID:' >>$destfile
echo '        case CTC_E_VLAN_INVALID_FID_ID:' >>$destfile
echo '        case CTC_E_VLAN_INVALID_VRFID:' >>$destfile
echo '        case CTC_E_VLAN_MAPPING_INVALID_SERVICE_ID:' >>$destfile
echo '        case CTC_E_FDB_INVALID_FID:' >>$destfile
echo '        case CTC_E_MIRROR_INVALID_MIRROR_LOG_ID:' >>$destfile
echo '        case CTC_E_L3IF_INVALID_IF_ID:' >>$destfile
echo '        case CTC_E_IPUC_INVALID_VRF:' >>$destfile
echo '        case CTC_E_IPUC_INVALID_L3IF:' >>$destfile
echo '        case CTC_E_IPMC_INVALID_VRF:' >>$destfile
echo '        case CTC_E_INVALID_TID:' >>$destfile
echo '        case CTC_E_SCL_GROUP_ID:' >>$destfile
echo '        case CTC_E_SCL_GROUP_ID_RSVED:' >>$destfile
echo '        case CTC_E_SCL_SERVICE_ID:' >>$destfile
echo '        case CTC_E_ACL_GROUP_ID:' >>$destfile
echo '        case CTC_E_ACL_GROUP_ID_RSVED:' >>$destfile
echo '        case CTC_E_ACL_SERVICE_ID:' >>$destfile
echo '           return CTC_E_BADID;' >>$destfile
echo '        case CTC_E_IN_USE:' >>$destfile
echo '        case CTC_E_NH_GLB_SRAM_IS_INUSE:' >>$destfile
echo '        case CTC_E_NH_GLB_SRAM_ISNOT_INUSE:' >>$destfile
echo '        case CTC_E_NH_EXIST_VC_LABEL:' >>$destfile
echo '        case CTC_E_NH_ECMP_IN_USE:' >>$destfile
echo '        case CTC_E_NH_HR_NH_IN_USE:' >>$destfile
echo '        case CTC_E_NH_CROSS_CHIP_RSPAN_NH_IN_USE:' >>$destfile
echo '        case CTC_E_NH_MCAST_MIRROR_NH_IN_USE:' >>$destfile
echo '        case CTC_E_QUEUE_SERVICE_REGISTERED:' >>$destfile
echo '        case CTC_E_QUEUE_INTERNAL_PORT_IN_USE:' >>$destfile
echo '        case CTC_E_OAM_MAID_ENTRY_REF_BY_MEP:' >>$destfile
echo '        case CTC_E_OAM_NH_OFFSET_IN_USE:' >>$destfile
echo '        case CTC_E_STATS_STATSID_ALREADY_IN_USE:' >>$destfile
echo '        case CTC_E_SCL_LABEL_IN_USE:' >>$destfile
echo '        case CTC_E_ACLQOS_LABEL_IN_USE:' >>$destfile
echo '        case CTC_E_ACL_LABEL_IN_USE:' >>$destfile
echo '        case CTC_E_QOS_POLICER_IN_USE:' >>$destfile
echo '           return CTC_E_IN_USE;' >>$destfile
echo '        case CTC_E_NOT_SUPPORT:' >>$destfile
echo '        case CTC_E_FEATURE_NOT_SUPPORT:' >>$destfile
echo '        case CTC_E_APS_DONT_SUPPORT_HW_BASED_APS_FOR_LINKAGG:' >>$destfile
echo '        case CTC_E_PORT_PVLAN_TYPE_NOT_SUPPORT:' >>$destfile
echo '        case CTC_E_SERDES_MODE_NOT_SUPPORT:' >>$destfile
echo '        case CTC_E_SERDES_PLL_NOT_SUPPORT:' >>$destfile
echo '        case CTC_E_MPLS_ENTRY_NOT_SUPPORT_STATS:' >>$destfile
echo '        case CTC_E_LINKAGG_DYNAMIC_BALANCE_NOT_SUPPORT:' >>$destfile
echo '        case CTC_E_SCL_UNSUPPORT:' >>$destfile
echo '        case CTC_E_INTR_NOT_SUPPORT:' >>$destfile
echo '        case CTC_E_SCL_STATS_NOT_SUPPORT:' >>$destfile
echo '        case CTC_E_SCL_HASH_ENTRY_UNSUPPORT_COPY:' >>$destfile
echo '        case CTC_E_QOS_THIS_PORT_NOT_SUPPORT_POLICER:' >>$destfile
echo '        case CTC_E_QOS_POLICER_NOT_SUPPORT_STATS:' >>$destfile
echo '        case CTC_E_ACL_STATS_NOT_SUPPORT:' >>$destfile
echo '        case CTC_E_ACL_HASH_ENTRY_UNSUPPORT_COPY:' >>$destfile
echo '           return CTC_E_NOT_SUPPORT;' >>$destfile
echo '        case CTC_E_NO_RESOURCE:' >>$destfile
echo '        case CTC_E_FDB_NO_RESOURCE:' >>$destfile
echo '        case CTC_E_FDB_SECURITY_VIOLATION:' >>$destfile
echo '        case CTC_E_NH_NHOFFSET_EXHAUST:' >>$destfile
echo '        case CTC_E_QUEUE_NOT_ENOUGH:' >>$destfile
echo '        case CTC_E_QUEUE_SERVICE_QUEUE_NO_HASH_ENTRY:' >>$destfile
echo '        case CTC_E_QUEUE_NO_FREE_INTERNAL_PORT:' >>$destfile
echo '        case CTC_E_OVERLAY_TUNNEL_NO_DA_INDEX_RESOURCE:' >>$destfile
echo '        case CTC_E_L3IF_ROUTER_MAC_EXHAUSTED:' >>$destfile
echo '        case CTC_E_IPMC_OFFSET_ALLOC_ERROR:' >>$destfile
echo '        case CTC_E_USRID_NO_KEY:' >>$destfile
echo '        case CTC_E_SCL_NO_KEY:' >>$destfile
echo '        case CTC_E_SCL_BUILD_AD_INDEX_FAILED:' >>$destfile
echo '        case CTC_E_SCL_AD_NO_MEMORY:' >>$destfile
echo '        case CTC_E_SCL_L4_PORT_RANGE_EXHAUSTED:' >>$destfile
echo '        case CTC_E_SCL_TCP_FLAG_EXHAUSTED:' >>$destfile
echo '        case CTC_E_ACL_L4_PORT_RANGE_EXHAUSTED:' >>$destfile
echo '        case CTC_E_ACL_TCP_FLAG_EXHAUSTED:' >>$destfile
echo '        case CTC_E_ACL_NO_ROOM_TO_MOVE_ENTRY:' >>$destfile
echo '        case CTC_E_VLAN_MAPPING_VRANGE_FULL:' >>$destfile
echo '           return CTC_E_NO_RESOURCE;' >>$destfile
echo '        case CTC_E_PROFILE_NO_RESOURCE:' >>$destfile
echo '           return CTC_E_PROFILE_NO_RESOURCE;' >>$destfile
echo '        case CTC_E_NO_MEMORY:' >>$destfile
echo '        case CTC_E_CANT_CREATE_AVL:' >>$destfile
echo '        case CTC_E_SPOOL_ADD_UPDATE_FAILED:' >>$destfile
echo '        case CTC_E_RPF_SPOOL_ADD_VECTOR_FAILED:' >>$destfile
echo '        case CTC_E_NO_ROOM_TO_MOVE_ENTRY:' >>$destfile
echo '        case CTC_E_FAIL_CREATE_MUTEX:' >>$destfile
echo '        case CTC_E_CREATE_MEM_CACHE_FAIL:' >>$destfile
echo '        case CTC_E_OAM_MEP_INDEX_VECTOR_ADD_FAIL:' >>$destfile
echo '        case CTC_E_SCL_HASH_INSERT_FAILED:' >>$destfile
echo '        case CTC_E_PTP_TS_ADD_BUFF_FAIL:' >>$destfile
echo '           return CTC_E_NO_MEMORY;' >>$destfile
echo '        case CTC_E_HASH_CONFLICT:' >>$destfile
echo '        case CTC_E_MPLS_NO_RESOURCE:' >>$destfile
echo '        case CTC_E_IPUC_HASH_CONFLICT:' >>$destfile
echo '        case CTC_E_IPMC_HASH_CONFLICT:' >>$destfile
echo '        case CTC_E_SCL_HASH_CONFLICT:' >>$destfile
echo '        case CTC_E_ACL_HASH_CONFLICT:' >>$destfile
echo '        case CTC_E_FDB_HASH_CONFLICT:' >>$destfile 
echo '            return CTC_E_HASH_CONFLICT;' >>$destfile
echo '        case CTC_E_NOT_INIT:' >>$destfile
echo '        case CTC_E_NH_NOT_INIT:' >>$destfile
echo '        case CTC_E_OAM_NOT_INIT:' >>$destfile
echo '        case CTC_E_OAM_TRPT_NOT_INIT:' >>$destfile
echo '        case CTC_E_BPE_NOT_INIT:' >>$destfile
echo '        case CTC_E_STK_NOT_INIT:' >>$destfile
echo '        case CTC_E_IPMC_GROUP_IS_NOT_INIT:' >>$destfile
echo '        case CTC_E_PTP_NOT_INIT:' >>$destfile
echo '        case CTC_E_DMA_NOT_INIT:' >>$destfile
echo '        case CTC_E_SYNCE_NOT_INIT:' >>$destfile
echo '        case CTC_E_INTR_NOT_INIT:' >>$destfile
echo '           return CTC_E_NOT_INIT;' >>$destfile
echo '        case CTC_E_INIT_FAIL:' >>$destfile
echo '        case CTC_E_SCL_INIT:' >>$destfile
echo '        case CTC_E_ACL_INIT:' >>$destfile
echo '        case CTC_E_DRV_FAIL:' >>$destfile
echo '           return CTC_E_INIT_FAIL;' >>$destfile
echo '        case CTC_E_NOT_READY:' >>$destfile
echo '        case CTC_E_QUEUE_SERVICE_QUEUE_NOT_INITIALIZED:' >>$destfile
echo '        case CTC_E_QUEUE_SERVICE_QUEUE_INITIALIZED:' >>$destfile
echo '        case CTC_E_OAM_MEP_LM_NOT_EN:' >>$destfile
echo '        case CTC_E_OAM_TRPT_SESSION_EN:' >>$destfile
echo '        case CTC_E_OAM_TRPT_SESSION_ALREADY_EN:' >>$destfile
echo '        case CTC_E_OAM_TRPT_SESSION_NOT_CFG:' >>$destfile
echo '        case CTC_E_PORT_ACL_EN_FIRST:' >>$destfile
echo '        case CTC_E_VLAN_CLASS_NOT_ENABLE:' >>$destfile
echo '        case CTC_E_VLAN_MAPPING_NOT_ENABLE:' >>$destfile
echo '        case CTC_E_VLAN_ACL_EN_FIRST:' >>$destfile
echo '        case CTC_E_L3IF_NO_ALLOCED_IPUCSA:' >>$destfile
echo '        case CTC_E_STATS_PORT_NOT_ENABLE:' >>$destfile
echo '        case CTC_E_STATS_NOT_ENABLE:' >>$destfile
echo '        case CTC_E_STATS_DMA_ENABLE_BUT_SW_DISABLE:' >>$destfile
echo '        case CTC_E_IPUC_VERSION_DISABLE:' >>$destfile
echo '        case CTC_E_IPUC_TUNNEL_INVALID:' >>$destfile
echo '        case CTC_E_IPMC_VERSION_DISABLE:' >>$destfile
echo '        case CTC_E_IPMC_RPF_CHECK_DISABLE:' >>$destfile
echo '        case CTC_E_USRID_DISABLE:' >>$destfile
echo '        case CTC_E_USRID_ALREADY_ENABLE:' >>$destfile
echo '        case CTC_E_SCL_DISABLE:' >>$destfile
echo '        case CTC_E_SCL_ALREADY_ENABLE:' >>$destfile
echo '        case CTC_E_SCL_NEED_DEBUG:' >>$destfile
echo '        case CTC_E_ACL_GROUP_NOT_EMPTY:' >>$destfile
echo '        case CTC_E_SCL_GROUP_NOT_EMPTY:' >>$destfile
echo '        case CTC_E_SCL_HAVE_ENABLED:' >>$destfile
echo '        case CTC_E_SCL_NOT_ENABLED:' >>$destfile
echo '        case CTC_E_ACLQOS_HAVE_ENABLED:' >>$destfile
echo '        case CTC_E_ACLQOS_NOT_ENABLED:' >>$destfile
echo '        case CTC_E_ACL_HAVE_ENABLED:' >>$destfile
echo '        case CTC_E_ACL_NOT_ENABLED:' >>$destfile
echo '        case CTC_E_QOS_POLICER_SERVICE_POLICER_NOT_ENABLE:' >>$destfile
echo '        case CTC_E_QOS_POLICER_STATS_NOT_ENABLE:' >>$destfile
echo '        case CTC_E_QOS_QUEUE_STATS_NOT_ENABLE:' >>$destfile
echo '        case CTC_E_SCL_ENTRY_INSTALLED:' >>$destfile
echo '        case CTC_E_SCL_GROUP_NOT_INSTALLED:' >>$destfile
echo '           return CTC_E_NOT_READY;' >>$destfile
echo '        case CTC_E_EXIST:' >>$destfile
echo '        case CTC_E_ENTRY_EXIST:' >>$destfile
echo '        case CTC_E_MEMBER_EXIST:' >>$destfile
echo '        case CTC_E_NH_EXIST:' >>$destfile
echo '        case CTC_E_NH_STATS_EXIST:' >>$destfile
echo '        case CTC_E_QUEUE_CHANNEL_SHAPE_PROF_EXIST:' >>$destfile
echo '        case CTC_E_QUEUE_SERVICE_EXIST:' >>$destfile
echo '        case CTC_E_APS_GROUP_EXIST:' >>$destfile
echo '        case CTC_E_APS_GROUP_NEXT_APS_EXIST:' >>$destfile
echo '        case CTC_E_OAM_MAID_ENTRY_EXIST:' >>$destfile
echo '        case CTC_E_OAM_CHAN_ENTRY_EXIST:' >>$destfile
echo '        case CTC_E_OAM_CHAN_LMEP_EXIST:' >>$destfile
echo '        case CTC_E_OAM_RMEP_EXIST:' >>$destfile
echo '        case CTC_E_OAM_NH_OFFSET_EXIST:' >>$destfile
echo '        case CTC_E_OAM_INDEX_EXIST:' >>$destfile
echo '        case CTC_E_MEMBER_PORT_EXIST:' >>$destfile
echo '        case CTC_E_ALREADY_PHY_IF_EXIST:' >>$destfile
echo '        case CTC_E_ALREADY_SUB_IF_EXIST:' >>$destfile
echo '        case CTC_E_MPLS_ENTRY_STATS_EXIST:' >>$destfile
echo '        case CTC_E_STK_TRUNK_HAS_EXIST:' >>$destfile
echo '        case CTC_E_STK_TRUNK_MEMBER_PORT_EXIST:' >>$destfile
echo '        case CTC_E_DATAPATH_PLL_EXIST_LANE:' >>$destfile
echo '        case CTC_E_VLAN_EXIST:' >>$destfile
echo '        case CTC_E_VLAN_RANGE_EXIST:' >>$destfile
echo '        case CTC_E_FDB_MAC_FILTERING_ENTRY_EXIST:' >>$destfile
echo '        case CTC_E_FDB_MCAST_ENTRY_EXIST:' >>$destfile
echo '        case CTC_E_FDB_KEY_ALREADY_EXIST:' >>$destfile
echo '        case CTC_E_L3IF_EXIST:' >>$destfile
echo '        case CTC_E_STATS_STATSID_EXIST:' >>$destfile
echo '        case CTC_E_IPMC_GROUP_HAS_EXISTED:' >>$destfile
echo '        case CTC_E_LINKAGG_HAS_EXIST:' >>$destfile
echo '        case CTC_E_SCL_GROUP_EXIST:' >>$destfile
echo '        case CTC_E_ACLQOS_ENTRY_EXIST:' >>$destfile
echo '        case CTC_E_ACL_ENTRY_EXIST:' >>$destfile
echo '        case CTC_E_ACL_GROUP_EXIST:' >>$destfile
echo '        case CTC_E_QOS_POLICER_CREATED:' >>$destfile
echo '           return CTC_E_EXIST;' >>$destfile
echo '        case CTC_E_NOT_EXIST:' >>$destfile
echo '        case CTC_E_QUEUE_CPU_REASON_NOT_CREATE:' >>$destfile
echo '        case CTC_E_ENTRY_NOT_EXIST:' >>$destfile
echo '        case CTC_E_MEMBER_NOT_EXIST:' >>$destfile
echo '        case CTC_E_NH_NOT_EXIST:' >>$destfile
echo '        case CTC_E_NH_STATS_NOT_EXIST:' >>$destfile
echo '        case CTC_E_NH_NOT_EXIST_TUNNEL_LABEL:' >>$destfile
echo '        case CTC_E_NH_ECMP_MEM_NOS_EXIST:' >>$destfile
echo '        case CTC_E_QUEUE_DROP_PROF_NOT_EXIST:' >>$destfile
echo '        case CTC_E_QUEUE_SHAPE_PROF_NOT_EXIST:' >>$destfile
echo '        case CTC_E_QUEUE_CHANNEL_SHAPE_PROF_NOT_EXIST:' >>$destfile
echo '        case CTC_E_QUEUE_SERVICE_NOT_EXIST:' >>$destfile
echo '        case CTC_E_QUEUE_SERVICE_GROUP_NOT_EXIST:' >>$destfile
echo '        case CTC_E_APS_GROUP_NOT_EXIST:' >>$destfile
echo '        case CTC_E_APS_RAPS_GROUP_NOT_EXIST:' >>$destfile
echo '        case CTC_E_OAM_MAID_ENTRY_NOT_FOUND:' >>$destfile
echo '        case CTC_E_OAM_CHAN_ENTRY_NOT_FOUND:' >>$destfile
echo '        case CTC_E_OAM_CHAN_MD_LEVEL_NOT_EXIST:' >>$destfile
echo '        case CTC_E_OAM_CHAN_LMEP_NOT_FOUND:' >>$destfile
echo '        case CTC_E_OAM_RMEP_NOT_FOUND:' >>$destfile
echo '        case CTC_E_OAM_NH_OFFSET_NOT_FOUND:' >>$destfile
echo '        case CTC_E_OVERLAY_TUNNEL_VN_FID_NOT_EXIST:' >>$destfile
echo '        case CTC_E_MEMBER_PORT_NOT_EXIST:' >>$destfile
echo '        case CTC_E_LOCAL_PORT_NOT_EXIST:' >>$destfile
echo '        case CTC_E_VPLS_VSI_NOT_CREATE:' >>$destfile
echo '        case CTC_E_MPLS_ENTRY_STATS_NOT_EXIST:' >>$destfile
echo '        case CTC_E_STK_TRUNK_NOT_EXIST:' >>$destfile
echo '        case CTC_E_STK_TRUNK_MEMBER_PORT_NOT_EXIST:' >>$destfile
echo '        case CTC_E_VLAN_NOT_CREATE:' >>$destfile
echo '        case CTC_E_VLAN_RANGE_NOT_EXIST:' >>$destfile
echo '        case CTC_E_FDB_DEFAULT_ENTRY_NOT_EXIST:' >>$destfile
echo '        case CTC_E_FDB_AD_INDEX_NOT_EXIST:' >>$destfile
echo '        case CTC_E_L3IF_VMAC_NOT_EXIST:' >>$destfile
echo '        case CTC_E_L3IF_NOT_EXIST:' >>$destfile
echo '        case CTC_E_L3IF_VRF_STATS_NOT_EXIST:' >>$destfile
echo '        case CTC_E_IPMC_GROUP_NOT_EXIST:' >>$destfile
echo '        case CTC_E_IPMC_STATS_NOT_EXIST:' >>$destfile
echo '        case CTC_E_LINKAGG_NOT_EXIST:' >>$destfile
echo '        case CTC_E_SCL_GROUP_UNEXIST:' >>$destfile
echo '        case CTC_E_SCL_LABEL_NOT_EXIST:' >>$destfile
echo '        case CTC_E_ACLQOS_ENTRY_NOT_EXIST:' >>$destfile
echo '        case CTC_E_ACLQOS_LABEL_NOT_EXIST:' >>$destfile
echo '        case CTC_E_QOS_POLICER_NOT_EXIST:' >>$destfile
echo '        case CTC_E_ACL_ENTRY_UNEXIST:' >>$destfile
echo '        case CTC_E_ACL_GROUP_UNEXIST:' >>$destfile
echo '        case CTC_E_ACL_LABEL_NOT_EXIST:' >>$destfile
echo '           return CTC_E_NOT_EXIST;' >>$destfile
echo '        case CTC_E_INVALID_CONFIG:' >>$destfile
echo '        case CTC_E_GLOBAL_CONFIG_CONFLICT:' >>$destfile
echo '        case CTC_E_MAC_NOT_USED:' >>$destfile
echo '        case CTC_E_NH_INVALID_DSEDIT_PTR:' >>$destfile
echo '        case CTC_E_NH_INVALID_NH_TYPE:' >>$destfile
echo '        case CTC_E_NH_INVALID_NH_SUB_TYPE:' >>$destfile
echo '        case CTC_E_NH_INVALID_VLAN_EDIT_TYPE:' >>$destfile
echo '        case CTC_E_NH_INVALID_DSNH_TYPE:' >>$destfile
echo '        case CTC_E_NH_VLAN_EDIT_CONFLICT:' >>$destfile
echo '        case CTC_E_NH_SHOULD_USE_DSNH8W:' >>$destfile
echo '        case CTC_E_NH_SHOULD_USE_DSNH4W:' >>$destfile
echo '        case CTC_E_NH_NHID_NOT_MATCH_NHTYPE:' >>$destfile
echo '        case CTC_E_NH_NO_LABEL:' >>$destfile
echo '        case CTC_E_NH_ISNT_UNROV:' >>$destfile
echo '        case CTC_E_NH_IS_UNROV:' >>$destfile
echo '        case CTC_E_NH_MEMBER_IS_UPMEP:' >>$destfile
echo '        case CTC_E_NH_EGS_EDIT_CONFLICT_VLAN_AND_MACDA_EDIT:' >>$destfile
echo '        case CTC_E_NH_LABEL_IS_MISMATCH_WITH_STATS:' >>$destfile
echo '        case CTC_E_NH_SUPPORT_PW_APS_UPDATE_BY_INT_ALLOC_DSNH:' >>$destfile
echo '        case CTC_E_QUEUE_DEPTH_NOT_EMPTY:' >>$destfile
echo '        case CTC_E_QUEUE_INVALID_CONFIG:' >>$destfile
echo '        case CTC_E_APS_INTERFACE_ERROR:' >>$destfile
echo '        case CTC_E_APS_MPLS_TYPE_L3IF_NOT_EXIST:' >>$destfile
echo '        case CTC_E_APS_DONT_SUPPORT_2_LEVEL_HW_BASED_APS:' >>$destfile
echo '        case CTC_E_OAM_CHAN_MD_LEVEL_EXIST:' >>$destfile
echo '        case CTC_E_OAM_CHAN_ENTRY_NUM_EXCEED:' >>$destfile
echo '        case CTC_E_OAM_LM_NUM_RXCEED:' >>$destfile
echo '        case CTC_E_OAM_CHAN_NOT_UP_DIRECTION:' >>$destfile
echo '        case CTC_E_OAM_CHAN_NOT_DOWN_DIRECTION:' >>$destfile
echo '        case CTC_E_OAM_TX_GPORT_AND_MASTER_GCHIP_NOT_MATCH:' >>$destfile
echo '        case CTC_E_OAM_TX_GPORT_AND_CHAN_GPORT_NOT_MATCH:' >>$destfile
echo '        case CTC_E_OAM_INVALID_CFG_IN_MEP_TYPE:' >>$destfile
echo '        case CTC_E_OAM_INVALID_CFG_LM:' >>$destfile
echo '        case CTC_E_OAM_NH_OFFSET_ADD_VECTOR_FAIL:' >>$destfile
echo '        case CTC_E_OAM_RMEP_D_LOC_PRESENT:' >>$destfile
echo '        case CTC_E_OAM_ITU_DEFECT_CLEAR_MODE:' >>$destfile
echo '        case CTC_E_OAM_INVALID_OPERATION_ON_CPU_MEP:' >>$destfile
echo '        case CTC_E_OAM_INVALID_OPERATION_ON_P2P_MEP:' >>$destfile
echo '        case CTC_E_OAM_INVALID_GLOBAL_PARAM_TYPE:' >>$destfile
echo '        case CTC_E_OAM_EXCEED_MAX_LEVEL_NUM_PER_CHAN:' >>$destfile
echo '        case CTC_E_OAM_TRPT_INTERVAL_CONFLICT:' >>$destfile
echo '        case CTC_E_BPE_INVALID_BPE_CAPABLE_GPORT:' >>$destfile
echo '        case CTC_E_BPE_INVLAID_BPE_MODE:' >>$destfile
echo '        case CTC_E_PORT_HAS_OTHER_FEATURE:' >>$destfile
echo '        case CTC_E_PORT_FEATURE_MISMATCH:' >>$destfile
echo '        case CTC_E_PORT_HAS_OTHER_RESTRICTION:' >>$destfile
echo '        case CTC_E_CHIP_DATAPATH_NOT_MATCH:' >>$destfile
echo '        case CTC_E_PORT_PROP_COLLISION:' >>$destfile
echo '        case CTC_E_PORT_MAC_IS_DISABLE:' >>$destfile
echo '        case CTC_E_MPLS_LABEL_ERROR:' >>$destfile
echo '        case CTC_E_STK_TRUNK_EXCEED_MAX_MEMBER_PORT:' >>$destfile
echo '        case CTC_E_STK_KEEPLIVE_TWO_MEMBER_AND_ONE_CPU:' >>$destfile
echo '        case CTC_E_DATAPATH_INVALID_PORT_TYPE:' >>$destfile
echo '        case CTC_E_DATAPATH_MAC_ENABLE:' >>$destfile
echo '        case CTC_E_INVALID_PORT_MAC_TYPE:' >>$destfile
echo '        case CTC_E_CANT_CHANGE_SERDES_MODE:' >>$destfile
echo '        case CTC_E_VLAN_RANGE_END_GREATER_START:' >>$destfile
echo '        case CTC_E_VLAN_MAPPING_GET_VLAN_RANGE_FAILED:' >>$destfile
echo '        case CTC_E_VLAN_MAPPING_VRANGE_OVERLAPPED:' >>$destfile
echo '        case CTC_E_VLAN_MAPPING_TAG_APPEND:' >>$destfile
echo '        case CTC_E_VLAN_MAPPING_TAG_SWAP:' >>$destfile
echo '        case CTC_E_FDB_L2MCAST_ADD_MEMBER_FAILED:' >>$destfile
echo '        case CTC_E_FDB_OPERATION_PAUSE:' >>$destfile
echo '        case CTC_E_FDB_HASH_REMOVE_FAILED:' >>$destfile
echo '        case CTC_E_FDB_DEFAULT_ENTRY_NOT_ALLOWED:' >>$destfile
echo '        case CTC_E_FDB_ONLY_SW_LEARN:' >>$destfile
echo '        case CTC_E_FDB_ONLY_HW_LEARN:' >>$destfile
echo '        case CTC_E_FDB_LOGIC_USE_HW_LEARN_DISABLE:' >>$destfile
echo '        case CTC_E_VLAN_FILTER_PORT:' >>$destfile
echo '        case CTC_E_FDB_L2MCAST_MEMBER_INVALID:' >>$destfile
echo '        case CTC_E_L3IF_MISMATCH:' >>$destfile
echo '        case CTC_E_STATS_PORT_STATS_NO_TYPE:' >>$destfile
echo '        case CTC_E_STATS_PHB_STATS_INVALID:' >>$destfile
echo '        case CTC_E_STATS_PORT_NOT_MAP_TO_MAC:' >>$destfile
echo '        case CTC_E_STATS_MAC_STATS_MODE_ASIC:' >>$destfile
echo '        case CTC_E_STATS_TABLE_NUM_NO_EXPECT:' >>$destfile
echo '        case CTC_E_STATS_VLAN_STATS_CONFLICT_WITH_PHB:' >>$destfile
echo '        case CTC_E_STATS_STATSID_TYPE_MISMATCH:' >>$destfile
echo '        case CTC_E_IPUC_NEED_L3IF:' >>$destfile
echo '        case CTC_E_IPUC_RPF_NOT_SUPPORT_THIS_MASK_LEN:' >>$destfile
echo '        case CTC_E_IPUC_NAT_NOT_SUPPORT_THIS_MASK_LEN:' >>$destfile
echo '        case CTC_E_IPUC_TUNNEL_RPF_INFO_MISMATCH:' >>$destfile
echo '        case CTC_E_IPUC_TUNNEL_PAYLOAD_TYPE_MISMATCH:' >>$destfile
echo '        case CTC_E_IPUC_INVALID_ROUTE_FLAG:' >>$destfile
echo '        case CTC_E_IPMC_ADD_MEMBER_FAILED:' >>$destfile
echo '        case CTC_E_IPMC_GROUP_IS_NOT_IP_MAC:' >>$destfile
echo '        case CTC_E_IPMC_GROUP_IS_IP_MAC:' >>$destfile
echo '        case CTC_E_SCL_HASH_CONVERT_FAILED:' >>$destfile
echo '        case CTC_E_SCL_GET_KEY_FAILED:' >>$destfile
echo '        case CTC_E_SCL_KEY_AD_EXIST:' >>$destfile
echo '        case CTC_E_SCL_BUILD_VLAN_ACTION_INDEX_FAILED:' >>$destfile
echo '        case CTC_E_SCL_CANNOT_OVERWRITE:' >>$destfile
echo '        case CTC_E_SCL_LKP_FAILED:' >>$destfile
echo '        case CTC_E_SCL_STATIC_ENTRY:' >>$destfile
echo '        case CTC_E_SCL_UPDATE_FAILED:' >>$destfile
echo '        case CTC_E_SCL_FLAG_COLLISION:' >>$destfile
echo '        case CTC_E_ACL_FLAG_COLLISION:' >>$destfile
echo '        case CTC_E_SCL_GET_NODES_FAILED:' >>$destfile
echo '        case CTC_E_SCL_GROUP_INFO:' >>$destfile
echo '        case CTC_E_SCL_ADD_TCAM_ENTRY_TO_WRONG_GROUP:' >>$destfile
echo '        case CTC_E_SCL_ADD_HASH_ENTRY_TO_WRONG_GROUP:' >>$destfile
echo '        case CTC_E_SCL_HASH_ENTRY_NO_PRIORITY:' >>$destfile
echo '        case CTC_E_SCL_KEY_ACTION_TYPE_NOT_MATCH:' >>$destfile
echo '        case CTC_E_QOS_POLICER_CIR_GREATER_THAN_PIR:' >>$destfile
echo '        case CTC_E_QOS_POLICER_CBS_GREATER_THAN_PBS:' >>$destfile
echo '        case CTC_E_ACLQOS_DIFFERENT_DIR:' >>$destfile
echo '        case CTC_E_ACLQOS_DIFFERENT_TYPE:' >>$destfile
echo '        case CTC_E_ACLQOS_DIFFERENT_CHIP:' >>$destfile
echo '        case CTC_E_PTP_TS_GET_BUFF_FAIL:' >>$destfile
echo '        case CTC_E_PTP_CAN_NOT_SET_TOW:' >>$destfile
echo '        case CTC_E_SYNCE_INVALID_RECOVERED_PORT:' >>$destfile
echo '        case CTC_E_QOS_NO_INT_POLICER_ENTRY:' >>$destfile
echo '        case CTC_E_QOS_NO_EXT_POLICER_ENTRY:' >>$destfile
echo '        case CTC_E_QOS_NO_INT_POLICER_PROFILE_ENTRY:' >>$destfile
echo '        case CTC_E_ACL_PBR_ENTRY_NO_NXTTHOP:' >>$destfile
echo '        case CTC_E_ACL_GET_NODES_FAILED:' >>$destfile
echo '        case CTC_E_SCL_GLOBAL_CFG_ERROR:' >>$destfile
echo '        case CTC_E_QOS_POLICER_NOT_BIND:' >>$destfile
echo '        case CTC_E_ACL_GROUP_INFO:' >>$destfile
echo '        case CTC_E_ACL_ENTRY_INSTALLED:' >>$destfile
echo '        case CTC_E_ACL_GET_STATS_FAILED:' >>$destfile
echo '        case CTC_E_ACL_GROUP_NOT_INSTALLED:' >>$destfile
echo '        case CTC_E_ACL_GLOBAL_CFG_ERROR:' >>$destfile
echo '        case CTC_E_ACL_ADD_TCAM_ENTRY_TO_WRONG_GROUP:' >>$destfile
echo '        case CTC_E_ACL_ADD_HASH_ENTRY_TO_WRONG_GROUP:' >>$destfile
echo '        case CTC_E_ACL_HASH_ENTRY_NO_PRIORITY:' >>$destfile
echo '        case CTC_E_IPUC_ADD_FAILED:' >>$destfile
echo '        case CTC_E_SCL_GET_BLOCK_INDEX_FAILED:' >>$destfile
echo '        case CTC_E_SCL_GET_STATS_FAILED:' >>$destfile
echo '        case CTC_E_ACL_PBR_ECMP_CMP_FAILED:' >>$destfile
echo '        case CTC_E_ACL_PBR_DO_ECMP_FAILED:' >>$destfile
echo '        case CTC_E_MUTEX_BUSY:' >>$destfile
echo '           return CTC_E_INVALID_CONFIG;' >>$destfile
echo '        default:' >>$destfile
echo '            return CTC_E_INVALID_PARAM;' >>$destfile
echo '    }' >>$destfile
echo '' >>$destfile
echo '}' >>$destfile
echo '' >>$destfile
echo 'int32' >>$destfile
echo 'ctc_set_error_code_mapping_en(uint8 enable)' >>$destfile
echo '{' >>$destfile
echo '    g_mapping_disable = !enable;' >>$destfile
echo '    return CTC_E_NONE;' >>$destfile
echo '}' >>$destfile


