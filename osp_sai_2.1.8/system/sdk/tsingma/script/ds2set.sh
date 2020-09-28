
cd ..

src="drv_enum.h"
dest="common.c"
src_dir="driver/goldengate/include/$src"
dest_dir="$dest"

str0="#define Set"
str1="(X, ...)        DRV_SET_FLD(X, "
str2="#define Get"
str3="(X, ...)        DRV_GET_FLD(X, "
strv=", ##__VA_ARGS__)"

sed -n "/^typedef enum tbls_id_e/,/MaxTblId_t/p" $src_dir > ds.txt
sed -i "/^enum/d;/{/d;/MaxTblId_t/d" ds.txt 
sed -i "s/_t.*//" ds.txt 
sed -i "s/^ \+//" ds.txt 
sed -i "s/[0-9a-Z_]\+/$str0&$str1&$strv\n$str2&$str3&$strv/" ds.txt
sed -i "/^$/d" ds.txt

#sed -i "/\/\* *[0-9]\+/d" $dest_dir
#
#sed -i "s/};/BBBBB\n};/" $dest_dir
#sed -i "s/BBBBB/cat ds.txt/e" $dest_dir

#rm ds.txt 
