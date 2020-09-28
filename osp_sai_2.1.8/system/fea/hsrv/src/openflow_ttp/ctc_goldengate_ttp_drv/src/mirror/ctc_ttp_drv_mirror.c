
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "ctc_ttp_drv_mirror.h"

static ctclib_opf_t *g_pst_ctc_drv_mirror_master = NULL;

int32
ctc_ttp_drv_mirror_id_alloc(uint32 *mirror_id)
{
    uint32 mirror_index = 0;
    mirror_index = ctclib_opf_alloc_offset(g_pst_ctc_drv_mirror_master, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == mirror_index)
    {
        return HSRV_E_RESOURCE_FULL;
    }
    *mirror_id = mirror_index;

    return HSRV_E_NONE;
}

int32
ctc_ttp_drv_mirror_id_free(uint32 mirror_id)
{
    return ctclib_opf_free_offset(g_pst_ctc_drv_mirror_master, 0, 1, mirror_id);
}

int32
ctc_ttp_drv_mirror_create(uint32 mirror_id, ctc_direction_t dir, uint32 gportid)
{
    ctc_mirror_dest_t   mirror_dest;
    int32   ret = 0;

    sal_memset(&mirror_dest, 0, sizeof(mirror_dest));
    mirror_dest.session_id = mirror_id & 0xFF;
    mirror_dest.dir = dir;
    mirror_dest.type = CTC_MIRROR_ACLLOG_SESSION;
    mirror_dest.dest_gport = gportid;

    ret = ctc_mirror_add_session(&mirror_dest);
    if (ret)
    {
        return ret;
    }
    
    return ret;
}

int32
ctc_ttp_drv_mirror_remove(uint32 mirror_id)
{
    ctc_mirror_dest_t   mirror_dest;
    int32   ret = 0;

    sal_memset(&mirror_dest, 0, sizeof(mirror_dest));

    mirror_dest.session_id = mirror_id & 0xFF;
    ret = ctc_mirror_remove_session(&mirror_dest);
    if (ret)
    {
        return ret;
    }
    return ret;
}

int32
ctc_ttp_drv_mirror_init()
{
    g_pst_ctc_drv_mirror_master = ctclib_opf_init(1);
    HSRV_OPENFLOW_MEM_CHECK(g_pst_ctc_drv_mirror_master);
    ctclib_opf_init_offset(g_pst_ctc_drv_mirror_master, 0, CTC_START_MIRROR_ID, CTC_MAX_MIRROR_ID);

    return 0;
}


