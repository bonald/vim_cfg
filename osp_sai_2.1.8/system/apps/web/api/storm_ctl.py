#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import string
import base64
from base import log
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
            for item in arr:
                item['sort_name'] = portMap.Mapping(item['key'])
                item['full_name'] = portMap.Mapping(item['key'], is_sort=False)
            obj['arr'] = arr
        else:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = _err_reason[1]
    else:
        arr = vcmd.get_lines('cdbctl read/cdb/interface/%s' % key)
        if arr[0].find('%') >= 0:
            for item in arr:
                item['sort_name'] = portMap.Mapping(item['key'])
                item['full_name'] = portMap.Mapping(item['key'], is_sort=False)
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = arr
        else:
            arr = vcmd.get_arr('cdbctl read/cdb/interface/%s' % key)
            if arr:
                for item in arr:
                    item['sort_name'] = portMap.Mapping(item['key'])
                    item['full_name'] = portMap.Mapping(item['key'], is_sort=False)
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
        'not support on this product',  # err_code: 2
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    interface = str(req_data.get('key'))
    if not interface:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    cmdstr="ls /switch/etc/cmd -l | grep 'storm-control'"
    status, lines = vcmd.get_status_lines(cmdstr)
    if lines[0]=='':
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = _err_reason[3]
        return obj
    print(lines)
    ucast_mode = str(req_data.get('ucast_mode'))
    ucast_rate = str(req_data.get('ucast_rate'))
    type = str(req_data.get('type'))    
    if type == 'Ucast':
        if ucast_mode == '1':  ##PPS
            ## check rate [0, 1000000000]
            cmd = 'cdbctl update/cdb/interface/%s/storm_control/set/unicast/pps/%s' % (interface, ucast_rate)
            status, lines = vcmd.get_status_lines(cmd)
        elif ucast_mode == '2':  ##Level
            ## check rate [0.00, 100.00]
            cmd = 'cdbctl update/cdb/interface/%s/storm_control/set/unicast/level/%s' % (interface, ucast_rate)
            status, lines = vcmd.get_status_lines(cmd)
        elif ucast_mode == '0':
            cmd = 'cdbctl update/cdb/interface/%s/storm_control/unset/unicast' % (interface)
            status, lines = vcmd.get_status_lines(cmd)
        else:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '% set unicast mode must be pps or level'
            return obj            
        
        if status != 0:
            obj['error'] = True
            obj['err_code'] = status
            obj['err_reason'] = '; '.join(lines)
            return obj

    bcast_mode = str(req_data.get('bcast_mode'))
    bcast_rate = str(req_data.get('bcast_rate'))
    if type == 'Bcast':
        if bcast_mode == '1':
            ## check rate [0, 1000000000]
            cmd = 'cdbctl update/cdb/interface/%s/storm_control/set/broadcast/pps/%s' % (interface, bcast_rate)
            status, lines = vcmd.get_status_lines(cmd)
        elif bcast_mode == '2':
            ## check rate [0.00, 100.00]
            cmd = 'cdbctl update/cdb/interface/%s/storm_control/set/broadcast/level/%s' % (interface, bcast_rate)
            status, lines = vcmd.get_status_lines(cmd)
        elif bcast_mode == '0':
            cmd = 'cdbctl update/cdb/interface/%s/storm_control/unset/broadcast' % (interface)
            status, lines = vcmd.get_status_lines(cmd)
        else:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '% set broadcast mode must be pps or level'
            return obj   
        if status != 0:
            obj['error'] = True
            obj['err_code'] = status
            obj['err_reason'] = '; '.join(lines)
            return obj            

    mcast_mode = str(req_data.get('mcast_mode'))
    mcast_rate = str(req_data.get('mcast_rate'))
    if type == 'Mcast':
        if mcast_mode == '1':
            cmd = 'cdbctl update/cdb/interface/%s/storm_control/set/multicast/pps/%s' % (interface, mcast_rate)
            status, lines = vcmd.get_status_lines(cmd)
        elif mcast_mode == '2':
            ## check rate [0.00, 100.00]
            cmd = 'cdbctl update/cdb/interface/%s/storm_control/set/multicast/level/%s' % (interface, mcast_rate)
            status, lines = vcmd.get_status_lines(cmd)
        elif mcast_mode == '0':
            cmd = 'cdbctl update/cdb/interface/%s/storm_control/unset/multicast' % (interface)
            status, lines = vcmd.get_status_lines(cmd)
        else:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '% set multicast mode must be pps or level'
            return obj
        if status != 0:
            obj['error'] = True
            obj['err_code'] = status
            obj['err_reason'] = '; '.join(lines)
            return obj
            
    return obj

if __name__ == '__main__':
    pass

