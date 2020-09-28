awk -f support_different.awk ~/ctcsdk/branches/tsingma/sdk/core/usw/include/sys/sys_usw_wb_common.h > warmboot_test.txt
#First
#sed -i '/^vlan.*/d' warmboot_test.txt
#sed -i '/^l3if.*/d' warmboot_test.txt
#sed -i '/^dot1ae.*/d' warmboot_test.txt
#sed -i '/^port.*/d' warmboot_test.txt
#sed -i '/^mpls.*/d' warmboot_test.txt
#sed -i '/^l2.*/d' warmboot_test.txt
#sed -i '/^ipmc.*/d' warmboot_test.txt
#sed -i '/^ipfix.*/d' warmboot_test.txt
#sed -i '/^linkagg.*/d' warmboot_test.txt
#sed -i '/^overlay.*/d' warmboot_test.txt
#sed -i '/^aps.*/d' warmboot_test.txt
#sed -i '/^mirror.*/d' warmboot_test.txt
sed -i '/^ofb.*/d' warmboot_test.txt
#Second
#sed -i '/^wlan.*/d' warmboot_test.txt
#sed -i '/^oam.*/d' warmboot_test.txt
#sed -i '/^stacking.*/d' warmboot_test.txt
#sed -i '/^scl.*/d' warmboot_test.txt
#Third
#sed -i '/^acl.*/d' warmboot_test.txt
#sed -i '/^ipuc.*/d' warmboot_test.txt
#sed -i '/^calpm.*/d' warmboot_test.txt
#sed -i '/^stats.*/d' warmboot_test.txt
#fourth
#sed -i '/^qos.*/d' warmboot_test.txt
#others
sed -i '/^lpm.*/d' warmboot_test.txt
sed -i '/^igs.*/d' warmboot_test.txt
sed -i '/^egs.*/d' warmboot_test.txt
#sed -i '/^interport.*/d' warmboot_test.txt
sed -i '/^datapath.*/d' warmboot_test.txt
sed -i '/^queue.*/d' warmboot_test.txt
sed -i '/^cpu.*/d' warmboot_test.txt
sed -i '/^learn.*/d' warmboot_test.txt
sed -i '/^adv.*/d' warmboot_test.txt
sed -i '/^.*%#$/d' warmboot_test.txt
sed -i '/^hash.*/d' warmboot_test.txt
sed -i '/^gg.*/d' warmboot_test.txt
sed -i '/^:.*/d' warmboot_test.txt
sed -i 's/ *#/#/' warmboot_test.txt 
sed -i 's/#//' warmboot_test.txt
sort -r warmboot_test.txt > warmboot_test_temp.txt
cp warmboot_test_temp.txt warmboot_test.txt
python warmboot_test.py
