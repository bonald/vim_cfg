#ifndef __HSRV_OPENFLOW_LINKAGG_H__
#define __HSRV_OPENFLOW_LINKAGG_H__


#define OFP_LINKAGG_TID_MIN 1
#define OFP_LINKAGG_TID_NUM 32  /* CTC_LINKAGG_MODE_32 */
#define OFP_LINKAGG_TID_MAX (OFP_LINKAGG_TID_MIN + OFP_LINKAGG_TID_NUM - 1)


/**
 * @brief Hsrv layer group linkagg master
 */
struct hsrv_openflow_linkagg_master_s
{
    ctclib_opf_t* pst_linkagg_tid;
};
typedef struct hsrv_openflow_linkagg_master_s hsrv_openflow_linkagg_master_t;

extern hsrv_openflow_linkagg_master_t* g_pst_openflow_linkagg_master;

/**
 * @brief Hsrv layer group alloc tid
 */
int32
hsrv_openflow_linkagg_offset_alloc(uint32* p_tid);

/**
 * @brief Hsrv layer group dealloc tid
 */
int32
hsrv_openflow_linkagg_offset_dealloc(uint32 tid);

/**
 * @brief Hsrv layer group linkagg create
 */
int32
hsrv_openflow_linkagg_create(uint32 tid);

/**
 * @brief Hsrv layer group linkagg delete
 */
int32
hsrv_openflow_linkagg_delete(uint32 tid);

/**
 * @brief Hsrv layer group linkagg add member
 */
int32
hsrv_openflow_linkagg_add_port(uint32 tid, uint16 gport);

/**
 * @brief Hsrv layer group linkagg delete member
 */
int32
hsrv_openflow_linkagg_del_port(uint32 tid, uint16 gport);

/**
 * @brief Hsrv layer group linkagg start
 */
int32
hsrv_openflow_linkagg_start(void);

/**
 * @brief Hsrv layer group linkagg init
 */
int32
hsrv_openflow_linkagg_init(hsrv_system_spec_info_t* p_system_info);

#endif /* !__HSRV_OPENFLOW_LINKAGG_H__ */
