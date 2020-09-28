/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-07-27
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_mirror.h"
#include "gen/tbl_cpu_mirror_global_define.h"
#include "gen/tbl_cpu_mirror_global.h"
#include <linux/ip.h>
#include <sys/statfs.h> 
#include "sal_common.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/
sai_object_id_t g_hsrv_mirror_sai_sid_obj_array[GLB_MIRROR_MAX_SESSION_ID+1];

#define PETH_IPV4_VERSION_SHIFT     4
#define PETH_IPV4_VERSION_NUM       4
#define PETH_IPV6_VERSION_NUM       6

typedef struct
{
    uint16_t tag;
    uint16_t proto;
} hsrv_vlanhdr_t;

struct pcap_file_header
{
     uint32 magic;
     uint16 version_major;
     uint16 version_minor;
     uint32 thiszone;
     uint32 sigfigs;
     uint32 snaplen;
     uint32 linktype;
};

struct pcap_pkthdr
{
    struct timeval ts;    /* time stamp */
    uint32 caplen;     /* length of portion present */
    uint32 len;       /* length this packet (off wire) */
};

void pcap_dump(u_char *, const struct pcap_pkthdr *, const u_char *);

/*add by shengx for mirror cpu , 2018-8-23*/
#define HSRV_MIRROR_TO_CPU_PACKET_SIZE 9601
#define HSRV_MIRROR_TO_CPU_FILENAME_LENGTH  64
#define HSRV_MIRROR_TO_CPU_URLFILENAME_LENGTH 128
#define HSRV_MIRROR_TO_CPU_PKT_STR_LENGTH 17*3
#define HSRV_MIRROR_TO_CPU_PKT_FILE_SIZE 10*1024*1024
#define HSRV_FLASH_FREE_SIZE_THRESHOLD 50*1024*1024

/******************************************************************************
* Defines and Macros
******************************************************************************/

extern int32
hsrv_acl_entry_action_update_by_mirror(uint32 sessin_id, uint8 is_add);

/***************************************************************************************************
 * Name         : hsrv_mirror_dest_port_update_flowcontrol
 * Purpose      : flowcontrol disable if the interface is configure dest mirror.          
 * Input        : ifindex, is_creat_dest
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/
 /*added by yejl to fix bug40234*/
static int32
hsrv_mirror_dest_port_update_flowcontrol(uint32 ifindex, bool is_creat_mirror_dest)
{
    tbl_interface_t* p_db_if = NULL;

    /* 1. judge interface and brgif is exist*/
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_NONE;
    }
    if (is_creat_mirror_dest)
    {
        GLB_SET_FLAG(p_db_if->mirror_enable, MIRROR_DEST_FLAG);
        HSRV_IF_ERROR_RETURN(hsrv_interface_set_flow_control(p_db_if));
    }
    else
    {
        GLB_UNSET_FLAG(p_db_if->mirror_enable, MIRROR_DEST_FLAG);
        HSRV_IF_ERROR_RETURN(hsrv_interface_set_flow_control(p_db_if));
    }
    return HSRV_E_NONE;
}
 
/***************************************************************************************************
 * Name         : hsrv_mirror_dest_port_update_vlan
 * Purpose      : remove or add the vlan if the interface is configure dest mirror.          
 * Input        : ifindex, is_creat_dest
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/
 /*added by yejl to fix bug39334*/
static int32
hsrv_mirror_dest_port_update_vlan(uint32 ifindex, bool is_creat_mirror_dest)
{
    tbl_interface_t* p_db_if = NULL;
    ds_brgif_t *p_brgif = NULL;
    uint16     tagging_mode = 0;
    uint32     vid = 0;
    sai_object_id_t oid = 0;

    /* 1. judge interface and brgif is exist*/
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_NONE;
    }
    p_brgif = p_db_if->brgif;
    if (NULL == p_brgif)
    {
        return HSRV_E_NONE;
    }
    /* 2. judge the vlan type */
    if (p_db_if->vlan_type == GLB_VLAN_PORT_TYPE_ACCESS)
    {
        tagging_mode = SAI_VLAN_PORT_UNTAGGED;
    }
    else if (p_db_if->vlan_type == GLB_VLAN_PORT_TYPE_TRUNK || p_db_if->vlan_type == GLB_VLAN_PORT_TYPE_QINQ)
    {
        tagging_mode = SAI_VLAN_PORT_TAGGED;
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &oid));

    /* 3. do add/remove action */
    for (vid = GLB_DEF_VLAN_ID; vid < GLB_MAX_VLAN_ID; vid++)
    {
       if (GLB_BMP_ISSET(p_brgif->allowed_vlan.vlan_bmp, vid))
       {
           if (is_creat_mirror_dest)
           {
               HSRV_IF_ERROR_RETURN(hsrv_vlan_remove_port_from_vlan(vid, tagging_mode, oid));
           }
           else
           {
               /*modified by yejl for bug 48629, 2019-03-28*/
               if (GLB_BMP_ISSET(p_brgif->untagged_vlan.vlan_bmp, vid))
               {
                    HSRV_IF_ERROR_RETURN(hsrv_vlan_add_port_to_vlan(vid, SAI_VLAN_PORT_UNTAGGED, oid));
               }
               else
               {
                    HSRV_IF_ERROR_RETURN(hsrv_vlan_add_port_to_vlan(vid, tagging_mode, oid));
               }
               /*end*/
           }
       }
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_mirror_creat_mirror_session
 * Purpose      : creat mirror session          
 * Input        : session_id, dest_type,ifindex
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/

int32
hsrv_mirror_creat_mirrror_session(uint32 session_id, uint8 dest_type, uint32 ifindex, vid_t vlan_id)
{
    sai_mirror_api_t* mirror_api = NULL;
    sai_object_id_t   sai_sid_obj = 0;
    sai_object_id_t   port_oid = 0;
    sai_object_list_t  obj_list; 
    sai_object_id_t    port_sid[1];
    uint32            count = 0;
    sai_attribute_t   attr[3];
       
    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);

    sal_memset(&obj_list, 0, sizeof(obj_list));
    sal_memset(&attr, 0, sizeof(sai_attribute_t));

    /*session id can't larger than 4*/
    if (session_id > GLB_MIRROR_MAX_SESSION_ID)
    {
        return HSRV_E_INVALID_PARAM;
    }

    /*judge the mirror session type*/
    attr[count].id = SAI_MIRROR_SESSION_ATTR_TYPE;
    if (MIRROR_DEST_PORT == dest_type)
    {
        attr[count].value.s32 = SAI_MIRROR_TYPE_LOCAL;
        count++;
    }
    else if (MIRROR_DEST_VLAN == dest_type)
    {
        attr[count].value.s32 = SAI_MIRROR_TYPE_REMOTE;
        count++;
        if (GLB_DEF_VLAN_ID > vlan_id || GLB_MAX_VLAN_ID <= vlan_id)
        {
            return HSRV_E_INVALID_PARAM;
        }
        attr[count].id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
        attr[count].value.s16 = vlan_id;
        count++;
    }
    else if (MIRROR_DEST_CPU == dest_type)
    {
        attr[count].value.s32 = SAI_MIRROR_TYPE_LOCAL;
        count++;
        ifindex = GLB_CPU_IFINDEX_MIN;
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }

    /*added by yejl to fix bug 44870, 2017-08-09*/
    /*if mirror session object id is exist, remove it first*/
    sai_sid_obj = g_hsrv_mirror_sai_sid_obj_array[session_id];
    if (0 != sai_sid_obj)
    {
        //remove mirror session id 
        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_MIRROR, (void**)&mirror_api));
        HSRV_IF_ERROR_RETURN(mirror_api->remove_mirror_session(sai_sid_obj));
        g_hsrv_mirror_sai_sid_obj_array[session_id] = 0; 
        sai_sid_obj = 0;
    }
    /*ended*/
    
    
    /*judge is port or agg and get the port object id*/
    HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port_oid));
    port_sid[0] = port_oid;
    obj_list.list = port_sid;
    obj_list.count = 1;
    attr[count].id = SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    sal_memcpy(&attr[count].value.objlist, &obj_list, sizeof(sai_object_list_t));
    count++;

    /*creat mirror session and get the sssion object id*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_MIRROR,(void**)&mirror_api));
    HSRV_IF_ERROR_RETURN(mirror_api->create_mirror_session(&sai_sid_obj, count, attr));
    g_hsrv_mirror_sai_sid_obj_array[session_id] = sai_sid_obj;

    /*update acl action entry*/
    HSRV_IF_ERROR_RETURN(hsrv_acl_entry_action_update_by_mirror(session_id, TRUE));
    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_mirror_delete_mirror_session
 * Purpose      : delete mirror session          
 * Input        : session_id, SESSION ID
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/
int32
hsrv_mirror_delete_mirror_session(uint32 session_id)
{
    sai_mirror_api_t* mirror_api = NULL;
    sai_object_id_t   sai_sid_obj = 0;
    
    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);

    /*session id can't larger than 4*/
    if (session_id > GLB_MIRROR_MAX_SESSION_ID)
    {
        return HSRV_E_INVALID_PARAM;
    }

    /*update acl entry action*/
    HSRV_IF_ERROR_RETURN(hsrv_acl_entry_action_update_by_mirror(session_id, FALSE));

    /*get the session object id from soft table*/
    sai_sid_obj = g_hsrv_mirror_sai_sid_obj_array[session_id];
    if (0 == sai_sid_obj)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    /*remove mirror session id */
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_MIRROR, (void**)&mirror_api));
    HSRV_IF_ERROR_RETURN(mirror_api->remove_mirror_session(sai_sid_obj));
    g_hsrv_mirror_sai_sid_obj_array[session_id] = 0;
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_mirror_creat_mirrror_multi_dest_session
 * Purpose      : creat mirrror multi dest session    
 * Input        : session_id, p_mirror,p_db_mirror
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/
int32
hsrv_mirror_creat_mirrror_multi_dest_session(uint32 session_id, tbl_mirror_t *p_mirror)
{
    sai_mirror_api_t*  mirror_api = NULL;
    sai_object_id_t    sai_sid_obj = 0;
    sai_object_id_t    port_oid = 0;
    sai_object_list_t  obj_list; 
    sai_object_id_t    port_sid[1];
    uint32             ifindex = 0;
    sai_attribute_t    attr[3];

    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);

    sal_memset(&obj_list, 0, sizeof(obj_list));
    sal_memset(&attr, 0, sizeof(sai_attribute_t));
    
    for (ifindex = 0; ifindex < GLB_BMP_BITS(p_mirror->dest_port_bmp); ifindex++)
    {
        if (GLB_BMP_ISSET(p_mirror->dest_port_bmp, ifindex))
        {
            /*added by yejl for bug 45096 and bug 45118, set the mirror dest port in tbl_fea_brg_if */
            HSRV_IF_ERROR_RETURN(hsrv_brgif_set_mirror_dest_port_enable(ifindex, TRUE));
             /*added by yejl to fix bug39334, remove the vlan of interface if the interface is configuring mirror dest*/
            HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_vlan(ifindex, TRUE));
             /*added by yejl to fix bug40234, disable the flowcontrol if the interface is configuring mirror dest*/
            HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_flowcontrol(ifindex, TRUE));
            /*added by hansf for bug 42030,delete the interface dynamic fdb when set interface dest mirror enable */
            HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_port(ifindex));
            
            /*mirror session type*/
            attr[0].id = SAI_MIRROR_SESSION_ATTR_TYPE;
            attr[0].value.s32 = SAI_MIRROR_TYPE_LOCAL;

            /*mirror destination port*/
            HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port_oid));
            port_sid[0] = port_oid;
            obj_list.list = port_sid;
            obj_list.count = 1;
            attr[1].id = SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
            sal_memcpy(&attr[1].value.objlist, &obj_list, sizeof(sai_object_list_t));

            /*add mirror destination port or not*/
            attr[2].id = SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD;
            attr[2].value.booldata = TRUE;

            /*creat mirror session and get the mirror session object id*/
            HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_MIRROR,(void**)&mirror_api));
            HSRV_IF_ERROR_RETURN(mirror_api->create_mirror_session(&sai_sid_obj, 3, attr));
            g_hsrv_mirror_sai_sid_obj_array[session_id] = sai_sid_obj;
            
            HSRV_IF_ERROR_RETURN(hsrv_acl_entry_action_update_by_mirror(session_id, TRUE));
        }    
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_mirror_update_mirrror_multi_dest_session
 * Purpose      : update mirrror multi dest session    
 * Input        : session_id, p_mirror,p_db_mirror
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/
int32
hsrv_mirror_update_mirrror_multi_dest_session(uint32 session_id, tbl_mirror_t *p_mirror,  tbl_mirror_t *p_db_mirror)
{
    sai_mirror_api_t*  mirror_api = NULL;
    sai_object_id_t    sai_sid_obj = 0;
    sai_object_id_t    port_oid = 0;
    sai_object_list_t  obj_list; 
    sai_object_id_t    port_sid[1];
    uint32             ifindex = 0;
    sai_attribute_t    attr;

    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_MIRROR,(void**)&mirror_api));

    sal_memset(&obj_list, 0, sizeof(obj_list));
    sal_memset(&attr, 0, sizeof(sai_attribute_t));
    
    sai_sid_obj = g_hsrv_mirror_sai_sid_obj_array[session_id];
    if (0 == sai_sid_obj)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    for (ifindex = 0; ifindex < GLB_BMP_BITS(p_mirror->dest_port_bmp); ifindex++)
    {
        if (GLB_BMP_ISSET(p_mirror->dest_port_bmp, ifindex) && 
            (!GLB_BMP_ISSET(p_db_mirror->dest_port_bmp, ifindex)))
        {
            /*added by yejl for bug 45096 and bug 45118, set the mirror dest port in tbl_fea_brg_if */
            HSRV_IF_ERROR_RETURN(hsrv_brgif_set_mirror_dest_port_enable(ifindex, TRUE));
            /*added by yejl to fix bug39334, remove the vlan of interface if the interface is configuring mirror dest*/
            HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_vlan(ifindex, TRUE));
            /*added by yejl to fix bug40234, disable the flowcontrol if the interface is configuring mirror dest*/
            HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_flowcontrol(ifindex, TRUE));
            /*added by hansf for bug 42030,delete the interface dynamic fdb when set interface dest mirror enable */
            HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_port(ifindex));

            /*set add multi-dest port is TRUE*/
            attr.id = SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD;
            attr.value.booldata = TRUE;
            HSRV_IF_ERROR_RETURN(mirror_api->set_mirror_session_attribute(sai_sid_obj, &attr));

            /*set minitor port*/
            HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port_oid));
            port_sid[0] = port_oid;
            obj_list.list = port_sid;
            obj_list.count = 1;
            attr.id = SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
            sal_memcpy(&attr.value.objlist, &obj_list, sizeof(sai_object_list_t));
            HSRV_IF_ERROR_RETURN(mirror_api->set_mirror_session_attribute(sai_sid_obj, &attr));
            
        }
        else if (!GLB_BMP_ISSET(p_mirror->dest_port_bmp, ifindex) && 
                (GLB_BMP_ISSET(p_db_mirror->dest_port_bmp, ifindex)))
        {
            /*set add multi-dest port is FALSE*/
            attr.id = SAI_MIRROR_SESSION_ATTR_MULTI_DEST_MEMBER_ADD;
            attr.value.booldata = FALSE;
            HSRV_IF_ERROR_RETURN(mirror_api->set_mirror_session_attribute(sai_sid_obj, &attr));

            /*set minitor port*/
            HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port_oid));
            port_sid[0] = port_oid;
            obj_list.list = port_sid;
            obj_list.count = 1;
            attr.id = SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
            sal_memcpy(&attr.value.objlist, &obj_list, sizeof(sai_object_list_t));
            HSRV_IF_ERROR_RETURN(mirror_api->set_mirror_session_attribute(sai_sid_obj, &attr));
            
            /*added by yejl for bug 45096 and bug 45118, set the mirror dest port in tbl_fea_brg_if */
            HSRV_IF_ERROR_RETURN(hsrv_brgif_set_mirror_dest_port_enable(ifindex, FALSE));
            /*added by yejl to fix bug39334, remove the vlan of interface if the interface is configuring mirror dest*/
            HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_vlan(ifindex, FALSE));
            /*added by yejl to fix bug40234, disable the flowcontrol if the interface is configuring mirror dest*/
            HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_flowcontrol(ifindex, FALSE));
        }
    }
    return HSRV_E_NONE;
}
    
/***************************************************************************************************
 * Name         : hsrv_mirror_set_source_egs_port_bmp
 * Purpose      : set source egs port bmp        
 * Input        : session_id, p_mirror,p_db_mirror
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/

int32
hsrv_show_cpu_mirror_buf_size(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_cpu_mirror_global_t* p_cpu_mirror = tbl_cpu_mirror_global_get_cpu_mirror_global();

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "Capture strategy : %s\n", (p_cpu_mirror->strategy ?"drop":"replace"));  
    sal_fprintf(fp, "Buffer size      : %u/%u\n", CTCLIB_SLISTCOUNT(p_cpu_mirror->buf_list), p_cpu_mirror->buf_size);
    
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

hsrv_cpu_mirror_node_t*
hsrv_mirror_get_node_by_id(uint32 packet_id)
{
    tbl_cpu_mirror_global_t* p_cpu_mirror = tbl_cpu_mirror_global_get_cpu_mirror_global();
    hsrv_cpu_mirror_node_t* p_node = NULL;
    ctclib_slistnode_t* listnode   = NULL;
    uint32 index = 0;

    if (p_cpu_mirror->buf_list->count < packet_id)
    {
        return NULL;
    }
    
    CTCLIB_SLIST_LOOP(p_cpu_mirror->buf_list, p_node, listnode)
    {
        index++;
        if (index == packet_id)
        {
            return p_node;
        }
    }

    return NULL;
}

#define HSRV_TAG_GET_VID(_TAG)  (_TAG & 0x0FFF)

#define HSRV_TAG_GET_PRI(_TAG)  ((_TAG >> 13) & 0x7)

#define TPOAM_GET_LABEL_ID(_label_) \
(((_label_) >> 12) & 0xFFFFF)

#define TPOAM_GET_LABEL_EXP(_label_) \
(((_label_) >> 9) & 0x7)

#define TPOAM_GET_LABEL_S(_label_) \
(((_label_) >> 8) & 0x1)

#define TPOAM_GET_LABEL_TTL(_label_) \
((_label_) & 0xFF)


static void
hsrv_mirror_cpu_decap_pkt(uint32 ifindex, uint8 *data, uint32 data_len, FILE *fp)
{
    struct ethhdr *eth = (struct ethhdr*)data;
    hsrv_vlanhdr_t *vlan = NULL;
    hsrv_vlanhdr_t *vlan_2nd = NULL;  
    uint8 *ipda = NULL;
    uint8 *ipsa = NULL;
    uint8 *l3_start = NULL;
    struct iphdr *iph = NULL;
    uint8 *l4_ptr = NULL;
    uint32 vlan_tag_flag = FALSE;
    uint32 vlan_tag_flag_2nd = FALSE;
    uint16 vlan_tag = 0;
    uint16 vlan_tag_2nd = 0;
    uint16 tpid = 0;
    uint16 ethertype = 0;
    uint16 l4srcport = 0;
    uint16 l4dstport = 0;
    uint8 ip_ver = 0;    
    uint32 current_label = 0;
    uint32 next_label = 0;
    uint32 sbit = 0;
    uint32 i = 0;
        
    /*1. data check */
    if (NULL == data)
    {
        return;
    }

    if (data_len < 12)
    {
        return;
    }

    /*2. L2 Header*/
    sal_fprintf(fp, "MACDA:%02x%02x.%02x%02x.%02x%02x, MACSA:%02x%02x.%02x%02x.%02x%02x\n",
        eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5],
        eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5]);

    tpid = GLB_DEFAULT_TPID;
    if (sal_htons(GLB_DEFAULT_TPID) == eth->h_proto)
    {
        vlan = (hsrv_vlanhdr_t *)(eth + 1);
        ethertype = vlan->proto;
        l3_start = (uint8*)(vlan + 1);
        vlan_tag_flag = TRUE;
        vlan_tag = sal_ntohs(vlan->tag);
        if (sal_htons(tpid) == vlan->proto)
        {
            vlan_2nd = (hsrv_vlanhdr_t *)(vlan+ 1);
            ethertype = vlan_2nd->proto;
            l3_start = (uint8*)(vlan_2nd + 1);
            vlan_tag_flag_2nd = TRUE;
            vlan_tag_2nd = sal_ntohs(vlan_2nd->tag);
        }
    }
    else
    {
        ethertype = eth->h_proto;
        l3_start = (uint8*)((eth + 1));
    }
    
    if (vlan_tag_flag_2nd)
    {
        sal_fprintf(fp, "SVLAN Tag: %u, priority: %u\n", HSRV_TAG_GET_VID(vlan_tag), HSRV_TAG_GET_PRI(vlan_tag));
        sal_fprintf(fp, "CVLAN Tag: %u, priority: %u\n", HSRV_TAG_GET_VID(vlan_tag_2nd), HSRV_TAG_GET_PRI(vlan_tag_2nd));
    }
    else if (vlan_tag_flag)
    {
         sal_fprintf(fp, "VLAN Tag: %u, priority: %u\n", HSRV_TAG_GET_VID(vlan_tag), HSRV_TAG_GET_PRI(vlan_tag));
    }
    ethertype = sal_ntohs(ethertype);
    
    /*3. packet */
    switch (ethertype)
    {
    case ETH_P_IPV4:
        iph = (struct iphdr*)l3_start;
        l4_ptr = (uint8 *)((uint8*)iph +(uint32)iph->ihl*4);
        ip_ver = *(uint8 *)(iph);
        ip_ver = ip_ver >> PETH_IPV4_VERSION_SHIFT;
        if (PETH_IPV4_VERSION_NUM == ip_ver)
        {
            ipsa = ((uint8*)iph)+12;
            ipda = ((uint8*)iph)+16;
            if (6 == (uint32)iph->protocol || 17 == (uint32)iph->protocol)
            {               
                sal_fprintf(fp, "IPDA: %d.%d.%d.%d, IPSA: %d.%d.%d.%d\n",
                    ipda[0], ipda[1], ipda[2], ipda[3],
                    ipsa[0], ipsa[1], ipsa[2], ipsa[3]);
                sal_fprintf(fp, "IPv4 Packet, IP Protocol is ");
                sal_fprintf(fp, "%s", (6 == iph->protocol) ? "TCP(6)\n":"UDP(17)\n");
                l4srcport = sal_ntoh16(*(uint16 *)(l4_ptr));
                l4dstport = sal_ntoh16(*(uint16 *)(l4_ptr + 2));
                sal_fprintf(fp, "L4SourcePort:%d, L4DestinationPort:%d\n", l4srcport, l4dstport);
            }
            else
            {
                sal_fprintf(fp, "IPDA: %d.%d.%d.%d, IPSA: %d.%d.%d.%d\n",
                    ipda[0], ipda[1], ipda[2], ipda[3],
                    ipsa[0], ipsa[1], ipsa[2], ipsa[3]);
                sal_fprintf(fp, "IPv4 Packet, IP Protocol is ");
                sal_fprintf(fp, "%d\n", iph->protocol);
            }
        }
        else
        {
            sal_fprintf(fp, "Unknown IP Packet\n");
        }

        break;
    case ETH_P_IPV6:
        sal_fprintf(fp, "IPv6 Packet\n");
        break;
    case ETH_P_ARP:
        sal_fprintf(fp, "ARP Packet\n");
        break;
    case ETH_P_MPLS:
    case ETH_P_MPLS_MCAST:
        current_label = sal_ntohl(*(uint32 *)l3_start);
        sal_fprintf(fp, "EtherType is 0x%x\n", ethertype);
        sal_fprintf(fp, "MPLS Label:%d, EXP:%d, S:%d, TTL:%d\n",
            TPOAM_GET_LABEL_ID(current_label), TPOAM_GET_LABEL_EXP(current_label), TPOAM_GET_LABEL_S(current_label), TPOAM_GET_LABEL_TTL(current_label));
        /*added by yejl for bug 50100, 2019-03-14*/
        sbit = TPOAM_GET_LABEL_S(current_label);
        while(!sbit)
        {
            next_label = sal_ntohl(*(uint32 *)(l3_start + 4 + i));
            sal_fprintf(fp, "MPLS Label:%d, EXP:%d, S:%d, TTL:%d\n",
            TPOAM_GET_LABEL_ID(next_label), TPOAM_GET_LABEL_EXP(next_label), TPOAM_GET_LABEL_S(next_label), TPOAM_GET_LABEL_TTL(next_label));
            i = i + 4;
            sbit = TPOAM_GET_LABEL_S(next_label); 
        }
        break;
    default:
        sal_fprintf(fp, "EtherType is 0x%x\n", ethertype);
        break;
    }
    return;
}

int32
hsrv_show_cpu_mirror_packet_node(uint32 packet_id, hsrv_cpu_mirror_node_t* p_node, FILE *fp)
{
    uint32 length = 0;
    int j = 0;

    length = (p_node->data_len >= 128) ? 128 : p_node->data_len;
    sal_fprintf(fp, "----------------------------------------\n");
    hsrv_show_cpu_mirror_packet_add(packet_id, p_node, fp);
    sal_fprintf(fp, "Data:");
    for (j = 0; j < length; j++)
    {
         if (0 == (j % 16))
         {
            sal_fprintf(fp, "    \n");
         }
         if (0 == (j % 2))
         {
            sal_fprintf(fp, " ");
         }
         sal_fprintf(fp, "%02x", p_node->data[j]);
    }
    
    sal_fprintf(fp, "\n");
    
    if (p_node->data_len > 128)
    {
        sal_fprintf(fp, " ......\n");
    }
    
   
    return PM_E_NONE;
}

int32
hsrv_show_cpu_mirror_packet_add(uint32 packet_id, hsrv_cpu_mirror_node_t* p_node, FILE *fp)
{
    char ifname_ext[IFNAME_EXT_SIZE];

    sal_fprintf(fp, "Packet     : %d\n", packet_id);
    sal_fprintf(fp, "Source port: %s\n", IFNAME_ETH2FULL(p_node->ifname, ifname_ext));
    hsrv_mirror_cpu_decap_pkt(p_node->ifindex, p_node->data, p_node->data_len, fp);
    sal_fprintf(fp, "Data length: %d\n", p_node->data_len);
    return PM_E_NONE;
}

int32
hsrv_show_cpu_mirror_packet_info(uint32 packet_id, cfg_cmd_para_t *para)
{
    tbl_cpu_mirror_global_t* p_cpu_mirror = tbl_cpu_mirror_global_get_cpu_mirror_global();
    hsrv_cpu_mirror_node_t* p_node = NULL;
    FILE *fp = NULL;

    p_node = hsrv_mirror_get_node_by_id(packet_id);
    if (NULL == p_node)
    {
        CFG_CONFLICT_RET("This packet ID %u not exist", packet_id);
    }
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    /* 1. get node by packet_id */
    sal_mutex_lock(p_cpu_mirror->buf_mutex);
    hsrv_show_cpu_mirror_packet_node(packet_id, p_node, fp);
    sal_mutex_unlock(p_cpu_mirror->buf_mutex);
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
hsrv_show_cpu_mirror_packet_all(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_cpu_mirror_global_t* p_cpu_mirror = tbl_cpu_mirror_global_get_cpu_mirror_global();
    hsrv_cpu_mirror_node_t* p_node = NULL;
    ctclib_slistnode_t* listnode   = NULL;
    uint32 packet_id = 0;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_mutex_lock(p_cpu_mirror->buf_mutex);
    CTCLIB_SLIST_LOOP(p_cpu_mirror->buf_list, p_node, listnode)
    {
        packet_id++;
        hsrv_show_cpu_mirror_packet_node(packet_id, p_node, fp);
    }
    sal_mutex_unlock(p_cpu_mirror->buf_mutex);
    
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_mirror_set_source_igs_port_bmp
 * Purpose      : set source igs port bmp        
 * Input        : session_id, p_mirror,p_db_mirror
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/
int32
hsrv_mirror_set_source_igs_port_bmp(uint32 session_id, tbl_mirror_t *p_mirror,  tbl_mirror_t *p_db_mirror)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t  attr;
    tbl_interface_t* p_db_if = NULL;
    sai_object_id_t  sai_sid_obj[1];
    uint32           ifindex = 0;
    sai_object_id_t  port_oid = 0;
    
    sal_memset(&attr,0,sizeof(sai_attribute_t));
    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);
    
    if (session_id > GLB_MIRROR_MAX_SESSION_ID)
    {
        return HSRV_E_INVALID_PARAM;
    }
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    
    for (ifindex = 0; ifindex < sizeof(p_mirror->source_igs_port_bmp)*8; ifindex++)
    {
        if (GLB_BMP_ISSET(p_mirror->source_igs_port_bmp, ifindex) && 
           (!GLB_BMP_ISSET(p_db_mirror->source_igs_port_bmp, ifindex)))
        {
            /*set port ingress mirror session*/
            p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_db_if)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            HSRV_IF_ERROR_RETURN(hsrv_interface_get_objectid(p_db_if, &port_oid));
            attr.id = SAI_PORT_ATTR_INGRESS_MIRROR_SESSION;
            sai_sid_obj[0] = g_hsrv_mirror_sai_sid_obj_array[session_id];
            attr.value.objlist.count = 1;
            attr.value.objlist.list = sai_sid_obj;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
        }     
        else if (!GLB_BMP_ISSET(p_mirror->source_igs_port_bmp, ifindex) && 
                (GLB_BMP_ISSET(p_db_mirror->source_igs_port_bmp, ifindex)))
        {
            /*unset port ingress mirror session*/
            p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_db_if)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            HSRV_IF_ERROR_RETURN(hsrv_interface_get_objectid(p_db_if, &port_oid));
            attr.id = SAI_PORT_ATTR_INGRESS_MIRROR_SESSION;
            attr.value.objlist.count = 0;
            attr.value.objlist.list = NULL;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
        }     
    }
    return HSRV_E_NONE;
}


int32
hsrv_mirror_set_source_egs_port_bmp(uint32 session_id, tbl_mirror_t *p_mirror,  tbl_mirror_t *p_db_mirror)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t  attr;
    tbl_interface_t* p_db_if = NULL;
    sai_object_id_t  sai_sid_obj[1];
    uint32           ifindex = 0;
    sai_object_id_t  port_oid =0;
    
    sal_memset(&attr,0,sizeof(sai_attribute_t));
    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);
    
    if (session_id > GLB_MIRROR_MAX_SESSION_ID)
    {
        return HSRV_E_INVALID_PARAM;
    }    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    
    for (ifindex =0; ifindex < sizeof(p_mirror->source_egs_port_bmp)*8; ifindex++)
    {
        if(GLB_BMP_ISSET(p_mirror->source_egs_port_bmp, ifindex) && 
           (!GLB_BMP_ISSET(p_db_mirror->source_egs_port_bmp, ifindex)))
        {   
            /*set port egress mirror session*/
            p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_db_if)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            HSRV_IF_ERROR_RETURN(hsrv_interface_get_objectid(p_db_if, &port_oid));
            attr.id = SAI_PORT_ATTR_EGRESS_MIRROR_SESSION;
            sai_sid_obj[0] = g_hsrv_mirror_sai_sid_obj_array[session_id];
            attr.value.objlist.count = 1;
            attr.value.objlist.list = sai_sid_obj;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
        } 
        else if (!GLB_BMP_ISSET(p_mirror->source_egs_port_bmp, ifindex) &&
                (GLB_BMP_ISSET(p_db_mirror->source_egs_port_bmp, ifindex)))
        {
            /*unset port egress mirror session*/
            p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_db_if)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            HSRV_IF_ERROR_RETURN(hsrv_interface_get_objectid(p_db_if, &port_oid));
            attr.id = SAI_PORT_ATTR_EGRESS_MIRROR_SESSION;
            attr.value.objlist.count = 0;
            attr.value.objlist.list = NULL;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
        }
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_mirror_set_source_igs_vlan_bmp
 * Purpose      : set source igs vlan bmp
 * Input        : session_id, p_mirror, p_db_mirror
 * Output       : N/A             
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/
int32
hsrv_mirror_set_source_igs_vlan_bmp(uint32 session_id, tbl_mirror_t *p_mirror,  tbl_mirror_t *p_db_mirror)
{
    sai_vlan_api_t*  vlan_api = NULL;
    tbl_vlan_t*      p_db_vlan = NULL;
    tbl_vlan_key_t   p_vlan_key;
    sai_object_id_t  sai_sid_obj[1];
    sai_attribute_t  attr;
    vid_t            vlan_id = 0;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));
    
    sal_memset(&attr, 0, sizeof(sai_attribute_t));
    sal_memset(&p_vlan_key, 0, sizeof(tbl_vlan_key_t));
    
    if (session_id > GLB_MIRROR_MAX_SESSION_ID)
    {
        return HSRV_E_INVALID_PARAM;
    }

    for (vlan_id = GLB_VLAN_DEFAULT_VID; vlan_id <= GLB_VLAN_MAX; vlan_id++)
    {
        if (GLB_BMP_ISSET(p_mirror->source_igs_vlan_bmp, vlan_id) && 
           (!GLB_BMP_ISSET(p_db_mirror->source_igs_vlan_bmp, vlan_id)))
        {
            /*set vlan ingress mirror session*/
            p_vlan_key.vid = vlan_id;
            p_db_vlan = tbl_vlan_get_vlan(&p_vlan_key);
            if (NULL == p_db_vlan)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            attr.id = SAI_VLAN_ATTR_INGRESS_MIRROR_SESSION;
            sai_sid_obj[0] = g_hsrv_mirror_sai_sid_obj_array[session_id];
            attr.value.objlist.count = 1;
            attr.value.objlist.list = sai_sid_obj;
            HSRV_IF_ERROR_RETURN(vlan_api->set_vlan_attribute(vlan_id, &attr));
        }     
        else if (!GLB_BMP_ISSET(p_mirror->source_igs_vlan_bmp, vlan_id) && 
                (GLB_BMP_ISSET(p_db_mirror->source_igs_vlan_bmp, vlan_id)))
        {
            /*unset vlan ingress mirror session*/
            p_vlan_key.vid = vlan_id;
            p_db_vlan = tbl_vlan_get_vlan(&p_vlan_key);
            if (NULL == p_db_vlan)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            attr.id = SAI_VLAN_ATTR_INGRESS_MIRROR_SESSION;
            attr.value.objlist.count = 0;
            attr.value.objlist.list = NULL;
            HSRV_IF_ERROR_RETURN(vlan_api->set_vlan_attribute(vlan_id, &attr));
        }     
    }
    return HSRV_E_NONE;
}
    
/***************************************************************************************************
 * Name         : hsrv_mirror_set_source_egs_vlan_bmp
 * Purpose      : set source egs vlan bmp        
 * Input        : session_id, p_mirror, p_db_mirror
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/
int32
hsrv_mirror_set_source_egs_vlan_bmp(uint32 session_id, tbl_mirror_t *p_mirror,  tbl_mirror_t *p_db_mirror)
{
    sai_vlan_api_t*  vlan_api = NULL;
    tbl_vlan_t*      p_db_vlan = NULL;
    tbl_vlan_key_t   p_vlan_key;
    sai_object_id_t  sai_sid_obj[1];
    sai_attribute_t  attr;
    vid_t            vlan_id = 0;
    
    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));

    sal_memset(&attr, 0, sizeof(sai_attribute_t));
    sal_memset(&p_vlan_key, 0, sizeof(tbl_vlan_key_t));
    
    if (session_id > GLB_MIRROR_MAX_SESSION_ID)
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    for (vlan_id = GLB_VLAN_DEFAULT_VID; vlan_id <= GLB_VLAN_MAX; vlan_id++)
    {
        if (GLB_BMP_ISSET(p_mirror->source_egs_vlan_bmp, vlan_id) && 
            (!GLB_BMP_ISSET(p_db_mirror->source_egs_vlan_bmp, vlan_id)))
        {
            /*set vlan egress mirror session*/
            p_vlan_key.vid = vlan_id;
            p_db_vlan = tbl_vlan_get_vlan(&p_vlan_key);
            if (NULL == p_db_vlan)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            attr.id = SAI_VLAN_ATTR_EGRESS_MIRROR_SESSION;
            sai_sid_obj[0] = g_hsrv_mirror_sai_sid_obj_array[session_id];
            attr.value.objlist.count = 1;
            attr.value.objlist.list = sai_sid_obj;
            HSRV_IF_ERROR_RETURN(vlan_api->set_vlan_attribute(vlan_id, &attr));
        }     
        else if (!GLB_BMP_ISSET(p_mirror->source_egs_vlan_bmp, vlan_id) && 
                (GLB_BMP_ISSET(p_db_mirror->source_egs_vlan_bmp, vlan_id)))
        {
            /*unset vlan egress mirror session*/
            p_vlan_key.vid = vlan_id;
            p_db_vlan = tbl_vlan_get_vlan(&p_vlan_key);
            if (NULL == p_db_vlan)
            {
                return HSRV_E_ENTRY_NOT_EXIST;
            }
            attr.id = SAI_VLAN_ATTR_EGRESS_MIRROR_SESSION;
            attr.value.objlist.count = 0;
            attr.value.objlist.list = NULL;
            HSRV_IF_ERROR_RETURN(vlan_api->set_vlan_attribute(vlan_id, &attr));
        }     
    }
    return HSRV_E_NONE;
}

int32
hsrv_mirror_set_source_cpu_direction(uint32 session_id, tbl_mirror_t *p_mirror,  tbl_mirror_t *p_db_mirror)
{
    sai_port_api_t*  port_api = NULL; 
    sai_mirror_api_t *mirror_api = NULL;
    sai_attribute_t  attr;
    sai_object_id_t  sai_sid_obj[1];
    sai_object_id_t  port_oid =0;
    uint32 ifindex = GLB_CPU_IFINDEX_MIN;
    uint32 igs_db_en = 0;
    uint32 igs_new_en = 0;
    uint32 egs_db_en = 0;
    uint32 egs_new_en = 0;
    uint32 cpu_dest_db_en = 0;
    uint32 cpu_dest_new_en = 0;
    
    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_MIRROR,(void**)&mirror_api));

    sal_memset(&attr, 0, sizeof(sai_attribute_t));
    
    igs_db_en = GLB_FLAG_ISSET(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_IGS);
    igs_new_en = GLB_FLAG_ISSET(p_mirror->cpu_source_dir, GLB_MIRROR_DIR_IGS);
    egs_db_en = GLB_FLAG_ISSET(p_db_mirror->cpu_source_dir, GLB_MIRROR_DIR_EGS);
    egs_new_en = GLB_FLAG_ISSET(p_mirror->cpu_source_dir, GLB_MIRROR_DIR_EGS);
    
    /*igress mirror session*/
    if (igs_db_en != igs_new_en)
    {
        if (igs_new_en && !igs_db_en)
        {
            /*get port_oid*/
            HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port_oid));
            attr.id = SAI_PORT_ATTR_INGRESS_MIRROR_SESSION;
            sai_sid_obj[0] = g_hsrv_mirror_sai_sid_obj_array[session_id];
            attr.value.objlist.count = 1;
            attr.value.objlist.list = sai_sid_obj;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));    
        }
        else if (!igs_new_en && igs_db_en)
        {        
            HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port_oid));
            attr.id = SAI_PORT_ATTR_INGRESS_MIRROR_SESSION;
            sai_sid_obj[0] = g_hsrv_mirror_sai_sid_obj_array[session_id];
            attr.value.objlist.count = 0;
            attr.value.objlist.list = NULL;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));

        }
        
    }
    
    /* egress mirror session*/
    if (egs_db_en != egs_new_en)
    { 
        if (egs_new_en && !egs_db_en)
        {
            HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port_oid));
            attr.id = SAI_PORT_ATTR_EGRESS_MIRROR_SESSION;
            sai_sid_obj[0] = g_hsrv_mirror_sai_sid_obj_array[session_id];
            attr.value.objlist.count = 1;
            attr.value.objlist.list = sai_sid_obj;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
            
        }
        else if (!egs_new_en && egs_db_en)
        {
            HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(ifindex, &port_oid));
            attr.id = SAI_PORT_ATTR_EGRESS_MIRROR_SESSION;
            sai_sid_obj[0] = g_hsrv_mirror_sai_sid_obj_array[session_id];
            attr.value.objlist.count = 0;
            attr.value.objlist.list = NULL;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid, &attr));
        }
    }

    cpu_dest_db_en = (igs_db_en || egs_db_en);
    cpu_dest_new_en = (igs_new_en || egs_new_en);
    
    if (cpu_dest_db_en != cpu_dest_new_en)
    {
        attr.id = SAI_MIRROR_SESSION_ATTR_CPU_DEST_EN;
        attr.value.u32 = cpu_dest_new_en;
        HSRV_IF_ERROR_RETURN(mirror_api->set_mirror_session_attribute(sai_sid_obj[0], &attr));
    }
    
    return HSRV_E_NONE;
   
}



hsrv_cpu_mirror_node_t*
hsrv_mirror_get_node()
{
    tbl_cpu_mirror_global_t *p_global = tbl_cpu_mirror_global_get_cpu_mirror_global();
    hsrv_cpu_mirror_node_t *p_node = NULL;
    ctclib_slistnode_t *p_head = NULL;

    if (p_global->buf_list->count < p_global->buf_size)
    {
        p_node = XCALLOC(MTYPE_TMP, sizeof(hsrv_cpu_mirror_node_t));
        return p_node;
    }

    if (GLB_CPU_MIRROR_STRATEGY_REPLACE == p_global->strategy)
    {
        p_head = CTCLIB_SLISTHEAD(p_global->buf_list);
        if (NULL == p_head)
        {
            return NULL;
        }
        p_node = p_head->data;
        if (NULL == p_node)
        {
            return NULL;
        }        
        ctclib_slistnode_delete(p_global->buf_list, p_node);
        sal_memset(p_node, 0, sizeof(hsrv_cpu_mirror_node_t));
        return p_node;
    }
    else
    {
        return NULL;
    }
}

int32
hsrv_mirror_to_cpu_rx_packet(uint8 * h_data, uint32 h_data_len, tbl_interface_t *p_db_if)
{
    tbl_cpu_mirror_global_t *p_global = tbl_cpu_mirror_global_get_cpu_mirror_global();
    hsrv_cpu_mirror_node_t *p_node = NULL;

    /* 1. parameter check */
    if (h_data_len >= MIRROR_TO_CPU_PACKET_SIZE)
    {
        return HSRV_E_DATA_IS_TOO_LONG;
    }

    if (h_data == NULL)
    {
        return HSRV_E_DATA_IS_NULL;
    }

    sal_mutex_lock(p_global->buf_mutex);

    /* 2. get node */
    p_node = hsrv_mirror_get_node();
    if (NULL == p_node)
    {
        sal_mutex_unlock(p_global->buf_mutex);
        return HSRV_E_NONE;
    }
    
    /* 3. assign packet info to node */
    p_node->data_len = (h_data_len < MIRROR_TO_CPU_PACKET_SIZE) ? h_data_len : MIRROR_TO_CPU_PACKET_SIZE;
    sal_memcpy(p_node->data, h_data, p_node->data_len);
    sal_strcpy(p_node->ifname, p_db_if->key.name);
    p_node->ifindex = p_db_if->ifindex;
    p_node->packet_is_written = FALSE;
    sal_gettimeofday(&p_node->tv);

    /* 4. add node to list */
    ctclib_slistnode_insert(p_global->buf_list, p_node);
    
    sal_mutex_unlock(p_global->buf_mutex);
        
    return HSRV_E_NONE;
}

int64_t
_hsrv_mirror_get_disk_free_size()
{
    struct statfs st;
    int64_t llSize = 0ll;
    int64_t llSizefree = 0ll;
    
    if (statfs(MIRROR_TO_CPU_FLASH_DIR, &st) == 0) 
    {
        llSize += (int64_t)st.f_blocks * st.f_bsize;
        llSizefree += (int64_t)st.f_bavail * st.f_bsize;
    }

    return llSizefree;
}
    
int32
hsrv_mirror_cpu_packet_file(uint32 packet_id, hsrv_cpu_mirror_node_t* p_node, FILE *fp, tbl_cpu_mirror_global_t *p_global, char *file_name_str, char *file_name)
{
    int64_t hsrv_flash_free_size = 0;
    int32 ret = 0;
    int32 j = 0;
    struct stat sbuf;
    
    /*1. check whether packets in buffer have been written*/
    if (!p_node->packet_is_written)
    {   
        hsrv_show_cpu_mirror_packet_add(packet_id, p_node, fp);
        sal_fprintf(fp, "Timestamp: %ld:%ld\n", p_node->tv.tv_sec, p_node->tv.tv_usec);
        sal_fprintf(fp, "Data:");
        for (j = 0; j < p_node->data_len; j++)
        {
             if (0 == (j % 16))
             {
                sal_fprintf(fp, "    \n");
             }
             if (0 == (j % 2))
             {
                sal_fprintf(fp, " ");
             }
             sal_fprintf(fp, "%02x", p_node->data[j]);
        }
        packet_id = packet_id + 1;

        hsrv_flash_free_size = _hsrv_mirror_get_disk_free_size();
        if (hsrv_flash_free_size < p_global->flash_size_threshold)
        {
            HSRV_LOG_WARN(" Memory of flash is full, stop write to file");
            ret = HSRV_E_RESOURCE_FULL;
        }
        sal_fprintf(fp, "\n----------------------------------------\n");
        if (!sal_stat(file_name, &sbuf))
        {
            if (sbuf.st_size > HSRV_MIRROR_TO_CPU_PKT_FILE_SIZE)
            {
                HSRV_LOG_WARN(" Mirror to cpu packet file flash:/mirror/%s has been written full", file_name_str);
                ret = HSRV_E_RESOURCE_FULL;
            }
        }
        else
        {
            HSRV_LOG_WARN(" flash:/mirror/%s has been removed or renamed\n", file_name_str);
            ret = HSRV_E_FAIL;
        }
    }
    p_node->packet_is_written = TRUE;
    
    return ret;
}

int32
hsrv_pcap_write_header(FILE *fp, int linktype, int thiszone, int snaplen)
{
    struct pcap_file_header hdr;
    hdr.magic = TCPDUMP_MAGIC;
    hdr.version_major = PCAP_VERSION_MAJOR;
    hdr.version_minor = PCAP_VERSION_MINOR;
    hdr.thiszone = thiszone;
    hdr.snaplen = snaplen;
    hdr.sigfigs = 0;
    hdr.linktype = linktype;
    
    if (sal_fwrite((char *)&hdr, sizeof(hdr), 1, fp) != 1)
    {
        return -1;
    }
    return 0;
}

int32
hsrv_mirror_txt_pkt_to_pcap(char *src_file, char *dst_file)
{   
    uint16 cnt = 0;
    int32 ret = 0;
    bool find_pkt = FALSE;
    FILE *fp_pacp = NULL;
    FILE *fp_txt = NULL;
    uint8_t pkt_data[CONVERT_PACKET_MAX_LENGTH] = {0};
    char line_buf[256] = {0};
    struct timeval tv;
    struct pcap_pkthdr ph;

    fp_pacp = sal_fopen(dst_file, "w+");
    if (!fp_pacp)
    {
        return -1;
    }
    hsrv_pcap_write_header(fp_pacp, LINKTYPE_ETHERNET, 0x0, 0x0000ffff);

    fp_txt = sal_fopen(src_file, "r");

    if (!fp_txt)
    {
        sal_fclose(fp_pacp);
        return -2;
    }

    while (sal_fgets(line_buf, 256, fp_txt) != NULL)
    {
        if (!strncasecmp(line_buf, "Timestamp:", 10))
        {
            sscanf(line_buf, "Timestamp: %d:%d", (int *)&tv.tv_sec, (int *)&tv.tv_usec);
            continue;
        }
        if (!strncasecmp(line_buf, "Data:", 5))
        {
            find_pkt = TRUE;
            continue;
        }
        if (!find_pkt)
        {
            continue;
        }
        if (strncasecmp(line_buf, "----------------------------------------", 40))
        {
            if (cnt > (CONVERT_PACKET_MAX_LENGTH -16))
            {
                continue;
            }
            ret = sscanf(line_buf, "%2hhx%2hhx %2hhx%2hhx %2hhx%2hhx %2hhx%2hhx %2hhx%2hhx %2hhx%2hhx %2hhx%2hhx %2hhx%2hhx", 
                &pkt_data[cnt],&pkt_data[cnt+1],&pkt_data[cnt+2],&pkt_data[cnt+3],
                &pkt_data[cnt+4],&pkt_data[cnt+5],&pkt_data[cnt+6],&pkt_data[cnt+7],
                &pkt_data[cnt+8],&pkt_data[cnt+9],&pkt_data[cnt+10],&pkt_data[cnt+11],
                &pkt_data[cnt+12],&pkt_data[cnt+13],&pkt_data[cnt+14],&pkt_data[cnt+15]);
            cnt += ret;
        }
        else
        {   
            ph.ts.tv_sec = tv.tv_sec;
            ph.ts.tv_usec = tv.tv_usec;
            ph.caplen = cnt;
            ph.len    = cnt;
            pcap_dump((uint8_t *)fp_pacp, &ph, pkt_data);
            fflush(fp_pacp);
            sal_memset(pkt_data, 0, sizeof(pkt_data));
            cnt = 0;
            find_pkt = FALSE;
        }
    }
    sal_fclose(fp_txt);	
    sal_fclose(fp_pacp);	
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_mirror_sync
 * Purpose      : load mirror cdb to sai          
 * Input        : p_tbl_node: table node pointer
                     p_ds_node: ds node pointer
                     oper: operation code
                     field_id: field id with operation
                     p_tbl: cdb table pointer
                     p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_mirror_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_mirror_t* p_mirror = NULL;
    tbl_mirror_t* p_db_mirror = NULL;
    uint32        ifindex = 0;
    
    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);  
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_mirror = (tbl_mirror_t*)p_tbl;
        switch (field_id)
        {
        case TBL_MIRROR_FLD_SOURCE_IGS_PORT_BMP:
            p_db_mirror = tbl_mirror_get_mirror(&p_mirror->key); 
            HSRV_PTR_CHECK(p_db_mirror);
            HSRV_IF_ERROR_RETURN(hsrv_mirror_set_source_igs_port_bmp(p_mirror->key.id, p_mirror, p_db_mirror));
            break;

        case TBL_MIRROR_FLD_SOURCE_EGS_PORT_BMP:
            p_db_mirror = tbl_mirror_get_mirror(&p_mirror->key); 
            HSRV_PTR_CHECK(p_db_mirror);
            HSRV_IF_ERROR_RETURN(hsrv_mirror_set_source_egs_port_bmp(p_mirror->key.id, p_mirror, p_db_mirror));
            break;
        case TBL_MIRROR_FLD_SOURCE_IGS_VLAN_BMP:
            p_db_mirror = tbl_mirror_get_mirror(&p_mirror->key); 
            HSRV_PTR_CHECK(p_db_mirror);
            HSRV_IF_ERROR_RETURN(hsrv_mirror_set_source_igs_vlan_bmp(p_mirror->key.id, p_mirror, p_db_mirror));
            break;
            
        case TBL_MIRROR_FLD_SOURCE_EGS_VLAN_BMP:
            p_db_mirror = tbl_mirror_get_mirror(&p_mirror->key); 
            HSRV_PTR_CHECK(p_db_mirror);
            HSRV_IF_ERROR_RETURN(hsrv_mirror_set_source_egs_vlan_bmp(p_mirror->key.id, p_mirror, p_db_mirror));
            break;
        case TBL_MIRROR_FLD_CPU_SOURCE_DIR:
            p_db_mirror = tbl_mirror_get_mirror(&p_mirror->key);
            HSRV_PTR_CHECK(p_db_mirror);
            HSRV_IF_ERROR_RETURN(hsrv_mirror_set_source_cpu_direction(p_mirror->key.id, p_mirror, p_db_mirror));
            break;
            
        case TBL_MIRROR_FLD_DEST_PORT:
            break;
            
      /* PM set field with order TBL_MIRROR_FLD_DEST_PORT, TBL_MIRROR_FLD_DEST_VLAN, TBL_MIRROR_FLD_DEST_TYPE
          * the port,vlan,type is ready when set TBL_MIRROR_FLD_DEST_TYPE
          */
        case TBL_MIRROR_FLD_DEST_TYPE:
            p_db_mirror = tbl_mirror_get_mirror(&p_mirror->key); 
            HSRV_PTR_CHECK(p_db_mirror);
            if (MIRROR_DEST_PORT == p_mirror->dest_type || MIRROR_DEST_VLAN == p_mirror->dest_type || MIRROR_DEST_CPU == p_mirror->dest_type)
            {
                 if (MIRROR_DEST_PORT == p_mirror->dest_type)
                 {
                     /*added by yejl for bug 45096 and bug 45118, set the mirror dest port in tbl_fea_brg_if */
                     HSRV_IF_ERROR_RETURN(hsrv_brgif_set_mirror_dest_port_enable(p_db_mirror->dest_port, TRUE));
                     /*added by yejl to fix bug39334, remove the vlan of interface if the interface is configuring mirror dest*/
                     HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_vlan(p_db_mirror->dest_port, TRUE));
                     /*added by yejl to fix bug40234, disable the flowcontrol if the interface is configuring mirror dest*/
                     HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_flowcontrol(p_db_mirror->dest_port, TRUE));
                     /*added by hansf for bug 42030,delete the interface dynamic fdb when set interface dest mirror enable */
                     HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_port(p_db_mirror->dest_port));
                 }
                    HSRV_IF_ERROR_RETURN(hsrv_mirror_creat_mirrror_session(p_mirror->key.id, p_mirror->dest_type, 
                    p_db_mirror->dest_port, p_db_mirror->dest_vlan));
            }
            else if (MIRROR_DEST_NONE == p_mirror->dest_type)
            {
                 HSRV_IF_ERROR_RETURN(hsrv_mirror_delete_mirror_session(p_mirror->key.id));

                 /*modified by yejl to fix bug 45128, 2017-09-06*/
                 if (MIRROR_DEST_PORT == p_db_mirror->dest_type)
                 {
                     /*added by yejl for bug 45096 and bug 45118, set the mirror dest port in tbl_fea_brg_if */
                     HSRV_IF_ERROR_RETURN(hsrv_brgif_set_mirror_dest_port_enable(p_db_mirror->dest_port, FALSE));
                     /*added by yejl to fix bug39334, add the vlan of interface if the interface is removing mirror dest*/
                     HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_vlan(p_db_mirror->dest_port, FALSE));
                     /*added by yejl to fix bug40234, enable the flowcontrol if the interface is configuring mirror dest*/
                     HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_flowcontrol(p_db_mirror->dest_port, FALSE));
                 }
                 else if (MIRROR_DEST_GROUP == p_db_mirror->dest_type)
                 {
                     for (ifindex = 0; ifindex < GLB_BMP_BITS(p_db_mirror->dest_port_bmp); ifindex++)
                     {
                        if (GLB_BMP_ISSET(p_db_mirror->dest_port_bmp, ifindex))
                        {
                             /*added by yejl for bug 45096 and bug 45118, set the mirror dest port in tbl_fea_brg_if */
                             HSRV_IF_ERROR_RETURN(hsrv_brgif_set_mirror_dest_port_enable(ifindex, FALSE));
                             /*added by yejl to fix bug39334, add the vlan of interface if the interface is removing mirror dest*/
                             HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_vlan(ifindex, FALSE));
                             /*added by yejl to fix bug40234, enable the flowcontrol if the interface is configuring mirror dest*/
                             HSRV_IF_ERROR_RETURN(hsrv_mirror_dest_port_update_flowcontrol(ifindex, FALSE));
                        }
                     }    
                 }
                 /*ended by yejl*/
                 
            }
            break;
        case TBL_MIRROR_FLD_DEST_PORT_BMP:
            p_db_mirror = tbl_mirror_get_mirror(&p_mirror->key); 
            HSRV_PTR_CHECK(p_db_mirror);
            if (MIRROR_DEST_GROUP == p_mirror->dest_type)
            {
                if (0 == g_hsrv_mirror_sai_sid_obj_array[p_mirror->key.id])
                {                    
                    HSRV_IF_ERROR_RETURN(hsrv_mirror_creat_mirrror_multi_dest_session(p_mirror->key.id, p_mirror));
                }
                else
                {
                    HSRV_IF_ERROR_RETURN(hsrv_mirror_update_mirrror_multi_dest_session(p_mirror->key.id, p_mirror, p_db_mirror));
                }
            }
            break;
        default:
            break;
        }
        break;
        
    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

int32
hsrv_cmd_act_cpu_mirror(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 packet_id = 0;
    char src_file[M_FULLPATH_MAX_LEN * 2];
    char dst_file[M_FULLPATH_MAX_LEN * 2];
    char *p = NULL;
    int32 i = 0;
    int32 rc = PM_E_NONE;
    
    switch (para->oper)
    { 
    case CDB_OPER_GET:
         
        if (0 == sal_strcmp(argv[0], "capture"))
        {
            hsrv_show_cpu_mirror_buf_size(para);
        }
        else if (0 == sal_strcmp(argv[0], "packet"))
        {
            if (0 == sal_strcmp(argv[1], "all"))
            {
                hsrv_show_cpu_mirror_packet_all(para);
            }
            else
            {
                PM_CFG_GET_UINT(packet_id, argv[1], para->p_rs);
                rc = hsrv_show_cpu_mirror_packet_info(packet_id, para);
            }
        }        
        break;
    case CDB_OPER_SET:
        sal_sprintf(src_file, FULL_NAME_FLASH);
        sal_strcat(src_file, "/");
        for (i = 1; i < argc-1; i++)
        {
            sal_strcat(src_file, argv[i]);
            sal_strcat(src_file, "/");
        }
        sal_strcat(src_file, argv[argc-1]);
        sal_strcpy(dst_file, src_file);
        p = sal_strstr(dst_file, ".txt");
        if (NULL == p)
        {
            CFG_INVALID_PARAM_RET("Not txt file");
        }
        if (!syscmd_file_exist(src_file))
        {
            CFG_INVALID_PARAM_RET("No such txt file error");
        }
        p[0] = '\0';
        sal_strcat(dst_file, ".pcap");
        hsrv_mirror_txt_pkt_to_pcap(src_file, dst_file);
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;;
}

int32
hsrv_mirror_mac_escape_add(tbl_mirror_mac_escape_t *p_mac_escape )
{
    sai_mac_escape_api_t*  macescape_api = NULL;
    sai_mac_escape_entry_t macescape_addr;

    if (NULL == p_mac_escape)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    sal_memset(&macescape_addr, 0, sizeof(macescape_addr));
    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_MAC_ESCAPE,(void**)&macescape_api));

    sal_memcpy(macescape_addr.mac, p_mac_escape->key.mac, sizeof(mac_addr_t));
    sal_memcpy(macescape_addr.mask, p_mac_escape->key.mask, sizeof(mac_addr_t));
    HSRV_IF_ERROR_RETURN(macescape_api->create_mac_escape_entry(&macescape_addr));
    
    return HSRV_E_NONE;
}

int32
hsrv_mirror_mac_escape_del(tbl_mirror_mac_escape_t *p_db_mac_escape)
{
    sai_mac_escape_api_t*  macescape_api = NULL;
    sai_mac_escape_entry_t macescape_addr;

    if (NULL == p_db_mac_escape)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    sal_memset(&macescape_addr, 0, sizeof(macescape_addr));
    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_MAC_ESCAPE,(void**)&macescape_api));
    
    sal_memcpy(macescape_addr.mac, p_db_mac_escape->key.mac, sizeof(mac_addr_t));
    sal_memcpy(macescape_addr.mask, p_db_mac_escape->key.mask, sizeof(mac_addr_t));
    HSRV_IF_ERROR_RETURN(macescape_api->delete_mac_escape_entry(&macescape_addr));

    return HSRV_E_NONE;
}

int32
hsrv_mirror_mac_escape_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_mirror_mac_escape_t *p_mac_escape = NULL;
    tbl_mirror_mac_escape_t *p_db_mac_escape = NULL;
    tbl_mirror_mac_escape_key_t* mac_escape_key = NULL;

    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_mac_escape = (tbl_mirror_mac_escape_t*)p_tbl;
        HSRV_PTR_CHECK(p_mac_escape);
        HSRV_IF_ERROR_RETURN(hsrv_mirror_mac_escape_add(p_mac_escape));
        break;

    case CDB_OPER_DEL:
        mac_escape_key = (tbl_mirror_mac_escape_key_t*)p_tbl;
        p_db_mac_escape = tbl_mirror_mac_escape_get_mirror_mac_escape(mac_escape_key);
        HSRV_PTR_CHECK(p_db_mac_escape);
        HSRV_IF_ERROR_RETURN(hsrv_mirror_mac_escape_del(p_db_mac_escape));
        break;

    case CDB_OPER_SET:
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_cpu_mirror_set_strategy(uint32 strategy)
{
    tbl_cpu_mirror_global_t cpu_mirror;
    int32 rc = PM_E_NONE;
    
    /* update local CDB */
    cpu_mirror.strategy = strategy;
    PM_E_RETURN(tbl_cpu_mirror_global_set_cpu_mirror_global_field(&cpu_mirror, TBL_CPU_MIRROR_GLOBAL_FLD_STRATEGY));
    
    return rc;
}

int32
hsrv_cpu_mirror_set_buf_size(uint32 buf_size)
{
    tbl_cpu_mirror_global_t *p_db_glb = tbl_cpu_mirror_global_get_cpu_mirror_global();
    tbl_cpu_mirror_global_t cpu_mirror;
    int32 rc = PM_E_NONE;

    /*modified by yejl for bug 50808, 2019-03-11*/
    sal_memset(&cpu_mirror, 0, sizeof(cpu_mirror));
    if (p_db_glb->buf_size > buf_size)
    {
        /*update the buf_size to CDB*/
        cpu_mirror.buf_size = buf_size;
        PM_E_RETURN(tbl_cpu_mirror_global_set_cpu_mirror_global_field(&cpu_mirror, TBL_CPU_MIRROR_GLOBAL_FLD_BUF_SIZE));
        hsrv_cpu_mirror_clear_cpu_mirror_buffer();
    }
    else
    {
        /*update the buf_size to CDB*/
        cpu_mirror.buf_size = buf_size;
        PM_E_RETURN(tbl_cpu_mirror_global_set_cpu_mirror_global_field(&cpu_mirror, TBL_CPU_MIRROR_GLOBAL_FLD_BUF_SIZE));
    }

    return rc;
}

static int32
_hsrv_cpu_mirror_cmd_set_strategy(uint32 strategy, tbl_cpu_mirror_global_t* p_cpu_mirror, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if ((GLB_CPU_MIRROR_STRATEGY_REPLACE != strategy) && (GLB_CPU_MIRROR_STRATEGY_DROP != strategy))
    {
        CFG_INVALID_PARAM_RET("Invalid strategy");
    }

    /* 2. check duplicate config */
    if (strategy == p_cpu_mirror->strategy)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hsrv_cpu_mirror_set_strategy(strategy));
    
    return rc;
}

static int32
_hsrv_cpu_mirror_cmd_set_buf_size(uint32 buf_size, tbl_cpu_mirror_global_t* p_cpu_mirror, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (GLB_CPU_MIRROR_BUF_SIZE_DEF < buf_size)
    {
        CFG_INVALID_PARAM_RET("Invalid buffer size %u", buf_size);
    }

    /* 2. check duplicate config */
    if (buf_size == p_cpu_mirror->buf_size)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(hsrv_cpu_mirror_set_buf_size(buf_size));
    
    return rc;
}

static int32
_hsrv_cpu_mirror_global_cmd_get_cpu_mirror_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_cpu_mirror_global_t* p_cpu_mirror = tbl_cpu_mirror_global_get_cpu_mirror_global();
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_cpu_mirror_global_dump_one(p_cpu_mirror, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_cpu_mirror_clear_cpu_mirror_buffer()
{
    tbl_cpu_mirror_global_t* p_cpu_mirror = tbl_cpu_mirror_global_get_cpu_mirror_global();
    hsrv_cpu_mirror_node_t* p_node = NULL;
    ctclib_slistnode_t* listnode   = NULL;
    ctclib_slistnode_t* next   = NULL;

    sal_mutex_lock(p_cpu_mirror->buf_mutex);
    CTCLIB_SLIST_LOOP_DEL(p_cpu_mirror->buf_list, p_node, listnode, next)
    {   
        ctclib_slistnode_delete(p_cpu_mirror->buf_list, p_node);
        HSRV_FREE(CTCLIB_MEM_MIRROR_MODULE, p_node);
    }
    sal_mutex_unlock(p_cpu_mirror->buf_mutex);

    return PM_E_NONE;
}

static char g_file_name_str[HSRV_MIRROR_TO_CPU_FILENAME_LENGTH];
static char g_file_name[HSRV_MIRROR_TO_CPU_FILENAME_LENGTH];

int32
hsrv_mirror_cpu_packet_capture_open_fp(tbl_cpu_mirror_global_t *p_global)
{
    FILE* fp = NULL;
    char url_name[HSRV_MIRROR_TO_CPU_FILENAME_LENGTH] = {0};
    sal_time_t date;
    struct tm *date_time = NULL;
    
    sal_time(&date);
    date_time = localtime(&date);
    if (NULL == date_time)
    {
        return HSRV_E_FAIL;
    }
    sal_snprintf(g_file_name_str, HSRV_MIRROR_TO_CPU_FILENAME_LENGTH, "%s-%04d-%02d-%02d-%02d-%02d-%02d", MIRROR_TO_CPU_PACKET_FILE_NAME, date_time->tm_year+1900, 
        date_time->tm_mon+1, date_time->tm_mday, date_time->tm_hour, date_time->tm_min, date_time->tm_sec);
    sal_snprintf(url_name, HSRV_MIRROR_TO_CPU_FILENAME_LENGTH, "%s", MIRROR_TO_CPU_PACKET_FILE_DIR);
    sal_mkdir (url_name, 0777);
    sal_snprintf(g_file_name, HSRV_MIRROR_TO_CPU_URLFILENAME_LENGTH, "%s/%s.txt", url_name, g_file_name_str);
    fp = sal_fopen(g_file_name, "wb+");
    if (fp == NULL)
    {
        HSRV_LOG_WARN(" Open file flash:/mirror/%s failed\n", g_file_name_str);
        return HSRV_E_FAIL;
    }

    p_global->capture_fp = fp;
    
    return HSRV_E_NONE;
}

void
hsrv_mirror_cpu_packet_capture_cb()
{
    tbl_cpu_mirror_global_t *p_global = tbl_cpu_mirror_global_get_cpu_mirror_global();
    hsrv_cpu_mirror_node_t *p_node = NULL;
    ctclib_slistnode_t* listnode = NULL;
    FILE* fp = p_global->capture_fp;
    uint32 packet_id = 0;
    int32 ret = 0;

    p_global->capture_timer = NULL;
    
    sal_mutex_lock(p_global->buf_mutex);
    CTCLIB_SLIST_LOOP(p_global->buf_list, p_node, listnode)
    {         
        packet_id++;
        ret = hsrv_mirror_cpu_packet_file(packet_id, p_node, fp, p_global, g_file_name_str, g_file_name);
        if (ret < 0)
        {
            sal_mutex_unlock(p_global->buf_mutex);
            goto ERROR;
        }        
        sal_fflush(fp);
    }
    sal_mutex_unlock(p_global->buf_mutex);

    p_global->capture_timer = ctc_task_add_timer(TASK_PRI_NORMAL, hsrv_mirror_cpu_packet_capture_cb, NULL, 2);
    return;

ERROR:
    if (fp)
    {
        sal_fclose(fp);
        p_global->capture_fp = NULL;
    }

    return;
}

int32
hsrv_cpu_mirror_cmd_start_capture_packet(cfg_cmd_para_t *para)
{
    int64_t hsrv_flash_free_size = 0;
    tbl_mirror_master_t *p_mirror_master = tbl_mirror_get_master();
    tbl_cpu_mirror_global_t *p_global = tbl_cpu_mirror_global_get_cpu_mirror_global();    
    tbl_mirror_t * p_mirror = NULL;    
    uint32 id = 0;
    uint32 has_cpu_dest = FALSE;
    int32 rc = HSRV_E_NONE;
    
    if (p_global->capture_timer)
    {
        CFG_CONFLICT_RET("Capture packets of mirror to cpu is starting");
        return PM_E_NONE;
    }
    
    for (id = 0; id < GLB_MIRROR_MAX_SESSION_ID+1; id++)
    {
        p_mirror = p_mirror_master->mirror_array[id];
        if (NULL == p_mirror)
        {
            continue;
        }
        if (MIRROR_DEST_CPU == p_mirror->dest_type)
        {
            has_cpu_dest = TRUE;
            break;
        }
    } 

    if (!has_cpu_dest)
    {
        CFG_CONFLICT_RET("CPU mirror session does not exist");
    }

    hsrv_flash_free_size = _hsrv_mirror_get_disk_free_size();
    if (hsrv_flash_free_size < p_global->flash_size_threshold)
    {
        CFG_CONFLICT_RET("Memory of flash is full");
        return PM_E_NONE;
    }

    rc = hsrv_mirror_cpu_packet_capture_open_fp(p_global);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Cannot open capture file");
        return PM_E_NONE;
    }

    p_global->capture_timer = ctc_task_add_timer(TASK_PRI_NORMAL, hsrv_mirror_cpu_packet_capture_cb, NULL, 0);
    
    return PM_E_NONE;
}

int32
hsrv_cpu_mirror_cmd_stop_capture_packet(cfg_cmd_para_t *para)
{
    tbl_cpu_mirror_global_t *p_global = tbl_cpu_mirror_global_get_cpu_mirror_global();
    FILE* fp = p_global->capture_fp;
    
    if (p_global->capture_timer)
    {
        ctc_task_delete_timer(p_global->capture_timer);
        p_global->capture_timer = NULL;
    }
    if (fp)
    {
        sal_fclose(fp);
        p_global->capture_fp = NULL;
    }
    
    return PM_E_NONE;
}

int32
hsrv_cmd_process_cpu_mirror_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    tbl_cpu_mirror_global_t *p_global = tbl_cpu_mirror_global_get_cpu_mirror_global();
    cdb_node_t* p_node = NULL;
    uint32 value = 0;
    uint32 i = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CPU_MIRROR_GLOBAL);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        
        switch (field_id)
        {
        case TBL_CPU_MIRROR_GLOBAL_FLD_STRATEGY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (0 == sal_strcmp(argv[1], "drop"))
            {
                value = GLB_CPU_MIRROR_STRATEGY_DROP;
            }
            else
            {
                value = GLB_CPU_MIRROR_STRATEGY_REPLACE;
            }         
            rc = _hsrv_cpu_mirror_cmd_set_strategy(value, p_global, para);
            break;
            
        case TBL_CPU_MIRROR_GLOBAL_FLD_BUF_SIZE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _hsrv_cpu_mirror_cmd_set_buf_size(value, p_global, para);
            break;

        case TBL_CPU_MIRROR_GLOBAL_FLD_WRITE_FILE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (0 == sal_strcmp("start", argv[1]))
            {
                rc = hsrv_cpu_mirror_cmd_start_capture_packet(para);
            }
            else
            {
                rc = hsrv_cpu_mirror_cmd_stop_capture_packet(para);
            }
            break;
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;      
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _hsrv_cpu_mirror_global_cmd_get_cpu_mirror_global(para, p_node, &field);
        break;
    case CDB_OPER_DEL:
        rc = hsrv_cpu_mirror_clear_cpu_mirror_buffer();
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
hsrv_mirror_start()
{
    tbl_cpu_mirror_global_t *p_global = tbl_cpu_mirror_global_get_cpu_mirror_global();

    p_global->buf_size = GLB_CPU_MIRROR_BUF_SIZE_DEF;
    p_global->strategy = GLB_CPU_MIRROR_STRATEGY_REPLACE;
    p_global->flash_size_threshold = HSRV_FLASH_FREE_SIZE_THRESHOLD;
    tbl_cpu_mirror_global_set_cpu_mirror_global_field(p_global, TBL_CPU_MIRROR_GLOBAL_FLD_BUF_SIZE);
    tbl_cpu_mirror_global_set_cpu_mirror_global_field(p_global, TBL_CPU_MIRROR_GLOBAL_FLD_STRATEGY);
    
    p_global->buf_list = ctclib_slist_create(NULL, NULL);
    sal_mutex_create(&p_global->buf_mutex);
    
    cdb_register_cfg_tbl_cb(TBL_CPU_MIRROR_GLOBAL, hsrv_cmd_process_cpu_mirror_global);
    return HSRV_E_NONE;
}

