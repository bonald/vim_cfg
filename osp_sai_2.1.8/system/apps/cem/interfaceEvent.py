import ceClass
import time
import sys 
import os
sys.path.insert(0, os.path.dirname(os.getcwd()))


	
class interfaceEvent(ceClass.cEvent):
	coreNum = 1
	
	def __init__(self, name):
		self.snmptrap = 0
		self.if_en = 0
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
		self.if_en = 0
		
	def getCoreNum(self):
		print self.coreNum
	
	def getEvent(self):
		if self.if_en == 1:
			return 1
		else:	
		    return 0
			
	def setIf_enable(self):
		self.if_en = 1
		
	def action2send_snmptrap(self,trap_agent):		
		return
		'''
		agent = self.trap_agent
		#This actually connects the agent to the master agent.
		try:
			agent.start()
		except netsnmpAgentException as e:
			print "{0}: {1}".format(prgname, e)
			sys.exit(1)
			
		enterprise_id = agent.get_enterprise_id()
		idle = 100 - self.cpu_usage	
		#config the trap oid,uptime,
		trapoid = "1.3.6.1.4.1.{0}.1.7.9".format(enterprise_id)
		uptime = 100 * int(os.popen('cat /proc/uptime').read().split('.')[0])
		vararr = ["1.3.6.1.4.1.{0}.1.2.11".format(enterprise_id), 'i', idle]
		agent.Trapv2(trapoid, uptime, vararr)	
		'''




