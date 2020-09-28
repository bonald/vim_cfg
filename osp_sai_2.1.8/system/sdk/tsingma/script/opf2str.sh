#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Need arguments which is a chip name!"
    exit
fi

cd ..

chip=$1
src="sys_${chip}_opf.h"
dest="ctc_ftm_cli.c"
src_dir="./core/${chip}/include/sys/$src"
dest_dir="./ctccli/sdkcli/${chip}/$dest"

sed -n "/^enum sys_${chip}_opf_type/,/MAX_OPF_TBL_NUM/p" $src_dir > ds.txt
sed -i "/^enum/d;/{/d;/MAX_OPF_TBL_NUM/d;" ds.txt 
sed -i "s/\([^ ]\+\),.*/\1/" ds.txt 
sed -i "s/[0-9A-Z_]\+/\"&\",/" ds.txt
sed -i "y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/" ds.txt
sed -i "s/_/-/g" ds.txt
sed -i "/\*/d;" ds.txt 

sed -i "s/char ctc_${chip}_opf_str.*/AAAAAAAAAAAAAAA\n&/" $dest_dir

sed -i "/char ctc_${chip}_opf_str/,/^\/\*end\*\/};/d" $dest_dir

sed -i "s/AAAAAAAA.*/char ctc_${chip}_opf_str[MAX_OPF_TBL_NUM][40] =\n{\n\/*end*\/};/" $dest_dir

sed -i "s/\/\*end\*\/};/BBBBB\n\/*end*\/};/" $dest_dir
sed -i "s/BBBBB/cat ds.txt/e" $dest_dir
rm ds.txt 
