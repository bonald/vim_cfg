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
# interface
#
################################################################################
################################################################################

################################################################################
#
#interfaceTable
#
################################################################################


INTERFACE_TABLE = None
class InterfaceTableHandler(SnmpTableBase):
    if_index_table = {}

    def __init__(self, agent):
        super(InterfaceTableHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        print "check interfaceTable colnum {0}".format(colnum)

        if colnum == TBL_INTERFACE_SPEED:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            speed = data
            if speed < 0 or speed > 6:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == TBL_INTERFACE_DUPLEX:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            duplex = data
            if duplex < 0 or duplex > 3:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == TBL_INTERFACE_RESET_CNT :
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            reset = data
            if reset != 1:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == TBL_INTERFACE_BANDWIDTH:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            bandwidth = data
            if bandwidth < 0 or bandwidth > 10000000:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == TBL_INTERFACE_UNIDIRECTIONAL:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            unidir = data
            if unidir != GLB_STATE_DISABLE and unidir != GLB_STATE_ENABLE and  \
                unidir != GLB_UNIDIR_RX_ONLY and unidir != GLB_UNIDIR_TX_ONLY :
                return self.set_request_error(SNMP_ERR_WRONGVALUE)


        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        exist = None
        print "set interfaceTable colnum {0}".format(colnum)
        indexes = self.get_request_indexes(INTERFACE_TABLE)
        #print indexes
        index = indexes[0]
        #print index
        if_name = InterfaceTableHandler.if_index_table[index]["if_key_name"].replace("_","/")
        #print if_name

        szmsg = execute_cmd('cdbctl read/cdb/interface/{0}'.format(if_name))
        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)

        if colnum == TBL_INTERFACE_SPEED:
            if data == 0:
                speed = "auto"
            elif data == 1:
                speed = "10"
            elif data == 2:
                speed = "100"
            elif data == 3:
                speed = "1000"
            elif data == 4:
                speed = "10G"
            elif data == 5:
                speed = "40G"
            elif data == 6:
                speed = "100G"
            elif data == 7:
                speed = "5G"
            elif data == 8:
                speed = "2.5G"
            else:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            szmsg = execute_cmd('cdbctl update/cdb/interface/{0}/speed/{1}'.format(if_name, speed))

        if colnum == TBL_INTERFACE_DUPLEX:
            if data == 0:
                duplex = "auto"
            elif data == 1:
                duplex = "half"
            elif data == 2:
                duplex = "full"
            else:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            szmsg = execute_cmd('cdbctl update/cdb/interface/{0}/duplex/{1}'.format(if_name, duplex))

        if colnum == TBL_INTERFACE_RESET_CNT:
            szmsg = execute_cmd('cdbctl delete/cdb/interface/{0}/stats'.format(if_name))


        if colnum == TBL_INTERFACE_BANDWIDTH:
            szmsg = execute_cmd('cdbctl update/cdb/interface/{0}/bandwidth/{1}'.format(if_name, data))

        if colnum == TBL_INTERFACE_UNIDIRECTIONAL:
            if data == GLB_STATE_DISABLE:
                unidir = "disable"
            elif data == GLB_STATE_ENABLE:
                unidir = "enable"
            elif data == GLB_UNIDIR_TX_ONLY:
                unidir = "tx-only"
            elif data == GLB_UNIDIR_RX_ONLY:
                unidir = "rx-only"
            else:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)
            szmsg = execute_cmd('cdbctl update/cdb/interface/{0}/unidir/{1}'.format(if_name, unidir))

        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "InterfaceTable subscriber called"
        ds_if_stats_flag = False

        if ds_id != CDB_INVALID_DS_ID:
            return

        if ds_id == CDB_INVALID_DS_ID:
            tbl_interface = tbl_pb2.tbl_interface()
            try:
                tbl_interface.ParseFromString(data)
            except:
                print 'ERROR!! Parse tbl_interface fail'
                return


        #print tbl_id, operation, field_id, ds_id, ds_id2
        #print tbl_interface, ds_if_stats
        #print InterfaceTableHandler.if_index_table
        if tbl_interface.key.name == "cpu":
            return
        GLB_INTERFACE_MUTEX.acquire()

        if operation == CDB_OPER_ADD:
            if ds_if_stats_flag != True:
                #print "ds_if_stats_flag : False, operation ADD"
                #print tbl_interface.ifindex, tbl_interface.key.name
                InterfaceTableHandler.if_index_table[tbl_interface.ifindex] = {"if_key_name": tbl_interface.key.name,
                                                                               "tap_mark_vlan": tbl_interface.tap_mark_vlan,
                                                                               "tap_group_name": tbl_interface.tap}
                newRow = INTERFACE_TABLE.addRow([AGENT.Integer32(tbl_interface.ifindex)])
                if_name = tbl_interface.key.name
                if if_name.find('_') != -1:
                    if_name = if_name.replace('_', '/')
                newRow.setRowCell(TBL_INTERFACE_NAME, AGENT.DisplayString(if_name))

                if tbl_interface.speed != 0:
                    newRow.setRowCell(TBL_INTERFACE_SPEED, AGENT.Integer32(tbl_interface.speed))
                else:
                    newRow.setRowCell(TBL_INTERFACE_SPEED, AGENT.Integer32(tbl_interface.oper_speed))
                if tbl_interface.duplex != 0:
                    newRow.setRowCell(TBL_INTERFACE_DUPLEX, AGENT.Integer32(tbl_interface.duplex))
                else:
                    newRow.setRowCell(TBL_INTERFACE_DUPLEX, AGENT.Integer32(tbl_interface.oper_duplex))

                newRow.setRowCell(TBL_INTERFACE_BANDWIDTH, AGENT.Integer32(tbl_interface.oper_bandwidth))
                if int(tbl_interface.unidir) == 0:
                    newRow.setRowCell(TBL_INTERFACE_UNIDIRECTIONAL, AGENT.Integer32(GLB_STATE_DISABLE))
                else:
                    if tbl_interface.unidir == 1:
                        newRow.setRowCell(TBL_INTERFACE_UNIDIRECTIONAL, AGENT.Integer32(GLB_STATE_ENABLE))
                    elif tbl_interface.unidir == 2:
                        newRow.setRowCell(TBL_INTERFACE_UNIDIRECTIONAL, AGENT.Integer32(GLB_UNIDIR_RX_ONLY))
                    elif tbl_interface.unidir == 3:
                        newRow.setRowCell(TBL_INTERFACE_UNIDIRECTIONAL, AGENT.Integer32(GLB_UNIDIR_TX_ONLY))

                if tbl_interface.running == 1 and tbl_interface.admin_up == 1:
                    newRow.setRowCell(TBL_INTERFACE_LINK_STATUS, AGENT.Integer32(GLB_STATE_ENABLE))
                else:
                    newRow.setRowCell(TBL_INTERFACE_LINK_STATUS, AGENT.Integer32(GLB_STATE_DISABLE))
            else:
                for key in InterfaceTableHandler.if_index_table:
                    print key

        if operation == CDB_OPER_DEL:
            for key in InterfaceTableHandler.if_index_table:
                if tbl_interface.key.name == InterfaceTableHandler.if_index_table[key]["if_key_name"]:
                    del InterfaceTableHandler.if_index_table[key]
                    INTERFACE_TABLE.delRow([AGENT.Integer32(key)])
                    break

        if operation == CDB_OPER_SET:
            if ds_if_stats_flag != True:
                #print "ds_if_stats_flag : False, operation SET"
                #print tbl_interface.ifindex, tbl_interface.key.name
                InterfaceTableHandler.if_index_table[tbl_interface.ifindex] = {"if_key_name": tbl_interface.key.name,
                                                                               "tap_mark_vlan": tbl_interface.tap_mark_vlan,
                                                                               "tap_group_name": tbl_interface.tap}
                curRow = INTERFACE_TABLE.addRow([AGENT.Integer32(tbl_interface.ifindex)])
                if field_id == TBL_INTERFACE_FLD_SPEED or field_id == TBL_INTERFACE_FLD_OPER_SPEED:
                    if tbl_interface.speed != 0 :
                        curRow.setRowCell(TBL_INTERFACE_SPEED, AGENT.Integer32(tbl_interface.speed))
                    else:
                        curRow.setRowCell(TBL_INTERFACE_SPEED, AGENT.Integer32(tbl_interface.oper_speed))
                if field_id == TBL_INTERFACE_FLD_DUPLEX or field_id == TBL_INTERFACE_FLD_OPER_DUPLEX:
                    if tbl_interface.duplex != 0:
                        curRow.setRowCell(TBL_INTERFACE_DUPLEX, AGENT.Integer32(tbl_interface.duplex))
                    else:
                        curRow.setRowCell(TBL_INTERFACE_DUPLEX, AGENT.Integer32(tbl_interface.oper_duplex))
                if field_id == TBL_INTERFACE_FLD_OPER_BANDWIDTH:
                    curRow.setRowCell(TBL_INTERFACE_BANDWIDTH, AGENT.Integer32(tbl_interface.oper_bandwidth))
                if field_id == TBL_INTERFACE_FLD_UNIDIR:
                    #print tbl_interface.unidir
                    if int(tbl_interface.unidir) == 0:
                        curRow.setRowCell(TBL_INTERFACE_UNIDIRECTIONAL, AGENT.Integer32(GLB_STATE_DISABLE))
                    else:
                        curRow.setRowCell(TBL_INTERFACE_UNIDIRECTIONAL, AGENT.Integer32(GLB_STATE_ENABLE))

                if field_id == TBL_INTERFACE_FLD_RUNNING or field_id == TBL_INTERFACE_FLD_ADMIN_UP :
                    if tbl_interface.running == 1 and tbl_interface.admin_up == 1:
                        curRow.setRowCell(TBL_INTERFACE_LINK_STATUS, AGENT.Integer32(GLB_STATE_ENABLE))
                    else:
                        curRow.setRowCell(TBL_INTERFACE_LINK_STATUS, AGENT.Integer32(GLB_STATE_DISABLE))

            else:
                for key in InterfaceTableHandler.if_index_table:
                    print key

        GLB_INTERFACE_MUTEX.release()

################################################################################
#
# Module Init
#
################################################################################
def init(agent_obj, subscriber):
    global AGENT
    AGENT = agent_obj
    enterpriseID = AGENT.get_enterprise_id()

    # interfaceTable register
    interface_table_handler = InterfaceTableHandler(AGENT)
    global INTERFACE_TABLE
    INTERFACE_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.42.1".format(enterpriseID),
        indexes = [
            AGENT.Integer32(0),
        ],
        columns = [
            (TBL_INTERFACE_NAME, AGENT.DisplayString(""), 0),
            (TBL_INTERFACE_SPEED,  AGENT.Integer32(0), 1),
            (TBL_INTERFACE_DUPLEX,  AGENT.Integer32(0), 1),
            (TBL_INTERFACE_RESET_CNT,  AGENT.Integer32(0), 1),
            (TBL_INTERFACE_BANDWIDTH,  AGENT.Integer32(0), 1),
            (TBL_INTERFACE_UNIDIRECTIONAL,  AGENT.Integer32(0), 1),
            (TBL_INTERFACE_LINK_STATUS,  AGENT.Integer32(0), 0),
            (TBL_INTERFACE_5MIN_IN_BITS_RATE,  AGENT.Counter64(0), 0),
            (TBL_INTERFACE_5MIN_IN_PKTS_RATE,  AGENT.Counter64(0), 0),
            (TBL_INTERFACE_5MIN_OUT_BITS_RATE,  AGENT.Counter64(0), 0),
            (TBL_INTERFACE_5MIN_OUT_PKTS_RATE,  AGENT.Counter64(0), 0),

        ],
        extendable = True,
        nodehandle = interface_table_handler.node_handler
    )
    subscriber.subscribeTable(TBL_INTERFACE, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, interface_table_handler.subscribe_handler)
    GLB_TABLE_DICT["interface"] = INTERFACE_TABLE
