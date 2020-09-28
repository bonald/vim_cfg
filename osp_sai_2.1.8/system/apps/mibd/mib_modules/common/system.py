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
# device
#
################################################################################
################################################################################
# Oid indexs
TBL_SYS_MEMORY_TOTAL   = 5
TBL_SYS_MEMORY_FREE    = 11
TBL_SYS_MEMORY_USED    = 12
TBL_SYS_MEMORY_USAGE   = 13

################################################################################
#
#memoryTable
#
################################################################################
SYS_MEMORY_INDEX = None
class SysMemoryIndexHandler(SnmpTableBase):

    def __init__(self, agent):
        super(SysMemoryIndexHandler, self).__init__(agent)
        szmsg = execute_cmd('cdbctl show/cdb/sys/mem_summary')


    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "SysMemoryIndexHandler subscriber called"

        tbl_mem_info = tbl_pb2.tbl_mem_summary()
        try:
            tbl_mem_info.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_mem_info fail'
            return

        #print tbl_mem_info

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            SYS_MEMORY_INDEX.update(0)

        if operation == CDB_OPER_DEL:
            SYS_MEMORY_INDEX.update(0)


SYS_MEMORY_TOTAL = None
class SysMemoryTotalHandler(SnmpTableBase):

    def __init__(self, agent):
        super(SysMemoryTotalHandler, self).__init__(agent)
        szmsg = execute_cmd('cdbctl show/cdb/sys/mem_summary')


    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "SysMemoryIndexHandler subscriber called"

        tbl_mem_info = tbl_pb2.tbl_mem_summary()
        try:
            tbl_mem_info.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_mem_info fail'
            return

        #print tbl_mem_info

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            SYS_MEMORY_TOTAL.update(tbl_mem_info.total)

SYS_MEMORY_FREE = None
class SysMemoryFreeHandler(SnmpTableBase):

    def __init__(self, agent):
        super(SysMemoryFreeHandler, self).__init__(agent)
        szmsg = execute_cmd('cdbctl show/cdb/sys/mem_summary')


    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "SysMemoryIndexHandler subscriber called"

        tbl_mem_info = tbl_pb2.tbl_mem_summary()
        try:
            tbl_mem_info.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_mem_info fail'
            return

        #print tbl_mem_info

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            SYS_MEMORY_FREE.update(tbl_mem_info.free)


SYS_MEMORY_USED = None
class SysMemoryUsedHandler(SnmpTableBase):

    def __init__(self, agent):
        super(SysMemoryUsedHandler, self).__init__(agent)
        szmsg = execute_cmd('cdbctl show/cdb/sys/mem_summary')


    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "SysMemoryIndexHandler subscriber called"

        tbl_mem_info = tbl_pb2.tbl_mem_summary()
        try:
            tbl_mem_info.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_mem_info fail'
            return

        #print tbl_mem_info

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            SYS_MEMORY_USED.update(tbl_mem_info.used)


SYS_MEMORY_USAGE = None
class SysMemoryUsageHandler(SnmpTableBase):

    def __init__(self, agent):
        super(SysMemoryUsageHandler, self).__init__(agent)
        szmsg = execute_cmd('cdbctl show/cdb/sys/mem_summary')


    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "SysMemoryIndexHandler subscriber called"

        tbl_mem_info = tbl_pb2.tbl_mem_summary()
        try:
            tbl_mem_info.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_mem_info fail'
            return

        #print tbl_mem_info

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            SYS_MEMORY_USAGE.update(int(float(tbl_mem_info.used)/float(tbl_mem_info.total)*100))

        if operation == CDB_OPER_DEL:
            SYS_MEMORY_USAGE.update(0)


################################################################################
#
#cpuusageTable
#
################################################################################

CPU_USG_INDEX = None
class CpuUsgIndexHandler(SnmpTableBase):

    def __init__(self, agent):
        super(CpuUsgIndexHandler, self).__init__(agent)

    def get(self, colnum):
        return self.snmp_return_integer(1)
        #CPU_USG_INDEX.update(1)


CPU_USG_TABLE = None
class CpuUsgTableHandler(SnmpTableBase):

    def __init__(self, agent):
        super(CpuUsgTableHandler, self).__init__(agent)

    def get(self, colnum):
        msg = os.popen('cdbctl show/cdb/app/cpu_utilization')
        lines = msg.readlines()
        for line in lines:
            arr = line.split()
            print arr
            if len(arr) >= 2 and arr[0] == "Total":
                return self.snmp_return_integer(int(float(arr[len(arr)-1])))


################################################################################
#
# Module Init
#
################################################################################
def init(agent_obj, subscriber):
    global AGENT
    AGENT = agent_obj
    enterpriseID = AGENT.get_enterprise_id()

    # sysmemory register
    memory_index_handler = SysMemoryIndexHandler(AGENT)
    global SYS_MEMORY_INDEX
    SYS_MEMORY_INDEX = AGENT.Integer32(
        oidstr = "1.3.6.1.4.1.{0}.1.1.1".format(enterpriseID),
        initval=0,
        nodehandle = memory_index_handler.node_handler
    )
    subscriber.subscribeTable(TBL_MEM_SUMMARY, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, memory_index_handler.subscribe_handler)

    memory_total_handler = SysMemoryTotalHandler(AGENT)
    global SYS_MEMORY_TOTAL
    SYS_MEMORY_TOTAL = AGENT.Integer32(
        oidstr="1.3.6.1.4.1.{0}.1.1.5".format(enterpriseID),
        initval=0,
        nodehandle = memory_total_handler.node_handler
    )
    subscriber.subscribeTable(TBL_MEM_SUMMARY, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_MEM_SUMMARY_FLD_TOTAL, memory_total_handler.subscribe_handler)

    memory_free_handler = SysMemoryFreeHandler(AGENT)
    global SYS_MEMORY_FREE
    SYS_MEMORY_FREE = AGENT.Integer32(
        oidstr="1.3.6.1.4.1.{0}.1.1.11".format(enterpriseID),
        initval=0,
        nodehandle = memory_free_handler.node_handler
    )
    subscriber.subscribeTable(TBL_MEM_SUMMARY, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_MEM_SUMMARY_FLD_FREE, memory_free_handler.subscribe_handler)

    memory_used_handler = SysMemoryUsedHandler(AGENT)
    global SYS_MEMORY_USED
    SYS_MEMORY_USED = AGENT.Integer32(
        oidstr="1.3.6.1.4.1.{0}.1.1.12".format(enterpriseID),
        initval=0,
        nodehandle=memory_used_handler.node_handler
    )
    subscriber.subscribeTable(TBL_MEM_SUMMARY, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              TBL_MEM_SUMMARY_FLD_USED, memory_used_handler.subscribe_handler)

    memory_usage_handler = SysMemoryUsageHandler(AGENT)
    global SYS_MEMORY_USAGE
    SYS_MEMORY_USAGE = AGENT.Integer32(
        oidstr="1.3.6.1.4.1.{0}.1.1.13".format(enterpriseID),
        initval=0,
        nodehandle=memory_usage_handler.node_handler
    )
    subscriber.subscribeTable(TBL_MEM_SUMMARY, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, memory_usage_handler.subscribe_handler)

    # cpuusageTable register
    cpu_index_handler = CpuUsgIndexHandler(AGENT)
    global CPU_USG_INDEX
    CPU_USG_INDEX = AGENT.Integer32(
        oidstr = "1.3.6.1.4.1.{0}.1.2.1".format(enterpriseID),
        initval = 1,
        nodehandle = cpu_index_handler.node_handler
    )

    # cpuusageTable register
    cpu_table_handler = CpuUsgTableHandler(AGENT)
    global CPU_USG_TABLE
    CPU_USG_TABLE = AGENT.Integer32(
        oidstr = "1.3.6.1.4.1.{0}.1.2.12".format(enterpriseID),
        initval = 0,
        nodehandle = cpu_table_handler.node_handler
    )


