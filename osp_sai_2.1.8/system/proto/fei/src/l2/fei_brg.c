
#include "proto.h"
#include "fei.h"
#include "fei_brg.h"
#include "hsrv_msg.h"

int32
fei_mlag_sync_fdb(glb_mlag_fdb_sync_t *req)
{
    uint32 len = 0;

    len = FEI_MLAG_FDB_SYNC_LEN(req->count);
    return fei_talk(FEI_MLAG_FDB_SYNC, req, len);
}

