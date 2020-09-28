import os
import re
os.getcwd()
os.chdir('../../driver/usw/include')
data = open('drv_ds.h')
out_field = open('drv_field.h', "w")
out_tbl = open('drv_tbl.h', "w")
tbl_id=0
out_tbl.write('#ifndef DRV_DS_LITE\n')
for each_line in data:
    matchObj = re.match(r'DRV_DEF_[A-Z_]*F\(([a-zA-Z0-9_]*)\, ([a-zA-Z0-9_]*)\, ([a-zA-Z0-9_]*)\).*' , each_line, re.I)
    if matchObj:
        out_field.write('#define ')
        out_field.write(matchObj.group(1))
        out_field.write('_')
        out_field.write(matchObj.group(2))
        out_field.write('_f ')
        out_field.write(matchObj.group(3))
        out_field.write('\n')
    matchTbl = re.match(r'DRV_DEF_[A-Z_]*D\(([a-zA-Z0-9_]*)\, ([a-zA-Z0-9_]*)\, ([a-zA-Z0-9_]*)\).*' , each_line, re.I)
    if matchTbl:
        out_tbl.write('#define ')
        out_tbl.write(matchTbl.group(1))
        out_tbl.write('_t ')
        out_tbl.write(matchTbl.group(3))
        out_tbl.write('\n')
        tbl_id=tbl_id+1
out_tbl.write('#define MaxTblId_t ')
out_tbl.write(str(tbl_id))
out_tbl.write('\n')
out_tbl.write('#else\n')
tbl_id=0
data.seek(0)
for each_line in data:
    matchTbl = re.match(r'DRV_DEF_SDK_D\(([a-zA-Z0-9_]*)\, ([a-zA-Z0-9_]*)\, ([a-zA-Z0-9_]*)\).*' , each_line, re.I)
    if matchTbl:
        out_tbl.write('#define ')
        out_tbl.write(matchTbl.group(1))
        out_tbl.write('_t ')
        out_tbl.write(str(tbl_id))
        out_tbl.write('\n')
        tbl_id=tbl_id+1
out_tbl.write('#define MaxTblId_t ')
out_tbl.write(str(tbl_id))
out_tbl.write('\n')
out_tbl.write('#endif\n')
data.close
out_field.close
out_tbl.close
