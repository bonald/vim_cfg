#!/usr/bin/python
#-*- coding: utf-8 -*-

import telnetlib
import config
import sys
import re
from base_cli import BaseCli
import socket


class Telnet(object):
    
    def __init__(self, host, port = 23, timeout = 10, debug = False, default_regexp = r'.'):
        """asynchronous connect, you might call wait() by yourself
        """
        self.__host = host
        self.__tn = telnetlib.Telnet(host, port, timeout)
        self.__debug = debug
        self.__default_regexp = default_regexp

    def wait(self, regex_arr = None):
        """wait stdout response
        """
        if regex_arr == None:
            regex_arr = [self.__default_regexp]
            
        (match_index, match_obj, text) = self.__tn.expect(regex_arr)
        if self.__debug:
            sys.stdout.write(text)
        return (match_index, text)


    def cmd(self, cmd):
        """send cmd, also asynchronous
        """
        self.__tn.write('%s\n' % str(cmd))

    def close(self):
        self.__tn.close()
    
    
class Remote(BaseCli):
    
    def __fix_telnet_bug(self, cmd, text):
        text = text.replace('\r\r\n', '')
        return text
        
    
    def __init__(self):
        self.__telnet = Telnet(host=config.remote, default_regexp=config.cmd_regexp)
        (index, text) = self.__telnet.wait([config.dut_regexp, config.login_stdout_username_info])
        if index == 1: # matched 'Username'
            self.__login()
        self.__shell_login()
        
    def __login(self):
        self.__telnet.cmd(config.login_username)
        self.__telnet.wait([config.login_stdout_pwd_info])
        self.__telnet.cmd(config.login_pwd)
        self.__telnet.wait([config.dut_regexp])
    
    def __shell_login(self):
        self.__telnet.cmd('start shell')
        self.__telnet.wait([config.dut_stdout_pwd_info])
        self.__telnet.cmd(config.shell_pwd)
        self.__telnet.wait()
        
    def close(self):
        self.__telnet.close()
        
    def __get_stdout(self, cmd):
        """send a command and return stdout
        """
        self.__telnet.cmd(cmd)
        (index, text) = self.__telnet.wait()
        text = self.__fix_telnet_bug(cmd, text)
        text = text.replace(cmd + '\r\n', '')
        text = text.replace(cmd, '')
        text = re.sub(config.cmd_regexp, '', text)
        return text.strip()
    
    def cmd(self, cmd):
        """send a command and return a array for each line
        """
        str_out = self.__get_stdout(cmd)
        return [x.strip() for x in str_out.split('\n')]
    
    def send(self, cmd):
        """excute cmd only, don't wait the response
        """
        self.__telnet.cmd(cmd)

    def getLocalIP(self):
        myname = socket.getfqdn(socket.gethostname())
        myaddr = socket.gethostbyname(myname)
        return myaddr



    def get_arr(self, cmd):
        """send cmd and convert response to dict array
        """
        res = self.cmd(cmd)
        arr = vcmd._get_arr(res)
        log.write('get_arr: %s\n    %s' % (str(cmd), str(arr)))
        return arr

if __name__ == '__main__':
    
    ins = Remote()
#    print ins.cmd('cdbctl read/cdb/l2/vlan')
#    print ins.execute('jfdklsajflksa')
    print ins.getLocalIP()
    #print ins.cmd('ls')
    #print ins.get_status_lines('cdbctl read/cdb/interface/eth-0-5')
    print ins.get_arr('cdbctl read/cdb/interface/eth-0-5')

    
    ins.close()
    
