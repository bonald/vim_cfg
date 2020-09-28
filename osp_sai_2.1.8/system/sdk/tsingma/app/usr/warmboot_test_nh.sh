awk -f support_different_nh.awk ~/ctcsdk/branches/tsingma/sdk/core/usw/include/sys/sys_usw_wb_nh.h > warmboot_test_nh.txt
#First
#sed -i '/^nh:sys_wb_nh_master_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_ipsa_v4_node_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_ipsa_v6_node_t.*/d' warmboot_test_nh.txt
##second
#sed -i '/^nh:sys_wb_nh_arp_t.*/d' warmboot_test_nh.txt
##third
#sed -i '/^nh:sys_wb_nh_brguc_node_t.*/d' warmboot_test_nh.txt
##fourth
#sed -i '/^nh:sys_wb_nh_info_mcast_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_info_com_hdr_t.*/d' warmboot_test_nh.txt
##fifth
#sed -i '/^nh:sys_wb_nh_l2edit_8w_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_l2edit_4w_t.*/d' warmboot_test_nh.txt
##sixth
#sed -i '/^nh:sys_wb_nh_mpls_tunnel_t.*/d' warmboot_test_nh.txt
##seventh
#sed -i '/^nh:sys_wb_nh_bind_ipuc_t.*/d' warmboot_test_nh.txt
##eighth
#sed -i '/^nh:sys_wb_nh_info_brguc_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_info_com_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_info_ecmp_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_info_ip_tunnel_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_info_ipuc_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_info_misc_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_info_mpls_t:.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_info_rspan_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_info_special_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_info_trill_t.*/d' warmboot_test_nh.txt
#sed -i '/^nh:sys_wb_nh_info_wlan_t.*/d' warmboot_test_nh.txt
sed -i '/^:.*/d' warmboot_test_nh.txt
sort -r warmboot_test_nh.txt > warmboot_test_temp_nh.txt
cp warmboot_test_temp_nh.txt warmboot_test_nh.txt
sed -i -e '/nh:sys_wb_nh_l3edit_t:union%l3\.$/d' -e '/nh:sys_wb_nh_nat_t:union%ipda\./d' warmboot_test_nh.txt
sed -i '/nh:sys_wb_nh_info_com_t:sys_wb_nh_info_com_hdr_t%hdr/d' warmboot_test_nh.txt
sed -i 's/nh:sys_wb_nh_info_com_t:uint32%calc_key_len\[0\]/nh:sys_wb_nh_info_com_t:uint32%calc_key_len[0]\nnh:sys_wb_nh_info_com_t:sys_wb_nh_info_com_hdr_t%hdr/' warmboot_test_nh.txt
python warmboot_test_nh.py
