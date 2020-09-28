/**
 @file glb_chip_specific.h
 
 @date 2015-07-01
 
 @version v1.0
 
The file define chip specific macros.

*/

#ifndef __GLB_CHIP_SPECIFIC_H__
#define __GLB_CHIP_SPECIFIC_H__

#define GLB_CONSOLE_RATE_CTC            9600
#define GLB_CONSOLE_RATE_ONIE           115200

#ifdef USW
/* Modifed by weizj for bug 36460 */
#define GLB_PORT_NUM_PER_CHIP           512 /* modified by xgu for gg system from 64 to 128, 2014-5-14*//*Should be same as CTC_MAX_PHY_PORT*/
#define GLB_CPU_LOCAL_PHYPORT           50  /* [GG]50*/
#define GLB_MAX_DLB_MEMBER_NUM          16
#ifdef TAPPRODUCT
#define GLB_LINKAGG_MAX_GROUP           16   /*[GB], 128 for humber sdk, and 256 for bay sdk*/
#define GLB_LINKAGG_MAX_MEMBER          64   /*get this from sdk?? humber supports 16, but bay only 8 */
#else
#define GLB_LINKAGG_MAX_GROUP           55   /*[GB], 128 for humber sdk, and 256 for bay sdk*/
#define GLB_LINKAGG_MAX_MEMBER          16   /*get this from sdk?? humber supports 16, but bay only 8 */
#endif

/* internal port is allocated by sdk, modified by liwh for bug 22533, 2013-03-18 */
#define GLB_INTERNAL_PORT_START         128
/* for bug 32547 modified for gg by liuyang 2015-6-8*/
#define GLB_INTERNAL_PORT_END           512

/* According to SDK's define
#define CTC_LPORT_CPU                   50
#define CTC_LPORT_ETH_CPU               49
*/
/*add by xgu for gg's cpu-traffic*/
#define GLB_SYSTEM_CPU_PORT             50
#define GLB_SYSTEM_ETH_CPU_PORT         49
#define GLB_SYSTEM_DMA_PORT             GLB_SYSTEM_ETH_CPU_PORT
#define GLB_RESERVED_PORT_FOR_EILOOP    56

#define GLB_DROP_PORT                   66 /*huangxt for merge sdk 2.3.5. change from 62 -> 66
                                             zhangdy for merge sdk v3.1.3. change to 48 SYS_RSV_PORT_DROP_ID. */

#define GLB_QOS_MAX_PORT_QUEUE_NUM      8   /*modified by hansf, duet2 use 8 queue mode 2018/09/20*/
#define GLB_QOS_USED_PORT_QUEUE_NUM     GLB_QOS_MAX_PORT_QUEUE_NUM

#ifdef TAPPRODUCT
#define GLB_LAG_GROUP_NUM               16
#else
#define GLB_LAG_GROUP_NUM               55
#endif

#define GLB_CONSOLE_RATE_DEFAULT        GLB_CONSOLE_RATE_ONIE

#elif GOLDENGATE
/* for GoldenGate / GG */
/* Modifed by weizj for bug 36460 */
#define GLB_PORT_NUM_PER_CHIP           512 /* modified by xgu for gg system from 64 to 128, 2014-5-14*//*Should be same as CTC_MAX_PHY_PORT*/
#define GLB_CPU_LOCAL_PHYPORT           50  /* [GG]50*/
#define GLB_MAX_DLB_MEMBER_NUM          16
#ifdef TAPPRODUCT
#define GLB_LINKAGG_MAX_GROUP           16   /*[GB], 128 for humber sdk, and 256 for bay sdk*/
#define GLB_LINKAGG_MAX_MEMBER          64   /*get this from sdk?? humber supports 16, but bay only 8 */
#else
#define GLB_LINKAGG_MAX_GROUP           55   /*[GB], 128 for humber sdk, and 256 for bay sdk*/
#define GLB_LINKAGG_MAX_MEMBER          16   /*get this from sdk?? humber supports 16, but bay only 8 */
#endif

/* internal port is allocated by sdk, modified by liwh for bug 22533, 2013-03-18 */
#define GLB_INTERNAL_PORT_START         128
/* for bug 32547 modified for gg by liuyang 2015-6-8*/
#define GLB_INTERNAL_PORT_END           512

/* According to SDK's define
#define CTC_LPORT_CPU                   50
#define CTC_LPORT_ETH_CPU               49
*/
/*add by xgu for gg's cpu-traffic*/
#define GLB_SYSTEM_CPU_PORT             50
#define GLB_SYSTEM_ETH_CPU_PORT         49
#define GLB_SYSTEM_DMA_PORT             GLB_SYSTEM_ETH_CPU_PORT
#define GLB_RESERVED_PORT_FOR_EILOOP    56

#define GLB_DROP_PORT                   66 /*huangxt for merge sdk 2.3.5. change from 62 -> 66
                                             zhangdy for merge sdk v3.1.3. change to 48 SYS_RSV_PORT_DROP_ID. */

#define GLB_QOS_MAX_PORT_QUEUE_NUM      8 /*modified by hansf,16 queue-> 8 queue 2017/09/14*/
/*modified by hansf 2017/09/14*/
//#define GLB_QOS_USED_PORT_QUEUE_NUM     (GLB_QOS_MAX_PORT_QUEUE_NUM-3)
#define GLB_QOS_USED_PORT_QUEUE_NUM    GLB_QOS_MAX_PORT_QUEUE_NUM

#ifdef TAPPRODUCT
#define GLB_LAG_GROUP_NUM               16
#else
#define GLB_LAG_GROUP_NUM               55
#endif

#define GLB_CONSOLE_RATE_DEFAULT        GLB_CONSOLE_RATE_ONIE

#else
/* for GreatBelt / GB */

#define GLB_PORT_NUM_PER_CHIP           64
#define GLB_CPU_LOCAL_PHYPORT           62  /* [GB], for humber, this number should be 53 */
#define GLB_MAX_DLB_MEMBER_NUM          8
#define GLB_LINKAGG_MAX_GROUP           32  /*[GB], 128 for humber sdk, and 256 for bay sdk*/
#define GLB_LINKAGG_MAX_MEMBER          16  /*get this from sdk?? humber supports 16, but bay only 8 */

/* internal port is allocated by sdk, modified by liwh for bug 22533, 2013-03-18 */
#define GLB_INTERNAL_PORT_START         64
#define GLB_INTERNAL_PORT_END           128

#define GLB_RESERVED_PORT_FOR_EILOOP    61
#define GLB_SYSTEM_CPU_PORT             62
#define GLB_SYSTEM_DMA_PORT             63

#define GLB_QOS_MAX_PORT_QUEUE_NUM      8
#define GLB_QOS_USED_PORT_QUEUE_NUM     GLB_QOS_MAX_PORT_QUEUE_NUM

#define GLB_LAG_GROUP_NUM               31

#define GLB_CONSOLE_RATE_DEFAULT        GLB_CONSOLE_RATE_CTC

#endif

typedef enum
{
    GLB_CHIP_INVALID    = 0,
    GLB_CHIP_GREATEBELT = 1,
    GLB_CHIP_GOLDENGATE = 2,
    GLB_CHIP_DUET2      = 3,
    GLB_CHIP_TSINGMA    = 4,
    GLB_CHIP_TYPE_MAX   = 5
} glb_chip_type_t;

#endif /* __GLB_CHIP_SPECIFIC_H__*/

