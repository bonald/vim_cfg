#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import os


def radius_server_get():
    """
    TBD
    """
    _err_reason = [
            '',             # err_code: 0
            'log is null',  # err_code: 1
            ]

    obj = {'auth_cfg': [], 'dot1x_radius':[], 'error': False, 'err_code': 0, 'err_reason': ''}

    status, arr = vcmd.get_status_arr('cdbctl read/cdb/app/auth_cfg')
    status1, arr1 = vcmd.get_status_arr('cdbctl read/cdb/app/auth_server')

    if (status != 0) and (status1 != 0):
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj
    else:
        for cfg in arr1:
            cfg['host'], cfg['type'], _ = cfg['key'].split('#')
            if cfg['is_inband'] == "0":
                cfg['is_inband'] = "mgmt"
            else:
                cfg['is_inband'] = "default"
            del cfg['key']

        obj['auth_cfg'] = arr
        obj['dot1x_radius'] = arr1
        #print obj

    return obj


def radius_server_add(req_data):
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
    vrf = str(req_data.get('inband'))
    key = str(req_data.get('key'))
    enable = str(req_data.get('KeyEn'))
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


    cmd = 'cdbctl create/cdb/sys/ns_route/1#1812#%s/7/%s/' % (host, NS_BAND)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
    if enable=="True":
        cmd = 'cdbctl create/cdb/l2/dot1x_radius/key/%s/1812/shared_secret/"%s"/max_retry/0/timeout/0/is_inband/%s/' % (host, key, NS_BAND)
    else:
        cmd = 'cdbctl create/cdb/l2/dot1x_radius/key/%s/1812/shared_secret//max_retry/0/timeout/0/is_inband/%s/' % (host, NS_BAND)

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj 

    return obj

def radius_server_del(req_data):
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
    vrf = str(req_data.get('inband'))
    if vrf == "mgmt":
        NS_BAND = "outband"
    else:
        NS_BAND = "inband"

    print host
    cmd = 'cdbctl delete/cdb/l2/dot1x_radius/key/%s/%s/1812' % (host, NS_BAND)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj 
   
    cmd = 'cdbctl delete/cdb/sys/ns_route/1#1812#%s/%s/' % (host, NS_BAND)
    vcmd.get_status_lines(cmd)

    return obj



if __name__ == '__main__':
    pass
