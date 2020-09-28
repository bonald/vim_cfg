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
import module_init

################################################################################
#
# Macro Definition
# Please make sure the macro definitions are the same with those in source code
#
################################################################################
MSG_OPER_SUB_REQ = 8

################################################################################
#
# Variable Definition
#
################################################################################
loop = True

prgname = sys.argv[0]

# Terminal width for pprint
columns = 80

RespHdr = common_pb2.subscribe_response()

################################################################################
#
# Python processing start
#
################################################################################
# Make sure we use the local copy, not a system-wide one
sys.path.insert(0, os.path.dirname(os.getcwd()))

# Process command line arguments
usage = "usage: %prog [options] [stop]"  
parser = optparse.OptionParser(usage=usage)
parser.add_option(
    "-m",
    "--mastersocket",
    dest="mastersocket",
    help="Sets the transport specification for the master agent's AgentX socket",
    default="/var/run/agentx-master"
)
parser.add_option(
    "-i",
    "--pid-file",
    dest="pidfile",
    help="Sets the pid file",
    default="/var/run/snmp_agent_main.pid"
)
parser.add_option(
    "-v",
    "--verbose",
    dest="verbose",
    action="store_true",
    help="Verbose mode",
)
parser.add_option(
    "-d",
    "--daemon",
    dest="bg",
    action="store_true",
    help="Daemon mode",
)
(options, args) = parser.parse_args()

# Process action
myaction=""
if len(args) == 1:
    if args[0] != "stop":
        parser.print_help()
        sys.exit(1)
    myaction="stop"
elif len(args) != 0:
    parser.print_help()
    sys.exit(1)

if myaction == "stop":
    class MyDaemon(Daemon):
        def run(self): pass
    do_daemon = MyDaemon(options.pidfile)
    do_daemon.stop()
    sys.exit(0)

# daemonize
if options.bg:
    class MyDaemon(Daemon):
        def run(self): pass
    do_daemon = MyDaemon(options.pidfile)
    do_daemon.start()
else: pass

# Signal handler function
def TermHandler(signum, frame):
    global loop
    loop = False

# Install a signal handler that terminates our simple agent when
# CTRL-C is pressed or a KILL signal is received
signal.signal(signal.SIGINT, TermHandler)
signal.signal(signal.SIGTERM, TermHandler)

# The simple agent's main loop. We loop endlessly until our signal
# handler above changes the "loop" variable.
print "{0}: Serving SNMP requests, press ^C to terminate...".format(prgname)

# Create an instance of the netsnmpAgent class.
try:
    agent = netsnmpAgent(
        AgentName      = "SystemMibAgent",
        MasterSocket   = options.mastersocket,
    )
except netsnmpAgentException as e:
    print "{0}: {1}".format(prgname, e)
    sys.exit(1)

################################################################################
# Subscribe Table Register
class tableSubscriber(object):
    def __init__(self, tbl_id, ds_id = None, ds_id2 = None, field_id = None, table_handler = None):
        self.tbl_id        = tbl_id
        self.ds_id         = ds_id
        self.ds_id2        = ds_id2
        self.field_id      = field_id
        self.table_handler = table_handler

# Subscribe CDB and process
class cdbSubscriber(object):
    def __init__(self):
        self.cdbPath = '/tmp/sock_ccs_sub_cdb'
        self.delay = 0.01
        self.bufferSize = 32000
        self.responsePktLength = 0       
        self.send_msg_hdr = msg_hdr_t()
        self.rcv_msg_hdr = msg_hdr_t()
        self.sub_req = common_pb2.subscribe_request()
        self.sub_resp = common_pb2.subscribe_response()
        self.sub_req_tbl = common_pb2.subscribe_request_tbl()
        self.send_msg_hdr.version = 0
        self.send_msg_hdr.operation = MSG_OPER_SUB_REQ
        self.send_msg_hdr.lengh = 0
        self.send_msg_hdr.seq = 1
        self.sub_list = list()
        self.sub_tbl_list = list()
        return

    def connect(self):
        self.cdbClientSock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.cdbClientSock.setblocking(1)
        try:
            self.cdbClientSock.connect(self.cdbPath)
        except socket.error, msg:
            print 'Error return when connect CDB Server args = ', msg
            return -1
        
        return 0

    def subscribeTable(self, tbl_id, ds_id = None, ds_id2 = None, field_id = None, table_handler = None):
        tableSubObj = tableSubscriber(tbl_id, ds_id, ds_id2, field_id, table_handler)
        self.sub_list.append(tableSubObj)

        if tbl_id in self.sub_tbl_list:
            return 0
        else:
            self.sub_tbl_list.append(tbl_id)

        self.sub_req_tbl.sync = 1
        self.sub_req_tbl.table.append(tbl_id)
        self.sub_req.type = 1
        self.sub_req.format = 1
        self.sub_req.tbl.CopyFrom(self.sub_req_tbl)
        self.sub_req.pid = os.getpid()
        self.sub_req.src = PM_ID_APPCFG

        #pack all struct data to buffer, prepare MSG
        req = self.sub_req.SerializeToString()
        self.send_msg_hdr.length = len(req) + 8
        hdr = pack('>' + self.send_msg_hdr.fmt,self.send_msg_hdr.version, self.send_msg_hdr.operation, self.send_msg_hdr.length, self.send_msg_hdr.seq)
        self.reqBuf = hdr + req
        
        #send require MSG to CDB Server 
        try:
            self.cdbClientSock.send(self.reqBuf)
        except socket.error, args:
            print 'Error return when send CDB Server args = ', args
            return -1
            
        #add ++ to seq self
        self.send_msg_hdr.seq += 1
        return 0

    def stopSubscriber(self):
        return 0
        
    def read(self):
        #read response data from socket 
        global RespHdr
        global loop
        lines = None
        while True:
            #read data to tmp buffer
            tmp = self.cdbClientSock.recv(self.bufferSize)
            if lines and len(lines) > 0:
                lines = lines + tmp
            else:
                lines = tmp
                
            pktLength = len(lines)
            #if __name__ == "__main__":
                #print 'into function read line'

            if pktLength < struct.calcsize(self.rcv_msg_hdr.fmt):
                #if __name__ == "__main__":
                    #print 'subscribe msg length is less than header length, it is', pktLength
                continue
    
            hdr = lines[:struct.calcsize(self.rcv_msg_hdr.fmt )]
            version, operation, length, seq = unpack('>' + self.rcv_msg_hdr.fmt , hdr)

            if pktLength < length:
                #if __name__ == "__main__":
                    #print 'waiting more packet bytes'
                    #print 'Subscribe msg length {0}, packets length {1}', length, pktLength
                continue

            while loop:
                pktLength = len(lines)
                if pktLength < struct.calcsize(self.rcv_msg_hdr.fmt ):
                    if __name__ == "__main__":
                        print 'subscribe msg length is less than header length, it is', pktLength
                    break

                hdr = lines[:struct.calcsize(self.rcv_msg_hdr.fmt )]
                    
                version, operation, length, seq = unpack('>' + self.rcv_msg_hdr.fmt , hdr)
                #print 'version, operation, length, seq =' , version, operation, length, seq 
    
                if pktLength < length:
                    #if __name__ == "__main__":
                        #print 'waiting more packet bytes'
                        #print 'Subscribe msg length {0}, packets length {1}', length, pktLength
                    break

                resp = lines[8:8 + 22 ]
                data = lines[8+22:length]
    
                #oper, tbl_type, tbl_id, ds_id, ds2_id, fields0, fields1  =unpack('>' + self.msg_data_hdr.fmt, s3)
                #print 'oper, tbl_type, tbl_id, ds_id, ds2_id, fields0, fields1 = ', oper, tbl_type, tbl_id, ds_id, ds2_id, fields0, fields1
                self.sub_resp.ParseFromString(resp)
                RespHdr.ParseFromString(resp)

                tbl_id   = RespHdr.tbl_id
                oper     = RespHdr.action
                ds_id    = RespHdr.ds_id
                ds_id2   = RespHdr.ds2_id
                field_id = RespHdr.field_id

                for tableSubObj in self.sub_list:
                    if tableSubObj.tbl_id != tbl_id:
                        continue
                    if tableSubObj.ds_id and tableSubObj.ds_id != ds_id:
                        continue
                    if tableSubObj.ds_id2 and tableSubObj.ds_id2 != ds_id2:
                        continue
                    if oper == CDB_OPER_SET and tableSubObj.field_id and tableSubObj.field_id != field_id:
                        continue
                    if tableSubObj.table_handler:
                        agent.data_mutex.acquire()
                        try:
                            tableSubObj.table_handler(tbl_id, oper, ds_id, ds_id2, field_id, data)
                        except:
                            pass
                        agent.data_mutex.release()
    
                if pktLength > length:
                    #if __name__ == "__main__":
                        #print 'receive one more msg'
                        #print 'Subscribe msg length {0}, packets length {1}', length, pktLength
                    lines = lines[length:]
                    continue
                else :
                    lines = None
                    break

subscriber = cdbSubscriber()
subscriber.connect()

libnsa.oem_init()
module_init.start(agent, subscriber)

def snmpAgentSubscriberMain():
    subscriber.read()

#subscribe thread
Thread1 = threading.Thread(target = snmpAgentSubscriberMain, name = "snmpAgentSubscriberMain")
Thread1.daemon = True
Thread1.start()


SYSTEM_OBJID		= 2
sysObjId = (c_oid * MAX_OID_LEN)()
sysObjId[0] = 0

# hex value
BOARD_E330_48T     		= "1"
BOARD_E330_48S_E   		= "3"
BOARD_E330_48S     		= "4"
BOARD_B330_48T     		= "1"
BOARD_G24EU        		= "1"
BOARD_GREATBELT_DEMO    = "1"
BOARD_ACTUS_NID         = "1"
BOARD_PTN722            = "1"
BOARD_E350_48T4X2Q             = "1"
BOARD_E350_48T4XG              = "2"
BOARD_E350_PHICOMM_8T12XG      = "3"
BOARD_E350_24T4XG              = "4"
BOARD_E350_8T12XG              = "5"
BOARD_E350_8T4S12XG            = "6"
BOARD_E350_8TS12X              = "7"
BOARD_E350_24X                 = "8"
BOARD_E350_MT_8T12XG           = "9"
BOARD_E350_MTRJ_8T12XG         = "a"
BOARD_E350_PEGATRON            = "10"
BOARD_E580_24Q                 = "1"
BOARD_E580_48X2Q4Z             = "2"
BOARD_E580_48X6Q               = "3"
BOARD_E580_32X2Q               = "4"
BOARD_E580_32X                 = "5"
BOARD_E580_48XT6Q              = "6"
BOARD_P580_48X6Q               = "1"
BOARD_P580_HW_48X2Q4Z          = "2"
BOARD_MAGOTAN_48X6Q            = "3"

# hex value
SERIES_E300				= "0"
SERIES_E350				= "1"
SERIES_E800				= "2"
SERIES_EX2000			= "3"
SERIES_EX3000			= "4"
SERIES_E310				= "5"
SERIES_E810				= "6"
SERIES_CPCI				= "7"
SERIES_SEOUL			= "8"
SERIES_E330				= "9"
SERIES_B330	 			= "b"
SERIES_HUMBER_DEMO 		= "f"
SERIES_GREATBELT_DEMO   = "c"
SERIES_E580             = "10"
SERIES_P580             = "11"
SERIES_ACTUS            = "b1"
SERIES_PTN722           = "87"

def mib_get_ObjID():
# format: 1.3.6.1.4.1.27975.99.xxxx
    if 0 != sysObjId[0]:  # already initialized
        return 9 * ctypes.sizeof(c_oid)

    sysObjId[0] = 1
    sysObjId[1] = 3
    sysObjId[2] = 6
    sysObjId[3] = 1
    sysObjId[4] = 4
    sysObjId[5] = 1
    sysObjId[6] = agent.get_enterprise_id()
    sysObjId[7] = 99

    mhndl = os.popen('cat /tmp/ctcos_board_info | grep -E "product_series|board_type"', "r")
    msg = mhndl.readline().strip("\r\n")
    if "" == msg:
        sysObjId[8] = 999999
        return 9 * ctypes.sizeof(c_oid)
    sno = btype = ""
    if msg.find("product_series") != -1:
        sno = msg[len("product_series"):].strip(" ")
        msg = mhndl.readline().strip("\r\n")
        btype = msg[len("board_type"):].strip(" ")
    elif msg.find("board_type") != -1:
        btype = msg[len("board_type"):].strip(" ")
        msg = mhndl.readline().strip("\r\n")
        sno = msg[len("product_series"):].strip(" ")

    sysObjId[8] = 0  # unknown device
    if SERIES_GREATBELT_DEMO == sno:
        if BOARD_GREATBELT_DEMO == btype:
            sysObjId[8] = 3485
        else:
            pass
    elif SERIES_B330 == sno:
        if BOARD_B330_48T == btype:
            sysObjId[8] = 3481
        else:
            pass
    elif SERIES_SEOUL == sno:
        if BOARD_G24EU == btype:
            sysObjId[8] = 3488
        else:
            pass
    elif SERIES_E350 == sno:
        if BOARD_E350_48T4XG == btype:
            sysObjId[8] = 3520
        elif BOARD_E350_PHICOMM_8T12XG == btype:
            sysObjId[8] = 3200
        else:
            pass
    elif SERIES_E580 == sno:
        if BOARD_E580_24Q == btype:
            sysObjId[8] = 5801
        elif BOARD_E580_48X2Q4Z == btype:
            sysObjId[8] = 5802
        elif BOARD_E580_48X6Q == btype:
            sysObjId[8] = 5803
        elif BOARD_E580_32X2Q == btype:
            sysObjId[8] = 5804
        else:
            pass
    else:
        pass
    return 9 * ctypes.sizeof(c_oid)

def mib2sys_var_handler(vp, name, namelength, exact, var_len, write_method):
    if agent.header_generic(vp, name, namelength, \
			exact, var_len, write_method) != MATCH_SUCCEEDED:
        return None

    if SYSTEM_OBJID == vp.contents.magic:
        var_len.contents.value = mib_get_ObjID()
        return ctypes.addressof(sysObjId)
    return None

mib2sys_handler = Netsnmp_Var_Handler(mib2sys_var_handler)

mib2sys_vars = [(SYSTEM_OBJID, ASN_OBJECT_ID, RONLY, mib2sys_handler, "2"),]

mib2_system_oid_str = "1.3.6.1.2.1.1"
agent.register_mib("mib2/system", mib2_system_oid_str, mib2sys_vars)

# Agent main processing loop
def snmpAgentMain():
    try:
        agent.start()
    except netsnmpAgentException as e:
        if options.verbose:
            print "{0}: {1}".format(prgname, e)
        sys.exit(1)

    if options.verbose:
        print "{0}: AgentX connection to snmpd established.".format(prgname)

    if options.verbose:
        # Helper function that dumps the state of all registered SNMP variables
        print "Enterprise ID: " + str(agent.get_enterprise_id())
        for context in agent.getContexts():
            print "{0}: Registered SNMP objects in Context \"{1}\": ".format(prgname, context)
            vars = agent.getRegistered(context)
            pprint.pprint(vars, width=columns)
            print
    
    agent.set_ping_interval(5)
    
    global loop
    while (loop):
        # Block and process SNMP requests, if available
        agent.check_and_process()

    if options.verbose:
        print "{0}: Terminating.".format(prgname)

#main thread
snmpAgentMain()
