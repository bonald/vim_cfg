#!/ctcdata/users/sw/wangqj/softwares/python/bin/python
#-*- coding: utf-8 -*-

from testlib import Testlib

if __name__ == '__main__':


    test = Testlib('10.10.39.157', ret_regexp='DUT\d.*#')

#    test.cmds(['configure terminal', 'qos global', 'qos enable', 'quit', 'qos global', 'no qos enable', 'end'])

    print test.cmd('show interface eth-0-1')

    test.exit()

