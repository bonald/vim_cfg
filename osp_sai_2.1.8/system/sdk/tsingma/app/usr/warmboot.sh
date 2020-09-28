sh warmboot_test_nh.sh
sh warmboot_test.sh
rm ctc_app_warmboot_test.c
sed -i -e 's/info_ip_tunnel_l3_edit_ipda/info_ip_tunnel_l3_edit_ipda_1/' -e  's/info_wlan_l3_edit_ipda/info_wlan_l3_edit_ipda_1/' -e 's/l2edit_offset TEXT/l2edit_offset_1 TEXT/' ../../core/app/ctc_app_warmboot_test.c

