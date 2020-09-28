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
        arr = vcmd.get_arr('cdbctl read/cdb/sys/fiber/')
        if arr:
            newArr = []
            for map in arr:
                m = {}
                for key, value in map.items():
                    if key == 'interface_name':
                        m['interface_name'] = str(value)
                        m['sort_name'] = portMap.Mapping(m['interface_name'])
                        m['full_name'] = portMap.Mapping(m['interface_name'], is_sort=False)

                    if key == 'fiber_type_name':
                        m['fiber_type_name'] = str(value)

                    if key == 'name':
                        m['name'] = str(value)   

                    if key == 'pn':
                        m['pn'] = str(value) 

                    if key == 'sn':
                        m['sn'] = str(value)  
                                               
                newArr.append(m)                             
            obj['arr'] = newArr            
        else:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = _err_reason[1]
    else:
        arr = vcmd.get_lines('cdbctl read/cdb/sys/fiber/%s' % key)
        if arr[0].find('%') >= 0:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = arr
        else:
            arr = vcmd.get_arr('cdbctl read/cdb/sys/fiber/%s' % key)
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

    return obj    
if __name__ == '__main__':
    pass

