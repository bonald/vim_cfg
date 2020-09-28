#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 3, 2016

@author: wangqj
'''

import base
import re
from vcl import vcmd

def get():
    """
    API:
    
    GET:
    
    Retrun: {
        ip: string, eg: '10.10.10.2/24'
        gateway: string, eg: '10.10.10.1'
    
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'cmd excute error', # err_code: 1
    ]
    
    obj = {'ip': '', 'gateway': '', 'error': False, 'err_code': 0, 'err_reason': ''}
    
    status, arr = vcmd.get_status_arr('cdbctl read/cdb/sys/manage_if')
            
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ''.join(arr)
    
    else:
        obj['ip'] = arr[0].get('addr').replace('#', '/')
        obj['gateway'] = arr[0].get('gateway')
    
    return obj


def update_ip(ip_addr):
    """
    API:
    
    PUT: /ip/<base64_ip_addr>/
        base64_ip_addr e.g.: '10.10.10.2/24' -> 'MTAuMTAuMTAuMi8yNA==\n'
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'modify mgmt ip address failed', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    if ip_addr == None or not re.match(r'^((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d))))\/((\d$)|([0-2]\d$))|(3[0-2]$)', ip_addr):
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    # update
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/sys/manage_if/addr/%s' % ip_addr)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj



def update_gateway(gateway):
    """
    API:
    
    PUT: /gateway/<base64_gateway>/
        base64_gateway e.g.: '10.10.10.2/24' -> 'MTAuMTAuMTAuMg==\n'
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'modify gateway ip address failed', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    if gateway == None or not re.match(r'^((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d))))$', gateway):
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/sys/manage_if/gateway/%s' % gateway)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj
