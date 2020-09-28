#!/usr/bin/python
# -*- coding: utf-8 -*-

import telnetlib
import sys
import types
import re


def format_str_color(string, int_f_color, int_b_color):
    return '\033[%d;%dm%s\033[0m' % (int_f_color, int_b_color, string)


class Testlib(object):
    def __init__(self, host, port=23, timeout=10, debug=False, ret_regexp=r'.'):
        self.__host = host
        self.__tn = telnetlib.Telnet(host, port, timeout)
        self.__debug = debug
        self.__ret_regexp = ret_regexp
        self.__wait()

    def __match(self):
        (match_index, match_obj, text) = self.__tn.expect([self.__ret_regexp, r'--More--'])
        if self.__debug:
            sys.stdout.write(text)
        return (match_index, text)

    def __wait(self):
        resp = ''
        while True:
            index, text = self.__match()
            resp += text
            if index == 0:
                break
            elif index == 1:
                self.__tn.write(' ')
        return resp

    def cmd(self, cmd):
        self.__tn.write('%s\n' % str(cmd))
        #print cmd
        resp = self.__wait()
        return resp

    def cmds(self, cmd_list):
        if type(cmd_list) is types.ListType:
            for cmd in cmd_list:
                self.cmds(cmd) if type(cmd) is types.ListType else self.cmd(cmd)
        else:
            self.cmd(cmd_list)

    def exit(self):
        self.__tn.write('exit\n')

    def read_all(self):
        return self.__tn.read_all()


_CLEAR_CONF = [  # format: match_regex, clear command (param use <$>)
    [r'\npolicy-map (\w+)', 'no policy-map <$>'],
    [r'\nclass-map (\w+)', 'no class-map <$>'],
    [r'\nip access-list (\w+)', 'no ip access-list <$>'],
    [r'\nvlan (\d)[\n\s]', 'no vlan <$>'],
    [r'\nvlan range ([\d,]+)', 'no vlan range <$>'],
    [r'\nvlan range (\d-\d)', 'no vlan range <$>'],
    [r'\nqos policer-profile (\w+)', 'no qos policer-profile <$>'],
    #    [r'\nqos global', ['qos global', 'no qos enable', 'quit']],
]


class DUTTestlib(Testlib):
    def __init__(self, host, port=23, timeout=10, debug=False, ret_regexp=r'DUT\d.*#'):
        super(DUTTestlib, self).__init__(host, port, timeout, debug, ret_regexp)

    def config(self, cmds):
        self.cmd('configure terminal')
        if (type(cmds) is types.ListType):
            self.cmds(cmds)
        else:
            self.cmd(cmds)
        self.cmd('end')

    def __exec_clear_config(self, conf_txt):
        cmds = []

        for item in _CLEAR_CONF:
            resp = re.findall(item[0], conf_txt)
            for i in resp:
                cmds.append = item[1].replace('<$>', i)
                self.config(cmds)

    def clear_config(self):
        print format_str_color('\nreading config...', 31, 40)
        conf_txt = self.cmd('show running-config')
        self.__exec_clear_config(conf_txt)
