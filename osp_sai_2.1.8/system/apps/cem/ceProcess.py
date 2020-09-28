#!/usr/bin/python
import ceClass
import cpuEvent
import memEvent
import interfaceEvent
import time
import cemClient 
import threading, signal,sys,os
import optparse
from daemon3x import daemon as Daemon


# Make sure we use the local copy, not a system-wide one
sys.path.insert(0, os.path.dirname(os.getcwd()))

prgname = sys.argv[0]

# Process command line arguments
usage = "usage: %prog [options] [stop]"  
parser = optparse.OptionParser(usage=usage)

parser.add_option(
	"-i",
	"--pid-file",
	dest="pidfile",
	help="Sets the pid file",
	default="/var/run/system_agent.pid"
)

parser.add_option(
	"-d",
	"--daemon",
	dest="bg",
	action="store_true",
	help="Daemon mode",
)
(options, args) = parser.parse_args()

myaction=""
if len(args) == 1:
	if args[0] != "stop":
		parser.print_help()
		sys.exit(1)
	myaction="stop"
elif len(args) != 0:
	parser.print_help()
	sys.exit(1)


# Get terminal width for usage with pprint


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

cEventList = []

cpuInst = cpuEvent.cpuEvent("cpu")
memInst = memEvent.memEvent("memory")
ifInst = interfaceEvent.interfaceEvent("interface")
cEventList.append(cpuInst)
cEventList.append(memInst)
cEventList.append(ifInst)
subscribe = cemClient.subscribeCDBTable()



def socketserver():	
	subscribe.connect()
	subscribe.subscribeTable(80)
	subscribe.subscribeTable(81)
	while True:
		(event_manager,update_event) = subscribe.read()	
		print 'msg is',event_manager,update_event
		
		if len(update_event) != 0 :
			if update_event[1] == 'delete':
				if update_event[0] == 'cpu':
					cpuInst.refresh()
				elif update_event[0] == 'memory':
					memInst.refresh()
				elif update_event[0] == 'interface':
					ifInst.refresh()	
					
		KS = list(event_manager.keys())
		KS.sort()
		if len(KS) is not 0:
			for k in KS:
				if event_manager[k]['event'] == 'cpu':
					tmpInst = cpuInst
					tmpInst.setThreshold(event_manager[k]['threshold'])
				elif event_manager[k]['event']  == 'memory':
					tmpInst = memInst 					
					tmpInst.setThreshold(event_manager[k]['threshold'])
				elif event_manager[k]['event']  == 'interface':
					tmpInst = ifInst
					tmpInst.setIf_enable()
				if	len(event_manager[k]['event']):
					if event_manager[k]['snmptrap'] == 1:
						tmpInst.set_snmptrap(1)
					else:
						tmpInst.set_snmptrap(0)
					if len(event_manager[k]['mail']['from']) != 0:
						tmp = event_manager[k]['mail']
						tmpInst.set_mail(tmp)
					else:	
						tmpInst.set_mail_disable()
					if len(event_manager[k]['loadpath']) != 0:
						tmp_path = event_manager[k]['loadpath']
						tmpInst.set_loadconfig(tmp_path)
					else:
						tmpInst.set_loadconfig()
					

def eventserver():
	from netsnmpagent import netsnmpAgent
	agent = netsnmpAgent(
			MasterSocket   = "/var/run/agentx-master",
		)
	while True:
		for cEventMem in cEventList:
			time.sleep(5)
			rc = cEventMem.getEvent()
			if rc == 1:
				if cEventMem.get_snmptrap() == 1:
					cEventMem.action2send_snmptrap(agent)
				if cEventMem.get_mail() == 1:
					cEventMem.action2send_mail()
				if cEventMem.get_loadconfig() == 1:
					cEventMem.action2load_config()

if __name__ == "__main__":
	threads = []
	t1 = threading.Thread(target = socketserver)
	threads.append(t1)
	t2 = threading.Thread(target = eventserver)
	threads.append(t2)
	for t in threads:
		t.setDaemon(True)
		t.start()
	t.join()	

	
	



