#!/usr/bin/python

from email.mime.multipart import MIMEMultipart
from email.mime.base import MIMEBase
from email.mime.text import MIMEText
# python 2.3.*: email.Utils email.Encoders
from email.utils import COMMASPACE,formatdate
from email import encoders
import sys, os, time
sys.path.insert(0, os.path.dirname(os.getcwd()))

import shlex
import base64	
import ctypes    	 
import platform
import shell_config
import re
		
CLONE_NEWNET = 0x40000000   
NETNSPATH = "/var/run/netns/mgmt"  

class cEvent:
	
	def __init__(self, name):
		self.name = name
		self.mail = 0
		self.snmptrap = 0
		self.fro = ''
		self.to = []
		self.cc = []
		self.subject = ''
		self.attach = []
		self.body = ''
		self.server_addr = ''
		self.server_port = 25
		self.username = ''
		self.password = ''
		self.is_emailed = 0
		self.start = 0

		
	def displayCevent(self):
		print self.name 
	
	def setns(self,path):            
		libc = ctypes.CDLL('libc.so.6', use_errno=True)
		netnspath = path
		nsfd = os.open(netnspath, os.O_RDONLY)
		self.get_setns()
		ret = libc.syscall(self.__NR_setns, nsfd, CLONE_NEWNET)
		return nsfd  
		
	def get_setns(self):
		if platform.machine() == 'ppc':
			self.__NR_setns = 350
		elif platform.machine() == 'i686':
			if sys.maxsize > 2**32:
				self.__NR_setns = 375
			else:	
				self.__NR_setns = 364
		print 'setns is ',self.__NR_setns		
				
	def getEvent(self):
		print "get centec event not define"	
		
	def set_snmptrap(self,snmptrap):
		self.snmptrap = snmptrap
		
	def get_snmptrap(self):
		return self.snmptrap
		
	def set_loadconfig(self,path=''):
		print'path is',path
		if len(path) != 0:
			self.loadconfig = 1
			self.loadpath = path
		else:
			self.loadconfig = 0
			self.loadpath = ''	
		
	def get_loadconfig(self):
		return self.loadconfig
		
	def set_mail_disable(self):
		self.mail = 0
		
	def set_mail(self, tmp):
		self.mail = 1
		self.fro = tmp['from']
		self.to = self.parse_mail_member(tmp['to'])
		self.cc = self.parse_mail_member(tmp['cc'])
		self.subject = tmp['subject']		
		self.subject = base64.decodestring(self.subject)
		self.body = tmp['body']	
		self.body = base64.decodestring(self.body)		
		self.attach = self.parse_mail_member(tmp['attach'])
		for j in range(len(self.attach)):
			self.attach[j] = base64.decodestring(self.attach[j])

		if '\\n' in self.subject:
			self.subject = self.subject.replace('\\n','\n')
		if '\\n' in self.body:
			self.body = self.body.replace('\\n','\n')	

		tmp_server = self.parse_mail_member(tmp['server'])
		if len(tmp_server) == 3:
			self.username = tmp_server[1]
			self.password = tmp_server[2]
			if ':' in tmp_server[0]:
				print'has port_id'
				str = tmp_server[0]
				str = str.split(':')
				self.server_addr = str[0]
				self.server_port = int(str[1])
			else:
				self.server_addr = tmp_server[0]
		elif len(tmp_server) == 1:
			if ':' in tmp_server[0]:
				print'has port_id'
				str = tmp_server[0]
				str = str.split(':')
				self.server_addr = str[0]
				if len(str[1]) == 0:
					self.server_port = 25
				else:	
					self.server_port = int(str[1])
			else:
				self.server_addr = tmp_server[0]
					
		
		
	def parse_mail_member(self,member):
		str =shlex.shlex(member,posix = True)
		str.whitespace = ','
		str.whitespace_split = True 
		return list(str)
		
	def get_mail(self):
		return self.mail
	
	def action2log(self, cEventCondition):
		logFile = open("/tmp/test.txt", "w")
		logFile.write(self.name + "::" + self.cCondition)
		logFile.close()
		print(open('/tmp/test.txt').read())
		
	def action2send_mail(self): 

		if self.is_emailed == 0:
			self.is_emailed = 1
			self.start = time.time()
			print 'start clock..'
		else:
			escaped = time.time() - self.start
			if escaped > 300 :
				self.is_emailed = 0
				print 'time refresh..'
			else:
				print 'time is',int(escaped)
				return
				
		self.setns(NETNSPATH) 
		if self.__NR_setns != 350:
			print 'UML can not send mails'
			return
			
		msg = MIMEMultipart() 
		msg['From'] = self.fro
		msg['Subject'] = self.subject 
		msg['To'] = COMMASPACE.join(self.to) #COMMASPACE==', ' 
		msg['Cc'] = COMMASPACE.join(self.cc) #COMMASPACE==', 
		msg['Date'] = formatdate(localtime=True) 
		msg.attach(MIMEText(self.body)) 
		 
		for file in self.attach: 
			part = MIMEBase('application', 'octet-stream') #'octet-stream': binary data 
			part.set_payload(open(file, 'rb').read()) 
			encoders.encode_base64(part) 
			part.add_header('Content-Disposition', 'attachment; filename="%s"' % os.path.basename(file)) 
			msg.attach(part) 
			

		import smtplib 
		if len(self.server_addr) == 0:
			smtp = smtplib.SMTP('localhost') 
			smtp.login('zhoulei', 'centec') 
		else:
			smtp = smtplib.SMTP(self.server_addr,self.server_port) 
		if len(self.username) != 0:	
			smtp.login(self.username, self.password) 
		smtp.sendmail(self.fro, self.to, msg.as_string()) 
		smtp.close() 
		
	def action2send_snmptrap(self,trap_agent):		
		agent = trap_agent
		#This actually connects the agent to the master agent.
		try:
			agent.start()
		except netsnmpAgentException as e:
			print "{0}: {1}".format(prgname, e)
			sys.exit(1)
			
		enterprise_id = agent.get_enterprise_id()
		cpu_using = 60	
		#config the trap oid,uptime,
		trapoid = "1.3.6.1.4.1.{0}.1.7.4".format(enterprise_id)
		uptime = 100 * int(os.popen('cat /proc/uptime').read().split('.')[0])
		vararr = ["1.3.6.1.4.1.{0}.1.1.11".format(enterprise_id), 'i', cpu_using]
		agent.Trapv2(trapoid, uptime, vararr)
		
	def action2load_config(self):	
		config = shell_config.KlishDriver()	
		try:	
			file = open(self.loadpath).read()	
		except:
			print'error path'
			return
		cmdlist = re.split(r'\n',file)
		config.startDriver()
		config.executeCmds(cmdlist)
		time.sleep(3)
		config.stopDriver()
	
		