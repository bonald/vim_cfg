#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd

def get():
    """
    API:

    GET:

    Retrun: {
        log_arr: [
            {
                time: time
                module: module and time
                enent:event
                severity:severity level
                log: syslog,
                sysname: hostname
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
            'log is null', # err_code: 1
            ]

    obj = {'log_arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    arr = vcmd.get_showarr('cdbctl show/cdb/app/syslog_cfg/buffer/300')
    if len(arr[0]) > 1:
        for line in arr:
            #line = "Mar  3 02:09:21 Switch syslog-ng-6: -- Syslog is alive --"
            tmp = {}
            tmp['time']             = line[:15]
            index = line.find('%')
            if (line.find('syslog-ng') > 0):
                module_index = line.find('syslog-ng-')
            else:
                module_index = line.find('-')
            if index >= 0 and module_index >=0:
                tmp['module'] = line[index+1:module_index]
            elif module_index >=0:
                item = line[15:module_index].split()
                if len(item) > 1:
                    tmp['module'] = item[1]
                else:
                    continue
            tmp['severity'] = line[module_index+1]
            line1 = line[module_index:]
            event_index = line1.find(':')
            tmp['event'] = line1[3:event_index]
            if len(tmp['event']) < 1:
                tmp['event'] = "N/A"

            if event_index >=0:
                tmp['log'] = line1[event_index+1:]

            obj['log_arr'].append(tmp)

    else:
        return obj

    return obj

def flush():
    """
    API:

    DELETE: {
        vlan_ids: [int, int, ....]
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
            'cannot remove vlan 1', # err_code: 2
            'excute error',         # err_code: 3
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    res = vcmd.execute('cdbctl delete/cdb/app/syslog_cfg/clearbuf')
    # param check
    if res:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;
    return obj

def log_cfg_get():
    """
    API:

    DELETE: {
        vlan_ids: [int, int, ....]
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
            'cannot remove vlan 1', # err_code: 2
            'excute error',         # err_code: 3
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    status, arr = vcmd.get_status_arr('cdbctl read/cdb/app/syslog_cfg/')

    # param check
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj

    else: 
        # '64,1.1.1.1;64,1.1.1.2;0,1.1.1.3;' -> {1.1.1.1:64, 1.1.1.2:64, 1.1.1.3:0}
        temp = arr[0].get('server_addr')
        if len(temp) != 0:
            temp = temp[:(len(temp)-1)]
            #temp = temp.replace('64,', '')
            temp = temp.split(';')

            addMap = {}
            for entry in temp:
                entry = entry.split(',')
                addMap[entry[1]] = entry[0]  
            arr[0]['server_addr'] = addMap
        else:
            temp = []

        obj['cfg']= arr[0] 

    return obj

def log_mgr_update_status(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request',          # err_code: 1
            'cannot remove vlan 1', # err_code: 2
            'excute error',         # err_code: 3
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    type   = str(req_data.get('type'))
    enable = str(req_data.get('status'))

    if type == 'server':
        status, lines = vcmd.get_status_lines('cdbctl update/cdb/app/syslog_cfg/enable_to_server/%s' % enable)

        # param check
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 1;
            obj['err_reason'] = _err_reason[1];
            return obj;

    if type == 'cache':
        status, lines = vcmd.get_status_lines('cdbctl update/cdb/app/syslog_cfg/enable_to_file/%s' % enable)

        # param check
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 1;
            obj['err_reason'] = _err_reason[1];
            return obj;

    if type == 'merge':
        status, lines = vcmd.get_status_lines('cdbctl update/cdb/app/syslog_cfg/enable_merge/%s' % enable)

        # param check
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 1;
            obj['err_reason'] = _err_reason[1];
            return obj;

    return obj  

def log_mgr_update_level(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request',          # err_code: 1
            'cannot remove vlan 1', # err_code: 2
            'excute error',         # err_code: 3
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    type  = str(req_data.get('type'))
    level = str(req_data.get('level'))

    if type == 'system':
        status, lines = vcmd.get_status_lines('cdbctl update/cdb/app/syslog_cfg/file_severity/%s' % level)

        # param check
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 1;
            obj['err_reason'] = _err_reason[1];
            return obj;

    if type == 'severity':
        status, lines = vcmd.get_status_lines('cdbctl update/cdb/app/syslog_cfg/server_severity/%s' % level)

        # param check
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 1;
            obj['err_reason'] = _err_reason[1];
            return obj;

    if type == 'time':
        status, lines = vcmd.get_status_lines('cdbctl update/cdb/app/syslog_cfg/timestamp/%s' % level)

        # param check
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 1;
            obj['err_reason'] = _err_reason[1];
            return obj;

    if type == 'cache':
        status, lines = vcmd.get_status_lines('cdbctl update/cdb/app/syslog_cfg/module_severity/%s' % level)

        # param check
        if status != 0:
            obj['error'] = True

            obj['err_code'] = 1;
            obj['err_reason'] = _err_reason[1];
            return obj;

    return obj  

def log_mgr_update_buffer_size(req_data):
    """
    TBD
    """    

    _err_reason = [
            '',                     # err_code: 0
            'bad request',          # err_code: 1
            'cannot remove vlan 1', # err_code: 2
            'excute error',         # err_code: 3
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    size = str(req_data.get('size'))

    status, lines = vcmd.get_status_lines('cdbctl update/cdb/app/syslog_cfg/logging_lines/%s' % size)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj  

def log_mgr_srv_addr_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request',          # err_code: 1
            'cannot remove vlan 1', # err_code: 2
            'excute error',         # err_code: 3
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    ipaddr  = str(req_data.get('serveraddr'))
    vrf     = str(req_data.get('inband'))

    if vrf == 'default':
        cmd_add_ns = 'cdbctl create/cdb/sys/ns_route/1#514#%s/4/inband/' % ipaddr
        cmd_create_srv = 'cdbctl update/cdb/app/syslog_cfg/server_addr/0/%s' % ipaddr
        cmd_update_srv = 'cdbctl delete/cdb/sys/ns_route/1#514#%s' % ipaddr
    else:
        cmd_add_ns = 'cdbctl create/cdb/sys/ns_route/1#514#%s/4//' % ipaddr
        cmd_create_srv = 'cdbctl update/cdb/app/syslog_cfg/server_addr/64/%s' % ipaddr
        cmd_update_srv = 'cdbctl delete/cdb/sys/ns_route/1#514#%s' % ipaddr

    status, lines = vcmd.get_status_lines(cmd_add_ns)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    status, lines = vcmd.get_status_lines(cmd_create_srv)
    if status != 0:

        # rollback process
        vcmd.get_status_lines(cmd_update_srv)

        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    return obj      

def log_mgr_srv_addr_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                     # err_code: 0
            'bad request',          # err_code: 1
            'cannot remove vlan 1', # err_code: 2
            'excute error',         # err_code: 3
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    ipaddr = str(req_data.get('serveraddr'))
    vrf    = str(req_data.get('vrf'))

    if vrf == 'default':
        cmd_del_ns = 'cdbctl delete/cdb/sys/ns_route/1#514#%s/inband/' % ipaddr
        cmd_del_srv = 'cdbctl delete/cdb/app/syslog_cfg/server_addr/0/%s' % ipaddr

    else:
        cmd_del_ns = 'cdbctl delete/cdb/sys/ns_route/1#514#%s/outband/' % ipaddr
        cmd_del_srv = 'cdbctl delete/cdb/app/syslog_cfg/server_addr/64/%s' % ipaddr

    status, lines = vcmd.get_status_lines(cmd_del_srv)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1;
        obj['err_reason'] = _err_reason[1];
        return obj;

    vcmd.get_status_lines(cmd_del_ns)

    return obj

if __name__ == '__main__':
    pass
