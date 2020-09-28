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
from interface import InterfaceTableHandler

AGENT = None
################################################################################
################################################################################
#
# tapgroup
#
################################################################################
################################################################################

################################################################################
#
# timeStampTable enterprise.112.1
#
################################################################################
# Oid indexes
TIMESTAMP_TBL_MEM_INDEX     = 1
TIMESTAMP_TBL_MEM_DESTMAC   = 2
TIMESTAMP_TBL_MEM_SRCMAC    = 3
TIMESTAMP_TBL_MEM_ETHERTYPE = 4
TIMESTAMP_TBL_MEM_ROWSTATUS = 10

# Macro definitions
TIMESTAMP_TBL_DEFAULT_INDEX = 0
TIMESTAMP_ETHERTYPE_LEN = 6

TIMESTAMP_TABLE = None
class TimeStampTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(TimeStampTableHandler, self).__init__(agent)
        self.macda = None
        self.macsa = None

    def check(self, colnum, var_type, data):
        print "check ExTable colnum {0}".format(colnum)
        indexes = self.get_request_indexes(TIMESTAMP_TABLE)
        idx = indexes[0]

        if idx != 0:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        if colnum == TIMESTAMP_TBL_MEM_DESTMAC or colnum == TIMESTAMP_TBL_MEM_SRCMAC:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if len(data) != GLB_SNMP_MAC_ADDR_LEN:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == TIMESTAMP_TBL_MEM_ETHERTYPE:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            ethertype = data
            if len(ethertype) > TIMESTAMP_ETHERTYPE_LEN:
                return self.set_request_error(SNMP_ERR_WRONGLENGTH)

            if ethertype[:2] != "0x" and ethertype[:2] != "0X":
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

            if len(re.search("0[xX][0-9a-fA-F]*", ethertype[0:len(ethertype)]).group(0)) != len(ethertype):
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == TIMESTAMP_TBL_MEM_ROWSTATUS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            cfg_status = data
            if cfg_status != SNMP_ROW_CREATEANDGO and \
                cfg_status != SNMP_ROW_DESTROY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        indexes = self.get_request_indexes(TIMESTAMP_TABLE)
        idx = indexes[0]

        if idx != 0:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        if colnum == TIMESTAMP_TBL_MEM_ROWSTATUS:
            print "colnum TIMESTAMP_TBL_MEM_ROWSTATUS"
            dstmac = self.get_table_row(TIMESTAMP_TBL_MEM_DESTMAC)
            srcmac = self.get_table_row(TIMESTAMP_TBL_MEM_SRCMAC)
            ethtype = self.get_table_row(TIMESTAMP_TBL_MEM_ETHERTYPE)

            src_mac = srcmac.encode("hex")
            str_mac_sa = src_mac[0:4]+"."+src_mac[4:8]+"."+src_mac[8:12]
            dst_mac = dstmac.encode("hex")
            str_dst_mac = dst_mac[0:4]+"."+dst_mac[4:8]+"."+dst_mac[8:12]
            #print str_mac_sa, str_dst_mac, ethtype

            rowstatus = data
            if rowstatus == SNMP_ROW_CREATEANDGO:
                szmsg = execute_cmd('cdbctl update/cdb/sys/sys_global/tap_ts_macda/{0}/{1}/{2}'.format(str_dst_mac, str_mac_sa, ethtype))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    increase_data_sync_add("TIMESTAMP_TABLE", "0")

            if rowstatus == SNMP_ROW_DESTROY:
                szmsg = execute_cmd('cdbctl update/cdb/sys/sys_global/tap_ts_macda/0.0.0/0.0.0/0')
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    increase_data_sync_del("TIMESTAMP_TABLE", "0")

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_sys_global = tbl_pb2.tbl_sys_global()
        try:
            tbl_sys_global.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_sys_global fail'
            return

        tmp = str(0)
        if operation == CDB_OPER_ADD:
            if tbl_sys_global.ether_type != 0 \
                    or tbl_sys_global.tap_ts_macda.mac != '\000\000\000\000\000\000' \
                    or tbl_sys_global.tap_ts_macsa.mac != '\000\000\000\000\000\000':
                if check_data_sync_add("TIMESTAMP_TABLE", "0"):
                    decrease_data_sync_add("TIMESTAMP_TABLE", "0")
                    return
                new_row = TIMESTAMP_TABLE.addRow([AGENT.Integer32(TIMESTAMP_TBL_DEFAULT_INDEX)])
                new_row.setRowCell(TIMESTAMP_TBL_MEM_DESTMAC,   AGENT.OctetString(tbl_sys_global.tap_ts_macda.mac))
                new_row.setRowCell(TIMESTAMP_TBL_MEM_SRCMAC,    AGENT.OctetString(tbl_sys_global.tap_ts_macsa.mac))
                new_row.setRowCell(TIMESTAMP_TBL_MEM_ETHERTYPE, AGENT.DisplayString(hex(tbl_sys_global.ether_type)))
                new_row.setRowCell(TIMESTAMP_TBL_MEM_ROWSTATUS, AGENT.Integer32(SNMP_ROW_ACTIVE))

        if operation == CDB_OPER_SET:
            if TBL_SYS_GLOBAL_FLD_TAP_TS_MACDA == field_id:
                self.macda = tbl_sys_global.tap_ts_macda
            if TBL_SYS_GLOBAL_FLD_TAP_TS_MACSA == field_id:
                self.macsa = tbl_sys_global.tap_ts_macsa
            if TBL_SYS_GLOBAL_FLD_ETHER_TYPE == field_id:
                if tbl_sys_global.ether_type != 0 \
                        or self.macda.mac != '\000\000\000\000\000\000' \
                        or self.macsa.mac != '\000\000\000\000\000\000':
                    if check_data_sync_add("TIMESTAMP_TABLE", "0"):
                        decrease_data_sync_add("TIMESTAMP_TABLE", "0")
                        return
                    new_row = TIMESTAMP_TABLE.addRow([AGENT.Integer32(TIMESTAMP_TBL_DEFAULT_INDEX)])
                    new_row.setRowCell(TIMESTAMP_TBL_MEM_DESTMAC,   AGENT.OctetString(self.macda.mac))
                    new_row.setRowCell(TIMESTAMP_TBL_MEM_SRCMAC,    AGENT.OctetString(self.macsa.mac))
                    new_row.setRowCell(TIMESTAMP_TBL_MEM_ETHERTYPE, AGENT.DisplayString(hex(tbl_sys_global.ether_type)))
                    new_row.setRowCell(TIMESTAMP_TBL_MEM_ROWSTATUS, AGENT.Integer32(SNMP_ROW_ACTIVE))
                else:
                    if check_data_sync_del("TIMESTAMP_TABLE", "0"):
                        decrease_data_sync_del("TIMESTAMP_TABLE", "0")
                        return
                    TIMESTAMP_TABLE.delRow([AGENT.Integer32(TIMESTAMP_TBL_DEFAULT_INDEX)])

################################################################################
#
# truncation enterprise.112.1
#
################################################################################
# Macro definitions
TRUNCATION_LEN_MIN = 64
TRUNCATION_LEN_MAX = 144

TRUNCATION = None
class TruncationHandler(SnmpTableBase):
    def __init__(self, agent):
        super(TruncationHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        trun_len = data
        if (trun_len < TRUNCATION_LEN_MIN or trun_len > TRUNCATION_LEN_MAX) and trun_len != 0:
            return self.set_request_error(SNMP_ERR_BADVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        trun_len = data

        szmsg = execute_cmd('cdbctl update/cdb/sys/sys_global/trunction_length/{0}'.format(trun_len))
        if szmsg.find('%') != -1:
            self.set_request_error(SNMP_ERR_GENERR)
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_sys_global = tbl_pb2.tbl_sys_global()
        try:
            tbl_sys_global.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_sys_global fail'
            return

        TRUNCATION.update(tbl_sys_global.trunction_length)

################################################################################
#
# tapGroupTable enterprise.112.3
#
################################################################################
# Oid indexes
TAP_GROUP_TBL_NAME        = 1
TAP_GROUP_TBL_GRP_ID      = 2
TAP_GROUP_TBL_ROWSTATUS   = 10
TAP_GROUP_TBL_DESCRIPTION = 11

# Macro definitions
TAP_GROUP_DESCRIPTION_LEN = 80
TAP_GROUP_NAME_LEN = 20
TAP_GROUP_ID_MAX = 512
TAP_GROUP_ID_MIN = 1

TAP_GROUP_TABLE = None
class TapGroupTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(TapGroupTableHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        print "check ExTable colnum {0}".format(colnum)

        # Index check
        indexes = self.get_request_indexes(TAP_GROUP_TABLE)
        grp_name = indexes[0]
        grp_id = indexes[1]
        if len(grp_name) > TAP_GROUP_NAME_LEN:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        match = re.search("^[a-zA-Z][0-9a-zA-Z._-]*$", grp_name[0:len(grp_name)])
        if not match:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        if len(match.group(0)) != len(grp_name):
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        if grp_id < TAP_GROUP_ID_MIN or grp_id > TAP_GROUP_ID_MAX:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)


        if colnum == TAP_GROUP_TBL_ROWSTATUS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            rowstatus = data
            if rowstatus != SNMP_ROW_CREATEANDGO and \
                rowstatus != SNMP_ROW_DESTROY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == TAP_GROUP_TBL_DESCRIPTION:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            description = data
            if len(description) > TAP_GROUP_DESCRIPTION_LEN:
                return self.set_request_error(SNMP_ERR_WRONGLENGTH)
            if len(re.search("^[0-9a-zA-Z._-]*", description[0:len(description)]).group(0)) != len(description):
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        indexes = self.get_request_indexes(TAP_GROUP_TABLE)
        grp_name = indexes[0]
        grp_id = indexes[1]

        exist = None
        print "action ExTable colnum {0}".format(colnum)

        szmsg = execute_cmd('cdbctl read/cdb/tap/tap_group/{0}'.format(grp_name))
        if szmsg.find('key') == -1:
            exist = False
        else:
            exist = True
        print "exist : {0}".format(exist)


        if colnum == TAP_GROUP_TBL_ROWSTATUS:
            print "colnum TAP_GROUP_TBL_ROWSTATUS"
            rowstatus = data
            if rowstatus == SNMP_ROW_CREATEANDGO:
                if exist:
                    return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

                szmsg = execute_cmd('cdbctl create/cdb/tap/tap_group/{0}/{1}'.format(grp_name, grp_id))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    increase_data_sync_add("TAP_GROUP_TABLE", grp_name)
                    GLB_TAP_GROUP_DICT[grp_name] = grp_id

                self.set_table_row(TAP_GROUP_TBL_DESCRIPTION, AGENT.DisplayString(""))

            if rowstatus == SNMP_ROW_DESTROY:
                if not exist:
                    return self.set_request_error(SNMP_ERR_NOSUCHNAME)

                szmsg = execute_cmd('cdbctl delete/cdb/tap/tap_group/{0}'.format(grp_name))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    increase_data_sync_del("TAP_GROUP_TABLE", grp_name)
                    del(GLB_TAP_GROUP_DICT[grp_name])

        if colnum == TAP_GROUP_TBL_DESCRIPTION:
            print "colnum TAP_GROUP_TBL_DESCRIPTION"
            description = data
            if not exist:
                return self.set_request_error(SNMP_ERR_NOSUCHNAME)

            if description != "":
                szmsg = execute_cmd('cdbctl update/cdb/tap/tap_group/{0}/desc/1/{1}'.format(grp_name, description))
            else:
                szmsg = execute_cmd('cdbctl update/cdb/tap/tap_group/{0}/desc/0'.format(grp_name))
            if szmsg.find('%') != -1:
                   return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_tap_group = tbl_pb2.tbl_tap_group()
        try:
            tbl_tap_group.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_tap_group fail'
            return

        if operation == CDB_OPER_ADD:
            if check_data_sync_add("TAP_GROUP_TABLE", tbl_tap_group.key.name):
                decrease_data_sync_add("TAP_GROUP_TABLE", tbl_tap_group.key.name)
                return
            GLB_TAP_GROUP_DICT[tbl_tap_group.key.name] = tbl_tap_group.id
            new_row = TAP_GROUP_TABLE.addRow([AGENT.DisplayString(tbl_tap_group.key.name), AGENT.Integer32(tbl_tap_group.id)])
            new_row.setRowCell(TAP_GROUP_TBL_ROWSTATUS, AGENT.Integer32(SNMP_ROW_ACTIVE))
            new_row.setRowCell(TAP_GROUP_TBL_DESCRIPTION, AGENT.DisplayString(tbl_tap_group.desc))

        if operation == CDB_OPER_DEL:
            print tbl_tap_group
            if check_data_sync_del("TAP_GROUP_TABLE", tbl_tap_group.key.name):
                decrease_data_sync_del("TAP_GROUP_TABLE", tbl_tap_group.key.name)
                return
            print "del tbl_tap_group" , tbl_tap_group
            group_id = GLB_TAP_GROUP_DICT[tbl_tap_group.key.name]
            TAP_GROUP_TABLE.delRow([AGENT.DisplayString(tbl_tap_group.key.name), AGENT.Integer32(group_id)])
            del (GLB_TAP_GROUP_DICT[tbl_tap_group.key.name])

        if operation == CDB_OPER_SET:
            if field_id == TBL_TAP_GROUP_FLD_DESC:
                old_row = TAP_GROUP_TABLE.addRow([AGENT.DisplayString(tbl_tap_group.key.name), AGENT.Integer32(tbl_tap_group.id)])
                old_row.setRowCell(TAP_GROUP_TBL_DESCRIPTION, AGENT.DisplayString(tbl_tap_group.desc))


################################################################################
#
# tapGroupIngressTable enterprise.112.4
#
################################################################################
# Oid indexes
TAP_GROUP_INGRESS_TBL_NAME        = 1
TAP_GROUP_INGRESS_TBL_IFIDX       = 2
TAP_GROUP_INGRESS_TBL_FLOWNAME    = 3
TAP_GROUP_INGRESS_TBL_MARKSRC     = 4
TAP_GROUP_INGRESS_TBL_UNTAG       = 5
TAP_GROUP_INGRESS_TBL_TRUNCATION  = 6
TAP_GROUP_INGRESS_TBL_EDIT_MAC_SA = 7
TAP_GROUP_INGRESS_TBL_EDIT_MAC_DA = 8
TAP_GROUP_INGRESS_TBL_EDIT_IP_SA_EN = 9
TAP_GROUP_INGRESS_TBL_EDIT_IP_SA  = 10
TAP_GROUP_INGRESS_TBL_EDIT_IP_DA_EN = 11
TAP_GROUP_INGRESS_TBL_EDIT_IP_DA  = 12
TAP_GROUP_INGRESS_TBL_ROWSTATUS   = 15

# Macro definitions
TAP_GROUP_INGRESS_UN_TAG_VLAN_ID = 0xffff
TAP_GROUP_FLOWNAME_LEN           = 20

TAP_GROUP_INGRESS_TABLE = None
class TapGroupIngressTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(TapGroupIngressTableHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        print "check ExTable colnum {0}".format(colnum)
        indexes = self.get_request_indexes(TAP_GROUP_INGRESS_TABLE)
        grp_name = indexes[0]
        ifidx = indexes[1]

        #szmsg = execute_cmd('cdbctl read/cdb/tap/tap_group/{0}'.format(grp_name))
        #if szmsg.find('key') == -1:
        #    return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)

        if_name = InterfaceTableHandler.if_index_table[ifidx]["if_key_name"].replace("_", "/")
        if len(if_name) == 0:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        #szmsg = execute_cmd('cdbctl read/cdb/interface/{0}'.format(if_name))
        #if szmsg.find('%') != -1:
        #    return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
            
        old_row_status = None
        old_row = TAP_GROUP_INGRESS_TABLE.getRow([AGENT.DisplayString(grp_name), AGENT.Integer32(ifidx)])
        if old_row:
            old_row_status = self.get_table_row(TAP_GROUP_INGRESS_TBL_ROWSTATUS)

        if colnum < TAP_GROUP_INGRESS_TBL_ROWSTATUS and old_row_status == SNMP_ROW_ACTIVE:
            return self.set_request_error(SNMP_ERR_NOTWRITABLE)

        if colnum == TAP_GROUP_INGRESS_TBL_ROWSTATUS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != SNMP_ROW_CREATEANDGO and data != SNMP_ROW_DESTROY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            if data == SNMP_ROW_CREATEANDGO and old_row_status == SNMP_ROW_ACTIVE:
                return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
            if data == SNMP_ROW_DESTROY and not old_row_status:
                return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)

        if colnum == TAP_GROUP_INGRESS_TBL_EDIT_IP_SA or colnum == TAP_GROUP_INGRESS_TBL_EDIT_IP_DA:
            if var_type != ASN_IPADDRESS:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            print data

        if colnum == TAP_GROUP_INGRESS_TBL_FLOWNAME:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            flow_name = data
            if len(flow_name) > TAP_GROUP_FLOWNAME_LEN:
                return self.set_request_error(SNMP_ERR_WRONGLENGTH)
            if len(re.search("^[0-9a-zA-Z._-]*", flow_name[0:len(flow_name)]).group(0)) != len(flow_name):
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == TAP_GROUP_INGRESS_TBL_MARKSRC:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            set_val = data
            if set_val < 1 or set_val > 4094:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == TAP_GROUP_INGRESS_TBL_UNTAG or colnum == TAP_GROUP_INGRESS_TBL_TRUNCATION or \
            colnum == TAP_GROUP_INGRESS_TBL_EDIT_IP_SA_EN or colnum == TAP_GROUP_INGRESS_TBL_EDIT_IP_DA_EN:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            set_val = data
            if set_val != GLB_SNMP_TRUE and set_val != GLB_SNMP_FALSE:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == TAP_GROUP_INGRESS_TBL_EDIT_MAC_SA or colnum == TAP_GROUP_INGRESS_TBL_EDIT_MAC_DA:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if len(data) != GLB_SNMP_MAC_ADDR_LEN:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        indexes = self.get_request_indexes(TAP_GROUP_INGRESS_TABLE)
        grp_name = indexes[0]
        ifidx = indexes[1]
        old_row_status = None
        old_row = TAP_GROUP_INGRESS_TABLE.getRow([AGENT.DisplayString(grp_name), AGENT.Integer32(ifidx)])
        if old_row:
            old_row_status = self.get_table_row(TAP_GROUP_INGRESS_TBL_ROWSTATUS)

        if_name = InterfaceTableHandler.if_index_table[ifidx]["if_key_name"].replace("_", "/")

        if colnum == TAP_GROUP_INGRESS_TBL_ROWSTATUS and data == SNMP_ROW_DESTROY:
            if old_row_status == SNMP_ROW_ACTIVE:
                tmp = grp_name + str(ifidx)
                if tmp in GLB_TAP_GRP_IGS_FLOW_DICT:
                    flowname = GLB_TAP_GRP_IGS_FLOW_DICT[tmp]
                    szmsg = execute_cmd('cdbctl delete/cdb/tap/tap_group_ingress_flow/{0}/{1}/{2}'.format(grp_name, if_name, flowname))
                else:
                    szmsg = execute_cmd('cdbctl delete/cdb/tap/tap_group_ingress/{0}#{1}'.format(grp_name, if_name))

                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    tmp = grp_name + str(ifidx)
                    if tmp in GLB_TAP_GRP_IGS_FLOW_DICT:
                        del(GLB_TAP_GRP_IGS_FLOW_DICT[tmp])
                    increase_data_sync_del("TAP_GROUP_INGRESS_TABLE", tmp)


        if colnum == TAP_GROUP_INGRESS_TBL_ROWSTATUS and  data == SNMP_ROW_CREATEANDGO:
            flowname = self.get_table_row(TAP_GROUP_INGRESS_TBL_FLOWNAME)
            marksrc = self.get_table_row(TAP_GROUP_INGRESS_TBL_MARKSRC)
            untag = self.get_table_row(TAP_GROUP_INGRESS_TBL_UNTAG)
            trun = self.get_table_row(TAP_GROUP_INGRESS_TBL_TRUNCATION)
            edit_mac_sa = self.get_table_row(TAP_GROUP_INGRESS_TBL_EDIT_MAC_SA)
            edit_mac_da = self.get_table_row(TAP_GROUP_INGRESS_TBL_EDIT_MAC_DA)
            edit_ip_sa_en = self.get_table_row(TAP_GROUP_INGRESS_TBL_EDIT_IP_SA_EN)
            edit_ip_da_en = self.get_table_row(TAP_GROUP_INGRESS_TBL_EDIT_IP_DA_EN)
            edit_ip_sa = self.get_table_row(TAP_GROUP_INGRESS_TBL_EDIT_IP_SA)
            edit_ip_da = self.get_table_row(TAP_GROUP_INGRESS_TBL_EDIT_IP_DA)


            if trun == GLB_SNMP_TRUE:
                truncation = "truncation"
                truncation1 = "true"
            else:
                truncation = ""
                truncation1 = "false"
            if untag == GLB_SNMP_TRUE:
                unta = "un-tag"
                unta1 = "true"
            else:
                unta = ""
                unta1 = "false"

            print "colnum TAP_GROUP_INGRESS_TBL_ROWSTATUS"

            if len(flowname) == 0:
                if unta1 == "true" and marksrc != 0:
                    return self.set_request_error(SNMP_ERR_WRONGVALUE)
                cmd = 'cdbctl create/cdb/tap/tap_group_ingress/{0}#{1}/mark-source/{2}/untag/{3}/truncation/{4}'.format\
                                            (grp_name, if_name, marksrc, unta1, truncation1)
                str_edit_mac_sa = ""
                str_edit_mac_da = ""
                if len(edit_mac_sa) == GLB_SNMP_MAC_ADDR_LEN:
                    mac = edit_mac_sa.encode("hex")
                    str_edit_mac_sa = mac[0:4] + "." + mac[4:8] + "." + mac[8:12]
                    cmd = cmd + "/edit-macsa/" + str_edit_mac_sa

                if len(edit_mac_da) == GLB_SNMP_MAC_ADDR_LEN:
                    mac = edit_mac_da.encode("hex")
                    str_edit_mac_da = mac[0:4] + "." + mac[4:8] + "." + mac[8:12]
                    cmd = cmd + "/edit-macda/" + str_edit_mac_da

                if edit_ip_sa_en == GLB_SNMP_TRUE:
                    cmd = cmd + "/edit-ipsa/" + edit_ip_sa

                if edit_ip_da_en == GLB_SNMP_TRUE:
                    cmd = cmd + "/edit-ipda/" + edit_ip_da

                szmsg = execute_cmd(cmd)
            else:
                if len(truncation) != 0:
                    return self.set_request_error(SNMP_ERR_WRONGVALUE)
                if marksrc != 0 and len(unta) != 0:
                    return self.set_request_error(SNMP_ERR_WRONGVALUE)
                if len(edit_mac_sa) == GLB_SNMP_MAC_ADDR_LEN or len(edit_mac_da) == GLB_SNMP_MAC_ADDR_LEN or \
                    edit_ip_sa_en == GLB_SNMP_TRUE or  edit_ip_da_en == GLB_SNMP_TRUE:
                    return self.set_request_error(SNMP_ERR_WRONGVALUE)
                if marksrc != 0 :
                    szmsg = execute_cmd('cdbctl create/cdb/tap/tap_group_ingress_flow/{0}/{1}/{2}/mark-source/{3}/{4}/'.format \
                            (grp_name, if_name, flowname, marksrc, unta))
                else:
                    szmsg = execute_cmd('cdbctl create/cdb/tap/tap_group_ingress_flow/{0}/{1}/{2}/mark-source//{3}/'.format \
                                (grp_name, if_name, flowname, unta))

            if szmsg.find('%') != -1:
                print szmsg
                return self.set_request_error(SNMP_ERR_GENERR)
            else:
                tmp = grp_name + str(ifidx)
                increase_data_sync_add("TAP_GROUP_INGRESS_TABLE", tmp)
                if len(flowname) != 0:
                    GLB_TAP_GRP_IGS_FLOW_DICT[tmp] = flowname

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):

        if tbl_id == TBL_TAP_GROUP_INGRESS_FLOW and ds_id == CDB_INVALID_DS_ID:
            tbl_tap_igs_flow = tbl_pb2.tbl_tap_group_ingress_flow()
            try:
                tbl_tap_igs_flow.ParseFromString(data)
            except:
                print 'ERROR!! Parse tbl_tap_igs_flow fail'
                return

        if tbl_id == TBL_TAP_GROUP_INGRESS and ds_id == CDB_INVALID_DS_ID:
            tbl_tap_igs = tbl_pb2.tbl_tap_group_ingress()
            try:
                tbl_tap_igs.ParseFromString(data)
            except:
                print 'ERROR!! Parse tbl_tap_igs_flow fail'
                return

        if operation == CDB_OPER_ADD:

            if tbl_id == TBL_TAP_GROUP_INGRESS_FLOW:
                ifindex = GLB_IF_NAME_DICT[tbl_tap_igs_flow.key.if_name]
                tmp = tbl_tap_igs_flow.key.tap_group + str(ifindex)
                if check_data_sync_add("TAP_GROUP_INGRESS_TABLE", tmp):
                    decrease_data_sync_add("TAP_GROUP_INGRESS_TABLE", tmp)
                    return
                new_row = TAP_GROUP_INGRESS_TABLE.addRow([AGENT.DisplayString(tbl_tap_igs_flow.key.tap_group), AGENT.Integer32(ifindex)])
                if tbl_tap_igs_flow.key.flow_name[:6] == "#FLOW#":
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_FLOWNAME, AGENT.DisplayString(tbl_tap_igs_flow.key.flow_name[6:]))
                    GLB_TAP_GRP_IGS_FLOW_DICT[tmp] = tbl_tap_igs_flow.key.flow_name[6:]
                else:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_FLOWNAME, AGENT.DisplayString(tbl_tap_igs_flow.key.flow_name))
                    GLB_TAP_GRP_IGS_FLOW_DICT[tmp] = tbl_tap_igs_flow.key.flow_name
                if tbl_tap_igs_flow.ing_untag_en != 0:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_UNTAG, AGENT.Integer32(GLB_SNMP_TRUE))
                else:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_UNTAG, AGENT.Integer32(GLB_SNMP_FALSE))
                if tbl_tap_igs_flow.ing_mark_vlan_en != 0:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_MARKSRC, AGENT.Integer32(tbl_tap_igs_flow.ing_mark_vlan_vid))
                new_row.setRowCell(TAP_GROUP_INGRESS_TBL_ROWSTATUS, AGENT.Integer32(SNMP_ROW_ACTIVE))

            if tbl_id == TBL_TAP_GROUP_INGRESS:
                ifindex = GLB_IF_NAME_DICT[tbl_tap_igs.key.if_name]
                tmp = tbl_tap_igs.key.tap_group + str(ifindex)
                if check_data_sync_add("TAP_GROUP_INGRESS_TABLE", tmp):
                    decrease_data_sync_add("TAP_GROUP_INGRESS_TABLE", tmp)
                    return
                new_row = TAP_GROUP_INGRESS_TABLE.addRow([AGENT.DisplayString(tbl_tap_igs.key.tap_group), AGENT.Integer32(ifindex)])

                if tbl_tap_igs.ing_untag_en != 0:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_UNTAG, AGENT.Integer32(GLB_SNMP_TRUE))
                else:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_UNTAG, AGENT.Integer32(GLB_SNMP_FALSE))
                if tbl_tap_igs.ing_mark_vlan_en != 0:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_MARKSRC, AGENT.Integer32(tbl_tap_igs.ing_mark_vlan_vid))
                if tbl_tap_igs.ing_trunction_en != 0:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_TRUNCATION, AGENT.Integer32(GLB_SNMP_TRUE))

                if tbl_tap_igs.ing_edit_dest_mac_en != 0:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_EDIT_MAC_DA, AGENT.OctetString(tbl_tap_igs.ing_edit_dest_mac.mac))

                if tbl_tap_igs.ing_edit_src_mac_en != 0:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_EDIT_MAC_SA, AGENT.OctetString(tbl_tap_igs.ing_edit_src_mac.mac))

                if tbl_tap_igs.ing_edit_ipda_en != 0:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_EDIT_IP_DA_EN, AGENT.Integer32(GLB_SNMP_TRUE))
                    ipda = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_tap_igs.ing_edit_ipda.ipv4_addr)))
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_EDIT_IP_DA, AGENT.IpAddress(ipda))
                else:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_EDIT_IP_DA_EN, AGENT.Integer32(GLB_SNMP_FALSE))

                if tbl_tap_igs.ing_edit_ipsa_en != 0:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_EDIT_IP_SA_EN, AGENT.Integer32(GLB_SNMP_TRUE))
                    ipsa = socket.inet_ntoa(struct.pack('I', socket.htonl(tbl_tap_igs.ing_edit_ipsa.ipv4_addr)))
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_EDIT_IP_SA, AGENT.IpAddress(ipsa))
                else:
                    new_row.setRowCell(TAP_GROUP_INGRESS_TBL_EDIT_IP_SA_EN, AGENT.Integer32(GLB_SNMP_FALSE))

                new_row.setRowCell(TAP_GROUP_INGRESS_TBL_ROWSTATUS, AGENT.Integer32(SNMP_ROW_ACTIVE))

        if operation == CDB_OPER_DEL:
            if tbl_id == TBL_TAP_GROUP_INGRESS_FLOW:
                ifindex = GLB_IF_NAME_DICT[tbl_tap_igs_flow.key.if_name]
                tmp = tbl_tap_igs_flow.key.tap_group + str(ifindex)
                if check_data_sync_del("TAP_GROUP_INGRESS_TABLE", tmp):
                    decrease_data_sync_del("TAP_GROUP_INGRESS_TABLE", tmp)
                    return
                del(GLB_TAP_GRP_IGS_FLOW_DICT[tmp])
                TAP_GROUP_INGRESS_TABLE.delRow([AGENT.DisplayString(tbl_tap_igs_flow.key.tap_group), AGENT.Integer32(ifindex)])
            if tbl_id == TBL_TAP_GROUP_INGRESS:
                ifindex = GLB_IF_NAME_DICT[tbl_tap_igs.key.if_name]
                tmp = tbl_tap_igs.key.tap_group + str(ifindex)
                if check_data_sync_del("TAP_GROUP_INGRESS_TABLE", tmp):
                    decrease_data_sync_del("TAP_GROUP_INGRESS_TABLE", tmp)
                    return
                TAP_GROUP_INGRESS_TABLE.delRow([AGENT.DisplayString(tbl_tap_igs.key.tap_group), AGENT.Integer32(ifindex)])

################################################################################
#
# tapGroupIngressTable enterprise.112.4
#
################################################################################
# Oid indexes
TAP_GROUP_EGRESS_TBL_NAME        = 1
TAP_GROUP_EGRESS_TBL_IFIDX       = 2
TAP_GROUP_EGRESS_TBL_TIMESTAMP   = 3
TAP_GROUP_EGRESS_TBL_ROWSTATUS   = 10

TAP_GROUP_EGRESS_TABLE = None
class TapGroupEgressTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(TapGroupEgressTableHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        print "check ExTable colnum {0}".format(colnum)
        indexes = self.get_request_indexes(TAP_GROUP_EGRESS_TABLE)
        grp_name = indexes[0]
        ifidx = indexes[1]

        szmsg = execute_cmd('cdbctl read/cdb/tap/tap_group/{0}'.format(grp_name))
        if szmsg.find('key') == -1:
            return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)

        if_name = InterfaceTableHandler.if_index_table[ifidx]["if_key_name"].replace("_", "/")
        if len(if_name) == 0:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
        szmsg = execute_cmd('cdbctl read/cdb/interface/{0}'.format(if_name))
        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
            
        old_row_status = None
        old_row = TAP_GROUP_EGRESS_TABLE.getRow([AGENT.DisplayString(grp_name), AGENT.Integer32(ifidx)])
        if old_row:
            old_row_status = self.get_table_row(TAP_GROUP_EGRESS_TBL_ROWSTATUS)

        if colnum < TAP_GROUP_EGRESS_TBL_ROWSTATUS and old_row_status == SNMP_ROW_ACTIVE:
            return self.set_request_error(SNMP_ERR_NOTWRITABLE)

        if colnum == TAP_GROUP_EGRESS_TBL_ROWSTATUS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            if data != SNMP_ROW_CREATEANDGO and data != SNMP_ROW_DESTROY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            if data == SNMP_ROW_CREATEANDGO and old_row_status == SNMP_ROW_ACTIVE:
                return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)
            if data == SNMP_ROW_DESTROY and not old_row_status:
                return self.set_request_error(SNMP_ERR_INCONSISTENTVALUE)

        if colnum == TAP_GROUP_EGRESS_TBL_TIMESTAMP:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            set_val = data
            if set_val != GLB_SNMP_TRUE and set_val != GLB_SNMP_FALSE:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        indexes = self.get_request_indexes(TAP_GROUP_EGRESS_TABLE)
        grp_name = indexes[0]
        ifidx = indexes[1]
        old_row_status = None
        old_row = TAP_GROUP_EGRESS_TABLE.getRow([AGENT.DisplayString(grp_name), AGENT.Integer32(ifidx)])
        if old_row:
            old_row_status = self.get_table_row(TAP_GROUP_EGRESS_TBL_ROWSTATUS)


        timestamp = int(self.get_table_row(TAP_GROUP_EGRESS_TBL_TIMESTAMP))
        #print "3. timestamp = ", timestamp
        if_name = InterfaceTableHandler.if_index_table[ifidx]["if_key_name"].replace("_", "/")


        if colnum == TAP_GROUP_EGRESS_TBL_ROWSTATUS:
            print "colnum TAP_GROUP_EGRESS_TBL_ROWSTATUS"
            rowstatus = data
            if rowstatus == SNMP_ROW_CREATEANDGO:
                if timestamp == GLB_SNMP_TRUE:
                    ts = "true"
                else :
                    ts = "false"

                szmsg = execute_cmd('cdbctl create/cdb/tap/tap_group_egress/{0}#{1}/timestamp/{2}'.format(grp_name, if_name, ts))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    tmp = grp_name + str(ifidx)
                    increase_data_sync_add("TAP_GROUP_EGRESS_TABLE", tmp)

            if rowstatus == SNMP_ROW_DESTROY and old_row_status == SNMP_ROW_ACTIVE:
                szmsg = execute_cmd('cdbctl delete/cdb/tap/tap_group_egress/{0}#{1}'.format(grp_name, if_name))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    tmp = grp_name + str(ifidx)
                    increase_data_sync_del("TAP_GROUP_EGRESS_TABLE", tmp)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_tap_egs = tbl_pb2.tbl_tap_group_egress()
        try:
            tbl_tap_egs.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_tap_egs fail'
            return

        ifindex = GLB_IF_NAME_DICT[tbl_tap_egs.key.if_name]
        if operation == CDB_OPER_ADD:
            tmp = tbl_tap_egs.key.tap_group + str(ifindex)
            if check_data_sync_add("TAP_GROUP_EGRESS_TABLE", tmp):
                decrease_data_sync_add("TAP_GROUP_EGRESS_TABLE", tmp)
                return
            new_row = TAP_GROUP_EGRESS_TABLE.addRow([AGENT.DisplayString(tbl_tap_egs.key.tap_group), AGENT.Integer32(ifindex)])
            if tbl_tap_egs.timestamp_en != 0:
                new_row.setRowCell(TAP_GROUP_EGRESS_TBL_TIMESTAMP, AGENT.Integer32(GLB_SNMP_TRUE))
            new_row.setRowCell(TAP_GROUP_EGRESS_TBL_ROWSTATUS, AGENT.Integer32(SNMP_ROW_ACTIVE))

        if operation == CDB_OPER_DEL:
            tmp = tbl_tap_egs.key.tap_group  + str(ifindex)
            if check_data_sync_del("TAP_GROUP_EGRESS_TABLE", tmp):
                check_data_sync_del("TAP_GROUP_EGRESS_TABLE", tmp)
                return
            TAP_GROUP_EGRESS_TABLE.delRow([AGENT.DisplayString(tbl_tap_egs.key.tap_group), AGENT.Integer32(ifindex)])




################################################################################
#
# TimestampSycn enterprise.112.5
#
################################################################################
TIMESTAMP_SYNC = None
class TimestampSyncHandler(SnmpTableBase):
    def __init__(self, agent):
        super(TimestampSyncHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        sync_type = data
        if sync_type != GLB_STATE_ENABLE and sync_type != GLB_STATE_DISABLE:
            return self.set_request_error(SNMP_ERR_BADVALUE)
        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        sync_type = data
        if sync_type == GLB_STATE_ENABLE:
            szmsg = execute_cmd('cdbctl update/cdb/ptp/fea_time/sync_type/systime')
        if sync_type == GLB_STATE_DISABLE:
            szmsg = execute_cmd('cdbctl update/cdb/ptp/fea_time/sync_type/none')

        if szmsg.find('%') != -1:
            self.set_request_error(SNMP_ERR_GENERR)
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        # print "TimestampSyncHandler subscriber called"
        tbl_fea_time = tbl_pb2.tbl_fea_time()
        try:
            tbl_fea_time.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_lag_global fail'
            return
        # print tbl_fea_time

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            if (tbl_fea_time.sync_type) == 0:
                TIMESTAMP_SYNC.update(GLB_STATE_DISABLE)
            else:
                TIMESTAMP_SYNC.update(GLB_STATE_ENABLE)


################################################################################
#
# Module Init
#
################################################################################
def init(agent_obj, subscriber):
    global AGENT
    AGENT = agent_obj
    enterpriseID = AGENT.get_enterprise_id()

    timestamp_table_handler = TimeStampTableHandler(AGENT)
    global TIMESTAMP_TABLE
    TIMESTAMP_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.112.1".format(enterpriseID),
        indexes = [
            AGENT.Integer32(0),
        ],
        columns = [
            (TIMESTAMP_TBL_MEM_DESTMAC,   AGENT.OctetString(''), 1),
            (TIMESTAMP_TBL_MEM_SRCMAC,    AGENT.OctetString(''), 1),
            (TIMESTAMP_TBL_MEM_ETHERTYPE, AGENT.DisplayString(""), 1),
            (TIMESTAMP_TBL_MEM_ROWSTATUS, AGENT.Integer32(SNMP_ROW_NOTREADY), 1),
        ],
        extendable = True,
        nodehandle = timestamp_table_handler.node_handler
    )
    TIMESTAMP_TABLE.set_allow_creation(True)
    TIMESTAMP_TABLE.set_rowstatus_column(TIMESTAMP_TBL_MEM_ROWSTATUS)
    subscriber.subscribeTable(TBL_SYS_GLOBAL, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, timestamp_table_handler.subscribe_handler)

    truncation_handler = TruncationHandler(AGENT)
    global TRUNCATION
    TRUNCATION = AGENT.Integer32(
        oidstr  = "1.3.6.1.4.1.{0}.112.2".format(enterpriseID),
        initval = 0,
        nodehandle = truncation_handler.node_handler
    )
    subscriber.subscribeTable(TBL_SYS_GLOBAL, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_SYS_GLOBAL_FLD_TRUNCTION_LENGTH, truncation_handler.subscribe_handler)

    tap_group_table_handler = TapGroupTableHandler(AGENT)
    global TAP_GROUP_TABLE
    TAP_GROUP_TABLE = AGENT.Table(
        oidstr  = "1.3.6.1.4.1.{0}.112.3".format(enterpriseID),
        indexes = [
            AGENT.DisplayString(""),
            AGENT.Integer32(0),
        ],
        columns = [
            (TAP_GROUP_TBL_ROWSTATUS,  AGENT.Integer32(SNMP_ROW_ACTIVE), 1),
            (TAP_GROUP_TBL_DESCRIPTION, AGENT.DisplayString(""), 1),
        ],
        extendable = True,
        nodehandle = tap_group_table_handler.node_handler
    )
    TAP_GROUP_TABLE.set_allow_creation(True)
    TAP_GROUP_TABLE.set_rowstatus_column(TAP_GROUP_TBL_ROWSTATUS)
    subscriber.subscribeTable(TBL_TAP_GROUP, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, tap_group_table_handler.subscribe_handler)

    tap_group_ingress_table_handler = TapGroupIngressTableHandler(AGENT)
    global TAP_GROUP_INGRESS_TABLE
    TAP_GROUP_INGRESS_TABLE = AGENT.Table(
        oidstr  = "1.3.6.1.4.1.{0}.112.4".format(enterpriseID),
        indexes = [
            AGENT.DisplayString(""),
            AGENT.Integer32(0),
        ],
        columns = [
            (TAP_GROUP_INGRESS_TBL_FLOWNAME,   AGENT.DisplayString(""), 1),
            (TAP_GROUP_INGRESS_TBL_MARKSRC,    AGENT.Integer32(0), 1),
            (TAP_GROUP_INGRESS_TBL_UNTAG,      AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (TAP_GROUP_INGRESS_TBL_TRUNCATION, AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (TAP_GROUP_INGRESS_TBL_EDIT_MAC_SA, AGENT.OctetString(''), 1),
            (TAP_GROUP_INGRESS_TBL_EDIT_MAC_DA, AGENT.OctetString(''), 1),
            (TAP_GROUP_INGRESS_TBL_EDIT_IP_SA_EN, AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (TAP_GROUP_INGRESS_TBL_EDIT_IP_SA, AGENT.IpAddress("0.0.0.0"), 1),
            (TAP_GROUP_INGRESS_TBL_EDIT_IP_DA_EN, AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (TAP_GROUP_INGRESS_TBL_EDIT_IP_DA, AGENT.IpAddress("0.0.0.0"), 1),
            (TAP_GROUP_INGRESS_TBL_ROWSTATUS,  AGENT.Integer32(SNMP_ROW_NOTREADY), 1),
        ],
        extendable = True,
        nodehandle = tap_group_ingress_table_handler.node_handler
    )
    TAP_GROUP_INGRESS_TABLE.set_allow_creation(True)
    TAP_GROUP_INGRESS_TABLE.set_rowstatus_column(TAP_GROUP_INGRESS_TBL_ROWSTATUS)
    subscriber.subscribeTable(TBL_TAP_GROUP_INGRESS, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, tap_group_ingress_table_handler.subscribe_handler)
    subscriber.subscribeTable(TBL_TAP_GROUP_INGRESS_FLOW, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, tap_group_ingress_table_handler.subscribe_handler)

    tap_group_egress_table_handler = TapGroupEgressTableHandler(AGENT)
    global TAP_GROUP_EGRESS_TABLE
    TAP_GROUP_EGRESS_TABLE = AGENT.Table(
        oidstr  = "1.3.6.1.4.1.{0}.112.5".format(enterpriseID),
        indexes = [
            AGENT.DisplayString(""),
            AGENT.Integer32(0),
        ],
        columns = [
            (TAP_GROUP_EGRESS_TBL_TIMESTAMP, AGENT.Integer32(GLB_SNMP_FALSE), 1),
            (TAP_GROUP_EGRESS_TBL_ROWSTATUS,  AGENT.Integer32(SNMP_ROW_NOTREADY), 1),
        ],
        extendable = True,
        nodehandle = tap_group_egress_table_handler.node_handler
    )
    TAP_GROUP_EGRESS_TABLE.set_allow_creation(True)
    TAP_GROUP_EGRESS_TABLE.set_rowstatus_column(TAP_GROUP_EGRESS_TBL_ROWSTATUS)
    subscriber.subscribeTable(TBL_TAP_GROUP_EGRESS, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, tap_group_egress_table_handler.subscribe_handler)

    timestamp_sync_handler = TimestampSyncHandler(AGENT)
    global TIMESTAMP_SYNC
    TIMESTAMP_SYNC = AGENT.Integer32(
        oidstr = "1.3.6.1.4.1.{0}.112.6".format(enterpriseID),
        initval=GLB_STATE_DISABLE,
        nodehandle = timestamp_sync_handler.node_handler
    )
    subscriber.subscribeTable(TBL_FEA_TIME, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, timestamp_sync_handler.subscribe_handler)