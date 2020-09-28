#!/usr/bin/python
#-*- coding: utf-8 -*-

from threading import Thread
from Queue import Queue
from vcl import vcmd

def run(vrf_id, dest_ip, q):
    (status, lines) = vcmd.get_status_lines('ip netns exec %s traceroute %s' % (str(vrf_id), str(dest_ip)))
    q.put('\n'.join(lines))

def wait(vrf_id, dest_ip, timeout):
    q = Queue()
    
    t = Thread(target=run, args=(vrf_id, dest_ip, q))
    t.setDaemon(True)
    t.start()
    
    timeout = timeout if timeout > 1 else 1
    res = ''
    
    try:
        res = q.get(timeout=timeout)
    except Exception:
        timeout = True
        return None
        
    return res

def get(vrf_id, dest_ip, timeout):
    """
    TBD
    """
    
    _err_reason = [
        '',                 # err_code: 0
        'cmd excute error', # err_code: 1
    ]
    
    obj = {'trace_resp': '', 'error': False, 'err_code': 0, 'err_reason': ''}
    
    res = wait(vrf_id, dest_ip, timeout)
    obj['trace_resp'] = 'timeout!' if res == None else res
    
    return obj
