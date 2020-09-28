#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import os

def tacas_server_get():
    """
    TBD
    """

    _err_reason = [
            '',             # err_code: 0
            'log is null',  # err_code: 1
            ]

    obj = {'server': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    status, arr = vcmd.get_status_arr('cdbctl read/cdb/app/auth_server')

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj

    else:
        for server in arr:
            server['host'], server['type'], _ = server['key'].split('#')
            if server['is_inband'] == "0":
                server['is_inband'] = "mgmt"
            else:
                server['is_inband'] = "default"
            del server['key']
        obj['server'] = arr
    return obj


def tacas_server_add(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                 # err_code: 0
            'bad request',      # err_code: 1
            'cmd excute error', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    host = str(req_data.get('host'))
    key = str(req_data.get('key'))
    enable = str(req_data.get('KeyEn'))
    vrf = str(req_data.get('inband'))
    if vrf == "mgmt":
        NS_BAND = "outband"
    else:
        NS_BAND = "inband"

    if False == os.path.isfile('/switch/etc/cmd/tap_group.xml'):
        cmd = 'cdbctl update/cdb/l3/check_mgmt_if_ip/%s' % (host)
        # excute
        status, lines = vcmd.get_status_lines(cmd)
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(lines)
            return obj 
 
    cmd = 'cdbctl create/cdb/sys/ns_route/0#49#%s/8/%s/' % (host, NS_BAND)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj 

    keystr = host + '#' 'tacplus' + '#' + '49'
    matchstr = '/key=%s' % (keystr)

    cmd = 'cdbctl read/cdb/app/auth_server/%s/key' % (keystr)
    status, lines = vcmd.get_status_lines(cmd)

    if lines == matchstr:
        cmd = 'cdbctl update/cdb/app/auth_server/%s' % (keystr)
    else:
        cmd = 'cdbctl create/cdb/app/auth_server/%s' % (keystr)

    cmd += '/is_inband/%s' % (NS_BAND)
    if enable == 'True' :
        cmd += '/secret/"%s"' % (key)
    else :
        cmd += '/secret/'

    #use default value, realize it later
    cmd += '/retries/3/timeout/5/port/49'
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        # add server fail, fallback
        cmd = 'cdbctl delete/cdb/sys/ns_route/0#49#%s' % (keystr)
        vcmd.get_status_lines(cmd)

        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj 

    return obj


def tacas_server_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                 # err_code: 0
            'bad request',      # err_code: 1
            'cmd excute error', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # param check 
    host = str(req_data.get('host'))
    keystr = host + '#' 'tacplus' + '#' + '49'
    vrf = str(req_data.get('inband'))
    if vrf == "mgmt":
        NS_BAND = "outband"
    else:
        NS_BAND = "inband"
    cmd = 'cdbctl delete/cdb/app/auth_server/%s/is_inband/%s' % (keystr, NS_BAND)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj 

    cmd = 'cdbctl delete/cdb/sys/ns_route/0#49#%s/%s/' % (host, NS_BAND)
    vcmd.get_status_lines(cmd)

    return obj

if __name__ == '__main__':
    pass
