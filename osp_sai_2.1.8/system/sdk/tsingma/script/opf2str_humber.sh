
cd ..

src="sys_humber_opf.h"
dest="ctc_ftm_cli.c"
src_dir="./core/humber/include/sys/$src"
dest_dir="./ctccli/sdkcli/humber/$dest"

sed -n "/^enum sys_humber_opf_type/,/MAX_OPF_TBL_NUM/p" $src_dir > ds.txt
sed -i "/^enum/d;/{/d;/MAX_OPF_TBL_NUM/d;" ds.txt 
sed -i "s/\([^ ]\+\),.*/\1/" ds.txt 
sed -i "s/[0-9A-Z_]\+/\"&\",/" ds.txt
sed -i 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/' ds.txt
sed -i "s/_/-/g" ds.txt
sed -i "/\*/d;" ds.txt 

sed -i 's/.*ctc_humber_opf_str.*/AAAAAAAAAAAAAAA\n&/' $dest_dir

sed -i '/ctc_humber_opf_str/,/^\/\*end\*\/};/d' $dest_dir

sed -i 's/AAAAAAAA.*/char ctc_humber_opf_str[MAX_OPF_TBL_NUM][40] =\n{\n\/*end*\/};/' $dest_dir

sed -i "s/\/\*end\*\/};/BBBBB\n\/*end*\/};/" $dest_dir
sed -i "s/BBBBB/cat ds.txt/e" $dest_dir
rm ds.txt 
