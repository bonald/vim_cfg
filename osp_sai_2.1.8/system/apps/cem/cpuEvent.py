import ceClass
import time
import sys 
import os
sys.path.insert(0, os.path.dirname(os.getcwd()))

	
class cpuEvent(ceClass.cEvent):
	coreNum = 1
	
	def __init__(self, name):
		self.cputhreshold = 100
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
		cpu = self.cpu_rate()
		if cpu > self.cputhreshold:
			print 'current cpu usage is %.2f%%,cpu threshold is %d%%' % (cpu, self.cputhreshold)
			return 1
			
	def setThreshold(self,value):
		self.cputhreshold = value
	
	def action2send_snmptrap(self,trap_agent):		
		agent = trap_agent
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
		
	def cpu_rate(self):  
		def cpu_r(): 
			f = open("/proc/stat","r") 
			for f_line in f: 
				break
			f.close() 
			f_line = f_line.split(" ") 
			f_line_a=[] 
			for i in f_line: 
				if i.isdigit(): 
					i=int(i) 
					f_line_a.append(i) 
			total = sum(f_line_a) 
			idle = f_line_a[3] 
			return total,idle 

		total_a,idle_a=cpu_r() 
		time.sleep(0.1) 
		total_b,idle_b=cpu_r() 

		sys_idle = idle_b - idle_a 
		sys_total = total_b - total_a 
		sys_us = sys_total - sys_idle 

		cpu_a = (float(sys_us)/sys_total)*100
		self.cpu_usage = int(cpu_a)
		return round(cpu_a, 2) 



