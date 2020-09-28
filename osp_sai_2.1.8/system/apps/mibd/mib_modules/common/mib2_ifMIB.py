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

# Macro definitions
IF_ALIAS_NAME_LEN = 16

# Function definitions
def get_interface_speed(tbl_interface):
    speed = tbl_interface.oper_bandwidth/1000
    return speed



# Table Handler
IFX_TABLE = None
class IfXTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(IfXTableHandler, self).__init__(agent)
        self.if_name_dict = dict()

    def check(self, colnum, var_type, data):
        if colnum == IFX_TBL_MEM_ALIAS:
            if var_type != ASN_OCTET_STR:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)

            alias = data
            if len(alias) > IF_ALIAS_NAME_LEN:
                return self.set_request_error(SNMP_ERR_WRONGLENGTH)

            if len(re.search("^[0-9a-zA-Z._-]*", alias[0:len(alias)]).group(0)) != len(alias):
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        if colnum == IFX_TBL_MEM_ALIAS:
            indexes = self.get_request_indexes(IFX_TABLE)
            ifindex = indexes[0]

            if str(ifindex) not in GLB_IF_INDEX_DICT:
                return self.set_request_error(SNMP_ERR_NOCREATION)

            ifname = GLB_IF_INDEX_DICT[str(ifindex)]
            alias = data
            if alias == "":
                cmd = 'cdbctl update/cdb/interface/{0}/desc'.format(ifname)
            else:
                encap_alias = execute_cmd('fnconvert -c encoding -m "{0}"'.format(alias))
                cmd = 'cdbctl update/cdb/interface/{0}/desc/{1}'.format(ifname, encap_alias)

            szmsg = execute_cmd(cmd)
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
            self.if_name_dict[tbl_interface.key.name] = tbl_interface.ifindex

            new_row = IFX_TABLE.addRow([AGENT.Integer32(tbl_interface.ifindex)])
            speed = get_interface_speed(tbl_interface)
            new_row.setRowCell(IFX_TBL_MEM_NAME, AGENT.DisplayString(tbl_interface.key.name.replace("_", "/")))
            #print "set IFX_TBL_MEM_NAME ",tbl_interface.key.name
            new_row.setRowCell(IFX_TBL_MEM_HIGH_SPEED, AGENT.Unsigned32(speed))
            new_row.setRowCell(IFX_TBL_MEM_ALIAS, AGENT.DisplayString(tbl_interface.desc))

        if operation == CDB_OPER_DEL:
            ifindex = self.if_name_dict[tbl_interface.key.name]
            IFX_TABLE.delRow([AGENT.Integer32(ifindex)])

            del(self.if_name_dict[tbl_interface.key.name])

        if operation == CDB_OPER_SET:
            old_row = IFX_TABLE.addRow([AGENT.Integer32(tbl_interface.ifindex)])
            if field_id == TBL_INTERFACE_FLD_BANDWIDTH or field_id == TBL_INTERFACE_FLD_OPER_BANDWIDTH or \
               field_id == TBL_INTERFACE_FLD_UP or field_id == TBL_INTERFACE_FLD_RUNNING or \
               field_id == TBL_INTERFACE_FLD_ADMIN_UP:
                speed = get_interface_speed(tbl_interface)
                old_row.setRowCell(IFX_TBL_MEM_HIGH_SPEED, AGENT.Unsigned32(speed))

            if field_id == TBL_INTERFACE_FLD_DESC:
                old_row.setRowCell(IFX_TBL_MEM_ALIAS, AGENT.DisplayString(tbl_interface.desc))

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

    ifx_table_handler = IfXTableHandler(AGENT)
    global IFX_TABLE
    IFX_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.2.1.31.1.1",
        indexes = [
            AGENT.Integer32(0),
        ],
        no_idx_col= True,
        columns = [
            (IFX_TBL_MEM_NAME,                  AGENT.DisplayString(""), 0),
            (IFX_TBL_MEM_IN_MULTICAST_PKTS,     AGENT.Counter32(0), 0),
            (IFX_TBL_MEM_IN_BROADCAST_PKTS,     AGENT.Counter32(0), 0),
            (IFX_TBL_MEM_OUT_MULTICAST_PKTS,    AGENT.Counter32(0), 0),
            (IFX_TBL_MEM_OUT_BROADCAST_PKTS,    AGENT.Counter32(0), 0),
            (IFX_TBL_MEM_HC_IN_OCTETS,          AGENT.Counter64(0), 0),
            (IFX_TBL_MEM_HC_IN_UCAST_PKTS,      AGENT.Counter64(0), 0),
            (IFX_TBL_MEM_HC_IN_MULTICAST_PKTS,  AGENT.Counter64(0), 0),
            (IFX_TBL_MEM_HC_IN_BROADCAST_PKTS,  AGENT.Counter64(0), 0),
            (IFX_TBL_MEM_HC_OUT_OCTETS,         AGENT.Counter64(0), 0),
            (IFX_TBL_MEM_HC_OUT_UCAST_PKTS,     AGENT.Counter64(0), 0),
            (IFX_TBL_MEM_HC_OUT_MULTICAST_PKTS, AGENT.Counter64(0), 0),
            (IFX_TBL_MEM_HC_OUT_BROADCAST_PKTS, AGENT.Counter64(0), 0),
            (IFX_TBL_MEM_HIGH_SPEED,            AGENT.Unsigned32(0), 0),
            (IFX_TBL_MEM_ALIAS,                 AGENT.DisplayString(""), 1),
        ],
        extendable = True,
        nodehandle = ifx_table_handler.node_handler
    )
    subscriber.subscribeTable(TBL_INTERFACE, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, ifx_table_handler.subscribe_handler)
    GLB_TABLE_DICT["ifxtable"] = IFX_TABLE
