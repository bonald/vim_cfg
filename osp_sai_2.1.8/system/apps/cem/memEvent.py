#!/usr/bin/python
import ceClass
import time
import sys,os
sys.path.insert(0, os.path.dirname(os.getcwd()))

	
class memEvent(ceClass.cEvent):
	coreNum = 1
	def __init__(self, name):
		self.memthreshold = 100
		self.snmptrap = 0
		self.mail = 0
		self.fro = ''
		self.to = []
		self.cc = []
		self.subject = ''
		self.attach = []
		self.body = ''
		self.server_addr = ''
		self.server_port = 0
		self.username = ''
		self.password = ''
		self.is_emailed = 0
		self.start = 0
		
	def refresh(self):
		self.memthreshold = 100
		self.mail = 0
		self.snmptrap = 0
		self.fro = ''
		self.to = []
		self.cc = []
		self.subject = ''
		self.attach = []
		self.body = ''
		self.server_addr = ''
		self.server_port = 0
		self.username = ''
		self.password = ''
		self.is_emailed = 0
		self.start = 0	
		
	def getCoreNum(self):
		print self.coreNum
	
	def getEvent(self):
		mem = self.mem_usage()
		self.current_mem = mem
		if mem > self.memthreshold:
			print 'current memory usage is %.2f%%,the memory threshold is %d%%'% (mem, self.memthreshold)
			return 1

	def actionEvent(self, ceCon):
		if 1 == ceCon:
			print 'memory is too high'
			
	def setThreshold(self,value):
		self.memthreshold = value
		
	def action2send_snmptrap(self,trap_agent):		
		agent = trap_agent
		#This actually connects the agent to the master agent.
		try:
			agent.start()
		except netsnmpAgentException as e:
			print "{0}: {1}".format(prgname, e)
			sys.exit(1)
			
		enterprise_id = agent.get_enterprise_id()
		idle = self.free_mem	
		#config the trap oid,uptime,
		trapoid = "1.3.6.1.4.1.{0}.1.7.4".format(enterprise_id)
		uptime = 100 * int(os.popen('cat /proc/uptime').read().split('.')[0])
		vararr = ["1.3.6.1.4.1.{0}.1.1.11".format(enterprise_id), 'i', idle]
		agent.Trapv2(trapoid, uptime, vararr)		
		
	def mem_usage(self):  
		try:
			fp = open("/proc/meminfo", 'r')
		except IOError, e:
			print "*** file open error", e

		allLines = fp.readlines()
		fp.close()
		
		for eachLine in allLines:
			if "MemTotal" in eachLine:
				totals = eachLine.split()
			if "MemFree" in eachLine:
				frees = eachLine.split()
			if "Cached" in eachLine and "SwapCached" not in eachLine:
				uses = eachLine.split()
		
		total = float(totals[1])
		free = float(frees[1])
		self.free_mem = int(free)
		use = float(uses[1])
		
		float_mem_using = 100 * ( (total-free)/ total )
		return round(float_mem_using, 2)		