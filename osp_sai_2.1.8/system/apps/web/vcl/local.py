#!/usr/bin/python
#-*- coding: utf-8 -*-

import os
import commands
from base_cli import BaseCli

class Local(BaseCli):
    
    def __init__(self):
        pass
    
    def cmd(self, cmd):
        """send a command and return a array for each line
        """
#        return [x.strip() for x in os.popen(cmd).readlines()]
        return [x.strip() for x in commands.getoutput(cmd).split('\n')]
    
    def send(self, cmd):
        """excute cmd only, don't wait the response
        """
        os.system(cmd)
        

if __name__ == '__main__':
    
    ins = Local()
    print ins.get_status_lines('ls')
    
    pass