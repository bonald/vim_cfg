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


def dot1x_auth_ctl_enable_get():
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

    status, arr = vcmd.get_status_arr('cdbctl read/cdb/l2/dot1x_global/global_auth_enable')
    
        
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj
    
    else:
        obj['dhcp_snooping_enable']= arr[0].get('global_auth_enable')
    

    return obj

def dot1x_auth_ctl_enable_set(enable):
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
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_global/global_auth_enable/%d' % int(enable))

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj


def dot1x_load_dot1x_port_get(key):
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
        arr = vcmd.get_arr('cdbctl read/cdb/l2/dot1x_port/')

        if arr:
            # pre-process for front-web easy display
            for entry in arr:
                if entry['guest_vlan'] == '0':
                    entry['guest_vlan'] = 'none'

                if entry['max_user'] == '0':
                    entry['max_user'] = 'none'
            obj['arr'] = arr
#        else:
#            obj['error'] = True
#            obj['err_code'] = 1
#            obj['err_reason'] = _err_reason[1]
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

def dot1x_load_dot1x_port_set(enable):
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
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_global/global_auth_enable/%d' % int(enable))
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj

    

def dot1x_re_active_radius_server():
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
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_radius/reactivate/all')
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj


def dot1x_re_active_tacacs_server():
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
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/app/auth_server/all/dead')
    
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj
    

def dot1x_init_port(req_data):
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
    status, lines = vcmd.get_status_lines(['cdbctl update/cdb/l2/dot1x_port/%s/initialize/1' % (port) for port in res])
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj

def dot1x_re_auth_port_set(req_data):
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
    status, lines = vcmd.get_status_lines(['cdbctl update/cdb/l2/dot1x_port/%s/reAuthenticate/1' % (port) for port in res])
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj
    
def dot1x_clear_port(req_data):
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
    status, lines = vcmd.get_status_lines(['cdbctl update/cdb/l2/clear_dot1x/interface/%s' % (port) for port in res])
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj
    
    
def dot1x_radius_server_deadtime_set(deadtime):
    """
    API:
    
    GET: / or /<int:deadtime>/
    
    @param deadtime: deadtime
        
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
        'group is not exist', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check 

    # excute

    status, arr = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_global/radius_default_deadtime/%d' % int(deadtime))

        
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj

def dot1x_radius_server_retransmit_set(retransmit):
    """
    API:
    
    GET: / or /<int:retransmit>/
    
    @param retransmit: retransmit
        
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
        'group is not exist', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check 

    # excute

    status, arr = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_global/radius_default_retry/%d' % int(retransmit))

        
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj


def dot1x_radius_server_timeout_set(timeout):
    """
    API:
    
    GET: / or /<int:deadtime>/
    
    @param deadtime: deadtime
        
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
        'group is not exist', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check 

    # excute

    status, arr = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_global/radius_default_timeout/%d' % int(timeout))

        
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj


def dot1x_radius_server_key_set(key):
    """
    API:
    
    GET: / or /<int:deadtime>/
    
    @param deadtime: deadtime
        
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'bad request',      # err_code: 1
        'cmd excute error', # err_code: 2
        'group is not exist', # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check 

    # excute
 
    status, arr = vcmd.cdbctl('update/cdb/l2/dot1x_global/radius_default_shared_secret/%s' % key)

        
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj    
    
    return obj


def dot1x_port_control_set(port_name, ctl_state):
    """
    API:
    
    PUT: /<port_name>/<ctl_state>/
    
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
    name = str(port_name)
    state = str(ctl_state)
      
    if state == "1" or state == "Auto":
      stat_value = "auto"
    elif state == "2" or state == "Force-unauthorized":
      stat_value = "force-unauthorized"      
    elif state == "3" or state == "Force-authorized":
      stat_value = "force-authorized"    
    elif state == "Disable":
      stat_value = "Disable"    

    if stat_value == "Disable":
      # excute
      status, lines = vcmd.get_status_lines('cdbctl delete/cdb/l2/dot1x_port/%s' % (name))
      if status != 0:
          obj['error'] = True
          obj['err_code'] = 2
          obj['err_reason'] = '; '.join(lines)
          return obj    
    else:      
      # excute
      status, lines = vcmd.get_status_lines('cdbctl create/cdb/l2/dot1x_port/%s/authControlledPortControl/%s' % (name, stat_value))
      if status != 0:
          obj['error'] = True
          obj['err_code'] = 2
          obj['err_reason'] = '; '.join(lines)
          return obj
    
    return obj

def dot1x_port_direction_set(port_name, direction):
    """
    API:
    
    PUT: /<port_name>/<direction>/
    
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
    name = str(port_name)
    state = str(direction)
        
    # excute

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/auth_ctrl_dir_adminControlledDirections/%s' % (name, state))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj
    
def dot1x_port_protocol_version_set(port_name, version):
    """
    API:
    
    PUT: /<port_name>/<version>/

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
    name = str(port_name)
    version = str(version)

    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/protocol_version/%s' % (name, version))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj
    
def dot1x_port_re_authentication_set(port_name, reAuth):
    """
    API:
    
    PUT: /<port_name>/<reAuth>/

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
    name = str(port_name)


    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/reAuthEnabled/%d' % (name, int(reAuth)))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj
    
def dot1x_port_mac_auth_bypass_set(port_name, bypass):
    """
    API:
    
    PUT: /<port_name>/<bypass>/
    
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
    name = str(port_name)

    # excute

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/mac_auth_bypass/%d' % (name, int(bypass)))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj    

    
def dot1x_port_max_req_set(port_name, req):
    """
    API:
    
    PUT: /<port_name>/<req>/

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
    name = str(port_name)

    # excute

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/auth_pae_reAuthMax/%d' % (name, int(req)))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj    


def dot1x_port_tx_period_set(port_name, period):
    """
    API:
    
    PUT: /<port_name>/<period>/
    
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
    name = str(port_name)

    # excute

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/auth_pae_txPeriod/%d' % (name, int(period)))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj        


def dot1x_port_quiet_period_set(port_name, period):
    """
    API:
    
    PUT: /<port_name>/<period>/
        
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
    name = str(port_name)

    # excute

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/auth_pae_quietPeriod/%d' % (name, int(period)))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj        
    
def dot1x_port_re_authperiod_set(port_name, re_authperiod):
    """
    API:
    
    PUT: /<port_name>/<re_authperiod>/
        
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
    name = str(port_name)

    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/reAuthPeriod/%d' % (name, int(re_authperiod)))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj 

def dot1x_port_supp_timeoutOne_set(port_name, supp_timeout):
    """
    API:
    
    PUT: /<port_name>/<supp_timeout>/

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
    name = str(port_name)

    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/auth_be_suppTimeout/%d' % (name, int(supp_timeout)))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj 

def dot1x_port_server_timeout_set(port_name, server_timeout):
    """
    API:
    
    PUT: /<port_name>/<server_timeout>/

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
    name = str(port_name)

    # excute

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/auth_be_serverTimeout/%d' % (name, int(server_timeout)))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj 

def dot1x_port_guest_vlan_set(port_name, guest_vlan):
    """
    API:
    
    PUT: /<port_name>/<guest_vlan>/

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
    name = str(port_name)

    # excute

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/guest_vlan/%d' % (name, int(guest_vlan)))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj 

def dot1x_port_port_mode_set(port_name, port_mode):
    """
    API:
    
    PUT: /<port_name>/<port_mode>/

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
    
    # mapping web to cmd
    if (port_mode == 1):
      mode = 1
    else: 
      mode = 0    
    
    # param check 
    name = str(port_name)

    # excute

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/auth_mode/%d' % (name, int(mode)))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj 

def dot1x_port_max_user_set(port_name, max_user):
    """
    API:
    
    PUT: /<port_name>/<max_user>/

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
    name = str(port_name)

    # excute

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/l2/dot1x_port/%s/max_user/%d' % (name, int(max_user)))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    
    return obj 
