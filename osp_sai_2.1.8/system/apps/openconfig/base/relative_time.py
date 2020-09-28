#!/usr/bin/python
#-*- coding: utf-8 -*-

import ctypes, os, sys
import time
from vcl import vcmd

CLOCK_MONOTONIC_RAW = 4 # see <linux/time.h>

class timespec(ctypes.Structure):
    _fields_ = [
        ('tv_sec', ctypes.c_long),
        ('tv_nsec', ctypes.c_long)
    ]

class timezone(ctypes.Structure):
    _fields_ = [
        ('tz_minuteswest', ctypes.c_int),
        ('tz_dsttime', ctypes.c_int)
    ]

if sys.platform == "win32":
    is_windows = True
else: 
    librt = ctypes.CDLL('librt.so.1', use_errno=True)
    clock_gettime = librt.clock_gettime
    clock_gettime.argtypes = [ctypes.c_int, ctypes.POINTER(timespec)]
    is_windows = False

def ctc_clock_gettime():
    if is_windows == False:
        t = timespec()
        if clock_gettime(CLOCK_MONOTONIC_RAW , ctypes.pointer(t)) != 0:
            errno_ = ctypes.get_errno()
            raise OSError(errno_, os.strerror(errno_))
        return t.tv_sec + t.tv_nsec * 1e-9
    else:
        exec_str = 'cat /proc/uptime'
        status, output = vcmd.get_status_lines(exec_str)
        if status != 0:
            return time.time()
        arr = output[0].split()
        return float(arr[0])
        
def get(expire):
    return time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time() - ctc_clock_gettime() + expire))

if __name__ == '__main__':
    print get(ctc_clock_gettime() + 600)
