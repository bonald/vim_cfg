
cd ..

src="drv_enum.h"
dest="ctc_goldengate_tr_str.c"
src_dir="driver/goldengate/include/$src"
dest_dir="ctccli/diagcli/goldengate/$dest"

sed -n "/^typedef enum tbls_id_e/,/MaxTblId_t/p" $src_dir > ds.txt
sed -i "/^enum/d;/{/d;/MaxTblId_t/d" ds.txt 
sed -i "s/_t//" ds.txt 
sed -i "s/[0-9a-Z_]\+/\"&\"/" ds.txt
sed -i "s/=\s*\([0-9]\+\)\s*,/,\/*\1*\//" ds.txt
sed -i "/^$/d" ds.txt

sed -i "/\/\* *[0-9]\+/d" $dest_dir

sed -i "s/};/BBBBB\n};/" $dest_dir
sed -i "s/BBBBB/cat ds.txt/e" $dest_dir

#rm ds.txt 
