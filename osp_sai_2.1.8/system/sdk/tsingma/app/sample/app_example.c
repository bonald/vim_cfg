/**
 @file ctc_isr.c

 @date 2010-1-26

 @version v2.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "api/include/ctc_api.h"


/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/



/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/



/*access port :default vlan 20*/
void port_access_port_example()
{
    uint16 gport;
    uint16 vlan_id = 0;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));
    /*vlan config*/
    vlan_id =20;
    user_vlan.vlan_id = 20;
    user_vlan.user_vlanptr = 20;
    user_vlan.fid = 20;
    ctc_vlan_create_uservlan(&user_vlan);

    /*port 0x000A as access port*/
    gport = 0x000A;
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_port_en(gport, TRUE);
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_port_set_vlan_ctl(gport, CTC_VLANCTL_DROP_ALL_TAGGED);

    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);

    /*only allow vlan 20 on port 10*/
    ctc_vlan_add_port(vlan_id, gport);
}

/*trunk port :default vlan 10,allow tagged  20,30,40*/
void port_trunk_port_example()
{
    uint16 gport;
    uint16 vlan_id = 0;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));
   /*port config*/
    gport = 0x000A;
    vlan_id =10;
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_port_en(gport, TRUE);
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);

    /*vlan config*/
    vlan_id =10;
    user_vlan.vlan_id = 10;
    user_vlan.user_vlanptr = 10;
    user_vlan.fid = 10;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port(vlan_id, gport);
    vlan_id =20;
    user_vlan.vlan_id = 20;
    user_vlan.user_vlanptr = 20;
    user_vlan.fid = 20;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port(vlan_id, gport);
    vlan_id =30;
    user_vlan.vlan_id = 30;
    user_vlan.user_vlanptr = 30;
    user_vlan.fid = 30;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port(vlan_id, gport);
    vlan_id =40;
    user_vlan.vlan_id = 40;
    user_vlan.user_vlanptr = 40;
    user_vlan.fid = 40;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port(vlan_id, gport);
}

/*hybrid port  10:default vlan 10, tagged  20,30,40,untagged  50,60*/
void port_hybrid_port_example()
{
    uint16 gport;
    uint16 vlan_id = 0;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));

    /*port config*/
    gport = 0x000A;
    vlan_id =10;
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_port_en(gport, TRUE);
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);

    /*vlan config*/
    vlan_id =10;
    user_vlan.vlan_id = 10;
    user_vlan.user_vlanptr = 10;
    user_vlan.fid = 10;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port(vlan_id, gport);
    ctc_vlan_set_tagged_port(vlan_id, gport, TRUE);
    vlan_id =20;
    user_vlan.vlan_id = 20;
    user_vlan.user_vlanptr = 20;
    user_vlan.fid = 20;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port(vlan_id, gport);
    ctc_vlan_set_tagged_port(vlan_id, gport, TRUE);
    vlan_id =30;
    user_vlan.vlan_id = 30;
    user_vlan.user_vlanptr = 30;
    user_vlan.fid = 30;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port(vlan_id, gport);
    ctc_vlan_set_tagged_port(vlan_id, gport, TRUE);
    vlan_id =40;
    user_vlan.vlan_id = 40;
    user_vlan.user_vlanptr = 40;
    user_vlan.fid = 40;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port(vlan_id, gport);
    ctc_vlan_set_tagged_port(vlan_id, gport, TRUE);
    vlan_id =50;
    user_vlan.vlan_id = 50;
    user_vlan.user_vlanptr = 50;
    user_vlan.fid = 50;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port(vlan_id, gport);
    ctc_vlan_set_tagged_port(vlan_id, gport, FALSE);
    vlan_id =60;
    user_vlan.vlan_id = 60;
    user_vlan.user_vlanptr = 60;
    user_vlan.fid = 60;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port(vlan_id,gport);
    ctc_vlan_set_tagged_port(vlan_id, gport, FALSE);
}


/*QinQ port 10:default s-vlan 100,cvlan 10 --> svlan 200 */
void port_set_qinq_port_example()
{
    uint16 gport;
    uint16 vlan_id = 0;
    ctc_vlan_mapping_t  vlan_mapping;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));
    /*port config*/
    gport = 0x000A;
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_port_en(gport, TRUE);

    /*port config*/
    gport = 0x000B;
    vlan_id =100;
    ctc_port_set_port_en(gport, TRUE);
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    ctc_port_set_scl_key_type(gport, CTC_INGRESS, 0, CTC_SCL_KEY_TYPE_VLAN_MAPPING_CVID);

    /*vlan config*/
    vlan_id =100;
    user_vlan.vlan_id = 100;
    user_vlan.user_vlanptr = 100;
    user_vlan.fid = 100;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port( vlan_id, gport);
    vlan_id =200;
    user_vlan.vlan_id = 200;
    user_vlan.user_vlanptr = 200;
    user_vlan.fid = 200;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port( vlan_id, gport);

    /*vlan mapping config*/
    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_CVID;
    vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;
    vlan_mapping.old_cvid = 10;
    vlan_mapping.new_svid = 200;
    vlan_mapping.stag_op = CTC_VLAN_TAG_OP_ADD;
    vlan_mapping.svid_sl = CTC_VLAN_TAG_SL_NEW;
    ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
}


void basic_bridge_add_example()
{
    uint16 gport;
    uint16 vlan_id = 0;
    ctc_l2_addr_t  l2_addr;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));
    /*1.Switch A configuration*/
    /*vlan config*/
    vlan_id =1;
    user_vlan.vlan_id = 1;
    user_vlan.user_vlanptr = 1;
    user_vlan.fid = 1;
    ctc_vlan_create_uservlan(&user_vlan);
    vlan_id =10;
    user_vlan.vlan_id = 10;
    user_vlan.user_vlanptr = 10;
    user_vlan.fid = 10;
    ctc_vlan_create_uservlan(&user_vlan);
    vlan_id =20;
    user_vlan.vlan_id = 20;
    user_vlan.user_vlanptr = 20;
    user_vlan.fid = 20;
    ctc_vlan_create_uservlan(&user_vlan);
    /*port 1 ,access port*/
    gport = 0x0001;
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_port_en(gport, TRUE);
    vlan_id =10;
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    ctc_vlan_add_port(vlan_id, gport);
    /*port 2 ,access port*/
    gport = 0x0002;
    ctc_port_set_port_en(gport, TRUE);
    ctc_port_set_mac_en(gport, TRUE);
    vlan_id =20;
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    ctc_vlan_add_port( vlan_id, gport);
    /*port 3 ,trunk port,allow vlan 10,20,default vlan 1*/
    gport = 0x0003;
    ctc_port_set_port_en(gport, TRUE);
    ctc_port_set_mac_en(gport, TRUE);
    vlan_id =1;
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    ctc_vlan_add_port(vlan_id, gport);
    vlan_id =10;
    ctc_vlan_add_port(vlan_id, gport);
    vlan_id =20;
    ctc_vlan_add_port(vlan_id, gport);
    /*fdb config ,PC1->PC3*/
    l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
    l2_addr.mac[0] = 0;
    l2_addr.mac[1] = 0;
    l2_addr.mac[2] = 0;
    l2_addr.mac[3] = 0x12;
    l2_addr.mac[4] = 0x34;
    l2_addr.mac[5] = 0x56;
    l2_addr.fid =10;
    l2_addr.gport = 0x0003;
    ctc_l2_add_fdb(&l2_addr);
    /*fdb config ,PC3->PC1*/
    l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
    l2_addr.mac[0] = 0;
    l2_addr.mac[1] = 0;
    l2_addr.mac[2] = 0;
    l2_addr.mac[3] = 0xab;
    l2_addr.mac[4] = 0xcd;
    l2_addr.mac[5] = 0xef;
    l2_addr.fid = 10;
    l2_addr.gport = 0x0001;
    ctc_l2_add_fdb(&l2_addr);
    l2_addr.flag = CTC_L2_FLAG_IS_STATIC | CTC_L2_FLAG_DISCARD
        | CTC_L2_FLAG_SRC_DISCARD;
    l2_addr.mac[0] = 0;
    l2_addr.mac[1] = 0;
    l2_addr.mac[2] = 0;
    l2_addr.mac[3] = 0xab;
    l2_addr.mac[4] = 0xcd;
    l2_addr.mac[5] = 0;
    l2_addr.fid = 10;
    ctc_l2_add_fdb(&l2_addr);    /*blackhone mac*/

    /*1.Switch B configuration*/
    /*vlan config*/
    vlan_id =1;
    user_vlan.vlan_id = 1;
    user_vlan.user_vlanptr = 1;
    user_vlan.fid = 1;
    ctc_vlan_create_uservlan(&user_vlan);
    vlan_id =10;
    user_vlan.vlan_id = 10;
    user_vlan.user_vlanptr = 10;
    user_vlan.fid = 10;
    ctc_vlan_create_uservlan(&user_vlan);
    vlan_id =20;
    user_vlan.vlan_id = 20;
    user_vlan.user_vlanptr = 20;
    user_vlan.fid = 20;
    ctc_vlan_create_uservlan(&user_vlan);
    /*port 1 ,access port*/
    gport = 0x0001;
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_port_en(gport, TRUE);
    vlan_id =10;
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    ctc_vlan_add_port( vlan_id, gport);
    /*port 2 ,access port*/
    gport = 0x0002;
    ctc_port_set_port_en(gport, TRUE);
    ctc_port_set_mac_en(gport, TRUE);
    vlan_id =20;
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    ctc_vlan_add_port( vlan_id, gport);
    /*port 3 ,trunk port,allow vlan 10,20,default vlan 1*/
    gport = 0x0003;
    ctc_port_set_port_en(gport, TRUE);
    ctc_port_set_mac_en(gport, TRUE);
    vlan_id =1;
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    ctc_vlan_add_port(vlan_id, gport);
    vlan_id =10;
    ctc_vlan_add_port(vlan_id, gport);
    vlan_id =20;
    ctc_vlan_add_port(vlan_id, gport);
    /*fdb config ,PC1->PC3*/
    l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
    l2_addr.mac[0] = 0;
    l2_addr.mac[1] = 0;
    l2_addr.mac[2] = 0;
    l2_addr.mac[3] = 0x12;
    l2_addr.mac[4] = 0x34;
    l2_addr.mac[5] = 0x56;
    l2_addr.fid = 10;
    l2_addr.gport = 0x0001;
    ctc_l2_add_fdb(&l2_addr);
    /*fdb config ,PC3->PC1*/
    l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
    l2_addr.mac[0] = 0;
    l2_addr.mac[1] = 0;
    l2_addr.mac[2] = 0;
    l2_addr.mac[3] = 0xab;
    l2_addr.mac[4] = 0xcd;
    l2_addr.mac[5] = 0xef;
    l2_addr.fid = 10;
    l2_addr.gport = 0x0003;
    ctc_l2_add_fdb(&l2_addr);
}


void qinq_add_example()
{
    uint16 gport;
    uint16 vlan_id = 0;
    ctc_vlan_mapping_t  vlan_mapping;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));
    /*1.Switch A configuration*/
    /*vlan config*/
    vlan_id = 1;  /*default vlan*/
    user_vlan.vlan_id = 1;
    user_vlan.user_vlanptr = 1;
    user_vlan.fid = 1;
    ctc_vlan_create_uservlan(&user_vlan);
    vlan_id = 10;
    user_vlan.vlan_id = 10;
    user_vlan.user_vlanptr = 10;
    user_vlan.fid = 10;
    ctc_vlan_create_uservlan(&user_vlan);
    vlan_id = 20;
    user_vlan.vlan_id = 20;
    user_vlan.user_vlanptr = 20;
    user_vlan.fid = 20;
    ctc_vlan_create_uservlan(&user_vlan);

    /*port 1 ,qinq port*/
    gport = 0x0001;
    ctc_port_set_mac_en (gport, TRUE);
    ctc_port_set_port_en (gport, TRUE);
    ctc_port_set_scl_key_type(gport, CTC_INGRESS, 0, CTC_SCL_KEY_TYPE_VLAN_MAPPING_CVID);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    vlan_id = 1;
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_vlan_add_port(vlan_id, gport);
    vlan_id = 10;
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_vlan_add_port(vlan_id, gport);
    vlan_id = 20;
    ctc_port_set_default_vlan(gport,vlan_id);
    ctc_vlan_add_port(vlan_id, gport);

    /*cvlan 10/20/30 ->svlan 10*/
    sal_memset(&vlan_mapping,0,sizeof(ctc_vlan_mapping_t));
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_CVID;
    vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;

    vlan_mapping.old_cvid = 10;
    vlan_mapping.new_svid = 10;
    vlan_mapping.stag_op = CTC_VLAN_TAG_OP_ADD;
    vlan_mapping.svid_sl = CTC_VLAN_TAG_SL_NEW;
    ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    vlan_mapping.old_cvid= 20;
    vlan_mapping.new_svid = 10;
    ctc_vlan_add_vlan_mapping(gport,&vlan_mapping);
    vlan_mapping.old_cvid = 30;
    vlan_mapping.new_svid = 10;
    ctc_vlan_add_vlan_mapping(gport,&vlan_mapping);

    /*cvlan 100/200/300 ->svlan 20*/
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_CVID;
    vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;
    vlan_mapping.old_cvid = 100;
    vlan_mapping.new_svid = 20;
    ctc_vlan_add_vlan_mapping(gport,&vlan_mapping);
    vlan_mapping.old_cvid = 200;
    vlan_mapping.new_svid = 10;
    ctc_vlan_add_vlan_mapping(gport,&vlan_mapping);
    vlan_mapping.old_cvid = 300;
    vlan_mapping.new_svid = 20;
    ctc_vlan_add_vlan_mapping(gport,&vlan_mapping);

    /*port config*/
    gport = 0x0002;
    ctc_port_set_port_en (gport,TRUE);
    ctc_port_set_mac_en (gport,TRUE);
    vlan_id = 10;
    ctc_port_set_default_vlan(gport,vlan_id);
    ctc_vlan_add_port( vlan_id, gport);
    vlan_id = 20;
    ctc_port_set_default_vlan(gport,vlan_id);
    ctc_vlan_add_port( vlan_id, gport);

    /*1.Switch B configuration*/
    /*vlan config*/
    vlan_id = 1;  /*default vlan*/
    user_vlan.vlan_id = 1;
    user_vlan.user_vlanptr = 1;
    user_vlan.fid = 1;
    ctc_vlan_create_uservlan(&user_vlan);
    vlan_id = 10;
    user_vlan.vlan_id = 10;
    user_vlan.user_vlanptr = 10;
    user_vlan.fid = 10;
    ctc_vlan_create_uservlan(&user_vlan);
    vlan_id = 20;
    user_vlan.vlan_id = 20;
    user_vlan.user_vlanptr = 20;
    user_vlan.fid = 20;
    ctc_vlan_create_uservlan(&user_vlan);

    /*port 1 ,qinq port*/
    gport = 0x0001;
    ctc_port_set_mac_en (gport, TRUE);
    ctc_port_set_port_en (gport,TRUE);
    ctc_port_set_scl_key_type(gport, CTC_INGRESS, 0, CTC_SCL_KEY_TYPE_VLAN_MAPPING_CVID);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    vlan_id = 1;
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_vlan_add_port( vlan_id, gport);
    vlan_id = 10;
    ctc_port_set_default_vlan(gport,vlan_id);
    ctc_vlan_add_port( vlan_id, gport);
    vlan_id = 20;
    ctc_port_set_default_vlan(gport,vlan_id);
    ctc_vlan_add_port( vlan_id, gport);

    /*cvlan 10/20/30 ->svlan 10*/
    sal_memset(&vlan_mapping,0,sizeof(ctc_vlan_mapping_t));
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_CVID;
    vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;
    vlan_mapping.old_cvid = 10;
    vlan_mapping.new_svid = 10;
    ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    vlan_mapping.old_cvid = 20;
    vlan_mapping.new_svid = 10;
    ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    vlan_mapping.old_cvid = 30;
    vlan_mapping.new_svid = 10;
    ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    /*cvlan 100/200/300 ->svlan 20*/
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_CVID;
    vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_SVID;
    vlan_mapping.old_cvid = 100;
    vlan_mapping.new_svid = 20;
    ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    vlan_mapping.old_cvid = 200;
    vlan_mapping.new_svid = 10;
    ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    vlan_mapping.old_cvid = 300;
    vlan_mapping.new_svid = 20;
    ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    /*port config*/
    gport = 0x0002;
    ctc_port_set_port_en(gport, TRUE);
    ctc_port_set_mac_en(gport, TRUE);
    vlan_id = 10;
    ctc_port_set_default_vlan(gport, vlan_id);
    ctc_vlan_add_port(vlan_id, gport);
    vlan_id = 20;
    ctc_port_set_default_vlan(gport,vlan_id);
    ctc_vlan_add_port(vlan_id, gport);
}

void l2mc_add_example()
{
    uint16 gport;
    uint16 vlan_id = 0;
    ctc_l2_mcast_addr_t   l2mc_addr;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));
    /*vlan config*/
    vlan_id =3;
    user_vlan.vlan_id = 3;
    user_vlan.user_vlanptr = 3;
    user_vlan.fid = 3;
    ctc_vlan_create_uservlan(&user_vlan);

    /*port 2: trunk port*/
    gport = 0x0002;
    ctc_port_set_mac_en (gport, TRUE);
    ctc_port_set_port_en (gport, TRUE);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    vlan_id = 3;
    ctc_vlan_add_port(vlan_id, gport);

    /*port 1: hybrid port:vlan 3: tagged*/
    gport = 0x0001;
    ctc_port_set_port_en (gport, TRUE);
    ctc_port_set_mac_en (gport, TRUE);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    vlan_id = 3;
    ctc_vlan_add_port(vlan_id, gport);

    /*create mcast group*/
    sal_memset(&l2mc_addr, 0, sizeof(ctc_l2_mcast_addr_t));
    l2mc_addr.mac[0] = 0x01;
    l2mc_addr.mac[1] = 0;
    l2mc_addr.mac[2] = 0x5e;
    l2mc_addr.mac[3] = 0x40;
    l2mc_addr.mac[4] = 0;
    l2mc_addr.mac[5] = 0x01;
    l2mc_addr.fid =3;
    l2mc_addr.l2mc_grp_id = 1;/*mcat group id*/
    ctc_l2mcast_add_addr(&l2mc_addr);

    /*add mcast member*/
    sal_memset(&l2mc_addr, 0, sizeof(ctc_l2_mcast_addr_t));
    l2mc_addr.mac[0] = 0x01;
    l2mc_addr.mac[1] = 0;
    l2mc_addr.mac[2] = 0;
    l2mc_addr.mac[3] = 0x12;
    l2mc_addr.mac[4] = 0x34;
    l2mc_addr.mac[5] = 0x56;
    l2mc_addr.fid =3;
    l2mc_addr.member.mem_port = 0x0001;
    ctc_l2mcast_add_member(&l2mc_addr);

}


void ipuc_add_example()
{
    uint16 gport;
    uint16 vlan_id = 0;
    ctc_ip_nh_param_t nh_param;
    ctc_ipuc_param_t  ipuc_info;
    ctc_l3if_t  l3if;
    uint16 l3if_id = 0;
    uint32 nhid = 0;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));
    /*vlan config*/
    vlan_id =10;
    user_vlan.vlan_id = 10;
    user_vlan.user_vlanptr = 10;
    user_vlan.fid = 10;
    ctc_vlan_create_uservlan(&user_vlan);
    vlan_id =20;
    user_vlan.vlan_id = 20;
    user_vlan.user_vlanptr = 20;
    user_vlan.fid = 20;
    ctc_vlan_create_uservlan(&user_vlan);
    vlan_id =40;
    user_vlan.vlan_id = 40;
    user_vlan.user_vlanptr = 40;
    user_vlan.fid = 40;
    ctc_vlan_create_uservlan(&user_vlan);

    /*Switch B configurations */
    /*port 1, hybrid port, and port default config:routed enable,*/
    gport = 0x0001;
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_port_en(gport, TRUE);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    vlan_id = 10;
    ctc_vlan_add_port(vlan_id, gport);
    /*port 2, hybrid port, and port default config:routed enable,*/
    gport = 0x0002;
    ctc_port_set_port_en(gport, TRUE);
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    vlan_id = 20;
    ctc_vlan_add_port(vlan_id, gport);
    /*port 3, hybrid port, and port default config:routed enable,*/
    gport = 0x0003;
    ctc_port_set_port_en(gport,TRUE);
    ctc_port_set_mac_en(gport,TRUE);
    ctc_port_set_vlan_filter_en(gport, CTC_BOTH_DIRECTION, TRUE);
    vlan_id = 40;
    ctc_vlan_add_port(vlan_id, gport);
    /*l3if config*/
    l3if_id =1;	/*allocated l3if id =1*/
    l3if.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
    l3if.vlan_id = 10;
    ctc_l3if_create (l3if_id, &l3if);
    l3if_id =2;	   /*allocated l3if id =2*/
    l3if.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
    l3if.vlan_id = 20;
    ctc_l3if_create (l3if_id, &l3if);
    l3if_id =3; /*allocated l3if id =3*/
    l3if.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
    l3if.vlan_id = 40;
    ctc_l3if_create(l3if_id, &l3if);

    /*1.create arp (1.1.4.1/0.0.2 : port 1) & host route*/
    sal_memset(&nh_param, 0, sizeof(ctc_ip_nh_param_t));
    nh_param.dsnh_offset = 0;  /**/
    nh_param.mac[0] = 0;
    nh_param.mac[1] = 0;
    nh_param.mac[2] = 0;
    nh_param.mac[3] = 0;
    nh_param.mac[4] = 0;
    nh_param.mac[5] = 0x02;
    nh_param.oif.gport = 0x0001;
    nh_param.oif.vid = 10;
    nhid  = 10;  /*the nhid mapping from system software's nexthop_index*/
    ctc_nh_add_ipuc(nhid, &nh_param);

    sal_memset(&ipuc_info,0,sizeof(ctc_ipuc_param_t));
    ipuc_info.ip.ipv4 = 0x01010401;
    ipuc_info.masklen =32;
    ipuc_info.nh_id = 10;
    ipuc_info.ip_ver = CTC_IP_VER_4;
    ipuc_info.route_flag = CTC_IPUC_FLAG_NEIGHBOR;
    ctc_ipuc_add(&ipuc_info);
    /*create lpm route*/
    sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
    ipuc_info.ip.ipv4 = 0x01010100;
    ipuc_info.masklen =24;
    ipuc_info.nh_id = 10;
    ipuc_info.ip_ver = CTC_IP_VER_4;
    ipuc_info.route_flag = CTC_IPUC_FLAG_CONNECT;
    ctc_ipuc_add(&ipuc_info);

    /*2.create arp (1.1.4.6/0.0.3 : port 2) & host route*/
    sal_memset(&nh_param,0,sizeof(ctc_ip_nh_param_t));
    nh_param.dsnh_offset = 0;  /**/
    nh_param.mac[0] = 0;
    nh_param.mac[1] = 0;
    nh_param.mac[2] = 0;
    nh_param.mac[3] = 0;
    nh_param.mac[4] = 0;
    nh_param.mac[5] = 0x03;
    nh_param.oif.gport = 0x0002;
    nh_param.oif.vid = 20;
    nhid  = 11;  /*the nhid mapping from system software's nexthop_index*/
    ctc_nh_add_ipuc(nhid, &nh_param);

    sal_memset(&ipuc_info,0,sizeof(ctc_ipuc_param_t));
    ipuc_info.ip.ipv4 = 0x01010406;
    ipuc_info.masklen =32;
    ipuc_info.nh_id = 11;
    ipuc_info.ip_ver = CTC_IP_VER_4;
    ipuc_info.route_flag = CTC_IPUC_FLAG_NEIGHBOR;
    ctc_ipuc_add(&ipuc_info);

    /*create lpm route*/
    sal_memset(&ipuc_info,0,sizeof(ctc_ipuc_param_t));
    ipuc_info.ip.ipv4 = 0x01010400;
    ipuc_info.masklen =24;
    ipuc_info.nh_id = 11;
    ipuc_info.ip_ver = CTC_IP_VER_4;
    ipuc_info.route_flag = CTC_IPUC_FLAG_CONNECT;
    ctc_ipuc_add(&ipuc_info);

    /*3.create arp (1.1.2.2/0.0.4 : port 3) & host route*/
    sal_memset(&nh_param,0,sizeof(ctc_ip_nh_param_t));
    nh_param.dsnh_offset = 0;  /**/
    nh_param.mac[0] = 0;
    nh_param.mac[1] = 0;
    nh_param.mac[2] = 0;
    nh_param.mac[3] = 0;
    nh_param.mac[4] = 0;
    nh_param.mac[5] = 0x04;
    nh_param.oif.gport = 0x0003;
    nh_param.oif.vid = 20;
    nhid  = 12;  /*the nhid mapping from system software's nexthop_index*/
    ctc_nh_add_ipuc(nhid, &nh_param);

    sal_memset(&ipuc_info,0,sizeof(ctc_ipuc_param_t));
    ipuc_info.ip.ipv4 = 0x01010202;
    ipuc_info.masklen =32;
    ipuc_info.nh_id = 12;
    ipuc_info.ip_ver = CTC_IP_VER_4;
    ipuc_info.route_flag = CTC_IPUC_FLAG_NEIGHBOR;
    ctc_ipuc_add(&ipuc_info);
}

void ipmc_add_example()
{
    uint16 gport;
    uint16 vlan_id = 0;
    ctc_l3if_t  l3if;
    uint16 l3if_id = 0;
    ctc_l3if_property_t l3if_prop = 0;

    ctc_ipmc_group_info_t* ipmac_group = NULL;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));
    ipmac_group = (ctc_ipmc_group_info_t*)sal_malloc(sizeof(ctc_ipmc_group_info_t));
    if(ipmac_group == NULL)
    {
       return;
    }
    sal_memset(ipmac_group, 0, sizeof(ctc_ipmc_group_info_t));

    /*Router configurations */
    /*port 2, phyif,*/
    gport = 0x0002;
    vlan_id = 3;
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_port_en(gport, TRUE);
    gport = 0x0001;
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_port_en(gport, TRUE);
    user_vlan.vlan_id = vlan_id;
    user_vlan.user_vlanptr = vlan_id;
    user_vlan.fid = vlan_id;
    ctc_vlan_create_uservlan(&user_vlan);
    ctc_vlan_add_port(vlan_id, gport);
    /*l3if config*/
    l3if_id =1;     /*allocated l3if id =1*/
    l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    l3if.gport = 0x0002;
    ctc_l3if_create(l3if_id, &l3if);
    l3if_prop = CTC_L3IF_PROP_IPV4_MCAST;
    ctc_l3if_set_property(l3if_id, l3if_prop, TRUE);
    ctc_port_set_phy_if_en(l3if.gport, TRUE);
    l3if_id = 2;     /*allocated l3if id =1*/
    l3if.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
    l3if.vlan_id = 3;
    ctc_l3if_create(l3if_id, &l3if);
    l3if_prop = CTC_L3IF_PROP_IPV4_MCAST;
    ctc_l3if_set_property(l3if_id, l3if_prop, TRUE);

    /*1.create mcast group & add member*/
    ipmac_group->address.ipv4.group_addr = 0xefc00001;
    ipmac_group->address.ipv4.src_addr = 0xc0a90101;
    ipmac_group->group_id  = 10;
    ipmac_group->group_ip_mask_len = 32;
    ipmac_group->src_ip_mask_len = 32;
    ipmac_group->ip_version = CTC_IP_VER_4;
    ctc_ipmc_add_group(ipmac_group);

    sal_memset(ipmac_group, 0, sizeof(ctc_ipmc_group_info_t));
    ipmac_group->address.ipv4.group_addr = 0xefc00001;
    ipmac_group->address.ipv4.src_addr = 0xc0a90101;
    ipmac_group->group_ip_mask_len = 32;
    ipmac_group->src_ip_mask_len = 32;
    ipmac_group->ip_version = CTC_IP_VER_4;
    ipmac_group->ipmc_member[0].global_port = 2;
    ipmac_group->ipmc_member[0].l3_if_type = CTC_L3IF_TYPE_PHY_IF;
    ipmac_group->ipmc_member[1].vlan_id = 3;
    ipmac_group->ipmc_member[1].global_port = 1;
    ipmac_group->ipmc_member[1].l3_if_type = CTC_L3IF_TYPE_VLAN_IF;
    ipmac_group->member_number = 1;
    ctc_ipmc_add_member(ipmac_group);

    sal_free(ipmac_group);
}

void mpls_vpls_add_example()
{
    uint16 gport, l3ifid = 0,nhid=0,sacl_id=0;
    ctc_l3if_t  l3if;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_l2_addr_t l2_addr;
    ctc_mpls_nexthop_tunnel_param_t  nh_tunnel_param;
    ctc_mpls_nexthop_tunnel_info_t   *p_nh_param;
    ctc_mpls_nexthop_param_t          nh_param;
    ctc_mpls_nexthop_push_param_t  *p_nh_param_push;
    ctc_mpls_ilm_t  mpls_ilm;
    ctc_port_scl_key_type_t type;
    ctc_l2dflt_addr_t l2dflt_addr;

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));
    /*Port config :AC Port:Port1*/
    gport = 0x0001;
    ctc_port_set_mac_en(gport, TRUE);
    ctc_port_set_port_en(gport,TRUE);
    /*Port config :PW Port:Port3 :phyif*/
    gport = 0x0003;
    ctc_port_set_port_en (gport,TRUE);
    ctc_port_set_mac_en (gport,TRUE);
    l3ifid =2;  /* allocated l3if id =2*/
    l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    l3if.gport = 0x0003;
    ctc_l3if_create (l3ifid, &l3if);
    ctc_port_set_phy_if_en(l3if.gport,TRUE);

    /*1.ac -> pw config*/
    /*vlan mapping config,vsi :1 --> fid :4096*/
    sal_memset(&vlan_mapping,0,sizeof(ctc_vlan_mapping_t));
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
    vlan_mapping.action =
        CTC_VLAN_MAPPING_OUTPUT_FID|CTC_VLAN_MAPPING_FLAG_VPLS|CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT;
    vlan_mapping.old_svid = 10;
    vlan_mapping.u3.fid = 4096;
    vlan_mapping.logic_src_port = 400;
    ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
    sacl_id=1;
    type=CTC_SCL_KEY_TYPE_VLAN_MAPPING_PORT;
    ctc_port_set_scl_key_type(gport, CTC_INGRESS, sacl_id, type);

    /* add default entry using logic port learning */
    l2dflt_addr.fid = 4096;
    l2dflt_addr.l2mc_grp_id = 4096;
    l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT;
    ctc_l2_add_default_entry(&l2dflt_addr);

    /* create PW1 tunnel label */
    sal_memset(&nh_tunnel_param, 0, sizeof(ctc_mpls_nexthop_tunnel_param_t));
    p_nh_param = &nh_tunnel_param.nh_param;
    p_nh_param->label_num=1;
    p_nh_param->mac[0] = 0;
    p_nh_param->mac[1] = 0xb;
    p_nh_param->mac[2] = 0;
    p_nh_param->mac[3] = 0xe;
    p_nh_param->mac[4] = 0;
    p_nh_param->mac[5] = 0xd;
    p_nh_param->tunnel_label[0].lable_flag=CTC_MPLS_NH_LABEL_IS_VALID;
    p_nh_param->tunnel_label[0].label=100;
    p_nh_param->tunnel_label[0].ttl=64;
    p_nh_param->tunnel_label[0].exp_type=0;
    p_nh_param->tunnel_label[0].exp=0x02;
    ctc_nh_add_mpls_tunnel_label(1, &nh_tunnel_param);

    /* create to PW1 nexthop (LSP label + PW label) */
    sal_memset(&nh_param,0,sizeof(ctc_mpls_nexthop_param_t));
    p_nh_param_push = &(nh_param.nh_para.nh_param_push);
    p_nh_param_push->nh_com.opcode = CTC_MPLS_NH_PUSH_OP_L2VPN;
    p_nh_param_push->nh_com.oif.gport= 1;
    p_nh_param_push->nh_com.mac[0] = 0;
    p_nh_param_push->nh_com.mac[1] = 0xb;
    p_nh_param_push->nh_com.mac[2] = 0;
    p_nh_param_push->nh_com.mac[3] = 0xe;
    p_nh_param_push->nh_com.mac[4] = 0;
    p_nh_param_push->nh_com.mac[5] = 0xd;
    p_nh_param_push->nh_com.vlan_info.svlan_edit_type= CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
    p_nh_param_push->nh_com.vlan_info.output_svid= 64;
    p_nh_param_push->nh_com.vlan_info.edit_flag= CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID;
    p_nh_param_push->push_label[0].lable_flag=CTC_MPLS_NH_LABEL_IS_VALID;
    p_nh_param_push->push_label[0].label=10;
    p_nh_param_push->push_label[0].ttl=100;
    p_nh_param_push->push_label[0].exp_type=0;
    p_nh_param_push->push_label[0].exp=1;
    p_nh_param_push->push_label[1].lable_flag=CTC_MPLS_NH_LABEL_IS_VALID;
    p_nh_param_push->push_label[1].label=100;
    p_nh_param_push->push_label[1].ttl=64;
    p_nh_param_push->push_label[1].exp_type=0;
    p_nh_param_push->push_label[1].exp=2;
    p_nh_param_push->label_num =2;
    nhid = 102;  /*the nhid mapping from system software's nexthop_index*/
    ctc_nh_add_mpls(nhid, &nh_param);
    /* l2 fdb add mac 0001.1111.abc5 fid 4096 nexthop 102,vsi :1 --> fid :4096 */
    nhid = 102;  /*to Pw nexthop*/
    sal_memset(&l2_addr,0,sizeof(ctc_l2_addr_t));
    l2_addr.mac[0] = 0;
    l2_addr.mac[1] = 0x01;
    l2_addr.mac[2] = 0x11;
    l2_addr.mac[3] = 0x11;
    l2_addr.mac[4] = 0xab;
    l2_addr.mac[5] = 0xc5;
    l2_addr.fid = 4096;
    l2_addr.gport = 0x0001;
    ctc_l2_add_fdb_with_nexthop(&l2_addr,nhid);

    /*2.pw -> ac config*/
    sal_memset(&mpls_ilm,0,sizeof(ctc_mpls_ilm_t));
    mpls_ilm.label = 100;
    mpls_ilm.pop=1; /*Pop LSP label*/
    ctc_mpls_add_ilm(&mpls_ilm);

    /*Decap PW label,map to vsi 1 (fid: 4096)*/
    sal_memset(&mpls_ilm,0,sizeof(ctc_mpls_ilm_t));
    mpls_ilm.label = 10;
    mpls_ilm.type = CTC_MPLS_LABEL_TYPE_VPLS;
    mpls_ilm.pwid = 300;
    mpls_ilm.logic_port_type = 1;
    mpls_ilm.fid = 4096;
    ctc_mpls_add_ilm(&mpls_ilm);

    /* l2 fdb add mac 0001.1111.abc1 fid 4096 port 0x0001*/
    sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));
    l2_addr.mac[0] = 0;
    l2_addr.mac[1] = 0x01;
    l2_addr.mac[2] = 0x11;
    l2_addr.mac[3] = 0x11;
    l2_addr.mac[4] = 0xab;
    l2_addr.mac[5] = 0xc1;
    l2_addr.fid = 4096;
    l2_addr.gport = 0x001;
    ctc_l2_add_fdb(&l2_addr);
}

void mpls_vpws_add_example()
{
    uint16 gport;
    ctc_l3if_t  l3if;
    uint16 l3ifid = 0;
    uint32 nhid =0;
    ctc_vlan_mapping_t vlan_mapping;
    uint32 to_pw_nh_id = 0;
    ctc_mpls_nexthop_tunnel_param_t nh_tunnel_param;
    ctc_mpls_nexthop_tunnel_info_t *p_nh_param;
    ctc_mpls_nexthop_param_t  nh_param;
    ctc_mpls_nexthop_push_param_t  *p_nh_param_push;
    ctc_mpls_ilm_t  mpls_ilm;
    ctc_mpls_l2vpn_pw_t l2vpn_pw;
    ctc_port_scl_key_type_t type;

    /*port1:Ac Port : config*/
    gport = 0x0001;
    ctc_port_set_mac_en (gport, TRUE);
    ctc_port_set_port_en (gport,TRUE);

    /*port1:PW Port : allocated l3if id =2*/
    gport = 0x0002;
    ctc_port_set_port_en (gport,TRUE);
    ctc_port_set_mac_en (gport,TRUE);
    l3ifid =2;
    l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    l3if.gport = 2;
    l3if.vlan_id = 0;
    ctc_l3if_create (l3ifid, &l3if);
    ctc_port_set_phy_if_en(l3if.gport , TRUE);

    /*AC-->PW*/
    /* create tunnel label */
    sal_memset(&nh_tunnel_param, 0, sizeof(nh_tunnel_param));
    p_nh_param = &nh_tunnel_param.nh_param;
    p_nh_param->label_num=1;
    p_nh_param->mac[0] = 0;
    p_nh_param->mac[1] = 0xb;
    p_nh_param->mac[2] = 0;
    p_nh_param->mac[3] = 0xe;
    p_nh_param->mac[4] = 0;
    p_nh_param->mac[5] = 0xd;
    p_nh_param->tunnel_label[0].lable_flag=CTC_MPLS_NH_LABEL_IS_VALID;
    p_nh_param->tunnel_label[0].label=10;
    p_nh_param->tunnel_label[0].ttl=10;
    p_nh_param->tunnel_label[0].exp_type=0;
    p_nh_param->tunnel_label[0].exp=2;
    ctc_nh_add_mpls_tunnel_label(1, &nh_tunnel_param);

    /* create to PW nexthop(LSP label + PW label) vpws nh */
    sal_memset(&nh_param,0,sizeof(ctc_mpls_nexthop_param_t));
    p_nh_param_push = &(nh_param.nh_para.nh_param_push);
    p_nh_param_push->nh_com.opcode = CTC_MPLS_NH_PUSH_OP_L2VPN;
    p_nh_param_push->nh_com.oif.gport= 1;
    p_nh_param_push->nh_com.mac[0] = 0;
    p_nh_param_push->nh_com.mac[1] = 0xb;
    p_nh_param_push->nh_com.mac[2] = 0;
    p_nh_param_push->nh_com.mac[3] = 0xe;
    p_nh_param_push->nh_com.mac[4] = 0;
    p_nh_param_push->nh_com.mac[5] = 0xd;
    p_nh_param_push->push_label[0].lable_flag=CTC_MPLS_NH_LABEL_IS_VALID;
    p_nh_param_push->push_label[0].label=100;/*pw label*/
    p_nh_param_push->push_label[0].ttl=100;
    p_nh_param_push->push_label[0].exp_type=0;
    p_nh_param_push->push_label[0].exp=1;
    p_nh_param_push->push_label[1].lable_flag=CTC_MPLS_NH_LABEL_IS_VALID;
    p_nh_param_push->push_label[1].label=10;  /*tunnel label*/
    p_nh_param_push->push_label[1].ttl=10;
    p_nh_param_push->push_label[1].exp_type=0;
    p_nh_param_push->push_label[1].exp=2;
    p_nh_param_push->label_num =2;
    p_nh_param_push->mtu_size=0;
    nhid = 102;  /*the nhid mapping from system software's nexthop_index*/
    ctc_nh_add_mpls(nhid, &nh_param);

    /* AC-> PW vlan mapping  mapping-to nexthop 102 vpws nexthop */
    to_pw_nh_id = nhid;
    sal_memset(&vlan_mapping,0,sizeof(ctc_vlan_mapping_t));
    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_SVID;
    vlan_mapping.action = CTC_VLAN_MAPPING_OUTPUT_NHID| CTC_VLAN_MAPPING_FLAG_VPWS;
    vlan_mapping.old_svid = 10;
    vlan_mapping.u3.nh_id = to_pw_nh_id;
    gport=1;
    ctc_vlan_add_vlan_mapping(1,&vlan_mapping);
    type = CTC_SCL_KEY_TYPE_VLAN_MAPPING_PORT;
    ctc_port_set_scl_key_type(gport, CTC_INGRESS, 1, type);

    /* PW-->AC */
    /*POP LSP label*/
    sal_memset(&mpls_ilm,0,sizeof(ctc_mpls_ilm_t));
    mpls_ilm.label = 10;
    mpls_ilm.pop   = 1;
    ctc_mpls_add_ilm(&mpls_ilm);

    sal_memset(&mpls_ilm,0,sizeof(ctc_mpls_ilm_t));
    mpls_ilm.label = 100;
    mpls_ilm.type = CTC_MPLS_LABEL_TYPE_VPWS;
    mpls_ilm.nh_id = nhid;
    ctc_mpls_add_ilm(&mpls_ilm);

    sal_memset(&l2vpn_pw,0,sizeof(ctc_mpls_l2vpn_pw_t));
    l2vpn_pw.label=100;
    l2vpn_pw.l2vpntype=CTC_MPLS_L2VPN_VPWS;
    gport = 0x0001;
    ctc_nh_get_l2uc(gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nhid);

}

void mpls_ilm_add_example()
{
    uint16 gport, nhid=0;
    ctc_l3if_t  l3if;
    ctc_mpls_nexthop_tunnel_param_t   nh_tunnel_param;
    ctc_mpls_nexthop_tunnel_info_t    *p_nh_param;
    ctc_mpls_nexthop_push_param_t     *p_nh_param_push;
    ctc_mpls_ilm_t  mpls_ilm;
    ctc_mpls_nexthop_param_t      nh_param;

    /*port config*/
    gport = 0x0001;
    ctc_port_set_mac_en (gport, TRUE);
    ctc_port_set_port_en (gport,TRUE);
    gport = 0x0002;
    ctc_port_set_port_en (gport,TRUE);
    ctc_port_set_mac_en (gport,TRUE);

    /*l3if config*/
    l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    l3if.gport = 0x0001;
    ctc_l3if_create (1, &l3if);    /*allocated l3if id =1*/
    ctc_port_set_phy_if_en(l3if.gport , TRUE);
    l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    l3if.gport = 0x0002;
    ctc_l3if_create (2, &l3if);    /*allocated l3if id =2*/
    ctc_port_set_phy_if_en(l3if.gport , TRUE);

    /*Create Tunnel label*/
    sal_memset(&nh_tunnel_param, 0, sizeof(ctc_mpls_nexthop_tunnel_param_t));
    p_nh_param = &nh_tunnel_param.nh_param;
    p_nh_param->label_num=1;
    p_nh_param->mac[0] = 0;
    p_nh_param->mac[1] = 0xb;
    p_nh_param->mac[2] = 0;
    p_nh_param->mac[3] = 0xe;
    p_nh_param->mac[4] = 0;
    p_nh_param->mac[5] = 0xd;
    p_nh_param->tunnel_label[0].lable_flag=CTC_MPLS_NH_LABEL_IS_VALID
                                        |CTC_MPLS_NH_LABEL_MAP_TTL;
    p_nh_param->tunnel_label[0].label=100;
    p_nh_param->tunnel_label[0].ttl=64;
    p_nh_param->tunnel_label[0].exp=0x01;
    ctc_nh_add_mpls_tunnel_label(1, &nh_tunnel_param);

    /* Create PW nexthop(swap LSP label)*/
    sal_memset(&nh_param,0,sizeof(ctc_mpls_nexthop_param_t));
    nh_param.nh_prop = CTC_MPLS_NH_PUSH_TYPE;
    p_nh_param_push = &(nh_param.nh_para.nh_param_push);
    p_nh_param_push->nh_com.opcode = CTC_MPLS_NH_PUSH_OP_NONE;
    p_nh_param_push->nh_com.oif.gport= 1;
    p_nh_param_push->nh_com.mac[0] = 0;
    p_nh_param_push->nh_com.mac[1] = 0xb;
    p_nh_param_push->nh_com.mac[2] = 0;
    p_nh_param_push->nh_com.mac[3] = 0xe;
    p_nh_param_push->nh_com.mac[4] = 0;
    p_nh_param_push->nh_com.mac[5] = 0xd;
    p_nh_param_push->push_label[0].lable_flag = CTC_MPLS_NH_LABEL_IS_VALID
                                               |CTC_MPLS_NH_LABEL_MAP_TTL;
    p_nh_param_push->push_label[0].label=100;
    p_nh_param_push->push_label[0].ttl=64;
    p_nh_param_push->push_label[0].exp=1;
    p_nh_param_push->label_num =1;
    nhid = 102;  /*the nhid mapping from system software's nexthop_index*/
    ctc_nh_add_mpls(nhid, &nh_param);

    /* add mpls ilm*/
    sal_memset(&mpls_ilm,0,sizeof(ctc_mpls_ilm_t));
    mpls_ilm.label = 200;
    mpls_ilm.nh_id = 102;
    mpls_ilm.model=CTC_MPLS_TUNNEL_MODE_UNIFORM;
    ctc_mpls_add_ilm(&mpls_ilm);
}

void mpls_l3vpn_add_example()
{
    uint16 gport,vlan_id = 0,l3if_id = 0;
    uint32 nhid=0;
    ctc_l3if_t  l3if;
    ctc_mpls_nexthop_tunnel_param_t  nh_tunnel_param;
    ctc_mpls_nexthop_tunnel_info_t   *p_nh_param;
    ctc_mpls_nexthop_param_t          nh_param;
    ctc_mpls_nexthop_push_param_t  *p_nh_param_push;
    ctc_mpls_ilm_t  mpls_ilm;
    ctc_ipuc_param_t  ipuc_info;
    ctc_vlan_uservlan_t user_vlan;

    sal_memset(&user_vlan, 0, sizeof(ctc_vlan_uservlan_t));
    /*vlan config*/
    vlan_id =10;
    user_vlan.vlan_id = 10;
    user_vlan.user_vlanptr = 10;
    user_vlan.fid = 10;
    ctc_vlan_create_uservlan(&user_vlan);

    vlan_id =30;
    user_vlan.vlan_id = 30;
    user_vlan.user_vlanptr = 30;
    user_vlan.fid = 30;
    ctc_vlan_create_uservlan(&user_vlan);

    /*Port config :AC Port:Port1*/
    gport = 0x0001;
    ctc_port_set_mac_en (gport, TRUE);
    ctc_port_set_port_en (gport, TRUE);
    ctc_port_set_vlan_filter_en(gport,CTC_BOTH_DIRECTION,TRUE);
    vlan_id = 30;
    ctc_vlan_add_port( vlan_id, gport);

    /*Port config :PW Port:Port3 :vlan if*/
    gport = 0x0003;
    ctc_port_set_port_en (gport,TRUE);
    ctc_port_set_mac_en (gport,TRUE);
    ctc_port_set_vlan_filter_en(gport,CTC_BOTH_DIRECTION,TRUE);
    vlan_id = 10;
    ctc_vlan_add_port( vlan_id, gport);

    ctc_l3if_create (l3if_id, &l3if);
    l3if_id =1;   /*allocated l3if id =1*/
    l3if.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
    l3if.vlan_id = 10;
    ctc_l3if_create (l3if_id, &l3if);
    l3if_id =3;   /*allocated l3if id =3*/
    l3if.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
    l3if.vlan_id = 30;
    ctc_l3if_create (l3if_id, &l3if);
    ctc_l3if_set_property(l3if_id,CTC_L3IF_PROP_VRF_ID,88);
    ctc_l3if_set_property(l3if_id,CTC_L3IF_PROP_VRF_EN,1);

    /* create PW1 tunnel label */
    sal_memset(&nh_tunnel_param,0,sizeof(ctc_mpls_nexthop_tunnel_param_t));
    p_nh_param = &nh_tunnel_param.nh_param;
    p_nh_param->label_num=1;
    p_nh_param->mac[0] = 0;
    p_nh_param->mac[1] = 0xb;
    p_nh_param->mac[2] = 0;
    p_nh_param->mac[3] = 0xe;
    p_nh_param->mac[4] = 0;
    p_nh_param->mac[5] = 0xd;
    p_nh_param->tunnel_label[0].lable_flag=CTC_MPLS_NH_LABEL_IS_VALID;
    p_nh_param->tunnel_label[0].label=100;
    p_nh_param->tunnel_label[0].ttl=64;
    p_nh_param->tunnel_label[0].exp_type=0;
    p_nh_param->tunnel_label[0].exp=0x02;
    ctc_nh_add_mpls_tunnel_label(1, &nh_tunnel_param);

    /* create to PW1 nexthop (LSP label + PW label) */
    sal_memset(&nh_param,0,sizeof(ctc_mpls_nexthop_param_t));
    p_nh_param_push = &(nh_param.nh_para.nh_param_push);
    p_nh_param_push->nh_com.opcode = CTC_MPLS_NH_PUSH_OP_ROUTE;
    p_nh_param_push->nh_com.oif.gport= 3;
    p_nh_param_push->nh_com.oif.vid = 10;
    p_nh_param_push->nh_com.mac[0] = 0;
    p_nh_param_push->nh_com.mac[1] = 0xb;
    p_nh_param_push->nh_com.mac[2] = 0;
    p_nh_param_push->nh_com.mac[3] = 0xe;
    p_nh_param_push->nh_com.mac[4] = 0;
    p_nh_param_push->nh_com.mac[5] = 0xd;
    p_nh_param_push->push_label[0].lable_flag=CTC_MPLS_NH_LABEL_IS_VALID;
    p_nh_param_push->push_label[0].label=10;
    p_nh_param_push->push_label[0].ttl=100;
    p_nh_param_push->push_label[0].exp_type=0;
    p_nh_param_push->push_label[0].exp=1;
    p_nh_param_push->push_label[1].lable_flag=CTC_MPLS_NH_LABEL_IS_VALID;
    p_nh_param_push->push_label[1].label=100;
    p_nh_param_push->push_label[1].ttl=64;
    p_nh_param_push->push_label[1].exp_type=0;
    p_nh_param_push->push_label[1].exp=2;
    p_nh_param_push->label_num =2;
    nhid = 102;  /*the nhid mapping from system software's nexthop_index*/
    ctc_nh_add_mpls(nhid, &nh_param);

    /*ac-->pw: create lpm route 10.3.1.1 nexthop 102*/
    sal_memset(&ipuc_info,0,sizeof(ctc_ipuc_param_t));
    ipuc_info.ip.ipv4 = 0x0a030101;
    ipuc_info.masklen =30;
    ipuc_info.vrf_id = 88;
    ipuc_info.nh_id = 102;
    ipuc_info.ip_ver = CTC_IP_VER_4;
    ipuc_info.route_flag = CTC_IPUC_FLAG_CONNECT;
    ctc_ipuc_add(&ipuc_info);

    /*2.pw -> ac config*/
    sal_memset(&mpls_ilm,0,sizeof(ctc_mpls_ilm_t));
    mpls_ilm.label = 100;
    mpls_ilm.pop=1; /*Pop LSP label*/
    mpls_ilm.decap = 1;
    ctc_mpls_add_ilm(&mpls_ilm);

    /*Decap PW label,map to vrf 88 (vrffid: 88)*/
    sal_memset(&mpls_ilm,0,sizeof(ctc_mpls_ilm_t));
    mpls_ilm.label = 200;
    mpls_ilm.nh_id = 2;
    mpls_ilm.id_type = CTC_MPLS_ID_VRF;
    mpls_ilm.flw_vrf_srv_aps.vrf_id = 88;
    mpls_ilm.type = CTC_MPLS_LABEL_TYPE_L3VPN;
    ctc_mpls_add_ilm(&mpls_ilm);

    /*create lpm route 10.3.1.1 nexthop 102*/
    sal_memset(&ipuc_info,0,sizeof(ctc_ipuc_param_t));
    ipuc_info.ip.ipv4 = 0x0a010101;
    ipuc_info.masklen =30;
    ipuc_info.nh_id = 102;
    ipuc_info.vrf_id = 88;
    ipuc_info.ip_ver = CTC_IP_VER_4;
    ipuc_info.route_flag = CTC_IPUC_FLAG_CONNECT;
    ctc_ipuc_add(&ipuc_info);

}

void mac_link_up_example()
{
    uint16 gport = 0;
    bool phy_exist = FALSE;

    /* set port enable */
    ctc_port_set_port_en(gport, TRUE);

    phy_exist = TRUE; /* get phy state from system */

    /* if phy exist ,set mac enable */
    if (phy_exist)
    {
        ctc_port_set_mac_en(gport, TRUE);
    }
}


void mac_link_down_example()
{
    uint16 gport = 0;
    bool phy_exist = FALSE;

    /* set port disable */
    ctc_port_set_port_en(gport, FALSE);

    phy_exist = TRUE; /* get phy state from system */

    /* if phy exist ,set mac disable */
    if (phy_exist)
    {
        ctc_port_set_mac_en(gport, FALSE);
    }
}

