#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Apr 27, 2016

@author: wangqj
'''

import random
from vcl import vcmd

def update():
    """
    API:
    
    PUT:
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'build cfg file failed', # err_code: 1
        'copy cfg file failed', # err_code: 2
        'remove tmp cfg file failed', # err_code: 3
        'send trap failed', # err_code: 4
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # build config file
    tmp_filename = 'config-%d' % random.randint(1, 100000)
    status = vcmd.execute('cdbctl buildcfg/startup/%s/cdb' % tmp_filename)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    # copy config file
    status = vcmd.execute('cp /tmp/%s /mnt/flash/startup-config.conf' % tmp_filename)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2]
        return obj
        
    # copy config file
    status = vcmd.execute('rm /tmp/%s' % tmp_filename)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = _err_reason[3]
        return obj
    
    # snmp
    status = vcmd.execute('cdbctl update/cdb/sys/send_trap/write_success')
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 4
        obj['err_reason'] = _err_reason[4]
        return obj
    
    return obj