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
import datetime

AGENT = None

################################################################################
################################################################################
#
# operate enterprise.1.3
#
################################################################################
################################################################################
# Oid indexes

# Macro definitions
SNMP_OPERATE_CONFIGSAVE_WRITE = 1

SNMP_OPERATE_RELOAD_WITHOUT_SAVE = 1
SNMP_OPERATE_RELOAD_WITH_SAVE    = 2

SNMP_OPERATE_RESETTOFACTORY_RESET = 1

################################################################################
#
# configSave enterprise.1.3.1
#
################################################################################
CONFIGSAVE = None
class ConfigSaveHandler(SnmpTableBase):
    def __init__(self, agent):
        super(ConfigSaveHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        oper = data
        if oper != SNMP_OPERATE_CONFIGSAVE_WRITE:
            return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        configfile = 'config-'+str(datetime.datetime.now().microsecond)

        szmsg = execute_cmd('cdbctl buildcfg/startup/{0}/cdb'.format(configfile))
        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)

        execute_cmd('cp /tmp/{0} /mnt/flash/startup-config.conf'.format(configfile))
        execute_cmd('rm -rf /tmp/{0}'.format(configfile))

        szmsg = execute_cmd('cdbctl update/cdb/sys/send_trap/write_success')
        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)
        return SNMP_ERR_NOERROR

################################################################################
#
# reload enterprise.1.3.2
#
################################################################################
RELOAD = None
class ReloadHandler(SnmpTableBase):
    def __init__(self, agent):
        super(ReloadHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        oper = data
        if oper != SNMP_OPERATE_RELOAD_WITHOUT_SAVE and oper != SNMP_OPERATE_RELOAD_WITH_SAVE:
            return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        oper = data

        if oper == SNMP_OPERATE_RELOAD_WITH_SAVE:
            configfile = 'config-'+str(datetime.datetime.now().microsecond)

            szmsg = execute_cmd('cdbctl buildcfg/startup/{0}/cdb'.format(configfile))
            if szmsg.find('%') != -1:
                return self.set_request_error(SNMP_ERR_GENERR)

            execute_cmd('cp /tmp/{0} /mnt/flash/startup-config.conf'.format(configfile))


            szmsg = execute_cmd('cdbctl update/cdb/sys/reboot/save')
            szmsg = execute_cmd("reboot_record 0x06 1>/dev/null 2>&1 && sync")
        else:

            szmsg = execute_cmd('cdbctl update/cdb/sys/reboot/')
            szmsg = execute_cmd("reboot_record 0x06 1>/dev/null 2>&1 && sync")

        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

################################################################################
#
# resetToFactory enterprise.1.3.20
#
################################################################################
RESETTOFACTORY = None
class ResetToFactoryHandler(SnmpTableBase):
    def __init__(self, agent):
        super(ResetToFactoryHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        oper = data
        if oper != SNMP_OPERATE_RESETTOFACTORY_RESET:
            return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        execute_cmd('rm /mnt/flash/startup-config.conf')
        szmsg = execute_cmd('cdbctl update/cdb/sys/reboot/')
        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)
        return SNMP_ERR_NOERROR

################################################################################
#
# Module Init
#
################################################################################
def init(agent, subscriber):
    global AGENT
    AGENT = agent
    enterpriseID = AGENT.get_enterprise_id()

    configsave_handler = ConfigSaveHandler(AGENT)
    global CONFIGSAVE
    CONFIGSAVE = AGENT.Integer32(
        oidstr  = "1.3.6.1.4.1.{0}.1.3.1".format(enterpriseID),
        initval = 0,
        nodehandle = configsave_handler.node_handler
    )

    reload_handler = ReloadHandler(AGENT)
    global RELOAD
    RELOAD = AGENT.Integer32(
        oidstr  = "1.3.6.1.4.1.{0}.1.3.2".format(enterpriseID),
        initval = 0,
        nodehandle = reload_handler.node_handler
    )

    resettofacory_handler = ResetToFactoryHandler(AGENT)
    global RESETTOFACTORY
    RESETTOFACTORY = AGENT.Integer32(
        oidstr  = "1.3.6.1.4.1.{0}.1.3.20".format(enterpriseID),
        initval = 0,
        nodehandle = resettofacory_handler.node_handler
    )
