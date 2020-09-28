#!/usr/bin/python
# -*- coding: utf-8 -*-

try:
    import pexpect
except:
    pass
import sys
import types
import re, socket


class Clilib(object):
    def __init__(self, host='127.0.0.1', port=23, timeout=300, debug=False, ret_regexp=r'.', ret_regexp1=r'.', ret_regexp2 =r'.', mode=r'klish -p'):
        self.__host = host
        self.__timeout = timeout
        self.__boot_mode = False
        self.__confirm_mode = False
        self.__confirm_string = ['Please type \'yes\' or \'no\','
                                 'Please answer \'yes\' or \'no\'']

        try:
            self.__cli = pexpect.spawn(mode, timeout = timeout)
            #self.__match()
        except:
            Exception
            print 'cli start up fail'
        self.__debug = debug
        self.__ret_regexp = ret_regexp
        self.__ret_regexp1 = ret_regexp1
        self.__ret_regexp2 = ret_regexp2
        self.__wait()

    def __match(self):
        hostname = socket.gethostname()
        ret_regexp=hostname + r'#'
        ret_regexp1=hostname + r'\(.*#'
        ret_regexp2=hostname + r'>'

        index = self.__cli.expect([ret_regexp, ret_regexp1,ret_regexp2,
                                  r'--More--', r'\[confirm\]', r'\[yes\/no\]',r'Password:',
                                   r'\[no\]',r'server \[\]',r'name \[\]', r'Password \[\]',
                                   r'(yes\/no)',r'Username:',pexpect.EOF, pexpect.TIMEOUT])
        text = self.__cli.before

        if self.__debug:
            sys.stdout.write(text)
        return (index, text)

    def __wait(self):
        resp = ''
        while True:
            index, text = self.__match()
            resp += text
            if index == 0 or index == 1 or index == 2 or index == 4 \
                    or index ==5 or index ==6 or index ==7 or index ==8 \
                    or index==9 or index==10 or index == 11 or index ==12:
                break
            elif index == 3:
                self.__cli.write(' ')
            else:
                index = 1000
                break
        return index, resp

    def __is_confirm_str_output(self,resp):
        for con_string in self.__confirm_string:
            if resp.find(con_string) >= 0:
                return True
        return False

    def cmd(self, cmd):
        if self.__boot_mode:
            self.__cli.write('%s' % str(cmd))
        elif self.__confirm_mode:
            self.__cli.write('%s\n' % str(cmd))
        else:
            # BUG 52277 rpc with include abnormal
            if cmd.find('include'):
                self.__cli.write('%s\n' % str(cmd))
            else:
                self.__cli.write('%s\n' % str(cmd + ' '))
        index, resp = self.__wait()

        if (self.__confirm_mode or self.__boot_mode) and \
                (index == 0 or index == 1) and resp.strip() \
                and not self.__is_confirm_str_output(resp):
            self.__cli.write('\n')
            index1, resp1 = self.__wait()
            #print 'index1, resp1'
            #print index1, resp1

        self.__confirm_mode = False
        self.__boot_mode = False
        if index > 3 and index < 100:
            self.__confirm_mode = True
        if self.__is_confirm_str_output(resp):
            self.__confirm_mode = True

        if index == 4:
            self.__boot_mode = True

        if index == 1000:
            self.__cli.sendcontrol('c')
            index, res = self.__wait()
            res += '% timeout for cli response'
        if index == 1000:
            return res
        else:
            return resp

    def __wait_mode(self):
        index, text = self.__match()
        return index, text

    def get_mode(self):
        self.__cli.write('%s\n' % str(' '))
        mode, text = self.__wait_mode()
        return mode

    def is_wait_input(self):
        return self.__confirm_mode

    def __easy_match(self):
        ret_regexp=r'#'
        ret_regexp1=r'>'
        index = self.__cli.expect([ret_regexp, ret_regexp1,pexpect.EOF, pexpect.TIMEOUT])
        text = self.__cli.before

        if self.__debug:
            sys.stdout.write(text)
        return (index, text)

    def __easy_wait(self):
        resp = ''
        while True:
            index, text = self.__easy_match()
            resp += text
            if index == 0 or index == 1 or index == 2:
                break
            elif index == 3:
                self.__cli.write(' ')
            else:
                index = 1000
                break
        return index, resp

    def easy_cmd(self, cmd):
        self.__cli.write('%s\n' % str(cmd))
        index, resp = self.__easy_wait()
        if index == 1000:
            self.__cli.sendcontrol('c')
            index, res = self.__wait()
            res += '% timeout for cli response'
        if index == 1000:
            return res
        else:
            return resp

    def exit(self):
        self.__cli.sendline('exit\n')

    def read_all(self):
        return self.__cli.send()

