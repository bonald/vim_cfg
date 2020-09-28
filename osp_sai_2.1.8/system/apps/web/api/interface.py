#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import string
import base64
import portMapping

portMap = portMapping.portNameMapping()

def get(key):
    """
    API:

    GET:

    Retrun: {
        arr: [
            tbl_interface
            ]
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
        'interface is not exist',         # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    
    if key == '' or key == 'all':
        arr = vcmd.get_arr('cdbctl read/cdb/interface')
        if arr:
            # process split port, assert a 'split_type' field
            for item in arr:
                
                if item['key'].startswith('eth-'):
                    item['split_type'] = 'splited' if ( item['key'].find('_') != -1 )else 'not_split'
                else:
                    item['split_type'] = 'not_support'

                if item['brgif'] and item['brgif']['allowed_vlan']: 

                    # change "all5-10" to 'ALL', if exist keyword 'all', we ignore other when display     
                    item['brgif']['allowed_vlan'] = 'ALL' if (item['brgif']['allowed_vlan'].find('all') == 0) else item['brgif']['allowed_vlan']

                item['sort_name'] = portMap.Mapping(item['key'])
                item['full_name'] = portMap.Mapping(item['key'], is_sort=False)
                item['svlan_tpid_mode'] = '0x%x' % int(item['svlan_tpid'])
            # process split port end
            
            obj['arr'] = arr
        else:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = _err_reason[1]
    else:
        arr = vcmd.get_lines('cdbctl read/cdb/interface/%s' % key)
        if arr[0].find('%') >= 0:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = arr
        else:
            arr = vcmd.get_arr('cdbctl read/cdb/interface/%s' % key)
            if arr:
                obj['arr'] = arr
            else:
                obj['error'] = True
                obj['err_code'] = 1
                obj['err_reason'] = _err_reason[1]

    return obj

def modify(req_data):
    """
    API:

    PUT: {/*tbl_interface*/
        key : key,
        desc : description,
        speed: 10G,
        up : 0,

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
        'cannot set interface', # err_code: 2
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # param check
    interface = str(req_data.get('key'))
    if not interface:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj

    operate = req_data.get('operate')

    if operate == 'description':
        desc = req_data.get('desc')
        if desc:
            desc = str(desc)
            if desc:
                cmd = 'cdbctl update/cdb/interface/%s/desc/%s' % (interface, desc)
                status, lines = vcmd.get_status_lines(cmd)
                if status != 0:
                    obj['error'] = True
                    obj['err_code'] = 2
                    obj['err_reason'] = '; '.join(lines)
                    return obj
        else :
            cmd = 'cdbctl update/cdb/interface/%s/desc' % (interface)
            status, lines = vcmd.get_status_lines(cmd)
            if status != 0:
                obj['error'] = True
                obj['err_code'] = 2
                obj['err_reason'] = '; '.join(lines)
                return obj

    if operate == 'speed':    
        speed = str(req_data.get('speed'))
        if speed:
            cmd = 'cdbctl update/cdb/interface/%s/speed/%s' % (interface, speed)
            status, lines = vcmd.get_status_lines(cmd)
            if status != 0:
                obj['error'] = True
                obj['err_code'] = 2
                obj['err_reason'] = '; '.join(lines)
                return obj

    if operate == 'duplex':    
        duplex = str(req_data.get('duplex'))
        if duplex:
            cmd = 'cdbctl update/cdb/interface/%s/duplex/%s' % (interface, duplex)
            status, lines = vcmd.get_status_lines(cmd)
            if status != 0:
                obj['error'] = True
                obj['err_code'] = 2
                obj['err_reason'] = '; '.join(lines)
                return obj

    if operate == 'svlan_tpid':
        svlan_tpid = str(req_data.get('svlan_tpid'))
        if svlan_tpid:
            cmd = 'cdbctl update/cdb/interface/%s/svlan_tpid/%s' % (interface, svlan_tpid)
            status, lines = vcmd.get_status_lines(cmd)
            if status != 0:
                obj['error'] = True
                obj['err_code'] = 2
                obj['err_reason'] = '; '.join(lines)
                return obj

    if operate == 'jumboframe_en':              
        jumboframe_en = str(req_data.get('jumboframe_en'))

        if jumboframe_en:
            if jumboframe_en == 'Enable':
                cmd = 'cdbctl update/cdb/interface/%s/jumboframe_en/1' % (interface)
                status, lines = vcmd.get_status_lines(cmd)
            elif jumboframe_en == 'Disable':
                cmd = 'cdbctl update/cdb/interface/%s/jumboframe_en/0' % (interface)
                status, lines = vcmd.get_status_lines(cmd)
            else:
                obj['error'] = True
                obj['err_code'] = 2
                obj['err_reason'] = '% set interface jumboframe_en need set enable or disable'
                return obj
            if status != 0:
                obj['error'] = True
                obj['err_code'] = 2
                obj['err_reason'] = '; '.join(lines)
                return obj

    if operate == 'mode':             
        mode = str(req_data.get('mode'))

        if mode:
            if mode == 'switch':               
                cmd = 'cdbctl update/cdb/interface/%s/mode/switch' % (interface)
                status, lines = vcmd.get_status_lines(cmd)
            elif mode == 'routed':
                cmd = 'cdbctl update/cdb/interface/%s/mode/routed' % (interface)
                status, lines = vcmd.get_status_lines(cmd)
            else:
                obj['error'] = True
                obj['err_code'] = 2
                obj['err_reason'] = '% set interface mode need set value switch or routed'
                return obj
            if status != 0 :
                obj['error'] = True
                obj['err_code'] = status
                obj['err_reason'] = '; '.join(lines)
                return obj

    if operate == 'vlan_type': 
        vlan_type = str(req_data.get('vlan_type'))
        if vlan_type:
            if vlan_type == 'access':
                cmd = 'cdbctl update/cdb/interface/%s/vlan_type/access' % (interface)
                status, lines = vcmd.get_status_lines(cmd)
            elif vlan_type == 'trunk':
                cmd = 'cdbctl update/cdb/interface/%s/vlan_type/trunk' % (interface)
                status, lines = vcmd.get_status_lines(cmd)
            else:
                obj['error'] = True
                obj['err_code'] = 2
                obj['err_reason'] = '% set interface vlan type need select access or trunk'
                return obj
            if status !=0:
                obj['error'] = True
                obj['err_code'] = 2
                obj['err_reason'] = '; '.join(lines)
                return obj

    if operate == 'aggregatePort': 
        # interface must 'eth-0-x_x' format
        if interface.find('_') == -1:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = 'Cannot split splited interface'
            return obj
        else:
            port, _ = interface.split('_')
        cmd = 'cdbctl update/cdb/sys/port/%s/split_type/none' % (port)
        status, lines = vcmd.get_status_lines(cmd)

        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(lines)
            return obj

    if operate == 'undoAggregatePort': 
        # interface must 'eth-0-x_x' format
        if interface.find('_') == -1:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = 'Cannot split splited interface'
            return obj
        else:
            port, _ = interface.split('_')
        # 10giga/40giga depend on board type. only support 10giga now
        cmd = 'cdbctl update/cdb/sys/port/%s/split_type/10giga' % (port)
        status, lines = vcmd.get_status_lines(cmd)
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(lines)
            return obj

    if operate == 'splitPort': 
        split_type = str(req_data.get('split_type'))
        cmd = 'cdbctl update/cdb/sys/port/%s/split_type/%s' % (interface, split_type)
        status, lines = vcmd.get_status_lines(cmd)

        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(lines)
            return obj

    if operate == 'undoSplitPort': 
        split_type = str(req_data.get('split_type'))
        cmd = 'cdbctl update/cdb/sys/port/%s/split_type/none' % (interface)
        status, lines = vcmd.get_status_lines(cmd)
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(lines)
            return obj

    if operate == 'switchPortType': 
        switch_type = str(req_data.get('switchPortType'))
        cmd = 'cdbctl update/cdb/sys/port/eth-0-1/port_media_type/%s' % (switch_type)
        status, lines = vcmd.get_status_lines(cmd)
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(lines)
            return obj

    return obj


def set_port_status(if_key, status):
    """
    Param: 
        if_key: eth-x-x
        status: True - no shutdown, False: shutdown

    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                         # err_code: 0
        'cmd excute error',         # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    
    cmd = 'cdbctl update/cdb/interface/%s/up/%d' % (if_key, 1 if status else 0)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj

def set_port_flow(req_data):
    """
    Param: 
        if_key: eth-x-x
        status: True - no shutdown, False: shutdown

    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                         # err_code: 0
        'cmd excute error',         # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    interface = str(req_data.get('key'))
    portFlowEn = str(req_data.get('portFlowEn'))
    portFlowName = str(req_data.get('portFlowName'))

    name = interface.replace('_', '/')

    if portFlowEn == 'False':
        cmd = 'cdbctl update/cdb/interface/%s/egs_acl/' % (name)
    else:
        cmd = 'cdbctl update/cdb/interface/%s/egs_acl/%s' % (name, portFlowName)

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj
    
    return obj

def get_port_split_status():
    """
    TBD
    """
    _err_reason = [
        '',                    # err_code: 0
        'interface is not exist',         # err_code: 1
    ]

    obj = {'arr': [], 'port_media_type':'', 'error': False, 'err_code': 0, 'err_reason': ''}
    arr = vcmd.get_arr('cdbctl show/cdb/sys/port/')

    split_arr = {};
    connection = portMap.getPortConnection()

    if arr:
        for item in arr:
            # 1. media type property only support 1 port eth-0-1 only in GG chip.
            if (item['key'] == '0'):
                obj['port_media_type'] = item['port_media_type'] 
            # 2. split_type              
            if -1 != item['name'].find(connection['sub_connection']):
                item['is_split'] = 'True'
            else:
                item['is_split'] = 'False'
            split_arr[item['name']] = {'split_type':item['split_type'],'is_split':item['is_split']}
        obj['arr'] = split_arr
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]

    return obj

if __name__ == '__main__':
    pass

