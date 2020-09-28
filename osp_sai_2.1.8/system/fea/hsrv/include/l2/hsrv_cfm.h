#ifndef __HSRV_CFM_H__
#define __HSRV_CFM_H__

struct hsrv_cfm_s
{
    ctclib_opf_t         *pst_mep_opf;                        /* OFP of MEP, allocate mep index */
    ctc_task_t           *pst_cfm_timer;
};
typedef struct hsrv_cfm_s hsrv_cfm_t;

int32
hsrv_cfm_start();

#define HSRV_CFM_MP_ADDR_INDIVIDUAL      0     /* Individual MP addressing mode */
#define HSRV_CFM_MP_ADDR_SHARED             1     /* Shared MP addressing mode */

#endif /* !__HSRV_CFM_H__ */


