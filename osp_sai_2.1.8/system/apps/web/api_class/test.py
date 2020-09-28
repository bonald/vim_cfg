#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Aug 20, 2016

@author: wangqj
'''
from webapi import WebApi, ApiError

class TestApi(WebApi):

    def get(self, string):
        return {'number': str(type(string))}
    
    
