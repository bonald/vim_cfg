#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import string
import base64
import types

def ntp_srv_get():
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}



    cmd = 'cdbctl read/cdb/app/ntp_server'

    arr = vcmd.get_arr(cmd)

    if arr:
        for entry in arr:

            if entry['is_inband'] == '1':
                entry['is_inband'] = 'default'
            elif entry['is_inband'] == '0':
                entry['is_inband'] = 'mgmt'

    if arr:
        obj['cfg'] = arr
    else:
        obj['cfg'] = []

    return obj

def clock_current_time_get():
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    status, cfg = vcmd.get_status_lines('cdbctl show/cdb/sys/clock')

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = _err_reason[1]
        return obj

    tmp = cfg[0].split()
    str = tmp[0] + ' ' + tmp[2] + ' ' + tmp[3] + ' ' + tmp[4] + ' ' + tmp[5] + ' ' + tmp[1]

    obj['cfg'] = str

    return obj

def clock_current_time_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'Set Time Form Error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    year = str(req_data.get('Year'))
    month = str(req_data.get('Month'))
    day = str(req_data.get('Day'))
    hour = str(req_data.get('Hour'))
    minute = str(req_data.get('Minute'))
    second = str(req_data.get('Second'))

    time = hour +":" + minute + ":" + second

    status, cfg = vcmd.get_status_lines('cdbctl update/cdb/sys/clock/%s/%s/%s/%s' % (time, day, month, year))

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = _err_reason[1]
        return obj
        
    return obj

def ntp_srv_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    ntp_srv = str(req_data.get('key'))
    vrf = str(req_data.get('is_inband'))

    if vrf == "mgmt":
        NS_BAND = "outband"
    else:
        NS_BAND = "inband"

    cmd = 'cdbctl delete/cdb/app/ntp_server/%s/%s/' % (ntp_srv, NS_BAND)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    cmd = 'cdbctl delete/cdb/sys/ns_route/1#123#%s/%s/' % (ntp_srv, NS_BAND)
    status, lines = vcmd.get_status_lines(cmd)

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    cmd = 'cdbctl delete/cdb/sys/inband_snat/1#123#%s/' % (ntp_srv)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj

def ntp_srv_add(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'Server already assign to ntp server',         # err_code: 1
            'Please check the ip address of ntp server!'
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}


    ntp_srv = str(req_data.get('ntp_srv'))
    vrf = str(req_data.get('inband'))

    if vrf == "mgmt":
        NS_BAND = "outband"
    else:
        NS_BAND = "inband"

    cmd = 'cdbctl create/cdb/sys/ns_route/1#123#%s/2/%s/' % (ntp_srv, NS_BAND)
    cmddelete = 'cdbctl delete/cdb/sys/ns_route/1#123#%s/%s/' % (ntp_srv, NS_BAND)
    print cmd
    print cmddelete
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0 and lines[0]=='% Server already assign to ntp server':
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;
    if status != 0 and lines[0]=='% Please check the ip address of ntp server!':
        obj['error'] = True
        obj['err_code'] = 2;
        obj['err_reason'] = _err_reason[2];
        return obj;

    cmd = 'cdbctl create/cdb/app/ntp_server/%s/' % (ntp_srv)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0 and lines[0]=='% Server already assign to ntp server':
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        status, lines = vcmd.get_status_lines(cmddelete)
        return obj;
    if status != 0 and lines[0]=='% Please check the ip address of ntp server!':
        obj['error'] = True
        obj['err_code'] = 2;
        obj['err_reason'] = _err_reason[2];
        status, lines = vcmd.get_status_lines(cmddelete)
        return obj;

    cmd = 'cdbctl update/cdb/app/ntp_server/set_band/%s/%s/' % (ntp_srv, NS_BAND)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0 and lines[0]=='% Server already assign to ntp server':
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        status, lines = vcmd.get_status_lines(cmddelete)
        return obj;
    if status != 0 and lines[0]=='% Please check the ip address of ntp server!':
        obj['error'] = True
        obj['err_code'] = 2;
        obj['err_reason'] = _err_reason[2];
        status, lines = vcmd.get_status_lines(cmddelete)
        return obj;

    return obj

def clock_timestamp_sync_set(enable):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    if enable:
        cmd = 'cdbctl update/cdb/ptp/fea_time/sync_type/systime'
    else:
        cmd = 'cdbctl update/cdb/ptp/fea_time/sync_type/none'

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = ';'.join(lines) if len(lines) >= 1 else _err_reason[1]
        return obj

    return obj

def clock_timestamp_sync_get():
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'last': [], 'type': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl show/cdb/ptp/fea_time/formatted'

    status, lines = vcmd.get_status_lines(cmd)

    if status != 0:
        return None
    else:

        for item in lines:
            if not item.find('Last Sync Time'):
                item = item.split("   : ")
                obj['last'] = item[1]

            elif not item.find('Sync Type'):
                item = item.split("        : ")
                obj['type'] = item[1]

    return obj    

if __name__ == '__main__':
    pass

