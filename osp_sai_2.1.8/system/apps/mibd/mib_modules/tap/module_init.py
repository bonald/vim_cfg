#!/usr/bin/python

#import sys, os, signal, time, re, optparse, pprint
#import binascii, xdrlib, threading, socket
#from daemon3x import daemon as Daemon
#from struct import *
#from netsnmpapi import *
#from netsnmpagent import *
#from cdb_const import*
#from cdb_define import*
#import tbl_pb2, common_pb2, ds_pb2

import stats, mib2_system, mib2_interfaces, mib2_ifMIB, system
import operate, interface, linkagg, management, tapflow, tapgroup


def start(agent, subscriber):
    mib2_system.init(agent, subscriber)
    mib2_interfaces.init(agent, subscriber)
    mib2_ifMIB.init(agent, subscriber)
    operate.init(agent, subscriber)
    interface.init(agent, subscriber)
    linkagg.init(agent, subscriber)
    management.init(agent, subscriber)
    tapflow.init(agent, subscriber)
    tapgroup.init(agent, subscriber)
    stats.init(agent, subscriber)
    system.init(agent, subscriber)

