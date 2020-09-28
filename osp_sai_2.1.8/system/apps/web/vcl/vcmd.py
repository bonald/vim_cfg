#!/usr/bin/python
#-*- coding: utf-8 -*-

import config
from remote import Remote
from local import Local
from base import log
import types

ins = Remote() if config.is_dev else Local()
_remote_cmd_max_len = 800
_escape_obj = { # str: replace_to
    '$': r'\$',
    '&': r'\&',
    '#': r'\#',
    ';': r'\;',
    '?': r'\?',
    '[': r'\[',
    ']': r'\]',
    '(': r'\(',
    ')': r'\)',
    '*': r'\*',
    '|': r'\|',
    '>': r'\>'
}

def getRemoteIP():
    rip = ins.getLocalIP() if config.is_dev else None
    return rip

def _decorate_split_cmds_if_use_remote(fn):
    def _deco(cmd):
        res = None
        convert_to_tuple = False
        # cmd is string
        if type(cmd) != types.ListType:
            res = fn(cmd)
        # cmd is list
        else:
            # local
            if not config.is_dev:
                res = fn('&&'.join(cmd))
            # remote
            else:
                # get split index
                cmd_split_arr = []
                cmd_len = 0
                for i in range(len(cmd)):
                    cmd_len += 2 + len(cmd[i]) # 2 means len('&&')
                    if cmd_len >= _remote_cmd_max_len:
                        cmd_split_arr.append(i - 1)
                        cmd_len = 0
                        
                # without split
                if not cmd_split_arr:
                    res = fn('&&'.join(cmd))
                # split
                else:
                    cmd_arr = []
                    current = 0
                    for i in cmd_split_arr:
                        cmd_arr.append(cmd[current:i])
                        current = i
                    cmd_arr.append(cmd[current:])
                    # excute
                    for cmds in cmd_arr:
                        cmd_str = '&&'.join(cmds)
                        res_tmp = fn(cmd_str)
                        if res_tmp != None:
                            # excute
                            if type(res_tmp) == types.IntType:
                                res = res_tmp
                                if res != 0:
                                    break
                            # get lines
                            elif type(res_tmp) == types.ListType:
                                if res == None:
                                    res = []
                                res.extend(res_tmp)
                            # get status lines
                            elif type(res_tmp) == types.TupleType:
                                if res == None:
                                    res = [0, []]
                                status, lines = res_tmp
                                res[0] = status
                                res[1].extend(lines)
                                if status != 0:
                                    break
                                convert_to_tuple = True
                    if convert_to_tuple:
                        res = tuple(res)
        return res
    return _deco

def _line_to_obj_without_nesting(line):
    obj = {}
    arr = []

    # line to obj
    if line[0] == '/':
        line = line[1:]
        
    # split items and values
    res = line.split('=')
    if(len(res) == 2):
        obj = {res[0]: res[1]}
    elif(len(res) > 2):
        arr.append(res[0])
        for i in res[1:-1]:
            cnt = i.count('/')
            if cnt == 1:
                for item in i.split('/'):
                    arr.append(item)
            elif cnt > 1:
                index = i.rfind('/')
                arr.append(i[0:index])
                arr.append(i[index + 1:])
        arr.append(res[-1])
        
    # make obj
    for i in range(len(arr) / 2):
        key = arr[i * 2]
        value = arr[i * 2 + 1]
        obj[key] = value
    
    return obj
    
    
def _split_line(line):
    '''
    e.g. '/key={/c=1,/c=2,}/a=1,/key={/c=3,/d=3,}/a=2,' -> ['/key={/c=1,/c=2,}/a=1', '/key={/c=3,/d=3,}/a=2']
         '/key=12.1.1.1#24/a=1,/key=12.2.2.2#24/a=2,' -> ['/key=12.1.1.1#24/a=1', '/key=12.2.2.2#24/a=2']
         '/key=value/key1=value1' -> '/key=value/key1=value1'
         '/key={/c=1,1,/c=2,2,}/a=1,/key={/c=3,3,/d=3,3,}/a=2,' -> ['/key={/c=1,1,/c=2,2,}/a=1', '/key={/c=3,3,/d=3,3,}/a=2']
    '''
    split_index_arr = []
    sub_dict_stack = []
    
    if not line:
        return line
    
    # delete last ','
    if line[-1] == ',':
        line = line[:-1]
    
    for i in range(len(line)):
        tmp = line[i]
        if tmp == '{':
            sub_dict_stack.append(i)
        elif tmp == '}':
            sub_dict_stack.pop()
        else:
            if len(sub_dict_stack) == 0: # check enable
                if tmp == ',' and (line[i + 1] == '/' or line[i + 1] == '}'):
                    split_index_arr.append(i)
                
    if len(split_index_arr) == 0: # not exist array
        return line
    else:
        ret_arr = []
        ptr = 0
        for index in split_index_arr:
            ret_arr.append(line[ptr:index].strip())
            ptr = index + 1
        ret_arr.append(line[ptr:].strip())
        
        # delete '\/$'
        ret_arr_tmp = []
        for tmp in ret_arr:
            if tmp[-1] == '/':
                tmp = tmp[:-1]
            ret_arr_tmp.append(tmp)
        return ret_arr_tmp

def _line_to_obj(line):
    obj = {}
    sub_dict = []   # [{key: string, value: string, str: string}, ....], only current layer
                    # e.g. '/key1=val1/sub={/key2=val2}' -> [{key: 'sub', value: '/key2=val2', str: '/sub={/key2=val2}'}]
    sub_dict_index_stack = []
    single_layer_line = line
    
    # get sub_dict
    for i in range(len(line)):
        if line[i] == '{':
            sub_dict_index_stack.append(i)
        elif line[i] == '}':
            start = sub_dict_index_stack.pop()
        else:
            continue
        
        # find sub dict
        if len(sub_dict_index_stack) == 0:
            # get value
            value = line[start+1 : i]
            # get key
            key_index = line[0 : start-1].rindex('/') + 1
            key = line[key_index : start-1]
            # get str
            string = line[key_index-1 : i+1]
            # append
            sub_dict.append({'key': key, 'value': value, 'str': string})
        
    
    # get single layer line
    for i in sub_dict:
        single_layer_line = single_layer_line.replace(i['str'], '')
        # get sub layer dict
        tmp_key = i['key']
        value = _split_line(i['value'])
        
        if type(value) == types.ListType:
            tmp_arr = []
            for val in value:
                tmp_arr.append(_line_to_obj(val))
            obj[tmp_key] = tmp_arr
        else:
            obj[tmp_key] = _line_to_obj(value)
        
        
    if len(single_layer_line) > 0:
        # merge obj
        obj = dict(_line_to_obj_without_nesting(single_layer_line), **obj)
    
    return obj

def _line_to_obj_simple(line):
    obj = {}
    
    for i in line.split('/'):
        if i:
            tmp = i.split('=')
            if len(tmp) == 2:
                obj[tmp[0]] = tmp[1]
    
    return obj 


def _get_arr(lines):
    arr = []
    for line in lines:
        tmp = _line_to_obj(line)
        if tmp:
            arr.append(tmp)
    return arr

def _get_arr_simple(lines):
    arr = []
    for line in lines:
        tmp = _line_to_obj_simple(line)
        if tmp:
            arr.append(tmp)
    return arr

def send(cmd):
    """excute cmd only, don't wait the response
    """
    ins.send(cmd)
    
@_decorate_split_cmds_if_use_remote
def execute(cmd):
    """only execute command, just return $?
    """
    res = ins.execute(cmd)
    log.write('excute: %s\n    %s' % (str(cmd), str(res)))
    return res

@_decorate_split_cmds_if_use_remote
def get_lines(cmd):
    """get cmd response
    """
    res = ins.cmd(cmd)
    log.write('get_lines: %s\n    %s' % (str(cmd), str(res)))
    return res

def get_arr(cmd):
    """send cmd and convert response to dict array
    """
    res = ins.cmd(cmd)
    arr = _get_arr(res)
    log.write('get_arr: %s\n    %s' % (str(cmd), str(arr)))
    return arr

def get_showarr(cmd):
    """send cmd and convert response to dict array
    """
    res = get_lines(cmd)
    log.write('get_showarr: %s\n    %s' % (cmd, str(res)))
    return res

def get_arr_simple(cmd):
    """send cmd and convert response to dict array, parse single layer
    """
    res = ins.cmd(cmd)
    arr = _get_arr_simple(res)
    log.write('get_arr: %s\n    %s' % (str(cmd), str(arr)))
    return arr

@_decorate_split_cmds_if_use_remote
def get_status_lines(cmd):
    """return status code and output lines array
    """
    (status, lines) = ins.get_status_lines(cmd)
    log.write('get_status_lines: %s\n    %s - %s' % (cmd, str(status), str(lines)))
    return (status, lines)

def get_status_lines_sp(cmd):
    """return status code and output lines array
    """
    (status, lines) = ins.get_status_lines_sp(cmd)
    log.write('get_status_lines: %s\n    %s - %s' % (cmd, str(status), str(lines)))
    return (status, lines)

def get_status_arr(cmd):
    """return status code and dict array
    """
    (status, lines) = ins.get_status_lines(cmd)
    if status != 0:
        log.write('get_status_arr ERROR: %s\n    %s - %s' % (cmd, str(status), str(lines)))
        return (status, lines)
    else:
        try:
            arr = _get_arr(lines)
            log.write('get_status_arr: %s\n    %s - %s' % (cmd, str(status), str(arr)))
            return (status, arr)
        except Exception:
            return (127, ['analyze the cmd output error'])
    
def get_status_arr_simple(cmd):
    """return status code and dict array, single layer
    """
    (status, lines) = ins.get_status_lines(cmd)
    if status != 0:
        log.write('get_status_arr ERROR: %s\n    %s - %s' % (cmd, str(status), str(lines)))
        return (status, lines)
    else:
        try:
            arr = _get_arr_simple(lines)
            log.write('get_status_arr: %s\n    %s - %s' % (cmd, str(status), str(arr)))
            return (status, arr)
        except Exception:
            return (127, ['analyze the cmd output error'])

def cdbctl(*args):
    """excute cdbctl, escape special character such as '$', '&'
    for example:
        vmcd.cdbctl('read', 'cdb', 'l2', 'vlan') will execute 'cdbctl read/cdb/l2/vlan'
    """
    if len(args) <= 0:
        raise Exception('args is null')

    arr = []
    for arg in args:
        if type(arg) != types.StringType:
            return (127, ['arg "%s" is not a string' % arg])
        for key in _escape_obj:
            arg = arg.replace(key, _escape_obj[key])
        arr.append(arg)
    cmd = 'cdbctl %s' % '/'.join(arr)
    return get_status_arr(cmd)
    
def cmd_get_token(szMsg, szToken):
    tlen = len(szToken)
    tidx = szMsg.find("/" + str(szToken) + "=")
    if -1 == tidx:
        return None
    nidx = tidx + tlen + 2
    if nidx == tidx:
        # the last token and empty value
        return ""
    if '{' == szMsg[nidx:nidx + 1]:
        # it's {...} entry
        vidx = szMsg[nidx:].find('}')
        if -1 == vidx:
            return None
        return szMsg[nidx + 1:nidx + vidx]
    # not {...} entry
    vidx = szMsg[nidx:].find('/')
    if -1 == vidx:
        # the last token
        return szMsg[nidx:]
    return szMsg[nidx: nidx + vidx]
    
if __name__ == '__main__':
    #    print get_lines('pwd')
#    print _line_to_obj('/mode=flash:/image=wangqj_web_test03.d/serverip=/ipaddr=/bootcmd=UNKNOWN/current_image=flash:/boot/wangqj_web_test03.d')    
#    print _line_to_obj('/mode=flash:')        
#    print 'execute:', execute(['echo %d' % x for x in range(1000)]) print 'execute:', execute(['echo %d' % x for x in range(1000)])
#    print 'get_lines:', get_lines(['echo %d' % x for x in range(1000)])
#    print 'get_status_lines:', get_status_lines(['echo %d' % x for x in range(1000)])

#    print _line_to_obj('/key=agg1/brgif={/pvid=1/security_list=}/lag={/mode=active/member_ports=eth-0-1,}/lacp={}/storm_control={}/aclqos_if={}/openflow_if={}/crc_error_check_en=0/dot1x_guest_vlan=0')
#    print _line_to_obj('/lag={/mode=active/member_ports=eth-0-1/sub_dict={/sub_key1=sub_val1/sub_key2=sub_val2/sub_sub_dict={/a=1/b=2}}/aaa=1}')
#    import time

#    t1 = time.time()
#    get_status_lines('cdbctl read/cdb/l2/vlan')
#    t2 = time.time()
#    print t2 - t1
#    
#    t1 = time.time()
#    get_status_arr('cdbctl read/cdb/l2/vlan')
#    t2 = time.time()
#    print t2 - t1

#    print get_status_arr_simple('cdbctl read/cdb/sys/bootimage')
#    print _line_to_obj('/ifc_ipv4={/key=12.1.1.1#24/a=1,1,/key=12.2.2.2#24/a=2,2,}/ifc_ipv6={/key={/c=1,/c=2,}/a=1,/key={/c=3,/d=3,}/a=2,}')
#    print _line_to_obj('/key=eth-0-12_1/mtu=1514/mac=001e.080a.4e01/ipv4_enable=0/ipv6_enable=0/arp_retrans_time=1/arp_timeout=3600/ifindex=0/kernel_ifindex=0/arp_retry_timeout=0/arp_proxy_en=0/local_arp_proxy_en=0/unicast_rpf_en=0/ip_unreachable_en=1/ip_redirects_en=1/dhcp_relay_option_trust=0/dhcp_server_group=0/dhcp_pdu_enabled=0/ifc_ipv4={/key=12.1.1.1#24/destination=12.1.1.255#24/flags_secondary=0/flags_anycast=0/flags_virtual=0/flags_duplicate=0/installed=0/,/key=12.2.2.2#24/destination=12.2.2.255#24/flags_secondary=1/flags_anycast=0/flags_virtual=0/flags_duplicate=0/installed=0/,}/ifc_ipv6={}')

#    print _line_to_obj('/x={/key={/c=1,/c=2,}/a=1,/key={/d=3,/d=3,}/a=2,}')
#    print _line_to_obj('/x={/key={/c=1,1,/c=2,2,}/a=1,/key={/d=3,3,/d=3,3,}/a=2,}')

#    print _split_line('/key={/c=1,/c=2,}/a=1,/key={/c=3,/d=3,}/a=2,')
#    print _split_line('/key={/c=1,1,/c=2,2,}/a=1,/key={/c=3,3,/d=3,3,}/a=2,')
    print _split_line('/a={/name=a/key=eth1,eth2,eth3,}')
    print _line_to_obj('/ifc_ipv4={/key=21.1.1.1#24/destination=21.1.1.255#24/flags_secondary=1/flags_anycast=0/flags_virtual=0/flags_duplicate=0/installed=0/,/key=31.1.1.1#24/destination=31.1.1.255#24/flags_secondary=1/flags_anycast=0/flags_virtual=0/flags_duplicate=0/installed=0/,/key=40.1.1.1#24/destination=40.1.1.255#24/flags_secondary=0/flags_anycast=0/flags_virtual=0/flags_duplicate=0/installed=0/,/key=41.1.1.1#24/destination=41.1.1.255#24/flags_secondary=1/flags_anycast=0/flags_virtual=0/flags_duplicate=0/installed=0/,/key=51.1.1.1#24/destination=51.1.1.255#24/flags_secondary=1/flags_anycast=0/flags_virtual=0/flags_duplicate=0/installed=0/,/key=61.1.1.1#24/destination=61.1.1.255#24/flags_secondary=1/flags_anycast=0/flags_virtual=0/flags_duplicate=0/installed=0/,/key=71.1.1.1#24/destination=71.1.1.255#24/flags_secondary=1/flags_anycast=0/flags_virtual=0/flags_duplicate=0/installed=0/,/key=81.1.1.1#24/destination=81.1.1.255#24/flags_secondary=1/flags_anycast=0/flags_virtual=0/flags_duplicate=0/installed=0/,/key=91.1.1.1#24/destination=91.1.1.255#24/flags_secondary=1/flags_anycast=0/flags_virtual=0/flags_duplicate=0/installed=0/,}')
    print _line_to_obj('/ifc_ipv4={/key=21.1.1.1#24/,/key=31.1.1.1#24/,/key=40.1.1.1#24/,/key=41.1.1.1#24/,/key=51.1.1.1#24/,/key=61.1.1.1#24/,/key=71.1.1.1#24/,/key=81.1.1.1#24/,/key=91.1.1.1#24/,} ')
#    print _split_line('/key=21.1.1.1#24/,/key=31.1.1.1#24/,/key=40.1.1.1#24/,/key=41.1.1.1#24/,/key=51.1.1.1#24/,/key=61.1.1.1#24/,/key=71.1.1.1#24/,/key=81.1.1.1#24/,/key=91.1.1.1#24/,')
#    print _split_line('/key=val1/,/key=val2/,/key=val3/,/key=val4/,')
    pass



