#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Aug 20, 2016

@author: wangqj
'''

class ApiError(Exception):
    
    def __init__(self, http_code, error_code = 1, detail = ''):
        self.http_code  = http_code;
        self.error_code = error_code
        self.detail     = detail
    
    def __str__(self):
        return '%d %s' % (self.err_code, self.err_reason)
    
    
