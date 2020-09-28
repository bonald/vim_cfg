cd ../../driver/usw/include/

../../../script/gen_drv/genInclude.pl > drv_ds.h 

../../../script/gen_drv/genDrv.pl DT2 > drv_d2_x.h
cp drv_d2_x.h drv_ds_dt2.h
../../../script/gen_drv/genDrv.pl TM > drv_tm_x.h
cp drv_tm_x.h drv_ds_tm.h
