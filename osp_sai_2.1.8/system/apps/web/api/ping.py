#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import json
def get(count, vrf_id, dest_ip,src_ip,src_if):
    """
    API:
    
    GET: /<int:count>/<vrf_id>/<dest_ip>/<src_ip>/
    
    Retrun: {
        ping_resp: string,
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'cmd excute error', # err_code: 1
    ]

    obj = {'ping_resp': '', 'error': False, 'err_code': 0, 'err_reason': ''}

    if src_if != 'undefined' and src_if !='null':
        str_a = str(src_if)
        if_dict = eval(str_a)
        if_name = str(if_dict['full_name'])


    if vrf_id=='mgmt' :
        (status, lines) = vcmd.get_status_lines('ip netns exec %s ping %s -c %d -i 0.1 -w 3' % (str(vrf_id), str(dest_ip), int(count)))
    elif (src_ip=='undefined' or src_ip =='null') and (src_if=='undefined' or src_if =='null') :
        (status, lines) = vcmd.get_status_lines('ip netns exec %s ping %s -c %d -i 0.1 -w 3' % (str(vrf_id), str(dest_ip), int(count)))
    elif (src_ip!='undefined' and src_ip!='null') and (src_if=='undefined' or src_if =='null'):
        (status, lines) = vcmd.get_status_lines('ip netns exec %s ping -I %s %s -c %d -i 0.1 -w 3' % (str(vrf_id), str(src_ip), str(dest_ip), int(count)))
    elif (src_ip=='undefined' or src_ip=='null') and (src_if!='undefined' and src_if !='null'):
        (status, lines) = vcmd.get_status_lines("ifconfig | grep -w %s -A 1 | tail -1 | grep -E -o '[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+' | head -1" %if_name)
        ifip=str(lines[0])
        (status, lines) = vcmd.get_status_lines('ip netns exec %s ping -I %s %s -c %d -i 0.1 -w 3' % (str(vrf_id), str(ifip), str(dest_ip), int(count)))
    elif (src_ip!='undefined' and src_ip!='null') and (src_if!='undefined' and src_if !='null'):
        lines = ['error:source ip and source cannot be specified at the same time']
    else:
        lines = ['error:cannot connect']
    if True or status == 0:
        obj['ping_resp'] = '\n'.join(lines)
            
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ''.join(lines)
    
    return obj
