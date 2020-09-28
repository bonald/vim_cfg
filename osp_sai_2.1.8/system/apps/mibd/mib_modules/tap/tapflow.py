#!/usr/bin/python

import sys, os, signal, time, re, optparse, pprint
import binascii, xdrlib, threading, socket
from daemon3x import daemon as Daemon
from struct import *
from netsnmpapi import *
from netsnmpagent import *
from cdb_const import*
from cdb_define import*
import tbl_pb2, common_pb2, ds_pb2

AGENT = None
################################################################################
################################################################################
#
# tapflow enterprise.111
#
################################################################################
################################################################################

################################################################################
#
# flowTable enterprise.111.1
#
################################################################################
# Oid indexes
FLOW_TBL_MEM_NAME        = 1
FLOW_TBL_MEM_DECAP       = 2
FLOW_TBL_MEM_ROWSTATUS   = 10
FLOW_TBL_MEM_DESCRIPTION = 11

# Macro definitions
FLOW_NAME_LEN = 20
FLOW_DESCRIPTION_LEN = 100

FLOW_TABLE = None
class FlowTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(FlowTableHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        # Index check
        indexes = self.get_request_indexes(FLOW_TABLE)
        flow_name = indexes[0]
        if len(flow_name) > FLOW_NAME_LEN:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        match = re.search("^[a-zA-Z][0-9a-zA-Z._-]*$", flow_name[0:len(flow_name)])
        if not match:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        if len(match.group(0)) != len(flow_name):
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        # Table Nodes check
        if colnum == FLOW_TBL_MEM_DECAP:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != GLB_SNMP_TRUE and data != GLB_SNMP_FALSE:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_TBL_MEM_ROWSTATUS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)

            rowstatus = data
            if rowstatus != SNMP_ROW_CREATEANDGO and rowstatus != SNMP_ROW_DESTROY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_TBL_MEM_DESCRIPTION:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)

            description = data
            if description == "":
                return SNMP_ERR_NOERROR

            if len(description) > FLOW_DESCRIPTION_LEN:
                return self.set_request_error(SNMP_ERR_WRONGLENGTH)

            if len(re.search("^[0-9a-zA-Z._-]*", description[0:len(description)]).group(0)) != len(description):
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        indexes = self.get_request_indexes(FLOW_TABLE)
        flow_name = indexes[0]
        old_row_status = None

        # Retrieve exsting row
        old_row = FLOW_TABLE.getRow([AGENT.DisplayString(flow_name)])
        if old_row:
            old_row_status = self.get_table_row(FLOW_TBL_MEM_ROWSTATUS)
            
 
        if colnum == FLOW_TBL_MEM_ROWSTATUS and data == SNMP_ROW_DESTROY:
            if old_row_status == SNMP_ROW_ACTIVE:
                szmsg = execute_cmd('cdbctl delete/cdb/l2/acl_config/{0}/type/flow'.format(flow_name))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    increase_data_sync_del("FLOW_TABLE", flow_name)

        if colnum == FLOW_TBL_MEM_ROWSTATUS and data == SNMP_ROW_CREATEANDGO:
            decap_status = self.get_table_row(FLOW_TBL_MEM_DECAP)
            if GLB_SNMP_TRUE == decap_status:
                szmsg = execute_cmd('cdbctl create/cdb/l2/acl_config/{0}/type/flow/flow_ace_type/decap'.format(flow_name))
            else:
                szmsg = execute_cmd('cdbctl create/cdb/l2/acl_config/{0}/type/flow'.format(flow_name))
            if szmsg.find('%') != -1:
                return self.set_request_error(SNMP_ERR_GENERR)
            else:
                increase_data_sync_add("FLOW_TABLE", flow_name)

        if old_row and colnum == FLOW_TBL_MEM_DESCRIPTION:
            description = data
            if description != "":
                szmsg = execute_cmd('cdbctl update/cdb/l2/acl_config/{0}/remark/add/{1}/type/flow'.format(flow_name, description))
            else:
                szmsg = execute_cmd('cdbctl update/cdb/l2/acl_config/{0}/remark/del/type/flow'.format(flow_name))
            if szmsg.find('%') != -1:
                   return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_acl_config = tbl_pb2.tbl_acl_config()
        try:
            tbl_acl_config.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_acl fail'
            return
            
        if tbl_acl_config.key.name[:6] != "#FLOW#":
            return
        else:
            flow_name = tbl_acl_config.key.name[6:]

        if operation == CDB_OPER_ADD:
            
            if check_data_sync_add("FLOW_TABLE", flow_name):
                decrease_data_sync_add("FLOW_TABLE", flow_name)
                return
            new_row = FLOW_TABLE.addRow([AGENT.DisplayString(flow_name)])
            if tbl_acl_config.decap_en != 0:
                new_row.setRowCell(FLOW_TBL_MEM_DECAP, AGENT.Integer32(GLB_SNMP_TRUE))
            else:
                new_row.setRowCell(FLOW_TBL_MEM_DECAP, AGENT.Integer32(GLB_SNMP_FALSE))
            new_row.setRowCell(FLOW_TBL_MEM_ROWSTATUS,   AGENT.Integer32(SNMP_ROW_ACTIVE))
            new_row.setRowCell(FLOW_TBL_MEM_DESCRIPTION, AGENT.DisplayString(tbl_acl_config.remark))

        if operation == CDB_OPER_DEL:
            if check_data_sync_del("FLOW_TABLE", flow_name):
                decrease_data_sync_del("FLOW_TABLE", flow_name)
                return
            FLOW_TABLE.delRow([AGENT.DisplayString(flow_name)])

        if operation == CDB_OPER_SET:
            if field_id == TBL_ACL_CONFIG_FLD_REMARK:
                old_row = FLOW_TABLE.addRow([AGENT.DisplayString(flow_name)])
                old_row.setRowCell(FLOW_TBL_MEM_DESCRIPTION, AGENT.DisplayString(tbl_acl_config.remark))

################################################################################
#
# flowRuleTable enterprise.111.2
#
################################################################################
# Oid indexes
FLOW_RULE_TBL_MEM_FLOWNAME      = 1
FLOW_RULE_TBL_MEM_INDEX         = 2
FLOW_RULE_TBL_MEM_ACT           = 3
FLOW_RULE_TBL_MEM_IPPROTOCOL    = 4
FLOW_RULE_TBL_MEM_TCPSRCOP      = 5
FLOW_RULE_TBL_MEM_TCPSRCPORT1   = 6
FLOW_RULE_TBL_MEM_TCPSRCPORT2   = 7
FLOW_RULE_TBL_MEM_TCPDESTOP     = 8
FLOW_RULE_TBL_MEM_TCPDESTPORT1  = 9
FLOW_RULE_TBL_MEM_TCPDESTPORT2  = 10
FLOW_RULE_TBL_MEM_TCPFLAGOP     = 11
FLOW_RULE_TBL_MEM_TCPFLAGURG    = 12
FLOW_RULE_TBL_MEM_TCPFLAGACK    = 13
FLOW_RULE_TBL_MEM_TCPFLAGPSH    = 14
FLOW_RULE_TBL_MEM_TCPFLAGRST    = 15
FLOW_RULE_TBL_MEM_TCPFLAGSYN    = 16
FLOW_RULE_TBL_MEM_TCPFLAGFIN    = 17
FLOW_RULE_TBL_MEM_UDPSRCOP      = 18
FLOW_RULE_TBL_MEM_UDPSRCPORT1   = 19
FLOW_RULE_TBL_MEM_UDPSRCPORT2   = 20
FLOW_RULE_TBL_MEM_UDPDESTOP     = 21
FLOW_RULE_TBL_MEM_UDPDESTPORT1  = 22
FLOW_RULE_TBL_MEM_UDPDESTPORT2  = 23
FLOW_RULE_TBL_MEM_SRCIP         = 24
FLOW_RULE_TBL_MEM_SRCWILD       = 25
FLOW_RULE_TBL_MEM_DESTIP        = 26
FLOW_RULE_TBL_MEM_DESTWILD      = 27
FLOW_RULE_TBL_MEM_IPPRECEDENCE  = 28
FLOW_RULE_TBL_MEM_IPDSCP        = 29
FLOW_RULE_TBL_MEM_IPFRAGMENTS   = 30
FLOW_RULE_TBL_MEM_IPOPTIONS     = 31
FLOW_RULE_TBL_MEM_TRUNCATION    = 32
FLOW_RULE_TBL_MEM_VLANID        = 33
FLOW_RULE_TBL_MEM_VLANID_WILD   = 34
FLOW_RULE_TBL_MEM_INNERVLANID   = 35
FLOW_RULE_TBL_MEM_INNERVLAN_WILD  = 36
FLOW_RULE_TBL_MEM_COS           = 37
FLOW_RULE_TBL_MEM_INNERCOS      = 38
FLOW_RULE_TBL_MEM_ETHERTYPE     = 39
FLOW_RULE_TBL_MEM_ETHERTYPEWILD = 40
FLOW_RULE_TBL_MEM_SRCMAC        = 41
FLOW_RULE_TBL_MEM_SRCMACWILD    = 42
FLOW_RULE_TBL_MEM_DESTMAC       = 43
FLOW_RULE_TBL_MEM_DESTMACWILD   = 44
FLOW_RULE_TBL_MEM_EDIT_MAC_DA   = 45
FLOW_RULE_TBL_MEM_EDIT_MAC_SA   = 46
FLOW_RULE_TBL_MEM_EDIT_IP_DA_EN = 47
FLOW_RULE_TBL_MEM_EDIT_IP_DA    = 48
FLOW_RULE_TBL_MEM_EDIT_IP_SA_EN = 49
FLOW_RULE_TBL_MEM_EDIT_IP_SA    = 50
FLOW_RULE_TBL_MEM_STRIP_HEADER  = 51
FLOW_RULE_TBL_MEM_VNI           = 52
FLOW_RULE_TBL_MEM_VXLAN_VNI     = 53
FLOW_RULE_TBL_MEM_VXLAN_VNI_MASK= 54
FLOW_RULE_TBL_MEM_UNTAG         = 55
FLOW_RULE_TBL_MEM_MARK_SRC_VLAN = 56
FLOW_RULE_TBL_MEM_UDF_TYPE      = 57
FLOW_RULE_TBL_MEM_UDF_VALUE     = 58
FLOW_RULE_TBL_MEM_UDF_MASK      = 59
FLOW_RULE_TBL_MEM_UDF_OFFSET    = 60
FLOW_RULE_TBL_MEM_IS_NVGRE      = 61
FLOW_RULE_TBL_MEM_GRE_KEY       = 62
FLOW_RULE_TBL_MEM_GRE_KEY_MASK  = 63
FLOW_RULE_TBL_MEM_EDIT_VLAN_EN = 64
FLOW_RULE_TBL_MEM_EDIT_VLAN    = 65
FLOW_RULE_TBL_MEM_INNER_MATCH_NAME = 66
FLOW_RULE_TBL_MEM_ROWSTATUS     = 70

# Macro definitions
FLOW_MAC_SA_FLAG = (1<<0)
FLOW_MAC_DA_FLAG = (1<<1)
FLOW_UNTAG_FLAG = (1<<2)
FLOW_VLAN_FLAG = (1<<3)
FLOW_INNER_VLAN_FLAG = (1<<4)
FLOW_ETHERTYPE_FLAG = (1<<5)
FLOW_IPSA_FLAG = (1<<6)
FLOW_IPDA_FLAG = (1<<7)
FLOW_IP_PROT_FLAG = (1<<8)
FLOW_SRC_PORT_FLAG = (1<<9)
FLOW_DST_PORT_FLAG = (1<<10)
FLOW_COS_FLAG = (1<<11)
FLOW_INNER_COS_FLAG = (1<<12)
FLOW_DSCP_FLAG = (1<<13)
FLOW_IP_FRAG_FLAG = (1<<14)
FLOW_IGMP_TYPE_FLAG = (1<<15)
FLOW_ICMP_TYPE_FLAG = (1<<16)
FLOW_ICMP_CODE_FLAG = (1<<17)
FLOW_TCP_FLAG = (1<<18)
FLOW_IP_PRECEDENCE_FLAG = (1<<19)
FLOW_TIME_RANGE_FLAG = (1<<20)
FLOW_INVALID_FLAG = (1<<21)
FLOW_TIMER_ACTIVE_FLAG = (1<<22)
FLOW_IP_OPTIONS_FLAG = (1<<23)
FLOW_TAP_ACTION_FLAG = (1<<24)
FLOW_TAP_TRUNCATION_FLAG = (1<<25)
FLOW_TAP_VXLAN_VNI_FLAG = (1<<26)
FLOW_TAP_UNTAG_FLAG = (1<<27)
FLOW_TAP_MARK_SOURCE_FLAG = (1<<28)
FLOW_TAP_FLOW_MATCH_FLAG = (1<<29)
FLOW_TAP_GRE_KEY_FLAG = (1<<30)
FLOW_TAP_UDF_FLAG = (1<<31)

FLOW_TAP_STR_HED_POS_FLAG = (1<<0)
FLOW_TAP_STR_HED_OFFSET_FLAG = (1<<1)

FLOW_RULE_INDEX_MIN = 1
FLOW_RULE_INDEX_MAX = 65535

FLOW_RULE_IP_PROTOCOL_MIN = 0
FLOW_RULE_IP_PROTOCOL_MAX = 255
FLOW_RULE_IP_PROTOCOL_ANY = 256

FLOW_RULE_L4_PORT_MIN = 0
FLOW_RULE_L4_PORT_MAX = 65535

FLOW_RULE_IPPRECEDENCE_MIN = 0
FLOW_RULE_IPPRECEDENCE_MAX = 8

FLOW_RULE_IPDSCP_MIN = 0
FLOW_RULE_IPDSCP_MAX = 64

FLOW_RULE_IPFRAGMENTS_NONE         = 0
FLOW_RULE_IPFRAGMENTS_FIRST        = 1
FLOW_RULE_IPFRAGMENTS_NON_OR_FIRST = 2
FLOW_RULE_IPFRAGMENTS_SMALL        = 3
FLOW_RULE_IPFRAGMENTS_NON_FIRST    = 4
FLOW_RULE_IPFRAGMENTS_ANY          = 5
FLOW_RULE_IPFRAGMENTS_NOUSE        = 6

FLOW_RULE_VLANID_MIN = 0
FLOW_RULE_VLANID_MAX = 4094

FLOW_RULE_COS_MIN = 0
FLOW_RULE_COS_MAX = 8

FLOW_RULE_VIN_MIN = 0
FLOW_RULE_VIN_MAX = 0xFFFFFF

FLOW_RULE_EHTER_TYPE_LEN_MAX = 6
FLOW_RULE_VNI_MASK_LEN_MAX = 8

FLOW_RULE_TABLE = None
class FlowRuleTableHandler(SnmpTableBase):
    def __init__(self, agentx):
        super(FlowRuleTableHandler, self).__init__(agentx)

    def check(self, colnum, var_type, data):
        # Index check
        indexes = self.get_request_indexes(FLOW_RULE_TABLE)
        flow_name = indexes[0]
        seq_no =  indexes[1]
        if len(flow_name) > FLOW_NAME_LEN:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        match = re.search("^[a-zA-Z][0-9a-zA-Z._-]*$", flow_name[0:len(flow_name)])
        if not match:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        if len(match.group(0)) != len(flow_name):
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        if seq_no < FLOW_RULE_INDEX_MIN or seq_no > FLOW_RULE_INDEX_MAX:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        old_row_status = None
        old_row = FLOW_RULE_TABLE.getRow([AGENT.DisplayString(flow_name), AGENT.Integer32(seq_no)])
        if old_row:
            old_row_status = self.get_table_row(FLOW_RULE_TBL_MEM_ROWSTATUS)
        if colnum < FLOW_RULE_TBL_MEM_ROWSTATUS and old_row_status == SNMP_ROW_ACTIVE:
            #print "colnum=  ", colnum , "  old_row_status == SNMP_ROW_ACTIVE"
            return self.set_request_error(SNMP_ERR_NOTWRITABLE)

        if colnum == FLOW_RULE_TBL_MEM_ACT:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < GLB_ACL_PERMIT or data > GLB_ACL_DENY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_IPPROTOCOL:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < FLOW_RULE_IP_PROTOCOL_MIN or data > FLOW_RULE_IP_PROTOCOL_ANY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_TCPSRCOP or colnum == FLOW_RULE_TBL_MEM_TCPDESTOP or\
                colnum == FLOW_RULE_TBL_MEM_UDPSRCOP or colnum == FLOW_RULE_TBL_MEM_UDPDESTOP:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != GLB_ACL_L4_MATCH_TYPE_EQUAL and data != GLB_ACL_L4_MATCH_TYPE_GREATER_THAN and\
                    data != GLB_ACL_L4_MATCH_TYPE_LESS_THAN and data != GLB_ACL_L4_MATCH_TYPE_RANGE and\
                    data != GLB_ACL_L4_MATCH_TYPE_INVALID and data != GLB_ACL_L4_MATCH_TYPE_ANY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_TCPSRCPORT1 or colnum == FLOW_RULE_TBL_MEM_TCPSRCPORT2 or\
                colnum == FLOW_RULE_TBL_MEM_TCPDESTPORT1 or colnum == FLOW_RULE_TBL_MEM_TCPDESTPORT2 or\
                colnum == FLOW_RULE_TBL_MEM_UDPDESTPORT1 or colnum == FLOW_RULE_TBL_MEM_UDPDESTPORT2 or\
                colnum == FLOW_RULE_TBL_MEM_UDPSRCPORT1 or colnum == FLOW_RULE_TBL_MEM_UDPSRCPORT2:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < FLOW_RULE_L4_PORT_MIN or data > FLOW_RULE_L4_PORT_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_TCPFLAGOP:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != GLB_ACL_MATCH_TYPE_ANY and data != GLB_ACL_MATCH_TYPE_ALL and\
                    data != GLB_ACL_MATCH_TYPE_INVALID:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_TCPFLAGURG or colnum == FLOW_RULE_TBL_MEM_TCPFLAGACK or\
                colnum == FLOW_RULE_TBL_MEM_TCPFLAGPSH or colnum == FLOW_RULE_TBL_MEM_TCPFLAGRST or\
                colnum == FLOW_RULE_TBL_MEM_TCPFLAGSYN or colnum == FLOW_RULE_TBL_MEM_TCPFLAGFIN or\
                colnum == FLOW_RULE_TBL_MEM_IPOPTIONS or colnum == FLOW_RULE_TBL_MEM_TRUNCATION:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != GLB_SNMP_TRUE and data != GLB_SNMP_FALSE:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_SRCIP or colnum == FLOW_RULE_TBL_MEM_SRCWILD or\
            colnum == FLOW_RULE_TBL_MEM_DESTIP or colnum == FLOW_RULE_TBL_MEM_DESTWILD or \
            colnum == FLOW_RULE_TBL_MEM_EDIT_IP_DA or colnum == FLOW_RULE_TBL_MEM_EDIT_IP_SA :
            if var_type != ASN_IPADDRESS:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)

        if colnum == FLOW_RULE_TBL_MEM_IPPRECEDENCE:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < FLOW_RULE_IPPRECEDENCE_MIN or data > FLOW_RULE_IPPRECEDENCE_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_IPDSCP:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < FLOW_RULE_IPDSCP_MIN or data > FLOW_RULE_IPDSCP_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_IPFRAGMENTS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < FLOW_RULE_IPFRAGMENTS_NONE or data > FLOW_RULE_IPFRAGMENTS_NOUSE:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_MARK_SRC_VLAN:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < FLOW_RULE_VLANID_MIN or data > FLOW_RULE_VLANID_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
                
        if colnum == FLOW_RULE_TBL_MEM_VLANID or colnum == FLOW_RULE_TBL_MEM_INNERVLANID :
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < FLOW_RULE_VLANID_MIN or data > GLB_ACL_VLAN_ID_INVALID:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
                
        if colnum == FLOW_RULE_TBL_MEM_VLANID_WILD or colnum == FLOW_RULE_TBL_MEM_INNERVLAN_WILD:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if len(data) > FLOW_RULE_EHTER_TYPE_LEN_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            match = re.search("^0x[0-9a-fA-F]{1,3}$", data)
            if not match:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            if len(match.group(0)) != len(data):
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_COS or colnum == FLOW_RULE_TBL_MEM_INNERCOS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < FLOW_RULE_COS_MIN or data > FLOW_RULE_COS_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_ETHERTYPE or colnum == FLOW_RULE_TBL_MEM_ETHERTYPEWILD:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if len(data) > FLOW_RULE_EHTER_TYPE_LEN_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            match = re.search("^0x[0-9a-fA-F]{1,4}$", data)
            if not match:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            if len(match.group(0)) != len(data):
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_SRCMAC or colnum == FLOW_RULE_TBL_MEM_SRCMACWILD or\
            colnum == FLOW_RULE_TBL_MEM_DESTMAC or colnum == FLOW_RULE_TBL_MEM_DESTMACWILD or\
            colnum ==  FLOW_RULE_TBL_MEM_EDIT_MAC_DA or colnum ==  FLOW_RULE_TBL_MEM_EDIT_MAC_SA:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if len(data) != GLB_SNMP_MAC_ADDR_LEN:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_UNTAG or colnum == FLOW_RULE_TBL_MEM_STRIP_HEADER or \
            colnum == FLOW_RULE_TBL_MEM_VNI or colnum == FLOW_RULE_TBL_MEM_EDIT_IP_DA_EN or \
            colnum == FLOW_RULE_TBL_MEM_EDIT_IP_SA_EN:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != GLB_SNMP_TRUE and data != GLB_SNMP_FALSE:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_VXLAN_VNI:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < FLOW_RULE_VIN_MIN and data > FLOW_RULE_VIN_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_VXLAN_VNI_MASK:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if len(data) > FLOW_RULE_VNI_MASK_LEN_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            match = re.search("^0x[0-9a-fA-F]{1,6}$", data)
            if not match:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            if len(match.group(0)) != len(data):
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == FLOW_RULE_TBL_MEM_ROWSTATUS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != SNMP_ROW_CREATEANDGO and data != SNMP_ROW_DESTROY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            if data == SNMP_ROW_CREATEANDGO and old_row_status == SNMP_ROW_ACTIVE:
                return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
            if data == SNMP_ROW_DESTROY and not old_row_status:
                return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        indexes = self.get_request_indexes(FLOW_RULE_TABLE)
        flow_name = indexes[0]
        seq_no = indexes[1]
        old_row_status = None

        szmsg = execute_cmd('cdbctl read/cdb/l2/acl_config/')
        if szmsg.find('/key=#FLOW#{0}/'.format(indexes[0])) == -1:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        old_row = FLOW_RULE_TABLE.getRow([AGENT.DisplayString(flow_name), AGENT.Integer32(seq_no)])
        if old_row:
            old_row_status = self.get_table_row(FLOW_RULE_TBL_MEM_ROWSTATUS)

        if colnum == FLOW_RULE_TBL_MEM_ROWSTATUS and data == SNMP_ROW_DESTROY:
            if old_row_status == SNMP_ROW_ACTIVE:
                cmd = "cdbctl delete/cdb/l2/ace_config/{0}/{1}/snmp_set/true/type/flow".format(flow_name, seq_no)
                szmsg = execute_cmd(cmd)
                if szmsg.find('%') != -1:
                    print "set del flow fail szmsg = ", szmsg
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    tmp = flow_name + str(seq_no)
                    print "set FLOW_RULE_TABLE sub_del tmp = ", tmp
                    increase_data_sync_del("FLOW_RULE_TABLE", tmp)

        if colnum == FLOW_RULE_TBL_MEM_ROWSTATUS and data == SNMP_ROW_CREATEANDGO:
            #print "1. colnum == FLOW_RULE_TBL_MEM_ROWSTATUS data == SNMP_ROW_CREATEANDGO, old_row_status = ",old_row_status
            #if old_row:
            #    if old_row_status == SNMP_ROW_ACTIVE:
            #        return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)

            act             = self.get_table_row(FLOW_RULE_TBL_MEM_ACT)
            ip_proto        = self.get_table_row(FLOW_RULE_TBL_MEM_IPPROTOCOL)
            tcp_src_op      = self.get_table_row(FLOW_RULE_TBL_MEM_TCPSRCOP)
            tcp_src_port1   = self.get_table_row(FLOW_RULE_TBL_MEM_TCPSRCPORT1)
            tcp_src_port2   = self.get_table_row(FLOW_RULE_TBL_MEM_TCPSRCPORT2)
            tcp_dst_op      = self.get_table_row(FLOW_RULE_TBL_MEM_TCPDESTOP)
            tcp_dst_port1   = self.get_table_row(FLOW_RULE_TBL_MEM_TCPDESTPORT1)
            tcp_dst_port2   = self.get_table_row(FLOW_RULE_TBL_MEM_TCPDESTPORT2)
            tcp_flag_op     = self.get_table_row(FLOW_RULE_TBL_MEM_TCPFLAGOP)
            tcp_flag_urg    = self.get_table_row(FLOW_RULE_TBL_MEM_TCPFLAGURG)
            tcp_flag_ack    = self.get_table_row(FLOW_RULE_TBL_MEM_TCPFLAGACK)
            tcp_flag_psh    = self.get_table_row(FLOW_RULE_TBL_MEM_TCPFLAGPSH)
            tcp_flag_rst    = self.get_table_row(FLOW_RULE_TBL_MEM_TCPFLAGRST)
            tcp_flag_syn    = self.get_table_row(FLOW_RULE_TBL_MEM_TCPFLAGSYN)
            tcp_flag_fin    = self.get_table_row(FLOW_RULE_TBL_MEM_TCPFLAGFIN)
            udp_src_op      = self.get_table_row(FLOW_RULE_TBL_MEM_UDPSRCOP)
            udp_src_port1   = self.get_table_row(FLOW_RULE_TBL_MEM_UDPSRCPORT1)
            udp_src_port2   = self.get_table_row(FLOW_RULE_TBL_MEM_UDPSRCPORT2)
            udp_dst_op      = self.get_table_row(FLOW_RULE_TBL_MEM_UDPDESTOP)
            udp_dst_port1   = self.get_table_row(FLOW_RULE_TBL_MEM_UDPDESTPORT1)
            udp_dst_port2   = self.get_table_row(FLOW_RULE_TBL_MEM_UDPDESTPORT2)
            src_ip          = self.get_table_row(FLOW_RULE_TBL_MEM_SRCIP)
            src_ip_wild     = self.get_table_row(FLOW_RULE_TBL_MEM_SRCWILD)
            dst_ip          = self.get_table_row(FLOW_RULE_TBL_MEM_DESTIP)
            dst_ip_wild     = self.get_table_row(FLOW_RULE_TBL_MEM_DESTWILD)
            ip_prec         = self.get_table_row(FLOW_RULE_TBL_MEM_IPPRECEDENCE)
            ip_dscp         = self.get_table_row(FLOW_RULE_TBL_MEM_IPDSCP)
            ip_frag         = self.get_table_row(FLOW_RULE_TBL_MEM_IPFRAGMENTS)
            ip_opt          = self.get_table_row(FLOW_RULE_TBL_MEM_IPOPTIONS)
            truncation      = self.get_table_row(FLOW_RULE_TBL_MEM_TRUNCATION)
            svlan_id        = self.get_table_row(FLOW_RULE_TBL_MEM_VLANID)
            svlan_id_wild   = self.get_table_row(FLOW_RULE_TBL_MEM_VLANID_WILD)
            cvlan_id        = self.get_table_row(FLOW_RULE_TBL_MEM_INNERVLANID)
            cvlan_id_wild   = self.get_table_row(FLOW_RULE_TBL_MEM_INNERVLAN_WILD)
            scos            = self.get_table_row(FLOW_RULE_TBL_MEM_COS)
            ccos            = self.get_table_row(FLOW_RULE_TBL_MEM_INNERCOS)
            ether_type      = self.get_table_row(FLOW_RULE_TBL_MEM_ETHERTYPE)
            ether_type_wild = self.get_table_row(FLOW_RULE_TBL_MEM_ETHERTYPEWILD)
            src_mac         = self.get_table_row(FLOW_RULE_TBL_MEM_SRCMAC)
            src_mac_wild    = self.get_table_row(FLOW_RULE_TBL_MEM_SRCMACWILD)
            dst_mac         = self.get_table_row(FLOW_RULE_TBL_MEM_DESTMAC)
            dst_mac_wild    = self.get_table_row(FLOW_RULE_TBL_MEM_DESTMACWILD)
            edit_mac_sa     = self.get_table_row(FLOW_RULE_TBL_MEM_EDIT_MAC_SA)
            edit_mac_da     = self.get_table_row(FLOW_RULE_TBL_MEM_EDIT_MAC_DA)
            edit_ip_sa_en   = self.get_table_row(FLOW_RULE_TBL_MEM_EDIT_IP_SA_EN)
            edit_ip_da_en   = self.get_table_row(FLOW_RULE_TBL_MEM_EDIT_IP_DA_EN)
            edit_ip_sa      = self.get_table_row(FLOW_RULE_TBL_MEM_EDIT_IP_SA)
            edit_ip_da      = self.get_table_row(FLOW_RULE_TBL_MEM_EDIT_IP_DA)
            strip_header    = self.get_table_row(FLOW_RULE_TBL_MEM_STRIP_HEADER)
            vxlan_vni       = self.get_table_row(FLOW_RULE_TBL_MEM_VNI)
            vni_value       = self.get_table_row(FLOW_RULE_TBL_MEM_VXLAN_VNI)
            vni_mask        = self.get_table_row(FLOW_RULE_TBL_MEM_VXLAN_VNI_MASK)
            untag           = self.get_table_row(FLOW_RULE_TBL_MEM_UNTAG)
            mark_src_vlan   = self.get_table_row(FLOW_RULE_TBL_MEM_MARK_SRC_VLAN)


            #print "FLOW Rule Table set"
            #print "act:{0}, ip_proto:{1}".format(act, ip_proto)
            #print "tcp_src_op:{0}, tcp_src_port1:{1}, tcp_src_port2:{2}, tcp_dst_op:{3}, tcp_dst_port1:{4}, tcp_dst_port2:{5}".format(tcp_src_op, tcp_src_port1, tcp_src_port2, tcp_dst_op, tcp_dst_port1, tcp_dst_port2)
            #print "tcp_flag_op:{0}, tcp_flag_urg:{1}, tcp_flag_ack:{2}, tcp_flag_psh:{3}, tcp_flag_rst:{4}, tcp_flag_syn:{5}, tcp_flag_fin:{6}".format(tcp_flag_op, tcp_flag_urg, tcp_flag_ack, tcp_flag_psh, tcp_flag_rst, tcp_flag_syn, tcp_flag_fin)
            #print "udp_src_op:{0}, udp_src_port1:{1}, udp_src_port2:{2}, udp_dst_op:{3}, udp_dst_port1:{4}, udp_dst_port2:{5}".format(udp_src_op, udp_src_port1, udp_src_port2, udp_dst_op, udp_dst_port1, udp_dst_port2)
            #print "src_ip:{0}, src_ip_wild:{1}, dst_ip:{2}, dst_ip_wild:{3}, ip_prec:{4}, ip_dscp:{5}, ip_frag:{6}, ip_opt:{7}".format(src_ip, src_ip_wild, dst_ip, dst_ip_wild, ip_prec, ip_dscp, ip_frag, ip_opt)
            #print "truncation:{0}".format(truncation)
            #print "svlan_id:{0}, cvlan_id:{1}, scos:{2}, ccos:{3}, ether_type:{4}, ether_type_wild:{5},svlan_id_wild(6), cvlan_id_wild(7)".format(svlan_id, cvlan_id, scos, ccos, ether_type, ether_type_wild, svlan_id_wild, cvlan_id_wild)
            #print "src_mac:{0}, src_mac_wild:{1}, dst_mac:{2}, dst_mac_wild:{3}".format(src_mac, src_mac_wild, dst_mac,dst_mac_wild)
            #print ""

            if act == GLB_ACL_PERMIT:
                filter = "permit"
            else:
                filter = "deny"

            if ip_proto == 256:
                ip_proto_type = "any"
                ip_proto_num = ""
            else:
                ip_proto_type = "protocol_num"
                ip_proto_num = str(ip_proto)

            src_port_op = ""
            dst_port_op = ""
            src_port1 = ""
            src_port2 = ""
            dst_port1 = ""
            dst_port2 = ""
            str_tcp_flag = ""
            str_tcp_urg = ""
            str_tcp_ack = ""
            str_tcp_psh = ""
            str_tcp_rst = ""
            str_tcp_syn = ""
            str_tcp_fin = ""
            if ip_proto == GLB_ACL_IP_PROTOCOL_TCP:
                if tcp_src_op == GLB_ACL_L4_MATCH_TYPE_EQUAL:
                    src_port_op = "eq"
                    src_port1 = str(tcp_src_port1)
                elif tcp_src_op == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                    src_port_op = "gt"
                    src_port1 = str(tcp_src_port1)
                elif tcp_src_op == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                    src_port_op = "lt"
                    src_port1 = str(tcp_src_port1)
                elif tcp_src_op == GLB_ACL_L4_MATCH_TYPE_RANGE:
                    src_port_op = "range"
                    src_port1 = str(tcp_src_port1)
                    src_port2 = str(tcp_src_port2)
                elif tcp_src_op == GLB_ACL_L4_MATCH_TYPE_ANY:
                    src_port_op = "any"

                if tcp_dst_op == GLB_ACL_L4_MATCH_TYPE_EQUAL:
                    dst_port_op = "eq"
                    dst_port1 = str(tcp_dst_port1)
                elif tcp_dst_op == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                    dst_port_op = "gt"
                    dst_port1 = str(tcp_dst_port1)
                elif tcp_dst_op == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                    dst_port_op = "lt"
                    dst_port1 = str(tcp_dst_port1)
                elif tcp_dst_op == GLB_ACL_L4_MATCH_TYPE_RANGE:
                    dst_port_op = "range"
                    dst_port1 = str(tcp_dst_port1)
                    dst_port2 = str(tcp_dst_port2)
                elif tcp_dst_op == GLB_ACL_L4_MATCH_TYPE_ANY:
                    tcp_dst_op = "any"

                if tcp_flag_op == GLB_ACL_MATCH_TYPE_ANY or tcp_flag_op == GLB_ACL_MATCH_TYPE_ALL:
                    if tcp_flag_op == GLB_ACL_MATCH_TYPE_ANY:
                        str_tcp_flag = "match-any"
                    if tcp_flag_op == GLB_ACL_MATCH_TYPE_ALL:
                        str_tcp_flag = "match-all"
                    if tcp_flag_urg == GLB_SNMP_TRUE:
                        str_tcp_urg = "urg"
                    if tcp_flag_ack == GLB_SNMP_TRUE:
                        str_tcp_ack = "ack"
                    if tcp_flag_psh == GLB_SNMP_TRUE:
                        str_tcp_psh = "psh"
                    if tcp_flag_rst == GLB_SNMP_TRUE:
                        str_tcp_rst = "rst"
                    if tcp_flag_syn == GLB_SNMP_TRUE:
                        str_tcp_syn = "syn"
                    if tcp_flag_fin == GLB_SNMP_TRUE:
                        str_tcp_fin = "fin"

            elif ip_proto == GLB_ACL_IP_PROTOCOL_UDP:
                if udp_src_op == GLB_ACL_L4_MATCH_TYPE_EQUAL:
                    src_port_op = "eq"
                    src_port1 = str(udp_src_port1)
                elif udp_src_op == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                    src_port_op = "gt"
                    src_port1 = str(udp_src_port1)
                elif udp_src_op == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                    src_port_op = "lt"
                    src_port1 = str(udp_src_port1)
                elif udp_src_op == GLB_ACL_L4_MATCH_TYPE_RANGE:
                    src_port_op = "range"
                    src_port1 = str(udp_src_port1)
                    src_port2 = str(udp_src_port2)
                elif udp_src_op == GLB_ACL_L4_MATCH_TYPE_ANY:
                    src_port_op = "any"

                if udp_dst_op == GLB_ACL_L4_MATCH_TYPE_EQUAL:
                    dst_port_op = "eq"
                    dst_port1 = str(udp_dst_port1)
                elif udp_dst_op == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                    dst_port_op = "gt"
                    dst_port1 = str(udp_dst_port1)
                elif udp_dst_op == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                    dst_port_op = "lt"
                    dst_port1 = str(udp_dst_port1)
                elif udp_dst_op == GLB_ACL_L4_MATCH_TYPE_RANGE:
                    dst_port_op = "range"
                    dst_port1 = str(udp_dst_port1)
                    dst_port2 = str(udp_dst_port2)
                elif udp_dst_op == GLB_ACL_L4_MATCH_TYPE_ANY:
                    dst_port_op = "any"

            if ip_prec == GLB_ACL_INVALID_IP_PRECEDENCE:
                str_ip_prec = ""
            else:
                str_ip_prec = str(ip_prec)

            if ip_dscp == GLB_ACL_INVALID_IP_DSCP:
                str_ip_dscp = ""
            else:
                str_ip_dscp = str(ip_dscp)

            if ip_frag == FLOW_RULE_IPFRAGMENTS_FIRST:
                str_ip_frag = "first-fragment"
            elif ip_frag == FLOW_RULE_IPFRAGMENTS_NON_FIRST:
                str_ip_frag = "non-first-fragment"
            elif ip_frag == FLOW_RULE_IPFRAGMENTS_NONE:
                str_ip_frag = "non-fragment"
            elif ip_frag == FLOW_RULE_IPFRAGMENTS_NON_OR_FIRST:
                str_ip_frag = "non-or-first-fragment"
            elif ip_frag == FLOW_RULE_IPFRAGMENTS_SMALL:
                str_ip_frag = "small-fragment"
            elif ip_frag == FLOW_RULE_IPFRAGMENTS_ANY:
                str_ip_frag = "any-fragment"
            else:
                str_ip_frag = ""

            if ip_opt == GLB_SNMP_TRUE:
                str_ip_opt = "options"
            else:
                str_ip_opt = ""

            if truncation == GLB_SNMP_TRUE:
                str_trunc = "truncation"
            else:
                str_trunc = ""

            if scos == GLB_ACL_INVALID_COS:
                str_scos = ""
            else:
                str_scos = str(scos)

            if GLB_ACL_VLAN_ID_INVALID == svlan_id:
                str_svlan_id = ""
                str_svlan_id_wild = ""
                str_s_id = ""
            else:
                str_s_id = "svlan"
                str_svlan_id = str(svlan_id)
                str_svlan_id_wild = str(svlan_id_wild)     
                
            if GLB_ACL_VLAN_ID_INVALID == cvlan_id:
                str_cvlan_id = ""
                str_c_id = ""
                str_cvlan_id_wild = ""
            else:
                str_c_id = "cvlan"
                str_cvlan_id = str(cvlan_id)
                str_cvlan_id_wild = str(cvlan_id_wild)
                
            if ccos == GLB_ACL_INVALID_COS:
                str_ccos = ""
            else:
                str_ccos = str(ccos)
              

            str_mac_sa_type = ""
            str_mac_sa = ""
            str_mac_sa_wild = ""
            str_mac_da_type = ""
            str_mac_da = ""
            str_mac_da_wild = ""

            if len(src_mac) == GLB_SNMP_MAC_ADDR_LEN:
                if len(src_mac_wild) != GLB_SNMP_MAC_ADDR_LEN:
                    return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
                mac = src_mac.encode("hex")
                str_mac_sa = mac[0:4]+"."+mac[4:8]+"."+mac[8:12]
                mac = src_mac_wild.encode("hex")
                str_mac_sa_wild = mac[0:4]+"."+mac[4:8]+"."+mac[8:12]
                str_mac_sa_type = "src_mac"

            if len(dst_mac) == GLB_SNMP_MAC_ADDR_LEN:
                if len(dst_mac_wild) != GLB_SNMP_MAC_ADDR_LEN:
                    return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
                mac = dst_mac.encode("hex")
                str_mac_da = mac[0:4]+"."+mac[4:8]+"."+mac[8:12]
                mac = dst_mac_wild.encode("hex")
                str_mac_da_wild = mac[0:4]+"."+mac[4:8]+"."+mac[8:12]
                str_mac_da_type = "dest_mac"

            str_ether_type = ""
            if len(ether_type):
                if len(ether_type_wild) == 0:
                    return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
                str_ether_type = "ether_type"

            if len(str_ether_type):
                cmd = "cdbctl create/cdb/l2/ace_config/{0}/{1}/{2}/type/flow/src_mac/{3}/{4}/{5}/dest_mac/{6}/{7}/{8}/" \
                        "vlan/{9}/{10}/{11}/inner_vlan/{12}/{13}/{14}/cos/{15}/{16}/0/inner_cos/{17}/{18}/0/"   \
                        "ether_type/{19}/{20}/{21}/src-ip////dst-ip////dscp////precedence////frag//"     \
                        "l3-protocol////src-l4-port////////igmp_type//icmp_type///tcpflag////////udf/////timerange//"     \
                        "options//truncation/{22}/snmp_set/true/"  \
                        .format(flow_name,str(seq_no),filter,str_mac_sa_type, str_mac_sa, str_mac_sa_wild,
                        str_mac_da_type, str_mac_da, str_mac_da_wild, str_s_id, str_svlan_id, str_svlan_id_wild,
                        str_c_id, str_cvlan_id, str_cvlan_id_wild,
                        str_scos, str_scos, str_ccos, str_ccos, str_ether_type, ether_type, ether_type_wild, str_trunc)
            else:
                cmd = "cdbctl create/cdb/l2/ace_config/{0}/{1}/{2}/type/flow/src_mac/{3}/{4}/{5}/dest_mac/{6}/{7}/{8}/" \
                        "vlan/{9}/{10}/{11}/inner_vlan/{12}/{13}/{14}/cos/{15}/{16}/0/inner_cos/{17}/{18}/0/"   \
                        "ether_type////src-ip/{19}/{20}/{21}/dst-ip/{22}/{23}/{24}/dscp/{25}/{26}/0/"  \
                        "precedence/{27}/{28}/0/frag/{29}/l3-protocol/{30}/{31}/0/src-l4-port/{32}/{33}/{34}/"    \
                        "dest-l4-port/{35}/{36}/{37}/igmp_type//icmp_type///tcpflag/{38}/{39}/{40}/{41}/{42}/{43}/{44}/"    \
                        "udf/////timerange//options/{45}/truncation/{46}/snmp_set/true/"  \
                        .format(flow_name, str(seq_no), filter, str_mac_sa_type, str_mac_sa, str_mac_sa_wild,
                        str_mac_da_type, str_mac_da, str_mac_da_wild, str_s_id, str_svlan_id, str_svlan_id_wild, str_c_id,
                        str_cvlan_id, str_cvlan_id_wild, str_scos, str_scos, str_ccos, str_ccos, src_ip, src_ip, src_ip_wild,
                        dst_ip, dst_ip, dst_ip_wild, str_ip_dscp, str_ip_dscp, str_ip_prec, str_ip_prec,str_ip_frag,
                        ip_proto_type, ip_proto_num, src_port_op, src_port1, src_port2, dst_port_op, dst_port1, dst_port2,
                        str_tcp_flag, str_tcp_ack, str_tcp_fin, str_tcp_psh, str_tcp_rst, str_tcp_syn, str_tcp_urg,
                        str_ip_opt, str_trunc)

            str_edit_mac_sa = ""
            str_edit_mac_da = ""

            if len(edit_mac_sa) == GLB_SNMP_MAC_ADDR_LEN:
                mac = edit_mac_sa.encode("hex")
                str_edit_mac_sa = mac[0:4] + "." + mac[4:8] + "." + mac[8:12]
                cmd = cmd + "edit-macsa/" + str_edit_mac_sa + "/"

            if len(edit_mac_da) == GLB_SNMP_MAC_ADDR_LEN:
                mac = edit_mac_da.encode("hex")
                str_edit_mac_da = mac[0:4] + "." + mac[4:8] + "." + mac[8:12]
                cmd = cmd + "edit-macda/" + str_edit_mac_da + "/"

            if vxlan_vni == GLB_SNMP_TRUE:
                cmd = cmd + "vni/" + str(vni_value) + "/" + vni_mask + "/"

            print edit_ip_da,edit_ip_sa
            if edit_ip_sa_en == GLB_SNMP_TRUE:
                cmd = cmd + "edit-ipsa/"+ edit_ip_sa + "/"

            if edit_ip_da_en == GLB_SNMP_TRUE:
                cmd = cmd + "edit-ipda/"+ edit_ip_da + "/"

            if strip_header == GLB_SNMP_TRUE:
                cmd = cmd + "strip-header/1/"

            if untag == GLB_SNMP_TRUE:
                cmd = cmd + "untag/1/"

            if mark_src_vlan != 0:
                cmd = cmd + "mark-source/" + str(mark_src_vlan) + "/"



            print "tapflow cmd = ", cmd
            szmsg = execute_cmd(cmd)
            print "szmsg == ",szmsg

            if szmsg.find('%') != -1 and szmsg.find('should be added') == -1:
                print "set add flow fail szmsg = ", szmsg
                return self.set_request_error(SNMP_ERR_GENERR)
            else:
                tmp = flow_name + str(seq_no)
                print "set FLOW_RULE_TABLE sub_add tmp = ", tmp
                increase_data_sync_add("FLOW_RULE_TABLE", tmp)
                print "set add sync FLOW_RULE_TABLE tmp = ", tmp
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_ace_config = tbl_pb2.tbl_ace_config()
        try:
            tbl_ace_config.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_ace_config fail'
            return

        #if tbl_ace_config.tap_snmp_set != 0:
        #    return
        if tbl_ace_config.key.acl_name[:6] != "#FLOW#":
            return
        else:
            flow_name = tbl_ace_config.key.acl_name[6:]

        if operation == CDB_OPER_ADD:
            tmp = flow_name + str(tbl_ace_config.key.seq_no)
            print "sub_add tmp = ", tmp
            if check_data_sync_add("FLOW_RULE_TABLE", tmp):
                decrease_data_sync_add("FLOW_RULE_TABLE", tmp)
                print "check return FLOW_RULE_TABLE sub_add tmp = ", tmp
                return
            if tbl_ace_config.tap_snmp_set != 0:
                return
            new_row = FLOW_RULE_TABLE.addRow([AGENT.DisplayString(flow_name), AGENT.Integer32(tbl_ace_config.key.seq_no)])
            #print tbl_ace_config
            if 0 == tbl_ace_config.deny:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_ACT, AGENT.Integer32(GLB_ACL_PERMIT))
            if (tbl_ace_config.key_flags & FLOW_IP_PROT_FLAG ) != 0:
                if tbl_ace_config.ip_protocol_mask == 0xff:
                    new_row.setRowCell(FLOW_RULE_TBL_MEM_IPPROTOCOL, AGENT.Integer32(FLOW_RULE_IP_PROTOCOL_ANY))
                else:
                    new_row.setRowCell(FLOW_RULE_TBL_MEM_IPPROTOCOL, AGENT.Integer32(tbl_ace_config.ip_protocol))
            if tbl_ace_config.ip_protocol == GLB_ACL_IP_PROTOCOL_TCP:
                if (tbl_ace_config.key_flags & FLOW_SRC_PORT_FLAG) != 0:
                    print tbl_ace_config.l4_src_port_type, tbl_ace_config.l4_src_port, tbl_ace_config.l4_src_port_mask
                    new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPSRCOP, AGENT.Integer32(tbl_ace_config.l4_src_port_type))
                    if tbl_ace_config.l4_src_port_type == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN :
                        new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port-1))
                    elif tbl_ace_config.l4_src_port_type == GLB_ACL_L4_MATCH_TYPE_LESS_THAN :
                        new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port_mask+1))
                    else:
                        new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port))
                        new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPSRCPORT2, AGENT.Integer32(tbl_ace_config.l4_src_port_mask))
                if (tbl_ace_config.key_flags & FLOW_DST_PORT_FLAG) != 0:
                    new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPDESTOP, AGENT.Integer32(tbl_ace_config.l4_dst_port_type))
                    if tbl_ace_config.l4_dst_port_type == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN :
                        new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port-1))
                    elif tbl_ace_config.l4_dst_port_type == GLB_ACL_L4_MATCH_TYPE_LESS_THAN :
                        new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port_mask+1))
                    else:
                        new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port))
                        new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPDESTPORT2, AGENT.Integer32(tbl_ace_config.l4_dst_port_mask))
            if tbl_ace_config.ip_protocol == GLB_ACL_IP_PROTOCOL_UDP:
               if (tbl_ace_config.key_flags & FLOW_SRC_PORT_FLAG) != 0:
                   new_row.setRowCell(FLOW_RULE_TBL_MEM_UDPSRCOP, AGENT.Integer32(tbl_ace_config.l4_src_port_type))
                   if tbl_ace_config.l4_src_port_type == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                       new_row.setRowCell(FLOW_RULE_TBL_MEM_UDPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port - 1))
                   elif tbl_ace_config.l4_src_port_type == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                       new_row.setRowCell(FLOW_RULE_TBL_MEM_UDPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port_mask + 1))
                   else:
                       new_row.setRowCell(FLOW_RULE_TBL_MEM_UDPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port))
                       new_row.setRowCell(FLOW_RULE_TBL_MEM_UDPSRCPORT2, AGENT.Integer32(tbl_ace_config.l4_src_port_mask))

               if (tbl_ace_config.key_flags & FLOW_DST_PORT_FLAG) != 0:
                   new_row.setRowCell(FLOW_RULE_TBL_MEM_UDPDESTOP, AGENT.Integer32(tbl_ace_config.l4_dst_port_type))
                   if tbl_ace_config.l4_dst_port_type == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                       new_row.setRowCell(FLOW_RULE_TBL_MEM_UDPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port - 1))
                   elif tbl_ace_config.l4_dst_port_type == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                       new_row.setRowCell(FLOW_RULE_TBL_MEM_UDPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port_mask + 1))
                   else:
                       new_row.setRowCell(FLOW_RULE_TBL_MEM_UDPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port))
                       new_row.setRowCell(FLOW_RULE_TBL_MEM_UDPDESTPORT2, AGENT.Integer32(tbl_ace_config.l4_dst_port_mask))
            if (tbl_ace_config.key_flags & FLOW_TCP_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPFLAGOP, AGENT.Integer32(tbl_ace_config.tcp_flags))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_URG:
                    new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPFLAGURG, AGENT.Integer32(GLB_SNMP_TRUE))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_ACK:
                    new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPFLAGACK, AGENT.Integer32(GLB_SNMP_TRUE))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_PSH:
                    new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPFLAGPSH, AGENT.Integer32(GLB_SNMP_TRUE))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_RST:
                    new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPFLAGRST, AGENT.Integer32(GLB_SNMP_TRUE))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_SYN:
                    new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPFLAGSYN, AGENT.Integer32(GLB_SNMP_TRUE))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_FIN:
                    new_row.setRowCell(FLOW_RULE_TBL_MEM_TCPFLAGFIN, AGENT.Integer32(GLB_SNMP_TRUE))
            if (tbl_ace_config.key_flags & FLOW_IPSA_FLAG) != 0 :
                ipsa = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_ace_config.src_ip.ipv4_addr)))
                ipsa_mask = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_ace_config.src_ip_mask.ipv4_addr)))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_SRCIP, AGENT.IpAddress(ipsa))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_SRCWILD, AGENT.IpAddress(ipsa_mask))

            if (tbl_ace_config.key_flags & FLOW_IPDA_FLAG) != 0:
                ipda = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_ace_config.dst_ip.ipv4_addr)))
                ipda_mask = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_ace_config.dst_ip_mask.ipv4_addr)))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_DESTIP, AGENT.IpAddress(ipda))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_DESTWILD, AGENT.IpAddress(ipda_mask))
            if (tbl_ace_config.key_flags & FLOW_IP_PRECEDENCE_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_IPPRECEDENCE, AGENT.Integer32(tbl_ace_config.ip_precedence))
            if (tbl_ace_config.key_flags & FLOW_DSCP_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_IPDSCP, AGENT.Integer32(tbl_ace_config.dscp))
            if (tbl_ace_config.key_flags & FLOW_IP_FRAG_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_IPFRAGMENTS, AGENT.Integer32(tbl_ace_config.ip_frag))
            if (tbl_ace_config.key_flags & FLOW_IP_OPTIONS_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_IPOPTIONS, AGENT.Integer32(GLB_SNMP_TRUE))
            if (tbl_ace_config.key_flags & FLOW_TAP_TRUNCATION_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_TRUNCATION, AGENT.Integer32(GLB_SNMP_TRUE))
            if (tbl_ace_config.key_flags & FLOW_VLAN_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_VLANID, AGENT.Integer32(tbl_ace_config.svlan))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_VLANID_WILD, AGENT.DisplayString(hex(tbl_ace_config.svlan_mask)))
            else:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_VLANID, AGENT.Integer32(GLB_ACL_VLAN_ID_INVALID))
            if (tbl_ace_config.key_flags & FLOW_INNER_VLAN_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_INNERVLANID, AGENT.Integer32(tbl_ace_config.cvlan))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_INNERVLAN_WILD, AGENT.DisplayString(hex(tbl_ace_config.cvlan_mask)))
            else:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_INNERVLANID, AGENT.Integer32(GLB_ACL_VLAN_ID_INVALID))
            if (tbl_ace_config.key_flags & FLOW_COS_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_COS, AGENT.Integer32(tbl_ace_config.svlan_cos))
            if (tbl_ace_config.key_flags & FLOW_INNER_COS_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_INNERCOS, AGENT.Integer32(tbl_ace_config.cvlan_cos))
            if (tbl_ace_config.key_flags & FLOW_ETHERTYPE_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_ETHERTYPE, AGENT.DisplayString(hex(tbl_ace_config.ether_type)))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_ETHERTYPEWILD, AGENT.DisplayString(hex(tbl_ace_config.ether_type_mask)))
            if (tbl_ace_config.key_flags & FLOW_MAC_SA_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_SRCMAC, AGENT.OctetString(tbl_ace_config.src_mac.mac))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_SRCMACWILD, AGENT.OctetString(tbl_ace_config.src_mac_mask.mac))
            if (tbl_ace_config.key_flags & FLOW_MAC_DA_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_DESTMAC, AGENT.OctetString(tbl_ace_config.dst_mac.mac))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_DESTMACWILD, AGENT.OctetString(tbl_ace_config.dst_mac_mask.mac))
                
            if tbl_ace_config.tap_action_edit_src_mac_en != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_EDIT_MAC_SA, AGENT.OctetString(tbl_ace_config.tap_action_edit_src_mac.mac))
            if tbl_ace_config.tap_action_edit_dest_mac_en != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_EDIT_MAC_DA, AGENT.OctetString(tbl_ace_config.tap_action_edit_dest_mac.mac))
            if tbl_ace_config.tap_action_edit_ipda_en != 0 :
                new_row.setRowCell(FLOW_RULE_TBL_MEM_EDIT_IP_DA_EN, AGENT.Integer32(GLB_SNMP_TRUE))
                ipda = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_ace_config.tap_action_edit_ipda.ipv4_addr)))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_EDIT_IP_DA, AGENT.IpAddress(ipda))
            if tbl_ace_config.tap_action_edit_ipsa_en != 0 :
                new_row.setRowCell(FLOW_RULE_TBL_MEM_EDIT_IP_SA_EN, AGENT.Integer32(GLB_SNMP_TRUE))
                ipsa = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_ace_config.tap_action_edit_ipsa.ipv4_addr)))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_EDIT_IP_SA, AGENT.IpAddress(ipsa))
                
            if (tbl_ace_config.key_flags & FLOW_TAP_VXLAN_VNI_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_VNI, AGENT.Integer32(GLB_SNMP_TRUE))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_VXLAN_VNI, AGENT.Integer32(tbl_ace_config.l4_vxlan_vni))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_VXLAN_VNI_MASK, AGENT.DisplayString(hex(tbl_ace_config.l4_vxlan_vni_mask)))
            else:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_VNI, AGENT.Integer32(GLB_SNMP_FALSE))
                new_row.setRowCell(FLOW_RULE_TBL_MEM_VXLAN_VNI, AGENT.Integer32(0))

            if (tbl_ace_config.key_flags & FLOW_TAP_UNTAG_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_UNTAG, AGENT.Integer32(GLB_SNMP_TRUE))
            else:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_UNTAG, AGENT.Integer32(GLB_SNMP_FALSE))
            
            if (tbl_ace_config.key_flags & FLOW_TAP_MARK_SOURCE_FLAG) != 0:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_MARK_SRC_VLAN, AGENT.Integer32(tbl_ace_config.tap_action_mark_vlan))
            else:
                new_row.setRowCell(FLOW_RULE_TBL_MEM_MARK_SRC_VLAN, AGENT.Integer32(0))
            #print "flow SNMP_ROW_ACTIVE"
            new_row.setRowCell(FLOW_RULE_TBL_MEM_ROWSTATUS, AGENT.Integer32(SNMP_ROW_ACTIVE))

        if operation == CDB_OPER_DEL:
            tmp = flow_name + str(tbl_ace_config.key.seq_no)
            print "sub_del tmp = ", tmp
            if check_data_sync_del("FLOW_RULE_TABLE", tmp):
                decrease_data_sync_del("FLOW_RULE_TABLE", tmp)
                print "check return FLOW_RULE_TABLE sub_del tmp = ", tmp
                return
            if tbl_ace_config.tap_snmp_set != 0:
                return
            FLOW_RULE_TABLE.delRow([AGENT.DisplayString(flow_name), AGENT.Integer32(tbl_ace_config.key.seq_no)])

################################################################################
#
# flowInterfaceStatsTable enterprise.111.3
#
################################################################################
# Oid indexes
#FLOW_IF_STATS_TBL_MEM_IFINDEX     = 1
#FLOW_IF_STATS_TBL_MEM_FLOWNAME    = 2
#FLOW_IF_STATS_TBL_MEM_SEQNO       = 3
#FLOW_IF_STATS_TBL_MEM_MATCH_PKTS  = 4
#FLOW_IF_STATS_TBL_MEM_MATCH_BYTES = 5
#FLOW_IF_STATS_TBL_MEM_CLEAR       = 10

FLOW_IF_STATS_TABLE = None
class FlowIfStatsTableHandler(SnmpTableBase):
    def __init__(self, agentx):
        super(FlowIfStatsTableHandler, self).__init__(agentx)
        self.flow = dict()
        self.if_tap_flow = dict()

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        GLB_FLOW_STATS_MUTEX.acquire()
        if tbl_id == TBL_TAP_GROUP_INGRESS_FLOW and ds_id == CDB_INVALID_DS_ID:
            tbl_tap_igs_flow = tbl_pb2.tbl_tap_group_ingress_flow()
            try:
                tbl_tap_igs_flow.ParseFromString(data)
            except:
                print 'ERROR!! Parse tbl_tap_igs_flow fail'
                GLB_FLOW_STATS_MUTEX.release()
                return

            if not tbl_tap_igs_flow.key.flow_name:
                GLB_FLOW_STATS_MUTEX.release()
                return
                
            if tbl_tap_igs_flow.key.flow_name[:6] != "#FLOW#":
                GLB_FLOW_STATS_MUTEX.release()
                return
            else:
                flow_name = tbl_tap_igs_flow.key.flow_name[6:]

            ifindex = GLB_IF_NAME_DICT[tbl_tap_igs_flow.key.if_name]
            if operation == CDB_OPER_ADD:
                if flow_name not in self.if_tap_flow:
                    if_list = list()
                    self.if_tap_flow[flow_name] = if_list
                if_list = self.if_tap_flow[flow_name]
                if_list.append(ifindex)
                FLOW_IF_STATS_TABLE.addRow([AGENT.Integer32(ifindex), AGENT.DisplayString(flow_name), AGENT.Integer32(0)])
                if flow_name in self.flow:
                    ace_list = self.flow[flow_name]
                    for seqno in ace_list:
                        FLOW_IF_STATS_TABLE.addRow([AGENT.Integer32(ifindex), AGENT.DisplayString(flow_name), AGENT.Integer32(seqno)])

            if operation == CDB_OPER_DEL:
                if flow_name in self.if_tap_flow:
                    if_list = self.if_tap_flow[flow_name]
                    if ifindex in if_list:
                        if_list.remove(ifindex)
                    if len(if_list) == 0:
                        del(self.if_tap_flow[flow_name])
                    FLOW_IF_STATS_TABLE.delRow([AGENT.Integer32(ifindex), AGENT.DisplayString(flow_name), AGENT.Integer32(0)])
                if flow_name in self.flow:
                    ace_list = self.flow[flow_name]
                    for seqno in ace_list:
                        FLOW_IF_STATS_TABLE.delRow([AGENT.Integer32(ifindex), AGENT.DisplayString(flow_name), AGENT.Integer32(seqno)])

        if tbl_id == TBL_ACL_CONFIG and ds_id == CDB_INVALID_DS_ID:
            tbl_acl_config = tbl_pb2.tbl_acl_config()
            try:
                tbl_acl_config.ParseFromString(data)
            except:
                print 'ERROR!! Parse tbl_acl_config fail'
                GLB_FLOW_STATS_MUTEX.release()
                return
                
            if tbl_acl_config.key.name[:6] != "#FLOW#":
                GLB_FLOW_STATS_MUTEX.release()
                return
            else:
                flow_name = tbl_acl_config.key.name[6:]

            #print "ACL 1", tbl_acl.key.name, self.if_tap_flow.keys(), self.flow.keys()
            if operation == CDB_OPER_ADD:
                ace_list = list()
                self.flow[flow_name] = ace_list
                #if tbl_acl.key.name in self.if_tap_flow:
                #    if_list = self.if_tap_flow[tbl_acl.key.name]
                #    for ifindex in if_list:
                #        FLOW_IF_STATS_TABLE.addRow([AGENT.Integer32(ifindex), AGENT.Integer32(0)])

            if operation == CDB_OPER_DEL:
                if flow_name in self.flow:
                    del(self.flow[flow_name])
                #if tbl_acl.key.name in self.if_tap_flow:
                #    if_list = self.if_tap_flow[tbl_acl.key.name]
                #    for ifindex in if_list:
                #        FLOW_IF_STATS_TABLE.delRow([AGENT.Integer32(ifindex), AGENT.Integer32(0)])
            #print "ACL 2", tbl_acl.key.name, self.if_tap_flow.keys(), self.flow.keys()

        if tbl_id == TBL_ACE_CONFIG and ds_id == CDB_INVALID_DS_ID:
            tbl_ace_config = tbl_pb2.tbl_ace_config()
            try:
                tbl_ace_config.ParseFromString(data)
            except:
                print 'ERROR!! Parse tbl_ace_config fail'
                GLB_FLOW_STATS_MUTEX.release()
                return
                
            if tbl_ace_config.key.acl_name[:6] != "#FLOW#":
                GLB_FLOW_STATS_MUTEX.release()
                return
            else:
                flow_name = tbl_ace_config.key.acl_name[6:]

            #print "ACE 1", ds_ip_ace.parent_acl.name, ds_ip_ace.key.seqno, self.if_tap_flow.keys(), self.flow.keys()
            if operation == CDB_OPER_ADD:
                if flow_name in self.flow:
                    ace_list = self.flow[flow_name]
                    if tbl_ace_config.key.seq_no not in ace_list:
                        ace_list.append(tbl_ace_config.key.seq_no)
                        # if flow applied to interface, add an instance
                        if flow_name in self.if_tap_flow:
                            if_list = self.if_tap_flow[flow_name]
                            for ifindex in if_list:
                                FLOW_IF_STATS_TABLE.addRow([AGENT.Integer32(ifindex),
                                                            AGENT.DisplayString(flow_name),
                                                            AGENT.Integer32(tbl_ace_config.key.seq_no)])

            if operation == CDB_OPER_DEL:
                if flow_name in self.flow:
                    ace_list = self.flow[flow_name]
                    if tbl_ace_config.key.seq_no in ace_list:
                        ace_list.remove(tbl_ace_config.key.seq_no)
                        # if flow applied to interface, remove an instance
                        if flow_name in self.if_tap_flow:
                            if_list = self.if_tap_flow[flow_name]
                            for ifindex in if_list:
                                FLOW_IF_STATS_TABLE.delRow([AGENT.Integer32(ifindex),
                                                            AGENT.DisplayString(flow_name),
                                                            AGENT.Integer32(tbl_ace_config.key.seq_no)])
            #print "ACE 2", ds_ip_ace.parent_acl.name, ds_ip_ace.key.seqno, self.if_tap_flow.keys(), self.flow.keys()
        GLB_FLOW_STATS_MUTEX.release()

FLOW_IF_EGRESS_STATS_TABLE = None
class FlowIfEgressStatsTableHandler(SnmpTableBase):
    def __init__(self, agentx):
        super(FlowIfEgressStatsTableHandler, self).__init__(agentx)
        #self.flow = dict()
        self.egress_flow = dict()
        self.if_egress_flow = dict()

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #GLB_FLOW_EGRESS_STATS_MUTEX.acquire()
        #print tbl_id, operation, ds_id, ds_id2, field_id
        if tbl_id == TBL_INTERFACE and ds_id == CDB_INVALID_DS_ID:
            tbl_interface = tbl_pb2.tbl_interface()
            try:
                tbl_interface.ParseFromString(data)
            except:
                print 'ERROR!! Parse tbl_interface fail'
                #GLB_FLOW_EGRESS_STATS_MUTEX.release()
                return

            #print tbl_interface.egs_acl

            if tbl_interface.egs_acl != "":
                if tbl_interface.ifindex not in self.if_egress_flow:
                    self.if_egress_flow[tbl_interface.ifindex] = tbl_interface.egs_acl
                print "add row 0", tbl_interface.ifindex
                newRow = FLOW_IF_EGRESS_STATS_TABLE.addRow([AGENT.Integer32(tbl_interface.ifindex), AGENT.Integer32(0)])
                msg = os.popen("cdbctl show/cdb/l2/show_acl_counter/{0}/out".format(tbl_interface.key.name), "r")
                lines = msg.readlines()

                for line in lines:
                    #print line
                    flow_arr = line.strip("\r\n").split(' ')
                    if len(flow_arr) >= 6 and flow_arr[0] == '(total':
                        newRow.setRowCell(FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_PKTS,  AGENT.Counter64(long(flow_arr[2])))
                        newRow.setRowCell(FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_BYTES, AGENT.Counter64(long(flow_arr[5])))
                if tbl_interface.egs_acl in self.egress_flow:
                    ace_list = self.egress_flow[tbl_interface.egs_acl]
                    for seqno in ace_list:
                        print "add row", tbl_interface.ifindex,seqno
                        newRow = FLOW_IF_EGRESS_STATS_TABLE.addRow([AGENT.Integer32(tbl_interface.ifindex),  AGENT.Integer32(seqno)])
                        for line in lines:
                            flow_arr = line.strip("\r\n").split(' ')
                            if len(flow_arr) >= 4 and flow_arr[1] == 'sequence-num' and int(flow_arr[2]) == seqno:
                                newRow.setRowCell(FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_PKTS,AGENT.Counter64(long(flow_arr[len(flow_arr) - 4])))
                                newRow.setRowCell(FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_BYTES, AGENT.Counter64(long(flow_arr[len(flow_arr)-2])))

            else:
                if tbl_interface.ifindex in self.if_egress_flow:
                    flow_name = self.if_egress_flow[tbl_interface.ifindex]
                    del(self.if_egress_flow[tbl_interface.ifindex])
                    print "del row 0", tbl_interface.ifindex
                    FLOW_IF_EGRESS_STATS_TABLE.delRow([AGENT.Integer32(tbl_interface.ifindex), AGENT.Integer32(0)])
                if flow_name in self.egress_flow:
                    ace_list = self.egress_flow[flow_name]
                    for seqno in ace_list:
                        print "del row", tbl_interface.ifindex, seqno
                        FLOW_IF_EGRESS_STATS_TABLE.delRow([AGENT.Integer32(tbl_interface.ifindex), AGENT.Integer32(seqno)])

        if tbl_id == TBL_ACL_CONFIG and ds_id == CDB_INVALID_DS_ID:
            tbl_acl_config = tbl_pb2.tbl_acl_config()
            try:
                tbl_acl_config.ParseFromString(data)
            except:
                print 'ERROR!! Parse tbl_acl_config fail'
                #GLB_FLOW_EGRESS_STATS_MUTEX.release()
                return
                
            if tbl_acl_config.key.name[:6] == "#FLOW#" or tbl_acl_config.key.name[:8] == "#FW_MAT#":
                return

            print "ACL 1", tbl_acl_config.key.name, self.egress_flow.keys(), self.if_egress_flow.keys()
            if operation == CDB_OPER_ADD:
                ace_list = list()
                self.egress_flow[tbl_acl_config.key.name] = ace_list
                #if tbl_acl.key.name in self.if_tap_flow:
                #    if_list = self.if_tap_flow[tbl_acl.key.name]
                #    for ifindex in if_list:
                #        FLOW_IF_STATS_TABLE.addRow([AGENT.Integer32(ifindex), AGENT.Integer32(0)])

            if operation == CDB_OPER_DEL:
                if tbl_acl_config.key.name in self.egress_flow:
                    del(self.egress_flow[tbl_acl_config.key.name])
                #if tbl_acl.key.name in self.if_tap_flow:
                #    if_list = self.if_tap_flow[tbl_acl.key.name]
                #    for ifindex in if_list:
                #        FLOW_IF_STATS_TABLE.delRow([AGENT.Integer32(ifindex), AGENT.Integer32(0)])
            #print "ACL 2", tbl_acl.key.name, self.if_tap_flow.keys(), self.flow.keys()

        if tbl_id == TBL_ACE_CONFIG and ds_id == CDB_INVALID_DS_ID:
            tbl_ace_config = tbl_pb2.tbl_ace_config()
            try:
                tbl_ace_config.ParseFromString(data)
            except:
                print 'ERROR!! Parse tbl_ace_config fail'
                #GLB_FLOW_EGRESS_STATS_MUTEX.release()
                return
                
            if tbl_ace_config.key.acl_name[:6] == "#FLOW#" or tbl_ace_config.key.acl_name[:8] == "#FW_MAT#":
                return

            print "ACE 1", tbl_ace_config.key.acl_name, tbl_ace_config.key.seq_no, self.egress_flow.keys(), self.if_egress_flow.keys()
            if operation == CDB_OPER_ADD:
                if tbl_ace_config.key.acl_name in self.egress_flow:
                    ace_list = self.egress_flow[tbl_ace_config.key.acl_name]
                    if tbl_ace_config.key.seq_no not in ace_list:
                        ace_list.append(tbl_ace_config.key.seq_no)
                        # if flow applied to interface, add an instance
                        for key in self.if_egress_flow:
                            if tbl_ace_config.key.acl_name == self.if_egress_flow[key]:
                                ifindex = key
                                print "ace add row", ifindex, tbl_ace_config.key.seq_no
                                for ifname in GLB_IF_NAME_DICT:
                                    if ifindex == GLB_IF_NAME_DICT[ifname]:
                                        msg = os.popen("cdbctl show/cdb/l2/show_acl_counter/{0}/out".format(ifname), "r")
                                        lines = msg.readlines()
                                newRow = FLOW_IF_EGRESS_STATS_TABLE.addRow([AGENT.Integer32(ifindex), AGENT.Integer32(0)])
                                if lines:
                                    for line in lines:
                                        # print line
                                        flow_arr = line.strip("\r\n").split(' ')
                                        if len(flow_arr) >= 6 and flow_arr[0] == '(total':
                                            newRow.setRowCell(FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_PKTS, AGENT.Counter64(long(flow_arr[2])))
                                            newRow.setRowCell(FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_BYTES, AGENT.Counter64(long(flow_arr[5])))
                                newRow = FLOW_IF_EGRESS_STATS_TABLE.addRow([AGENT.Integer32(ifindex), AGENT.Integer32(tbl_ace_config.key.seq_no)])
                                if lines:
                                    for line in lines:
                                        # print line
                                        flow_arr = line.strip("\r\n").split(' ')
                                        if len(flow_arr) >= 4 and flow_arr[1] == 'sequence-num' and int(flow_arr[2]) == tbl_ace_config.key.seq_no:
                                            newRow.setRowCell(FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_PKTS, AGENT.Counter64(long(flow_arr[len(flow_arr) - 4])))
                                            newRow.setRowCell(FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_BYTES, AGENT.Counter64(long(flow_arr[len(flow_arr) - 2])))

            if operation == CDB_OPER_DEL:
                if tbl_ace_config.key.acl_name in self.egress_flow:
                    ace_list = self.egress_flow[tbl_ace_config.key.acl_name]
                    if tbl_ace_config.key.seq_no in ace_list:
                        ace_list.remove(tbl_ace_config.key.seq_no)
                        for key in self.if_egress_flow:
                            if tbl_ace_config.key.acl_name == self.if_egress_flow[key]:
                                print "ace del row", key, tbl_ace_config.key.seq_no
                                FLOW_IF_EGRESS_STATS_TABLE.delRow([AGENT.Integer32(key), AGENT.Integer32(0)])
                                FLOW_IF_EGRESS_STATS_TABLE.delRow([AGENT.Integer32(key), AGENT.Integer32(tbl_ace_config.key.seq_no)])
            #print "ACE 2", ds_ip_ace.parent_acl.name, ds_ip_ace.key.seqno, self.if_tap_flow.keys(), self.flow.keys()
        #GLB_FLOW_EGRESS_STATS_MUTEX.release()

        
################################################################################
#
# ipaccessTable enterprise.111.5
#
################################################################################
# Oid indexes
IP_ACCESS_TBL_MEM_NAME        = 1
IP_ACCESS_TBL_MEM_ROWSTATUS   = 10
IP_ACCESS_TBL_MEM_DESCRIPTION = 11

# Macro definitions
IP_ACCESS_NAME_LEN = 20
IP_ACCESS_DESCRIPTION_LEN = 100

IP_ACCESS_TABLE = None
class IpAccessTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(IpAccessTableHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        # Index check
        indexes = self.get_request_indexes(IP_ACCESS_TABLE)
        ip_access_name = indexes[0]
        if len(ip_access_name) > IP_ACCESS_NAME_LEN:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        match = re.search("^[a-zA-Z][0-9a-zA-Z._-]*$", ip_access_name[0:len(ip_access_name)])
        if not match:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        if len(match.group(0)) != len(ip_access_name):
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        # Table Nodes check
        if colnum == IP_ACCESS_TBL_MEM_ROWSTATUS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)

            rowstatus = data
            if rowstatus != SNMP_ROW_CREATEANDGO and rowstatus != SNMP_ROW_DESTROY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_TBL_MEM_DESCRIPTION:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)

            description = data
            if description == "":
                return SNMP_ERR_NOERROR

            if len(description) > IP_ACCESS_DESCRIPTION_LEN:
                return self.set_request_error(SNMP_ERR_WRONGLENGTH)

            if len(re.search("^[0-9a-zA-Z._-]*", description[0:len(description)]).group(0)) != len(description):
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        indexes = self.get_request_indexes(IP_ACCESS_TABLE)
        ip_access_name = indexes[0]
        old_row_status = None

        # Retrieve exsting row
        old_row = IP_ACCESS_TABLE.getRow([AGENT.DisplayString(ip_access_name)])
        if old_row:
            old_row_status = self.get_table_row(IP_ACCESS_TBL_MEM_ROWSTATUS)

        if colnum == IP_ACCESS_TBL_MEM_ROWSTATUS:
            rowstatus = data
            if rowstatus == SNMP_ROW_CREATEANDGO:
                #if old_row_status and old_row_status == SNMP_ROW_ACTIVE:
                #    return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)

                szmsg = execute_cmd('cdbctl create/cdb/l2/acl_config/{0}'.format(ip_access_name))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    increase_data_sync_add("IP_ACCESS_TABLE", ip_access_name)

            if rowstatus == SNMP_ROW_DESTROY:
                if not old_row:
                    return self.set_request_error(SNMP_ERR_NOSUCHNAME)

                #szmsg = execute_cmd('cdbctl read/cdb/sys/get_flow_ref/{0}'.format(ip_access_name))
                #if szmsg.find('%') != -1:
                #    return self.set_request_error(SNMP_ERR_GENERR)

                szmsg = execute_cmd('cdbctl delete/cdb/l2/acl_config/{0}'.format(ip_access_name))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    increase_data_sync_del("IP_ACCESS_TABLE", ip_access_name)

        if colnum == IP_ACCESS_TBL_MEM_DESCRIPTION:
            description = data
            if description != "":
                szmsg = execute_cmd('cdbctl update/cdb/l2/acl_config/{0}/remark/add/{1}'.format(ip_access_name, description))
            else:
                szmsg = execute_cmd('cdbctl update/cdb/l2/acl_config/{0}/remark/del'.format(ip_access_name))
            if szmsg.find('%') != -1:
                   return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_acl_config = tbl_pb2.tbl_acl_config()
        try:
            tbl_acl_config.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_acl fail'
            return
            
        if tbl_acl_config.key.name[:6] == "#FLOW#" or tbl_acl_config.key.name[:8] == "#FW_MAT#" :
            return

        if operation == CDB_OPER_ADD:
            if check_data_sync_add("IP_ACCESS_TABLE", tbl_acl_config.key.name):
                decrease_data_sync_add("IP_ACCESS_TABLE", tbl_acl_config.key.name)
                return
            new_row = IP_ACCESS_TABLE.addRow([AGENT.DisplayString(tbl_acl_config.key.name)])
            new_row.setRowCell(IP_ACCESS_TBL_MEM_ROWSTATUS,   AGENT.Integer32(SNMP_ROW_ACTIVE))
            new_row.setRowCell(IP_ACCESS_TBL_MEM_DESCRIPTION, AGENT.DisplayString(tbl_acl_config.remark))

        if operation == CDB_OPER_DEL:
            if check_data_sync_del("IP_ACCESS_TABLE", tbl_acl_config.key.name):
                decrease_data_sync_del("IP_ACCESS_TABLE", tbl_acl_config.key.name)
                return
            IP_ACCESS_TABLE.delRow([AGENT.DisplayString(tbl_acl_config.key.name)])

        if operation == CDB_OPER_SET:
            if field_id == TBL_ACL_CONFIG_FLD_REMARK:
                old_row = IP_ACCESS_TABLE.addRow([AGENT.DisplayString(tbl_acl_config.key.name)])
                old_row.setRowCell(IP_ACCESS_TBL_MEM_DESCRIPTION, AGENT.DisplayString(tbl_acl_config.remark))

################################################################################
#
# ipaccessRuleTable enterprise.111.6
#
################################################################################
# Oid indexes
IP_ACCESS_RULE_TBL_MEM_NAME          = 1
IP_ACCESS_RULE_TBL_MEM_INDEX         = 2
IP_ACCESS_RULE_TBL_MEM_ACT           = 3
IP_ACCESS_RULE_TBL_MEM_IPPROTOCOL    = 4
IP_ACCESS_RULE_TBL_MEM_TCPSRCOP      = 5
IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT1   = 6
IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT2   = 7
IP_ACCESS_RULE_TBL_MEM_TCPDESTOP     = 8
IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT1  = 9
IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT2  = 10
IP_ACCESS_RULE_TBL_MEM_TCPFLAGOP     = 11
IP_ACCESS_RULE_TBL_MEM_TCPFLAGURG    = 12
IP_ACCESS_RULE_TBL_MEM_TCPFLAGACK    = 13
IP_ACCESS_RULE_TBL_MEM_TCPFLAGPSH    = 14
IP_ACCESS_RULE_TBL_MEM_TCPFLAGRST    = 15
IP_ACCESS_RULE_TBL_MEM_TCPFLAGSYN    = 16
IP_ACCESS_RULE_TBL_MEM_TCPFLAGFIN    = 17
IP_ACCESS_RULE_TBL_MEM_UDPSRCOP      = 18
IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT1   = 19
IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT2   = 20
IP_ACCESS_RULE_TBL_MEM_UDPDESTOP     = 21
IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT1  = 22
IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT2  = 23
IP_ACCESS_RULE_TBL_MEM_SRCIP         = 24
IP_ACCESS_RULE_TBL_MEM_SRCWILD       = 25
IP_ACCESS_RULE_TBL_MEM_DESTIP        = 26
IP_ACCESS_RULE_TBL_MEM_DESTWILD      = 27
IP_ACCESS_RULE_TBL_MEM_IPPRECEDENCE  = 28
IP_ACCESS_RULE_TBL_MEM_IPDSCP        = 29
IP_ACCESS_RULE_TBL_MEM_IPFRAGMENTS   = 30
IP_ACCESS_RULE_TBL_MEM_IPOPTIONS     = 31
IP_ACCESS_RULE_TBL_MEM_VLANID        = 32
IP_ACCESS_RULE_TBL_MEM_VLANID_WILD   = 33
IP_ACCESS_RULE_TBL_MEM_INNERVLANID   = 34
IP_ACCESS_RULE_TBL_MEM_INNERVLAN_WILD  = 35
IP_ACCESS_RULE_TBL_MEM_COS           = 36
IP_ACCESS_RULE_TBL_MEM_INNERCOS      = 37
IP_ACCESS_RULE_TBL_MEM_ETHERTYPE     = 38
IP_ACCESS_RULE_TBL_MEM_ETHERTYPEWILD = 39
IP_ACCESS_RULE_TBL_MEM_SRCMAC        = 40
IP_ACCESS_RULE_TBL_MEM_SRCMACWILD    = 41
IP_ACCESS_RULE_TBL_MEM_DESTMAC       = 42
IP_ACCESS_RULE_TBL_MEM_DESTMACWILD   = 43
IP_ACCESS_RULE_TBL_MEM_ROWSTATUS     = 50

# Macro definitions
IP_ACCESS_MAC_SA_FLAG = (1<<0)
IP_ACCESS_MAC_DA_FLAG = (1<<1)
IP_ACCESS_UNTAG_FLAG = (1<<2)
IP_ACCESS_VLAN_FLAG = (1<<3)
IP_ACCESS_INNER_VLAN_FLAG = (1<<4)
IP_ACCESS_ETHERTYPE_FLAG = (1<<5)
IP_ACCESS_IPSA_FLAG = (1<<6)
IP_ACCESS_IPDA_FLAG = (1<<7)
IP_ACCESS_IP_PROT_FLAG = (1<<8)
IP_ACCESS_SRC_PORT_FLAG = (1<<9)
IP_ACCESS_DST_PORT_FLAG = (1<<10)
IP_ACCESS_COS_FLAG = (1<<11)
IP_ACCESS_INNER_COS_FLAG = (1<<12)
IP_ACCESS_DSCP_FLAG = (1<<13)
IP_ACCESS_IP_FRAG_FLAG = (1<<14)
IP_ACCESS_IGMP_TYPE_FLAG = (1<<15)
IP_ACCESS_ICMP_TYPE_FLAG = (1<<16)
IP_ACCESS_ICMP_CODE_FLAG = (1<<17)
IP_ACCESS_TCP_FLAG = (1<<18)
IP_ACCESS_IP_PRECEDENCE_FLAG = (1<<19)
IP_ACCESS_TIME_RANGE_FLAG = (1<<20)
IP_ACCESS_INVALID_FLAG = (1<<21)
IP_ACCESS_TIMER_ACTIVE_FLAG = (1<<23)
IP_ACCESS_IP_OPTIONS_FLAG = (1<<23)
IP_ACCESS_TAP_ACTION_FLAG = (1<<24)
IP_ACCESS_TAP_TRUNCATION_FLAG = (1<<25)


IP_ACCESS_RULE_INDEX_MIN = 1
IP_ACCESS_RULE_INDEX_MAX = 65535

IP_ACCESS_RULE_IP_PROTOCOL_MIN = 0
IP_ACCESS_RULE_IP_PROTOCOL_MAX = 255
IP_ACCESS_RULE_IP_PROTOCOL_ANY = 256

IP_ACCESS_RULE_L4_PORT_MIN = 0
IP_ACCESS_RULE_L4_PORT_MAX = 65535

IP_ACCESS_RULE_IPPRECEDENCE_MIN = 0
IP_ACCESS_RULE_IPPRECEDENCE_MAX = 8

IP_ACCESS_RULE_IPDSCP_MIN = 0
IP_ACCESS_RULE_IPDSCP_MAX = 64

IP_ACCESS_RULE_IPFRAGMENTS_NONE         = 0
IP_ACCESS_RULE_IPFRAGMENTS_FIRST        = 1
IP_ACCESS_RULE_IPFRAGMENTS_NON_OR_FIRST = 2
IP_ACCESS_RULE_IPFRAGMENTS_SMALL        = 3
IP_ACCESS_RULE_IPFRAGMENTS_NON_FIRST    = 4
IP_ACCESS_RULE_IPFRAGMENTS_ANY          = 5
IP_ACCESS_RULE_IPFRAGMENTS_NOUSE        = 6

IP_ACCESS_RULE_VLANID_MIN = 0
IP_ACCESS_RULE_VLANID_MAX = 4094

IP_ACCESS_RULE_COS_MIN = 0
IP_ACCESS_RULE_COS_MAX = 8

IP_ACCESS_RULE_EHTER_TYPE_LEN_MAX = 6

IP_ACCESS_RULE_TABLE = None
class IpAccessRuleTableHandler(SnmpTableBase):
    def __init__(self, agentx):
        super(IpAccessRuleTableHandler, self).__init__(agentx)

    def check(self, colnum, var_type, data):
        # Index check
        indexes = self.get_request_indexes(IP_ACCESS_RULE_TABLE)
        ip_access_name = indexes[0]
        seq_no =  indexes[1]
        if len(ip_access_name) > IP_ACCESS_NAME_LEN:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        match = re.search("^[a-zA-Z][0-9a-zA-Z._-]*$", ip_access_name[0:len(ip_access_name)])
        if not match:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        if len(match.group(0)) != len(ip_access_name):
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        if seq_no < IP_ACCESS_RULE_INDEX_MIN or seq_no > IP_ACCESS_RULE_INDEX_MAX:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        old_row_status = None
        old_row = IP_ACCESS_RULE_TABLE.getRow([AGENT.DisplayString(ip_access_name), AGENT.Integer32(seq_no)])
        if old_row:
            old_row_status = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_ROWSTATUS)
        if colnum < IP_ACCESS_RULE_TBL_MEM_ROWSTATUS and old_row_status == SNMP_ROW_ACTIVE:
            #print "colnum=  ", colnum , "  old_row_status == SNMP_ROW_ACTIVE"
            return self.set_request_error(SNMP_ERR_NOTWRITABLE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_ACT:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < GLB_ACL_PERMIT or data > GLB_ACL_DENY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_IPPROTOCOL:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < IP_ACCESS_RULE_IP_PROTOCOL_MIN or data > IP_ACCESS_RULE_IP_PROTOCOL_ANY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_TCPSRCOP or colnum == IP_ACCESS_RULE_TBL_MEM_TCPDESTOP or\
                colnum == IP_ACCESS_RULE_TBL_MEM_UDPSRCOP or colnum == IP_ACCESS_RULE_TBL_MEM_UDPDESTOP:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != GLB_ACL_L4_MATCH_TYPE_EQUAL and data != GLB_ACL_L4_MATCH_TYPE_GREATER_THAN and\
                    data != GLB_ACL_L4_MATCH_TYPE_LESS_THAN and data != GLB_ACL_L4_MATCH_TYPE_RANGE and\
                    data != GLB_ACL_L4_MATCH_TYPE_INVALID and data != GLB_ACL_L4_MATCH_TYPE_ANY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT1 or colnum == IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT2 or\
                colnum == IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT1 or colnum == IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT2 or\
                colnum == IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT1 or colnum == IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT2 or\
                colnum == IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT1 or colnum == IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT2:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < IP_ACCESS_RULE_L4_PORT_MIN or data > IP_ACCESS_RULE_L4_PORT_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_TCPFLAGOP:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != GLB_ACL_MATCH_TYPE_ANY and data != GLB_ACL_MATCH_TYPE_ALL and\
                    data != GLB_ACL_MATCH_TYPE_INVALID:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_TCPFLAGURG or colnum == IP_ACCESS_RULE_TBL_MEM_TCPFLAGACK or\
                colnum == IP_ACCESS_RULE_TBL_MEM_TCPFLAGPSH or colnum == IP_ACCESS_RULE_TBL_MEM_TCPFLAGRST or\
                colnum == IP_ACCESS_RULE_TBL_MEM_TCPFLAGSYN or colnum == IP_ACCESS_RULE_TBL_MEM_TCPFLAGFIN or\
                colnum == IP_ACCESS_RULE_TBL_MEM_IPOPTIONS :
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != GLB_SNMP_TRUE and data != GLB_SNMP_FALSE:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_SRCIP or colnum == IP_ACCESS_RULE_TBL_MEM_SRCWILD or\
                colnum == IP_ACCESS_RULE_TBL_MEM_DESTIP or colnum == IP_ACCESS_RULE_TBL_MEM_DESTWILD:
            if var_type != ASN_IPADDRESS:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_IPPRECEDENCE:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < IP_ACCESS_RULE_IPPRECEDENCE_MIN or data > IP_ACCESS_RULE_IPPRECEDENCE_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_IPDSCP:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < IP_ACCESS_RULE_IPDSCP_MIN or data > IP_ACCESS_RULE_IPDSCP_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_IPFRAGMENTS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < IP_ACCESS_RULE_IPFRAGMENTS_NONE or data > IP_ACCESS_RULE_IPFRAGMENTS_NOUSE:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_VLANID or colnum == IP_ACCESS_RULE_TBL_MEM_INNERVLANID:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < IP_ACCESS_RULE_VLANID_MIN or data > GLB_ACL_VLAN_ID_INVALID:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
                
        if colnum == IP_ACCESS_RULE_TBL_MEM_VLANID_WILD or colnum == IP_ACCESS_RULE_TBL_MEM_INNERVLAN_WILD:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if len(data) > FLOW_RULE_EHTER_TYPE_LEN_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            match = re.search("^0x[0-9a-fA-F]{1,3}$", data)
            if not match:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            if len(match.group(0)) != len(data):
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_COS or colnum == IP_ACCESS_RULE_TBL_MEM_INNERCOS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data < IP_ACCESS_RULE_COS_MIN or data > IP_ACCESS_RULE_COS_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_ETHERTYPE or colnum == IP_ACCESS_RULE_TBL_MEM_ETHERTYPEWILD:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if len(data) > IP_ACCESS_RULE_EHTER_TYPE_LEN_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            match = re.search("^0x[0-9a-fA-F]{1,4}$", data)
            if not match:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            if len(match.group(0)) != len(data):
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_SRCMAC or colnum == IP_ACCESS_RULE_TBL_MEM_SRCMACWILD or\
                colnum == IP_ACCESS_RULE_TBL_MEM_DESTMAC or colnum == IP_ACCESS_RULE_TBL_MEM_DESTMACWILD:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if len(data) != GLB_SNMP_MAC_ADDR_LEN:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == IP_ACCESS_RULE_TBL_MEM_ROWSTATUS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != SNMP_ROW_CREATEANDGO and data != SNMP_ROW_DESTROY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            if data == SNMP_ROW_CREATEANDGO and old_row_status == SNMP_ROW_ACTIVE:
                return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
            if data == SNMP_ROW_DESTROY and not old_row_status:
                return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        indexes = self.get_request_indexes(IP_ACCESS_RULE_TABLE)
        ip_access_name = indexes[0]
        seq_no = indexes[1]
        old_row_status = None

        szmsg = execute_cmd('cdbctl read/cdb/l2/acl_config/')
        if szmsg.find('/key={0}/'.format(indexes[0])) == -1:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        old_row = IP_ACCESS_RULE_TABLE.getRow([AGENT.DisplayString(ip_access_name), AGENT.Integer32(seq_no)])
        if old_row:
            old_row_status = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_ROWSTATUS)

        if colnum == IP_ACCESS_RULE_TBL_MEM_ROWSTATUS and data == SNMP_ROW_DESTROY:
            if old_row_status == SNMP_ROW_ACTIVE:
                cmd = "cdbctl delete/cdb/l2/ace_config/{0}/{1}/snmp_set/true/".format(ip_access_name, seq_no)
                szmsg = execute_cmd(cmd)
                if szmsg.find('%') != -1:
                    print "set del flow fail szmsg = ", szmsg
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    tmp = ip_access_name + str(seq_no)
                    print "set IP_ACCESS_RULE_TABLE sub_del tmp = ", tmp
                    increase_data_sync_del("IP_ACCESS_RULE_TABLE", tmp)

        if colnum == IP_ACCESS_RULE_TBL_MEM_ROWSTATUS and data == SNMP_ROW_CREATEANDGO:
            #print "1. colnum == IP_ACCESS_RULE_TBL_MEM_ROWSTATUS data == SNMP_ROW_CREATEANDGO, old_row_status = ",old_row_status
            #if old_row:
            #    if old_row_status == SNMP_ROW_ACTIVE:
            #        return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)

            act             = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_ACT)
            ip_proto        = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_IPPROTOCOL)
            tcp_src_op      = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPSRCOP)
            tcp_src_port1   = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT1)
            tcp_src_port2   = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT2)
            tcp_dst_op      = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPDESTOP)
            tcp_dst_port1   = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT1)
            tcp_dst_port2   = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT2)
            tcp_flag_op     = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPFLAGOP)
            tcp_flag_urg    = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPFLAGURG)
            tcp_flag_ack    = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPFLAGACK)
            tcp_flag_psh    = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPFLAGPSH)
            tcp_flag_rst    = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPFLAGRST)
            tcp_flag_syn    = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPFLAGSYN)
            tcp_flag_fin    = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_TCPFLAGFIN)
            udp_src_op      = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_UDPSRCOP)
            udp_src_port1   = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT1)
            udp_src_port2   = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT2)
            udp_dst_op      = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_UDPDESTOP)
            udp_dst_port1   = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT1)
            udp_dst_port2   = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT2)
            src_ip          = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_SRCIP)
            src_ip_wild     = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_SRCWILD)
            dst_ip          = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_DESTIP)
            dst_ip_wild     = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_DESTWILD)
            ip_prec         = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_IPPRECEDENCE)
            ip_dscp         = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_IPDSCP)
            ip_frag         = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_IPFRAGMENTS)
            ip_opt          = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_IPOPTIONS)
            svlan_id        = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_VLANID)
            cvlan_id        = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_INNERVLANID)
            svlan_id_wild   = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_VLANID_WILD)
            cvlan_id_wild   = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_INNERVLAN_WILD)
            scos            = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_COS)
            ccos            = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_INNERCOS)
            ether_type      = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_ETHERTYPE)
            ether_type_wild = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_ETHERTYPEWILD)
            src_mac         = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_SRCMAC)
            src_mac_wild    = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_SRCMACWILD)
            dst_mac         = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_DESTMAC)
            dst_mac_wild    = self.get_table_row(IP_ACCESS_RULE_TBL_MEM_DESTMACWILD)

            #print "IP_ACCESS Rule Table set"
            #print "act:{0}, ip_proto:{1}".format(act, ip_proto)
            #print "tcp_src_op:{0}, tcp_src_port1:{1}, tcp_src_port2:{2}, tcp_dst_op:{3}, tcp_dst_port1:{4}, tcp_dst_port2:{5}".format(tcp_src_op, tcp_src_port1, tcp_src_port2, tcp_dst_op, tcp_dst_port1, tcp_dst_port2)
            #print "tcp_flag_op:{0}, tcp_flag_urg:{1}, tcp_flag_ack:{2}, tcp_flag_psh:{3}, tcp_flag_rst:{4}, tcp_flag_syn:{5}, tcp_flag_fin:{6}".format(tcp_flag_op, tcp_flag_urg, tcp_flag_ack, tcp_flag_psh, tcp_flag_rst, tcp_flag_syn, tcp_flag_fin)
            #print "udp_src_op:{0}, udp_src_port1:{1}, udp_src_port2:{2}, udp_dst_op:{3}, udp_dst_port1:{4}, udp_dst_port2:{5}".format(udp_src_op, udp_src_port1, udp_src_port2, udp_dst_op, udp_dst_port1, udp_dst_port2)
            #print "src_ip:{0}, src_ip_wild:{1}, dst_ip:{2}, dst_ip_wild:{3}, ip_prec:{4}, ip_dscp:{5}, ip_frag:{6}, ip_opt:{7}".format(src_ip, src_ip_wild, dst_ip, dst_ip_wild, ip_prec, ip_dscp, ip_frag, ip_opt)
            #print "truncation:{0}".format(truncation)
            #print "svlan_id:{0}, cvlan_id:{1}, scos:{2}, ccos:{3}, ether_type:{4}, ether_type_wild:{5}".format(svlan_id, cvlan_id, scos, ccos, ether_type, ether_type_wild)
            #print "src_mac:{0}, src_mac_wild:{1}, dst_mac:{2}, dst_mac_wild:{3}".format(src_mac, src_mac_wild, dst_mac,dst_mac_wild)
            #print ""

            if act == GLB_ACL_PERMIT:
                filter = "permit"
            else:
                filter = "deny"

            if ip_proto == 256:
                ip_proto_type = "any"
                ip_proto_num = ""
            else:
                ip_proto_type = "protocol_num"
                ip_proto_num = str(ip_proto)

            src_port_op = ""
            dst_port_op = ""
            src_port1 = ""
            src_port2 = ""
            dst_port1 = ""
            dst_port2 = ""
            str_tcp_flag = ""
            str_tcp_urg = ""
            str_tcp_ack = ""
            str_tcp_psh = ""
            str_tcp_rst = ""
            str_tcp_syn = ""
            str_tcp_fin = ""
            if ip_proto == GLB_ACL_IP_PROTOCOL_TCP:
                if tcp_src_op == GLB_ACL_L4_MATCH_TYPE_EQUAL:
                    src_port_op = "eq"
                    src_port1 = str(tcp_src_port1)
                elif tcp_src_op == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                    src_port_op = "gt"
                    src_port1 = str(tcp_src_port1)
                elif tcp_src_op == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                    src_port_op = "lt"
                    src_port1 = str(tcp_src_port1)
                elif tcp_src_op == GLB_ACL_L4_MATCH_TYPE_RANGE:
                    src_port_op = "range"
                    src_port1 = str(tcp_src_port1)
                    src_port2 = str(tcp_src_port2)
                elif tcp_src_op == GLB_ACL_L4_MATCH_TYPE_ANY:
                    src_port_op = "any"

                if tcp_dst_op == GLB_ACL_L4_MATCH_TYPE_EQUAL:
                    dst_port_op = "eq"
                    dst_port1 = str(tcp_dst_port1)
                elif tcp_dst_op == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                    dst_port_op = "gt"
                    dst_port1 = str(tcp_dst_port1)
                elif tcp_dst_op == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                    dst_port_op = "lt"
                    dst_port1 = str(tcp_dst_port1)
                elif tcp_dst_op == GLB_ACL_L4_MATCH_TYPE_RANGE:
                    dst_port_op = "range"
                    dst_port1 = str(tcp_dst_port1)
                    dst_port2 = str(tcp_dst_port2)
                elif tcp_dst_op == GLB_ACL_L4_MATCH_TYPE_ANY:
                    tcp_dst_op = "any"

                if tcp_flag_op == GLB_ACL_MATCH_TYPE_ANY or tcp_flag_op == GLB_ACL_MATCH_TYPE_ALL:
                    if tcp_flag_op == GLB_ACL_MATCH_TYPE_ANY:
                        str_tcp_flag = "match-any"
                    if tcp_flag_op == GLB_ACL_MATCH_TYPE_ALL:
                        str_tcp_flag = "match-all"
                    if tcp_flag_urg == GLB_SNMP_TRUE:
                        str_tcp_urg = "urg"
                    if tcp_flag_ack == GLB_SNMP_TRUE:
                        str_tcp_ack = "ack"
                    if tcp_flag_psh == GLB_SNMP_TRUE:
                        str_tcp_psh = "psh"
                    if tcp_flag_rst == GLB_SNMP_TRUE:
                        str_tcp_rst = "rst"
                    if tcp_flag_syn == GLB_SNMP_TRUE:
                        str_tcp_syn = "syn"
                    if tcp_flag_fin == GLB_SNMP_TRUE:
                        str_tcp_fin = "fin"

            elif ip_proto == GLB_ACL_IP_PROTOCOL_UDP:
                if udp_src_op == GLB_ACL_L4_MATCH_TYPE_EQUAL:
                    src_port_op = "eq"
                    src_port1 = str(udp_src_port1)
                elif udp_src_op == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                    src_port_op = "gt"
                    src_port1 = str(udp_src_port1)
                elif udp_src_op == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                    src_port_op = "lt"
                    src_port1 = str(udp_src_port1)
                elif udp_src_op == GLB_ACL_L4_MATCH_TYPE_RANGE:
                    src_port_op = "range"
                    src_port1 = str(udp_src_port1)
                    src_port2 = str(udp_src_port2)
                elif udp_src_op == GLB_ACL_L4_MATCH_TYPE_ANY:
                    src_port_op = "any"

                if udp_dst_op == GLB_ACL_L4_MATCH_TYPE_EQUAL:
                    dst_port_op = "eq"
                    dst_port1 = str(udp_dst_port1)
                elif udp_dst_op == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                    dst_port_op = "gt"
                    dst_port1 = str(udp_dst_port1)
                elif udp_dst_op == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                    dst_port_op = "lt"
                    dst_port1 = str(udp_dst_port1)
                elif udp_dst_op == GLB_ACL_L4_MATCH_TYPE_RANGE:
                    dst_port_op = "range"
                    dst_port1 = str(udp_dst_port1)
                    dst_port2 = str(udp_dst_port2)
                elif udp_dst_op == GLB_ACL_L4_MATCH_TYPE_ANY:
                    dst_port_op = "any"

            if ip_prec == GLB_ACL_INVALID_IP_PRECEDENCE:
                str_ip_prec = ""
            else:
                str_ip_prec = str(ip_prec)

            if ip_dscp == GLB_ACL_INVALID_IP_DSCP:
                str_ip_dscp = ""
            else:
                str_ip_dscp = str(ip_dscp)

            if ip_frag == IP_ACCESS_RULE_IPFRAGMENTS_FIRST:
                str_ip_frag = "first-fragment"
            elif ip_frag == IP_ACCESS_RULE_IPFRAGMENTS_NON_FIRST:
                str_ip_frag = "non-first-fragment"
            elif ip_frag == IP_ACCESS_RULE_IPFRAGMENTS_NONE:
                str_ip_frag = "non-fragment"
            elif ip_frag == IP_ACCESS_RULE_IPFRAGMENTS_NON_OR_FIRST:
                str_ip_frag = "non-or-first-fragment"
            elif ip_frag == IP_ACCESS_RULE_IPFRAGMENTS_SMALL:
                str_ip_frag = "small-fragment"
            elif ip_frag == IP_ACCESS_RULE_IPFRAGMENTS_ANY:
                str_ip_frag = "any-fragment"
            else:
                str_ip_frag = ""

            if ip_opt == GLB_SNMP_TRUE:
                str_ip_opt = "options"
            else:
                str_ip_opt = ""

            str_trunc = ""

            if scos == GLB_ACL_INVALID_COS:
                str_scos = ""
            else:
                str_scos = str(scos)

            if ccos == GLB_ACL_INVALID_COS:
                str_ccos = ""
            else:
                str_ccos = str(ccos)

            if GLB_ACL_VLAN_ID_INVALID == svlan_id:
                str_svlan_id = ""
                str_svlan_id_wild = ""
                str_s_id = ""
            else:
                str_svlan_id = str(svlan_id)
                str_svlan_id_wild = str(svlan_id_wild)
                str_s_id = "svlan"
              
            if GLB_ACL_VLAN_ID_INVALID == cvlan_id:
                str_cvlan_id = ""
                str_cvlan_id_wild = ""
                str_c_id = ""
            else:
                str_cvlan_id = str(cvlan_id)
                str_cvlan_id_wild = str(cvlan_id_wild)
                str_c_id = "cvlan"  
            
            str_mac_sa_type = ""
            str_mac_sa = ""
            str_mac_sa_wild = ""
            str_mac_da_type = ""
            str_mac_da = ""
            str_mac_da_wild = ""

            if len(src_mac) == GLB_SNMP_MAC_ADDR_LEN:
                if len(src_mac_wild) != GLB_SNMP_MAC_ADDR_LEN:
                    return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
                mac = src_mac.encode("hex")
                str_mac_sa = mac[0:4]+"."+mac[4:8]+"."+mac[8:12]
                mac = src_mac_wild.encode("hex")
                str_mac_sa_wild = mac[0:4]+"."+mac[4:8]+"."+mac[8:12]
                str_mac_sa_type = "src_mac"

            if len(dst_mac) == GLB_SNMP_MAC_ADDR_LEN:
                if len(dst_mac_wild) != GLB_SNMP_MAC_ADDR_LEN:
                    return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
                mac = dst_mac.encode("hex")
                str_mac_da = mac[0:4]+"."+mac[4:8]+"."+mac[8:12]
                mac = dst_mac_wild.encode("hex")
                str_mac_da_wild = mac[0:4]+"."+mac[4:8]+"."+mac[8:12]
                str_mac_da_type = "dest_mac"

            str_ether_type = ""
            if len(ether_type):
                if len(ether_type_wild) == 0:
                    return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
                str_ether_type = "ether_type"

            if len(str_ether_type):
                cmd = "cdbctl create/cdb/l2/ace_config/{0}/{1}/{2}/src_mac/{3}/{4}/{5}/dest_mac/{6}/{7}/{8}/" \
                        "vlan/{9}/{10}/{11}/inner_vlan/{12}/{13}/{14}/cos/{15}/{16}/0/inner_cos/{17}/{18}/0/"   \
                        "ether_type/{19}/{20}/{21}/src-ip////dst-ip////dscp////precedence////frag//"     \
                        "l3-protocol////src-l4-port////////igmp_type//icmp_type///tcpflag////////udf/////timerange//"     \
                        "options//truncation/{22}/snmp_set/true/"  \
                        .format(ip_access_name,str(seq_no),filter,str_mac_sa_type, str_mac_sa, str_mac_sa_wild,
                        str_mac_da_type, str_mac_da, str_mac_da_wild, str_s_id, str_svlan_id, str_svlan_id_wild, 
                        str_c_id, str_cvlan_id, str_cvlan_id_wild, str_scos, str_scos, str_ccos, str_ccos, str_ether_type,
                        ether_type, ether_type_wild, str_trunc)
            else:
                cmd = "cdbctl create/cdb/l2/ace_config/{0}/{1}/{2}/src_mac/{3}/{4}/{5}/dest_mac/{6}/{7}/{8}/" \
                        "vlan/{9}/{10}/{11}/inner_vlan/{12}/{13}/{14}/cos/{15}/{16}/0/inner_cos/{17}/{18}/0/"   \
                        "ether_type////src-ip/{19}/{20}/{21}/dst-ip/{22}/{23}/{24}/dscp/{25}/{26}/0/"  \
                        "precedence/{27}/{28}/0/frag/{29}/l3-protocol/{30}/{31}/0/src-l4-port/{32}/{33}/{34}/"    \
                        "dest-l4-port/{35}/{36}/{37}/igmp_type//icmp_type///tcpflag/{38}/{39}/{40}/{41}/{42}/{43}/{44}/"    \
                        "udf/////timerange//options/{45}/truncation/{46}/snmp_set/true/"  \
                        .format(ip_access_name, str(seq_no), filter, str_mac_sa_type, str_mac_sa, str_mac_sa_wild,
                        str_mac_da_type, str_mac_da, str_mac_da_wild, str_s_id, str_svlan_id, str_svlan_id_wild, str_c_id,
                        str_cvlan_id, str_cvlan_id_wild, str_scos, str_scos, str_ccos, str_ccos, src_ip, src_ip, src_ip_wild,
                        dst_ip, dst_ip, dst_ip_wild, str_ip_dscp, str_ip_dscp, str_ip_prec, str_ip_prec,str_ip_frag,
                        ip_proto_type, ip_proto_num, src_port_op, src_port1, src_port2, dst_port_op, dst_port1, dst_port2,
                        str_tcp_flag, str_tcp_ack, str_tcp_fin, str_tcp_psh, str_tcp_rst, str_tcp_syn, str_tcp_urg,
                        str_ip_opt, str_trunc)

            #print cmd
            szmsg = execute_cmd(cmd)
            #print szmsg

            if szmsg.find('%') != -1 and szmsg.find('should be added') == -1:
                print "set add flow fail szmsg = ", szmsg
                return self.set_request_error(SNMP_ERR_GENERR)
            else:
                tmp = ip_access_name + str(seq_no)
                print "set IP_ACCESS_RULE_TABLE sub_add tmp = ", tmp
                increase_data_sync_add("IP_ACCESS_RULE_TABLE", tmp)
                print "set add sync IP_ACCESS_RULE_TABLE tmp = ", tmp
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_ace_config = tbl_pb2.tbl_ace_config()
        try:
            tbl_ace_config.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_ace_config fail'
            return

        #if tbl_ace_config.tap_snmp_set != 0:
        #    return
        
        if tbl_ace_config.key.acl_name[:6] == "#FLOW#" or tbl_ace_config.key.acl_name[:8] == "#FW_MAT#" :
            return

        if operation == CDB_OPER_ADD:
            tmp = tbl_ace_config.key.acl_name + str(tbl_ace_config.key.seq_no)
            print "sub_add tmp = ", tmp
            if check_data_sync_add("IP_ACCESS_RULE_TABLE", tmp):
                decrease_data_sync_add("IP_ACCESS_RULE_TABLE", tmp)
                print "check return IP_ACCESS_RULE_TABLE sub_add tmp = ", tmp
                return
            if tbl_ace_config.tap_snmp_set != 0:
                return
            new_row = IP_ACCESS_RULE_TABLE.addRow([AGENT.DisplayString(tbl_ace_config.key.acl_name), AGENT.Integer32(tbl_ace_config.key.seq_no)])
            #print tbl_ace_config
            if 0 == tbl_ace_config.deny:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_ACT, AGENT.Integer32(GLB_ACL_PERMIT))
            if (tbl_ace_config.key_flags & IP_ACCESS_IP_PROT_FLAG ) != 0:
                if tbl_ace_config.ip_protocol_mask == 0xff:
                    new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_IPPROTOCOL, AGENT.Integer32(IP_ACCESS_RULE_IP_PROTOCOL_ANY))
                else:
                    new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_IPPROTOCOL, AGENT.Integer32(tbl_ace_config.ip_protocol))
            if tbl_ace_config.ip_protocol == GLB_ACL_IP_PROTOCOL_TCP:
                if (tbl_ace_config.key_flags & IP_ACCESS_SRC_PORT_FLAG) != 0:
                    print tbl_ace_config.l4_src_port_type, tbl_ace_config.l4_src_port, tbl_ace_config.l4_src_port_mask
                    new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPSRCOP, AGENT.Integer32(tbl_ace_config.l4_src_port_type))
                    if tbl_ace_config.l4_src_port_type == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN :
                        new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port-1))
                    elif tbl_ace_config.l4_src_port_type == GLB_ACL_L4_MATCH_TYPE_LESS_THAN :
                        new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port_mask+1))
                    else:
                        new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port))
                        new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT2, AGENT.Integer32(tbl_ace_config.l4_src_port_mask))
                if (tbl_ace_config.key_flags & IP_ACCESS_DST_PORT_FLAG) != 0:
                    new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPDESTOP, AGENT.Integer32(tbl_ace_config.l4_dst_port_type))
                    if tbl_ace_config.l4_dst_port_type == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN :
                        new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port-1))
                    elif tbl_ace_config.l4_dst_port_type == GLB_ACL_L4_MATCH_TYPE_LESS_THAN :
                        new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port_mask+1))
                    else:
                        new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port))
                        new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT2, AGENT.Integer32(tbl_ace_config.l4_dst_port_mask))
            if tbl_ace_config.ip_protocol == GLB_ACL_IP_PROTOCOL_UDP:
               if (tbl_ace_config.key_flags & IP_ACCESS_SRC_PORT_FLAG) != 0:
                   new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_UDPSRCOP, AGENT.Integer32(tbl_ace_config.l4_src_port_type))
                   if tbl_ace_config.l4_src_port_type == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                       new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port - 1))
                   elif tbl_ace_config.l4_src_port_type == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                       new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port_mask + 1))
                   else:
                       new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT1, AGENT.Integer32(tbl_ace_config.l4_src_port))
                       new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT2, AGENT.Integer32(tbl_ace_config.l4_src_port_mask))

               if (tbl_ace_config.key_flags & IP_ACCESS_DST_PORT_FLAG) != 0:
                   new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_UDPDESTOP, AGENT.Integer32(tbl_ace_config.l4_dst_port_type))
                   if tbl_ace_config.l4_dst_port_type == GLB_ACL_L4_MATCH_TYPE_GREATER_THAN:
                       new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port - 1))
                   elif tbl_ace_config.l4_dst_port_type == GLB_ACL_L4_MATCH_TYPE_LESS_THAN:
                       new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port_mask + 1))
                   else:
                       new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT1, AGENT.Integer32(tbl_ace_config.l4_dst_port))
                       new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT2, AGENT.Integer32(tbl_ace_config.l4_dst_port_mask))
            if (tbl_ace_config.key_flags & IP_ACCESS_TCP_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPFLAGOP, AGENT.Integer32(tbl_ace_config.tcp_flags))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_URG:
                    new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPFLAGURG, AGENT.Integer32(GLB_SNMP_TRUE))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_ACK:
                    new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPFLAGACK, AGENT.Integer32(GLB_SNMP_TRUE))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_PSH:
                    new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPFLAGPSH, AGENT.Integer32(GLB_SNMP_TRUE))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_RST:
                    new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPFLAGRST, AGENT.Integer32(GLB_SNMP_TRUE))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_SYN:
                    new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPFLAGSYN, AGENT.Integer32(GLB_SNMP_TRUE))
                if tbl_ace_config.tcp_flags_mask & GLB_ACL_TCP_FLAG_FIN:
                    new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_TCPFLAGFIN, AGENT.Integer32(GLB_SNMP_TRUE))
            if (tbl_ace_config.key_flags & IP_ACCESS_IPSA_FLAG) != 0 :
                ipsa = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_ace_config.src_ip.ipv4_addr)))
                ipsa_mask = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_ace_config.src_ip_mask.ipv4_addr)))
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_SRCIP, AGENT.IpAddress(ipsa))
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_SRCWILD, AGENT.IpAddress(ipsa_mask))
            if (tbl_ace_config.key_flags & IP_ACCESS_IPDA_FLAG) != 0:
                ipda = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_ace_config.dst_ip.ipv4_addr)))
                ipda_mask = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_ace_config.dst_ip_mask.ipv4_addr)))
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_DESTIP, AGENT.IpAddress(ipda))
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_DESTWILD, AGENT.IpAddress(ipda_mask))

            if (tbl_ace_config.key_flags & IP_ACCESS_IP_PRECEDENCE_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_IPPRECEDENCE, AGENT.Integer32(tbl_ace_config.ip_precedence))
            if (tbl_ace_config.key_flags & IP_ACCESS_DSCP_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_IPDSCP, AGENT.Integer32(tbl_ace_config.dscp))
            if (tbl_ace_config.key_flags & IP_ACCESS_IP_FRAG_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_IPFRAGMENTS, AGENT.Integer32(tbl_ace_config.ip_frag))
            if (tbl_ace_config.key_flags & IP_ACCESS_IP_OPTIONS_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_IPOPTIONS, AGENT.Integer32(GLB_SNMP_TRUE))
            if (tbl_ace_config.key_flags & IP_ACCESS_VLAN_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_VLANID, AGENT.Integer32(tbl_ace_config.svlan))
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_VLANID_WILD, AGENT.DisplayString(hex(tbl_ace_config.svlan_mask)))
            else:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_VLANID, AGENT.Integer32(GLB_ACL_VLAN_ID_INVALID))
            if (tbl_ace_config.key_flags & IP_ACCESS_INNER_VLAN_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_INNERVLANID, AGENT.Integer32(tbl_ace_config.cvlan))
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_INNERVLAN_WILD, AGENT.DisplayString(hex(tbl_ace_config.cvlan_mask)))
            else:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_INNERVLANID, AGENT.Integer32(GLB_ACL_VLAN_ID_INVALID))

            if (tbl_ace_config.key_flags & IP_ACCESS_COS_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_COS, AGENT.Integer32(tbl_ace_config.svlan_cos))
            if (tbl_ace_config.key_flags & IP_ACCESS_INNER_COS_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_INNERCOS, AGENT.Integer32(tbl_ace_config.cvlan_cos))
            if (tbl_ace_config.key_flags & IP_ACCESS_ETHERTYPE_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_ETHERTYPE, AGENT.DisplayString(hex(tbl_ace_config.ether_type)))
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_ETHERTYPEWILD, AGENT.DisplayString(hex(tbl_ace_config.ether_type_mask)))
            if (tbl_ace_config.key_flags & IP_ACCESS_MAC_SA_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_SRCMAC, AGENT.OctetString(tbl_ace_config.src_mac.mac))
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_SRCMACWILD, AGENT.OctetString(tbl_ace_config.src_mac_mask.mac))
            if (tbl_ace_config.key_flags & IP_ACCESS_MAC_DA_FLAG) != 0:
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_DESTMAC, AGENT.OctetString(tbl_ace_config.dst_mac.mac))
                new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_DESTMACWILD, AGENT.OctetString(tbl_ace_config.dst_mac_mask.mac))
            #print "acl SNMP_ROW_ACTIVE"
            new_row.setRowCell(IP_ACCESS_RULE_TBL_MEM_ROWSTATUS, AGENT.Integer32(SNMP_ROW_ACTIVE))

        if operation == CDB_OPER_DEL:
            tmp = tbl_ace_config.key.acl_name + str(tbl_ace_config.key.seq_no)
            print "sub_del tmp = ", tmp
            if check_data_sync_del("IP_ACCESS_RULE_TABLE", tmp):
                decrease_data_sync_del("IP_ACCESS_RULE_TABLE", tmp)
                print "check return IP_ACCESS_RULE_TABLE sub_del tmp = ", tmp
                return
            if tbl_ace_config.tap_snmp_set != 0:
                return
            IP_ACCESS_RULE_TABLE.delRow([AGENT.DisplayString(tbl_ace_config.key.acl_name), AGENT.Integer32(tbl_ace_config.key.seq_no)])


################################################################################
#
# Module Init
#
################################################################################
def init(agent_obj, subscriber):
    global AGENT
    AGENT = agent_obj
    enterpriseID = AGENT.get_enterprise_id()

    flow_table_handler = FlowTableHandler(AGENT)
    global FLOW_TABLE
    FLOW_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.111.1".format(enterpriseID),
        indexes = [
            AGENT.DisplayString(""),
        ],
        columns = [
            (FLOW_TBL_MEM_DECAP, AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (FLOW_TBL_MEM_ROWSTATUS,  AGENT.Integer32(SNMP_ROW_NOTINSERVICE), 1),
            (FLOW_TBL_MEM_DESCRIPTION, AGENT.DisplayString(""), 1),
        ],
        extendable = True,
        nodehandle = flow_table_handler.node_handler
    )
    FLOW_TABLE.set_allow_creation(True)
    FLOW_TABLE.set_rowstatus_column(FLOW_TBL_MEM_ROWSTATUS)
    subscriber.subscribeTable(TBL_ACL_CONFIG, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, flow_table_handler.subscribe_handler)

    flow_rule_table_handler = FlowRuleTableHandler(AGENT)
    global FLOW_RULE_TABLE
    FLOW_RULE_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.111.2".format(enterpriseID),
        indexes = [
            AGENT.DisplayString(""),
            AGENT.Integer32(0),
        ],
        columns = [
            (FLOW_RULE_TBL_MEM_ACT,           AGENT.Integer32(GLB_ACL_DENY), 1),
            (FLOW_RULE_TBL_MEM_IPPROTOCOL,    AGENT.Integer32(FLOW_RULE_IP_PROTOCOL_ANY), 1),
            (FLOW_RULE_TBL_MEM_TCPSRCOP,      AGENT.Integer32(GLB_ACL_L4_MATCH_TYPE_INVALID), 1),
            (FLOW_RULE_TBL_MEM_TCPSRCPORT1,   AGENT.Integer32(0), 1),
            (FLOW_RULE_TBL_MEM_TCPSRCPORT2,   AGENT.Integer32(0), 1),
            (FLOW_RULE_TBL_MEM_TCPDESTOP,     AGENT.Integer32(GLB_ACL_L4_MATCH_TYPE_INVALID), 1),
            (FLOW_RULE_TBL_MEM_TCPDESTPORT1,  AGENT.Integer32(0), 1),
            (FLOW_RULE_TBL_MEM_TCPDESTPORT2,  AGENT.Integer32(0), 1),
            (FLOW_RULE_TBL_MEM_TCPFLAGOP,     AGENT.Integer32(GLB_ACL_MATCH_TYPE_INVALID), 1),
            (FLOW_RULE_TBL_MEM_TCPFLAGURG,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (FLOW_RULE_TBL_MEM_TCPFLAGACK,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (FLOW_RULE_TBL_MEM_TCPFLAGPSH,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (FLOW_RULE_TBL_MEM_TCPFLAGRST,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (FLOW_RULE_TBL_MEM_TCPFLAGSYN,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (FLOW_RULE_TBL_MEM_TCPFLAGFIN,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (FLOW_RULE_TBL_MEM_UDPSRCOP,      AGENT.Integer32(GLB_ACL_L4_MATCH_TYPE_INVALID), 1),
            (FLOW_RULE_TBL_MEM_UDPSRCPORT1,   AGENT.Integer32(0), 1),
            (FLOW_RULE_TBL_MEM_UDPSRCPORT2,   AGENT.Integer32(0), 1),
            (FLOW_RULE_TBL_MEM_UDPDESTOP,     AGENT.Integer32(GLB_ACL_L4_MATCH_TYPE_INVALID), 1),
            (FLOW_RULE_TBL_MEM_UDPDESTPORT1,  AGENT.Integer32(0), 1),
            (FLOW_RULE_TBL_MEM_UDPDESTPORT2,  AGENT.Integer32(0), 1),
            (FLOW_RULE_TBL_MEM_SRCIP,         AGENT.IpAddress("0.0.0.0"), 1),
            (FLOW_RULE_TBL_MEM_SRCWILD,       AGENT.IpAddress("255.255.255.255"), 1),
            (FLOW_RULE_TBL_MEM_DESTIP,        AGENT.IpAddress("0.0.0.0"), 1),
            (FLOW_RULE_TBL_MEM_DESTWILD,      AGENT.IpAddress("255.255.255.255"), 1),
            (FLOW_RULE_TBL_MEM_IPPRECEDENCE,  AGENT.Integer32(GLB_ACL_INVALID_IP_PRECEDENCE), 1),
            (FLOW_RULE_TBL_MEM_IPDSCP,        AGENT.Integer32(GLB_ACL_INVALID_IP_DSCP), 1),
            (FLOW_RULE_TBL_MEM_IPFRAGMENTS,   AGENT.Integer32(FLOW_RULE_IPFRAGMENTS_NOUSE), 1),
            (FLOW_RULE_TBL_MEM_IPOPTIONS,     AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (FLOW_RULE_TBL_MEM_TRUNCATION,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (FLOW_RULE_TBL_MEM_VLANID,        AGENT.Integer32(GLB_ACL_VLAN_ID_INVALID), 1),
            (FLOW_RULE_TBL_MEM_VLANID_WILD,   AGENT.DisplayString('0xfff'), 1),
            (FLOW_RULE_TBL_MEM_INNERVLANID,   AGENT.Integer32(GLB_ACL_VLAN_ID_INVALID), 1),
            (FLOW_RULE_TBL_MEM_INNERVLAN_WILD,   AGENT.DisplayString('0xfff'), 1),
            (FLOW_RULE_TBL_MEM_COS,           AGENT.Integer32(GLB_ACL_INVALID_COS), 1),
            (FLOW_RULE_TBL_MEM_INNERCOS,      AGENT.Integer32(GLB_ACL_INVALID_COS), 1),
            (FLOW_RULE_TBL_MEM_ETHERTYPE,     AGENT.DisplayString(''), 1),
            (FLOW_RULE_TBL_MEM_ETHERTYPEWILD, AGENT.DisplayString(''), 1),
            (FLOW_RULE_TBL_MEM_SRCMAC,        AGENT.OctetString(''), 1),
            (FLOW_RULE_TBL_MEM_SRCMACWILD,    AGENT.OctetString(''), 1),
            (FLOW_RULE_TBL_MEM_DESTMAC,       AGENT.OctetString(''), 1),
            (FLOW_RULE_TBL_MEM_DESTMACWILD,   AGENT.OctetString(''), 1),
            (FLOW_RULE_TBL_MEM_EDIT_MAC_DA, AGENT.OctetString(''), 1),
            (FLOW_RULE_TBL_MEM_EDIT_MAC_SA, AGENT.OctetString(''), 1),
            (FLOW_RULE_TBL_MEM_EDIT_IP_DA_EN, AGENT.Integer32(2), 1),
            (FLOW_RULE_TBL_MEM_EDIT_IP_DA, AGENT.IpAddress("0.0.0.0"), 1),
            (FLOW_RULE_TBL_MEM_EDIT_IP_SA_EN, AGENT.Integer32(2), 1),
            (FLOW_RULE_TBL_MEM_EDIT_IP_SA, AGENT.IpAddress("0.0.0.0"), 1),
            (FLOW_RULE_TBL_MEM_STRIP_HEADER, AGENT.Integer32(2), 1),
            (FLOW_RULE_TBL_MEM_VNI, AGENT.Integer32(2), 1),
            (FLOW_RULE_TBL_MEM_VXLAN_VNI, AGENT.Integer32(0), 1),
            (FLOW_RULE_TBL_MEM_VXLAN_VNI_MASK, AGENT.DisplayString(''), 1),
            (FLOW_RULE_TBL_MEM_UNTAG, AGENT.Integer32(2), 1),
            (FLOW_RULE_TBL_MEM_MARK_SRC_VLAN, AGENT.Integer32(0), 1),
            (FLOW_RULE_TBL_MEM_ROWSTATUS,     AGENT.Integer32(SNMP_ROW_NOTINSERVICE), 1),
        ],
        extendable = True,
        nodehandle = flow_rule_table_handler.node_handler
    )
    FLOW_RULE_TABLE.set_allow_creation(True)
    FLOW_RULE_TABLE.set_rowstatus_column(FLOW_RULE_TBL_MEM_ROWSTATUS)
    subscriber.subscribeTable(TBL_ACE_CONFIG, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, flow_rule_table_handler.subscribe_handler)

    flow_if_stats_table_handler = FlowIfStatsTableHandler(AGENT)
    global FLOW_IF_STATS_TABLE
    FLOW_IF_STATS_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.111.3".format(enterpriseID),
        indexes = [
            AGENT.Integer32(0),
            AGENT.DisplayString(""),
            AGENT.Integer32(0),
        ],
        columns = [
            (FLOW_IF_STATS_TBL_MEM_MATCH_PKTS,  AGENT.Counter64(0), 0),
            (FLOW_IF_STATS_TBL_MEM_MATCH_BYTES, AGENT.Counter64(0), 0),
        ],
        extendable = True,
        nodehandle = flow_if_stats_table_handler.node_handler
    )
    subscriber.subscribeTable(TBL_ACL_CONFIG, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, flow_if_stats_table_handler.subscribe_handler)
    subscriber.subscribeTable(TBL_ACE_CONFIG, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, flow_if_stats_table_handler.subscribe_handler)
    subscriber.subscribeTable(TBL_TAP_GROUP_INGRESS_FLOW, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, flow_if_stats_table_handler.subscribe_handler)
    GLB_TABLE_DICT["flow_stats"] = FLOW_IF_STATS_TABLE


    flow_if_egress_stats_table_handler = FlowIfEgressStatsTableHandler(AGENT)
    global FLOW_IF_EGRESS_STATS_TABLE
    FLOW_IF_EGRESS_STATS_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.111.4".format(enterpriseID),
        indexes = [
            AGENT.Integer32(0),
            AGENT.Integer32(0),
        ],
        columns = [
            (FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_PKTS,  AGENT.Counter64(0), 0),
            (FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_BYTES, AGENT.Counter64(0), 0),
        ],
        extendable = True,
        nodehandle = flow_if_egress_stats_table_handler.node_handler
    )
    subscriber.subscribeTable(TBL_INTERFACE, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, flow_if_egress_stats_table_handler.subscribe_handler)
    subscriber.subscribeTable(TBL_ACL_CONFIG, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, flow_if_egress_stats_table_handler.subscribe_handler)
    subscriber.subscribeTable(TBL_ACE_CONFIG, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, flow_if_egress_stats_table_handler.subscribe_handler)
    GLB_TABLE_DICT["flow_egress_stats"] = FLOW_IF_EGRESS_STATS_TABLE


    ip_access_table_handler = IpAccessTableHandler(AGENT)
    global IP_ACCESS_TABLE
    IP_ACCESS_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.111.5".format(enterpriseID),
        indexes = [
            AGENT.DisplayString(""),
        ],
        columns = [
            (IP_ACCESS_TBL_MEM_ROWSTATUS,  AGENT.Integer32(SNMP_ROW_NOTINSERVICE), 1),
            (IP_ACCESS_TBL_MEM_DESCRIPTION, AGENT.DisplayString(""), 1),
        ],
        extendable = True,
        nodehandle = ip_access_table_handler.node_handler
    )
    IP_ACCESS_TABLE.set_allow_creation(True)
    IP_ACCESS_TABLE.set_rowstatus_column(IP_ACCESS_TBL_MEM_ROWSTATUS)
    subscriber.subscribeTable(TBL_ACL_CONFIG, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, ip_access_table_handler.subscribe_handler)

    ip_access_rule_table_handler = IpAccessRuleTableHandler(AGENT)
    global IP_ACCESS_RULE_TABLE
    IP_ACCESS_RULE_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.111.6".format(enterpriseID),
        indexes = [
            AGENT.DisplayString(""),
            AGENT.Integer32(0),
        ],
        columns = [
            (IP_ACCESS_RULE_TBL_MEM_ACT,           AGENT.Integer32(GLB_ACL_DENY), 1),
            (IP_ACCESS_RULE_TBL_MEM_IPPROTOCOL,    AGENT.Integer32(IP_ACCESS_RULE_IP_PROTOCOL_ANY), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPSRCOP,      AGENT.Integer32(GLB_ACL_L4_MATCH_TYPE_INVALID), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT1,   AGENT.Integer32(0), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPSRCPORT2,   AGENT.Integer32(0), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPDESTOP,     AGENT.Integer32(GLB_ACL_L4_MATCH_TYPE_INVALID), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT1,  AGENT.Integer32(0), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPDESTPORT2,  AGENT.Integer32(0), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPFLAGOP,     AGENT.Integer32(GLB_ACL_MATCH_TYPE_INVALID), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPFLAGURG,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPFLAGACK,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPFLAGPSH,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPFLAGRST,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPFLAGSYN,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (IP_ACCESS_RULE_TBL_MEM_TCPFLAGFIN,    AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (IP_ACCESS_RULE_TBL_MEM_UDPSRCOP,      AGENT.Integer32(GLB_ACL_L4_MATCH_TYPE_INVALID), 1),
            (IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT1,   AGENT.Integer32(0), 1),
            (IP_ACCESS_RULE_TBL_MEM_UDPSRCPORT2,   AGENT.Integer32(0), 1),
            (IP_ACCESS_RULE_TBL_MEM_UDPDESTOP,     AGENT.Integer32(GLB_ACL_L4_MATCH_TYPE_INVALID), 1),
            (IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT1,  AGENT.Integer32(0), 1),
            (IP_ACCESS_RULE_TBL_MEM_UDPDESTPORT2,  AGENT.Integer32(0), 1),
            (IP_ACCESS_RULE_TBL_MEM_SRCIP,         AGENT.IpAddress("0.0.0.0"), 1),
            (IP_ACCESS_RULE_TBL_MEM_SRCWILD,       AGENT.IpAddress("255.255.255.255"), 1),
            (IP_ACCESS_RULE_TBL_MEM_DESTIP,        AGENT.IpAddress("0.0.0.0"), 1),
            (IP_ACCESS_RULE_TBL_MEM_DESTWILD,      AGENT.IpAddress("255.255.255.255"), 1),
            (IP_ACCESS_RULE_TBL_MEM_IPPRECEDENCE,  AGENT.Integer32(GLB_ACL_INVALID_IP_PRECEDENCE), 1),
            (IP_ACCESS_RULE_TBL_MEM_IPDSCP,        AGENT.Integer32(GLB_ACL_INVALID_IP_DSCP), 1),
            (IP_ACCESS_RULE_TBL_MEM_IPFRAGMENTS,   AGENT.Integer32(FLOW_RULE_IPFRAGMENTS_NOUSE), 1),
            (IP_ACCESS_RULE_TBL_MEM_IPOPTIONS,     AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (IP_ACCESS_RULE_TBL_MEM_VLANID,        AGENT.Integer32(GLB_ACL_VLAN_ID_INVALID), 1),
            (IP_ACCESS_RULE_TBL_MEM_VLANID_WILD,   AGENT.DisplayString('0xfff'), 1),
            (IP_ACCESS_RULE_TBL_MEM_INNERVLANID,   AGENT.Integer32(GLB_ACL_VLAN_ID_INVALID), 1),
            (IP_ACCESS_RULE_TBL_MEM_INNERVLAN_WILD,   AGENT.DisplayString('0xfff'), 1),
            (IP_ACCESS_RULE_TBL_MEM_COS,           AGENT.Integer32(GLB_ACL_INVALID_COS), 1),
            (IP_ACCESS_RULE_TBL_MEM_INNERCOS,      AGENT.Integer32(GLB_ACL_INVALID_COS), 1),
            (IP_ACCESS_RULE_TBL_MEM_ETHERTYPE,     AGENT.DisplayString(''), 1),
            (IP_ACCESS_RULE_TBL_MEM_ETHERTYPEWILD, AGENT.DisplayString(''), 1),
            (IP_ACCESS_RULE_TBL_MEM_SRCMAC,        AGENT.OctetString(''), 1),
            (IP_ACCESS_RULE_TBL_MEM_SRCMACWILD,    AGENT.OctetString(''), 1),
            (IP_ACCESS_RULE_TBL_MEM_DESTMAC,       AGENT.OctetString(''), 1),
            (IP_ACCESS_RULE_TBL_MEM_DESTMACWILD,   AGENT.OctetString(''), 1),
            (IP_ACCESS_RULE_TBL_MEM_ROWSTATUS,     AGENT.Integer32(SNMP_ROW_NOTINSERVICE), 1),
        ],
        extendable = True,
        nodehandle = ip_access_rule_table_handler.node_handler
    )
    IP_ACCESS_RULE_TABLE.set_allow_creation(True)
    IP_ACCESS_RULE_TABLE.set_rowstatus_column(IP_ACCESS_RULE_TBL_MEM_ROWSTATUS)
    subscriber.subscribeTable(TBL_ACE_CONFIG, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, ip_access_rule_table_handler.subscribe_handler)

