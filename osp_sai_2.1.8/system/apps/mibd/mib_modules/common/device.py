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

################################################################################
#
#fiberTable
#
################################################################################
FIBER_TABLE = None
class FiberTableHandler(SnmpTableBase):

    def __init__(self, agent):
        super(FiberTableHandler, self).__init__(agent)


    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "FiberTableHandler subscriber called"
        tbl_fiber_info = tbl_pb2.tbl_fiber()
        try:
            tbl_fiber_info.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_fiber_info fail'
            return

        #print tbl_fiber_info

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            if tbl_fiber_info.ddm_support:
                new_row = FIBER_TABLE.addRow([AGENT.Integer32(tbl_fiber_info.key.port_id+1)])
                if 0 == tbl_fiber_info.channel_idx:
                    tx_pwer = str(int(tbl_fiber_info.tx_pwr2[0] * 100))
                    rx_pwer = str(int(tbl_fiber_info.rx_pwr2[0] * 100))
                elif 1 == tbl_fiber_info.channel_idx:
                    tx_pwer = str(int(tbl_fiber_info.tx_pwr2[1] * 100))
                    rx_pwer = str(int(tbl_fiber_info.rx_pwr2[1] * 100))
                elif 2 == tbl_fiber_info.channel_idx:
                    tx_pwer = str(int(tbl_fiber_info.tx_pwr2[2] * 100))
                    rx_pwer = str(int(tbl_fiber_info.rx_pwr2[2] * 100))
                elif 3 == tbl_fiber_info.channel_idx:
                    tx_pwer = str(int(tbl_fiber_info.tx_pwr2[3] * 100))
                    rx_pwer = str(int(tbl_fiber_info.rx_pwr2[3] * 100))
                else:
                    tx_pwer = str(int(tbl_fiber_info.tx_pwr2[0] * 100)) + ',' + str(int(tbl_fiber_info.tx_pwr2[1] * 100)) + \
                        ',' + str(int(tbl_fiber_info.tx_pwr2[2] * 100)) + ',' + str(int(tbl_fiber_info.tx_pwr2[3] * 100))
                    rx_pwer = str(int(tbl_fiber_info.rx_pwr2[0] * 100)) + ',' + str(int(tbl_fiber_info.rx_pwr2[1] * 100)) + \
                        ',' + str(int(tbl_fiber_info.rx_pwr2[2] * 100)) + ',' + str(int(tbl_fiber_info.rx_pwr2[3] * 100))
                new_row.setRowCell(TBL_FIBER_TX_PWR2, AGENT.DisplayString(tx_pwer))
                new_row.setRowCell(TBL_FIBER_RX_PWR2, AGENT.DisplayString(rx_pwer))

                new_row.setRowCell(TBL_FIBER_TX_HIGH_ALARM, AGENT.DisplayString(str(int(tbl_fiber_info.tx_pwr[0] * 100))))
                new_row.setRowCell(TBL_FIBER_RX_HIGH_ALARM, AGENT.DisplayString(str(int(tbl_fiber_info.rx_pwr[0] * 100))))
                new_row.setRowCell(TBL_FIBER_TX_LOW_ALARM, AGENT.DisplayString(str(int(tbl_fiber_info.tx_pwr[1] * 100))))
                new_row.setRowCell(TBL_FIBER_RX_LOW_ALARM, AGENT.DisplayString(str(int(tbl_fiber_info.rx_pwr[1] * 100))))
                new_row.setRowCell(TBL_FIBER_TX_HIGH_WARN, AGENT.DisplayString(str(int(tbl_fiber_info.tx_pwr[2] * 100))))
                new_row.setRowCell(TBL_FIBER_RX_HIGH_WARN, AGENT.DisplayString(str(int(tbl_fiber_info.rx_pwr[2] * 100))))
                new_row.setRowCell(TBL_FIBER_TX_LOW_WARN, AGENT.DisplayString(str(int(tbl_fiber_info.tx_pwr[3] * 100))))
                new_row.setRowCell(TBL_FIBER_RX_LOW_WARN, AGENT.DisplayString(str(int(tbl_fiber_info.rx_pwr[3] * 100))))

        if operation == CDB_OPER_DEL:
            FIBER_TABLE.delRow([AGENT.Integer32(tbl_fiber_info.key.port_id+1)])

################################################################################
#
#fanTable
#
################################################################################
FAN_TABLE = None
class FanTableHandler(SnmpTableBase):

    def __init__(self, agent):
        super(FanTableHandler, self).__init__(agent)

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        #print "FanTableHandler subscriber called"

        tbl_fan_info = tbl_pb2.tbl_fan()
        try:
            tbl_fan_info.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_fiber_info fail'
            return

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            index = (tbl_fan_info.key.id & 0xff) + 1
            new_row = FAN_TABLE.addRow([AGENT.Integer32(index)])
            new_row.setRowCell(TBL_FAN_MODULE, AGENT.Integer32(tbl_fan_info.tray+1))
            if tbl_fan_info.status != "OK":
                status = 2
            else:
                status = 1
            new_row.setRowCell(TBL_FAN_STATUS, AGENT.Integer32(status))
            if 2 == status:
                new_row.setRowCell(TBL_FAN_SPEED, AGENT.Integer32(0))
            else:
                new_row.setRowCell(TBL_FAN_SPEED, AGENT.Integer32(tbl_fan_info.percent))

        if operation == CDB_OPER_DEL:
            index = (tbl_fan_info.key.id & 0xff) + 1
            FAN_TABLE.delRow([AGENT.Integer32(index)])


################################################################################
#
# temprTable
#
################################################################################
TEMPR_TABLE = None

class TemprTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(TemprTableHandler, self).__init__(agent)

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        # print "TemprTableHandler subscriber called"

        tbl_sensor = tbl_pb2.tbl_sensor()
        try:
            tbl_sensor.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_sensor fail'
            return

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            new_row = TEMPR_TABLE.addRow([AGENT.Integer32(tbl_sensor.key.id+1)])
            new_row.setRowCell(TBL_TEMPR_TEMPERATURE, AGENT.Integer32(tbl_sensor.temperature))
            new_row.setRowCell(TBL_TEMPR_LOW, AGENT.Integer32(tbl_sensor.tmpr_low))
            new_row.setRowCell(TBL_TEMPR_HIGH, AGENT.Integer32(tbl_sensor.tmpr_high))
            new_row.setRowCell(TBL_TEMPR_CRITICAL, AGENT.Integer32(tbl_sensor.tmpr_critical))
            val = 0
            if tbl_sensor.position == "AROUND_CPU":
                val = TBL_TMP_POSISION_CPU
            elif tbl_sensor.position == "BEFORE_CHIP":
                val = TBL_TMP_POSISION_BEFOR_CHIP
            elif tbl_sensor.position == "BEHIND_CHIP":
                val = TBL_TMP_POSISION_BEHIND_CHIP
            elif tbl_sensor.position == "AROUND_FAN":
                val = TBL_TMP_POSISION_FAN
            elif tbl_sensor.position == "AROUND_PSU":
                val = TBL_TMP_POSISION_PSU
            elif tbl_sensor.position == "SWITCH_CHIP0":
                val = TBL_TMP_POSISION_CHIP0
            elif tbl_sensor.position == "SWITCH_CHIP1":
                val = TBL_TMP_POSISION_CHIP1
            elif tbl_sensor.position == "AROUND_CHIP":
                val = TBL_TMP_POSISION_AROUND_CHIP
            new_row.setRowCell(TBL_TEMPR_LOCATION, AGENT.Integer32(val))

        if operation == CDB_OPER_DEL:
            TEMPR_TABLE.delRow([AGENT.Integer32(tbl_sensor.key.id+1)])


################################################################################
#
# powerTable
#
################################################################################
POWER_TABLE = None

class PowerTableHandler(SnmpTableBase):
    def __init__(self, agent):
        super(PowerTableHandler, self).__init__(agent)

    def subscriber(self, tbl_id, operation, ds_id, ds_id2, field_id, data):
        # print "TemprTableHandler subscriber called"

        tbl_psu = tbl_pb2.tbl_psu()
        try:
            tbl_psu.ParseFromString(data)
        except:
            print 'ERROR!! Parse tbl_psu fail'
            return
        #print tbl_psu

        if operation == CDB_OPER_ADD or operation == CDB_OPER_SET:
            new_row = POWER_TABLE.addRow([AGENT.Integer32(tbl_psu.key.id+1)])

            if tbl_psu.absent == 0:
                new_row.setRowCell(TBL_POWER_STATUS, AGENT.Integer32(TBL_POWER_PRESENT_VAL))
            else :
                new_row.setRowCell(TBL_POWER_STATUS, AGENT.Integer32(TBL_POWER_ABSENT_VAL))

            val = 4
            if tbl_psu.mode == "AC":
                val = 1
            elif tbl_psu.mode == "DC":
                val = 2
            elif tbl_psu.mode == "-":
                val = 3
            new_row.setRowCell(TBL_POWER_TYPE, AGENT.Integer32(val))
            if tbl_psu.alert_status == "ALERT" and tbl_psu.absent == 0 :
                new_row.setRowCell(TBL_POWER_ALERT_STATUS, AGENT.Integer32(2))
            elif tbl_psu.alert_status == "NO":
                new_row.setRowCell(TBL_POWER_ALERT_STATUS, AGENT.Integer32(1))
            else :
                new_row.setRowCell(TBL_POWER_ALERT_STATUS, AGENT.Integer32(3))

            if tbl_psu.absent != 0:
                new_row.setRowCell(TBL_POWER_WORK_STATUS, AGENT.Integer32(3))
            elif tbl_psu.run_status == "FAIL":
                new_row.setRowCell(TBL_POWER_WORK_STATUS, AGENT.Integer32(2))
            elif tbl_psu.run_status == "OK":
                new_row.setRowCell(TBL_POWER_WORK_STATUS, AGENT.Integer32(1))
            else :
                new_row.setRowCell(TBL_POWER_WORK_STATUS, AGENT.Integer32(3))


        if operation == CDB_OPER_DEL:
            POWER_TABLE.delRow([AGENT.Integer32(tbl_psu.key.id+1)])

################################################################################
#
# Module Init
#
################################################################################
def init(agent_obj, subscriber):
    global AGENT
    AGENT = agent_obj
    enterpriseID = AGENT.get_enterprise_id()

    # fiberTable register
    fiber_table_handler = FiberTableHandler(AGENT)
    global FIBER_TABLE
    FIBER_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.1.11".format(enterpriseID),
        indexes = [
            AGENT.Integer32(0),
        ],
        columns = [
            (TBL_FIBER_RX_PWR2, AGENT.DisplayString(""), 0),
            (TBL_FIBER_TX_PWR2, AGENT.DisplayString(""), 0),
            (TBL_FIBER_TX_HIGH_ALARM, AGENT.DisplayString(""), 0),
            (TBL_FIBER_RX_HIGH_ALARM, AGENT.DisplayString(""), 0),
            (TBL_FIBER_TX_HIGH_WARN, AGENT.DisplayString(""), 0),
            (TBL_FIBER_RX_HIGH_WARN, AGENT.DisplayString(""), 0),
            (TBL_FIBER_TX_LOW_ALARM, AGENT.DisplayString(""), 0),
            (TBL_FIBER_RX_LOW_ALARM, AGENT.DisplayString(""), 0),
            (TBL_FIBER_TX_LOW_WARN, AGENT.DisplayString(""), 0),
            (TBL_FIBER_RX_LOW_WARN, AGENT.DisplayString(""), 0),
        ],
        extendable = True,
        nodehandle = fiber_table_handler.node_handler
    )
    subscriber.subscribeTable(TBL_FIBER, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, fiber_table_handler.subscribe_handler)


    # powerTable register
    power_table_handler = PowerTableHandler(AGENT)
    global POWER_TABLE
    POWER_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.37.1.2".format(enterpriseID),
        indexes = [
            AGENT.Integer32(0),
        ],
        columns = [
            (TBL_POWER_STATUS, AGENT.Integer32(0), 0),
            (TBL_POWER_WORK_STATUS, AGENT.Integer32(0), 0),
            (TBL_POWER_TYPE, AGENT.Integer32(0), 0),
            (TBL_POWER_ALERT_STATUS, AGENT.Integer32(0), 0),
        ],
        extendable = True,
        nodehandle = power_table_handler.node_handler
    )
    subscriber.subscribeTable(TBL_PSU, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, power_table_handler.subscribe_handler)



    # fanTable register
    fan_table_handler = FanTableHandler(AGENT)
    global FAN_TABLE
    FAN_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.37.2".format(enterpriseID),
        indexes = [
            AGENT.Integer32(0),
        ],
        columns = [
            (TBL_FAN_MODULE, AGENT.Integer32(0), 0),
            (TBL_FAN_STATUS, AGENT.Integer32(0), 0),
            (TBL_FAN_SPEED, AGENT.Integer32(0), 0),

        ],
        extendable = True,
        nodehandle = fan_table_handler.node_handler
    )
    subscriber.subscribeTable(TBL_FAN, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, fan_table_handler.subscribe_handler)

    # temprTable register
    tempr_table_handler = TemprTableHandler(AGENT)
    global TEMPR_TABLE
    TEMPR_TABLE = AGENT.Table(
        oidstr = "1.3.6.1.4.1.{0}.37.4".format(enterpriseID),
        indexes = [
            AGENT.Integer32(0),
        ],
        columns = [
            (TBL_TEMPR_TEMPERATURE, AGENT.Integer32(0), 0),
            (TBL_TEMPR_LOW, AGENT.Integer32(0), 0),
            (TBL_TEMPR_HIGH, AGENT.Integer32(0), 0),
            (TBL_TEMPR_CRITICAL, AGENT.Integer32(0), 0),
            (TBL_TEMPR_LOCATION, AGENT.Integer32(0), 0),

        ],
        extendable = True,
        nodehandle = tempr_table_handler.node_handler
    )
    subscriber.subscribeTable(TBL_SENSOR, CDB_INVALID_DS_ID, CDB_INVALID_DS_ID,
                              None, tempr_table_handler.subscribe_handler)
