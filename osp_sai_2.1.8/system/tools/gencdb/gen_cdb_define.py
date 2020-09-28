#!/usr/bin/python
import os
import sys
import string
import CONST
import cdb_util
import tbl_pb
import ds_pb
import commands


import re
from stat import *

outputFileName = 'cdb_define.py'
outputConstName = 'cdb_const_tmp.py'
#PBC_PY_PATH             = 'protobuf/py/
#CDB_H_PATH              = 'core/include/gen/'


def creatCDBDefine(infile, outfile) :
        tmp = open(outfile, 'w')
        tmp.close()

def writeCDBDefine(infile, outfile):
        sourceFile = open(infile, 'r')
        print infile, outfile
        to = open(outfile, 'a')
        start = 0

        strLine = sourceFile.readline()
        while strLine:
            strLine = strLine.strip('\n')
            strLine = strLine.strip('\r')
            if strLine.find('typedef enum') >= 0:
                start =  1
                
            if start == 1 and strLine.find('{') < 0 and strLine.find('}') < 0 and strLine.find('typedef enum') < 0 and strLine.find('=') > 0:
                tmp = strLine.split(',')
                tmp1 =  tmp[0].lstrip(' ')
                to.write(tmp1 + '\n')
                
            if start == 1 and strLine.find('}') >= 0:
                start = 0
                to.write('\n')
                
            try:
                strLine = sourceFile.readline()
            except:
                    error_return("Error at line %s", strLine)
                    
        sourceFile.close()
        to.close()
            
 
creatCDBDefine(CONST.CDB_H_PATH, CONST.PBC_PY_PATH + outputFileName)
#creatCDBDefine(CONST.CDB_H_PATH, CONST.PBC_PY_PATH + outputConstName)
#writeCDBDefine(CONST.CDB_H_PATH + 'cdb_define.h', CONST.PBC_PY_PATH + outputConstName)
for f in os.listdir(CONST.CDB_H_PATH):
        pathname = os.path.join(CONST.CDB_H_PATH, f)
        if pathname.find('define') > 0:
            writeCDBDefine(pathname, CONST.PBC_PY_PATH + outputFileName)

os.system('cp ' + CONST.PBC_PY_PATH + outputFileName + ' ' + '../../apps/mibd/')
os.system('cp ' + CONST.PBC_PY_PATH + '*.*' + ' ' + '../../apps/mibd/')
print 'cp ' + CONST.PBC_PY_PATH + '*.*' + ' ' + '../../apps/mibd/'
#os.system('cp ' + CONST.PBC_PY_PATH + outputConstName + ' ' + '../../apps/mibd/')
