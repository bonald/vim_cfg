chipname=$1
filename=$chipname\_dkit_discard_type
if [ $chipname == "duet2" ]; then
    exit
fi
if [ $chipname == "tsingma" ]; then
    exit
fi

echo "************************************************"
echo "auto generate discard description "
echo "************************************************"
destfile="../../dkits/$chipname/ctc_$filename.c"
srcfile="../../dkits/$chipname/ctc_$filename.h"
srcfilename="ctc_$filename.h"
 echo  '#include "sal.h"'                                                                                     >$destfile
 echo  '#include'  "\"$srcfilename\""                                                                                      >>$destfile
 echo  ''                                                                                                           >>$destfile
 echo  'const char *ctc_'$chipname'_dkit_get_reason_desc(ctc_dkit_discard_reason_id_t reason_id)'                                                    >>$destfile
 echo  '{'                                                                                                          >>$destfile
 echo  '    static char  reason_desc[256+1] = {0};'                                                                     >>$destfile
 echo  '    if (reason_id == CTC_DKIT_DISCARD_INVALIED)'                                                                          >>$destfile
 echo  '    {'                                                                                                      >>$destfile
 echo  '        return " ";'                                                                                        >>$destfile
 echo  '    }'                                                                                                      >>$destfile
 echo  ''                                                                                                           >>$destfile
 echo  '    switch(reason_id)'                                                                                     >>$destfile
 echo  '    {'                                                                                                      >>$destfile
 sed "/^\s*CTC_DKIT_IPE/! d;s/^\s*\(CTC_DKIT_IPE_\w*\).*<\s*\(.*\)\s*\*.*$/     case \1:  \n       return \"\2\";   /" $srcfile          >>$destfile
 sed "/^\s*CTC_DKIT_EPE/! d;s/^\s*\(CTC_DKIT_EPE_\w*\).*<\s*\(.*\)\s*\*.*$/     case \1:  \n       return \"\2\";   /" $srcfile          >>$destfile
 sed "/^\s*CTC_DKIT_BSR/! d;s/^\s*\(CTC_DKIT_BSR_\w*\).*<\s*\(.*\)\s*\*.*$/     case \1:  \n       return \"\2\";   /" $srcfile          >>$destfile
 sed "/^\s*CTC_DKIT_NETRX/! d;s/^\s*\(CTC_DKIT_NETRX_\w*\).*<\s*\(.*\)\s*\*.*$/     case \1:  \n       return \"\2\";   /" $srcfile          >>$destfile
 sed "/^\s*CTC_DKIT_NETTX/! d;s/^\s*\(CTC_DKIT_NETTX_\w*\).*<\s*\(.*\)\s*\*.*$/     case \1:  \n       return \"\2\";   /" $srcfile          >>$destfile
 sed "/^\s*CTC_DKIT_OAM/! d;s/^\s*\(CTC_DKIT_OAM_\w*\).*<\s*\(.*\)\s*\*.*$/     case \1:  \n       return \"\2\";   /" $srcfile          >>$destfile
 echo  '      default:'                                                                                             >>$destfile
 echo  '      sal_sprintf(reason_desc,"Reason id:%d",reason_id);'                                                  >>$destfile
 echo  '      return reason_desc;'                                                                                   >>$destfile
 echo  '    }'                                                                                                      >>$destfile
 echo  ''                                                                                                           >>$destfile
 echo  '}'                                                                                                          >>$destfile
 echo  ''                                                                                                          >>$destfile

 echo  'const char *ctc_'$chipname'_dkit_get_sub_reason_desc(ctc_dkit_discard_sub_reason_id_t reason_id)'                                                    >>$destfile
 echo  '{'                                                                                                          >>$destfile
 echo  '    static char  reason_desc[256+1] = {0};'                                                                     >>$destfile
 echo  '    if (reason_id == CTC_DKIT_SUB_DISCARD_INVALIED)'                                                                          >>$destfile
 echo  '    {'                                                                                                      >>$destfile
 echo  '        return " ";'                                                                                        >>$destfile
 echo  '    }'                                                                                                      >>$destfile
 echo  ''                                                                                                           >>$destfile
 echo  '    switch(reason_id)'                                                                                     >>$destfile
 echo  '    {'                                                                                                      >>$destfile
 sed "/^\s*CTC_DKIT_SUB/! d;s/^\s*\(CTC_DKIT_SUB_\w*\).*<\s*\(.*\)\s*\*.*$/     case \1:  \n       return \"\2\";   /" $srcfile          >>$destfile
 echo  '      default:'                                                                                             >>$destfile
 echo  '      sal_sprintf(reason_desc,"Sub reason id:%d",reason_id);'                                                  >>$destfile
 echo  '      return reason_desc;'                                                                                   >>$destfile
 echo  '    }'                                                                                                      >>$destfile
 echo  ''                                                                                                           >>$destfile
 echo  '}'                                                                                                          >>$destfile
 echo  ''                                                                                                          >>$destfile


