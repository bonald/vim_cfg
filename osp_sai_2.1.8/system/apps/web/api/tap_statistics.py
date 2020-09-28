#!/usr/bin/python
# -*- coding: utf-8 -*-

from vcl import vcmd
import re

def get():

    _err_reason = [
        '',  # err_code: 0
        'interface statistic is not exist',  # err_code: 1
    ]
    obj = {'if_statistic_arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    status,lines = vcmd.get_status_lines('cdbctl show/cdb/interface/status')
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    for i in lines:
        tmp = {}
        prt = i.split(' ',1);
        if(prt[0][0:4] == 'eth-' or prt[0][0:3] == 'agg'):
            tmp['full_name'] = prt[0];
            flag = flow_config_en(prt[0])
            tmp['flow_flag'] = flag
            obj['if_statistic_arr'].append(tmp)

    return obj

def flow_config_en(ifname):
    status, lines = vcmd.get_status_lines('cdbctl show/cdb/tap/show_tap_flow_counter/%s/seq/' % ifname)
    if lines[0] == '' or status != 0:
        return False
    return True


def get_flow_statistics(ifname):

    _err_reason = [
        '',  # err_code: 0
        'get statistic on interface error',  # err_code: 1
    ]
    obj = {'flow_statistics': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    status, lines = vcmd.get_status_lines('cdbctl show/cdb/tap/show_tap_flow_counter/%s/seq/' % ifname)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj

    if lines[0] == '':
        return obj

    # major process logic
    for data in lines:
        if data.find('TAP group') != -1:
            continue
        if data.find('flow name') != -1:
            ace_arr = {}
            ace_arr_count = 0
            flow_name = data[len('flow name: '):]
        elif data.find('(total') != -1:
            te = {}
            flow_total_statistics = {}
            flow_bytes_total = re.findall(r'\d+',data)
            flow_total_statistics['bytes'] = flow_bytes_total[0]
            flow_total_statistics['total'] = flow_bytes_total[1]
            te['ace_arr'] = ace_arr
            te['flow_name'] = flow_name
            te['flow_total_statistics'] = flow_total_statistics
            obj['flow_statistics'].append(te)
        else:
            ace_one = {}
            index = data.find('( bytes')
            bytes_total = re.findall(r'\d+',data[index:])
            ace_one['flow_name'] = flow_name
            ace_one['rule'] = data[0:index]
            ace_one['bytes'] = bytes_total[0]
            ace_one['total'] = bytes_total[1]
            ace_arr[ace_arr_count] = ace_one
            ace_arr_count+=1

    return obj
if __name__ == '__main__':
    pass

