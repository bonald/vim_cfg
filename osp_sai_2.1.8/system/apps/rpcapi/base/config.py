#!/usr/bin/python
#-*- coding: utf-8 -*-

import sys

root_dir = sys.path[0]  # eg: /var/www
secret_key = 'laU22IQ&*^!@#$fk123jsda123fas'
token_live_time = 10 # 10 minutes

log_dir = '%s/log' % root_dir
log_en = False# log enable