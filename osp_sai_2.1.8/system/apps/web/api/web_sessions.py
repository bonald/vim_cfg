#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import url_for
import os
import base
from vcl import vcmd
import types

def get(file_type):
    """
    API:
    
    GET:
    
    Retrun: {
        sess: [
            {id: string, user: string, expire: string, client: string,},
            ...
        ],
    }
    """
    
    # get web sessions
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    res = []
    
    # get cmd lines
    cmd = 'cdbctl read/cdb/app/login/web | grep web';
    lines = vcmd.get_lines(cmd)
    if not lines:
        return res
    
    # make dict
    for line in lines:
        key = vcmd.cmd_get_token(line, "key")
        user = vcmd.cmd_get_token(line, "user")
        deccmd = 'fnconvert -c decoding -m "%s"'%(user)
        decuser = vcmd.get_lines(deccmd)
        ipaddr = vcmd.cmd_get_token(line, "ipaddr")
        etime = base.relative_time.get(int(vcmd.cmd_get_token(line, "expire_time")))
        res.append({
            'id': key,
            'user': decuser[0],
            'expire': etime,
            'client': ipaddr,
        });
    
    obj['sess'] = res
    return obj

def delete(req_data):
    """
    API:
    
    DELETE: {
       file_arr: [
            {
               'id': string,
            },
            ...
            ...
       ],
    }
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    """
    
    _err_reason = [
        '',              # err_code: 0
        'bad request',   # err_code: 1
        'delete failed', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    sess_arr = req_data.get('sess_arr')
    if not type(sess_arr) is types.ListType: # bad request
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
        
    # make cmd
    for sid in sess_arr:
        exec_str = 'cdbctl delete/cdb/app/login/%s' %(str(sid));
        status, output = vcmd.get_status_lines(exec_str);
    
    return obj
