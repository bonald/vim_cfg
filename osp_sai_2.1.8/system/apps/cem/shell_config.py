#!/usr/bin/env python
#!-*-coding:utf-8-*-

import sys
import os
import pty
import select
import time
import thread
import signal

class KlishDriver:
    def __init__(self):
        self.shell = "klish"
        self.delay = 0.03
        return

    def getDriverName(self):
        '''
        Get driver name
        '''
        return "Klish"

    def startDriver(self):
		'''
		Start driver
		'''
		self.pid, self.fd = pty.fork()
		self.outd = self.fd

		self.ind = self.fd
		self.errd = self.fd
		if self.pid == 0:
			try:
				os.execv("/usr/sbin/klish", ["klish"])
			except :
				log = open('/tmp/test.txt','w')
				print >> log, "error"
				log.close()
		return

    def stopDriver(self):
		'''
		Stop driver
		'''
		os.kill(self.pid,signal.SIGKILL)
		os.wait()
		return

    def Read(self):
        num_iterations = 0
        any_lines_read = 0
        lines = ""

        while True:
            try:
                r = select.select([self.outd], [], [], self.delay)[0]
            except select.error, msg:
                r = [1, ]
            for file_iter in r:	
                print "outd is", self.outd			
                tmp = os.read(self.outd, 1024)
                lines += tmp
                if tmp == "":
                    r = []
                    break
                any_lines_read = 1
                num_iterations += 1
            if not len(r) and len(lines):
                lines_to_return = lines.split(os.linesep)
                time.sleep(1)
                return lines_to_return

                break
            elif not any_lines_read and not num_iterations:
                break
            else:
                pass
        return

    def ExecuteCmd(self, cmd):
        print cmd
        cmd = cmd.strip()
        cmd = cmd + os.linesep
        self.Write(cmd)
        return

    def Write(self, cmd):
        os.write(self.ind, cmd)
        return

    def executeCmds(self, cmdTokenList):
		for strCmd in cmdTokenList:
			self.ExecuteCmd(strCmd)



