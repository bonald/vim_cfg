
#include "proto.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"

uint32 g_vty_fail_count = 0;

extern int
vtyclient_init();

int32
rt_vty_init();

extern int 
vtysh_execute(const char *line);

int32
rt_vty_if_cmd(char *ifname, char *cmd)
{
    char ifcmd[CMD_BUF_64];
    tbl_route_global_t *p_glb = tbl_route_global_get_route_global();
    int rc = 0;

    if (!p_glb->vty_connected)
    {
        rt_vty_init();
        if (!p_glb->vty_connected)
        {
            p_glb->vty_nonconn_count++;
            return PM_E_SOCK;
        }
    }

    switch (p_glb->vty_mode)
    {
    case GLB_VTY_MODE_EXEC:
         vtysh_execute("configure terminal");
         break;
    case GLB_VTY_MODE_CONFIG:
         break;
    case GLB_VTY_MODE_INTERFACE:
    case GLB_VTY_MODE_ROUTER:
         vtysh_execute("exit");
         break;
    default:
         vtysh_execute("end");
         vtysh_execute("configure terminal");
         break;
    }
    sal_memset(ifcmd, 0, sizeof(ifcmd));
    sal_sprintf(ifcmd, "interface %s", ifname);
    rc = vtysh_execute(ifcmd);
    if (rc)
    {
        return rc;
    }
    rc = vtysh_execute(cmd);
    vtysh_execute("end");
    p_glb->vty_mode = GLB_VTY_MODE_EXEC;

    if (0 != rc)
    {
        log_sys(M_MOD_ROUTE, E_ERROR, "vtyclient if %s cmd %s failed", ifname, cmd);
        g_vty_fail_count++;
    }
    return rc;
}

int32
rt_vty_router_ospf_cmd(char *cmd)
{
    char rtcmd[CMD_BUF_64];
    tbl_route_global_t *p_glb = tbl_route_global_get_route_global();
    int rc = 0;

    if (!p_glb->vty_connected)
    {
        rt_vty_init();
        if (!p_glb->vty_connected)
        {
            p_glb->vty_nonconn_count++;
            return PM_E_SOCK;
        }
    }

    switch (p_glb->vty_mode)
    {
    case GLB_VTY_MODE_EXEC:
         vtysh_execute("configure terminal");
         break;
    case GLB_VTY_MODE_CONFIG:
         break;
    case GLB_VTY_MODE_INTERFACE:
    case GLB_VTY_MODE_ROUTER:
         vtysh_execute("exit");
         break;
    default:
         vtysh_execute("end");
         vtysh_execute("configure terminal");
         break;
    }
    sal_memset(rtcmd, 0, sizeof(rtcmd));
    sal_sprintf(rtcmd, "router ospf");
    rc = vtysh_execute(rtcmd);
    if (rc)
    {
        return rc;
    }
    rc = vtysh_execute(cmd);
    vtysh_execute("end");
    p_glb->vty_mode = GLB_VTY_MODE_EXEC;

    /* modified by liwh for bug 49192, 2018-09-21 
         for warning(CMD_WARNING 1), shouldn't print error for ospf cli */
    if ((0 != rc) && (1 != rc))
    /* liwh end */
    {
        log_sys(M_MOD_ROUTE, E_ERROR, "vtyclient route-ospf cmd %s failed", cmd);
        g_vty_fail_count++;
    }
    return rc;
}

int32
rt_vty_config_cmd(char *cmd)
{
    tbl_route_global_t *p_glb = tbl_route_global_get_route_global();
    int rc = 0;

    if (!p_glb->vty_connected)
    {
        rt_vty_init();
        if (!p_glb->vty_connected)
        {
            p_glb->vty_nonconn_count++;
            return PM_E_SOCK;
        }
    }

    switch (p_glb->vty_mode)
    {
    case GLB_VTY_MODE_EXEC:
         vtysh_execute("configure terminal");
         break;
    case GLB_VTY_MODE_CONFIG:
         break;
    case GLB_VTY_MODE_INTERFACE:
    case GLB_VTY_MODE_ROUTER:
         vtysh_execute("exit");
         break;
    default:
         vtysh_execute("end");
         vtysh_execute("configure terminal");
         break;
    }
    
    rc = vtysh_execute(cmd);
    vtysh_execute("end");
    p_glb->vty_mode = GLB_VTY_MODE_EXEC;

    if (0 != rc)
    {
        log_sys(M_MOD_ROUTE, E_ERROR, "vtyclient config cmd %s failed", cmd);
        g_vty_fail_count++;
    }
    return rc;
}

int32
rt_vty_exec_cmd(char *cmd)
{
    tbl_route_global_t *p_glb = tbl_route_global_get_route_global();
    int rc = 0;

    if (!p_glb->vty_connected)
    {
        rt_vty_init();
        if (!p_glb->vty_connected)
        {
            p_glb->vty_nonconn_count++;
            return PM_E_SOCK;
        }
    }

    switch (p_glb->vty_mode)
    {
    case GLB_VTY_MODE_EXEC:
        break;
    case GLB_VTY_MODE_CONFIG:
    case GLB_VTY_MODE_INTERFACE:
    case GLB_VTY_MODE_ROUTER:
    default:
        vtysh_execute("end");
        break;
    }
    
    rc = vtysh_execute(cmd);
    p_glb->vty_mode = GLB_VTY_MODE_EXEC;
    
    if (0 != rc)
    {
        log_sys(M_MOD_ROUTE, E_ERROR, "vtyclient exec cmd %s failed", cmd);
        g_vty_fail_count++;
    }
    return rc;
}

int32
rt_vty_init()
{
    tbl_route_global_t *p_glb = tbl_route_global_get_route_global();
    int rc = 0;

    if (p_glb->vty_connected)
    {
        return 0;
    }

    rc = vtyclient_init();
    if (rc < 0)
    {
        log_sys(M_MOD_ROUTE, E_ERROR, "Init FRR VTY client failed");
    }
    else
    {
        p_glb->vty_connected = 1;
        tbl_route_global_set_route_global_field(p_glb, TBL_ROUTE_GLOBAL_FLD_VTY_CONNECTED);
    }

/*SYSTEM MODIFIED by chenc for routed connect frr failed*/
#if defined(_GLB_UML_SYSTEM_) && defined(TSINGMA)
    rc = vtyclient_init();
    if (rc < 0)
    {
        log_sys(M_MOD_ROUTE, E_ERROR, "Init FRR VTY client failed");
    }
    else
    {
        p_glb->vty_connected = 1;
        tbl_route_global_set_route_global_field(p_glb, TBL_ROUTE_GLOBAL_FLD_VTY_CONNECTED);
    }
#endif
    
    return rc;
}
