#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on July 5, 2016

@author: huwx
'''

from vcl import vcmd
import base
import types

def _get_arp_table():
    
    status, lines = vcmd.get_status_lines('cdbctl read/cdb/l3/show_arp/ | tail -n +3 | awk \'{printf"%s@%s@%s@%s@%s\\n", $1, $2, $3, $4, $5}\'')
    if status != 0:
        return None
    else:
        arr = []
        for line in lines:
            tmp = line.split('@')
            if len(tmp) != 5:
                continue
            
            arr.append({
                'protocol': tmp[0],
                'ip_address': tmp[1],
                'age_time': tmp[2],
                'mac_address': tmp[3],
                'port': tmp[4],
            })
            #print arr
        return arr

def get():
    """
    API:
    
    GET:
    
    Retrun: {
        arp_table: [
            {protocol: string, ip_address: string, age_time: string, mac_address: string, port: string},
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
    
    obj = {'arp_table': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    
    res = _get_arp_table()
    #print res
    if res == None:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    # success
    obj['arp_table'] = res
    #print obj
    return obj 

def gratuitous_arp_get():
    """
    API:
        
    Retrun: {
        gratuitous_arp_learn_en:1 or 0
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
    
    # excute

    status, arr = vcmd.get_status_arr('cdbctl read/cdb/l3/route_global')
        
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj
    
    else:
        obj['gratuitous_arp_learn_en']= arr[0].get('gratuitous_arp_learn_en')
    #print obj
    return obj

def gratuitous_arp_set(req_data):
    """
    API:
    
    PUT: {
        gratuitous_arp : enable,
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
    enable = int(req_data.get('gratuitous_arp'))
    #print enable
    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l3/route_global/gratuitous_arp_learn_en/%d' % int(enable))
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj
    
     

def delete(del_arr):
    """
    API:
    
    DELETE: {

        {ip_address: string}

    }
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    _err_reason = [
        '',                 # err_code: 0
        'request is empty',  # err_code: 1
        'bad request',      # err_code: 2
        'cmd excute error', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # empty
    #print del_arr
    if not del_arr:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj

    
    ip_address = str(del_arr.get('ip_address'))
    cmd = 'cdbctl delete/cdb/l3/arp_fib/%s' % (ip_address)
    # bad request
    if not cmd:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2]
        return obj

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj



def add(new_arp):
    """
    API:
    
    POST: {
        ip_address: string,
        mac_address: string
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
    
    res, err = base.check.dictionary(new_arp, [
        ['ip_address', 'string', 'bad_request: ip address', 1],
        ['mac_address', 'string', 're', r'^([\da-f]{1,4}\.){2}[\da-f]{1,4}$', 'bad_request: mac_address', 1],
    ])
    
    #print new_arp
    # error
    if res == None:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj
        
    # add fdb
    cmd = 'cdbctl create/cdb/l3/arp_fib/%s/%s' % (res['ip_address'], res['mac_address'])
    status, lines = vcmd.get_status_lines(cmd)
    # error
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
        
    return obj


def clear():
    """
    API:

    CLAER:

    Return: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    _err_reason = [
        '',                    # err_code: 0
        'arp cache clear failed',         # err_code: 1
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l3/flush_arp')
    vcmd.execute('sleep 1')
    if status != 0:
        obj['error'] = True
        obj['err_code'] = status
        obj['err_reason'] = _err_reason[1]
        return obj

    return obj
    

