#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import string
import base64


def clear():
    _err_reason = [
        '',                     # err_code: 0
        'bad request param',    # err_code: 1
        'cannot set iptable prevent', # err_code: 2
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/sys/iptables_prevent/pkt_statistic_update/0')
    if status != 0:
        obj['error'] = True
        obj['err_code'] = status
        obj['err_reason'] = '; '.join(lines)
        return obj

    return obj


def get():
    """
    API:

    GET:

    Retrun: {
        arr: {[
            ddos_prevent
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
        'iptables prevent is not exist',         # err_code: 1
    ]

    obj = {'arr': {}, 'error': False, 'err_code': 0, 'err_reason': ''}

    vcmd.execute('cdbctl update/cdb/sys/iptables_prevent/pkt_statistic_update/1')
    tmp = vcmd.get_lines('cdbctl read/cdb/sys/iptables_prevent')
    if tmp:
        prevent_info = tmp[0]
        obj['arr'] = vcmd._line_to_obj_simple(prevent_info)
        #print obj['arr']
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
    return obj

def modify(req_data):
    """
    API:

    PUT: {/*tbl_iptables_prevent*/
        type : prevent_type,
        value: num,

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
        'cannot set iptable prevent', # err_code: 2
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # param check
    #print req_data
    pre_type = str(req_data.get('type'))
    #print pre_type
    if not pre_type:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj

    pre_value = str(req_data.get('value'))
    #print pre_value 
    if pre_type:
        if pre_type == 'smpkt':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/small_pkt_length/%s' % (pre_value)
            status, lines = vcmd.get_status_lines(cmd)
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_small_pkt/1'
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'icmp':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/icmp_rate_limit/%s' % (pre_value)
            status, lines = vcmd.get_status_lines(cmd)
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_icmp/1'
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'udp':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/udp_rate_limit/%s' % (pre_value)
            status, lines = vcmd.get_status_lines(cmd)
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_udp/1'
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'tcp':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/tcp_rate_limit/%s' % (pre_value)
            status, lines = vcmd.get_status_lines(cmd)
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_tcp/1'
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'fraggle':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_fraggle/%s' % (pre_value) 
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'smurf':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_smurf/%s' % (pre_value) 
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'maceq':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_maceq/%s' % (pre_value) 
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'ipeq':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_ipeq/%s' % (pre_value) 
            status, lines = vcmd.get_status_lines(cmd)
        else:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '% set ddos prevent error type'
            return obj
        if status != 0:
            obj['error'] = True
            obj['err_code'] = status
            obj['err_reason'] = '; '.join(lines)
            return obj

    return obj

def delete(req_data):
    """
    API:

    DELETE: {/*tbl_iptables_prevent*/
        type : prevent_type,
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
    # param check
    pre_type = str(req_data.get('type'))
    if not pre_type:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    if pre_type:
        if pre_type == 'smpkt':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_small_pkt/0'
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'icmp':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_icmp/0'
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'udp':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_udp/0' 
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'tcp':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_tcp/0' 
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'fraggle':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_fraggle/0' 
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'smurf':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_smurf/0' 
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'maceq':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_maceq/0' 
            status, lines = vcmd.get_status_lines(cmd)
        elif pre_type == 'ipeq':
            cmd = 'cdbctl update/cdb/sys/iptables_prevent/ipt_cfg_ipeq/0' 
            status, lines = vcmd.get_status_lines(cmd)
        else:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '% set ddos prevent error type'
            return obj
        if status != 0:
            obj['error'] = True
            obj['err_code'] = status
            obj['err_reason'] = '; '.join(lines)
            return obj
    
    return obj

if __name__ == '__main__':
    pass

