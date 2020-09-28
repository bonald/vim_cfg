#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import string
import base64
import base
import types
import portMapping

portMap = portMapping.portNameMapping()

def get(key):
    """
    API:

    GET:

    Retrun: {
        if_statistic_arr: [
            {
                ifidx: int,
                octets_rcv_rate: int,
                pkts_rcv_rate: int,
                octets_send_rate: int,
                pkts_send_rate: int,
                octets_rcv: int,
                pkts_rcv: int,
                uc_pkts_rcv: int,
                brdc_pkts_rcv: int,
                mc_pkts_rcv: int,
                oam_pkts_rcv: int,
                undersize_pkts: int,
                oversize_pkts: int,
                mac_rcv_error: int,
                bad_crc: int,
                frame_error: int,
                drop_events: int,
                pause_rcv: int,
                octets_send: int,
                pkts_send: int,
                uc_pkts_send: int,
                brdc_pkts_send: int,
                mc_pkts_send: int,
                oam_pkts_send: int,
                underruns: int,
                mac_transmit_err: int,
                pause_send: int,
                fcs_pkts_rcv: int,
                fcs_octets_rcv: int,
                fcs_pkts_send: int,
                fcs_octets_send: int,
                fragments_pkts: int,
                bad_pkts_rcv: int,
                bad_octets_rcv: int,
                ifname: string
            },
            ...
            ...
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    _err_reason = [
        '',                    # err_code: 0
        'interface statistic is not exist',         # err_code: 1
    ]

    obj = {'if_statistic_arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}
    
    arr_if = vcmd.get_arr('cdbctl read/cdb/interface')

    if key == '' or key == 'all':
        arr = vcmd.get_arr('cdbctl read/cdb/sys/port_stats')
        if arr:
            for i in arr:
                tmp = {}
                tmp['ifidx'] = i.get('key')
                if arr_if:
                    for j in arr_if:
                        if j.get('ifindex') == i.get('key'):
                            tmp['ifname'] = j.get('key')
                            tmp['sort_name'] = portMap.Mapping(tmp['ifname'])
                            tmp['full_name'] = portMap.Mapping(tmp['ifname'], is_sort=False)
                            break
                else:
                    tmp['ifname'] = ''

                if j.get('ifg_stats_en') == "1":
                    octets_rcv = str(int(i.get('octets_rcv')) + int(i.get('pkts_rcv')) * 20)
                    tmp['octets_rcv'] = octets_rcv
                else:
                    tmp['octets_rcv'] = i.get('octets_rcv')

                tmp['pkts_rcv'] = i.get('pkts_rcv')
                tmp['uc_pkts_rcv'] = i.get('uc_pkts_rcv')
                tmp['brdc_pkts_rcv'] = i.get('brdc_pkts_rcv')
                tmp['mc_pkts_rcv'] = i.get('mc_pkts_rcv')
                tmp['oam_pkts_rcv'] = i.get('oam_pkts_rcv')
                tmp['undersize_pkts'] = i.get('undersize_pkts')
                tmp['oversize_pkts'] = i.get('oversize_pkts')
                tmp['mac_rcv_error'] = i.get('mac_rcv_error')
                tmp['bad_crc'] = i.get('bad_crc')
                tmp['frame_error'] = i.get('frame_error')
                tmp['drop_events'] = i.get('drop_events')
                tmp['pause_rcv'] = i.get('pause_rcv')

                if j.get('ifg_stats_en') == "1":
                    octets_send = str(int(i.get('octets_send')) + int(i.get('pkts_send')) * 20)
                    tmp['octets_send'] = octets_send
                else:
                    tmp['octets_send'] = i.get('octets_send')

                tmp['pkts_send'] = i.get('pkts_send')
                tmp['uc_pkts_send'] = i.get('uc_pkts_send')
                tmp['brdc_pkts_send'] = i.get('brdc_pkts_send')
                tmp['mc_pkts_send'] = i.get('mc_pkts_send')
                tmp['oam_pkts_send'] = i.get('oam_pkts_send')
                tmp['underruns'] = i.get('underruns')
                tmp['mac_transmit_err'] = i.get('mac_transmit_err')
                tmp['pause_send'] = i.get('pause_send')
                tmp['fcs_pkts_rcv'] = i.get('fcs_pkts_rcv')
                tmp['fcs_octets_rcv'] = i.get('fcs_octets_rcv')
                tmp['fcs_pkts_send'] = i.get('fcs_pkts_send')
                tmp['fcs_octets_send'] = i.get('fcs_octets_send')
                tmp['fragments_pkts'] = i.get('fragments_pkts')
                tmp['bad_pkts_rcv'] = i.get('bad_pkts_rcv')
                tmp['bad_octets_rcv'] = i.get('bad_octets_rcv')
                if arr_if:
                    for j in arr_if:
                        if j.get('ifindex') == i.get('key'):
                            tmp['ifname'] = j.get('key')
                            tmp['sort_name'] = portMap.Mapping(tmp['ifname'])
                            tmp['full_name'] = portMap.Mapping(tmp['ifname'], is_sort=False)
                            break
                else:
                    tmp['ifname'] = ''
                    
                arr_rate = vcmd.get_arr('cdbctl read/cdb/sys/port_stats_rate')  
                if arr_rate:
                    for j in arr_rate:
                        if j.get('key') == i.get('key'):
                            tmp['octets_rcv_rate'] = j.get('octets_rcv_rate')
                            tmp['pkts_rcv_rate'] = j.get('pkts_rcv_rate')
                            tmp['octets_send_rate'] = j.get('octets_send_rate')
                            tmp['pkts_send_rate'] = j.get('pkts_send_rate')
                        break
                else:
                    tmp['octets_rcv_rate'] = 0
                    tmp['pkts_rcv_rate'] = 0
                    tmp['octets_send_rate'] = 0
                    tmp['pkts_send_rate'] = 0
                    
                obj['if_statistic_arr'].append(tmp)

        else:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = _err_reason[1]
    else:
        arr = vcmd.get_lines('cdbctl read/cdb/sys/port_stats/%s' % key)
        status, lines = vcmd.get_status_lines()
        if arr[0].find('%') >= 0:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = arr
        else:
            arr = vcmd.get_arr('cdbctl read/cdb/sys/port_stats/%s' % key)
            
            if arr:
                for i in arr:
                    tmp = {}
                    tmp['ifidx'] = i.get('key')
                    #tmp['octets_rcv_rate'] = i.get('octets_rcv_rate')
                    #tmp['pkts_rcv_rate'] = i.get('pkts_rcv_rate')
                    #tmp['octets_send_rate'] = i.get('octets_send_rate')
                    #tmp['pkts_send_rate'] = i.get('pkts_send_rate')
                    tmp['octets_rcv'] = i.get('octets_rcv')
                    tmp['pkts_rcv'] = i.get('pkts_rcv')
                    tmp['uc_pkts_rcv'] = i.get('uc_pkts_rcv')
                    tmp['brdc_pkts_rcv'] = i.get('brdc_pkts_rcv')
                    tmp['mc_pkts_rcv'] = i.get('mc_pkts_rcv')
                    tmp['oam_pkts_rcv'] = i.get('oam_pkts_rcv')
                    tmp['undersize_pkts'] = i.get('undersize_pkts')
                    tmp['oversize_pkts'] = i.get('oversize_pkts')
                    tmp['mac_rcv_error'] = i.get('mac_rcv_error')
                    tmp['bad_crc'] = i.get('bad_crc')
                    tmp['frame_error'] = i.get('frame_error')
                    tmp['drop_events'] = i.get('drop_events')
                    tmp['pause_rcv'] = i.get('pause_rcv')
                    tmp['octets_send'] = i.get('octets_send')
                    tmp['pkts_send'] = i.get('pkts_send')
                    tmp['uc_pkts_send'] = i.get('uc_pkts_send')
                    tmp['brdc_pkts_send'] = i.get('brdc_pkts_send')
                    tmp['mc_pkts_send'] = i.get('mc_pkts_send')
                    tmp['oam_pkts_send'] = i.get('oam_pkts_send')
                    tmp['underruns'] = i.get('underruns')
                    tmp['mac_transmit_err'] = i.get('mac_transmit_err')
                    tmp['pause_send'] = i.get('pause_send')
                    tmp['fcs_pkts_rcv'] = i.get('fcs_pkts_rcv')
                    tmp['fcs_octets_rcv'] = i.get('fcs_octets_rcv')
                    tmp['fcs_pkts_send'] = i.get('fcs_pkts_send')
                    tmp['fcs_octets_send'] = i.get('fcs_octets_send')
                    tmp['fragments_pkts'] = i.get('fragments_pkts')
                    tmp['bad_pkts_rcv'] = i.get('bad_pkts_rcv')
                    tmp['bad_octets_rcv'] = i.get('bad_octets_rcv')
                    if arr_if:
                        for j in arr_if:
                            if j.get('ifindex') == i.get('key'):
                                tmp['ifname'] = j.get('key')
                                tmp['sort_name'] = portMap.Mapping(tmp['ifname'])
                                tmp['full_name'] = portMap.Mapping(tmp['ifname'], is_sort=False)
                                break
                        else:
                            tmp['ifname'] = ''
                            
                    arr_rate = vcmd.get_arr('cdbctl read/cdb/sys/port_stats_rate/%s' % key)
                    if arr_rate:
                        for j in arr_rate:
                            if j.get('key') == j.get('key'):
                                tmp['octets_rcv_rate'] = j.get('octets_rcv_rate')
                                tmp['pkts_rcv_rate'] = j.get('pkts_rcv_rate')
                                tmp['octets_send_rate'] = j.get('octets_send_rate')
                                tmp['pkts_send_rate'] = j.get('pkts_send_rate')
                            break
                    else:
                        tmp['octets_rcv_rate'] = 0
                        tmp['pkts_rcv_rate'] = 0
                        tmp['octets_send_rate'] = 0
                        tmp['pkts_send_rate'] = 0
                    obj['if_statistic_arr'].append(tmp)
            else:
                obj['error'] = True
                obj['err_code'] = 1
                obj['err_reason'] = _err_reason[1]

    return obj



import time
def clear(req_data):
    """
    API:
    
    PUT: {
        ifidx_arr: [int, int, ....]
    }
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    """
    
    _err_reason = [
        '',                     # err_code: 0
        'bad request',          # err_code: 1
        'cannot clear stats',   # err_code: 2
        'excute error',         # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    res, err = base.check.dictionary(req_data, [
        ['ifidx_arr', 'array', 'bad request: ifindex', 1],
    ])

    if not res:
        obj['error'] = True
        obj['err_code'] = err['err_code'];
        obj['err_reason'] = err['err_reason'];
        return obj;
    #ifidx, err = base.check.array(res['ifidx_arr'], ['string', '>=', 1, 'and', '<=', 65535, 'ifindex is out of range', 1])
    #if not ifidx:
    #    obj['error'] = True
    #    obj['err_code'] = err['err_code'];
    #    obj['err_reason'] = err['err_reason'];
    #    return obj;

    status, output = vcmd.get_status_lines(['cdbctl update/cdb/sys/port_stats/%s ;cdbctl update/cdb/sys/clear_if_stats_rate/%s' % (x, x) for x in res['ifidx_arr']])
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2];
    
    return obj

if __name__ == '__main__':
    pass

