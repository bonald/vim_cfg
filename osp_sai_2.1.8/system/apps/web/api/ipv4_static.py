#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''

import base
import types
from vcl import vcmd
from base import log


def ipv4_static_route_set(req_data):
    """
    API:

    PUT: {/*static_route_cfg*/
        dest : dest,
        mast : mast,
        nexthop: nexthop,
        distance : <1-255>,

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
         
    ip = str(req_data.get('dest'))
    mast = str(req_data.get('mast'))
    nexthop = str(req_data.get('nexthop'))
    distance = str(req_data.get('distance'))
    
    ip_with_mast = "%s/%s"  % (ip, mast)

    if distance == "None":
      cmd = 'cdbctl create/cdb/l3/static_route_cfg/ip/%s/ipgw/%s/distance//track_id/' % (ip_with_mast, nexthop)
    else: 
      cmd = 'cdbctl create/cdb/l3/static_route_cfg/ip/%s/ipgw/%s/distance/%s/track_id/' % (ip_with_mast, nexthop, distance)
    
    (status, lines) = vcmd.get_status_lines(cmd)
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
       
    return obj

        
def ipv4_static_route_delete(req_data):
    """
    API:
    
    DELETE: /<int:gid>/
    {
        entry_arr: 
        [
            { ip: "1.1.1.0/24", nexthop: "11.1.1.1"},
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
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    entry_arr = req_data.get('entry_arr')
    for item in entry_arr:
        ip = str(item.get('ip'))
        nexthop = str(item.get('nexthop'))
        if nexthop == "connected":
            cmd = 'cdbctl delete/cdb/l3/static_route_cfg/ip/%s/null/0' % (ip)
        else:
            cmd='cdbctl delete/cdb/l3/static_route_cfg/ip/%s/ipgw/%s' %(ip, nexthop)
        status, lines = vcmd.get_status_lines(cmd)
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(lines)
            return obj     


    return obj
            

      

    
