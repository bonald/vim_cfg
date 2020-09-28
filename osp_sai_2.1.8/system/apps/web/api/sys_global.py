#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 3, 2016

@author: wangqj
'''

import base
from vcl import vcmd

def get():
    """
    API:
    
    GET:
    
    Retrun: {
        hostname: string
    
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'cmd excute error', # err_code: 1
    ]
    
    obj = {'hostname': '', 'error': False, 'err_code': 0, 'err_reason': ''}
    
    status, arr = vcmd.get_status_arr('cdbctl read/cdb/sys/sys_global/hostname/')
            
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ''.join(arr)
    
    else:
        obj['hostname']= arr[0].get('hostname')
    
    return obj



def update(req_data):
    """
    API:
    
    PUT: {
        hostname: string[1-16]
    }
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    res, err = base.check.dictionary(req_data, [
        ['hostname', 'string', 're', r'^[a-zA-Z][a-zA-Z0-9\.\-_]{0,63}$', 'bad request: hostname', 1]
    ])
    
    if not res:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj
    
    # update
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/sys/sys_global/hostname/%s' % res['hostname'])
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj

