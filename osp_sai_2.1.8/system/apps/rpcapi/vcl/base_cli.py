#!/usr/bin/python
#-*- coding: utf-8 -*-

import base

class BaseCli:
    
    def send(self, cmd):
        """excute cmd only, don't wait the response
        """
        pass
    
    def cmd(self, cmd):
        """send a command and return a array for each line
        """
        pass
        
    def execute(self, cmd):
        """only execute command, just return $?
        """        
        lines = self.cmd(cmd + ';echo $?')
        return base.parse.parse_int(lines[-1])

    def get_status_lines(self, cmd):
        """return status code and output lines array
        """
        lines = self.cmd(cmd + ';echo $?')
        if len(lines) == 0:
            return (127, ['reap is null'])
        elif len(lines) == 1:
            return (base.parse.parse_int(lines[-1]), [''])
        else:
            return (base.parse.parse_int(lines[-1]), lines[0:-1])

    def get_status_lines_sp(self, cmd):
        """return status code and output lines array
        """
        lines = self.cmd(cmd)
        retv = self.cmd('echo $?')
        reti = base.parse.parse_int(retv[0])
        if len(lines) == 0:
            return (reti, [''])
        else:
            return (reti, lines[0:])