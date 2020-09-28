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
# management enterprise.43
#
################################################################################
################################################################################
# Oid indexes

# Macro definitions

################################################################################
#
# managementIpAddress enterprise.43.100
#
################################################################################
MGMTIPADDR = None
class MgmtIpAddrHandler(SnmpTableBase):
    def __init__(self, agent):
        super(MgmtIpAddrHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_OCTET_STR:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        addr = data
        if addr == "":
            return SNMP_ERR_NOERROR

        match = re.search("^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)(\/(3[0-2]|[12]?\d))$", addr)
        if not match:
            return self.set_request_error(SNMP_ERR_WRONGVALUE)
        if len(match.group(0)) != len(addr):
            return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        addr = data

        if addr == "":
            cmd = 'cdbctl update/cdb/sys/manage_if/addr/0.0.0.0/0'
        else:
            cmd = 'cdbctl update/cdb/sys/manage_if/addr/{0}'.format(addr)

        szmsg = execute_cmd(cmd)
        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_manage_if = tbl_pb2.tbl_manage_if()
        try:
            tbl_manage_if.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_manage_if fail'
            return

        ip_addr = socket.inet_ntoa(struct.pack('!L', tbl_manage_if.addr.ipv4_addr))
        mgmt_addr = "{0}/{1}".format(ip_addr, tbl_manage_if.addr.prefixlen)
        MGMTIPADDR.update(mgmt_addr)

################################################################################
#
# managementIpGateway enterprise.43.101
#
################################################################################
MGMTIPGATEWAY = None
class MgmtIpGatewayHandler(SnmpTableBase):
    def __init__(self, agent):
        super(MgmtIpGatewayHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_IPADDRESS:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        addr = data
        if addr == "":
            return SNMP_ERR_NOERROR

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        addr = data

        if addr == "":
            cmd = 'cdbctl update/cdb/sys/manage_if/gateway/0.0.0.0'
        else:
            cmd = 'cdbctl update/cdb/sys/manage_if/gateway/{0}'.format(addr)

        szmsg = execute_cmd(cmd)
        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_manage_if = tbl_pb2.tbl_manage_if()
        try:
            tbl_manage_if.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_manage_if fail'
            return

        mgmt_gw = socket.inet_ntoa(struct.pack('!L', tbl_manage_if.gateway.ipv4_addr))
        MGMTIPGATEWAY.update(mgmt_gw)

################################################################################
#
# Module Init
#
################################################################################
def init(agent, subscriber):
    global AGENT
    AGENT = agent
    enterpriseID = AGENT.get_enterprise_id()

    mgmtipaddr_handler = MgmtIpAddrHandler(AGENT)
    global MGMTIPADDR
    MGMTIPADDR = AGENT.DisplayString(
        oidstr  = "1.3.6.1.4.1.{0}.43.100".format(enterpriseID),
        initval = "",
        nodehandle = mgmtipaddr_handler.node_handler
    )
    subscriber.subscribeTable(TBL_MANAGE_IF, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_MANAGE_IF_FLD_ADDR, mgmtipaddr_handler.subscribe_handler)

    mgmtgateway_handler = MgmtIpGatewayHandler(AGENT)
    global MGMTIPGATEWAY
    MGMTIPGATEWAY = AGENT.IpAddress(
        oidstr  = "1.3.6.1.4.1.{0}.43.101".format(enterpriseID),
        initval = "0.0.0.0",
        nodehandle = mgmtgateway_handler.node_handler
    )
    subscriber.subscribeTable(TBL_MANAGE_IF, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_MANAGE_IF_FLD_GATEWAY, mgmtgateway_handler.subscribe_handler)
