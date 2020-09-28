#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Apr 28, 2016

@author: wangqj
'''

from vcl import vcmd
import base
import types

def _get_mac_address_table():
    
    status, lines = vcmd.get_status_lines('cdbctl read/cdb/l2/show_fdb/all | grep \'^[0-9]\' | awk \'{printf"%s@%s@%s@%s\\n", $1, $2, $3, $4}\'')
    if status != 0:
        return None
    else:
        arr = []
        for line in lines:
            tmp = line.split('@')
            if len(tmp) != 4:
                continue
            
            arr.append({
                'vlan_id': base.parse.parse_int(tmp[0], -1),
                'mac_address': tmp[1],
                'type': tmp[2],
                'port': tmp[3],
            })
        return arr

def get():
    """
    API:
    
    GET:
    
    Retrun: {
        mac_address_table: [
            {vlan_id: int, mac_address: string, type: string, port: string},
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
    
    obj = {'mac_address_table': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    
    res = _get_mac_address_table()
    if res == None:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    # success
    obj['mac_address_table'] = res
    
    return obj 



def delete(del_arr):
    """
    API:
    
    DELETE: {
        mac_address_table: [
            {vlan_id: int, mac_address: string, type: string, port: string},
            ...
            ...
        ]
    }
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    _err_reason = [
        '',                 # err_code: 0
        'request is empty', # err_code: 1
        'bad request',      # err_code: 2
        'cmd excute error', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # empty
    if not del_arr:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj

    # delete
    cmd_arr = []
    for fdb in del_arr:
        if not fdb or type(fdb) != types.DictType:
            continue
        mac_address = str(fdb.get('mac_address'))
        vlan_id = base.parse.parse_int(fdb.get('vlan_id'))
        port = str(fdb.get('port'))
        fdbtype = str(fdb.get('type'))
        
        if not mac_address or not vlan_id or not port:
            continue
        
        if (fdbtype != 'static'):
            cmd = 'cdbctl update/cdb/action/l2_action/flush_fdb/fdb/dynamic/address/%s'%(mac_address)
        else :
            #print mac_address, vlan_id, fdbtype
            cmd = 'cdbctl delete/cdb/l2/fdb/%s#%d/interface/%s' % (mac_address, vlan_id, port)
        cmd_arr.append(cmd)
        
    # bad request
    if not cmd_arr:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2]
        return obj
    
    # exec delete
    cmd_str = ';'.join(cmd_arr)
    status, lines = vcmd.get_status_lines(cmd_str)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = ';'.join(lines) if len(lines) >= 1 else _err_reason[3]
        return obj

    return obj


def delete_all():
    """
    API:
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    _err_reason = [
        '',                 # err_code: 0
        'request is empty', # err_code: 1
        'bad request',      # err_code: 2
        'cmd excute error', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # exec delete
    cmd_static = 'cdbctl update/cdb/action/l2_action/flush_fdb/fdb/static'
    cmd_dynamic = 'cdbctl update/cdb/action/l2_action/flush_fdb/fdb/dynamic'
    
    # delete directly, dont need to care any result
    status, lines = vcmd.get_status_lines(cmd_static)
    status, lines = vcmd.get_status_lines(cmd_dynamic)

    return obj

def add(new_fdb):
    """
    API:
    
    POST: {
        vlan_id: int,
        mac_address: string,
        port: string
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
    
    res, err = base.check.dictionary(new_fdb, [
        ['vlan_id', 'int', '>=', 1, 'and', '<=', 4094, 'bad request: vlan_id', 1],
        ['mac_address', 'string', 're', r'^([\da-f]{1,4}\.){2}[\da-f]{1,4}$', 'bad_request: mac_address', 1],
        ['port', 'string', 'bad_request: port', 1],
    ])
    
    # error
    if res == None:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj
        
    # add fdb
    cmd = 'cdbctl create/cdb/l2/fdb/%s#%d/interface/%s' % (res['mac_address'], res['vlan_id'], res['port'])
    status, lines = vcmd.get_status_lines(cmd)
    # error
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
        
    return obj




