#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on July 14, 2016

@author: huwx
'''

from vcl import vcmd
import base
import types

def _get_worm_filter():
    
    status, lines = vcmd.get_status_lines('cdbctl show/cdb/l2/show_acl_worm_filter_counter | awk \'{printf"%s@%s@%s@%s@%s\\n", $2, $3, $4, $7, $9}\'')
    if status != 0:
        return None
    else:
        arr = []
        for line in lines:
            #print line
            tmp = line.split('@')
            if len(tmp) != 5:
                continue
            
            arr.append({
                'filtername': tmp[0],
                'protocol': tmp[1],
                'dstport': tmp[2],
                'bytes': tmp[3],
                'packets': tmp[4],
            })
            #print arr
        return arr

def get():
    """
    API:
    
    GET:
    
    Retrun: {
        worm_filter: [
            {filtername: string, protocol: string, dstport: string, bytes: string, packets: string},
            ...
            ...
        ],
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'cmd excute error', # err_code: 1
    ]
    
    obj = {'worm_filter': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    
    res = _get_worm_filter()
    #print res
    if res == None:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    # success
    obj['worm_filter'] = res
    #print obj
    return obj 

     

def delete(del_arr):
    """
    API:
    
    DELETE: {

        {filtername: string}

    }
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    _err_reason = [
        '',                 # err_code: 0
        'request is empty'  # err_code: 1
        'bad request',      # err_code: 2
        'cmd excute error', # err_code: 3
        'delete fail',
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # empty
    #print del_arr
    if not del_arr:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj

    
    filtername = str(del_arr.get('filtername'))
    cmd = 'cdbctl delete/cdb/l2/acl_worm_filter/%s' % (filtername)
    # bad request
    if not cmd:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2]
        return obj

    status, lines = vcmd.get_status_lines(cmd)
    #print cmd
    if status != 0:
        #print status
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = _err_reason[3]
        return obj

    return obj


import time
def add(new_filter):
    """
    API:
    
    POST: {
        filtername: string,
        protocol: string,
        dstport: string,
        isrange: string
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

    #time.sleep(1)
    #print '*' * 100
    #print new_filter
    
    res, err = base.check.dictionary(new_filter, [
        ['filtername', 'string', 'bad_request: filtername', 1],
        ['protocol', 'string', 'in', ['tcp', 'udp'], 'bad_request: protocol', 1],
        ['dstport', 'string', 'bad_request: dstport', 1],
    ])
    
    is_range = new_filter.get('isrange')
    is_range = is_range if is_range != None else ''
    
    #print '-' * 100
    #print res, err
    
    #print new_filter
    # error
    if res == None:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj
        
    # add filter
    if is_range == '':
        if res['protocol'] == 'tcp':
            cmd = 'cdbctl create/cdb/l2/acl_worm_filter/%s/1/%s/0' % (res['filtername'], res['dstport'])
        else :
            cmd = 'cdbctl create/cdb/l2/acl_worm_filter/%s/0/%s/0' % (res['filtername'], res['dstport'])
    else:
        if res['protocol'] == 'tcp':
            cmd = 'cdbctl create/cdb/l2/acl_worm_filter/%s/1/%s/%s' % (res['filtername'], res['dstport'], is_range)
        else :
            cmd = 'cdbctl create/cdb/l2/acl_worm_filter/%s/0/%s/%s' % (res['filtername'], res['dstport'], is_range)
        
    #print cmd
    status, lines = vcmd.get_status_lines(cmd)
    # error
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
        
    return obj




