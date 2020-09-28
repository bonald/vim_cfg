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
# linkAgg
#
################################################################################
################################################################################

################################################################################
#
#staticLinkAggTable
#
################################################################################
# Oid indexs
TBL_STATIC_LINKAGG_NUMBER        = 1
TBL_STATIC_LINKAGG_ROW_STATUS    = 10
GLB_LAG_GROUP_NUM                = 55

GLB_LAG_LOAD_BALANCE_SRC_MAC       = (1 << 0)
GLB_LAG_LOAD_BALANCE_DST_MAC       = (1 << 1)
GLB_LAG_LOAD_BALANCE_SRC_IP        = (1 << 2)
GLB_LAG_LOAD_BALANCE_DST_IP        = (1 << 3)
GLB_LAG_LOAD_BALANCE_IP_PROTOCOL   = (1 << 4)
GLB_LAG_LOAD_BALANCE_SRC_PORT      = (1 << 5)
GLB_LAG_LOAD_BALANCE_DST_PORT      = (1 << 6)

STATIC_LINKAGG_TABLE = None
class StaticLinkAggTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(StaticLinkAggTableHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        print "check ExTable colnum {0}".format(colnum)
        indexes = self.get_request_indexes(STATIC_LINKAGG_TABLE)
        #print indexes
        lagid = indexes[0]
        ifidx = indexes[1]
        if lagid > GLB_LAG_GROUP_NUM or lagid < 1:
            return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if_name = InterfaceTableHandler.if_index_table[ifidx]["if_key_name"].replace("_", "/")
        if if_name.find("eth") == -1:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        if colnum == TBL_STATIC_LINKAGG_ROW_STATUS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            rowstatus = data
            if rowstatus != SNMP_ROW_CREATEANDGO and \
                rowstatus != SNMP_ROW_DESTROY:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        exist = None
        print "set staticAgg Table colnum {0}".format(colnum)
        indexes = self.get_request_indexes(STATIC_LINKAGG_TABLE)
        #print indexes
        lagid = indexes[0]
        ifidx = indexes[1]

        if_name = InterfaceTableHandler.if_index_table[ifidx]["if_key_name"].replace("_", "/")
        if if_name.find("eth") == -1:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        szmsg = execute_cmd('cdbctl read/cdb/interface/{0}'.format(if_name))
        if szmsg.find('%') != -1:
            return self.set_request_error(SNMP_ERR_GENERR)

        szmsg = execute_cmd('cdbctl read/cdb/interface/{0}/lag'.format(if_name))
        if szmsg.find('mode=static') != -1:
            exist = True
        else:
            exist = False

        if colnum == TBL_STATIC_LINKAGG_ROW_STATUS:
            print "colnum 10 TBL_STATIC_LINKAGG_ROW_STATUS"
            rowstatus = data
            if rowstatus == SNMP_ROW_CREATEANDGO:
                if exist:
                    return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)
                szmsg = execute_cmd('cdbctl create/cdb/interface/{0}/lag/{1}/mode/static'.format(if_name, lagid))
                #print if_name, szmsg
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    tmp = str(lagid) + str(ifidx)
                    increase_data_sync_add("STATIC_LINKAGG_TABLE", tmp)

            if rowstatus == SNMP_ROW_DESTROY:
                if not exist:
                    return self.set_request_error(SNMP_ERR_NOSUCHNAME)
                szmsg = execute_cmd('cdbctl delete/cdb/interface/{0}/lag/static'.format(if_name))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
                else:
                    tmp = str(lagid) + str(ifidx)
                    increase_data_sync_del("STATIC_LINKAGG_TABLE", tmp)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        if ds_id == DS_LAG:
            ds_lag = ds_pb2.ds_lag()
            try:
                ds_lag.ParseFromString(data)
            except:
                print 'ERROR!! Parse ds_if_stats fail'
                return
        else:
            return

        if ds_lag.parent_if.name.find('eth') == -1:
            return

        if operation == CDB_OPER_ADD:
            for key in InterfaceTableHandler.if_index_table:
                if ds_lag.parent_if.name == InterfaceTableHandler.if_index_table[key]["if_key_name"]:
                    tmp = str(ds_lag.lag_id) + str(key)
                    if check_data_sync_add("STATIC_LINKAGG_TABLE", tmp):
                        decrease_data_sync_add("STATIC_LINKAGG_TABLE", tmp)
                        return
                    #print "ds_lag_flag : True, operation ADD"
                    #print key, ds_lag.parent_if.name, ds_lag.lag_id
                    newRow = STATIC_LINKAGG_TABLE.addRow([AGENT.Integer32(ds_lag.lag_id), AGENT.Integer32(key)])
                    newRow.setRowCell(TBL_STATIC_LINKAGG_ROW_STATUS, AGENT.Integer32(SNMP_ROW_ACTIVE))
                    break

        if operation == CDB_OPER_DEL:
            for key in InterfaceTableHandler.if_index_table:
                if ds_lag.parent_if.name == InterfaceTableHandler.if_index_table[key]["if_key_name"]:
                    tmp = str(ds_lag.lag_id) + str(key)
                    if check_data_sync_del("STATIC_LINKAGG_TABLE", tmp):
                        decrease_data_sync_del("STATIC_LINKAGG_TABLE", tmp)
                        return
                    #print "ds_lag_flag : True, operation DEL"
                    #print key, ds_lag.parent_if.name, ds_lag.lag_id
                    STATIC_LINKAGG_TABLE.delRow([AGENT.Integer32(ds_lag.lag_id), AGENT.Integer32(key)])
                    break

################################################################################
#
#lacpLoadBalanceObject
#
################################################################################
################################################################################
#
#linkAggLoadBalDstMac 1.3.6.1.4.1.entriprise_oid.10.100.1
#
################################################################################

LOADBALANCE_DSTMAC = None
class LoadBalanceDstMacHandler(SnmpTableBase):
    def __init__(self, agent):
        super(LoadBalanceDstMacHandler, self).__init__(agent)


    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        bal_type = data
        if bal_type != GLB_STATE_ENABLE and bal_type != GLB_STATE_DISABLE :
            return self.set_request_error(SNMP_ERR_BADVALUE)
        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        bal_type = data
        if bal_type == GLB_STATE_ENABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/set/dst-mac')
        if bal_type == GLB_STATE_DISABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/unset/dst-mac')

        if szmsg.find('%') != -1:
            self.set_request_error(SNMP_ERR_GENERR)
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "LoadBalanceDstMacHandler subscriber called"
        tbl_lag_global = tbl_pb2.tbl_lag_global()
        try:
            tbl_lag_global.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_lag_global fail'
            return
        #print tbl_lag_global.load_balance

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            if (tbl_lag_global.load_balance & GLB_LAG_LOAD_BALANCE_DST_MAC) == 0:
                LOADBALANCE_DSTMAC.update(GLB_STATE_DISABLE)
            else:
                LOADBALANCE_DSTMAC.update(GLB_STATE_ENABLE)


################################################################################
#
# linkAggLoadBalSrcMac 1.3.6.1.4.1.entriprise_oid.10.100.2
#
################################################################################

LOADBALANCE_SRCMAC = None


class LoadBalanceSrcMacHandler(SnmpTableBase):
    def __init__(self, agent):
        super(LoadBalanceSrcMacHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        bal_type = data
        if bal_type != GLB_STATE_ENABLE and bal_type != GLB_STATE_DISABLE:
            return self.set_request_error(SNMP_ERR_BADVALUE)
        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        bal_type = data
        if bal_type == GLB_STATE_ENABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/set/src-mac')
        if bal_type == GLB_STATE_DISABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/unset/src-mac')

        if szmsg.find('%') != -1:
            self.set_request_error(SNMP_ERR_GENERR)
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "LoadBalanceSrcMacHandler subscriber called"
        tbl_lag_global = tbl_pb2.tbl_lag_global()
        try:
            tbl_lag_global.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_lag_global fail'
            return
        #print tbl_lag_global.load_balance

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            if (tbl_lag_global.load_balance & GLB_LAG_LOAD_BALANCE_SRC_MAC) == 0:
                LOADBALANCE_SRCMAC.update(GLB_STATE_DISABLE)
            else:
                LOADBALANCE_SRCMAC.update(GLB_STATE_ENABLE)


################################################################################
#
# linkAggLoadBalSrcMac 1.3.6.1.4.1.entriprise_oid.10.100.3
#
################################################################################

LOADBALANCE_DSTIP = None


class LoadBalanceDstIpHandler(SnmpTableBase):
    def __init__(self, agent):
        super(LoadBalanceDstIpHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        loadbal_type = data
        if loadbal_type != GLB_STATE_ENABLE and loadbal_type != GLB_STATE_DISABLE:
            return self.set_request_error(SNMP_ERR_BADVALUE)
        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        loadbal_type = data
        if loadbal_type == GLB_STATE_ENABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/set/dst-ip')
        if loadbal_type == GLB_STATE_DISABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/unset/dst-ip')

        if szmsg.find('%') != -1:
            self.set_request_error(SNMP_ERR_GENERR)
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "LoadBalanceDstIpHandler subscriber called"
        tbl_lag_global = tbl_pb2.tbl_lag_global()
        try:
            tbl_lag_global.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_lag_global fail'
            return
        #print tbl_lag_global.load_balance

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            if (tbl_lag_global.load_balance & GLB_LAG_LOAD_BALANCE_DST_IP) == 0:
                LOADBALANCE_DSTIP.update(GLB_STATE_DISABLE)
            else:
                LOADBALANCE_DSTIP.update(GLB_STATE_ENABLE)


################################################################################
#
# linkAggLoadBalSrcMac 1.3.6.1.4.1.entriprise_oid.10.100.4
#
################################################################################

LOADBALANCE_SRCIP = None


class LoadBalanceSrcIpHandler(SnmpTableBase):
    def __init__(self, agent):
        super(LoadBalanceSrcIpHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        type = data
        if type != GLB_STATE_ENABLE and type != GLB_STATE_DISABLE:
            return self.set_request_error(SNMP_ERR_BADVALUE)
        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        type = data
        if type == GLB_STATE_ENABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/set/src-ip')
        if type == GLB_STATE_DISABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/unset/src-ip')

        if szmsg.find('%') != -1:
            self.set_request_error(SNMP_ERR_GENERR)
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "LoadBalanceSrcIpHandler subscriber called"
        tbl_lag_global = tbl_pb2.tbl_lag_global()
        try:
            tbl_lag_global.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_lag_global fail'
            return
        #print tbl_lag_global.load_balance

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            if (tbl_lag_global.load_balance & GLB_LAG_LOAD_BALANCE_SRC_IP) == 0:
                LOADBALANCE_SRCIP.update(GLB_STATE_DISABLE)
            else:
                LOADBALANCE_SRCIP.update(GLB_STATE_ENABLE)


################################################################################
#
# linkAggLoadBalSrcMac 1.3.6.1.4.1.entriprise_oid.10.100.5
#
################################################################################

LOADBALANCE_IPPRO = None


class LoadBalanceIpProHandler(SnmpTableBase):
    def __init__(self, agent):
        super(LoadBalanceIpProHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        type = data
        if type != GLB_STATE_ENABLE and type != GLB_STATE_DISABLE:
            return self.set_request_error(SNMP_ERR_BADVALUE)
        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        type = data
        if type == GLB_STATE_ENABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/set/ip-protocol')
        if type == GLB_STATE_DISABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/unset/ip-protocol')

        if szmsg.find('%') != -1:
            self.set_request_error(SNMP_ERR_GENERR)
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "LoadBalanceIpProHandler subscriber called"
        tbl_lag_global = tbl_pb2.tbl_lag_global()
        try:
            tbl_lag_global.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_lag_global fail'
            return
        #print tbl_lag_global.load_balance

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            if (tbl_lag_global.load_balance & GLB_LAG_LOAD_BALANCE_IP_PROTOCOL) == 0:
                LOADBALANCE_IPPRO.update(GLB_STATE_DISABLE)
            else:
                LOADBALANCE_IPPRO.update(GLB_STATE_ENABLE)


################################################################################
#
# linkAggLoadBalSrcMac 1.3.6.1.4.1.entriprise_oid.10.100.6
#
################################################################################

LOADBALANCE_DSTL4 = None


class LoadBalanceDstL4Handler(SnmpTableBase):
    def __init__(self, agent):
        super(LoadBalanceDstL4Handler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        type = data
        if type != GLB_STATE_ENABLE and type != GLB_STATE_DISABLE:
            return self.set_request_error(SNMP_ERR_BADVALUE)
        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        type = data
        if type == GLB_STATE_ENABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/set/dst-port-l4')
        if type == GLB_STATE_DISABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/unset/dst-port-l4')

        if szmsg.find('%') != -1:
            self.set_request_error(SNMP_ERR_GENERR)
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "LoadBalanceDstL4Handler subscriber called"
        tbl_lag_global = tbl_pb2.tbl_lag_global()
        try:
            tbl_lag_global.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_lag_global fail'
            return
        #print tbl_lag_global.load_balance

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            if (tbl_lag_global.load_balance & GLB_LAG_LOAD_BALANCE_DST_PORT) == 0:
                LOADBALANCE_DSTL4.update(GLB_STATE_DISABLE)
            else:
                LOADBALANCE_DSTL4.update(GLB_STATE_ENABLE)


################################################################################
#
# linkAggLoadBalSrcMac 1.3.6.1.4.1.entriprise_oid.10.100.7
#
################################################################################

LOADBALANCE_SRCL4 = None


class LoadBalanceSrcL4Handler(SnmpTableBase):
    def __init__(self, agent):
        super(LoadBalanceSrcL4Handler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        if var_type != ASN_INTEGER:
            return self.set_request_error(SNMP_ERR_WRONGTYPE)

        type = data
        if type != GLB_STATE_ENABLE and type != GLB_STATE_DISABLE:
            return self.set_request_error(SNMP_ERR_BADVALUE)
        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        type = data
        if type == GLB_STATE_ENABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/set/src-port-l4')
        if type == GLB_STATE_DISABLE:
            szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance/unset/src-port-l4')

        if szmsg.find('%') != -1:
            self.set_request_error(SNMP_ERR_GENERR)
        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "LoadBalanceSrcL4Handler subscriber called"
        tbl_lag_global = tbl_pb2.tbl_lag_global()
        try:
            tbl_lag_global.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_lag_global fail'
            return
        #print tbl_lag_global.load_balance

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            if (tbl_lag_global.load_balance & GLB_LAG_LOAD_BALANCE_SRC_PORT) == 0:
                LOADBALANCE_SRCL4.update(GLB_STATE_DISABLE)
            else:
                LOADBALANCE_SRCL4.update(GLB_STATE_ENABLE)


################################################################################
#
#RoRobinCfgTable   1.3.6.1.4.1.entriprise_oid.10.101
#
################################################################################
# Oid indexs
TBL_ROUND_ROBIN_GROUPID        = 1
TBL_ROUND_ROBIN_CFG_STATUS     = 2

TBL_ROUND_ROBIN_GROUPID_MIN    = 1
TBL_ROUND_ROBIN_GROUPID_MAX    = 15
GLB_AGG_MODE_STATIC            = 0
GLB_ROUND_ROBIN_LOAD_BALANCE   = 2



ROROBIN_CFG_TABLE = None
class RoRobinCfgTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(RoRobinCfgTableHandler, self).__init__(agent)

    def check(self, colnum, var_type, data):
        print "check ExTable colnum {0}".format(colnum)

        if colnum == TBL_ROUND_ROBIN_GROUPID:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            groupid = data
            if groupid < TBL_ROUND_ROBIN_GROUPID_MIN or \
                groupid > TBL_ROUND_ROBIN_GROUPID_MAX:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        if colnum == TBL_ROUND_ROBIN_CFG_STATUS:
            if var_type != ASN_INTEGER:
                return self.set_request_error(SNMP_ERR_WRONGTYPE)
            cfg_status = data
            if cfg_status != GLB_STATE_DISABLE and \
                cfg_status != GLB_STATE_ENABLE:
                return self.set_request_error(SNMP_ERR_WRONGVALUE)

        return SNMP_ERR_NOERROR

    def set(self, colnum, data):
        exist = None
        print "set ROROBIN_CFG_TABLE colnum {0}".format(colnum)
        indexes = self.get_request_indexes(ROROBIN_CFG_TABLE)
        #print indexes
        grpid = indexes[0]
        #print grpid

        if grpid < TBL_ROUND_ROBIN_GROUPID_MIN or \
           grpid > TBL_ROUND_ROBIN_GROUPID_MAX:
            return self.set_request_error(SNMP_ERR_INCONSISTENTNAME)

        if colnum == TBL_ROUND_ROBIN_CFG_STATUS:
            print "colnum 2 "
            cfg_status = data
            if cfg_status == GLB_STATE_DISABLE:
                szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance_mode/{0}/static'.format(grpid))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)
            if cfg_status == GLB_STATE_ENABLE:
                szmsg = execute_cmd('cdbctl update/cdb/sys/lag_global/load_balance_mode/{0}/round-robin'.format(grpid))
                if szmsg.find('%') != -1:
                    return self.set_request_error(SNMP_ERR_GENERR)

        return SNMP_ERR_NOERROR

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        tbl_lag_global = tbl_pb2.tbl_lag_global()
        try:
            tbl_lag_global.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_lag_global fail'
            return

        rr_cfg = tbl_lag_global.load_balance_mode
        rr_cfg = [ord(x) for x in rr_cfg]

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            x = TBL_ROUND_ROBIN_GROUPID_MIN
            while x <= TBL_ROUND_ROBIN_GROUPID_MAX:
                newRow = ROROBIN_CFG_TABLE.addRow([AGENT.Integer32(x)])
                if rr_cfg[x] == GLB_ROUND_ROBIN_LOAD_BALANCE:
                    newRow.setRowCell(TBL_ROUND_ROBIN_CFG_STATUS, AGENT.Integer32(GLB_STATE_ENABLE))
                else:
                    newRow.setRowCell(TBL_ROUND_ROBIN_CFG_STATUS, AGENT.Integer32(GLB_STATE_DISABLE))
                x += 1


################################################################################
#
# Module Init
#
################################################################################
def init(agent, subscriber):
    global AGENT
    AGENT = agent
    enterpriseID = AGENT.get_enterprise_id()


    # staticLinkAggTable register
    staticlinkagg_table_handler = StaticLinkAggTableHandler(AGENT)
    global STATIC_LINKAGG_TABLE
    STATIC_LINKAGG_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.10.3.1".format(enterpriseID),
        indexes = [
            AGENT.Integer32(0),
            AGENT.Integer32(0),
        ],
        columns = [
            #(TBL_STATIC_LINKAGG_NUMBER,  AGENT.Integer32(0), 0),
            (TBL_STATIC_LINKAGG_ROW_STATUS, AGENT.Integer32(SNMP_ROW_ACTIVE), 1),

        ],
        extendable = True,
        nodehandle = staticlinkagg_table_handler.node_handler
    )
    STATIC_LINKAGG_TABLE.set_allow_creation(True)
    STATIC_LINKAGG_TABLE.set_rowstatus_column(TBL_STATIC_LINKAGG_ROW_STATUS)
    subscriber.subscribeTable(TBL_INTERFACE, DS_LAG, None,
                              None, staticlinkagg_table_handler.subscribe_handler)


    loadbalance_dstmac_handler = LoadBalanceDstMacHandler(AGENT)
    global LOADBALANCE_DSTMAC
    LOADBALANCE_DSTMAC = AGENT.Integer32(
        oidstr = "1.3.6.1.4.1.{0}.10.100.1".format(enterpriseID),
        initval=GLB_STATE_DISABLE,
        nodehandle = loadbalance_dstmac_handler.node_handler
    )

    subscriber.subscribeTable(TBL_LAG_GLOBAL, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_LAG_GLOBAL_FLD_LOAD_BALANCE, loadbalance_dstmac_handler.subscribe_handler)

    loadbalance_srcmac_handler = LoadBalanceSrcMacHandler(AGENT)
    global LOADBALANCE_SRCMAC
    LOADBALANCE_SRCMAC = AGENT.Integer32(
        oidstr = "1.3.6.1.4.1.{0}.10.100.2".format(enterpriseID),
        initval=GLB_STATE_DISABLE,
        nodehandle=loadbalance_srcmac_handler.node_handler
    )

    subscriber.subscribeTable(TBL_LAG_GLOBAL, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_LAG_GLOBAL_FLD_LOAD_BALANCE, loadbalance_srcmac_handler.subscribe_handler)

    loadbalance_dstip_handler = LoadBalanceDstIpHandler(AGENT)
    global LOADBALANCE_DSTIP
    LOADBALANCE_DSTIP = AGENT.Integer32(
        oidstr = "1.3.6.1.4.1.{0}.10.100.3".format(enterpriseID),
        initval=GLB_STATE_ENABLE,
        nodehandle=loadbalance_dstip_handler.node_handler
    )

    subscriber.subscribeTable(TBL_LAG_GLOBAL, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_LAG_GLOBAL_FLD_LOAD_BALANCE, loadbalance_dstip_handler.subscribe_handler)

    loadbalance_srcip_handler = LoadBalanceSrcIpHandler(AGENT)
    global LOADBALANCE_SRCIP
    LOADBALANCE_SRCIP = AGENT.Integer32(
        oidstr = "1.3.6.1.4.1.{0}.10.100.4".format(enterpriseID),
        initval=GLB_STATE_ENABLE,
        nodehandle = loadbalance_srcip_handler.node_handler
    )

    subscriber.subscribeTable(TBL_LAG_GLOBAL, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_LAG_GLOBAL_FLD_LOAD_BALANCE, loadbalance_srcip_handler.subscribe_handler)

    loadbalance_ippro_handler = LoadBalanceIpProHandler(AGENT)
    global LOADBALANCE_IPPRO
    LOADBALANCE_IPPRO = AGENT.Integer32(
        oidstr = "1.3.6.1.4.1.{0}.10.100.5".format(enterpriseID),
        initval=GLB_STATE_DISABLE,
        nodehandle = loadbalance_ippro_handler.node_handler
    )

    subscriber.subscribeTable(TBL_LAG_GLOBAL, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_LAG_GLOBAL_FLD_LOAD_BALANCE, loadbalance_ippro_handler.subscribe_handler)

    loadbalance_dstl4_handler = LoadBalanceDstL4Handler(AGENT)
    global LOADBALANCE_DSTL4
    LOADBALANCE_DSTL4 = AGENT.Integer32(
        oidstr = "1.3.6.1.4.1.{0}.10.100.6".format(enterpriseID),
        initval=1,
        nodehandle = loadbalance_dstl4_handler.node_handler
    )

    subscriber.subscribeTable(TBL_LAG_GLOBAL, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_LAG_GLOBAL_FLD_LOAD_BALANCE, loadbalance_dstl4_handler.subscribe_handler)

    loadbalance_srcl4_handler = LoadBalanceSrcL4Handler(AGENT)
    global LOADBALANCE_SRCL4
    LOADBALANCE_SRCL4 = AGENT.Integer32(
        oidstr = "1.3.6.1.4.1.{0}.10.100.7".format(enterpriseID),
        initval=GLB_STATE_ENABLE,
        nodehandle = loadbalance_srcl4_handler.node_handler
    )

    subscriber.subscribeTable(TBL_LAG_GLOBAL, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_LAG_GLOBAL_FLD_LOAD_BALANCE, loadbalance_srcl4_handler.subscribe_handler)

    # roRobinCfgTable register
    roRobin_table_handler = RoRobinCfgTableHandler(AGENT)
    global ROROBIN_CFG_TABLE
    ROROBIN_CFG_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.10.101".format(enterpriseID),
        indexes = [
            AGENT.Integer32(0),
        ],
        columns = [
            (TBL_ROUND_ROBIN_CFG_STATUS, AGENT.Integer32(GLB_STATE_DISABLE), 1),

        ],
        extendable = True,
        nodehandle = roRobin_table_handler.node_handler
    )
    subscriber.subscribeTable(TBL_LAG_GLOBAL, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_MODE, roRobin_table_handler.subscribe_handler)