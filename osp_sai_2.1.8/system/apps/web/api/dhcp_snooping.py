#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''

import base
import types
import os
from vcl import vcmd
import  shutil
from base import log
from vcl import config
    
def load_dhcp_enabl_get():
    """
    API:
        
    Retrun: {
        if /:
            group_arr: [group_dict, group_dict, ...],
        elif /<int:git>/:
            group: group_dict
        
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    group_dict {
        lag_name: string,
        protocol: 'static' | 'LACP',
        group_state: 'L2' | 'L3',
        ports_in_bundle: int,
        member_ports: ['eth-0-1', ...],
        max_ports_count: int,
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
        'group is not exist', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # excute

    status, arr = vcmd.get_status_arr('cdbctl read/cdb/sys/sys_global/dhcp_snooping_enable')
        
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj
    
    else:
        obj['dhcp_snooping_enable']= arr[0].get('dhcp_snooping_enable')
    
    return obj

def load_dhcp_enabl_set(enable):
    """
    API:
    
    PUT: /<int:gid>/
    {
        if_arr: [
            'eth-0-1',
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
    
    # param check 
       
    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/sys/sys_global/dhcp_snooping_enable/%d' % int(enable))
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj

    
def load_dhcp_service_enabl_get():
    """
    API:
        
    Retrun: {
        if /:
            group_arr: [group_dict, group_dict, ...],
        elif /<int:git>/:
            group: group_dict
        
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    group_dict {
        lag_name: string,
        protocol: 'static' | 'LACP',
        group_state: 'L2' | 'L3',
        ports_in_bundle: int,
        member_ports: ['eth-0-1', ...],
        max_ports_count: int,
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
        'group is not exist', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # excute

    status, arr = vcmd.get_status_arr('cdbctl read/cdb/sys/sys_global/dhcp_service_enable')
        
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj
    
    else:
        obj['dhcp_service_enable']= arr[0].get('dhcp_service_enable')
    
    return obj

def load_dhcp_service_enabl_set(enable):
    """
    API:
    
    PUT: /<int:gid>/
    {
        if_arr: [
            'eth-0-1',
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
    
    # param check 
       
    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/sys/sys_global/dhcp_service_enable/%d' % int(enable))
     
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj
     
    
def load_dhcp_verify_mac_get():
    """
    API:
        
    Retrun: {
        if /:
            group_arr: [group_dict, group_dict, ...],
        elif /<int:git>/:
            group: group_dict
        
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    group_dict {
        lag_name: string,
        protocol: 'static' | 'LACP',
        group_state: 'L2' | 'L3',
        ports_in_bundle: int,
        member_ports: ['eth-0-1', ...],
        max_ports_count: int,
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
        'group is not exist', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # excute

    status, arr = vcmd.get_status_arr('cdbctl read/cdb/dhcp/dhcsnooping/verify_mac_address')
        
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj
    
    else:
        obj['verify_mac_address']= arr[0].get('verify_mac_address')
    
    return obj

def load_dhcp_verify_mac_set(enable):
    """
    API:
    
    PUT: /<int:gid>/
    {
        if_arr: [
            'eth-0-1',
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
    
    # param check 
       
    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/dhcp/dhcsnooping/verify_mac_address/%d' % int(enable))
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj


def dhcp_save_database_set():
    """
    API:
    
    PUT: /<int:gid>/
    {
        if_arr: [
            'eth-0-1',
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
    
    # param check 
       
    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/dhcp/dhcsnooping_db/save')
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj


def dhcp_allow_untrusted_get():
    """
    API:

    Retrun: {
        if /:
            group_arr: [group_dict, group_dict, ...],
        elif /<int:git>/:
            group: group_dict

        error: bool,
        err_code: int,
        err_reason: string
    }

    group_dict {
        lag_name: string,
        protocol: 'static' | 'LACP',
        group_state: 'L2' | 'L3',
        ports_in_bundle: int,
        member_ports: ['eth-0-1', ...],
        max_ports_count: int,
    }
    """

    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
        'group is not exist',  # err_code: 3
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # excute

    arr = vcmd.get_arr('cdbctl read/cdb/dhcp/dhcsnooping/allow_untrusted')

    if arr:
        obj['allow_untrusted'] = arr[0]['allow_untrusted']
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]

    return obj


def dhcp_allow_untrusted_set(enable):
    """
    API:
    
    PUT: /<int:gid>/
    {
        if_arr: [
            'eth-0-1',
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
    
    # param check 
       
    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/dhcp/dhcsnooping/allow_untrusted/%d' % int(enable))
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj

    
def dhcp_snooping_information_option_get():
    """
    API:
        
    Retrun: {
        if /:
            group_arr: [group_dict, group_dict, ...],
        elif /<int:git>/:
            group: group_dict
        
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    group_dict {
        lag_name: string,
        protocol: 'static' | 'LACP',
        group_state: 'L2' | 'L3',
        ports_in_bundle: int,
        member_ports: ['eth-0-1', ...],
        max_ports_count: int,
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
        'group is not exist', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # excute
    
    arr = vcmd.get_arr('cdbctl read/cdb/dhcp/dhcsnooping/add_agent_options')
    
    if arr:
        obj['add_agent_options'] = arr[0]['add_agent_options']
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]

    return obj    
    
    
def dhcp_snooping_information_option_set(enable):
    """
    API:
    
    PUT: /<int:gid>/
    {
        if_arr: [
            'eth-0-1',
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
    
    # param check 
       
    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/dhcp/dhcsnooping/add_agent_options/%d' % int(enable))
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj


def dhcp_port_trust_get():
    """
    API:

    {
        if_arr: [
            'eth-0-1',
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
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'cmd excute error',  # err_code: 2
        'group is not exist',  # err_code: 3
    ]

    obj = {
        'trust_port': [],
        'error': False,
        'err_code': 0,
        'err_reason': ''
    }

    # excute

    status, lines = vcmd.get_status_lines('cdbctl show/cdb/dhcp/dhcsnooping/trusted')

    if status != 0 or len(lines) <= 1:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ''.join(lines)
        return obj

    # "2    0002.0001.0001 eth-0-22   58295      2.1.1.1"
    # ['2', '', '', '', '0002.0001.0001', 'eth-0-22', '', '', '58295', '', '', '', '', '', '2.1.1.1']

    line = lines[2:]

    lists = []
    if len(line) >= 1:
        for arr in line:
            list = arr.split()
            lists.extend(list)

    obj['trust_port'].extend(lists)
    return obj

def dhcp_port_trust_set(req_data):
    """
    API:
    
    {
        if_arr: [
            'eth-0-1',
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
    
    # param check 
    if req_data.get('if_arr') == None or type(req_data['if_arr']) != types.ListType:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj    
    
    res, err = base.check.array(req_data['if_arr'], ['string', 'bad request: if_arr', 1], False)

    if res == None:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj    
    
    # How delete old configure ?
 
                
    # excute
    status, lines = vcmd.get_status_lines(['cdbctl update/cdb/interface/%s/dhcp_snooping_trust/1' % (port) for port in res])
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj
        

def dhcp_del_port_trust(req_data):
    """
    API:
    
    {
        if_arr: [
            'eth-0-1',
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
    
    # param check 
    if req_data.get('if_arr') == None or type(req_data['if_arr']) != types.ListType:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj    
    
    res, err = base.check.array(req_data['if_arr'], ['string', 'bad request: if_arr', 1], False)

    if res == None:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj    
    
    # How delete old configure ?
 
                
    # excute
    status, lines = vcmd.get_status_lines(['cdbctl update/cdb/interface/%s/dhcp_snooping_trust/0' % (port) for port in res])
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj


def dhcp_load_bind_entry_get():
    """
    API:

    GET:

    Retrun: {
        arr: [
         {
          vlan      : 2
          mac       : 1.1.1
          interface : eth-0-11
          lease     : 111
          ip        : 2.2.2.2
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
        '',  # err_code: 0
        'cmd excute error',  # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    arr = vcmd.get_showarr('cdbctl show/cdb/dhcp/dhcbinding/configed')

    # "2    0002.0001.0001 eth-0-22   58295      2.1.1.1"
    # ['2', '', '', '', '0002.0001.0001', 'eth-0-22', '', '', '58295', '', '', '', '', '', '2.1.1.1']

    if len(arr) <= 3:
        return obj

    arr = arr[3:]

    if len(arr[0]) > 1:
        for line in arr:
            tmp = {}
            item = line.split()
            tmp['vlan'] = item[0];
            tmp['mac'] = item[1];
            tmp['Interface'] = item[2];
            tmp['Lease'] = item[3];
            tmp['ip'] = item[4];
            tmp['type'] = 'manual';

            obj['arr'].append(tmp)
    else:
        obj['error'] = False
        obj['err_code'] = 0
        obj['err_reason'] = ''
        return obj

    arr = vcmd.get_showarr('cdbctl show/cdb/dhcp/dhcbinding/learning')

    if len(arr) <= 3:
        return obj

    arr = arr[3:]

    if len(arr[0]) > 1:
        for line in arr:
            tmp = {}
            item = line.split()
            tmp['vlan'] = item[0];
            tmp['mac'] = item[1];
            tmp['Interface'] = item[2];
            tmp['Lease'] = item[3];
            tmp['ip'] = item[4];
            tmp['type'] = 'learning';

            obj['arr'].append(tmp)
    else:
        obj['error'] = False
        obj['err_code'] = 0
        obj['err_reason'] = ''
        return obj


    return obj


def dhcp_del_one_bind_entry(mac):
    """

    """
    
    _err_reason = [
        '',                 # err_code: 0
        'cmd excute error', # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}  
    
    status, lines = vcmd.get_status_lines(' cdbctl delete/cdb/dhcp/dhcbinding/configed/mac/%s' % mac)
    
    return obj


def dhcp_del_all_bind_entry():
    """
    API:
    
    GET:
    
    Retrun: {
        arr: [
         {
          vlan      : 2
          mac       : 1.1.1
          interface : eth-0-11
          lease     : 111
          ip        : 2.2.2.2
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
        '',                 # err_code: 0
        'cmd excute error', # err_code: 1
    ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}  

    # param check 
    if req_data.get('if_arr') == None or type(req_data['if_arr']) != types.ListType:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj    
    
    res, err = base.check.array(req_data['all_bind_entry'], ['string', 'bad request: all_bind_entry', 1], False)    
    
    return obj
    
def dhcp_bind_entry_set(new_fdb):
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
        ['mac_address', 'string', 're', r'^([\da-fA-F]{1,4}\.){2}[\da-fA-F]{1,4}$', 'bad_request: mac_address', 1],
        ['port', 'string', 'bad_request: port', 1],
        ['ip', 'string', 'bad_request: ip address', 1],        
        ['expiry', 'int', '>=', 0, 'and', '<=', 86400, 'bad request: expiry', 1],   
    ])
    # error
    if res == None:
        obj['error'] = True
        obj['err_code'] = err['err_code']
        obj['err_reason'] = err['err_reason']
        return obj
    
    # add fdb
    cmd = 'cdbctl create/cdb/dhcp/dhcbinding/%s#0/vid/%d/client_ip/%s/ifname/%s/lease/%d/state/running/type/configed' % (res['mac_address'], res['vlan_id'], res['ip'], res['port'], res['expiry'])

    status, lines = vcmd.get_status_lines(cmd)
    # error
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
        
    return obj


def dhcp_packet_statistics_get():
    """
    API:
    
    GET:
    
    Retrun: {
        statistics: [
         {
          DHCP packets        : 2
          BOOTP packets       : 1
          BOOTP invalid       : 0
          ....
         },
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
    
    obj = {
        'statistics': [],
        'error': False,
        'err_code': 0, 
        'err_reason': ''
    }
    
    (status, array) = vcmd.get_status_lines('cdbctl show/cdb/dhcp/dhcsnooping/statistics')

    if status != 0 or len(array) <= 2:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ''.join(array)
        return obj
    temp = {}
    lines = array[2:]
    
    if len(lines) >= 1:
      for arr in lines:
          list = arr.split()
 
          if (list[0] == 'DHCP'):
            temp['dhcp'] = list[2]
          if (list[0] == 'BOOTP'):
            temp['bootp'] = list[2]
          if (list[1] == 'forwarded'):
            temp['forwarded'] = list[2]   
          if (list[1] == 'invalid'):
            temp['invalid'] = list[2]  
          if (list[1] == 'MAC'):
            temp['MAC'] = list[5]
          if (list[1] == 'dropped'):
            temp['dropped'] = list[2]  
          
      obj['statistics'].append(temp)
    else:
       obj['error'] = False
       obj['err_code'] = 0
       obj['err_reason'] = ''  

    return obj


def dhcp_packet_statistics_delete():


    """
    API:

    GET:

    Retrun: {
        statistics: [
         {
          DHCP packets        : 2
          BOOTP packets       : 1
          BOOTP invalid       : 0
          ....
         },
        ],
        error: bool,
        err_code: int,
        err_reason: string
    }
    """

    _err_reason = [
        '',                  # err_code: 0
        'cmd excute error',  # err_code: 1
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    status, lines = vcmd.get_status_lines('cdbctl delete/cdb/dhcp/dhcsnooping/statistics')
    if status != 0:
        obj['error'] = True
        obj['err_code'] = status
        obj['err_reason'] = '; '.join(lines)
        return obj

    return obj


def dhcp_get_interval():
    """
    API:
        
    Retrun: {

        database_save_interval: database_save_interval,        
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
        'group is not exist', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # excute
    
    arr = vcmd.get_arr('cdbctl read/cdb/dhcp/dhcsnooping/database_save_interval')
    
    if arr:
        obj['database_save_interval'] = arr[0]['database_save_interval']
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]

    return obj    
    
        
def dhcp_modify_interval_set(interval):
    """
    API:
    
    PUT: /<int:gid>/
    {
        if_arr: [
            'eth-0-1',
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
    
    # param check 
       
    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/dhcp/dhcsnooping/database_save_interval/%d' % int(interval))
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj
        

def dhcp_modify_remote_id(remote_id):
    """
    API:
    
    PUT: /<int:gid>/
    {
        if_arr: [
            'eth-0-1',
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
    
    # param check 
       
    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/dhcp/dhcsnooping/hostname_as_remote_id/0/remote_id_string/%s' % remote_id)
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj

def dhcp_add_snoop_vlan(vlan_str):
    """
    API:
    
    PUT: /<int:vid>/

    
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
       
    # excute

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/dhcp/dhcsnooping/vlans/%s' % vlan_str)
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj    
    

def dhcp_del_snoop_vlan(vlan_str):
    """
    API:
    
    PUT: /<int:vid>/

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
       
    # excute

    status, lines = vcmd.get_status_lines('cdbctl delete/cdb/dhcp/dhcsnooping/vlans/%s' % vlan_str)
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj       

def dhcp_load_snoop_db(db_str):
    """
    API:
    
    PUT: /<str:db_str>/

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
    path_str='/mnt/flash/'+db_str
    # excute
    status, lines = vcmd.get_status_lines('echo "`mktemp /tmp/dhcbindingXXXXXX`"')
    tmp_lines=lines[0]
    cmd= 'cp ' + path_str+' ' + tmp_lines
    status, lines = vcmd.get_status_lines(cmd)
    load_name = os.path.basename(tmp_lines)
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/dhcp/dhcsnooping_db/load/%s' %load_name)
    status, lines = vcmd.get_status_lines('rm %s' %tmp_lines)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj

    return obj           
