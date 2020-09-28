#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on May 9, 2016

@author: wangqj
'''

import base
import types
import copy
from vcl import vcmd
from base import log


def ipv4_routing_get(key):
    """
    API:

    GET:

    Retrun: {
        static: [
            route_entry,
            ...
            ],

        connect: [
            route_entry,
            ...
            ],

        ospf: [
            route_entry,
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

    obj = {'static': [], 'connect': [], 'ospf': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    if key == '' or key == 'all':
        arr = vcmd.get_lines('vtysh -c "show ip route"')

        if len(arr) <= 3:
          return obj  
          
        lines = arr[4:]        
    
        if len(arr[0]) > 1:
          lasttmp = {}
          lasttype = ''
          for line in lines:
              tmp = {}
              item = line.split()
              
              # static routing 
              if item and (item[0].find('S') != -1): 
                  # For example:  ['S>*', '1.1.1.0/24', '[11/0]', 'via', '2.1.1.1', 'inactive']  
                  
                  tmp['selected']   = "Yes" if ( item[0].find('>') != -1 ) else "No"
                  tmp['fib']        = "Yes" if ( item[0].find('*') != -1 ) else "No"
                  
                  tmp['type']       = 'S';
                  tmp['ip']         = item[1]
                  
                  # distance/metric 
                  tmp['distance']   = item[2]
                  
                  if len(item) == 7:
                      # black-hole-route
                      tmp['nexthop']    = item[5].strip(',')
                      tmp['active']     = item[6].strip(',')
                  else:
                      # static route
                      tmp['nexthop']    = item[4].strip(',')
                      tmp['active']     = item[5].strip(',')

                  lasttmp = tmp
                  lasttype = 'static'
                  obj['static'].append(copy.deepcopy(tmp))

               
              ## just simple handler, may be exist bug
              # For example:  
                  ['S>*', '1.1.1.0/24', '[11/0]', 'via', '2.1.1.1', 'inactive'] 
                  [                               'via', '2.1.1.2', 'inactive']                   
              # FixME
              #if item and (item[0].find('via') != -1): 
              #
              #    #tmp = obj['arr'][-1].copy()
              #    tmp['nexthop'] = item[1].strip(',')
              #    tmp['active'] = item[2]

              #    obj['static'].append(tmp) 
              
              # connect routing  
              if item and (item[0].find('C') != -1): 
                  # For example:  ['C>*', '2.1.1.0/24', 'is', 'directly', 'connected,', 'eth-0-2'] 
                  
                  tmp['type']       = 'C';
                  tmp['selected']   = "Yes" if ( item[0].find('>') != -1 ) else "No"
                  tmp['fib']        = "Yes" if ( item[0].find('*') != -1 ) else "No"
                  
                  tmp['ip']         = item[1]
                  tmp['ifname']   = item[5]

                  lasttmp = tmp
                  lasttype = 'connect'
                  obj['connect'].append(copy.deepcopy(tmp))
              # OSPF routing
              if item and (item[0].find('O') != -1):
                  if 'directly' in item:
                      # For example:  "O   1.1.1.0/24 [110/10] is directly connected, eth-0-49/1, 00:03:14" 
                      tmp['type']       = 'O'
                      tmp['selected']   = "Yes"
                      tmp['fib']        = "Yes" if ( item[0].find('*') != -1 ) else "No"
                      
                      tmp['ip']         = item[1]

                      tmp['ifname']   = item[6].strip(',')

                      tmp['uptime']   = item[7]


                  else:
                      # For example:  "O>* 2.1.1.0/24 [110/20] via 1.1.1.2, eth-0-49/1, 00:03:0"
                      #                O   109.109.109.109/32 [110/100] via 0.0.0.0, loopback0 onlink, 07:01:19
                      tmp['type']       = 'O'
                      tmp['selected']   = "No"
                      tmp['fib']        = "Yes" if ( item[0].find('*') != -1 ) else "No"
                      tmp['ip']         = item[1]
                      tmp['ifname']   = item[5].strip(',')
                      if item[5].find('loopback') != -1:
                          tmp['uptime']   = item[7]
                      else:
                          tmp['uptime']   = item[6]

                  lasttmp = tmp
                  lasttype = 'ospf'
                  obj['ospf'].append(copy.deepcopy(tmp))

              if item and (item[0].find('via') != -1):
                  tmp = lasttmp
                  tmp['nexthop'] = item[1].strip(',')
                  tmp['active'] = item[2]
                  obj[lasttype].append(copy.deepcopy(tmp))

              if item and (item[1].find('via') != -1):
                  tmp = lasttmp
                  tmp['nexthop'] = item[2].strip(',')
                  tmp['active'] = item[3]
                  obj[lasttype].append(copy.deepcopy(tmp))

        else:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = _err_reason[1]

    return obj
