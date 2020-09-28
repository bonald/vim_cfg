#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import string

def fanGet():
    """
    API:

    GET:

    Retrun: {
        arr: [
            {
                tbl_fan;
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
        'log is null',         # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    arr = vcmd.get_arr('cdbctl read/cdb/sys/fan')
    if arr:
        obj['arr'] = arr
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
    return obj

def psuGet():
    """
    API:

    GET:

    Retrun: {
        arr: [
            {
                tbl_psu;
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
        'log is null',         # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    arr = vcmd.get_arr('cdbctl read/cdb/sys/psu')
    if arr:
        obj['arr'] = arr
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
    return obj

def sensorGet():
    """
    API:

    GET:

    Retrun: {
        arr: [
            {
                tbl_sensor;
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
        'log is null',         # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    arr = vcmd.get_arr('cdbctl read/cdb/sys/sensor')
    if arr:
        obj['arr'] = arr
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
    return obj


def cardGet():
    """
    API:

    GET:

    Retrun: {
        arr: [
            {
                tbl_card;
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
        'log is null',         # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    arr = vcmd.get_arr('cdbctl read/cdb/sys/card')
    
    if arr:
        obj['arr'] = arr
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
    return obj


def versionGet():
    """
    API:

    GET:

    Retrun: {
        arr: [
            {
                tbl_version;
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
        'log is null',         # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    arr = vcmd.get_arr('cdbctl read/cdb/sys/version')
    if arr:
        obj['arr'] = arr
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
    return obj


def bootimageGet():
    """
    API:

    GET:

    Retrun: {
        arr: [
            {
                tbl_bootimage;
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
        'log is null',         # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    arr = vcmd.get_arr('cdbctl read/cdb/sys/bootimage')
    if arr:
        obj['arr'] = arr
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
    return obj

def sysglobalGet():
     """
     API:

     GET:

     Retrun: {
         arr: [
             {
                 tbl_sys_global;
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
         'log is null',         # err_code: 1
     ]

     obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
     arr = vcmd.get_arr('cdbctl read/cdb/sys/sys_global')
     if arr:
         obj['arr'] = arr

         mgmt_mac = vcmd.get_showarr('cdbctl show/cdb/sys/show_management_if')
         if len(mgmt_mac[0]) > 1:
             for line in mgmt_mac:
                 if line.find('HWaddr') >= 0:
                     arr[0]['mgmt_routemac'] = line[line.find('HWaddr: ') + len('HWaddr: '):]
                     break

     else:
         obj['error'] = True
         obj['err_code'] = 1
         obj['err_reason'] = _err_reason[1]
     return obj

def memoryGet():
     """
     API:

     GET:

     Retrun: {
         arr: [
             {
                Total memory      : 1005808 KB
                Used memory       : 412608 KB
                Freed memory      : 593200 KB
                Buffer memory     : 0 KB
                Cached memory     : 216140 KB
                Memory utilization: 41.02%
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
         'log is null',         # err_code: 1
     ]

     obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
     arr = vcmd.get_showarr('cdbctl show/cdb/sys/mem_summary')
     tmp = {}
     if len(arr[0]) > 1:
        for line in arr:
            item = line.split(':')
            if item[0].find('Total') >= 0:
                tmp['total'] = item[1]
            if item[0].find('Used') >= 0:
                tmp['used'] = item[1]
            if item[0].find('Freed') >= 0:
                tmp['freed'] = item[1]
            if item[0].find('Buffer') >= 0:
                tmp['buffer'] = item[1]
            if item[0].find('Cached') >= 0:
                tmp['cached'] = item[1]
            if item[0].find('Memory utilization') >= 0:
                tmp['usage'] = item[1]
        obj['arr'].append(tmp)
     else:
         obj['error'] = True
         obj['err_code'] = 1
         obj['err_reason'] = _err_reason[1]
     return obj

def cpuGet():
     """
     API:

     GET:

     Retrun: {
         arr: [
             {
        command          usage(%)
        -------+-----------------+----------------------------------------
        fea               6.3
        klish             4.4
        angel             3.6
        kmemleak          3.3
        ksoftirqd         2.4
        watchdog          2.1
        init              1.6
        cds               0.4
        ccs               0.2
        switch            0.1
        Total            24.4
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
         'log is null',         # err_code: 1
     ]

     obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
     arr = vcmd.get_showarr('cdbctl show/cdb/app/cpu_utilization')
     tmp = {}
     if len(arr[0]) > 1:
        for line in arr:
            item1 = string.join(string.split(line, ' '),'')
            item = line.split(' ')
            if item[0].find('Total') >= 0:
                tmp['total'] = item1.lstrip(item[0])
            if item[0].find('fea') >= 0:
                tmp['fea'] = item1.lstrip(item[0])
        obj['arr'].append(tmp)
     else:
         obj['error'] = True
         obj['err_code'] = 1
         obj['err_reason'] = _err_reason[1]
     return obj

def uptimeGet():
     """
     API:

     GET:

     Retrun: {
         arr: [
             0 days, 1 hours, 14 minutes
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
         'log is null',         # err_code: 1
     ]

     obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
     arr = vcmd.get_showarr('cdbctl show/cdb/sys/version')
     tmp = {}
     if len(arr[0]) > 1:
        for line in arr:
            if line.find('uptime') >= 0:
                tmp['uptime'] = line[line.find('is ') + len('is '):]
                obj['arr'].append(tmp)
                break
     else:
         obj['error'] = True
         obj['err_code'] = 1
         obj['err_reason'] = _err_reason[1]
     return obj

if __name__ == '__main__':
    pass

