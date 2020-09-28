#!/usr/bin/python
#
# cem client
#
# Copyright (c) 2015 Centec Networks (suzhou) Co., Ltd
#


import sys, os, signal, time, re
import optparse
import pprint

# Make sure we use the local copy, not a system-wide one
sys.path.insert(0, os.path.dirname(os.getcwd()))


# Install a signal handler that terminates our simple agent when
# CTRL-C is pressed or a KILL signal is received
def TermHandler(signum, frame):
	global loop
	loop = False
signal.signal(signal.SIGINT, TermHandler)
signal.signal(signal.SIGTERM, TermHandler)



"""
# Subscribe CDB data.
# Example for VLAN/interface

#CDS Message subscribe request format = msg_hdr_t + msg_subscribe_hdr_t

#CDS Message subscribe response format = msg_hdr_t + msg_data_hdr_t + DATA 
"""

import threading
from socket import *
import select
import binascii
import struct
import ctypes
import sys
import xdrlib
import time
import Queue
from py_cdb_client import *
import string
import base64

CDB_OPER_ADD      = 0
CDB_OPER_SET      = 1
CDB_OPER_GET      = 2
CDB_OPER_DEL      = 3
CDB_OPER_SHOW     = 4
CDB_OPER_VIEW     = 5
CDB_OPER_BUILDCFG = 6
PM_ID_EVENTMGR      = 14

global event_manager
event_manager = {}

update_event = []
	
def modify_str(tmp):
	n = len(filter(lambda x: x in string.printable, tmp))
	tmp = tmp[0:n]
	return tmp

class tbl_cem_t(object):
	"""
	Table INTERFACE struct
	"""
	key = ''                 
	event = ''             
	threshold = 0             
	snmptrap = 0 
	loadpath = ''
	server = ''                 
	fro = ''             
	to = ''             
	cc = ''
	subject = ''
	body = ''
	attach = ''

	fmt = '64s'+'32s'+'I'+'I'+'64s'+'1024s'+'1024s'+'1024s'+'1024s'+'64s'+ '512s' +'1024s'
	
class tbl_log_t(object):
    """
    Table TBL_LOG struct
    """
    key                            = 0
    log_id                         = 0
    severity                       = 0
    timestamp                      = 0
    data                           = ''
    fmt = 'I'+'I'+'I'+'I'+'256s'	
	
class mail_t(object):
    """
    mail_t struct
    """
    server = ''                 
    from1 = ''             
    to = ''             
    cc = ''
    subject = ''
    body = ''
    attach = ''

    fmt = '1024s'+'1024s'+'1024s'+'1024s'+'64s'+ '512s' +'1024s'
	
			
def get_event_manager():
	print 'event_manager1',event_manager
	return event_manager

class subscribeCDBTable(object):

    def __init__(self):
        self.cdbPath = '/tmp/sock_ccs_sub_cdb'
        self.cdbSubscribeMsgLen = 24
        self.delay = 0.01
        self.bufferSize = 32000
        self.responsePktLength = 0
        
        #subscribe Require MSG Header = Header + UnitHeader
        #Header
        self.send_msg_hdr = msg_hdr_t()
        self.rcv_msg_hdr = msg_hdr_t()
        self.msg_subscribe_hdr = msg_subscribe_hdr_t()
        self.msg_data_hdr = msg_data_hdr_t()

        self.send_msg_hdr.version = 0
        self.send_msg_hdr.operation = 6
        self.send_msg_hdr.lengh = 0
        self.send_msg_hdr.seq = 1
        
        
        #struct require MSG buffer, '24B' means 24 unsiged char bytes
        self.requireStruct = struct.Struct(self.send_msg_hdr.fmt + self.msg_subscribe_hdr.fmt + 'I')
        self.HeaderLengh = struct.calcsize(self.send_msg_hdr.fmt + self.msg_subscribe_hdr.fmt + 'I') 

        self.send_msg_hdr.lengh = self.HeaderLengh
        
        #get buffer pointer, it is used to be sending param 
        self.requireMsgBuffer = ctypes.create_string_buffer(self.requireStruct.size)
        return 

    def connect(self):
        '''
        Connect CDS Server
        '''
        self.cdbClientSock = socket(AF_UNIX, SOCK_STREAM)
        self.cdbClientSock.setblocking(1)
        try:
            self.cdbClientSock.connect(self.cdbPath)
        except error, args:
            print 'Error return when connect CDB Server args = ', args
            return -1
            
        return 0

    def subscribeTable(self, tableId):
        '''
        Subscribe Table per table 
        '''       
        self.msg_subscribe_hdr.type = 1
        self.msg_subscribe_hdr.node_type = 1
        self.msg_subscribe_hdr.tbl_id = tableId
        self.msg_subscribe_hdr.ds_id = 0
        self.msg_subscribe_hdr.tbl_num = 1
        self.msg_subscribe_hdr.field = 0
        self.msg_subscribe_hdr.pid = os.getpid()
        self.msg_subscribe_hdr.src = PM_ID_EVENTMGR
        #PM_ID_APPCFG


        #pack all struct data to buffer, prepare MSG
        struct.pack_into('>' + self.send_msg_hdr.fmt + self.msg_subscribe_hdr.fmt, \
            self.requireMsgBuffer,0, \
            self.send_msg_hdr.version, \
            self.send_msg_hdr.operation, \
            self.send_msg_hdr.lengh, \
            self.send_msg_hdr.seq, \
            self.msg_subscribe_hdr.type,\
            self.msg_subscribe_hdr.node_type,\
            self.msg_subscribe_hdr.tbl_id,\
            self.msg_subscribe_hdr.ds_id,\
            self.msg_subscribe_hdr.tbl_num,\
            self.msg_subscribe_hdr.field,\
            self.msg_subscribe_hdr.pid,\
            self.msg_subscribe_hdr.src)

        struct.pack_into('I', self.requireMsgBuffer, struct.calcsize(self.send_msg_hdr.fmt + self.msg_subscribe_hdr.fmt), tableId)


        #for unit test 
        if __name__ == "__main__":
            print 'After pack, the MSG is  ', binascii.hexlify(self.requireMsgBuffer)

        #send require MSG to CDB Server 
        try:
            self.cdbClientSock.send(self.requireMsgBuffer)
        except socket.error, args:
            print 'Error return when send CDB Server args = ', args
            return -1
            
        #add ++ to seq self
        self.send_msg_hdr.seq += 1
        
        return 0

    def stopSubscriber(self):
        '''
        Stop stopSubscriber 
        '''
        return 0
        
    def read(self):

		#read response data from socket 
		firstTime = 1
		s = self.cdbClientSock
		inputs = [s]

		while True:
			#read data to tmp buffer			
			tmp = self.cdbClientSock.recv(self.bufferSize)

			if firstTime:
				lines = tmp
			else:
				lines = lines + tmp

			pktLength = len(lines)


			if pktLength < struct.calcsize(self.rcv_msg_hdr.fmt ):
				if __name__ == "__main__":
					print 'subscribe msg length is less than header length, it is', pktLength
				continue

			s2 = lines[:struct.calcsize(self.rcv_msg_hdr.fmt )]
				
			version, operation, length, seq = struct.unpack('>' + self.rcv_msg_hdr.fmt , s2)
			#print 'version, operation, length, seq =' , version, operation, length, seq 

			if pktLength < length:
				if __name__ == "__main__":
					print 'waiting more packet bytes'
					print 'Subscribe msg length {0}, packets length {1}', length, pktLength
				continue

			while True:

				pktLength = len(lines)

				if pktLength < struct.calcsize(self.rcv_msg_hdr.fmt ):
					if __name__ == "__main__":
						print 'subscribe msg length is less than header length, it is', pktLength
					break

				s2 = lines[:struct.calcsize(self.rcv_msg_hdr.fmt )]
					
				version, operation, length, seq = struct.unpack('>' + self.rcv_msg_hdr.fmt , s2)
				#print 'version, operation, length, seq =' , version, operation, length, seq 

				if pktLength < length:
					if __name__ == "__main__":
						print 'waiting more packet bytes'
						print 'Subscribe msg length {0}, packets length {1}', length, pktLength
					break
						
				s3 = lines[struct.calcsize(self.rcv_msg_hdr.fmt ):struct.calcsize(self.rcv_msg_hdr.fmt ) + struct.calcsize(self.msg_data_hdr.fmt ) ]
				s4 = lines[:length]

				oper, tbl_type, tbl_id, ds_id, ds2_id, fields0, fields1  = struct.unpack('>' + self.msg_data_hdr.fmt, s3)
				
				if __name__ == "__main__":
					print 'tbl_id =', tbl_id
					
				if tbl_id == 82:
				    self.CemTableProcess(s4)
				elif tbl_id == 81:
					self.LogTableProcess(s4)
				if pktLength > length:
					print 'receive one more msg'
					lines = lines[length:]
					continue
				else :
					break
					
			return (event_manager, update_event)		
					
    def CemTableProcess(self, tmp):
		"""
		process tbl_cem packets
		"""
		global update_event
		rcv_msg_hdr = msg_hdr_t()
		msg_data_hdr = msg_data_hdr_t()
		responsePktLength = 0

		responsePktLength = len(tmp)
	
		dataLength = responsePktLength -struct.calcsize('>' + rcv_msg_hdr.fmt ) - struct.calcsize('>' + msg_data_hdr.fmt )
		s1 = struct.unpack('B'*responsePktLength, tmp)    
		s2 = tmp[:struct.calcsize(rcv_msg_hdr.fmt )]
		s3 = tmp[struct.calcsize(rcv_msg_hdr.fmt ):struct.calcsize(rcv_msg_hdr.fmt ) + struct.calcsize(msg_data_hdr.fmt ) ]
		s4 = tmp[struct.calcsize(rcv_msg_hdr.fmt ):]
		data = tmp[struct.calcsize(rcv_msg_hdr.fmt ) + struct.calcsize(msg_data_hdr.fmt ) :]
		datax = struct.unpack('B'*dataLength , data)
		tbl_cem = tbl_cem_t()
		struct_mail = mail_t()
		version, operation, length, seq = struct.unpack('>' + rcv_msg_hdr.fmt , s2)
		

		if  responsePktLength!= length:
			print 'ERROR!! Subscribe msg length %d, packets length %d', length, responsePktLength
			return 
		oper, tbl_type, tbl_id, ds_id, ds2_id, fields0, fields1=struct.unpack('>' + msg_data_hdr.fmt, s3)
		#print 'oper, tbl_type, tbl_id, ds_id, ds2_id, fields0, fields1 = ', oper, tbl_type, tbl_id, ds_id, ds2_id, fields0, fields1
		   
		if oper == CDB_OPER_ADD:
			print 'creat cemtable ....'
			update_event = []
			
		elif oper == CDB_OPER_SET:

			tbl_cem.key,\
			tbl_cem.event,\
			tbl_cem.threshold,\
			tbl_cem.snmptrap,\
			tbl_cem.loadpath,\
			tbl_cem.server,\
			tbl_cem.fro,\
			tbl_cem.to,\
			tbl_cem.cc,\
			tbl_cem.subject,\
			tbl_cem.body,\
			tbl_cem.attach = struct.unpack(tbl_cem.fmt, data)
						
			tbl_cem.key = modify_str(tbl_cem.key)
			tbl_cem.event = modify_str(tbl_cem.event)
			if tbl_cem.event == 'if-updown':
				tbl_cem.event = 'interface'
			tbl_cem.loadpath = modify_str(tbl_cem.loadpath)
			tbl_cem.loadpath = base64.decodestring(tbl_cem.loadpath)
			tbl_cem.server = modify_str(tbl_cem.server)
			tbl_cem.fro = modify_str(tbl_cem.fro)
			tbl_cem.to = modify_str(tbl_cem.to)
			tbl_cem.cc = modify_str(tbl_cem.cc)
			tbl_cem.subject = modify_str(tbl_cem.subject)
			tbl_cem.body = modify_str(tbl_cem.body)
			tbl_cem.attach = modify_str(tbl_cem.attach)
			
			if len(tbl_cem.event) == 0:
				if event_manager.has_key(tbl_cem.key):			
					event = event_manager[tbl_cem.key]['event']
					update_event = [event,'delete']	
			else:
				update_event = [tbl_cem.event,'update']
			event_manager[tbl_cem.key] = {'event' : tbl_cem.event, 'threshold' : tbl_cem.threshold, 'snmptrap' : tbl_cem.snmptrap,'loadpath': tbl_cem.loadpath}
			event_manager[tbl_cem.key]['mail'] ={'server' : tbl_cem.server, 'from' : tbl_cem.fro, 'to' : tbl_cem.to, 'subject' : tbl_cem.subject, 'cc' : tbl_cem.cc, 'body' : tbl_cem.body, 'attach' : tbl_cem.attach}		
			return (event_manager, update_event)	
			
		elif oper == CDB_OPER_DEL:
			print 'data is',data
			tbl_cem.key = data						
			tbl_cem.key = modify_str(tbl_cem.key)
			if event_manager.has_key(tbl_cem.key):			
				key = event_manager[tbl_cem.key]['event']
				update_event = [key,'delete']	
				event_manager.pop(tbl_cem.key) 
				return (event_manager, update_event)

    def LogTableProcess(self, tmp):
		"""
		process tbl_cem packets
		"""
		global update_event
		rcv_msg_hdr = msg_hdr_t()
		msg_data_hdr = msg_data_hdr_t()
		responsePktLength = 0

		responsePktLength = len(tmp)

		dataLength = responsePktLength -struct.calcsize('>' + rcv_msg_hdr.fmt ) - struct.calcsize('>' + msg_data_hdr.fmt )
		s1 = struct.unpack('B'*responsePktLength, tmp)    
		s2 = tmp[:struct.calcsize(rcv_msg_hdr.fmt )]
		s3 = tmp[struct.calcsize(rcv_msg_hdr.fmt ):struct.calcsize(rcv_msg_hdr.fmt ) + struct.calcsize(msg_data_hdr.fmt ) ]
		s4 = tmp[struct.calcsize(rcv_msg_hdr.fmt ):]
		data = tmp[struct.calcsize(rcv_msg_hdr.fmt ) + struct.calcsize(msg_data_hdr.fmt ) :]
		print 'data len is',len(data)	
	
		datax = struct.unpack('B'*dataLength , data)
		tbl_log = tbl_log_t()
	
		version, operation, length, seq = struct.unpack('>' + rcv_msg_hdr.fmt , s2)


		if  responsePktLength!= length:
			print 'ERROR!! Subscribe msg length %d, packets length %d', length, responsePktLength
			return 
		oper, tbl_type, tbl_id, ds_id, ds2_id, fields0, fields1= struct.unpack('>' + msg_data_hdr.fmt, s3)
		#print 'oper, tbl_type, tbl_id, ds_id, ds2_id, fields0, fields1 = ', oper, tbl_type, tbl_id, ds_id, ds2_id, fields0, fields1
					
		if oper == CDB_OPER_ADD:

			tbl_log.key,\
			tbl_log.log_id,\
			tbl_log.severity,\
			tbl_log.timestamp,\
			tbl_log.data = struct.unpack(tbl_log.fmt, data)
						
			tbl_log.data = modify_str(tbl_log.data)
			print 'tbl_log_data is', tbl_log.data
			data = tbl_log.data + '\n'
			log = open('/tmp/log.txt','a+')
			log.write(data) 
			log.close()
			
			
			
			




