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
# interfaces 1.3.6.1.2.1.2
#
################################################################################
################################################################################

################################################################################
#
# ifTable 1.3.6.1.2.1.2.2
#
################################################################################


def get_interface_speed(tbl_interface):
    if tbl_interface.oper_bandwidth*1000 > 4294967295:
        ifspeed = 4294967295
    else:
        ifspeed = tbl_interface.oper_bandwidth*1000
    return ifspeed

# Table Handler
IF_TABLE = None
class IfTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(IfTableHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if colnum == IF_TBL_MEM_ADMIN_STATUS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)

            if data != GLB_STATE_ENABLE and data != GLB_STATE_DISABLE:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        if colnum == IF_TBL_MEM_ADMIN_STATUS:
            indexes = self.get_request_indexes(IF_TABLE)
            ifindex = indexes[0]

            if str(ifindex) not in GLB_IF_INDEX_DICT:
                return self.set_request_error(SNMP_ERR_NOCREATION)

            ifname = GLB_IF_INDEX_DICT[str(ifindex)]
            if data == GLB_STATE_ENABLE:
                szmsg = execute_cmd('cdbctl update/cdb/interface/{0}/up/1'.format(ifname))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
            elif data == GLB_STATE_DISABLE:
                szmsg = execute_cmd('cdbctl update/cdb/interface/{0}/up/0'.format(ifname))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_interface = tbl_pb2.tbl_interface()
        try:
            tbl_interface.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_interface fail'
            return

        if tbl_interface.key.name == "cpu":
            return

        GLB_INTERFACE_MUTEX.acquire()

        if operation == CDB_OPER_ADD:
            GLB_IF_NAME_DICT[tbl_interface.key.name] = tbl_interface.ifindex
            GLB_IF_INDEX_DICT[str(tbl_interface.ifindex)] = tbl_interface.key.name
            stats = [0] * TBL_PORT_STATS_FLD_MAX
            GLB_IF_STATS_DICT[tbl_interface.key.name] = stats

            new_row = IF_TABLE.addRow([AGENT.Integer32(tbl_interface.ifindex)])
            new_row.setRowCell(IF_TBL_MEM_INDEX, AGENT.Integer32(tbl_interface.ifindex))
            new_row.setRowCell(IF_TBL_MEM_DESCR, AGENT.DisplayString(tbl_interface.key.name.replace("_", "/")))
            new_row.setRowCell(IF_TBL_MEM_MTU, AGENT.Integer32(tbl_interface.mtu))
            speed = get_interface_speed(tbl_interface)
            new_row.setRowCell(IF_TBL_MEM_SPEED, AGENT.Unsigned32(speed))
            if tbl_interface.hw_type == GLB_IF_TYPE_LINKAGG_IF or tbl_interface.hw_type == GLB_IF_TYPE_VLAN_IF:
                new_row.setRowCell(IF_TBL_MEM_PHYS_ADDRESS, AGENT.OctetString(tbl_interface.mac_addr.mac))
            elif tbl_interface.hw_type == GLB_IF_TYPE_LOOPBACK_IF:
                new_row.setRowCell(IF_TBL_MEM_PHYS_ADDRESS, AGENT.OctetString(""))
            else:
                new_row.setRowCell(IF_TBL_MEM_PHYS_ADDRESS, AGENT.OctetString(tbl_interface.hw_mac_addr.mac))
            if tbl_interface.admin_up:
                new_row.setRowCell(IF_TBL_MEM_ADMIN_STATUS, AGENT.Integer32(GLB_STATE_ENABLE))
            if tbl_interface.running == 1 and tbl_interface.admin_up == 1:
                new_row.setRowCell(IF_TBL_MEM_OPER_STATUS, AGENT.Integer32(GLB_STATE_ENABLE))

            if tbl_interface.hw_type == GLB_IF_TYPE_LINKAGG_IF:
                new_row.setRowCell(IF_TBL_MEM_TYPE, AGENT.Integer32(GLB_IF_TYPE_AGG))
            elif tbl_interface.hw_type == GLB_IF_TYPE_VLAN_IF:
                new_row.setRowCell(IF_TBL_MEM_TYPE, AGENT.Integer32(GLB_IF_TYPE_VLANIF))
            elif tbl_interface.hw_type == GLB_IF_TYPE_LOOPBACK_IF:
                new_row.setRowCell(IF_TBL_MEM_TYPE, AGENT.Integer32(GLB_IF_TYPE_SOFTWARELOOPBACK))
            elif tbl_interface.hw_type == GLB_IF_TYPE_TUNNEL_IF:
                new_row.setRowCell(IF_TBL_MEM_TYPE, AGENT.Integer32(GLB_IF_TYPE_TUNNEL))
            else:
                new_row.setRowCell(IF_TBL_MEM_TYPE, AGENT.Integer32(GLB_IF_TYPE_ETHERNETCSMACD))

        if operation == CDB_OPER_DEL:
            ifindex = GLB_IF_NAME_DICT[tbl_interface.key.name]
            del(GLB_IF_INDEX_DICT[str(ifindex)])
            del(GLB_IF_STATS_DICT[tbl_interface.key.name])
            del(GLB_IF_NAME_DICT[tbl_interface.key.name])

            IF_TABLE.delRow([AGENT.Integer32(ifindex)])

        if operation == CDB_OPER_SET:
            old_row = IF_TABLE.addRow([AGENT.Integer32(tbl_interface.ifindex)])
            if field_id == TBL_INTERFACE_FLD_BANDWIDTH or field_id == TBL_INTERFACE_FLD_OPER_BANDWIDTH:
                speed = get_interface_speed(tbl_interface)
                old_row.setRowCell(IF_TBL_MEM_SPEED, AGENT.Unsigned32(speed))

            if field_id == TBL_INTERFACE_FLD_MTU:
                old_row.setRowCell(IF_TBL_MEM_MTU, AGENT.Integer32(tbl_interface.mtu))

            if field_id == TBL_INTERFACE_FLD_ADMIN_UP:
                if tbl_interface.admin_up:
                    old_row.setRowCell(IF_TBL_MEM_ADMIN_STATUS, AGENT.Integer32(GLB_STATE_ENABLE))
                else:
                    old_row.setRowCell(IF_TBL_MEM_ADMIN_STATUS, AGENT.Integer32(GLB_STATE_DISABLE))
                speed = get_interface_speed(tbl_interface)
                old_row.setRowCell(IF_TBL_MEM_SPEED, AGENT.Unsigned32(speed))

            if field_id == TBL_INTERFACE_FLD_RUNNING or field_id == TBL_INTERFACE_FLD_ADMIN_UP:
                if tbl_interface.running == 1 and tbl_interface.admin_up == 1:
                    old_row.setRowCell(IF_TBL_MEM_OPER_STATUS, AGENT.Integer32(GLB_STATE_ENABLE))
                else:
                    old_row.setRowCell(IF_TBL_MEM_OPER_STATUS, AGENT.Integer32(GLB_STATE_DISABLE))
                speed = get_interface_speed(tbl_interface)
                old_row.setRowCell(IF_TBL_MEM_SPEED, AGENT.Unsigned32(speed))

            if field_id == TBL_INTERFACE_FLD_MAC_ADDR:
                if tbl_interface.hw_type == GLB_IF_TYPE_VLAN_IF or tbl_interface.hw_type == GLB_IF_TYPE_LINKAGG_IF:
                    old_row.setRowCell(IF_TBL_MEM_PHYS_ADDRESS, AGENT.OctetString(tbl_interface.mac_addr.mac))

            if field_id == TBL_INTERFACE_FLD_HW_MAC_ADDR:
                if tbl_interface.hw_type == GLB_IF_TYPE_PORT_IF:
                    old_row.setRowCell(IF_TBL_MEM_PHYS_ADDRESS, AGENT.OctetString(tbl_interface.hw_mac_addr.mac))

        GLB_INTERFACE_MUTEX.release()

################################################################################
#
# Module Init
#
################################################################################
def init(agent_obj, subscriber):
    global AGENT
    AGENT = agent_obj
    #enterpriseID = AGENT.get_enterprise_id()

    if_table_handler = IfTableHandler(AGENT)
    global IF_TABLE
    IF_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.2.1.2.2",
        indexes = [
            AGENT.Integer32(0),
        ],
        no_idx_col=True,
        columns = [
            (IF_TBL_MEM_INDEX,        AGENT.Integer32(0), 0),
            (IF_TBL_MEM_TYPE,         AGENT.Integer32(0), 0),
            (IF_TBL_MEM_DESCR,        AGENT.DisplayString(""), 0),
            (IF_TBL_MEM_MTU,          AGENT.Integer32(0), 0),
            (IF_TBL_MEM_SPEED,        AGENT.Unsigned32(0), 0),
            (IF_TBL_MEM_PHYS_ADDRESS, AGENT.OctetString(""), 0),
            (IF_TBL_MEM_ADMIN_STATUS, AGENT.Integer32(GLB_STATE_DISABLE), 1),
            (IF_TBL_MEM_OPER_STATUS,  AGENT.Integer32(GLB_STATE_DISABLE), 0),
            #(IF_TBL_MEM_LAST_CHANGE,  AGENT.TimeTicks(0), 0),
            (IF_TBL_MEM_IN_OCTECTS, AGENT.Counter32(0), 0),
            (IF_TBL_MEM_IN_UCASTPKTS, AGENT.Counter32(0), 0),
            (IF_TBL_MEM_IN_NUCASTPKTS, AGENT.Counter32(0), 0),
            (IF_TBL_MEM_IN_DISCARDS,  AGENT.Counter32(0), 0),
            (IF_TBL_MEM_OUT_OCTECTS, AGENT.Counter32(0), 0),
            (IF_TBL_MEM_OUT_UCAST_PKTS, AGENT.Counter32(0), 0),
            (IF_TBL_MEM_OUT_NUCAST_PKTS, AGENT.Counter32(0), 0),
            (IF_TBL_MEM_IN_ERRORS,    AGENT.Counter32(0), 0),
            (IF_TBL_MEM_OUT_DISCARDS, AGENT.Counter32(0), 0),
            (IF_TBL_MEM_OUT_ERRORS,   AGENT.Counter32(0), 0),
        ],
        extendable = True,
        nodehandle = if_table_handler.node_handler
    )
    subscriber.subscribeTable(TBL_INTERFACE, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, if_table_handler.subscribe_handler)
    GLB_TABLE_DICT["iftable"] = IF_TABLE
