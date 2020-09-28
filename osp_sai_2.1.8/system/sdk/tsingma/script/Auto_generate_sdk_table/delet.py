import sys
import re
file_tm = open('../../driver/usw/include/drv_ds_tm.h', 'r')
file_sdk = open('sys_c_temp.txt', 'r')
fileline_tm = file_tm.readlines()
fileline_sdk = file_sdk.readlines()

dic_tm = {}
for line in fileline_tm:
    if (-1 != line.find('DRV_DEF_M') or -1 != line.find('DRV_DEF_C')):
        continue
    elif (-1 != line.find('DRV_DEF_D') or -1 != line.find('DRV_DEF_F')):
        macro_name = line.split(',')
        table_name = macro_name[2]
        if(table_name == ""):
            macro_name = line.split('//')
            table_name = macro_name[1]
        table_name = re.sub(r' *', '', table_name)
        table_name=re.sub(r'[0-9]', '', table_name)
        dic_tm[table_name] = 0
for line in fileline_sdk:
    line = re.sub('_t\n', '', line)
    line = re.sub(r'[0-9]', '', line)
    dic_tm[line] = 1
for line in fileline_tm:
    line = re.sub(r'\n', '', line)
    if (-1 == line.find('DRV_DEF_D') and -1 == line.find('DRV_DEF_F')):
        print line
        continue
    macro_name = line.split(',')
    table_name = macro_name[2]
    if(table_name == ""):
            macro_name = line.split('//')
            table_name = macro_name[1]
    table_name = re.sub(r' *', '', table_name)
    table_name=re.sub(r'[0-9]', '', table_name)
    if (dic_tm[table_name] == 1):
        line = re.sub('DRV_DEF_D', 'DRV_DEF_SDK_D', line)
        line = re.sub('DRV_DEF_F', 'DRV_DEF_SDK_F', line)
        line = re.sub('DRV_DEF_M', 'DRV_DEF_SDK_M', line)
    print line





