#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Apr 25, 2016

@author: wangqj
'''

import base
from vcl import vcmd

def get():
    """
    API:
    
    GET: 
    
    Retrun: {
        flash: {
            size: string(eg: 700M),
            used: string(eg: 500M),
            avail: string(eg: 200M),
            used_p: string(eg: 20%),
        },
        
        boot: {
            size: string(eg: 700M),
            used: string(eg: 500M),
            avail: string(eg: 200M),
            used_p: int(eg: 20 means 20%),
        }
        
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    _err_reason = [
        '',              # err_code: 0
        'excute error',  # err_code: 3
    ]
    
    obj = {'flash': {}, 'boot': {}, 'error': False, 'err_code': 0, 'err_reason': ''}
    
    # get lines
    status, lines = vcmd.get_status_lines('df -h | awk \'{print $2, $3, $4, $5, $6}\'')
    if status != 0 or len(lines) <= 1:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    # make dict
    ptr = None
    for line in lines[1:]:
        tmp = line.split(' ')
        if tmp[4] == '/mnt/flash':
            ptr = obj['flash']
        elif tmp[4] == '/mnt/flash/boot':
            ptr = obj['boot']
        else:
            continue
        ptr['size'] = tmp[0]
        ptr['used'] = tmp[1]
        ptr['avail'] = tmp[2]
        ptr['used_p'] = base.parse.parse_int(tmp[3])
    
    return obj



