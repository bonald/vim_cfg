#!/usr/bin/python
#-*- coding: utf-8 -*-

import os
import commands
import sys
import time

def start():
    root_path = sys.path[0]
    # vcl __init__
    vcl_init_path = root_path + '/vcl/__init__.py'	
    os.system(r"sed -i 's/config\.is_dev.*=.*True/config\.is_dev = False/g' %s" % vcl_init_path) # disable dev mode

    # base config
    base_config_path = root_path + '/base/config.py'
    os.system(r"sed -i 's/log_en.*=.*True/log_en = False/g' %s" % base_config_path) # disable log

    # disable debug
    index_path = root_path + '/index.py'
    os.system(r"sed -i 's/debug.*=.*True/debug=False/g' %s" % index_path) # disable flask debug 

    # modify itsdangerout EPOCH
    uname_arch = os.popen('uname -m').read().strip()
    if cmp(uname_arch, 'x86_64') == 0:
        os.system(r"sed -i 's/^EPOCH = 1293840000/EPOCH = 0/' /usr/local/ctc_python/lib/python2.7/site-packages/itsdangerous.py")
    elif os.path.exists('/etc/debian_version'):
        os.system(r"sed -i 's/^EPOCH = 1293840000/EPOCH = 0/' /usr/lib/python2.7/dist-packages/itsdangerous.py")
    else:
        os.system(r"sed -i 's/^EPOCH = 1293840000/EPOCH = 0/' /usr/lib/python2.7/site-packages/itsdangerous-0.24-py2.7.egg/itsdangerous.py")

    # run server
    os.system('python %s >/dev/null 2>/dev/null &' % index_path)

def stop():
    # kill the process
    if os.path.exists('/etc/debian_version'):
        os.system(r"ps aux --cols 200 | grep python | grep rpcapi/index.py | awk '{print $2}' | xargs kill >/dev/null 2>&1")
    else:
        os.system(r"ps aux | grep python | grep rpcapi/index.py | awk '{print $2}' | xargs kill >/dev/null 2>&1")

def main():

    if len(sys.argv) <= 1:
        print 'usage: run_server.py [start|stop|restart]'
        exit(2)
    opt = sys.argv[1]
    if opt == 'start':
        start()
    elif opt == 'stop':
        stop()
    elif opt == 'restart':
        stop()
        start()
    else:
        print 'usage: run_server.py [start|stop|restart]'
        exit(2)


if __name__ == '__main__':

    main()
