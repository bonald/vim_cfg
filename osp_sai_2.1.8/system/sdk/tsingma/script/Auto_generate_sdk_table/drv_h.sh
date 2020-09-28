sort sys_c_temp.txt | uniq > sys_c.txt
cp sys_c.txt  sys_c_temp.txt
cd ../../driver/usw/include
#rm drv_ds_dt2.h drv_ds_tm.h 
git pull
cd -
sed -i 's/\.\.\/\.\.\/driver\/usw\/include\/drv_ds_.*\.h/\.\.\/\.\.\/driver\/usw\/include\/drv_ds_dt2.h/' delet.py
python delet.py > drv_dt2.h
sed -i 's/\.\.\/\.\.\/driver\/usw\/include\/drv_ds_.*\.h/\.\.\/\.\.\/driver\/usw\/include\/drv_ds_tm.h/' delet.py
python delet.py > drv_tm.h 
cp drv_tm.h ../../driver/usw/include/drv_ds_tm.h
cp drv_dt2.h ../../driver/usw/include/drv_ds_dt2.h
cd ../../driver/usw/include
perl ../../../script/genDrv.pl > drv.h
mv drv.h drv_ds.h 
cd -
