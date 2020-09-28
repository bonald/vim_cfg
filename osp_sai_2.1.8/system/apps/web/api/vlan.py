#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import base

def status():
    """
    API:
    
    GET:
    
    Retrun: {
        vlan_arr: [
            {
                id: int,
                member_port: int or range,
                name: string
            },
            ...
            ...
        ],
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                    # err_code: 0
        'vlan status is null', # err_code: 1
    ]
    
    obj = {'vlan_arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    
    arr = vcmd.get_arr_simple('cdbctl read/cdb/l2/vlan')
    if arr:
        for i in arr:
            tmp = {}
            tmp['id']             = base.parse.parse_int(i.get('key'), 0)
#            tmp['status']         = ('active' if i['enable'] else 'inactive') if i.get('enable') else None
#            tmp['mac_learing_en'] = ('enable' if i['mac_learning_en'] == 1 else 'disable') if i.get('mac_learning_en') else None
#            tmp['max_mac_cnt']    = i.get('max_mac_count')
            tmp['member_port']    = i.get('member_port')
            tmp['name']           = i.get('name')
            
            obj['vlan_arr'].append(tmp)
            
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
    
    return obj

def delete(req_data):
    """
    API:
    
    DELETE: {
        vlan_ids: [int, int, ....]
    }
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    """
    
    _err_reason = [
        '',                     # err_code: 0
        'bad request',          # err_code: 1
        'excute error',         # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    res, err = base.check.dictionary(req_data, [
        ['vlan_ids', 'array', 'bad request: vlan_ids', 1],
    ])
    if not res:
        obj['error'] = True
        obj['err_code'] = err['err_code'];
        obj['err_reason'] = err['err_reason'];
        return obj;
    
    vlan_ids, err = base.check.array(res['vlan_ids'], ['int', '>=', 2, 'and', '<=', 4095, 'vlan_id is not in [2,4095]', 1])
    if not vlan_ids:
        obj['error'] = True
        obj['err_code'] = err['err_code'];
        obj['err_reason'] = err['err_reason'];
        return obj;
    
    status, lines = vcmd.get_status_lines(['cdbctl delete/cdb/l2/vlan/%d' % x for x in vlan_ids])
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2;
        obj['err_reason'] = '; '.join(lines);

    return obj

def add(req_data):
    """
    API:
    
    POST: {
        id: vlan_id,
        name: vlan_name,
    }
    
    Return: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    """
    
    _err_reason = [
        '',                     # err_code: 0
        'bad request',          # err_code: 1
        'cannot create vlan',   # err_code: 2
        'cannot set vlan name', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    res, err = base.check.dictionary(req_data, [
        ['id', 'int', '>=', 1, 'and', '<=', 4095, 'bad request: id', 1],
        #['name', 'string', 're', r'^[a-zA-Z][a-zA-Z\.\-\_0-9]{0,15}$', 'bad request: name', 1],
    ])
    if not res:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj
    
    vlan_id = res['id']
    #vlan_name = res['name']
    vlan_name = str(req_data.get('name'))
    #print vlan_name
    if vlan_name:
        res, err = base.check.dictionary(req_data, [
            ['id', 'int', '>=', 1, 'and', '<=', 4095, 'bad request: id', 1],
            ['name', 'string', 're', r'^[a-zA-Z][a-zA-Z\.\-\_0-9]{0,15}$', 'illegality name', 1],
        ])
        if not res:
            obj['error'] = True
            obj['err_code'] = err['err_code']
            obj['err_reason'] = err['err_reason']
            return obj
    
    # add new vlan
    res = vcmd.execute('cdbctl create/cdb/l2/vlan/%d' % vlan_id)
    if res != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2]
        return obj
    
    # modify vlan name
    if vlan_name:
        res = vcmd.execute('cdbctl update/cdb/l2/vlan/%d/name/%s' % (vlan_id, vlan_name))
        if res != 0:
            obj['error'] = True
            obj['err_code'] = 3
            obj['err_reason'] = _err_reason[3]
            return obj
    
    return obj
    
    
def modify(req_data):
    """
    API:
    
    PUT: {
        id: vlan_id,
        name: vlan_name,
    }
    
    Return: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    """
    
    _err_reason = [
        '',                     # err_code: 0
        'bad request param',    # err_code: 1
        'cannot set vlan name', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    res, err = base.check.dictionary(req_data, [
        ['id', 'int', '>=', 1, 'and', '<=', 4095, 'bad request: id', 1],
        ['name', 'string', 're', r'^[a-zA-Z][a-zA-Z\.\-\_0-9]{0,15}$', 'bad request: name', 1],
    ])
    if not res:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj
    
    vlan_id = res['id']
    vlan_name = res['name']
    
    # modify vlan name
    res = vcmd.execute('cdbctl update/cdb/l2/vlan/%d/name/%s' % (vlan_id, vlan_name))
    if res != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2]
        return obj
    
    return obj    
    
    
def access_update(req_data):
    """
    API:
    
    PUT: {
        vlan_id: vlan id,
        interface_arr: [
            eth-0-1,
            eth-0-2,
            ......
        ],
    }
    
    Return: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                     # err_code: 0
        'bad request',          # err_code: 1
        'execute error'         # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    res1, err1 = base.check.dictionary(req_data, [
        ['vlan_id', 'int', '>=', '1', 'or', '<=', '4095', 'bad request: vlan_id', 1],
        ['interface_arr', 'array', 'bad request: interface_arr', 1]
    ])
    res2, err2 = base.check.array(req_data.get('interface_arr'), ['string', 're', r'^[a-zA-Z0-9\-_]{3,15}$', 'bad request: interface_arr', 1], True)
    
    if not res1 or not res2:
        obj['error'] = True
        err_ref = err1 if not res1 else err2
        obj['err_code'] = err_ref['err_code']
        obj['err_reason'] = err_ref['err_reason']
        return obj
    
    # modify pvid
    cmd_arr = ['cdbctl update/cdb/interface/%s/brgif/pvid/%d/access' % (eth, res1['vlan_id']) for eth in res2]
    status, lines = vcmd.get_status_lines(cmd_arr)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj    

def trunk_update(req_data):
    """
    API:
    
    PUT: {
        vlan_id: int,
        permit_vlan: vlan string,
        interface_arr: [
            eth-0-1,
            eth-0-2,
            ......
        ],
    }
    
    Return: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    """
    
    _err_reason = [
        '',                     # err_code: 0
        'bad request',          # err_code: 1
        'modify pvid error'     # err_code: 2
        'add permit vlan error' # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    res1, err1 = base.check.dictionary(req_data, [
        ['vlan_id', 'int', '>=', '1', 'or', '<=', '4095', 'bad request: vlan_id', 1],
        ['permit_vlan', 'string', 're', r'^[0-9\-,]+$', 'bad request: permit_vlan', 1],
        ['interface_arr', 'array', 'bad request: interface_arr', 1]
    ])
    res2, err2 = base.check.array(req_data.get('interface_arr'), ['string', 're', r'^[a-zA-Z0-9\-_]{3,15}$', 'bad request: interface_arr', 1], False)
    
    if not res1 or not res2:
        obj['error'] = True
        err_ref = err1 if not res1 else err2
        obj['err_code'] = err_ref['err_code']
        obj['err_reason'] = err_ref['err_reason']
        return obj
    
    # modify pvid
    cmd_arr = ['cdbctl update/cdb/interface/%s/brgif/pvid/%d/trunk' % (eth, res1['vlan_id']) for eth in res2]
    status, lines = vcmd.get_status_lines(cmd_arr)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    # modify permit vlan
    cmd_arr = ['cdbctl update/cdb/interface/%s/brgif/allowed_vlan/trunk/none&&cdbctl update/cdb/interface/%s/brgif/allowed_vlan/trunk/add/%s' % (eth, eth, res1['permit_vlan']) for eth in res2]
    status, lines = vcmd.get_status_lines(cmd_arr)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj    


def range_add(req_data):
    """
    API:
    
    POST: {
        range: vlan range string,
    }
    
    Return: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    """
    _err_reason = [
        '',                     # err_code: 0
        'bad request',          # err_code: 1
        'cannot create vlan',   # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    res, err = base.check.dictionary(req_data, [
        ['range', 'string', 're', r'^((\d{1,4}(\-\d{1,4})?),)*(\d{1,4}(\-\d{1,4})?)$', 'bad request: range', 1],
    ])
    if not res:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj
    
    range_str = res['range']
    cmd = 'cdbctl create/cdb/l2/vlanrange/%s' % range_str
    res = vcmd.execute(cmd)
    if res != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2]
        return obj
    
    return obj

def range_delete(req_data):
    """
    API:
    
    POST: {
        range: vlan range string,
    }
    
    Return: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    """
    _err_reason = [
        '',                     # err_code: 0
        'bad request',          # err_code: 1
        'cannot delete vlan',   # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    res, err = base.check.dictionary(req_data, [
        ['range', 'string', 're', r'^((\d{1,4}(\-\d{1,4})?),)*(\d{1,4}(\-\d{1,4})?)$', 'bad request: range', 1],
    ])
    if not res:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj
    
    range_str = res['range']
    cmd = 'cdbctl delete/cdb/l2/vlanrange/%s' % range_str
    res = vcmd.execute(cmd)
    if res != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2]
        return obj
    
    return obj   
