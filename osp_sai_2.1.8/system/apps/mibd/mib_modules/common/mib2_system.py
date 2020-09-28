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
# system 1.3.6.1.2.1.1
#
################################################################################
################################################################################
# Oid indexes

# Macro definitions

################################################################################
#
# sysDescr 1.3.6.1.2.1.1.1
#
################################################################################
SYSDESCR = None
class SysDescrHandler(SnmpTableBase):
    def __init__(self, agent):
        super(SysDescrHandler, self).__init__(agent)
        self.sys_descr = None

    def get(self, colnum):
        if not self.sys_descr:
            self.sys_descr = execute_cmd('cat /switch/etc/snmpd.conf | grep sysdescr')
        self.sys_descr = self.sys_descr.replace("sysdescr ","",1)
        return self.snmp_return_string(self.sys_descr)

################################################################################
#
# sysObjectID 1.3.6.1.2.1.1.2
#
################################################################################
SYSOBJID = None
class SysObjIdHandler(SnmpTableBase):
    def __init__(self, agent):
        super(SysObjIdHandler, self).__init__(agent)
        sysObjId = (c_oid * MAX_OID_LEN)()
        sysObjId[0] = 0

    #def get(self, colnum):
    #    ticks = 100 * int(execute_cmd('cat /proc/uptime').split('.')[0])
    #    return self.snmp_return_object_id()

################################################################################
#
# sysUpTime 1.3.6.1.2.1.1.3
#
################################################################################
SYSUPTIME = None
class SysUptimeHandler(SnmpTableBase):
    def __init__(self, agent):
        super(SysUptimeHandler, self).__init__(agent)

    def get(self, colnum):
        ticks = 100 * int(execute_cmd('cat /proc/uptime').split('.')[0])
        return self.snmp_return_timeticks(ticks)

################################################################################
#
# sysContact 1.3.6.1.2.1.1.4
#
################################################################################
SYS_CONTACT_LEN = 256
SYSCONTACT = None
class SysContactHandler(SnmpTableBase):
    def __init__(self, agent):
        super(SysContactHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_OCTET_STR:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        contact = data
        if contact == "":
            return SNMP_ERR_NOERROR

        if len(contact) > SYS_CONTACT_LEN:
            return self.set_request_error(SNMP_ERR_WRONGLENGTH)

        if len(re.search("^[0-9a-zA-Z\s?]+$", contact[0:len(contact)]).group(0)) != len(contact):
            return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        contact = data

        if contact == "":
            cmd = 'cdbctl update/cdb/app/snmp_cfg/contact'
        else:
            encap_contact = execute_cmd('fnconvert -c encoding -m "{0}"'.format(contact))
            cmd = 'cdbctl update/cdb/app/snmp_cfg/contact/{0}'.format(encap_contact)

        szmsg = execute_cmd(cmd)
        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_snmp_cfg = tbl_pb2.tbl_snmp_cfg()
        try:
            tbl_snmp_cfg.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_snmp_cfg fail'
            return

        SYSCONTACT.update(tbl_snmp_cfg.contact.info)

################################################################################
#
# sysName 1.3.6.1.2.1.1.5
#
################################################################################
SYS_NAME_LEN = 16
SYSNAME = None
class SysNameHandler(SnmpTableBase):
    def __init__(self, agent):
        super(SysNameHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_OCTET_STR:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        new_name = data
        if new_name == "":
            return SNMP_ERR_NOERROR

        if len(new_name) > SYS_NAME_LEN:
            return self.set_request_error(SNMP_ERR_WRONGLENGTH)

        if len(re.search("^[0-9a-zA-Z._-]*", new_name[0:len(new_name)]).group(0)) != len(new_name):
            return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        new_name = data

        if new_name == "":
            new_name = "Switch"

        szmsg = execute_cmd('cdbctl update/cdb/sys/sys_global/hostname/{0}'.format(new_name))
        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_sys_global = tbl_pb2.tbl_sys_global()
        try:
            tbl_sys_global.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_sys_global fail'
            return

        SYSNAME.update(tbl_sys_global.hostname)

################################################################################
#
# sysLocation 1.3.6.1.2.1.1.6
#
################################################################################
SYS_LOCATION_LEN = 256
SYSLOCATION = None
class SysLocationHandler(SnmpTableBase):
    def __init__(self, agent):
        super(SysLocationHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_OCTET_STR:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        location = data
        if location == "":
            return SNMP_ERR_NOERROR

        if len(location) > SYS_CONTACT_LEN:
            return self.set_request_error(SNMP_ERR_WRONGLENGTH)

        if len(re.search("^[0-9a-zA-Z\s?]+$", location[0:len(location)]).group(0)) != len(location):
            return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        location = data

        if location == "":
            cmd = 'cdbctl update/cdb/app/snmp_cfg/location'
        else:
            encap_loc = execute_cmd('fnconvert -c encoding -m "{0}"'.format(location))
            cmd = 'cdbctl update/cdb/app/snmp_cfg/location/{0}'.format(encap_loc)

        szmsg = execute_cmd(cmd)
        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_snmp_cfg = tbl_pb2.tbl_snmp_cfg()
        try:
            tbl_snmp_cfg.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_snmp_cfg fail'
            return

        SYSLOCATION.update(tbl_snmp_cfg.location.info)

################################################################################
#
# Module Init
#
################################################################################
def init(agent, subscriber):
    global AGENT
    AGENT = agent

    sysdescr_handler = SysDescrHandler(AGENT)
    global SYSDESCR
    SYSDESCR = AGENT.DisplayString(
        oidstr  = "1.3.6.1.2.1.1.1",
        initval = "",
        nodehandle = sysdescr_handler.node_handler
    )

    #sysobjid_handler = SysObjIdHandler(AGENT)
    #global SYSOBJID
    #SYSOBJID = AGENT.ObjectIdentify(
    #    oidstr="1.3.6.1.2.1.1.2",
    #    initval=0,
    #    nodehandle=sysobjid_handler.node_handler
    #)

    sysuptime_handler = SysUptimeHandler(AGENT)
    global SYSUPTIME
    SYSUPTIME = AGENT.TimeTicks(
        oidstr  = "1.3.6.1.2.1.1.3",
        initval = 0,
        nodehandle = sysuptime_handler.node_handler
    )

    syscontact_handler = SysContactHandler(AGENT)
    global SYSCONTACT
    SYSCONTACT = AGENT.DisplayString(
        oidstr  = "1.3.6.1.2.1.1.4",
        initval = "",
        nodehandle = syscontact_handler.node_handler
    )
    subscriber.subscribeTable(TBL_SNMP_CFG, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_SNMP_CFG_FLD_CONTACT, syscontact_handler.subscribe_handler)

    sysname_handler = SysNameHandler(AGENT)
    global SYSNAME
    SYSNAME = AGENT.DisplayString(
        oidstr  = "1.3.6.1.2.1.1.5",
        initval = "Switch",
        nodehandle = sysname_handler.node_handler
    )
    subscriber.subscribeTable(TBL_SYS_GLOBAL, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_SYS_GLOBAL_FLD_HOSTNAME, sysname_handler.subscribe_handler)

    syslocation_handler = SysLocationHandler(AGENT)
    global SYSLOCATION
    SYSLOCATION = AGENT.DisplayString(
        oidstr  = "1.3.6.1.2.1.1.6",
        initval = "",
        nodehandle = syslocation_handler.node_handler
    )
    subscriber.subscribeTable(TBL_SNMP_CFG, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_SNMP_CFG_FLD_LOCATION, syslocation_handler.subscribe_handler)
