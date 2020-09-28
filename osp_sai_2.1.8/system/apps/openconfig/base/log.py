#!/usr/bin/python
#-*- coding: utf-8 -*-

import config
import time
import os

log_dir = config.log_dir

fp = None
fp_refresh_time = None

# check if log folder exists
if not os.path.exists(log_dir):
    os.mkdir(log_dir)
    
def _refresh_fp():
    global fp
    global fp_refresh_time
    global log_dir
    
    # close
    if fp:
        fp.close()
    
    # get time
    time_ym = time.strftime('%y_%m')
    time_day = time.strftime('%d')
    
    # check folder
    dir_str = log_dir + '/' + time_ym
    if not os.path.exists(dir_str):
        os.mkdir(dir_str)
    
    # open file
    if fp == None:
        filename = dir_str + '/' + time_day + '.log' 
        
        # check if file exists
        if not os.path.exists(filename):
            fp = open(filename, 'w')
        else:
            fp = open(filename, 'a')
    
    # refresh time
    fp_refresh_time = time_day


def write(string):
    global fp
    
    if config.log_en == False:
        return
    
    if fp == None or time.strftime('%d') != fp_refresh_time:
        _refresh_fp()
    
    time_str_in_log = time.strftime('%y-%m-%d %H:%M:%S ')
    
    # write
    fp.write(time_str_in_log + string + '\n')
    fp.flush()
        
if __name__ == '__main__':
    write('test_1')
    write('test_2')
    write('test_3')
    pass