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


def ipv4_port_load_l3_port_get(key):
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
    entry = []

    if key == '' or key == 'all':
        arr = vcmd.get_arr('cdbctl read/cdb/l3/route_if/')
        
        if arr:
          for i in range(len(arr)):  
            ipv4 = arr[i].get('ifc_ipv4')
            
            if (isinstance(ipv4, list)):
              
              for item in ipv4:
                # Add option web need
                item['name'] = arr[i].get('key') 
                item['ip_redirects_en'] = arr[i].get('ip_redirects_en') 
                item['unicast_rpf_en'] = arr[i].get('unicast_rpf_en') 
                
                # ',/key' --> 'key'
                if item.has_key(',/key'):

                  item['ip'] = item[',/key'].split('#')[0]
                  item['mask'] = item[',/key'].split('#')[1]

                  del item[',/key']

                if item.has_key('key'):

                  item['ip'] = item['key'].split('#')[0]
                  item['mask'] = item['key'].split('#')[1]

                  del item['key']
                  
                # append to entry
                entry.append(item)
                
            if (isinstance(ipv4, dict)):
              ipv4['name'] = arr[i].get('key')
              ipv4['ip_redirects_en'] = arr[i].get('ip_redirects_en') 
              ipv4['unicast_rpf_en'] = arr[i].get('unicast_rpf_en')
              if (ipv4.has_key('key')):
                item = ipv4['key'].split('#')
                ipv4['ip'] = item[0]
                ipv4['mask'] = item[1]
              else:
                ipv4['ip'] = 'N/A'
                ipv4['mask'] = 'N/A'
              entry.append(ipv4)
 
          obj['arr'] = entry  

    else:
        arr = vcmd.get_lines('cdbctl read/cdb/l2/dot1x_port/%s' % key)
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
    
    
def ipv4_port_verify_unicast_reverse_path_set(iface, enable):
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
         



    iface = str(iface)    
    set_value = '1' if (enable == 'Enable') else '0'
    
    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l3/route_if/%s/unicast_rpf_en/%s' % (iface, set_value))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj
    
def ipv4_port_ip_redirect_set(iface, enable):
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
         
    iface = str(iface)    
    set_value = '1' if (enable == 'Enable') else '0'
    
    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l3/route_if/%s/ip_redirects_en/%s' % (iface, set_value))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj

    
def ipv4_port_ip_address_set(iface, ipaddr, mask, enable, secondary):
    """
    API:

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
    
    iface = str(iface)    
    ipaddr = str(ipaddr)    
    mask = str(mask)    
    enable = str(enable)    
    secondary = str(secondary)   
    
    flag_secondary="1" if secondary=="Secondary" or secondary=="1" else ''

    cmd = 'cdbctl update/cdb/l3/route_if/%s/ifc_ipv4/add/%s/%s/%s' % (iface, ipaddr, mask, flag_secondary)
        
    if enable == "Add":
      status, lines = vcmd.get_status_lines('cdbctl update/cdb/l3/route_if/%s/ifc_ipv4/add/%s/%s/%s' % (iface, ipaddr, mask, flag_secondary))
      if status != 0:
          obj['error'] = True
          obj['err_code'] = 2
          obj['err_reason'] = '; '.join(lines)

    else:   
      if flag_secondary:     
        status, lines = vcmd.get_status_lines('cdbctl update/cdb/l3/route_if/%s/ifc_ipv4/del/%s/%s/%s' % (iface, ipaddr, mask, flag_secondary))
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(lines)

      else:
        status, lines = vcmd.get_status_lines('cdbctl update/cdb/l3/route_if/%s/ifc_ipv4/del/%s/%s' % (iface, ipaddr, mask))
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(lines)

    
    return obj










    
    
