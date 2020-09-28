
#!/usr/bin/env python
# encoding: utf-8
'''
Copyright 2014 Centec Networks Inc. All rights reserved.

@summary:     define CDB Table struct
@organization: Centec Networks
@copyright:    2014 Centec Networks Inc. All rights reserved.
'''

'''
this table describe the format for the struct

Format C Type Python type Standard size Notes 


Format C Type               Python type         Standard size     Notes 
x      pad                  byte no value     
c      char                 string of length 1  1   
b      signed char          integer             1                (3) 
B      unsigned char        integer             1                (3) 
?      _Bool                bool                1                (1) 
h      short                integer             2                (3) 
H      unsigned short       integer             2                (3) 
i      int                  integer             4                (3) 
I      unsigned int         integer             4                (3) 
l      long                 integer             4                (3) 
L      unsigned long        integer             4                (3) 
q      long long            integer             8                (2), (3) 
Q      unsigned long long   integer             8                (2), (3) 
f      float                float               4                (4) 
d      double               float               8                (4) 
s      char[]               string     
p      char[]               string     
P      void *               integer                              (5), (3) 





Notes:

(1) The '?' conversion code corresponds to the _Bool type defined by C99. If this type is not available, it is simulated using a char. In standard mode, it is always represented by one byte.

New in version 2.6.

(2) The 'q' and 'Q' conversion codes are available in native mode only if the platform C compiler supports C long long, or, on Windows, __int64. They are always available in standard modes.

New in version 2.2.

(3) When attempting to pack a non-integer using any of the integer conversion codes, if the non-integer has a __index__() method then that method is called to convert the argument to an integer before packing. If no __index__() method exists, or the call to __index__() raises TypeError, then the __int__() method is tried. However, the use of __int__() is deprecated, and will raise DeprecationWarning.

Changed in version 2.7: Use of the __index__() method for non-integers is new in 2.7.

Changed in version 2.7: Prior to version 2.7, not all integer conversion codes would use the __int__() method to convert, and DeprecationWarning was raised only for float arguments.

(4) For the 'f' and 'd' conversion codes, the packed representation uses the IEEE 754 binary32 (for 'f') or binary64 (for 'd') format, regardless of the floating-point format used by the platform.

(5) The 'P' format character is only available for the native byte ordering (selected as the default or with the '@' byte order character). The byte order character '=' chooses to use little- or big-endian ordering based on the host system. The struct module does not interpret this as native ordering, so the 'P' format is not available.


'''

import sys, os, signal, time, re, optparse, pprint
import binascii, xdrlib, threading, socket
from daemon3x import daemon as Daemon
from struct import *
from netsnmpapi import *
from netsnmpagent import *
from cdb_const import*
from cdb_define import*
import tbl_pb2, common_pb2, ds_pb2
import commands

# Macro definitions
CDB_OPER_ADD      = 0
CDB_OPER_SET      = 1
CDB_OPER_GET      = 2
CDB_OPER_DEL      = 3
CDB_OPER_SHOW     = 4
CDB_OPER_VIEW     = 5
CDB_OPER_BUILDCFG = 6
CDB_OPER_SYNC_REFER = 7

CDB_INVALID_DS_ID = 4294967295
GLB_COUNTER32_MAX = 4294967295

GLB_STATE_ENABLE = 1
GLB_STATE_DISABLE = 2
GLB_UNIDIR_RX_ONLY = 3
GLB_UNIDIR_TX_ONLY = 4

GLB_IFF_UP = 1
GLB_IFF_RUNNING = 0x40

GLB_IF_TYPE_NULL = 0
GLB_IF_TYPE_PORT_IF = 1
GLB_IF_TYPE_LINKAGG_IF = 2
GLB_IF_TYPE_VLAN_IF = 3
GLB_IF_TYPE_SUB_IF = 4
GLB_IF_TYPE_LOOPBACK_IF = 5
GLB_IF_TYPE_TUNNEL_IF = 6
GLB_IF_TYPE_MAX = 7

GLB_IF_TYPE_ETHERNETCSMACD = 6
GLB_IF_TYPE_SOFTWARELOOPBACK = 24
GLB_IF_TYPE_TUNNEL = 131
GLB_IF_TYPE_VLANIF = 136
GLB_IF_TYPE_AGG = 161

PM_ID_INVALID       = 0
PM_ID_CTL           = 1
PM_ID_CCS           = 2
PM_ID_CDS           = 3
PM_ID_CHSM          = 4
PM_ID_SWITCH        = 5
PM_ID_ROUTED        = 6
PM_ID_OPM           = 7
PM_ID_AUTHD         = 8
PM_ID_APPCFG        = 9
PM_ID_FEA           = 10
PM_ID_OPENFLOW      = 11
PM_ID_DHCRELAY      = 12
PM_ID_DHCSNOOPING   = 13
PM_ID_DHCLIENT      = 14
PM_ID_PTP           = 15
PM_ID_BHM           = 16
PM_ID_EVENTMGR      = 17
PM_ID_SSM           = 18
PM_ID_SUBSCRIBER    = 19
PM_ID_NCS           = 20
PM_ID_DHCPD6        = 21
PM_ID_DHCLIENT6     = 22
PM_ID_MAX           = 23

GLB_SPEED_AUTO = 0
GLB_SPEED_10M  = 1
GLB_SPEED_100M = 2
GLB_SPEED_1G   = 3
GLB_SPEED_10G  = 4
GLB_SPEED_40G  = 5
GLB_SPEED_100G = 6
GLB_SPEED_5G   = 7
GLB_SPEED_2_5G = 8

GLB_SUPPORT_SPEED_100M    = 0x1
GLB_SUPPORT_SPEED_1G      = 0x2
GLB_SUPPORT_SPEED_10G     = 0x4
GLB_SUPPORT_SPEED_40G     = 0x8
GLB_SUPPORT_SPEED_100G    = 0x10

GLB_ACL_PERMIT = 1
GLB_ACL_DENY   = 2

GLB_ACL_L4_MATCH_TYPE_EQUAL        = 1
GLB_ACL_L4_MATCH_TYPE_LESS_THAN    = 2
GLB_ACL_L4_MATCH_TYPE_GREATER_THAN = 3
GLB_ACL_L4_MATCH_TYPE_RANGE        = 4
GLB_ACL_L4_MATCH_TYPE_ANY          = 5
GLB_ACL_L4_MATCH_TYPE_INVALID      = 6

GLB_ACL_MATCH_TYPE_ANY     = 0
GLB_ACL_MATCH_TYPE_ALL     = 1
GLB_ACL_MATCH_TYPE_INVALID = 2
GLB_ACL_VLAN_ID_INVALID    = 4096

GLB_SNMP_TRUE  = 1
GLB_SNMP_FALSE = 2

GLB_ACL_INVALID_IP_PRECEDENCE = 8
GLB_ACL_INVALID_IP_DSCP = 64
GLB_ACL_INVALID_COS = 8

GLB_ACL_IP_PROTOCOL_TCP = 6
GLB_ACL_IP_PROTOCOL_UDP = 17

GLB_ACL_TCP_FLAG_FIN = 1 << 0
GLB_ACL_TCP_FLAG_SYN = 1 << 1
GLB_ACL_TCP_FLAG_RST = 1 << 2
GLB_ACL_TCP_FLAG_PSH = 1 << 3
GLB_ACL_TCP_FLAG_ACK = 1 << 4
GLB_ACL_TCP_FLAG_URG = 1 << 5

GLB_SNMP_MAC_ADDR_LEN = 6

# Global variables
GLB_IF_INDEX_DICT = dict()
GLB_IF_NAME_DICT = dict()
GLB_IF_STATS_DICT = dict()
GLB_FLOW_STATS_DICT = dict()
GLB_CPU_UTILIZ_DICT = dict()

GLB_INTERFACE_MUTEX = threading.Lock()
GLB_FLOW_STATS_MUTEX = threading.Lock()


# Table handlers
GLB_TABLE_DICT = dict()

# Table mem indexed
################################################################################
#
# ifTable 1.3.6.1.2.1.2.2
#
################################################################################
# Oid indexes
IF_TBL_MEM_INDEX             = 1
IF_TBL_MEM_DESCR             = 2
IF_TBL_MEM_TYPE              = 3
IF_TBL_MEM_MTU               = 4
IF_TBL_MEM_SPEED             = 5
IF_TBL_MEM_PHYS_ADDRESS      = 6
IF_TBL_MEM_ADMIN_STATUS      = 7
IF_TBL_MEM_OPER_STATUS       = 8
#IF_TBL_MEM_LAST_CHANGE       = 9
IF_TBL_MEM_IN_OCTECTS        = 10
IF_TBL_MEM_IN_UCASTPKTS      = 11
IF_TBL_MEM_IN_NUCASTPKTS     = 12
IF_TBL_MEM_IN_DISCARDS       = 13
IF_TBL_MEM_IN_ERRORS         = 14
#IF_TBL_MEM_IN_UNKNOWN_PROTOS = 15
IF_TBL_MEM_OUT_OCTECTS       = 16
IF_TBL_MEM_OUT_UCAST_PKTS    = 17
IF_TBL_MEM_OUT_NUCAST_PKTS   = 18
IF_TBL_MEM_OUT_DISCARDS      = 19
IF_TBL_MEM_OUT_ERRORS        = 20
#IF_TBL_MEM_OUT_QLEN          = 21
#IF_TBL_MEM_SPECIFIC          = 22

################################################################################
#
# ifTable 1.3.6.1.2.1.2.2
#
################################################################################
# Oid indexes
IFX_TBL_MEM_NAME                      = 1
IFX_TBL_MEM_IN_MULTICAST_PKTS         = 2
IFX_TBL_MEM_IN_BROADCAST_PKTS         = 3
IFX_TBL_MEM_OUT_MULTICAST_PKTS        = 4
IFX_TBL_MEM_OUT_BROADCAST_PKTS        = 5
IFX_TBL_MEM_HC_IN_OCTETS              = 6
IFX_TBL_MEM_HC_IN_UCAST_PKTS          = 7
IFX_TBL_MEM_HC_IN_MULTICAST_PKTS      = 8
IFX_TBL_MEM_HC_IN_BROADCAST_PKTS      = 9
IFX_TBL_MEM_HC_OUT_OCTETS             = 10
IFX_TBL_MEM_HC_OUT_UCAST_PKTS         = 11
IFX_TBL_MEM_HC_OUT_MULTICAST_PKTS     = 12
IFX_TBL_MEM_HC_OUT_BROADCAST_PKTS     = 13
#IFX_TBL_MEM_LINK_UP_DOWN_TRAP_ENABLE  = 14
IFX_TBL_MEM_HIGH_SPEED                = 15
#IFX_TBL_MEM_PROMISCUOUS_MODE          = 16
#IFX_TBL_MEM_CONNECTOR_PRESENT         = 17
IFX_TBL_MEM_ALIAS                     = 18
#IFX_TBL_MEM_COUNTER_DISCONTINUTY_TIME = 19

################################################################################
#
#fiberTable
#
################################################################################
# Oid indexs
TBL_FIBER_INDEX                      = 1
TBL_FIBER_TX_PWR2                    = 2
TBL_FIBER_RX_PWR2                    = 3
TBL_FIBER_TX_HIGH_ALARM              = 4
TBL_FIBER_RX_HIGH_ALARM              = 5
TBL_FIBER_TX_HIGH_WARN               = 6
TBL_FIBER_RX_HIGH_WARN               = 7
TBL_FIBER_TX_LOW_ALARM               = 8
TBL_FIBER_RX_LOW_ALARM               = 9
TBL_FIBER_TX_LOW_WARN                = 10
TBL_FIBER_RX_LOW_WARN                = 11

################################################################################
#
#fanTable
#
################################################################################
# Oid indexs
TBL_FAN_INDEX                      = 1
TBL_FAN_MODULE                     = 3
TBL_FAN_STATUS                     = 4
TBL_FAN_SPEED                      = 5

################################################################################
#
#temprTable
#
################################################################################
# Oid indexs
TBL_TEMPR_INDEX                      = 1
TBL_TEMPR_TEMPERATURE                = 2
TBL_TEMPR_LOW                        = 3
TBL_TEMPR_HIGH                       = 4
TBL_TEMPR_CRITICAL                   = 5
TBL_TEMPR_LOCATION                   = 6

TBL_POWER_INDEX                      = 1
TBL_POWER_STATUS                     = 2
TBL_POWER_WORK_STATUS                = 3
TBL_POWER_TYPE                       = 4
TBL_POWER_ALERT_STATUS               = 7

TBL_POWER_PRESENT_VAL                = 1
TBL_POWER_ABSENT_VAL                 = 2

TBL_TMP_POSISION_CPU                 = 1
TBL_TMP_POSISION_BEFOR_CHIP          = 2
TBL_TMP_POSISION_BEHIND_CHIP         = 3
TBL_TMP_POSISION_FAN                 = 4
TBL_TMP_POSISION_PSU                 = 5
TBL_TMP_POSISION_CHIP0               = 6
TBL_TMP_POSISION_CHIP1               = 7
TBL_TMP_POSISION_AROUND_CHIP         = 8

################################################################################
#
#interfaceTable
#
################################################################################
# Oid indexs
TBL_INTERFACE_INDEX                      = 1
TBL_INTERFACE_NAME                       = 2
TBL_INTERFACE_TYPE                       = 3
TBL_INTERFACE_SPEED                      = 7
TBL_INTERFACE_DUPLEX                     = 8
TBL_INTERFACE_RESET_CNT                  = 9
TBL_INTERFACE_BANDWIDTH                  = 15
TBL_INTERFACE_UNIDIRECTIONAL             = 31
TBL_INTERFACE_ENGRESS_FLOWNAME           = 32
TBL_INTERFACE_LINK_STATUS                = 33
TBL_INTERFACE_RESET_FLOWCNT              = 34
TBL_INTERFACE_RESET_EGS_FLOWCNT          = 35
TBL_INTERFACE_5MIN_IN_BITS_RATE          = 41
TBL_INTERFACE_5MIN_IN_PKTS_RATE          = 42
TBL_INTERFACE_5MIN_OUT_BITS_RATE         = 43
TBL_INTERFACE_5MIN_OUT_PKTS_RATE         = 44

################################################################################
#
# flowInterfaceStatsTable enterprise.111.3
#
################################################################################
# Oid indexes
FLOW_IF_STATS_TBL_MEM_IFINDEX     = 1
FLOW_IF_STATS_TBL_MEM_FLOWNAME    = 2
FLOW_IF_STATS_TBL_MEM_SEQNO       = 3
FLOW_IF_STATS_TBL_MEM_MATCH_PKTS  = 4
FLOW_IF_STATS_TBL_MEM_MATCH_BYTES = 5

# Oid indexes
FLOW_IF_EGRESS_STATS_TBL_MEM_IFINDEX     = 1
FLOW_IF_EGRESS_STATS_TBL_MEM_SEQNO       = 2
FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_PKTS  = 3
FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_BYTES = 4


"""
typedef struct
{
    uint8       version;    /* version */
    uint8       operation;  /* msg_oper_t */
    uint16      length;
    uint32      seq;        /* sequence number */
} msg_hdr_t;
    
"""
class msg_hdr_t(object):
    """
    Message Header
    """
    version = 0
    operation = 0
    length = 0
    seq = 0
    fmt = 'BBHI'

def cmd(cmd):
    '''
    send a command and return a array for each line
    '''
    return [x.strip() for x in os.popen(cmd).readlines()]

def _line_to_obj_without_nesting(line):
    obj = {}
    arr = []

    # line to obj
    if line[0] == '/':
        line = line[1:]
        
    # split items and values
    res = line.split('=')
    if(len(res) == 2):
        obj = {res[0]: res[1]}
    elif(len(res) > 2):
        arr.append(res[0])
        for i in res[1:-1]:
            cnt = i.count('/')
            if cnt == 1:
                for item in i.split('/'):
                    arr.append(item)
            elif cnt > 1:
                index = i.rfind('/')
                arr.append(i[0:index])
                arr.append(i[index + 1:])
        arr.append(res[-1])
        
    # make obj
    for i in range(len(arr) / 2):
        key = arr[i * 2]
        value = arr[i * 2 + 1]
        obj[key] = value
    
    return obj
    

def _line_to_obj(line):
    obj = {}
    sub_dict = []   # [{key: string, value: string, str: string}, ....], only current layer
                    # e.g. '/key1=val1/sub={/key2=val2}' -> [{key: 'sub', value: '/key2=val2', str: '/sub={/key2=val2}'}]
    sub_dict_index_stack = []
    single_layer_line = line
    
    # get sub_dict
    for i in range(len(line)):
        if line[i] == '{':
            sub_dict_index_stack.append(i)
        elif line[i] == '}':
            start = sub_dict_index_stack.pop()
        else:
            continue
        
        # find sub dict
        if len(sub_dict_index_stack) == 0:
            # get value
            value = line[start+1 : i]
            # get key
            key_index = line[0 : start-1].rindex('/') + 1
            key = line[key_index : start-1]
            # get str
            string = line[key_index-1 : i+1]
            # append
            sub_dict.append({'key': key, 'value': value, 'str': string})
        
    
    # get single layer line
    for i in sub_dict:
        single_layer_line = single_layer_line.replace(i['str'], '')
        # get sub layer dict
        tmp_key = i['key']
        tmp_obj = _line_to_obj(i['value'])
        obj[tmp_key] = tmp_obj
        
        
    if len(single_layer_line) > 0:
        # merge obj
        obj = dict(_line_to_obj_without_nesting(single_layer_line), **obj)
    
    return obj

def _line_to_obj_simple(line):
    obj = {}
    
    for i in line.split('/'):
        if i:
            tmp = i.split('=')
            if len(tmp) == 2:
                obj[tmp[0]] = tmp[1]
    
    return obj 


def _get_arr(lines):
    arr = []
    for line in lines:
        tmp = _line_to_obj(line)
        if tmp:
            arr.append(tmp)
    return arr

def _get_arr_simple(lines):
    arr = []
    for line in lines:
        tmp = _line_to_obj_simple(line)
        if tmp:
            arr.append(tmp)
    return arr


def get_lines(cmd1):
    """get cmd response
    """
    res = cmd(cmd1)
    return res

def get_arr(cmd1):
    """send cmd and convert response to dict array
    """
    res = cmd(cmd1)
    arr = _get_arr(res)
    return arr

def get_showarr(cmd1):
    """send cmd and convert response to dict array
    """
    res = get_lines(cmd1)
    return res

def get_arr_simple(cmd1):
    """send cmd and convert response to dict array, parse single layer
    """
    res = cmd(cmd1)
    arr = _get_arr_simple(res)
    return arr

def execute_cmd(cmd1):
    """execute bash command and return the string
    """
    #print "exec cmd", cmd1
    return os.popen(cmd1).read().strip("\r\n")

def check_ip_address(ip):
    p = re.compile('^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$')
    if p.match(ip):
        return True
    else:
        return False
        

GLB_TAP_GROUP_DICT = dict()
GLB_TAP_GRP_IGS_FLOW_DICT = dict()
###############################################################################
# To resolve data sync issue
###############################################################################
GLB_DATA_SYNC_ADD = dict()
GLB_DATA_SYNC_DEL = dict()

def increase_data_sync_add(tbl_name, index):
    if tbl_name not in GLB_DATA_SYNC_ADD:
        GLB_DATA_SYNC_ADD[tbl_name] = dict()
    tmp_dict = GLB_DATA_SYNC_ADD[tbl_name]
    if index in tmp_dict:
        tmp_dict[index] += 1
    else:
        tmp_dict[index] = 1
    #print tbl_name, "increase syncd add", tmp_dict[index]

def increase_data_sync_del(tbl_name, index):
    if tbl_name not in GLB_DATA_SYNC_DEL:
        GLB_DATA_SYNC_DEL[tbl_name] = dict()
    tmp_dict = GLB_DATA_SYNC_DEL[tbl_name]
    if index in tmp_dict:
        tmp_dict[index] += 1
    else:
        tmp_dict[index] = 1
    print tbl_name, "increase syncd del", tmp_dict[index]

def check_data_sync_add(tbl_name, index):
    if tbl_name not in GLB_DATA_SYNC_ADD:
        GLB_DATA_SYNC_ADD[tbl_name] = dict()
    tmp_dict = GLB_DATA_SYNC_ADD[tbl_name]
    if index in tmp_dict:
        if tmp_dict[index] >= 1:
            print tbl_name, "skip syncd add", tmp_dict[index]
            return True
    return False

def check_data_sync_del(tbl_name, index):
    if tbl_name not in GLB_DATA_SYNC_DEL:
        GLB_DATA_SYNC_DEL[tbl_name] = dict()
    tmp_dict = GLB_DATA_SYNC_DEL[tbl_name]
    if index in tmp_dict:
        if tmp_dict[index] >= 1:
            #print tbl_name, "skip syncd del", tmp_dict[index]
            return True
    return False

def decrease_data_sync_add(tbl_name, index):
    if tbl_name not in GLB_DATA_SYNC_ADD:
        GLB_DATA_SYNC_ADD[tbl_name] = dict()
    tmp_dict = GLB_DATA_SYNC_ADD[tbl_name]
    if index in tmp_dict:
        #print tbl_name, "decrease syncd add", tmp_dict[index]
        tmp_dict[index] -= 1
        if tmp_dict[index] <= 0:
            del(tmp_dict[index])

def decrease_data_sync_del(tbl_name, index):
    if tbl_name not in GLB_DATA_SYNC_DEL:
        GLB_DATA_SYNC_DEL[tbl_name] = dict()
    tmp_dict = GLB_DATA_SYNC_DEL[tbl_name]
    if index in tmp_dict:
        #print tbl_name, "decrease syncd del", tmp_dict[index]
        tmp_dict[index] -= 1
        if tmp_dict[index] <= 0:
            del(tmp_dict[index])
