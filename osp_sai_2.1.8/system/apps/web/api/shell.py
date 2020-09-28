#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''

import base64
from vcl import vcmd

def get(cmd_base64):
    """
    API:
    
    GET: /<cmd>/
    
    Retrun: {
        resp: {
            status: int,
            lines: array,
        }
        error: bool,
        err_code: int,
        err_reason: string
    }
    """    
    
    _err_reason = [
        '',                    # err_code: 0
    ]

    obj = {'resp': {'status': 0, 'lines': ['']}, 'error': False, 'err_code': 0, 'err_reason': ''}
    
    cmd = base64.decodestring(cmd_base64)
    
    obj['resp']['status'], obj['resp']['lines'] = vcmd.get_status_lines(cmd)
    
    return obj



