#if (FEATURE_MODE == 0)
/**
 @file sys_usw_ipfix.c

 @author  Copyright (C) 2013 Centec Networks Inc.  All rights reserved.

 @date 2013-10-17

 @version v3.0
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_const.h"
#include "ctc_ipfix.h"
#include "ctc_interrupt.h"
#include "ctc_warmboot.h"
#include "ctc_spool.h"

#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_dma.h"
#include "sys_usw_ipfix.h"
#include "sys_usw_ftm.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"
#include "sys_usw_interrupt.h"
#include "usw/include/drv_common.h"

#include "drv_api.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define IPFIX_TRAVERSE_TYPE_PROCESS(key_index, key_type, is_remove,dir)\
    do{\
	            key_index = DRV_IS_DUET2(lchip)?key_index:key_index/2;\
	            key_index = SYS_IPFIX_GET_ADINDEX(key_index, dir);\
                if(is_remove)\
                {\
                    ret  = ((ctc_ipfix_traverse_remove_cmp)fn)(&ipfix_data, p_data->user_data);\
                    if(ret == CTC_E_NONE)\
                    {\
                        CTC_ERROR_GOTO(_sys_usw_ipfix_delete_entry_by_index(lchip, key_type, key_index,dir, 1), ret, end_pro);\
                    }\
                }\
                else if(traverse_action == SYS_IPFIX_DUMP_ENYRY_INFO)\
                    {\
                        ((ctc_ipfix_traverse_fn)fn)(&ipfix_data, &key_index);\
                    }\
                    else\
                    {\
                        ((ctc_ipfix_traverse_fn)fn)(&ipfix_data, p_data->user_data);\
                    }\
    }while(0)

#define SYS_IPFIX_MAX_SAMPLE_PROFILE 4

struct sys_ipfix_sample_profile_s
{
    uint16  interval;
    uint8   dir;
    uint8   mode;      /*bit0 sample mode, bit1 sample_type, refer to ctc_ipfix_flow_cfg_t*/

    uint16  ad_index;
};
typedef struct sys_ipfix_sample_profile_s sys_ipfix_sample_profile_t;

struct sys_ipfix_master_s
{
    ctc_ipfix_fn_t ipfix_cb;
    uint32         max_ptr;
    uint32         aging_interval;
    uint32         exp_cnt_stats[8];     /* statistics based export reason */

    uint64         exp_pkt_cnt_stats[8]; /* 0-Expired, 1-Tcp Close, 2-PktCnt Overflow, 3-ByteCnt Overflow */
    uint64         exp_pkt_byte_stats[8];/* 4-Ts Overflow, 5-Conflict, 6-New Flow, 7-Dest Change */
    void*          user_data;
    ctc_spool_t*   sample_spool;            /* data is sys_ipfix_sample_profile_t*/
    uint32         sip_idx_bmp;
    uint32         sip_interval[2*SYS_IPFIX_MAX_SAMPLE_PROFILE];
    sal_mutex_t*   p_mutex;

};
typedef struct sys_ipfix_master_s sys_ipfix_master_t;

#define SYS_IPFIX_MERGE_TYPE_VXLAN  1
#define SYS_IPFIX_MERGE_TYPE_GRE    2
#define SYS_IPFIX_MERGE_TYPE_WLAN 3


sys_ipfix_master_t  *p_usw_ipfix_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define SYS_IPFIX_INIT_CHECK(lchip) \
    do { \
        LCHIP_CHECK(lchip);\
        if (NULL == p_usw_ipfix_master[lchip]){ \
            SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    } while (0)

#define SYS_IPFIX_LOCK \
    do                                       \
    {                                          \
        if(p_usw_ipfix_master[lchip]->p_mutex != NULL)\
        {                                            \
            sal_mutex_lock(p_usw_ipfix_master[lchip]->p_mutex);  \
        }                                                 \
    }while(0)

#define SYS_IPFIX_UNLOCK \
    do               \
    {                                         \
        if(p_usw_ipfix_master[lchip]->p_mutex != NULL)\
        {                                          \
            sal_mutex_unlock(p_usw_ipfix_master[lchip]->p_mutex); \
        }                                                   \
    }while(0)

#define SYS_IPFIX_DBG_OUT(level,FMT, ...) \
    do{\
        CTC_DEBUG_OUT(ipfix, ipfix, IPFIX_SYS, level, FMT, ##__VA_ARGS__); \
    }while(0)
#define SYS_IPFIX_GET_TAB_BY_TYPE(key_type) \
((key_type==CTC_IPFIX_KEY_HASH_MAC)?DsIpfixL2HashKey_t:\
            ((key_type==CTC_IPFIX_KEY_HASH_IPV4)?DsIpfixL3Ipv4HashKey_t:\
            ((key_type==CTC_IPFIX_KEY_HASH_MPLS)?DsIpfixL3MplsHashKey_t:\
            ((key_type==CTC_IPFIX_KEY_HASH_IPV6)?DsIpfixL3Ipv6HashKey_t : DsIpfixL2L3HashKey_t))))

#define SYS_IPFIX_MAP_IP_FRAG(ctc_ip_frag) \
    (((ctc_ip_frag) == CTC_IP_FRAG_NOT_FIRST)?3:(((ctc_ip_frag) == CTC_IP_FRAG_SMALL)?2:(((ctc_ip_frag) == CTC_IP_FRAG_FIRST)?1:0)))
#define SYS_IPFIX_UNMAP_IP_FRAG(drv_ip_frag) \
    (((drv_ip_frag) == 3)? CTC_IP_FRAG_NOT_FIRST:(((drv_ip_frag) == 2)? CTC_IP_FRAG_SMALL:(((drv_ip_frag) == 1)? CTC_IP_FRAG_FIRST: CTC_IP_FRAG_NON)))
#define SYS_IPFIX_AD_INDEX(key_type, key_index) ()

#define SYS_IPFIX_OP_DIR(dir) ((1 == MCHIP_CAP(SYS_CAP_IPFIX_MEMORY_SHARE))?2:dir)
#define SYS_IPFIX_GET_TABLE(tbl_id, dir) _sys_usw_ipfix_get_tbl_id_by_dir(lchip, tbl_id, dir)
#define SYS_IPFIX_GET_INDEX(index, dir) ((1 == MCHIP_CAP(SYS_CAP_IPFIX_MEMORY_SHARE))? index: ((dir == CTC_INGRESS)?(index):(index - 4096)))
#define SYS_IPFIX_GET_ADINDEX(index, dir) ((1 == MCHIP_CAP(SYS_CAP_IPFIX_MEMORY_SHARE))? index: ((dir == CTC_INGRESS)?(index):(index - 8192)))

STATIC int32
_sys_usw_ipfix_get_entry_by_key(uint8 lchip, ctc_ipfix_data_t* p_key, uint32* rst_hit, uint32* rst_key_index);
extern void
_sys_usw_ipfix_process_isr(ctc_ipfix_data_t* info, void* userdata);
extern int32
sys_usw_ipfix_ftm_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info);
STATIC int32
sys_usw_ipfix_wb_sync(uint8 lchip);
STATIC int32
sys_usw_ipfix_wb_restore(uint8 lchip);

/* Do Not change the following data anyway*/
#define SYS_USW_PROFILE0_SEED 4094
#define SYS_USW_PROFILE2_SEED 4095
#define SYS_USW_PROFILE3_SEED 4094

#define SYS_USW_PROFILE0_NUM_CNT 465
#define SYS_USW_PROFILE2_NUM_CNT 511
#define SYS_USW_PROFILE3_NUM_CNT 465

static uint16 sys_usw_ipfix_random_data[4][512] = {
    {
        12 ,19 ,24 ,27 ,38 ,49 ,54 ,70 ,73 ,77 ,78 ,99 ,103 ,108 ,141 ,146 ,153 ,154 ,157 ,183 ,184 ,199 ,204 ,207 ,216 ,226 ,
        229 ,242 ,273 ,282 ,286 ,292 ,304 ,307 ,308 ,315 ,331 ,332 ,347 ,357 ,366 ,369 ,399 ,408 ,415 ,421 ,433 ,434 ,442 ,
        453 ,458 ,462 ,480 ,484 ,487 ,495 ,496 ,521 ,537 ,542 ,547 ,564 ,567 ,572 ,579 ,584 ,588 ,609 ,610 ,614 ,617 ,630 ,
        663 ,664 ,668 ,674 ,690 ,693 ,694 ,701 ,714 ,733 ,736 ,739 ,744 ,759 ,779 ,788 ,795 ,796 ,799 ,817 ,821 ,830 ,841 ,
        842 ,862 ,867 ,868 ,884 ,906 ,917 ,925 ,926 ,928 ,944 ,951 ,960 ,968 ,971 ,975 ,991 ,993 ,997 ,1002 ,1030 ,1041 ,
        1042 ,1049 ,1075 ,1080 ,1084 ,1092 ,1095 ,1100 ,1107 ,1129 ,1134 ,1145 ,1159 ,1168 ,1171 ,1176 ,1197 ,1210 ,1213 ,
        1218 ,1221 ,1222 ,1229 ,1234 ,1255 ,1256 ,1260 ,1284 ,1299 ,1300 ,1326 ,1329 ,1337 ,1338 ,1345 ,1349 ,1358 ,1380 ,
        1387 ,1388 ,1391 ,1403 ,1425 ,1429 ,1455 ,1456 ,1464 ,1467 ,1471 ,1472 ,1479 ,1488 ,1509 ,1517 ,1518 ,1530 ,1555 ,
        1559 ,1564 ,1577 ,1590 ,1593 ,1597 ,1598 ,1601 ,1606 ,1622 ,1635 ,1643 ,1660 ,1682 ,1685 ,1704 ,1719 ,1724 ,1727 ,
        1731 ,1735 ,1736 ,1762 ,1769 ,1770 ,1773 ,1789 ,1793 ,1813 ,1814 ,1822 ,1835 ,1851 ,1852 ,1856 ,1859 ,1860 ,1867 ,
        1876 ,1889 ,1898 ,1902 ,1920 ,1936 ,1940 ,1943 ,1951 ,1973 ,1978 ,1982 ,1985 ,1986 ,1994 ,2005 ,2024 ,2031 ,2047 ,
        2054 ,2057 ,2061 ,2083 ,2084 ,2087 ,2099 ,2124 ,2139 ,2140 ,2150 ,2161 ,2162 ,2169 ,2184 ,2191 ,2200 ,2213 ,2214 ,
        2221 ,2226 ,2258 ,2265 ,2269 ,2279 ,2288 ,2291 ,2295 ,2296 ,2308 ,2316 ,2319 ,2331 ,2337 ,2342 ,2353 ,2382 ,2385 ,
        2393 ,2394 ,2398 ,2416 ,2420 ,2427 ,2437 ,2442 ,2445 ,2446 ,2458 ,2468 ,2479 ,2511 ,2512 ,2520 ,2523 ,2533 ,2546 ,
        2549 ,2563 ,2568 ,2572 ,2588 ,2594 ,2598 ,2601 ,2633 ,2646 ,2653 ,2654 ,2659 ,2675 ,2676 ,2690 ,2697 ,2698 ,2717 ,
        2720 ,2727 ,2743 ,2760 ,2775 ,2776 ,2780 ,2783 ,2802 ,2806 ,2813 ,2825 ,2830 ,2846 ,2848 ,2851 ,2859 ,2900 ,2907 ,
        2911 ,2913 ,2929 ,2933 ,2934 ,2942 ,2944 ,2955 ,2959 ,2977 ,2978 ,2981 ,2986 ,2997 ,3018 ,3034 ,3037 ,3040 ,3060 ,
        3063 ,3071 ,3075 ,3076 ,3091 ,3110 ,3117 ,3118 ,3129 ,3154 ,3158 ,3161 ,3180 ,3187 ,3192 ,3195 ,3196 ,3202 ,3206 ,
        3213 ,3239 ,3240 ,3244 ,3247 ,3256 ,3271 ,3287 ,3309 ,3314 ,3321 ,3322 ,3329 ,3334 ,3342 ,3345 ,3364 ,3371 ,3375 ,
        3396 ,3408 ,3411 ,3419 ,3438 ,3449 ,3454 ,3460 ,3463 ,3471 ,3472 ,3498 ,3501 ,3514 ,3525 ,3537 ,3538 ,3541 ,3546 ,
        3568 ,3579 ,3583 ,3585 ,3586 ,3593 ,3606 ,3627 ,3628 ,3644 ,3651 ,3667 ,3668 ,3671 ,3676 ,3702 ,3705 ,3709 ,3712 ,
        3715 ,3719 ,3720 ,3735 ,3746 ,3753 ,3757 ,3778 ,3797 ,3798 ,3805 ,3816 ,3832 ,3839 ,3840 ,3844 ,3851 ,3873 ,3881 ,
        3882 ,3886 ,3902 ,3905 ,3921 ,3926 ,3947 ,3956 ,3964 ,3967 ,3970 ,3973 ,3989 ,4000 ,4008 ,4011 ,4031 ,4048 ,4052 ,
        4063 ,4074 ,4085 ,4090 ,4093 ,4094
    },
    {
        /*zero, just for code simple*/
    },
    {
        1, 2, 3, 4, 6, 7, 9, 11, 13, 14, 19, 23, 26, 28, 29, 37, 39, 46, 52, 57, 58, 74, 78, 93, 105, 111, 115, 116, 148, 157,
        177, 186, 197, 211, 222, 223, 231, 233, 235, 296, 314, 315, 317, 333, 334, 352, 354, 355, 359, 372, 373, 395, 422,
        444, 447, 463, 464, 466, 467, 470, 477, 484, 503, 513, 534, 536, 556, 592, 614, 616, 625, 628, 629, 631, 634, 646,
        655, 666, 668, 669, 704, 708, 709, 711, 718, 722, 745, 746, 790, 797, 819, 823, 832, 839, 844, 846, 884, 888, 889,
        895, 896, 905, 915, 925, 926, 927, 928, 929, 931, 932, 933, 934, 937, 940, 942, 944, 948, 955, 957, 968, 974, 979,
        980, 983, 1001, 1005, 1007, 1024, 1026, 1033, 1068, 1073, 1079, 1082, 1107, 1112, 1140, 1145, 1149, 1152, 1177, 1178,
        1181, 1185, 1187, 1191, 1204, 1223, 1228, 1232, 1233, 1235, 1245, 1248, 1250, 1252, 1255, 1256, 1258, 1259, 1261,
        1263, 1267, 1268, 1269, 1274, 1293, 1310, 1318, 1320, 1332, 1337, 1338, 1339, 1365, 1366, 1368, 1378, 1408, 1417,
        1418, 1419, 1422, 1436, 1437, 1444, 1473, 1480, 1490, 1491, 1492, 1536, 1539, 1575, 1577, 1580, 1582, 1594, 1595,
        1638, 1647, 1651, 1652, 1665, 1679, 1683, 1688, 1692, 1694, 1706, 1721, 1768, 1777, 1778, 1791, 1792, 1798, 1811,
        1825, 1831, 1850, 1853, 1854, 1856, 1859, 1861, 1863, 1864, 1865, 1866, 1869, 1871, 1874, 1875, 1881, 1885, 1888,
        1897, 1899, 1910, 1911, 1914, 1931, 1936, 1937, 1942, 1949, 1957, 1959, 1961, 1966, 2003, 2010, 2014, 2047, 2048,
        2049, 2051, 2053, 2062, 2066, 2103, 2136, 2146, 2159, 2165, 2205, 2206, 2214, 2215, 2224, 2225, 2227, 2234, 2280,
        2281, 2286, 2290, 2299, 2304, 2315, 2316, 2326, 2355, 2356, 2360, 2362, 2371, 2375, 2382, 2402, 2409, 2446, 2457,
        2459, 2464, 2467, 2471, 2490, 2492, 2496, 2500, 2505, 2510, 2511, 2512, 2513, 2514, 2516, 2519, 2520, 2522, 2526,
        2535, 2537, 2538, 2539, 2548, 2550, 2560, 2564, 2587, 2589, 2601, 2618, 2620, 2622, 2624, 2636, 2637, 2638, 2641,
        2643, 2650, 2659, 2664, 2665, 2670, 2672, 2674, 2675, 2677, 2678, 2681, 2682, 2685, 2707, 2708, 2717, 2730, 2731,
        2732, 2737, 2757, 2766, 2784, 2788, 2793, 2816, 2817, 2835, 2836, 2839, 2845, 2873, 2874, 2889, 2895, 2901, 2908,
        2947, 2960, 2978, 2980, 2983, 2985, 2997, 3003, 3013, 3016, 3019, 3026, 3071, 3072, 3073, 3079, 3099, 3150, 3151,
        3155, 3160, 3161, 3165, 3188, 3191, 3205, 3206, 3211, 3228, 3249, 3277, 3294, 3303, 3304, 3305, 3308, 3317, 3323,
        3328, 3330, 3342, 3348, 3357, 3359, 3360, 3366, 3367, 3369, 3373, 3377, 3380, 3383, 3384, 3385, 3388, 3389, 3390,
        3401, 3402, 3406, 3413, 3431, 3440, 3442, 3444, 3456, 3495, 3498, 3502, 3537, 3546, 3549, 3554, 3556, 3557, 3561,
        3583, 3584, 3597, 3623, 3628, 3650, 3651, 3653, 3662, 3672, 3700, 3702, 3706, 3709, 3712, 3719, 3722, 3726, 3728,
        3731, 3732, 3734, 3738, 3739, 3740, 3742, 3743, 3748, 3749, 3751, 3763, 3768, 3770, 3776, 3795, 3797, 3799, 3821,
        3822, 3826, 3828, 3862, 3873, 3874, 3884, 3899, 3915, 3917, 3918, 3919, 3922, 3932, 3946, 3961, 4006, 4007, 4021,
        4028, 4051, 4073, 4084, 4090, 4093, 4094, 4095
    },
    {
        14, 29, 35, 37, 59, 67, 69, 70, 75, 85, 94, 99, 117, 118, 130, 135, 138, 140, 143, 151, 170, 188, 191, 199, 212, 234,
        236, 242, 260, 271, 273, 276, 281, 284, 287, 300, 303, 327, 340, 353, 377, 383, 398, 413, 424, 432, 438, 454, 461, 464,
        469, 470, 472, 477, 485, 486, 496, 520, 523, 542, 547, 552, 563, 566, 568, 571, 574, 582, 600, 606, 608, 655, 657, 663,
        681, 698, 705, 706, 727, 737, 746, 753, 754, 759, 762, 767, 770, 796, 809, 826, 836, 849, 850, 865, 866, 868, 873, 876,
        882, 889, 909, 923, 928, 933, 936, 939, 941, 944, 955, 971, 973, 981, 992, 1011, 1033, 1041, 1047, 1071, 1084, 1092,
        1095, 1105, 1127, 1132, 1137, 1140, 1143, 1145, 1148, 1165, 1166, 1176, 1189, 1198, 1200, 1205, 1208, 1213, 1214, 1216,
        1240, 1246, 1269, 1291, 1309, 1310, 1315, 1318, 1323, 1325, 1326, 1334, 1341, 1355, 1357, 1363, 1382, 1397, 1410, 1412,
        1434, 1455, 1468, 1474, 1492, 1495, 1506, 1508, 1511, 1514, 1519, 1524, 1535, 1537, 1540, 1546, 1548, 1553, 1562, 1580,
        1593, 1594, 1601, 1618, 1642, 1644, 1652, 1672, 1691, 1699, 1701, 1725, 1728, 1731, 1733, 1736, 1741, 1747, 1752, 1765,
        1776, 1779, 1800, 1819, 1838, 1846, 1856, 1867, 1872, 1875, 1878, 1883, 1886, 1888, 1891, 1910, 1922, 1929, 1938, 1943,
        1945, 1946, 1951, 1961, 1962, 1983, 1985, 2002, 2023, 2041, 2047, 2055, 2065, 2066, 2081, 2082, 2090, 2095, 2097, 2106,
        2113, 2119, 2143, 2154, 2169, 2184, 2190, 2198, 2211, 2224, 2254, 2264, 2267, 2275, 2278, 2280, 2283, 2286, 2291, 2296,
        2309, 2331, 2333, 2339, 2352, 2376, 2379, 2397, 2400, 2411, 2416, 2421, 2424, 2427, 2429, 2433, 2452, 2466, 2473, 2481,
        2482, 2484, 2489, 2492, 2514, 2516, 2538, 2553, 2564, 2583, 2594, 2618, 2620, 2631, 2636, 2642, 2647, 2650, 2652, 2655,
        2668, 2671, 2682, 2693, 2702, 2707, 2709, 2710, 2715, 2718, 2725, 2726, 2739, 2765, 2782, 2795, 2803, 2805, 2816, 2821,
        2822, 2824, 2829, 2838, 2845, 2848, 2869, 2870, 2893, 2910, 2912, 2918, 2936, 2948, 2967, 2985, 2991, 2993, 3009, 3012,
        3017, 3020, 3023, 3028, 3039, 3049, 3068, 3071, 3075, 3080, 3088, 3093, 3096, 3101, 3104, 3107, 3125, 3147, 3160, 3181,
        3187, 3189, 3202, 3217, 3236, 3258, 3260, 3274, 3281, 3284, 3289, 3290, 3292, 3305, 3306, 3324, 3330, 3345, 3369, 3375,
        3383, 3394, 3399, 3401, 3402, 3407, 3410, 3417, 3439, 3449, 3450, 3456, 3459, 3462, 3467, 3470, 3472, 3483, 3494, 3504,
        3507, 3531, 3544, 3552, 3558, 3582, 3592, 3598, 3600, 3621, 3638, 3656, 3677, 3678, 3685, 3688, 3693, 3694, 3701, 3710,
        3713, 3732, 3735, 3745, 3748, 3751, 3753, 3756, 3767, 3772, 3777, 3783, 3801, 3820, 3839, 3844, 3847, 3858, 3876, 3887,
        3890, 3892, 3895, 3898, 3903, 3914, 3922, 3924, 3948, 3967, 3971, 3995, 3997, 4005, 4022, 4045, 4046, 4059, 4070, 4077,
        4083, 4086, 4091, 4093, 4094
    }
};

/****************************************************************************
 *
* Function
*
*****************************************************************************/

STATIC uint32
_sys_usw_ipfix_get_tbl_id_by_dir(uint8 lchip, uint32 tbl_id, uint8 dir)
{
    uint32 out_tbl_id = 0;

    if (DRV_IS_DUET2(lchip))
    {
        return tbl_id;
    }

    switch(tbl_id)
    {
    case DsIpfixL2HashKey_t:
        out_tbl_id = (dir == CTC_INGRESS)?DsIpfixL2HashKey0_t:DsIpfixL2HashKey1_t;
        break;
    case DsIpfixL2L3HashKey_t:
        out_tbl_id = (dir == CTC_INGRESS)?DsIpfixL2L3HashKey0_t:DsIpfixL2L3HashKey1_t;
        break;
    case DsIpfixL3Ipv4HashKey_t:
        out_tbl_id = (dir == CTC_INGRESS)?DsIpfixL3Ipv4HashKey0_t:DsIpfixL3Ipv4HashKey1_t;
        break;
    case DsIpfixL3Ipv6HashKey_t:
        out_tbl_id = (dir == CTC_INGRESS)?DsIpfixL3Ipv6HashKey0_t:DsIpfixL3Ipv6HashKey1_t;
        break;
    case DsIpfixL3MplsHashKey_t:
        out_tbl_id = (dir == CTC_INGRESS)?DsIpfixL3MplsHashKey0_t:DsIpfixL3MplsHashKey1_t;
        break;
    case IpfixL2HashFieldSelect_t:
        out_tbl_id = (dir == CTC_INGRESS)?IpfixL2HashFieldSelect0_t:IpfixL2HashFieldSelect1_t;
        break;
    case IpfixL2L3HashFieldSelect_t:
        out_tbl_id = (dir == CTC_INGRESS)?IpfixL2L3HashFieldSelect0_t:IpfixL2L3HashFieldSelect1_t;
        break;
    case IpfixL3Ipv4HashFieldSelect_t:
        out_tbl_id = (dir == CTC_INGRESS)?IpfixL3Ipv4HashFieldSelect0_t:IpfixL3Ipv4HashFieldSelect1_t;
        break;
    case IpfixL3Ipv6HashFieldSelect_t:
        out_tbl_id = (dir == CTC_INGRESS)?IpfixL3Ipv6HashFieldSelect0_t:IpfixL3Ipv6HashFieldSelect1_t;
        break;
    case IpfixL3MplsHashFieldSelect_t:
        out_tbl_id = (dir == CTC_INGRESS)?IpfixL3MplsHashFieldSelect0_t:IpfixL3MplsHashFieldSelect1_t;
        break;
    case DsIpfixSessionRecord_t:
        out_tbl_id = (dir == CTC_INGRESS)?DsIpfixSessionRecord0_t:DsIpfixSessionRecord1_t;
        break;
    default:
        out_tbl_id = tbl_id;
        break;

    }

    return out_tbl_id;
}

STATIC int32 _sys_usw_ipfix_sample_interval_convert(uint8 lchip, sys_ipfix_sample_profile_t* p_profile, uint16* drv_interval )
{
    if(p_profile->mode & 0x01)
    {
        if(DRV_IS_DUET2(lchip))
        {
            uint16 number_cnt = 0;
            if(p_profile->ad_index == 1 || p_profile->ad_index > 3)
            {
                SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ipfix random sample can not use profile id 1, line:%d\n", __LINE__);
                return CTC_E_INVALID_PARAM;
            }

            if(p_profile->ad_index == 0 )
            {
                number_cnt = SYS_USW_PROFILE0_NUM_CNT;
            }
            else if(p_profile->ad_index == 2)
            {
                number_cnt = SYS_USW_PROFILE2_NUM_CNT;
            }
            else
            {
                number_cnt = SYS_USW_PROFILE3_NUM_CNT;
            }
            *drv_interval = sys_usw_ipfix_random_data[p_profile->ad_index][number_cnt/p_profile->interval];
        }
        else
        {
            *drv_interval = (1 == p_profile->interval)?4096:(4095/p_profile->interval);
        }
    }
    else
    {
        *drv_interval = p_profile->interval - 1;
    }
    return CTC_E_NONE;
}

STATIC uint32 _sys_usw_ipfix_sample_spool_hask_make(sys_ipfix_sample_profile_t* p_data)
{
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    return ctc_hash_caculate(sizeof(sys_ipfix_sample_profile_t)-sizeof(p_data->ad_index), p_data);
}

STATIC bool _sys_usw_ipfix_sample_spool_hash_cmp(sys_ipfix_sample_profile_t* p_ad1, sys_ipfix_sample_profile_t* p_ad2)
{
    uint32 size;
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (!p_ad1 || !p_ad2)
    {
        return FALSE;
    }

    size = sizeof(sys_ipfix_sample_profile_t)-sizeof(p_ad1->ad_index);
    if (!sal_memcmp(p_ad1, p_ad2, size))
    {
        return TRUE;
    }

    return FALSE;
}

int32
_sys_usw_ipfix_sample_spool_alloc_index(sys_ipfix_sample_profile_t* p_ad, void* user_data)
{
    uint8 lchip = *(uint8*)user_data;
    uint16 ad_index = 0;
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        ad_index = (CTC_INGRESS == p_ad->dir) ? p_ad->ad_index : (p_ad->ad_index + MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE));
        CTC_BIT_SET(p_usw_ipfix_master[lchip]->sip_idx_bmp, ad_index);
    }
    else if(CTC_INGRESS == p_ad->dir)
    {
        for(ad_index = 0; ad_index < MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE); ad_index++)
        {
            if(!CTC_IS_BIT_SET(p_usw_ipfix_master[lchip]->sip_idx_bmp, ad_index) && (!(p_ad->mode&0x01) || ad_index != 1 ))
            {
                CTC_BIT_SET(p_usw_ipfix_master[lchip]->sip_idx_bmp, ad_index);
                p_ad->ad_index = ad_index;
                break;
            }

            if(ad_index == MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE))
            {
                SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Ingress sample no resource\n");
                return -1;
            }
        }
    }
    else
    {
        for(ad_index = MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE); ad_index < 2*MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE); ad_index++)
        {
            if(!CTC_IS_BIT_SET(p_usw_ipfix_master[lchip]->sip_idx_bmp, ad_index) && (!(p_ad->mode&0x01) || ad_index != 1+MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE) ))
            {
                CTC_BIT_SET(p_usw_ipfix_master[lchip]->sip_idx_bmp, ad_index);
                p_ad->ad_index = ad_index-MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE);
                break;
            }

            if(ad_index == 2*MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE))
            {
                SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Egress sample no resource\n");
                return -1;
            }
        }
    }

    return 0;
}

int32
_sys_usw_ipfix_sample_spool_free_index(sys_ipfix_sample_profile_t* p_ad, void* user_data)
{
    uint8 lchip = *(uint8*)user_data;
    uint16 ad_index;

    CTC_PTR_VALID_CHECK(p_ad);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if(CTC_INGRESS == p_ad->dir)
    {
        ad_index = p_ad->ad_index;
    }
    else
    {
        ad_index = p_ad->ad_index + MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE);
    }

    CTC_BIT_UNSET(p_usw_ipfix_master[lchip]->sip_idx_bmp, ad_index);
    return 0;
}

STATIC int32 _sys_usw_ipfix_init_register(uint8 lchip, void* p_global_cfg)
{
    uint32 cmd = 0;
    uint32 core_frequecy = 0;
    uint32 tick_interval = 0;
    uint32 max_phy_ptr = 0;
    uint32 byte_cnt[2] = {0};
    uint64 tmp = 0;
    uint32 value_a = 0;
    ctc_ipfix_global_cfg_t ipfix_cfg;
    uint32 ipfix_max_entry_num;

    IpfixHashLookupCtl_m ipfix_hash;
    IpeFwdFlowHashCtl_m  ipfix_flow_hash;
    EpeAclQosCtl_m       epe_acl_qos_ctl;
    IpfixAgingTimerCtl_m  aging_timer;
    IpfixEngineCtl_m      ipfix_ctl;
    IpePreLookupFlowCtl_m lookup_ctl;
    FlowAccLfsrCtl_m       flowacclfsrctl;

    sal_memset(&aging_timer, 0, sizeof(IpfixAgingTimerCtl_m));
    sal_memset(&ipfix_ctl, 0, sizeof(IpfixEngineCtl_m));

    sal_memset(&ipfix_hash, 0, sizeof(IpfixHashLookupCtl_m));
    sal_memset(&ipfix_flow_hash, 0, sizeof(ipfix_flow_hash));
    sal_memset(&lookup_ctl, 0, sizeof(IpePreLookupFlowCtl_m));

    sal_memset(&ipfix_cfg, 0, sizeof(ctc_ipfix_global_cfg_t));
    sal_memset(&flowacclfsrctl, 0, sizeof(flowacclfsrctl));

    /*p_global_cfg is NULL, using default config, else using user define */
    sal_memcpy(&ipfix_cfg, p_global_cfg, sizeof(ctc_ipfix_global_cfg_t));

    /*default aging interval : 10s*/
    core_frequecy = sys_usw_get_core_freq(lchip, 0);
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsIpfixL2HashKey_t, &ipfix_max_entry_num));
    tmp = (((uint64)core_frequecy * 1000 * 1000 ) / DOWN_FRE_RATE);
    tmp *= ipfix_cfg.aging_interval;
    if (DRV_IS_DUET2(lchip))
    {
        max_phy_ptr = ipfix_max_entry_num - 4;
        tick_interval =  tmp / (max_phy_ptr / 4);
		p_usw_ipfix_master[lchip]->max_ptr =  (tmp / tick_interval)*4;
    }
    else
    {
        max_phy_ptr = ipfix_max_entry_num / 2 - 1;
        tick_interval =  tmp / (max_phy_ptr);
		p_usw_ipfix_master[lchip]->max_ptr =  (tmp / tick_interval);
    }

    SetIpfixAgingTimerCtl(V, agingUpdEn_f, &aging_timer, 0);
    SetIpfixAgingTimerCtl(V, cpuAgingEn_f, &aging_timer, 0);
    SetIpfixAgingTimerCtl(V, agingInterval_f, &aging_timer, tick_interval);
    SetIpfixAgingTimerCtl(V, agingMaxPtr_f , &aging_timer, p_usw_ipfix_master[lchip]->max_ptr);
    SetIpfixAgingTimerCtl(V, agingMinPtr_f , &aging_timer, 0);
    SetIpfixAgingTimerCtl(V, agingPhyMaxPtr_f , &aging_timer, max_phy_ptr);
    cmd = DRV_IOW( IpfixAgingTimerCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aging_timer));
    cmd = DRV_IOW( IpfixAgingTimerCtl0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aging_timer));
    cmd = DRV_IOW( IpfixAgingTimerCtl1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aging_timer));


    SetIpfixEngineCtl(V, bypassUpdateOp_f, &ipfix_ctl, 0);

    byte_cnt[0] = (ipfix_cfg.bytes_cnt) & 0xffffffff;
    byte_cnt[1] = (ipfix_cfg.bytes_cnt >> 32) & 0x1f;
    SetIpfixEngineCtl(A, byteCntWraparoundThre_f, &ipfix_ctl, byte_cnt);

    if(ipfix_cfg.conflict_export)
    {
        value_a = 1;
        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_ipfixConflictPktLogEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_a));
        cmd = DRV_IOW(EpeHeaderEditCtl_t,EpeHeaderEditCtl_ipfixConflictPktLogEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_a));
    }
    ipfix_cfg.pkt_cnt = (ipfix_cfg.pkt_cnt == 0)?0:(ipfix_cfg.pkt_cnt - 1);
    SetIpfixEngineCtl(V, enableRecordMissPktCnt_f , &ipfix_ctl, 1);
    SetIpfixEngineCtl(V, isCpuSetFlowKeyMode_f , &ipfix_ctl, 0);
     /*SetIpfixEngineCtl(V, localPhyPortBase_f , &ipfix_ctl, 0x100);*/
    SetIpfixEngineCtl(V, ignorTcpClose_f , &ipfix_ctl, 0);   /*spec7.1bug*/
    SetIpfixEngineCtl(V, pktCntWraparoundThre_f , &ipfix_ctl, ipfix_cfg.pkt_cnt);
    SetIpfixEngineCtl(V, tsWraparoundThre_f , &ipfix_ctl, ipfix_cfg.times_interval);  /*1s*/
    SetIpfixEngineCtl(V, unknownPktDestIsVlanId_f, &ipfix_ctl, 0);
    SetIpfixEngineCtl(V, ipfixTimeGranularityMode_f, &ipfix_ctl, 1);/*0-8ms; 1-1ms mode;2-16ms;3-1s*/
    cmd = DRV_IOW( IpfixEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_ctl));
    cmd = DRV_IOW( IpfixEngineCtl0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_ctl));
    cmd = DRV_IOW( IpfixEngineCtl1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_ctl));

    cmd = DRV_IOR( IpeFwdFlowHashCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_flow_hash));
    SetIpeFwdFlowHashCtl(V, ipfixDestUseApsGroupId_f , &ipfix_flow_hash, 1);
    SetIpeFwdFlowHashCtl(V, ipfixDestUseEcmpGroupId_f , &ipfix_flow_hash,0);
    SetIpeFwdFlowHashCtl(V, ipfixDestUseLagId_f , &ipfix_flow_hash,1);

    SetIpeFwdFlowHashCtl(V, ipfixHashBackToL3V6Type_f , &ipfix_flow_hash, 1);
    SetIpeFwdFlowHashCtl(V, microFlowHashBackToL3V6Type_f , &ipfix_flow_hash, 1);
    SetIpeFwdFlowHashCtl(V, ipfixHashL2L3ForcebackL2Type_f , &ipfix_flow_hash, 0x3);
    SetIpeFwdFlowHashCtl(V, microFlowHashL2L3ForcebackL2Type_f , &ipfix_flow_hash, 0x3);
    SetIpeFwdFlowHashCtl(V, ipfixHashL2L3ForcebackL3Type_f , &ipfix_flow_hash, 1);
    SetIpeFwdFlowHashCtl(V, microFlowHashL2L3ForcebackL3Type_f , &ipfix_flow_hash, 1);
    SetIpeFwdFlowHashCtl(V, mirrorPktEnableMicroflowPolicing_f , &ipfix_flow_hash, 0);
    cmd = DRV_IOW( IpeFwdFlowHashCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_flow_hash));

    sal_memset(&epe_acl_qos_ctl, 0, sizeof(epe_acl_qos_ctl));
    cmd = DRV_IOR(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_qos_ctl));

    SetEpeAclQosCtl(V, ipfixHashBackToL3V6Type_f, &epe_acl_qos_ctl, 1);
    SetEpeAclQosCtl(V, microFlowHashBackToL3V6Type_f, &epe_acl_qos_ctl, 1);
    SetEpeAclQosCtl(V, ipfixHashL2L3ForcebackL2Type_f, &epe_acl_qos_ctl, 0x3);
    SetEpeAclQosCtl(V, microFlowHashL2L3ForcebackL2Type_f, &epe_acl_qos_ctl, 0x3);
    SetEpeAclQosCtl(V, ipfixHashL2L3ForcebackL3Type_f, &epe_acl_qos_ctl, 1);
    SetEpeAclQosCtl(V, microFlowHashL2L3ForcebackL3Type_f, &epe_acl_qos_ctl, 1);

    cmd = DRV_IOW( EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_qos_ctl));
    /*set merge global control */
    cmd = DRV_IOR( IpePreLookupFlowCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lookup_ctl));
    SetIpePreLookupFlowCtl(V, flowVxlanFlowKeyL4UseOuter_f, &lookup_ctl, 1);
    SetIpePreLookupFlowCtl(V, flowGreFlowKeyL4UseOuter_f  , &lookup_ctl, 1);
    SetIpePreLookupFlowCtl(V, flowCapwapFlowKeyL4UseOuter_f,&lookup_ctl, 1);
    cmd = DRV_IOW( IpePreLookupFlowCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lookup_ctl));

    /* ipfix mirror*/
    value_a = 1;
    cmd = DRV_IOW( IpeFwdCtl_t, IpeFwdCtl_ipfixMirrorPktLogEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_a));

    cmd = DRV_IOW( EpeHeaderEditCtl_t, EpeHeaderEditCtl_ipfixMirrorPktLogEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_a));

    SetFlowAccLfsrCtl(V, igr0RandSeedValue_f, &flowacclfsrctl, SYS_USW_PROFILE0_SEED);
    SetFlowAccLfsrCtl(V, igr2RandSeedValue_f, &flowacclfsrctl, SYS_USW_PROFILE2_SEED);
    SetFlowAccLfsrCtl(V, igr3RandSeedValue_f, &flowacclfsrctl, SYS_USW_PROFILE3_SEED);
    SetFlowAccLfsrCtl(V, egr0RandSeedValue_f, &flowacclfsrctl, SYS_USW_PROFILE0_SEED);
    SetFlowAccLfsrCtl(V, egr2RandSeedValue_f, &flowacclfsrctl, SYS_USW_PROFILE2_SEED);
    SetFlowAccLfsrCtl(V, egr3RandSeedValue_f, &flowacclfsrctl, SYS_USW_PROFILE3_SEED);
    cmd = DRV_IOW(FlowAccLfsrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &flowacclfsrctl));

    value_a = 32;
    cmd = DRV_IOW(FlowAccMiscCtl_t, FlowAccMiscCtl_hashCacheClrInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_a));
    return CTC_E_NONE;
}

STATIC int32
sys_usw_ipfix_overflow_isr(uint8 lchip, uint32 intr, void* p_data)
{
    CTC_INTERRUPT_EVENT_FUNC event_cb;

    CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_IPFIX_OVERFLOW, &event_cb));
    if(event_cb)
    {
        uint8  gchip;
        ctc_ipfix_event_t event;
        uint32 cmd = DRV_IOR(IpfixEngineInterrupt_t, IpfixEngineInterrupt_interruptValid_f);
        uint32 exceed_threshold;

        DRV_IOCTL(lchip, 0, cmd, &exceed_threshold);
        event.exceed_threshold = exceed_threshold;
        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        CTC_ERROR_RETURN(event_cb(gchip, &event));
    }
    else
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
			return CTC_E_NOT_INIT;
    }

    return CTC_E_NONE;

}

int32
sys_usw_ipfix_init(uint8 lchip, void* p_global_cfg)
{
    uint32 entry_num = 0;
    ctc_spool_t spool;
    uintptr chip_id = lchip;
    int32  ret = CTC_E_NONE;
    sys_ipfix_sample_profile_t static_sample_prf;
    uint8  work_status = 0;

    CTC_PTR_VALID_CHECK(p_global_cfg);

    if (0 == SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_IPFIX))
    {
        return CTC_E_NONE;
    }

    sys_usw_ftm_query_table_entry_num(lchip, DsIpfixL2HashKey_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_IPFIX_ENTRY_NUM) = entry_num;
    if (!entry_num)
    {   /* no resources */
        return CTC_E_NONE;
    }

    if (p_usw_ipfix_master[lchip])
    {
        return CTC_E_NONE;
    }

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }
    p_usw_ipfix_master[lchip] = (sys_ipfix_master_t*)mem_malloc(MEM_IPFIX_MODULE, sizeof(sys_ipfix_master_t));

    if (NULL == p_usw_ipfix_master[lchip])
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;
    }

    sal_memset(p_usw_ipfix_master[lchip], 0, sizeof(sys_ipfix_master_t));
    sal_mutex_create(&p_usw_ipfix_master[lchip]->p_mutex);

    /*hash spool*/
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE);
    spool.max_count = MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE);
    spool.user_data_size = sizeof(sys_ipfix_sample_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_ipfix_sample_spool_hask_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_ipfix_sample_spool_hash_cmp;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_ipfix_sample_spool_alloc_index;
    spool.spool_free = (spool_free_fn)_sys_usw_ipfix_sample_spool_free_index;
    p_usw_ipfix_master[lchip]->sample_spool = ctc_spool_create(&spool);
    if(NULL == p_usw_ipfix_master[lchip]->sample_spool)
    {
        ret = CTC_E_NO_MEMORY;
        goto error;
    }

    /*static spool add*/
    sal_memset(&static_sample_prf, 0, sizeof(static_sample_prf));
    static_sample_prf.interval = 4;
    static_sample_prf.mode = 0x0;
    CTC_ERROR_GOTO(ctc_spool_static_add(p_usw_ipfix_master[lchip]->sample_spool, &static_sample_prf), ret, error);

    CTC_ERROR_GOTO(_sys_usw_ipfix_init_register(lchip, p_global_cfg), ret, error);
    if (!p_usw_ipfix_master[lchip]->ipfix_cb)
    {
        sys_usw_ipfix_register_cb(lchip, _sys_usw_ipfix_process_isr, (void*)chip_id);
    }

    CTC_ERROR_GOTO(sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_IPFIX,sys_usw_ipfix_ftm_cb), ret, error);
    CTC_ERROR_GOTO(sys_usw_dma_register_cb(lchip, SYS_DMA_CB_TYPE_IPFIX,
        sys_usw_ipfix_sync_data), ret, error);
    CTC_ERROR_GOTO(sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_IPFIX_USEAGE_OVERFLOW, sys_usw_ipfix_overflow_isr),
                                                                                ret, error);
    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_GOTO(sys_usw_ipfix_wb_restore(lchip), ret, error);
    }

    drv_usw_agent_register_cb(DRV_AGENT_CB_GET_IPFIX, (void*)sys_usw_ipfix_get_cb);
    drv_usw_agent_register_cb(DRV_AGENT_CB_IPFIX_EXPORT, (void*)_sys_usw_ipfix_export_stats);
    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_IPFIX, sys_usw_ipfix_wb_sync), ret, error);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;
error:
    if(p_usw_ipfix_master[lchip])
    {
        if(p_usw_ipfix_master[lchip]->sample_spool)
        {
            ctc_spool_free(p_usw_ipfix_master[lchip]->sample_spool);
        }

        if(p_usw_ipfix_master[lchip]->p_mutex)
        {
            sal_mutex_destroy(p_usw_ipfix_master[lchip]->p_mutex);
        }

        mem_free(p_usw_ipfix_master[lchip]);
    }
    return ret;

}

int32
sys_usw_ipfix_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    if (NULL == p_usw_ipfix_master[lchip])
    {
        return CTC_E_NONE;
    }

    ctc_spool_free(p_usw_ipfix_master[lchip]->sample_spool);
    sal_mutex_destroy(p_usw_ipfix_master[lchip]->p_mutex);
    mem_free(p_usw_ipfix_master[lchip]);

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_ipfix_set_l2_hash_field_sel(uint8 lchip, ctc_ipfix_hash_field_sel_t* field_sel)
{

    IpfixL2HashFieldSelect_m  hash_field;
    uint8 global_port_type = 0;
    uint32 cmd = 0;

    sal_memset(&hash_field, 0, sizeof(IpfixL2HashFieldSelect_m));

    if((field_sel->u.mac.mac_sa && field_sel->u.mac.src_cid) || (field_sel->u.mac.mac_da && field_sel->u.mac.dst_cid))
    {
        return CTC_E_PARAM_CONFLICT;
    }

    SetIpfixL2HashFieldSelect(V, cfiEn_f, &hash_field, field_sel->u.mac.cfi);
    SetIpfixL2HashFieldSelect(V, cosEn_f, &hash_field, field_sel->u.mac.cos);
    SetIpfixL2HashFieldSelect(V, vlanIdEn_f, &hash_field, field_sel->u.mac.vlan_id);
    SetIpfixL2HashFieldSelect(V, etherTypeEn_f, &hash_field, field_sel->u.mac.eth_type);
    SetIpfixL2HashFieldSelect(V, macDaEn_f, &hash_field, field_sel->u.mac.mac_da);
    SetIpfixL2HashFieldSelect(V, macSaEn_f, &hash_field, field_sel->u.mac.mac_sa);
    SetIpfixL2HashFieldSelect(V, macSaUseSrcCategoryId_f, &hash_field, field_sel->u.mac.src_cid);
    SetIpfixL2HashFieldSelect(V, macDaUseDstCategoryId_f, &hash_field, field_sel->u.mac.dst_cid);
    SetIpfixL2HashFieldSelect(V, srcCategoryIdEn_f, &hash_field, field_sel->u.mac.src_cid);
    SetIpfixL2HashFieldSelect(V, dstCategoryIdEn_f, &hash_field, field_sel->u.mac.dst_cid);
    SetIpfixL2HashFieldSelect(V, ipfixCfgProfileIdEn_f, &hash_field, field_sel->u.mac.profile_id);

    if(field_sel->u.mac.vlan_id || field_sel->u.mac.cfi || field_sel->u.mac.cos)
    {
        SetIpfixL2HashFieldSelect(V, vlanIdValidEn_f, &hash_field, 1);
    }

    if(field_sel->u.mac.gport)
    {
        global_port_type = 1;
    }
    else if(field_sel->u.mac.logic_port)
    {
        global_port_type = 2;
    }
    else if(field_sel->u.mac.metadata)
    {
        if (DRV_IS_DUET2(lchip))
        {
            global_port_type = 3;
        }
        else
        {
            global_port_type = 4;
        }
    }
    else
    {
        global_port_type = 0;
    }
    SetIpfixL2HashFieldSelect(V, globalPortType_f, &hash_field, global_port_type);

    cmd = DRV_IOW( IpfixL2HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));
    cmd = DRV_IOW( IpfixL2HashFieldSelect0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));
    cmd = DRV_IOW( IpfixL2HashFieldSelect1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_ipfix_set_ipv4_hash_field_sel(uint8 lchip, ctc_ipfix_hash_field_sel_t* field_sel)
{

    IpfixL3Ipv4HashFieldSelect_m  hash_field;
    uint32 cmd = 0;
    uint32 ip_prefix_len = 0;
    uint8 global_port_type = 0;

    uint8 is_gre = !!field_sel->u.ipv4.gre_key;
    uint8 is_nvgre = !!field_sel->u.ipv4.nvgre_key;
    uint8 is_icmp = field_sel->u.ipv4.icmp_code || field_sel->u.ipv4.icmp_type;
    uint8 is_igmp = !!field_sel->u.ipv4.igmp_type;
    uint8 is_vxlan = !!field_sel->u.ipv4.vxlan_vni;
    uint8 is_l4_port = field_sel->u.ipv4.l4_src_port || field_sel->u.ipv4.l4_dst_port;

    if(field_sel->u.ipv4.l4_sub_type)
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;
    }

    if((field_sel->u.ipv4.ip_sa && field_sel->u.ipv4.src_cid) || (field_sel->u.ipv4.ip_da && field_sel->u.ipv4.dst_cid))
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if((is_gre + is_nvgre + is_icmp + is_igmp + is_vxlan + is_l4_port) > 1 )
    {
        return CTC_E_PARAM_CONFLICT;
    }

    sal_memset(&hash_field, 0, sizeof(IpfixL3Ipv4HashFieldSelect_m));
    /*share field mode*/
    if((field_sel->u.ipv4.ip_frag || field_sel->u.ipv4.tcp_flags) &&
        !(field_sel->u.ipv4.ip_identification || field_sel->u.ipv4.ip_pkt_len || field_sel->u.ipv4.vrfid))
    {
        SetIpfixL3Ipv4HashFieldSelect(V, shareFieldMode_f, &hash_field, 0);
        SetIpfixL3Ipv4HashFieldSelect(V, u1_g1_fragInfoEn_f, &hash_field, field_sel->u.ipv4.ip_frag);
        SetIpfixL3Ipv4HashFieldSelect(V, u1_g1_tcpFlagsEn_f, &hash_field, field_sel->u.ipv4.tcp_flags);
    }
    else if(field_sel->u.ipv4.ip_identification && !(field_sel->u.ipv4.ip_frag || field_sel->u.ipv4.tcp_flags
                                                    || field_sel->u.ipv4.ip_pkt_len  || field_sel->u.ipv4.vrfid))
    {
        SetIpfixL3Ipv4HashFieldSelect(V, shareFieldMode_f, &hash_field, 1);
        SetIpfixL3Ipv4HashFieldSelect(V, u1_g2_ipIdentificationEn_f, &hash_field, field_sel->u.ipv4.ip_identification);
    }
    else if(field_sel->u.ipv4.ip_pkt_len && !(field_sel->u.ipv4.tcp_flags || field_sel->u.ipv4.ip_identification
                                                || field_sel->u.ipv4.vrfid))
    {
        SetIpfixL3Ipv4HashFieldSelect(V, shareFieldMode_f, &hash_field, 2);
        SetIpfixL3Ipv4HashFieldSelect(V, u1_g3_fragInfoEn_f, &hash_field, field_sel->u.ipv4.ip_frag);
        SetIpfixL3Ipv4HashFieldSelect(V, u1_g3_ipLengthEn_f, &hash_field, field_sel->u.ipv4.ip_pkt_len);
    }
    else if(field_sel->u.ipv4.vrfid && !(field_sel->u.ipv4.tcp_flags || field_sel->u.ipv4.ip_identification ||
                                            field_sel->u.ipv4.ip_pkt_len))
    {
        SetIpfixL3Ipv4HashFieldSelect(V, shareFieldMode_f, &hash_field, 3);
        SetIpfixL3Ipv4HashFieldSelect(V, u1_g4_fragInfoEn_f, &hash_field, field_sel->u.ipv4.ip_frag);
        SetIpfixL3Ipv4HashFieldSelect(V, u1_g4_vrfIdEn_f, &hash_field, field_sel->u.ipv4.vrfid);
    }
    else if(field_sel->u.ipv4.ip_frag || field_sel->u.ipv4.tcp_flags || field_sel->u.ipv4.ip_identification
                                                    || field_sel->u.ipv4.ip_pkt_len || field_sel->u.ipv4.vrfid)
    {
        return CTC_E_PARAM_CONFLICT;
    }

    SetIpfixL3Ipv4HashFieldSelect(V, ipfixCfgProfileIdEn_f, &hash_field, field_sel->u.ipv4.profile_id);
    SetIpfixL3Ipv4HashFieldSelect(V, dscpEn_f, &hash_field, field_sel->u.ipv4.dscp);
    SetIpfixL3Ipv4HashFieldSelect(V, ecnEn_f, &hash_field, field_sel->u.ipv4.ecn);
    SetIpfixL3Ipv4HashFieldSelect(V, ttlEn_f, &hash_field, field_sel->u.ipv4.ttl);
    SetIpfixL3Ipv4HashFieldSelect(V, icmpOpcodeEn_f, &hash_field, field_sel->u.ipv4.icmp_code);
    SetIpfixL3Ipv4HashFieldSelect(V, icmpTypeEn_f, &hash_field, field_sel->u.ipv4.icmp_type);
    SetIpfixL3Ipv4HashFieldSelect(V, igmpTypeEn_f, &hash_field, field_sel->u.ipv4.igmp_type);
    SetIpfixL3Ipv4HashFieldSelect(V, greKeyEn_f, &hash_field, field_sel->u.ipv4.gre_key);
    SetIpfixL3Ipv4HashFieldSelect(V, nvgreVsidEn_f, &hash_field, field_sel->u.ipv4.nvgre_key);
    SetIpfixL3Ipv4HashFieldSelect(V, ignorVxlan_f, &hash_field, (field_sel->u.ipv4.vxlan_vni)?0:1);

    if (field_sel->u.ipv4.ip_da)
    {
        IPFIX_IPV4_MASK_LEN_CHECK(field_sel->u.ipv4.ip_da_mask);
        ip_prefix_len = field_sel->u.ipv4.ip_da_mask - 1 ;

        SetIpfixL3Ipv4HashFieldSelect(V,  ipDaPrefixLength_f, &hash_field, ip_prefix_len);
        SetIpfixL3Ipv4HashFieldSelect(V,  ipDaEn_f, &hash_field, 1);
    }

    if (field_sel->u.ipv4.ip_sa)
    {
        IPFIX_IPV4_MASK_LEN_CHECK(field_sel->u.ipv4.ip_sa_mask);
        ip_prefix_len = field_sel->u.ipv4.ip_sa_mask - 1;

        SetIpfixL3Ipv4HashFieldSelect(V,  ipSaPrefixLength_f, &hash_field, ip_prefix_len);
        SetIpfixL3Ipv4HashFieldSelect(V,  ipSaEn_f, &hash_field, 1);
    }
    SetIpfixL3Ipv4HashFieldSelect(V, ipSaUseSrcCategoryId_f, &hash_field, field_sel->u.ipv4.src_cid);
    SetIpfixL3Ipv4HashFieldSelect(V, ipDaUseDstCategoryId_f, &hash_field, field_sel->u.ipv4.dst_cid);
    SetIpfixL3Ipv4HashFieldSelect(V, srcCategoryIdEn_f, &hash_field, field_sel->u.ipv4.src_cid);
    SetIpfixL3Ipv4HashFieldSelect(V, dstCategoryIdEn_f, &hash_field, field_sel->u.ipv4.dst_cid);
    SetIpfixL3Ipv4HashFieldSelect(V, vxlanVniEn_f, &hash_field, field_sel->u.ipv4.vxlan_vni);
    SetIpfixL3Ipv4HashFieldSelect(V, l4DestPortEn_f, &hash_field, field_sel->u.ipv4.l4_dst_port);
    SetIpfixL3Ipv4HashFieldSelect(V, l4SourcePortEn_f, &hash_field, field_sel->u.ipv4.l4_src_port);
    SetIpfixL3Ipv4HashFieldSelect(V, layer3HeaderProtocolEn_f, &hash_field, field_sel->u.ipv4.ip_protocol);
    SetIpfixL3Ipv4HashFieldSelect(V, isVxlanEn_f , &hash_field, field_sel->u.ipv4.vxlan_vni);

    if(field_sel->u.ipv4.gport)
    {
        global_port_type = 1;
    }
    else if(field_sel->u.ipv4.logic_port)
    {
        global_port_type = 2;
    }
    else if(field_sel->u.ipv4.metadata)
    {
        if (DRV_IS_DUET2(lchip))
        {
            global_port_type = 3;
        }
        else
        {
            global_port_type = 4;
        }
    }
    else
    {
        global_port_type = 0;
    }
    SetIpfixL3Ipv4HashFieldSelect(V, globalPortType_f, &hash_field, global_port_type);
    cmd = DRV_IOW( IpfixL3Ipv4HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));
    cmd = DRV_IOW( IpfixL3Ipv4HashFieldSelect0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));
    cmd = DRV_IOW( IpfixL3Ipv4HashFieldSelect1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));


  return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_set_ipv6_hash_field_sel(uint8 lchip, ctc_ipfix_hash_field_sel_t* field_sel)
{

    IpfixL3Ipv6HashFieldSelect_m  hash_field;
    uint32 cmd = 0;
    uint32 ip_prefix_len = 0;
    uint8 global_port_type = 0;

    uint8 is_gre = !!field_sel->u.ipv6.gre_key;
    uint8 is_nvgre = !!field_sel->u.ipv6.nvgre_key;
    uint8 is_icmp = field_sel->u.ipv6.icmp_code || field_sel->u.ipv6.icmp_type;
    uint8 is_igmp = !!field_sel->u.ipv6.igmp_type;
    uint8 is_vxlan = !!field_sel->u.ipv6.vxlan_vni;
    uint8 is_l4_port = field_sel->u.ipv6.l4_src_port || field_sel->u.ipv6.l4_dst_port;
    uint8 is_capwap = field_sel->u.ipv6.wlan_ctl_packet || field_sel->u.ipv6.wlan_radio_id || field_sel->u.ipv6.wlan_radio_mac;

    uint8 is_mode0 = (field_sel->u.ipv6.ip_sa && (field_sel->u.ipv6.ip_sa_mask > 64)) || \
                                                            (field_sel->u.ipv6.ip_da && (field_sel->u.ipv6.ip_da_mask > 64));
    uint8 is_mode1 = (field_sel->u.ipv6.flow_label || field_sel->u.ipv6.ttl || field_sel->u.ipv6.ecn ||
                                            field_sel->u.ipv6.ip_frag || field_sel->u.ipv6.aware_tunnel_info_en ||
                                            field_sel->u.ipv6.src_cid || field_sel->u.ipv6.dst_cid || field_sel->u.ipv6.tcp_flags );

    if(field_sel->u.ipv6.l4_type || field_sel->u.ipv6.l4_sub_type)
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (field_sel->u.ipv6.aware_tunnel_info_en)
    {
        if ((is_gre + is_nvgre + is_vxlan + is_capwap) > 1 )
        {
            return CTC_E_PARAM_CONFLICT;
        }

        if ((is_icmp + is_igmp + is_l4_port) > 1 )
        {
            return CTC_E_PARAM_CONFLICT;
        }
    }
    else if ((is_gre + is_nvgre + is_icmp + is_igmp + is_vxlan + is_l4_port) > 1 )
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if(is_mode0 && is_mode1)
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if(is_capwap && !field_sel->u.ipv6.aware_tunnel_info_en)
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "capwap info must use merge key\n");
        return CTC_E_NOT_SUPPORT;
    }

    sal_memset(&hash_field, 0, sizeof(IpfixL3Ipv6HashFieldSelect_m));
    if(is_mode1)
    {
        SetIpfixL3Ipv6HashFieldSelect(V, ipAddressMode_f, &hash_field, 1);
    }

    SetIpfixL3Ipv6HashFieldSelect(V, ipfixCfgProfileIdEn_f, &hash_field, field_sel->u.ipv6.profile_id);
    SetIpfixL3Ipv6HashFieldSelect(V, dscpEn_f, &hash_field, field_sel->u.ipv6.dscp);
    SetIpfixL3Ipv6HashFieldSelect(V, icmpOpcodeEn_f, &hash_field, field_sel->u.ipv6.icmp_code);
    SetIpfixL3Ipv6HashFieldSelect(V, icmpTypeEn_f, &hash_field, field_sel->u.ipv6.icmp_type);
    SetIpfixL3Ipv6HashFieldSelect(V, igmpTypeEn_f, &hash_field, field_sel->u.ipv6.igmp_type);
    SetIpfixL3Ipv6HashFieldSelect(V, l4DestPortEn_f, &hash_field, field_sel->u.ipv6.l4_dst_port);
    SetIpfixL3Ipv6HashFieldSelect(V, l4SourcePortEn_f, &hash_field, field_sel->u.ipv6.l4_src_port);
    SetIpfixL3Ipv6HashFieldSelect(V, layer3HeaderProtocolEn_f, &hash_field, field_sel->u.ipv6.ip_protocol);
    if(!field_sel->u.ipv6.aware_tunnel_info_en)
    {
        SetIpfixL3Ipv6HashFieldSelect(V, greKeyEn_f, &hash_field, field_sel->u.ipv6.gre_key);
        SetIpfixL3Ipv6HashFieldSelect(V, nvgreVsidEn_f, &hash_field, field_sel->u.ipv6.nvgre_key);
        SetIpfixL3Ipv6HashFieldSelect(V, vxlanVniEn_f, &hash_field, field_sel->u.ipv6.vxlan_vni);
        SetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyMode_f, &hash_field, 0);
    }
    else
    {
        SetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyMode_f, &hash_field, 1);
        SetIpfixL3Ipv6HashFieldSelect(V, g1_ignorVxlan_f, &hash_field, !(field_sel->u.ipv6.vxlan_vni));
        SetIpfixL3Ipv6HashFieldSelect(V, g1_ignorCapwap_f, &hash_field, !is_capwap);
        SetIpfixL3Ipv6HashFieldSelect(V, g1_ignorGreWithKey_f, &hash_field, !(field_sel->u.ipv6.gre_key + field_sel->u.ipv6.nvgre_key));
        SetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyVxlanVniEn_f, &hash_field, field_sel->u.ipv6.vxlan_vni);
        SetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyNvgreVsidEn_f, &hash_field, field_sel->u.ipv6.nvgre_key);
        SetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyGreKeyEn_f, &hash_field, field_sel->u.ipv6.gre_key);
        SetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyCapwapRadioMacEn_f, &hash_field, field_sel->u.ipv6.wlan_radio_mac);
        SetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyCapwapRidEn_f, &hash_field, field_sel->u.ipv6.wlan_radio_id);
        SetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyCapwapControlPktEn_f, &hash_field, field_sel->u.ipv6.wlan_ctl_packet);
    }
    if (field_sel->u.ipv6.ip_da)
    {
        IPFIX_IPV6_MASK_LEN_CHECK(field_sel->u.ipv6.ip_da_mask);
        ip_prefix_len = (field_sel->u.ipv6.ip_da_mask>>2)-1;
        SetIpfixL3Ipv6HashFieldSelect(V, ipDaPrefixLength_f, &hash_field, ip_prefix_len);
        SetIpfixL3Ipv6HashFieldSelect(V, ipDaEn_f, &hash_field, 1);
    }

    if (field_sel->u.ipv6.ip_sa)
    {
        IPFIX_IPV6_MASK_LEN_CHECK(field_sel->u.ipv6.ip_sa_mask);
        ip_prefix_len = (field_sel->u.ipv6.ip_sa_mask>>2)-1;
        SetIpfixL3Ipv6HashFieldSelect(V, ipSaPrefixLength_f, &hash_field, ip_prefix_len);
        SetIpfixL3Ipv6HashFieldSelect(V, ipSaEn_f, &hash_field, 1);
    }

    SetIpfixL3Ipv6HashFieldSelect(V, g1_srcCategoryIdEn_f, &hash_field, field_sel->u.ipv6.src_cid);
    SetIpfixL3Ipv6HashFieldSelect(V, g1_dstCategoryIdEn_f, &hash_field, field_sel->u.ipv6.dst_cid);
    SetIpfixL3Ipv6HashFieldSelect(V, g1_ipv6FlowLabelEn_f, &hash_field, field_sel->u.ipv6.flow_label);
    SetIpfixL3Ipv6HashFieldSelect(V, g1_ttlEn_f, &hash_field, field_sel->u.ipv6.ttl);
    SetIpfixL3Ipv6HashFieldSelect(V, g1_ecnEn_f, &hash_field, field_sel->u.ipv6.ecn);
    SetIpfixL3Ipv6HashFieldSelect(V, g1_fragInfoEn_f, &hash_field, field_sel->u.ipv6.ip_frag);
    SetIpfixL3Ipv6HashFieldSelect(V, g1_tcpFlagsEn_f, &hash_field, field_sel->u.ipv6.tcp_flags);
    if(field_sel->u.ipv6.gport)
    {
        global_port_type = 1;
    }
    else if(field_sel->u.ipv6.logic_port)
    {
        global_port_type = 2;
    }
    else if(field_sel->u.ipv6.metadata)
    {
        if (DRV_IS_DUET2(lchip))
        {
            global_port_type = 3;
        }
        else
        {
            global_port_type = 4;
        }
    }
    else
    {
        global_port_type = 0;
    }
    SetIpfixL3Ipv6HashFieldSelect(V, globalPortType_f, &hash_field, global_port_type);

    cmd = DRV_IOW( IpfixL3Ipv6HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));
    cmd = DRV_IOW( IpfixL3Ipv6HashFieldSelect0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));
    cmd = DRV_IOW( IpfixL3Ipv6HashFieldSelect1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_set_mpls_hash_field_sel(uint8 lchip, ctc_ipfix_hash_field_sel_t* field_sel)
{

    IpfixL3MplsHashFieldSelect_m  hash_field;
    uint32 cmd = 0;
    uint8 global_port_type = 0;
    sal_memset(&hash_field, 0, sizeof(IpfixL3MplsHashFieldSelect_m));
    SetIpfixL3MplsHashFieldSelect(V, ipfixCfgProfileIdEn_f, &hash_field, field_sel->u.mpls.profile_id);
    SetIpfixL3MplsHashFieldSelect(V,  labelNumEn_f      ,&hash_field, field_sel->u.mpls.label_num);
    SetIpfixL3MplsHashFieldSelect(V,  mplsExp0En_f      ,&hash_field,field_sel->u.mpls.mpls_label0_exp);
    SetIpfixL3MplsHashFieldSelect(V,  mplsExp1En_f      ,&hash_field,field_sel->u.mpls.mpls_label1_exp);
    SetIpfixL3MplsHashFieldSelect(V,  mplsExp2En_f      ,&hash_field, field_sel->u.mpls.mpls_label2_exp);
    SetIpfixL3MplsHashFieldSelect(V, mplsLabel0En_f    , &hash_field,field_sel->u.mpls.mpls_label0_label);
    SetIpfixL3MplsHashFieldSelect(V, mplsLabel1En_f    , &hash_field,field_sel->u.mpls.mpls_label1_label);
    SetIpfixL3MplsHashFieldSelect(V, mplsLabel2En_f    , &hash_field,field_sel->u.mpls.mpls_label2_label);
    SetIpfixL3MplsHashFieldSelect(V, mplsSbit0En_f     , &hash_field,field_sel->u.mpls.mpls_label0_s);
    SetIpfixL3MplsHashFieldSelect(V, mplsSbit1En_f     , &hash_field,field_sel->u.mpls.mpls_label1_s);
    SetIpfixL3MplsHashFieldSelect(V, mplsSbit2En_f     , &hash_field,field_sel->u.mpls.mpls_label2_s);
    SetIpfixL3MplsHashFieldSelect(V, mplsTtl0En_f      , &hash_field,field_sel->u.mpls.mpls_label0_ttl);
    SetIpfixL3MplsHashFieldSelect(V, mplsTtl1En_f      , &hash_field,field_sel->u.mpls.mpls_label1_ttl);
    SetIpfixL3MplsHashFieldSelect(V, mplsTtl2En_f      , &hash_field,field_sel->u.mpls.mpls_label2_ttl);

    if(field_sel->u.mpls.gport)
    {
        global_port_type = 1;
    }
    else if(field_sel->u.mpls.logic_port)
    {
        global_port_type = 2;
    }
    else if(field_sel->u.mpls.metadata)
    {
        if (DRV_IS_DUET2(lchip))
        {
            global_port_type = 3;
        }
        else
        {
            global_port_type = 4;
        }
    }
    else
    {
        global_port_type = 0;
    }
    SetIpfixL3MplsHashFieldSelect(V, globalPortType_f, &hash_field, global_port_type);
    cmd = DRV_IOW( IpfixL3MplsHashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));
    cmd = DRV_IOW( IpfixL3MplsHashFieldSelect0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));
    cmd = DRV_IOW( IpfixL3MplsHashFieldSelect1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_ipfix_set_l2l3_hash_field_sel(uint8 lchip, ctc_ipfix_hash_field_sel_t* field_sel)
{

    IpfixL2L3HashFieldSelect_m  hash_field;
    uint32 cmd = 0;
    uint32 ip_prefix_len = 0;
    uint8 global_port_type = 0;
    uint8 is_capwap = field_sel->u.l2_l3.wlan_ctl_packet || field_sel->u.l2_l3.wlan_radio_id || field_sel->u.l2_l3.wlan_radio_mac;
    uint8 is_gre = !!field_sel->u.l2_l3.gre_key;
    uint8 is_nvgre = !!field_sel->u.l2_l3.nvgre_key;
    uint8 is_icmp = field_sel->u.l2_l3.icmp_code || field_sel->u.l2_l3.icmp_type;
    uint8 is_igmp = !!field_sel->u.l2_l3.igmp_type;
    uint8 is_vxlan = !!field_sel->u.l2_l3.vxlan_vni;
    uint8 is_l4_port = field_sel->u.l2_l3.l4_src_port || field_sel->u.l2_l3.l4_dst_port || field_sel->u.l2_l3.tcp_flags;

    uint8 is_mpls = field_sel->u.l2_l3.label_num || \
                    field_sel->u.l2_l3.mpls_label0_exp || \
                    field_sel->u.l2_l3.mpls_label1_exp || \
                    field_sel->u.l2_l3.mpls_label2_exp || \
                    field_sel->u.l2_l3.mpls_label0_label || \
                    field_sel->u.l2_l3.mpls_label1_label || \
                    field_sel->u.l2_l3.mpls_label2_label || \
                    field_sel->u.l2_l3.mpls_label0_s || \
                    field_sel->u.l2_l3.mpls_label1_s || \
                    field_sel->u.l2_l3.mpls_label2_s || \
                    field_sel->u.l2_l3.mpls_label0_ttl || \
                    field_sel->u.l2_l3.mpls_label1_ttl || \
                    field_sel->u.l2_l3.mpls_label2_ttl;


    if(field_sel->u.l2_l3.l4_type || field_sel->u.l2_l3.l4_sub_type)
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (field_sel->u.l2_l3.aware_tunnel_info_en)
    {
        if ((is_gre + is_nvgre + is_vxlan + is_capwap) > 1 )
        {
            return CTC_E_PARAM_CONFLICT;
        }

         if ((is_icmp + is_igmp + is_l4_port) > 1 )
         {
             return CTC_E_PARAM_CONFLICT;
         }
    }
    else if ((is_gre + is_nvgre + is_icmp + is_igmp + is_vxlan + is_l4_port) > 1 )
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if((field_sel->u.l2_l3.ip_sa && field_sel->u.l2_l3.src_cid) || (field_sel->u.l2_l3.ip_da && field_sel->u.l2_l3.src_cid))
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if((field_sel->u.l2_l3.ctag_cfi || field_sel->u.l2_l3.ctag_cos || field_sel->u.l2_l3.ctag_vlan) &&
        (field_sel->u.l2_l3.stag_cfi || field_sel->u.l2_l3.stag_cos || field_sel->u.l2_l3.stag_vlan))
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if(is_mpls && (field_sel->u.l2_l3.ecn || field_sel->u.l2_l3.ttl || field_sel->u.l2_l3.dscp || \
                        field_sel->u.l2_l3.ip_sa || field_sel->u.l2_l3.ip_da || field_sel->u.l2_l3.aware_tunnel_info_en))
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if(field_sel->u.l2_l3.aware_tunnel_info_en && (field_sel->u.l2_l3.ip_protocol || field_sel->u.l2_l3.ip_frag || \
                        field_sel->u.l2_l3.ip_identification || field_sel->u.l2_l3.vrfid || field_sel->u.l2_l3.tcp_flags))
    {
        return CTC_E_PARAM_CONFLICT;
    }
    if(is_capwap && !field_sel->u.l2_l3.aware_tunnel_info_en)
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "capwap info must use merge key\n");
        return CTC_E_NOT_SUPPORT;
    }

    sal_memset(&hash_field, 0, sizeof(IpfixL2L3HashFieldSelect_m));
    SetIpfixL2L3HashFieldSelect(V,  vlanIdEn_f       , &hash_field,field_sel->u.l2_l3.stag_vlan || field_sel->u.l2_l3.ctag_vlan);
    SetIpfixL2L3HashFieldSelect(V,  cfiEn_f       , &hash_field,field_sel->u.l2_l3.stag_cfi || field_sel->u.l2_l3.ctag_cfi);
    SetIpfixL2L3HashFieldSelect(V,  cosEn_f       , &hash_field,field_sel->u.l2_l3.stag_cos || field_sel->u.l2_l3.ctag_cos);
    if(field_sel->u.l2_l3.ctag_cfi || field_sel->u.l2_l3.ctag_cos || field_sel->u.l2_l3.ctag_vlan ||
        field_sel->u.l2_l3.stag_cfi || field_sel->u.l2_l3.stag_cos || field_sel->u.l2_l3.stag_vlan)
    {
        SetIpfixL2L3HashFieldSelect(V,  vlanIdValidEn_f  , &hash_field,1);
    }

    if (field_sel->u.l2_l3.ip_da)
    {
        IPFIX_IPV4_MASK_LEN_CHECK(field_sel->u.l2_l3.ip_da_mask);
        ip_prefix_len = field_sel->u.l2_l3.ip_da_mask - 1;

        SetIpfixL2L3HashFieldSelect(V,  ipDaPrefixLength_f, &hash_field, ip_prefix_len);
        SetIpfixL2L3HashFieldSelect(V,  ipDaEn_f, &hash_field, 1);
    }

    if (field_sel->u.l2_l3.ip_sa)
    {
        IPFIX_IPV4_MASK_LEN_CHECK(field_sel->u.l2_l3.ip_sa_mask);
        ip_prefix_len = field_sel->u.l2_l3.ip_sa_mask - 1;

       SetIpfixL2L3HashFieldSelect(V,  ipSaPrefixLength_f, &hash_field, ip_prefix_len);
       SetIpfixL2L3HashFieldSelect(V,  ipSaEn_f, &hash_field, 1);
    }
    SetIpfixL2L3HashFieldSelect(V, ipSaUseSrcCategoryId_f, &hash_field, field_sel->u.l2_l3.src_cid);
    SetIpfixL2L3HashFieldSelect(V, ipDaUseDstCategoryId_f, &hash_field, field_sel->u.l2_l3.dst_cid);
    SetIpfixL2L3HashFieldSelect(V, srcCategoryIdEn_f, &hash_field, field_sel->u.l2_l3.src_cid);
    SetIpfixL2L3HashFieldSelect(V, dstCategoryIdEn_f, &hash_field, field_sel->u.l2_l3.dst_cid);
    SetIpfixL2L3HashFieldSelect(V, ipfixCfgProfileIdEn_f, &hash_field, field_sel->u.l2_l3.profile_id);
    SetIpfixL2L3HashFieldSelect(V,  dscpEn_f          , &hash_field,field_sel->u.l2_l3.dscp);
    SetIpfixL2L3HashFieldSelect(V,  ecnEn_f           , &hash_field,field_sel->u.l2_l3.ecn);
    SetIpfixL2L3HashFieldSelect(V,  ttlEn_f           , &hash_field,field_sel->u.l2_l3.ttl);
    SetIpfixL2L3HashFieldSelect(V,  etherTypeEn_f     , &hash_field,field_sel->u.l2_l3.eth_type);
    SetIpfixL2L3HashFieldSelect(V,  icmpOpcodeEn_f    , &hash_field,field_sel->u.l2_l3.icmp_code);
    SetIpfixL2L3HashFieldSelect(V,  icmpTypeEn_f      , &hash_field,field_sel->u.l2_l3.icmp_type);
    SetIpfixL2L3HashFieldSelect(V,  igmpTypeEn_f      , &hash_field,field_sel->u.l2_l3.igmp_type);

    SetIpfixL2L3HashFieldSelect(V,  l4DestPortEn_f, &hash_field,field_sel->u.l2_l3.l4_dst_port);
    SetIpfixL2L3HashFieldSelect(V,  l4SourcePortEn_f, &hash_field,field_sel->u.l2_l3.l4_src_port);
    SetIpfixL2L3HashFieldSelect(V,  labelNumEn_f, &hash_field,field_sel->u.l2_l3.label_num);
    SetIpfixL2L3HashFieldSelect(V,  macDaEn_f         , &hash_field,field_sel->u.l2_l3.mac_da);
    SetIpfixL2L3HashFieldSelect(V,  macSaEn_f         , &hash_field,field_sel->u.l2_l3.mac_sa);
    SetIpfixL2L3HashFieldSelect(V,  mplsExp0En_f      , &hash_field,field_sel->u.l2_l3.mpls_label0_exp);
    SetIpfixL2L3HashFieldSelect(V,  mplsExp1En_f      , &hash_field,field_sel->u.l2_l3.mpls_label1_exp);
    SetIpfixL2L3HashFieldSelect(V,  mplsExp2En_f      , &hash_field,field_sel->u.l2_l3.mpls_label2_exp);
    SetIpfixL2L3HashFieldSelect(V, mplsLabel0En_f    , &hash_field,field_sel->u.l2_l3.mpls_label0_label);
    SetIpfixL2L3HashFieldSelect(V, mplsLabel1En_f    , &hash_field,field_sel->u.l2_l3.mpls_label1_label);
    SetIpfixL2L3HashFieldSelect(V, mplsLabel2En_f    , &hash_field,field_sel->u.l2_l3.mpls_label2_label);
    SetIpfixL2L3HashFieldSelect(V, mplsSbit0En_f     , &hash_field,field_sel->u.l2_l3.mpls_label0_s);
    SetIpfixL2L3HashFieldSelect(V, mplsSbit1En_f     , &hash_field,field_sel->u.l2_l3.mpls_label1_s);
    SetIpfixL2L3HashFieldSelect(V, mplsSbit2En_f     , &hash_field,field_sel->u.l2_l3.mpls_label2_s);
    SetIpfixL2L3HashFieldSelect(V, mplsTtl0En_f      , &hash_field,field_sel->u.l2_l3.mpls_label0_ttl);
    SetIpfixL2L3HashFieldSelect(V, mplsTtl1En_f      , &hash_field,field_sel->u.l2_l3.mpls_label1_ttl);
    SetIpfixL2L3HashFieldSelect(V, mplsTtl2En_f      , &hash_field,field_sel->u.l2_l3.mpls_label2_ttl);

    SetIpfixL2L3HashFieldSelect(V, ignorVxlan_f    , &hash_field, !is_vxlan);
    SetIpfixL2L3HashFieldSelect(V, ignorGreWithKey_f    , &hash_field, !(is_gre+is_nvgre));
    SetIpfixL2L3HashFieldSelect(V, isVxlanEn_f    , &hash_field, (field_sel->u.l2_l3.aware_tunnel_info_en) ? 0 : is_vxlan);
    SetIpfixL2L3HashFieldSelect(V, ignorCapwap_f    , &hash_field, !is_capwap);

    SetIpfixL2L3HashFieldSelect(V, mergeKeyMode_f    , &hash_field,field_sel->u.l2_l3.aware_tunnel_info_en);
    if(field_sel->u.l2_l3.aware_tunnel_info_en)
    {
        SetIpfixL2L3HashFieldSelect(V,  g1_mergeKeyVxlanVniEn_f    , &hash_field,is_vxlan);
        SetIpfixL2L3HashFieldSelect(V,  g1_mergeKeyGreKeyEn_f      , &hash_field, is_gre);
        SetIpfixL2L3HashFieldSelect(V,  g1_mergeKeyNvgreVsidEn_f     , &hash_field, is_nvgre);
        SetIpfixL2L3HashFieldSelect(V, g1_mergeKeyCapwapRadioMacEn_f, &hash_field, field_sel->u.l2_l3.wlan_radio_mac);
        SetIpfixL2L3HashFieldSelect(V, g1_mergeKeyCapwapRidEn_f, &hash_field, field_sel->u.l2_l3.wlan_radio_id);
        SetIpfixL2L3HashFieldSelect(V, g1_mergeKeyCapwapControlPktEn_f, &hash_field, field_sel->u.l2_l3.wlan_ctl_packet);
    }
    else
    {
        SetIpfixL2L3HashFieldSelect(V,  vxlanVniEn_f    , &hash_field, is_vxlan);
        SetIpfixL2L3HashFieldSelect(V,  greKeyEn_f    , &hash_field, is_gre);
        SetIpfixL2L3HashFieldSelect(V,  nvgreVsidEn_f    , &hash_field, is_nvgre);

        SetIpfixL2L3HashFieldSelect(V,  g2_layer3HeaderProtocolEn_f  , &hash_field,field_sel->u.l2_l3.ip_protocol);
        SetIpfixL2L3HashFieldSelect(V,  g2_vrfIdEn_f  , &hash_field, !!field_sel->u.l2_l3.vrfid);
        SetIpfixL2L3HashFieldSelect(V,  g2_ipIdentificationEn_f  , &hash_field, !!field_sel->u.l2_l3.ip_identification);
        SetIpfixL2L3HashFieldSelect(V,  g2_fragInfoEn_f  , &hash_field, !!field_sel->u.l2_l3.ip_frag);
        SetIpfixL2L3HashFieldSelect(V,  g2_tcpFlagsEn_f  , &hash_field, field_sel->u.l2_l3.tcp_flags);
    }
    if(field_sel->u.l2_l3.gport)
    {
        global_port_type = 1;
    }
    else if(field_sel->u.l2_l3.logic_port)
    {
        global_port_type = 2;
    }
    else if(field_sel->u.l2_l3.metadata)
    {
        if (DRV_IS_DUET2(lchip))
        {
            global_port_type = 3;
        }
        else
        {
            global_port_type = 4;
        }
    }
    else
    {
        global_port_type = 0;
    }
    SetIpfixL2L3HashFieldSelect(V, globalPortType_f, &hash_field, global_port_type);

    cmd = DRV_IOW( IpfixL2L3HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));
    cmd = DRV_IOW( IpfixL2L3HashFieldSelect0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));
    cmd = DRV_IOW( IpfixL2L3HashFieldSelect1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel->field_sel_id, cmd, &hash_field));


    return CTC_E_NONE;
}

extern int32
sys_usw_ipfix_set_hash_field_sel(uint8 lchip, ctc_ipfix_hash_field_sel_t* field_sel)
{
    int32 ret = CTC_E_NONE;
    SYS_IPFIX_INIT_CHECK(lchip);

    CTC_PTR_VALID_CHECK(field_sel);
    CTC_MAX_VALUE_CHECK(field_sel->field_sel_id, MCHIP_CAP(SYS_CAP_IPFIX_MAX_HASH_SEL_ID));

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_IPFIX_LOCK;
    switch(field_sel->key_type)
    {
    case CTC_IPFIX_KEY_HASH_MAC:
        CTC_ERROR_GOTO(_sys_usw_ipfix_set_l2_hash_field_sel(lchip, field_sel), ret, error_pro);
        break;
    case CTC_IPFIX_KEY_HASH_IPV4:
        CTC_ERROR_GOTO(_sys_usw_ipfix_set_ipv4_hash_field_sel(lchip, field_sel), ret, error_pro);
        break;
    case CTC_IPFIX_KEY_HASH_MPLS:
        CTC_ERROR_GOTO(_sys_usw_ipfix_set_mpls_hash_field_sel(lchip, field_sel), ret, error_pro);
        break;
    case CTC_IPFIX_KEY_HASH_IPV6:
        CTC_ERROR_GOTO(_sys_usw_ipfix_set_ipv6_hash_field_sel(lchip, field_sel), ret, error_pro);
        break;
    case CTC_IPFIX_KEY_HASH_L2_L3:
        CTC_ERROR_GOTO(_sys_usw_ipfix_set_l2l3_hash_field_sel(lchip, field_sel), ret, error_pro);
        break;
    default:
        SYS_IPFIX_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }

error_pro:
    SYS_IPFIX_UNLOCK;
    return ret;
}

extern int32
sys_usw_ipfix_get_hash_field_sel(uint8 lchip, uint8 field_sel_id, uint8 key_type, ctc_ipfix_hash_field_sel_t* field_sel)
{
    uint32 cmd = 0;
    IpfixL2HashFieldSelect_m  l2_hash_field;
    IpfixL3Ipv4HashFieldSelect_m  v4_hash_field;
    IpfixL3Ipv6HashFieldSelect_m  v6_hash_field;
    IpfixL3MplsHashFieldSelect_m  mpls_hash_field;
    IpfixL2L3HashFieldSelect_m  l2l3_hash_field;

    SYS_IPFIX_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(field_sel);
    CTC_MAX_VALUE_CHECK(field_sel_id, MCHIP_CAP(SYS_CAP_IPFIX_MAX_HASH_SEL_ID));
    switch(key_type)
    {
        case CTC_IPFIX_KEY_HASH_MAC:
            cmd = DRV_IOR( IpfixL2HashFieldSelect_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &l2_hash_field));
            field_sel->u.mac.profile_id = GetIpfixL2HashFieldSelect(V, ipfixCfgProfileIdEn_f, &l2_hash_field);
            field_sel->u.mac.cfi = GetIpfixL2HashFieldSelect(V, cfiEn_f, &l2_hash_field);
            field_sel->u.mac.cos = GetIpfixL2HashFieldSelect(V, cosEn_f, &l2_hash_field);
            field_sel->u.mac.vlan_id = GetIpfixL2HashFieldSelect(V, vlanIdEn_f, &l2_hash_field);
            field_sel->u.mac.eth_type = GetIpfixL2HashFieldSelect(V, etherTypeEn_f, &l2_hash_field);
            field_sel->u.mac.mac_da = GetIpfixL2HashFieldSelect(V, macDaEn_f, &l2_hash_field);
            field_sel->u.mac.mac_sa = GetIpfixL2HashFieldSelect(V, macSaEn_f, &l2_hash_field);
            field_sel->u.mac.src_cid = GetIpfixL2HashFieldSelect(V, srcCategoryIdEn_f, &l2_hash_field);
            field_sel->u.mac.dst_cid = GetIpfixL2HashFieldSelect(V, dstCategoryIdEn_f, &l2_hash_field);
             /*field_sel->u.mac.ctag_valid = GetIpfixL2HashFieldSelect(V, flowL2KeyUseCvlanEn_f, &l2_hash_field);*/
            switch(GetIpfixL2HashFieldSelect(V, globalPortType_f, &l2_hash_field))
            {
                case 1:
                    field_sel->u.mac.gport = 1;
                    break;
                case 2:
                    field_sel->u.mac.logic_port = 1;
                    break;
                case 3:
                case 4:
                    field_sel->u.mac.metadata = 1;
                    break;
                default:
                    break;
            }
            break;
        case CTC_IPFIX_KEY_HASH_IPV4:
            cmd = DRV_IOR( IpfixL3Ipv4HashFieldSelect_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &v4_hash_field));
            field_sel->u.ipv4.profile_id = GetIpfixL3Ipv4HashFieldSelect(V, ipfixCfgProfileIdEn_f, &v4_hash_field);
            field_sel->u.ipv4.dscp = GetIpfixL3Ipv4HashFieldSelect(V, dscpEn_f, &v4_hash_field);
            field_sel->u.ipv4.icmp_code = GetIpfixL3Ipv4HashFieldSelect(V, icmpOpcodeEn_f, &v4_hash_field);
            field_sel->u.ipv4.icmp_type = GetIpfixL3Ipv4HashFieldSelect(V, icmpTypeEn_f, &v4_hash_field);
            field_sel->u.ipv4.igmp_type = GetIpfixL3Ipv4HashFieldSelect(V, igmpTypeEn_f, &v4_hash_field);
            field_sel->u.ipv4.gre_key = GetIpfixL3Ipv4HashFieldSelect(V, greKeyEn_f, &v4_hash_field);
            field_sel->u.ipv4.nvgre_key = GetIpfixL3Ipv4HashFieldSelect(V, nvgreVsidEn_f, &v4_hash_field);
            field_sel->u.ipv4.ip_da = GetIpfixL3Ipv4HashFieldSelect(V,  ipDaEn_f, &v4_hash_field);
            field_sel->u.ipv4.ip_da_mask = field_sel->u.ipv4.ip_da ? (GetIpfixL3Ipv4HashFieldSelect(V,  ipDaPrefixLength_f, &v4_hash_field) + 1) : 0;
            field_sel->u.ipv4.ip_sa = GetIpfixL3Ipv4HashFieldSelect(V,  ipSaEn_f, &v4_hash_field);
            field_sel->u.ipv4.ip_sa_mask = field_sel->u.ipv4.ip_sa ? (GetIpfixL3Ipv4HashFieldSelect(V,  ipSaPrefixLength_f, &v4_hash_field)  + 1) : 0;
            field_sel->u.ipv4.vxlan_vni = GetIpfixL3Ipv4HashFieldSelect(V, vxlanVniEn_f, &v4_hash_field);
            field_sel->u.ipv4.l4_dst_port = GetIpfixL3Ipv4HashFieldSelect(V, l4DestPortEn_f, &v4_hash_field);
            field_sel->u.ipv4.l4_src_port = GetIpfixL3Ipv4HashFieldSelect(V, l4SourcePortEn_f, &v4_hash_field);
            field_sel->u.ipv4.ttl= GetIpfixL3Ipv4HashFieldSelect(V, ttlEn_f, &v4_hash_field);
            field_sel->u.ipv4.ecn= GetIpfixL3Ipv4HashFieldSelect(V, ecnEn_f, &v4_hash_field);

            field_sel->u.ipv4.src_cid = GetIpfixL3Ipv4HashFieldSelect(V, srcCategoryIdEn_f, &v4_hash_field);
            field_sel->u.ipv4.dst_cid = GetIpfixL3Ipv4HashFieldSelect(V, dstCategoryIdEn_f, &v4_hash_field);
            switch(GetIpfixL3Ipv4HashFieldSelect(V, shareFieldMode_f, &v4_hash_field))
            {
                case 0:
                    field_sel->u.ipv4.ip_frag = GetIpfixL3Ipv4HashFieldSelect(V, u1_g1_fragInfoEn_f, &v4_hash_field);
                    field_sel->u.ipv4.tcp_flags = GetIpfixL3Ipv4HashFieldSelect(V, u1_g1_tcpFlagsEn_f, &v4_hash_field);
                    break;
                case 1:
                    field_sel->u.ipv4.ip_identification = GetIpfixL3Ipv4HashFieldSelect(V, u1_g2_ipIdentificationEn_f, &v4_hash_field);
                    break;
                case 2:
                    field_sel->u.ipv4.ip_frag = GetIpfixL3Ipv4HashFieldSelect(V, u1_g3_fragInfoEn_f, &v4_hash_field);
                    field_sel->u.ipv4.ip_pkt_len = GetIpfixL3Ipv4HashFieldSelect(V, u1_g3_ipLengthEn_f, &v4_hash_field);
                    break;
                default:
                    field_sel->u.ipv4.ip_frag = GetIpfixL3Ipv4HashFieldSelect(V, u1_g4_fragInfoEn_f, &v4_hash_field);
                    field_sel->u.ipv4.vrfid = GetIpfixL3Ipv4HashFieldSelect(V, u1_g4_vrfIdEn_f, &v4_hash_field);
                    break;
            }
            switch(GetIpfixL3Ipv4HashFieldSelect(V, globalPortType_f, &v4_hash_field))
            {
                case 1:
                    field_sel->u.ipv4.gport = 1;
                    break;
                case 2:
                    field_sel->u.ipv4.logic_port = 1;
                    break;
                case 3:
                case 4:
                    field_sel->u.ipv4.metadata = 1;
                    break;
                default:
                    break;
            }
            break;

        case CTC_IPFIX_KEY_HASH_MPLS:
            cmd = DRV_IOR( IpfixL3MplsHashFieldSelect_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &mpls_hash_field));
            field_sel->u.mpls.profile_id = GetIpfixL3MplsHashFieldSelect(V, ipfixCfgProfileIdEn_f, &mpls_hash_field);
            field_sel->u.mpls.label_num = GetIpfixL3MplsHashFieldSelect(V,  labelNumEn_f, &mpls_hash_field);
            field_sel->u.mpls.mpls_label0_exp = GetIpfixL3MplsHashFieldSelect(V,  mplsExp0En_f, &mpls_hash_field);
            field_sel->u.mpls.mpls_label1_exp = GetIpfixL3MplsHashFieldSelect(V,  mplsExp1En_f      ,&mpls_hash_field);
            field_sel->u.mpls.mpls_label2_exp = GetIpfixL3MplsHashFieldSelect(V,  mplsExp2En_f      ,&mpls_hash_field);
            field_sel->u.mpls.mpls_label0_label = GetIpfixL3MplsHashFieldSelect(V, mplsLabel0En_f    , &mpls_hash_field);
            field_sel->u.mpls.mpls_label1_label = GetIpfixL3MplsHashFieldSelect(V, mplsLabel1En_f    , &mpls_hash_field);
            field_sel->u.mpls.mpls_label2_label = GetIpfixL3MplsHashFieldSelect(V, mplsLabel2En_f    , &mpls_hash_field);
            field_sel->u.mpls.mpls_label0_s = GetIpfixL3MplsHashFieldSelect(V, mplsSbit0En_f     , &mpls_hash_field);
            field_sel->u.mpls.mpls_label1_s = GetIpfixL3MplsHashFieldSelect(V, mplsSbit1En_f     , &mpls_hash_field);
            field_sel->u.mpls.mpls_label2_s = GetIpfixL3MplsHashFieldSelect(V, mplsSbit2En_f     , &mpls_hash_field);
            field_sel->u.mpls.mpls_label0_ttl = GetIpfixL3MplsHashFieldSelect(V, mplsTtl0En_f      , &mpls_hash_field);
            field_sel->u.mpls.mpls_label1_ttl = GetIpfixL3MplsHashFieldSelect(V, mplsTtl1En_f      , &mpls_hash_field);
            field_sel->u.mpls.mpls_label2_ttl = GetIpfixL3MplsHashFieldSelect(V, mplsTtl2En_f      , &mpls_hash_field);
            switch(GetIpfixL3MplsHashFieldSelect(V, globalPortType_f, &mpls_hash_field))
            {
                case 1:
                    field_sel->u.mpls.gport = 1;
                    break;
                case 2:
                    field_sel->u.mpls.logic_port = 1;
                    break;
                case 3:
                case 4:
                    field_sel->u.mpls.metadata = 1;
                    break;
                default:
                    break;
            }
            break;
        case CTC_IPFIX_KEY_HASH_IPV6:
            cmd = DRV_IOR( IpfixL3Ipv6HashFieldSelect_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &v6_hash_field));
            field_sel->u.ipv6.profile_id = GetIpfixL3Ipv6HashFieldSelect(V, ipfixCfgProfileIdEn_f, &v6_hash_field);
            field_sel->u.ipv6.dscp = GetIpfixL3Ipv6HashFieldSelect(V, dscpEn_f, &v6_hash_field);
            field_sel->u.ipv6.icmp_code = GetIpfixL3Ipv6HashFieldSelect(V, icmpOpcodeEn_f, &v6_hash_field);
            field_sel->u.ipv6.icmp_type = GetIpfixL3Ipv6HashFieldSelect(V, icmpTypeEn_f, &v6_hash_field);
            field_sel->u.ipv6.igmp_type = GetIpfixL3Ipv6HashFieldSelect(V, igmpTypeEn_f, &v6_hash_field);
            field_sel->u.ipv6.gre_key = GetIpfixL3Ipv6HashFieldSelect(V, greKeyEn_f, &v6_hash_field) ||
                                            GetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyGreKeyEn_f, &v6_hash_field);
            field_sel->u.ipv6.nvgre_key = GetIpfixL3Ipv6HashFieldSelect(V, nvgreVsidEn_f, &v6_hash_field) ||
                                            GetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyNvgreVsidEn_f, &v6_hash_field);
            field_sel->u.ipv6.aware_tunnel_info_en = GetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyMode_f, &v6_hash_field);
            field_sel->u.ipv6.ip_da = GetIpfixL3Ipv6HashFieldSelect(V, ipDaEn_f, &v6_hash_field);
            field_sel->u.ipv6.ip_da_mask = field_sel->u.ipv6.ip_da ? ((GetIpfixL3Ipv6HashFieldSelect(V, ipDaPrefixLength_f, &v6_hash_field)+1)<<2) : 0;
            field_sel->u.ipv6.ip_sa = GetIpfixL3Ipv6HashFieldSelect(V, ipSaEn_f, &v6_hash_field);
            field_sel->u.ipv6.ip_sa_mask = field_sel->u.ipv6.ip_sa ? ((GetIpfixL3Ipv6HashFieldSelect(V, ipSaPrefixLength_f, &v6_hash_field)+1)<<2) : 0;
            field_sel->u.ipv6.flow_label = GetIpfixL3Ipv6HashFieldSelect(V, g1_ipv6FlowLabelEn_f, &v6_hash_field);
            field_sel->u.ipv6.vxlan_vni = GetIpfixL3Ipv6HashFieldSelect(V, vxlanVniEn_f, &v6_hash_field) ||
                                            GetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyVxlanVniEn_f, &v6_hash_field);
            field_sel->u.ipv6.wlan_radio_mac = GetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyCapwapRadioMacEn_f, &v6_hash_field);
            field_sel->u.ipv6.wlan_radio_id = GetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyCapwapRidEn_f, &v6_hash_field);
            field_sel->u.ipv6.wlan_ctl_packet = GetIpfixL3Ipv6HashFieldSelect(V, g1_mergeKeyCapwapControlPktEn_f, &v6_hash_field);
            field_sel->u.ipv6.l4_dst_port = GetIpfixL3Ipv6HashFieldSelect(V, l4DestPortEn_f, &v6_hash_field);
            field_sel->u.ipv6.l4_src_port = GetIpfixL3Ipv6HashFieldSelect(V, l4SourcePortEn_f, &v6_hash_field);
            field_sel->u.ipv6.flow_label= GetIpfixL3Ipv6HashFieldSelect(V, g1_ipv6FlowLabelEn_f, &v6_hash_field);
            field_sel->u.ipv6.ttl = GetIpfixL3Ipv6HashFieldSelect(V, g1_ttlEn_f, &v6_hash_field);
            field_sel->u.ipv6.ecn = GetIpfixL3Ipv6HashFieldSelect(V, g1_ecnEn_f, &v6_hash_field);
            field_sel->u.ipv6.ip_frag = GetIpfixL3Ipv6HashFieldSelect(V, g1_fragInfoEn_f, &v6_hash_field);
            field_sel->u.ipv6.tcp_flags = GetIpfixL3Ipv6HashFieldSelect(V, g1_tcpFlagsEn_f, &v6_hash_field);
            field_sel->u.ipv6.src_cid = GetIpfixL3Ipv6HashFieldSelect(V, g1_srcCategoryIdEn_f, &v6_hash_field);
            field_sel->u.ipv6.dst_cid = GetIpfixL3Ipv6HashFieldSelect(V, g1_dstCategoryIdEn_f, &v6_hash_field);
            switch(GetIpfixL3Ipv6HashFieldSelect(V, globalPortType_f, &v6_hash_field))
            {
                case 1:
                    field_sel->u.ipv6.gport = 1;
                    break;
                case 2:
                    field_sel->u.ipv6.logic_port = 1;
                    break;
                case 3:
                case 4:
                    field_sel->u.ipv6.metadata = 1;
                    break;
                default:
                    break;
            }
            break;
        case CTC_IPFIX_KEY_HASH_L2_L3:
            cmd = DRV_IOR( IpfixL2L3HashFieldSelect_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, field_sel_id, cmd, &l2l3_hash_field));

            field_sel->u.l2_l3.stag_cfi= GetIpfixL2L3HashFieldSelect(V, cfiEn_f       , &l2l3_hash_field);
            field_sel->u.l2_l3.stag_cos= GetIpfixL2L3HashFieldSelect(V, cosEn_f       , &l2l3_hash_field);
            field_sel->u.l2_l3.stag_vlan= GetIpfixL2L3HashFieldSelect(V, vlanIdEn_f       , &l2l3_hash_field);
            field_sel->u.l2_l3.src_cid = GetIpfixL2L3HashFieldSelect(V, srcCategoryIdEn_f, &l2l3_hash_field);
            field_sel->u.l2_l3.dst_cid = GetIpfixL2L3HashFieldSelect(V, dstCategoryIdEn_f, &l2l3_hash_field);
            field_sel->u.l2_l3.profile_id = GetIpfixL2L3HashFieldSelect(V, ipfixCfgProfileIdEn_f, &l2l3_hash_field);
            field_sel->u.l2_l3.dscp= GetIpfixL2L3HashFieldSelect(V,  dscpEn_f          , &l2l3_hash_field);
            field_sel->u.l2_l3.ecn= GetIpfixL2L3HashFieldSelect(V,  ecnEn_f           , &l2l3_hash_field);
            field_sel->u.l2_l3.eth_type = GetIpfixL2L3HashFieldSelect(V,  etherTypeEn_f     , &l2l3_hash_field);
            field_sel->u.l2_l3.icmp_code= GetIpfixL2L3HashFieldSelect(V,  icmpOpcodeEn_f    , &l2l3_hash_field);
            field_sel->u.l2_l3.icmp_type= GetIpfixL2L3HashFieldSelect(V,  icmpTypeEn_f      , &l2l3_hash_field);
            field_sel->u.l2_l3.igmp_type= GetIpfixL2L3HashFieldSelect(V,  igmpTypeEn_f      , &l2l3_hash_field);
            field_sel->u.l2_l3.ip_da = GetIpfixL2L3HashFieldSelect(V,  ipDaEn_f, &l2l3_hash_field);
            field_sel->u.l2_l3.ip_da_mask = field_sel->u.l2_l3.ip_da ? (GetIpfixL2L3HashFieldSelect(V,  ipDaPrefixLength_f, &l2l3_hash_field)+1) : 0;
            field_sel->u.l2_l3.ip_sa = GetIpfixL2L3HashFieldSelect(V,  ipSaEn_f, &l2l3_hash_field);
            field_sel->u.l2_l3.ip_sa_mask = field_sel->u.l2_l3.ip_sa ? (GetIpfixL2L3HashFieldSelect(V,  ipSaPrefixLength_f, &l2l3_hash_field)+1) : 0;
            field_sel->u.l2_l3.l4_dst_port = GetIpfixL2L3HashFieldSelect(V,  l4DestPortEn_f, &l2l3_hash_field);
            field_sel->u.l2_l3.l4_src_port= GetIpfixL2L3HashFieldSelect(V,  l4SourcePortEn_f, &l2l3_hash_field);
            field_sel->u.l2_l3.label_num= GetIpfixL2L3HashFieldSelect(V,  labelNumEn_f, &l2l3_hash_field);
            field_sel->u.l2_l3.mac_da= GetIpfixL2L3HashFieldSelect(V,  macDaEn_f         , &l2l3_hash_field);
            field_sel->u.l2_l3.mac_sa= GetIpfixL2L3HashFieldSelect(V,  macSaEn_f         , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label0_exp= GetIpfixL2L3HashFieldSelect(V,  mplsExp0En_f      , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label1_exp= GetIpfixL2L3HashFieldSelect(V,  mplsExp1En_f      , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label2_exp= GetIpfixL2L3HashFieldSelect(V,  mplsExp2En_f      , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label0_label= GetIpfixL2L3HashFieldSelect(V, mplsLabel0En_f    , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label1_label= GetIpfixL2L3HashFieldSelect(V, mplsLabel1En_f    , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label2_label= GetIpfixL2L3HashFieldSelect(V, mplsLabel2En_f    , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label0_s= GetIpfixL2L3HashFieldSelect(V, mplsSbit0En_f     , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label1_s= GetIpfixL2L3HashFieldSelect(V, mplsSbit1En_f     , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label2_s= GetIpfixL2L3HashFieldSelect(V, mplsSbit2En_f     , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label0_ttl = GetIpfixL2L3HashFieldSelect(V, mplsTtl0En_f      , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label1_ttl= GetIpfixL2L3HashFieldSelect(V, mplsTtl1En_f      , &l2l3_hash_field);
            field_sel->u.l2_l3.mpls_label2_ttl= GetIpfixL2L3HashFieldSelect(V, mplsTtl2En_f      , &l2l3_hash_field);
            field_sel->u.l2_l3.ttl= GetIpfixL2L3HashFieldSelect(V, ttlEn_f           , &l2l3_hash_field);
            field_sel->u.l2_l3.aware_tunnel_info_en = GetIpfixL2L3HashFieldSelect(V, mergeKeyMode_f, &l2l3_hash_field);
            if(field_sel->u.l2_l3.aware_tunnel_info_en)
            {
                field_sel->u.l2_l3.vxlan_vni= GetIpfixL2L3HashFieldSelect(V,g1_mergeKeyVxlanVniEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.gre_key = GetIpfixL2L3HashFieldSelect(V,g1_mergeKeyGreKeyEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.nvgre_key = GetIpfixL2L3HashFieldSelect(V,g1_mergeKeyNvgreVsidEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.wlan_radio_mac = GetIpfixL2L3HashFieldSelect(V,g1_mergeKeyCapwapRadioMacEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.wlan_radio_id = GetIpfixL2L3HashFieldSelect(V,g1_mergeKeyCapwapRidEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.wlan_ctl_packet = GetIpfixL2L3HashFieldSelect(V,g1_mergeKeyCapwapControlPktEn_f, &l2l3_hash_field);
            }
            else
            {
                field_sel->u.l2_l3.vxlan_vni= GetIpfixL2L3HashFieldSelect(V,vxlanVniEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.gre_key = GetIpfixL2L3HashFieldSelect(V,greKeyEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.nvgre_key = GetIpfixL2L3HashFieldSelect(V,nvgreVsidEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.ip_protocol = GetIpfixL2L3HashFieldSelect(V,g2_layer3HeaderProtocolEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.vrfid = GetIpfixL2L3HashFieldSelect(V,g2_vrfIdEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.ip_identification = GetIpfixL2L3HashFieldSelect(V,g2_ipIdentificationEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.ip_frag = GetIpfixL2L3HashFieldSelect(V,g2_fragInfoEn_f, &l2l3_hash_field);
                field_sel->u.l2_l3.tcp_flags = GetIpfixL2L3HashFieldSelect(V,g2_tcpFlagsEn_f, &l2l3_hash_field);
            }

            switch(GetIpfixL2L3HashFieldSelect(V, globalPortType_f, &l2l3_hash_field))
            {
                case 1:
                    field_sel->u.l2_l3.gport = 1;
                    break;
                case 2:
                    field_sel->u.l2_l3.logic_port = 1;
                    break;
                case 3:
                case 4:
                    field_sel->u.l2_l3.metadata = 1;
                    break;
                default:
                    break;
            }
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

extern int32
sys_usw_ipfix_set_global_cfg(uint8 lchip, ctc_ipfix_global_cfg_t *ipfix_cfg)
{
    IpfixAgingTimerCtl_m aging_timer;
    IpeRouteCtl_m route_ctl;
    IpfixEngineCtl_m    ipfix_ctl;

    uint32 cmd = 0;
    uint64 tmp = 0;
    uint32 core_frequecy = 0;
    uint32 max_age_ms = 0;
    uint32 age_seconds = 0;
    uint32 tick_interval = 0;
    uint32 byte_cnt[2] = {0};
    uint32 pkt_cnt = 0;
    uint32 value_a = 0;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_IPFIX_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(ipfix_cfg);
    CTC_MAX_VALUE_CHECK(ipfix_cfg->unkown_pkt_dest_type, 1);
    CTC_MAX_VALUE_CHECK(ipfix_cfg->bytes_cnt, 0xFFFFFFFF);
    CTC_MAX_VALUE_CHECK(ipfix_cfg->pkt_cnt, 0x3FFFFFF);

    if (ipfix_cfg->pkt_cnt == 0)
    {
        pkt_cnt = 0;
    }
    else
    {
        pkt_cnt = (ipfix_cfg->pkt_cnt - 1);
    }

    sal_memset(&aging_timer, 0, sizeof(IpfixAgingTimerCtl_m));
    sal_memset(&ipfix_ctl, 0, sizeof(IpfixEngineCtl_m));
    sal_memset(&route_ctl, 0, sizeof(IpeRouteCtl_m));

    if(ipfix_cfg->sample_mode)
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    if (ipfix_cfg->times_interval > 0xfffff)
    {
        return CTC_E_INVALID_PARAM;
    }

    core_frequecy = sys_usw_get_core_freq(lchip, 0);
    max_age_ms = (CTC_MAX_UINT32_VALUE / ((core_frequecy * 1000 * 1000 / DOWN_FRE_RATE))) * (p_usw_ipfix_master[lchip]->max_ptr/4);
    if (max_age_ms < MCHIP_CAP(SYS_CAP_IPFIX_MIN_AGING_INTERVAL) || age_seconds > max_age_ms)
    {
        return CTC_E_INVALID_PARAM;
    }

    tmp = (((uint64)core_frequecy * 1000 *1000)/DOWN_FRE_RATE);
    tmp *= ipfix_cfg->aging_interval;
    tick_interval = tmp / (p_usw_ipfix_master[lchip]->max_ptr/4);
    SYS_IPFIX_LOCK;
    p_usw_ipfix_master[lchip]->aging_interval = ipfix_cfg->aging_interval;
    SYS_IPFIX_UNLOCK;
    if ((tick_interval == 0) && (ipfix_cfg->aging_interval !=0))
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"aging_interval is 0! \n");
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR( IpfixAgingTimerCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aging_timer));
    SetIpfixAgingTimerCtl(V, agingUpdEn_f, &aging_timer, (ipfix_cfg->aging_interval !=0));
    SetIpfixAgingTimerCtl(V, cpuAgingEn_f, &aging_timer, !!ipfix_cfg->sw_learning_en);

    if (ipfix_cfg->aging_interval)
    {
        SetIpfixAgingTimerCtl(V, agingInterval_f, &aging_timer, tick_interval);
    }

    cmd = DRV_IOW( IpfixAgingTimerCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aging_timer));
    cmd = DRV_IOW( IpfixAgingTimerCtl0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aging_timer));
    cmd = DRV_IOW( IpfixAgingTimerCtl1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &aging_timer));



    byte_cnt[0] = (ipfix_cfg->bytes_cnt) & 0xffffffff;
    cmd = DRV_IOR( IpfixEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_ctl));
    SetIpfixEngineCtl(A, byteCntWraparoundThre_f, &ipfix_ctl, byte_cnt);

    value_a = !!ipfix_cfg->conflict_export;
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_ipfixConflictPktLogEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_a));
    cmd = DRV_IOW(EpeHeaderEditCtl_t,EpeHeaderEditCtl_ipfixConflictPktLogEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value_a));

    SetIpfixEngineCtl(V, pktCntWraparoundThre_f , &ipfix_ctl, pkt_cnt);
    SetIpfixEngineCtl(V, ignorTcpClose_f , &ipfix_ctl, ((ipfix_cfg->tcp_end_detect_en)?0:1));
    SetIpfixEngineCtl(V, tsWraparoundThre_f , &ipfix_ctl, ipfix_cfg->times_interval);  /*granularity is 1ms*/
    SetIpfixEngineCtl(V, isCpuSetFlowKeyMode_f , &ipfix_ctl, ipfix_cfg->sw_learning_en);
    SetIpfixEngineCtl(V, unknownPktDestIsVlanId_f, &ipfix_ctl, ipfix_cfg->unkown_pkt_dest_type);
    SetIpfixEngineCtl(V, newFlowExportModeForIpfix_f  , &ipfix_ctl, !!ipfix_cfg->new_flow_export_en);
    if(ipfix_cfg->threshold)
    {
        SetIpfixEngineCtl(V, flowUseageIntEnable_f, &ipfix_ctl, 0x3);
    }
    else
    {
        SetIpfixEngineCtl(V, flowUseageIntEnable_f, &ipfix_ctl, 0);
    }
    SetIpfixEngineCtl(V, flowCountThreshold_f, &ipfix_ctl, ipfix_cfg->threshold);
    SetIpfixEngineCtl(V, forceEgrIpfixUseJitterMonitor_f, &ipfix_ctl, ipfix_cfg->latency_type);

    cmd = DRV_IOW( IpfixEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_ctl));
    cmd = DRV_IOW( IpfixEngineCtl0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_ctl));
    cmd = DRV_IOW( IpfixEngineCtl1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_ctl));

    if(0 == ipfix_cfg->conflict_cnt)
    {
        cmd = DRV_IOW( IpfixMissPktCounter_t, IpfixMissPktCounter_missPktCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_cfg->conflict_cnt));
        cmd = DRV_IOW( IpfixMissPktCounter0_t, IpfixMissPktCounter0_missPktCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_cfg->conflict_cnt));
        cmd = DRV_IOW( IpfixMissPktCounter1_t, IpfixMissPktCounter1_missPktCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_cfg->conflict_cnt));
    }
    return CTC_E_NONE;
}

extern int32
sys_usw_ipfix_get_global_cfg(uint8 lchip, ctc_ipfix_global_cfg_t *ipfix_cfg)
{
    IpfixAgingTimerCtl_m aging_timer;
    IpfixEngineCtl_m    ipfix_ctl;
    IpeRouteCtl_m route_ctl;
    IpeFwdCtl_m   ipe_fwd_ctl;

    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 core_frequecy = 0;
    uint32 max_age_ms = 0;
    uint32 age_seconds = 0;
    uint32 byte_cnt[2] = {0};

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_IPFIX_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(ipfix_cfg);

    sal_memset(&aging_timer, 0, sizeof(IpfixAgingTimerCtl_m));
    sal_memset(&ipfix_ctl, 0, sizeof(IpfixEngineCtl_m));
    sal_memset(&ipe_fwd_ctl,0,sizeof(ipe_fwd_ctl));

    core_frequecy = sys_usw_get_core_freq(lchip, 0);
    max_age_ms = (CTC_MAX_UINT32_VALUE / (core_frequecy * 1000 * 1000)) * p_usw_ipfix_master[lchip]->max_ptr;
    if (max_age_ms < MCHIP_CAP(SYS_CAP_IPFIX_MIN_AGING_INTERVAL) || age_seconds > max_age_ms)
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_IPFIX_LOCK;
    cmd = DRV_IOR( IpeRouteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &route_ctl), ret, error_pro);

    cmd = DRV_IOR( IpfixAgingTimerCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &aging_timer), ret, error_pro);

    ipfix_cfg->aging_interval = p_usw_ipfix_master[lchip]->aging_interval;
    cmd = DRV_IOR( IpfixEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ipfix_ctl), ret, error_pro);
    GetIpfixEngineCtl(A, byteCntWraparoundThre_f, &ipfix_ctl, byte_cnt);

    ipfix_cfg->bytes_cnt = 0;
    ipfix_cfg->bytes_cnt = byte_cnt[1];
    ipfix_cfg->bytes_cnt <<= 32;
    ipfix_cfg->bytes_cnt |= byte_cnt[0];
    ipfix_cfg->sw_learning_en = GetIpfixEngineCtl(V, isCpuSetFlowKeyMode_f , &ipfix_ctl);
    ipfix_cfg->pkt_cnt = GetIpfixEngineCtl(V, pktCntWraparoundThre_f , &ipfix_ctl)+1;
    ipfix_cfg->times_interval = GetIpfixEngineCtl(V, tsWraparoundThre_f, &ipfix_ctl);  /*1s*/
    ipfix_cfg->tcp_end_detect_en = !GetIpfixEngineCtl(V, ignorTcpClose_f, &ipfix_ctl);
    ipfix_cfg->new_flow_export_en = GetIpfixEngineCtl(V, newFlowExportModeForIpfix_f, &ipfix_ctl);
    ipfix_cfg->unkown_pkt_dest_type = GetIpfixEngineCtl(V, unknownPktDestIsVlanId_f, &ipfix_ctl);
    ipfix_cfg->threshold = GetIpfixEngineCtl(V, flowCountThreshold_f, &ipfix_ctl);
    ipfix_cfg->latency_type = GetIpfixEngineCtl(V, forceEgrIpfixUseJitterMonitor_f, &ipfix_ctl);

    cmd = DRV_IOR(IpeFwdCtl_t,DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl), ret, error_pro);

    ipfix_cfg->conflict_export = (uint8)GetIpeFwdCtl(V,ipfixConflictPktLogEn_f,&ipe_fwd_ctl);
    if(DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOR(IpfixMissPktCounter_t, IpfixMissPktCounter_missPktCount_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &(ipfix_cfg->conflict_cnt)), ret, error_pro);
    }
    else
    {
        cmd = DRV_IOR(IpfixMissPktCounter0_t, IpfixMissPktCounter0_missPktCount_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &(core_frequecy)), ret, error_pro);
        cmd = DRV_IOR(IpfixMissPktCounter1_t, IpfixMissPktCounter1_missPktCount_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &(ipfix_cfg->conflict_cnt)), ret, error_pro);
        ipfix_cfg->conflict_cnt += core_frequecy;
    }
error_pro:
    SYS_IPFIX_UNLOCK;
    return ret;
}
extern int32
sys_usw_ipfix_set_flow_cfg(uint8 lchip, ctc_ipfix_flow_cfg_t* p_flow_cfg)
{
    uint32  cmd = 0;
    uint32  sample_enable = 0;
    uint32  sample_profile_index = 0;
    uint8   flow_type = 0;
    uint32  step = 0;
    int32   ret;
    uint32  cfg_table_id;
    uint32  sample_cfg_table_id;

    DsIngressIpfixConfig_m  ipfix_cfg;
    IpfixIngSamplingProfile_m  ipfix_sampling_cfg;

    sys_ipfix_sample_profile_t* p_get_sample_prf;
    sys_ipfix_sample_profile_t new_sample_prf;
    sys_ipfix_sample_profile_t old_sample_prf;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_IPFIX_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_flow_cfg);

    sal_memset(&ipfix_cfg, 0, sizeof(ipfix_cfg));
    sal_memset(&ipfix_sampling_cfg, 0, sizeof(ipfix_sampling_cfg));
    sal_memset(&old_sample_prf, 0, sizeof(old_sample_prf));

    CTC_MAX_VALUE_CHECK(p_flow_cfg->dir, CTC_EGRESS);

    CTC_MAX_VALUE_CHECK(p_flow_cfg->sample_mode, 1);
    CTC_MAX_VALUE_CHECK(p_flow_cfg->sample_type, 1);
    CTC_MAX_VALUE_CHECK(p_flow_cfg->flow_type, CTC_IPFIX_FLOW_TYPE_MAX-1);


    if ((p_flow_cfg->sample_mode == 0) && (p_flow_cfg->sample_interval > 0x1fff))
    {
        return CTC_E_INVALID_PARAM;
    }
    if ((p_flow_cfg->sample_mode == 1) && (p_flow_cfg->sample_interval > 400))
    {
        return CTC_E_INVALID_PARAM;
    }

    if(p_flow_cfg->sample_interval == 1)
    {
        p_flow_cfg->sample_interval =0;
    }
    if (DRV_IS_DUET2(lchip))
    {
        if (p_flow_cfg->dir == CTC_INGRESS)
        {
            cfg_table_id = DsIngressIpfixConfig_t;
            sample_cfg_table_id = IpfixIngSamplingProfile_t;
        }
        else
        {
            cfg_table_id = DsEgressIpfixConfig_t;
            sample_cfg_table_id = IpfixEgrSamplingProfile_t;
        }
    }
    else
    {
        if (p_flow_cfg->dir == CTC_INGRESS)
        {
            cfg_table_id = DsIpfixConfig0_t;
            sample_cfg_table_id = IpfixSamplingProfile0_t;
        }
        else
        {
            cfg_table_id = DsIpfixConfig1_t;
            sample_cfg_table_id = IpfixSamplingProfile1_t;
        }
    }
    cmd = DRV_IOR( cfg_table_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_flow_cfg->profile_id, cmd, &ipfix_cfg));
    cmd = DRV_IOR( sample_cfg_table_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipfix_sampling_cfg));

    sample_enable = GetDsIngressIpfixConfig(V, samplingEn_f, &ipfix_cfg);
    sample_profile_index = GetDsIngressIpfixConfig(V, samplingProfileIndex_f, &ipfix_cfg);

    step = IpfixIngSamplingProfile_array_1_ingSamplingPktInterval_f - IpfixIngSamplingProfile_array_0_ingSamplingPktInterval_f;

    SYS_IPFIX_LOCK;
    if(sample_enable)
    {
        old_sample_prf.dir = p_flow_cfg->dir;
        old_sample_prf.mode = GetIpfixIngSamplingProfile(V, \
                                    array_0_ingSamplingMode_f+step*sample_profile_index, &ipfix_sampling_cfg);
        if(p_flow_cfg->dir == CTC_INGRESS)
        {
            old_sample_prf.interval = p_usw_ipfix_master[lchip]->sip_interval[sample_profile_index];
        }
        else
        {
            old_sample_prf.interval = p_usw_ipfix_master[lchip]->sip_interval[sample_profile_index+MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE)];
        }
        old_sample_prf.mode |= GetIpfixIngSamplingProfile(V, \
                                    array_0_ingCountBasedSamplingMode_f+step*sample_profile_index, &ipfix_sampling_cfg) << 1;
    }

    if(p_flow_cfg->sample_interval != 0) /*add or update*/
    {
        uint16 drv_interval = 0;
        new_sample_prf.dir = p_flow_cfg->dir;
        new_sample_prf.interval = p_flow_cfg->sample_interval;
        new_sample_prf.mode = (!!p_flow_cfg->sample_type << 1) | (!!p_flow_cfg->sample_mode);

        ret = ctc_spool_add(p_usw_ipfix_master[lchip]->sample_spool, &new_sample_prf, &old_sample_prf, &p_get_sample_prf);
        if(ret)
        {
            SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"add sample spool error, %%ret=%d\n", ret);
            SYS_IPFIX_UNLOCK;
            return ret;
        }
        if(p_flow_cfg->dir == CTC_INGRESS)
        {
            p_usw_ipfix_master[lchip]->sip_interval[p_get_sample_prf->ad_index] = p_flow_cfg->sample_interval;
        }
        else
        {
            p_usw_ipfix_master[lchip]->sip_interval[p_get_sample_prf->ad_index+MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE)] = p_flow_cfg->sample_interval;
        }
        /*write hw*/
        sample_profile_index = p_get_sample_prf->ad_index;
        _sys_usw_ipfix_sample_interval_convert(lchip, p_get_sample_prf, &drv_interval);
        SetIpfixIngSamplingProfile(V, array_0_ingSamplingPktInterval_f+step*sample_profile_index, &ipfix_sampling_cfg, drv_interval);
        SetIpfixIngSamplingProfile(V, array_0_ingSamplingMode_f+step*sample_profile_index, &ipfix_sampling_cfg, p_get_sample_prf->mode&0x01);
        SetIpfixIngSamplingProfile(V, array_0_ingCountBasedSamplingMode_f+step*sample_profile_index, &ipfix_sampling_cfg, (p_get_sample_prf->mode>>1)&0x01);

        SetDsIngressIpfixConfig(V, samplingProfileIndex_f, &ipfix_cfg, p_get_sample_prf->ad_index);
        SetDsIngressIpfixConfig(V, samplingEn_f, &ipfix_cfg, 1);
    }
    else if(sample_enable)
    {
        ret = ctc_spool_remove(p_usw_ipfix_master[lchip]->sample_spool, &old_sample_prf, &p_get_sample_prf);
        if(ret)
        {
            SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"remove sample spool error, %%ret=%d\n", ret);
            SYS_IPFIX_UNLOCK;
            return ret;
        }
        /*write hw*/
        SetDsIngressIpfixConfig(V, samplingProfileIndex_f, &ipfix_cfg, 0);
        SetDsIngressIpfixConfig(V, samplingEn_f, &ipfix_cfg, 0);
    }

    if(CTC_IPFIX_FLOW_TYPE_ALL_PACKET == p_flow_cfg->flow_type)
    {
        flow_type = 0;
    }
    else if(CTC_IPFIX_FLOW_TYPE_NON_DISCARD_PACKET == p_flow_cfg->flow_type)
    {
        flow_type = 1;
    }
    else if(CTC_IPFIX_FLOW_TYPE_DISCARD_PACKET == p_flow_cfg->flow_type)
    {
        flow_type = 2;
    }
    SetDsIngressIpfixConfig(V, flowType_f, &ipfix_cfg, flow_type);

    SetDsIngressIpfixConfig(V, sessionNumLimitEn_f, &ipfix_cfg, !!p_flow_cfg->learn_disable);
    SetDsIngressIpfixConfig(V, tcpSessionEndDetectDisable_f, &ipfix_cfg, !!p_flow_cfg->tcp_end_detect_disable);
    /*now only support dropped packet*/
    SetDsIngressIpfixConfig(V, ipfixAdMode_f, &ipfix_cfg, 1);
    SetDsIngressIpfixConfig(V, ipfixMirrorCount_f, &ipfix_cfg, p_flow_cfg->log_pkt_count);

    cmd = DRV_IOW(cfg_table_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, p_flow_cfg->profile_id, cmd, &ipfix_cfg), p_usw_ipfix_master[lchip]->p_mutex);
    cmd = DRV_IOW( sample_cfg_table_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipfix_sampling_cfg), p_usw_ipfix_master[lchip]->p_mutex);

    SYS_IPFIX_UNLOCK;
    return CTC_E_NONE;
}
extern int32
sys_usw_ipfix_get_flow_cfg(uint8 lchip, ctc_ipfix_flow_cfg_t* p_flow_cfg)
{
    uint32 cmd = 0;
    uint32  samping_idx = 0;
    uint8 flow_type = 0;
    uint8 step = 0;
    uint32  cfg_table_id;
    uint32  sample_cfg_table_id;
    IpfixIngSamplingProfile_m  ipfix_ing_sampleing_cfg;
    DsIngressIpfixConfig_m  ipfix_cfg;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_IPFIX_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_flow_cfg);
    CTC_MAX_VALUE_CHECK(p_flow_cfg->dir, CTC_EGRESS);

    SYS_IPFIX_LOCK;
    step = IpfixIngSamplingProfile_array_1_ingSamplingPktInterval_f - IpfixIngSamplingProfile_array_0_ingSamplingPktInterval_f;

    if (p_flow_cfg->dir == CTC_INGRESS)
    {

        if (DRV_IS_DUET2(lchip))
        {
            cfg_table_id = DsIngressIpfixConfig_t;
            sample_cfg_table_id = IpfixIngSamplingProfile_t;
        }
        else
        {
            cfg_table_id = DsIpfixConfig0_t;
            sample_cfg_table_id = IpfixSamplingProfile0_t;
        }
    }
    else
    {
        if (DRV_IS_DUET2(lchip))
        {
            cfg_table_id = DsEgressIpfixConfig_t;
            sample_cfg_table_id = IpfixEgrSamplingProfile_t;
        }
        else
        {

            cfg_table_id = DsIpfixConfig1_t;
            sample_cfg_table_id = IpfixSamplingProfile1_t;

        }
    }
    cmd = DRV_IOR( sample_cfg_table_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &ipfix_ing_sampleing_cfg), p_usw_ipfix_master[lchip]->p_mutex);
    cmd = DRV_IOR( cfg_table_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, p_flow_cfg->profile_id, cmd, &ipfix_cfg), p_usw_ipfix_master[lchip]->p_mutex);

    p_flow_cfg->tcp_end_detect_disable = GetDsIngressIpfixConfig(V, tcpSessionEndDetectDisable_f, &ipfix_cfg);
    p_flow_cfg->learn_disable = GetDsIngressIpfixConfig(V, sessionNumLimitEn_f, &ipfix_cfg);
    p_flow_cfg->log_pkt_count = GetDsIngressIpfixConfig(V, ipfixMirrorCount_f, &ipfix_cfg);
    flow_type = GetDsIngressIpfixConfig(V, flowType_f, &ipfix_cfg);
    if(0 == flow_type)
    {
        p_flow_cfg->flow_type = CTC_IPFIX_FLOW_TYPE_ALL_PACKET;
    }
    else if(1 == flow_type)
    {
        p_flow_cfg->flow_type = CTC_IPFIX_FLOW_TYPE_NON_DISCARD_PACKET;
    }
    else if(2 == flow_type)
    {
        p_flow_cfg->flow_type = CTC_IPFIX_FLOW_TYPE_DISCARD_PACKET;
    }


    if(GetDsIngressIpfixConfig(V, samplingEn_f, &ipfix_cfg))
    {
        samping_idx = GetDsIngressIpfixConfig(V, samplingProfileIndex_f, &ipfix_cfg);
        p_flow_cfg->sample_mode = GetIpfixIngSamplingProfile(V, array_0_ingSamplingMode_f + samping_idx*step, &ipfix_ing_sampleing_cfg);
        p_flow_cfg->sample_type = GetIpfixIngSamplingProfile(V, array_0_ingCountBasedSamplingMode_f+samping_idx*step, &ipfix_ing_sampleing_cfg);

        if(p_flow_cfg->dir == CTC_INGRESS)
        {
            p_flow_cfg->sample_interval = p_usw_ipfix_master[lchip]->sip_interval[samping_idx];
        }
        else
        {
            p_flow_cfg->sample_interval = p_usw_ipfix_master[lchip]->sip_interval[samping_idx+MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE)];
        }
    }

    SYS_IPFIX_UNLOCK;
    return CTC_E_NONE;
}
extern int32
sys_usw_ipfix_register_cb(uint8 lchip, ctc_ipfix_fn_t callback,void *userdata)
{
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_IPFIX_INIT_CHECK(lchip);

    SYS_IPFIX_LOCK;

    p_usw_ipfix_master[lchip]->ipfix_cb = callback;
    p_usw_ipfix_master[lchip]->user_data = userdata;

    SYS_IPFIX_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_ipfix_get_cb(uint8 lchip, void **cb, void** user_data)
{
    SYS_IPFIX_INIT_CHECK(lchip);

    *cb = p_usw_ipfix_master[lchip]->ipfix_cb;
    *user_data = p_usw_ipfix_master[lchip]->user_data;

    return CTC_E_NONE;
}

int32
sys_usw_ipfix_ftm_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    CTC_PTR_VALID_CHECK(specs_info);
    SYS_IPFIX_INIT_CHECK(lchip);

    specs_info->used_size = sys_usw_ipfix_get_flow_cnt(lchip, 0);

    return CTC_E_NONE;
}
uint32
sys_usw_ipfix_get_flow_cnt(uint8 lchip, uint32 detail)
{
    uint32 cmd = 0;
    uint32 cnt = 0;
    uint32 index = 0;
    DsIpfixL2HashKey_m l2_key;
    uint32 entry_num;

    SYS_IPFIX_INIT_CHECK(lchip);
    if (DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOR( IpfixFlowEntryCounter_t, IpfixFlowEntryCounter_flowEntryCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cnt));
    }
    else
    {
        uint32 egress_entry_cnt = 0;
        cmd = DRV_IOR( IpfixFlowEntryCounter0_t, IpfixFlowEntryCounter0_flowEntryCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cnt));

        cmd = DRV_IOR( IpfixFlowEntryCounter1_t, IpfixFlowEntryCounter1_flowEntryCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &egress_entry_cnt));

        cnt += egress_entry_cnt;
    }
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsIpfixL2HashKey_t, &entry_num));
    if (detail)
    {
        cmd = DRV_IOR( DsIpfixL2HashKey_t, DRV_ENTRY_FLAG);

        for (index = 0; index < entry_num; index++)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index*2, cmd, &l2_key));
            if (0 != GetDsIpfixL2HashKey(V, hashKeyType_f , &l2_key))
            {
                SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Entry %d exist! \n", index);
            }
        }
    }

    return cnt;
}

int32
sys_usw_ipfix_entry_usage_overflow(uint8 lchip, uint32 intr, void* p_data)
{
    uint32 entry_cnt = 0;

    entry_cnt = sys_usw_ipfix_get_flow_cnt(lchip, 0);

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Ipfix usage overflow!!! \n");
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Current entry cnt:%d \n", entry_cnt);

    return CTC_E_NONE;
}

int32
_sys_usw_ipfix_get_str(uint32 flag, char* p_str)
{
    if (flag&CTC_IPFIX_DATA_L2_MCAST_DETECTED)
    {
        sal_strcat((char*)p_str, "L2-Mcast ");
    }
    if (flag&CTC_IPFIX_DATA_L3_MCAST_DETECTED)
    {
        sal_strcat((char*)p_str, "L3-Mcast ");
    }
    if (flag&CTC_IPFIX_DATA_BCAST_DETECTED)
    {
        sal_strcat((char*)p_str, "Bcast ");
    }
    if (flag&CTC_IPFIX_DATA_APS_DETECTED)
    {
        sal_strcat((char*)p_str, "Aps ");
    }
    if (flag&CTC_IPFIX_DATA_ECMP_DETECTED)
    {
        sal_strcat((char*)p_str, "ECMP ");
    }
    if (flag&CTC_IPFIX_DATA_FRAGMENT_PKT_DETECTED)
    {
        sal_strcat((char*)p_str, "Frag ");
    }
    if (flag&CTC_IPFIX_DATA_DROP_DETECTED)
    {
        sal_strcat((char*)p_str, "Drop ");
    }
    if (flag&CTC_IPFIX_DATA_LINKAGG_DETECTED)
    {
        sal_strcat((char*)p_str, "Linkagg ");
    }
    if (flag&CTC_IPFIX_DATA_TTL_CHANGE)
    {
        sal_strcat((char*)p_str, "TTL-Change ");
    }
    if (flag&CTC_IPFIX_DATA_UNKNOW_PKT_DETECTED)
    {
        sal_strcat((char*)p_str, "Unkown-Pkt ");
    }
    if (flag&CTC_IPFIX_DATA_CVLAN_TAGGED)
    {
        sal_strcat((char*)p_str, "Cvlan-Valid ");
    }
    if (flag&CTC_IPFIX_DATA_SVLAN_TAGGED)
    {
        sal_strcat((char*)p_str, "Svlan-Valid ");
    }
    if (flag&CTC_IPFIX_DATA_SRC_CID_VALID)
    {
        sal_strcat((char*)p_str, "Src Cid Valid");
    }
    if (flag&CTC_IPFIX_DATA_DST_CID_VALID)
    {
        sal_strcat((char*)p_str, "Dest Cid Valid");
    }
    if (flag&CTC_IPFIX_DATA_MFP_VALID)
    {
        sal_strcat((char*)p_str, "MFP Valid");
    }
    return CTC_E_NONE;
}

void
_sys_usw_ipfix_process_isr(ctc_ipfix_data_t* info, void* userdata)
{
    char str[256] = {0};
    char* reason_str[9] = {"NO Export","Expired","Tcp Close", "PktCnt Overflow","ByteCnt Overflow","Ts Overflow","Hash Conflict","New Flow", "Dest Change",};
    uint32 tempip = 0;
    char buf[CTC_IPV6_ADDR_STR_LEN];
    uint32 ipv6_address[4] = {0, 0, 0, 0};
    uint8 index = 0;
    uint32 cmd = 0;
    IpfixEngineCtl_m    ipfix_ctl;
    uint8 lchip = (uintptr)userdata;

    if (NULL == info)
    {
        return;
    }
    sal_memset(&ipfix_ctl, 0, sizeof(IpfixEngineCtl_m));
    str[0] = '\0';

    _sys_usw_ipfix_get_str(info->flags, str);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"\n********Ipfix Export Information******** \n");

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: %s\n", "Dir",(info->dir)?"Egress":"Ingress");
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: %s\n", "Export_reason", reason_str[info->export_reason]);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Start_timestamp", (info->start_timestamp));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Last_timestamp", (info->last_timestamp));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%"PRIx64"\n", "Byte_count", (info->byte_count));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Pkt_count", (info->pkt_count));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Min_ttl", (info->min_ttl));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Max_ttl", (info->max_ttl));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Tcp flags(action)", (info->tcp_flags));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x(%s)\n", "Flag", (info->flags), str);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Port_type", (info->port_type));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Logic-port", (info->logic_port));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Gport", (info->gport));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Svlan", (info->svlan));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Svlan_prio", (info->svlan_prio));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Svlan_cfi", (info->svlan_cfi));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Cvlan", (info->cvlan));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Cvlan_prio", (info->cvlan_prio));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Cvlan_cfi", (info->cvlan_cfi));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Ether_type", (info->ether_type));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Source cid", (info->src_cid));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Dest cid", (info->dst_cid));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Profile ID", (info->profile_id));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Hash field sel ID", (info->field_sel_id));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s:%.4x.%.4x.%.4x \n", "Macsa",
        sal_ntohs(*(unsigned short*)&info->src_mac[0]),  sal_ntohs(*(unsigned short*)&info->src_mac[2]), sal_ntohs(*(unsigned short*)&info->src_mac[4]));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s:%.4x.%.4x.%.4x \n", "Macda",
        sal_ntohs(*(unsigned short*)&info->dst_mac[0]),  sal_ntohs(*(unsigned short*)&info->dst_mac[2]), sal_ntohs(*(unsigned short*)&info->dst_mac[4]));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Dest_gport", (info->dest_gport));

    if (CTC_FLAG_ISSET(info->flags, CTC_IPFIX_DATA_APS_DETECTED))
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Dest_group_id", (info->dest_group_id));
    }
    else
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Dest_group_id", (info->dest_group_id));
    }

    tempip = sal_ntohl(info->l3_info.ipv4.ipda);
    sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: %s\n", "Ipv4_da", buf);

    tempip = sal_ntohl(info->l3_info.ipv4.ipsa);
    sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: %s\n", "Ipv4_sa", buf);

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Dscp", ((info->key_type == CTC_IPFIX_KEY_HASH_IPV6)?(info->l3_info.ipv6.dscp):(info->l3_info.ipv4.dscp)));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Ecn", ((info->key_type == CTC_IPFIX_KEY_HASH_IPV6)?(info->l3_info.ipv6.ecn):(info->l3_info.ipv4.ecn)));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Ttl", ((info->key_type == CTC_IPFIX_KEY_HASH_IPV6)?(info->l3_info.ipv6.ttl):(info->l3_info.ipv4.ttl)));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Vrfid", (info->l3_info.vrfid));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Ip fragment", (info->l3_info.ip_frag));

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Ip packet length", (info->l3_info.ipv4.ip_pkt_len));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Ip identification", (info->l3_info.ipv4.ip_identification));
    ipv6_address[0] = sal_ntohl(info->l3_info.ipv6.ipda[0]);
    ipv6_address[1] = sal_ntohl(info->l3_info.ipv6.ipda[1]);
    ipv6_address[2] = sal_ntohl(info->l3_info.ipv6.ipda[2]);
    ipv6_address[3] = sal_ntohl(info->l3_info.ipv6.ipda[3]);

    sal_inet_ntop(AF_INET6, ipv6_address, buf, CTC_IPV6_ADDR_STR_LEN);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: %s\n", "Ipv6_da", buf);

    ipv6_address[0] = sal_ntohl(info->l3_info.ipv6.ipsa[0]);
    ipv6_address[1] = sal_ntohl(info->l3_info.ipv6.ipsa[1]);
    ipv6_address[2] = sal_ntohl(info->l3_info.ipv6.ipsa[2]);
    ipv6_address[3] = sal_ntohl(info->l3_info.ipv6.ipsa[3]);
    sal_inet_ntop(AF_INET6, ipv6_address, buf, CTC_IPV6_ADDR_STR_LEN);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: %s\n", "Ipv6_sa", buf);

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Flow_label", (info->l3_info.ipv6.flow_label));

    if(info->key_type == CTC_IPFIX_KEY_HASH_MPLS || info->key_type == CTC_IPFIX_KEY_HASH_L2_L3)
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Label_num", (info->l3_info.mpls.label_num));
        for (index = 0; index < CTC_IPFIX_LABEL_NUM; index++)
        {
            SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  Label%d_exp %-5s: 0x%x\n", index, "", (info->l3_info.mpls.label[index].exp));
            SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  Label%d_ttl %-5s: 0x%x\n", index, "", (info->l3_info.mpls.label[index].ttl));
            SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  Label%d_sbit %-4s: 0x%x\n", index, "", (info->l3_info.mpls.label[index].sbit));
            SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  Label%d_label %-3s: 0x%x\n", index, "", (info->l3_info.mpls.label[index].label));
        }
    }

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "IP_protocol", (info->l4_info.type.ip_protocol));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Aware_tunnel_info_en", (info->l4_info.aware_tunnel_info_en));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Gre_key", (info->l4_info.gre_key));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Vni", (info->l4_info.vni));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Icmpcode", (info->l4_info.icmp.icmpcode));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Icmp_type", (info->l4_info.icmp.icmp_type));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "L4_Source_port", (info->l4_info.l4_port.source_port));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "L4_Dest_port", (info->l4_info.l4_port.dest_port));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Igmp Type", (info->l4_info.igmp.igmp_type));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Tcp Flags(key)", (info->l4_info.tcp_flags));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s:%.4x.%.4x.%.4x \n", "Wlan Radio Mac",
        sal_ntohs(*(unsigned short*)&info->l4_info.wlan.radio_mac[0]),  sal_ntohs(*(unsigned short*)&info->l4_info.wlan.radio_mac[2]),
                                                                        sal_ntohs(*(unsigned short*)&info->l4_info.wlan.radio_mac[4]));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Wlan Radio Id:", (info->l4_info.wlan.radio_id));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT,"  %-16s: 0x%x\n", "Wlan control packet:", (info->l4_info.wlan.is_ctl_pkt));

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "  %-16s: 0x%x\n", "Min Latency:", (info->min_latency));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "  %-16s: 0x%x\n", "Max Latency:", (info->max_latency));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "  %-16s: 0x%x\n", "Min Jitter:", (info->min_jitter));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_EXPORT, "  %-16s: 0x%x\n", "Max Jitter:", (info->max_jitter));

    /* delete sw-learning entry form asic, when this entry is aging */
    cmd = DRV_IOR( IpfixEngineCtl_t, DRV_ENTRY_FLAG);
    (DRV_IOCTL(lchip, 0, cmd, &ipfix_ctl));
    if(GetIpfixEngineCtl(V, isCpuSetFlowKeyMode_f , &ipfix_ctl) && 1 == info->export_reason )
    {
        (sys_usw_ipfix_delete_entry_by_key(lchip, info));
    }

    return;
}

int32
sys_usw_ipfix_show_entry_info(ctc_ipfix_data_t* p_info, void* user_data)
{
    uint32 tbl_id = 0;
    uint16 detail = 0;
    uint32 entry_index = 0;

    char key_tbl_name[30] = {0};
    char ad_tbl_name[30] = {0};
    char* key_type[5] = {"Mac_key","L2l3_key","Ipv4_key", "Ipv6_key","Mpls_key"};
    char* dir[2] = {"Ingress", "Egress"};
    uint8 lchip = 0;

    SYS_IPFIX_INIT_CHECK(lchip);

    if(user_data != NULL)
    {
        entry_index = *(uint32*)user_data;
    }


    detail = entry_index >> 16;
    entry_index = entry_index&0x0000ffff;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-9x", entry_index);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-11s", key_type[p_info->key_type]);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%.4x %-1s", p_info->gport, "");
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d", p_info->field_sel_id);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9d", p_info->max_ttl);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-9d", p_info->min_ttl);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-9x", p_info->last_timestamp);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7s\n", dir[p_info->dir]);


    if (1 == detail)
    {
        tbl_id = SYS_IPFIX_GET_TAB_BY_TYPE(p_info->key_type);
        tbl_id = SYS_IPFIX_GET_TABLE(tbl_id, p_info->dir);
        drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, tbl_id, 0, key_tbl_name);

        tbl_id = DsIpfixSessionRecord_t;
        tbl_id = SYS_IPFIX_GET_TABLE(tbl_id, p_info->dir);
        drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, tbl_id, 0, ad_tbl_name);

        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Detail Entry Info\n");
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------\n");
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :0x%-8x\n", key_tbl_name, entry_index);
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-32s :0x%-8x\n", ad_tbl_name, entry_index);

    }

    return CTC_E_NONE;
}

int32
_sys_usw_ipfix_export_stats(uint8 lchip, uint8 exp_reason, uint64 pkt_cnt, uint64 byte_cnt)
{
    switch (exp_reason)
    {
        /* 1. Expired */
        case CTC_IPFIX_REASON_EXPIRED:
            p_usw_ipfix_master[lchip]->exp_cnt_stats[0] ++ ;
            p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats[0] += pkt_cnt;
            p_usw_ipfix_master[lchip]->exp_pkt_byte_stats[0] += byte_cnt;
            break;
        /* 2. Tcp Close */
        case CTC_IPFIX_REASON_TCP_CLOSE:
            p_usw_ipfix_master[lchip]->exp_cnt_stats[1] ++ ;
            p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats[1] += pkt_cnt;
            p_usw_ipfix_master[lchip]->exp_pkt_byte_stats[1] += byte_cnt;
            break;
        /* 3. PktCnt Overflow */
        case CTC_IPFIX_REASON_PACKET_CNT_OVERFLOW:
            p_usw_ipfix_master[lchip]->exp_cnt_stats[2] ++ ;
            p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats[2] += pkt_cnt;
            p_usw_ipfix_master[lchip]->exp_pkt_byte_stats[2] += byte_cnt;
            break;
        /* 4. ByteCnt Overflow */
        case CTC_IPFIX_REASON_BYTE_CNT_OVERFLOW:
            p_usw_ipfix_master[lchip]->exp_cnt_stats[3] ++ ;
            p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats[3] += pkt_cnt;
            p_usw_ipfix_master[lchip]->exp_pkt_byte_stats[3] += byte_cnt;
            break;
        /* 5.Ts Overflow */
        case CTC_IPFIX_REASON_LAST_TS_OVERFLOW:
            p_usw_ipfix_master[lchip]->exp_cnt_stats[4] ++ ;
            p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats[4] += pkt_cnt;
            p_usw_ipfix_master[lchip]->exp_pkt_byte_stats[4] += byte_cnt;
            break;
        /* 6. Conflict */
        case CTC_IPFIX_REASON_HASH_CONFLICT:
            p_usw_ipfix_master[lchip]->exp_cnt_stats[5] ++ ;
            p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats[5] += pkt_cnt;
            p_usw_ipfix_master[lchip]->exp_pkt_byte_stats[5] += byte_cnt;
            break;
        /* 7. New Flow */
        case CTC_IPFIX_REASON_NEW_FLOW_INSERT:
            p_usw_ipfix_master[lchip]->exp_cnt_stats[6] ++ ;
            p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats[6] += pkt_cnt;
            p_usw_ipfix_master[lchip]->exp_pkt_byte_stats[6] += byte_cnt;
            break;
        /* 8. Dest Change*/
        case CTC_IPFIX_REASON_DEST_CHANGE:
            p_usw_ipfix_master[lchip]->exp_cnt_stats[7] ++ ;
            p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats[7] += pkt_cnt;
            p_usw_ipfix_master[lchip]->exp_pkt_byte_stats[7] += byte_cnt;
            break;
        default:
            break;
    }

    return CTC_E_NONE;
}

#define __SYNC_DMA_INFO__
STATIC int32 __sys_usw_ipfix_unmapping_export_reason(uint8 sys_reason )
{
    switch(sys_reason)
    {
        case SYS_IPFIX_EXPORTREASON_EXPIRED:
            return CTC_IPFIX_REASON_EXPIRED;
        case SYS_IPFIX_EXPORTREASON_TCP_SESSION_CLOSE:
            return CTC_IPFIX_REASON_TCP_CLOSE;
        case SYS_IPFIX_EXPORTREASON_PACKET_COUNT_OVERFLOW:
            return CTC_IPFIX_REASON_PACKET_CNT_OVERFLOW;
        case SYS_IPFIX_EXPORTREASON_BYTE_COUNT_OVERFLOW:
            return CTC_IPFIX_REASON_BYTE_CNT_OVERFLOW;
        case SYS_IPFIX_EXPORTREASON_NEW_FLOW_EXPORT:
            return CTC_IPFIX_REASON_NEW_FLOW_INSERT;
        case SYS_IPFIX_EXPORTREASON_DESTINATION_CHANGE:
            return CTC_IPFIX_REASON_DEST_CHANGE;
        case SYS_IPFIX_EXPORTREASON_TS_COUNT_OVERFLOW:
            return CTC_IPFIX_REASON_LAST_TS_OVERFLOW;
        default:
            return SYS_IPFIX_EXPORTREASON_NO_EXPORT;
    }

}

STATIC int32 __sys_usw_ipfix_unmapping_key_type(uint8 sys_key_type )
{
    switch(sys_key_type)
    {
        case SYS_IPFIX_HASH_TYPE_L2:
            return CTC_IPFIX_KEY_HASH_MAC;
        case SYS_IPFIX_HASH_TYPE_L2L3:
            return CTC_IPFIX_KEY_HASH_L2_L3;
        case SYS_IPFIX_HASH_TYPE_IPV4:
            return CTC_IPFIX_KEY_HASH_IPV4;
        case SYS_IPFIX_HASH_TYPE_IPV6:
            return CTC_IPFIX_KEY_HASH_IPV6;
        case SYS_IPFIX_HASH_TYPE_MPLS:
            return CTC_IPFIX_KEY_HASH_MPLS;
        default:
            return CTC_IPFIX_KEY_NUM;
    }
}

STATIC int32
_sys_usw_ipfix_parser_ad_info(uint8 lchip, ctc_ipfix_data_t* p_user_info, void* p_info)
{
    uint8 dest_chip_id = 0;
    uint32 byte_cnt[2] = {0};
    uint32 dest_type = 0;
    uint32 sub_dest_type = 0;
    uint8 jitter_mode = 0;
    uint8 latency_mode = 0;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);


    dest_type = GetDmaToCpuIpfixAccL2KeyFifo(V, destinationType_f, p_info);

    if (!DRV_IS_DUET2(lchip) && p_user_info->dir == CTC_EGRESS)
    {
        jitter_mode = (dest_type == 4);
        latency_mode = (dest_type == 2);
        dest_type = SYS_IPFIX_UNICAST_DEST;
    }

    if (latency_mode == 1) /* latency*/
    {
        p_user_info->min_latency =  ((GetDmaToCpuIpfixAccL2KeyFifo(V, u1_gMfp_mfpToken_f , p_info) << 8) |
                                     GetDmaToCpuIpfixAccL2KeyFifo(V, minTtl_f , p_info));

        p_user_info->max_latency =  (((GetDmaToCpuIpfixAccL2KeyFifo(V, fragment_f , p_info) << 31)&0x1) |
                                  ((GetDmaToCpuIpfixAccL2KeyFifo(V, nonFragment_f , p_info) << 30)&0x1) |
                                  ((GetDmaToCpuIpfixAccL2KeyFifo(V, lastTs_f , p_info) << 8)&0x3FFFFF) |
                                   GetDmaToCpuIpfixAccL2KeyFifo(V, maxTtl_f , p_info));
    }
    else
    {
        if (GetDmaToCpuIpfixAccL2KeyFifo(V, fragment_f , p_info))
        {
            p_user_info->flags |= CTC_IPFIX_DATA_FRAGMENT_PKT_DETECTED;
        }

        p_user_info->start_timestamp = GetDmaToCpuIpfixAccL2KeyFifo(V, u1_gIpfix_firstTs_f , p_info);
        p_user_info->last_timestamp = GetDmaToCpuIpfixAccL2KeyFifo(V, u1_gIpfix_lastTs_f , p_info);

        if (jitter_mode == 1)
        {
            p_user_info->max_jitter =  (GetDmaToCpuIpfixAccL2KeyFifo(V, minTtl_f , p_info) << 8 |
                                         GetDmaToCpuIpfixAccL2KeyFifo(V, maxTtl_f , p_info));

            p_user_info->min_jitter = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gL2Mcast_l2McGroupId_f , p_info);
        }
        else
        {
            GetDmaToCpuIpfixAccL2KeyFifo(A, byteCount_f , p_info, byte_cnt);
            p_user_info->byte_count = 0;
            p_user_info->byte_count = byte_cnt[1];
            p_user_info->byte_count <<= 32;
            p_user_info->byte_count |= byte_cnt[0];

            p_user_info->max_ttl = GetDmaToCpuIpfixAccL2KeyFifo(V, maxTtl_f , p_info);
            p_user_info->min_ttl = GetDmaToCpuIpfixAccL2KeyFifo(V, minTtl_f , p_info);
        }
    }

    p_user_info->export_reason = __sys_usw_ipfix_unmapping_export_reason(GetDmaToCpuIpfixAccL2KeyFifo(V, exportReason_f , p_info));

    p_user_info->tcp_flags = GetDmaToCpuIpfixAccL2KeyFifo(V, tcpFlagsStatus_f , p_info);
    p_user_info->pkt_count = GetDmaToCpuIpfixAccL2KeyFifo(V, packetCount_f , p_info);


    if (GetDmaToCpuIpfixAccL2KeyFifo(V, u2_gAdMode1_droppedPacket_f  , p_info))
    {
        p_user_info->flags |= CTC_IPFIX_DATA_DROP_DETECTED;
    }

    switch(dest_type)
    {
        case SYS_IPFIX_UNICAST_DEST:
            dest_chip_id = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gUcastToNormal_destChipId_f , p_info);
            p_user_info->dest_gport = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gUcastToNormal_destId_f , p_info);
            p_user_info->dest_gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(dest_chip_id, p_user_info->dest_gport);
            break;

        case SYS_IPFIX_L2MC_DEST:
            p_user_info->dest_group_id = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gL2Mcast_l2McGroupId_f , p_info);
            p_user_info->flags |= CTC_IPFIX_DATA_L2_MCAST_DETECTED;
            break;
        case SYS_IPFIX_L3MC_DEST:
            p_user_info->dest_group_id = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gL3Mcast_l3McGroupId_f , p_info);
            p_user_info->flags |= CTC_IPFIX_DATA_L3_MCAST_DETECTED;
            break;
        case SYS_IPFIX_BCAST_DEST:
            p_user_info->dest_group_id = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gBcast_floodingId_f, p_info);
            p_user_info->flags |= CTC_IPFIX_DATA_BCAST_DETECTED;
            break;
        case SYS_IPFIX_UNKNOW_PKT_DEST:
            p_user_info->dest_group_id = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gUnknownPkt_floodingId_f, p_info);
            p_user_info->flags |= CTC_IPFIX_DATA_UNKNOW_PKT_DETECTED;
            break;

        case SYS_IPFIX_UNION_DEST:
            sub_dest_type = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gApsGroup_destType_f , p_info);
            switch(sub_dest_type)
            {
                case SYS_IPFIX_SUB_APS_DEST:
                    p_user_info->dest_group_id = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gApsGroup_apsGroupId_f , p_info);
                    p_user_info->flags |= CTC_IPFIX_DATA_APS_DETECTED;
                    break;
                case SYS_IPFIX_SUB_ECMP_DEST:
                    dest_chip_id = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gUcastToNormal_destChipId_f , p_info);
                    p_user_info->dest_gport = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gUcastToNormal_destId_f , p_info);
                    p_user_info->dest_gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(dest_chip_id, p_user_info->dest_gport);
                    p_user_info->flags |= CTC_IPFIX_DATA_ECMP_DETECTED;
                    break;
                case SYS_IPFIX_SUB_LINKAGG_DEST:
                    p_user_info->dest_group_id = GetDmaToCpuIpfixAccL2KeyFifo(V, uDestinationInfo_gUcastToLagg_linkaggGroupId_f , p_info);
                    p_user_info->flags |= CTC_IPFIX_DATA_LINKAGG_DETECTED;
                    break;
                default:
                    return CTC_E_INVALID_PARAM;
            }
            break;
        default:
            break;
    }
    if(GetDmaToCpuIpfixAccL2KeyFifo(V, u1Type_f , p_info) && GetDmaToCpuIpfixAccL2KeyFifo(V, u1_gMfp_mfpProfId_f , p_info))
    {
        p_user_info->flags |= CTC_IPFIX_DATA_MFP_VALID;
    }
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_ipfix_parser_l2_info(uint8 lchip, DmaToCpuIpfixAccL2KeyFifo_m* p_info, ctc_ipfix_data_t* user_info)
{
    uint32 cmd = 0;
    IpfixL2HashFieldSelect_m  hash_field;
    hw_mac_addr_t             mac_sa   = { 0 };
    hw_mac_addr_t             mac_da   = { 0 };
    uint8 use_cvlan = 0;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    user_info->key_type = CTC_IPFIX_KEY_HASH_MAC;
    user_info->field_sel_id = GetDmaToCpuIpfixAccL2KeyFifo(V, flowFieldSel_f,p_info);
    user_info->profile_id = GetDmaToCpuIpfixAccL2KeyFifo(V, ipfixCfgProfileId_f, p_info);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "hash_sel:%d  configure profile id: %d\n",user_info->field_sel_id,
                                    user_info->profile_id);
    CTC_MAX_VALUE_CHECK(user_info->field_sel_id, 15);

    cmd = DRV_IOR(IpfixL2HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, user_info->field_sel_id, cmd, &hash_field));

    switch(GetIpfixL2HashFieldSelect(V, globalPortType_f, &hash_field))
    {
        case 1:
            user_info->port_type = CTC_IPFIX_PORT_TYPE_GPORT;
            user_info->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDmaToCpuIpfixAccL2KeyFifo(V, globalPort_f, p_info));
            break;
        case 2:
            user_info->logic_port = GetDmaToCpuIpfixAccL2KeyFifo(V, globalPort_f, p_info);
            user_info->port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
            break;
        case 3:
        case 4:
            user_info->logic_port = GetDmaToCpuIpfixAccL2KeyFifo(V, globalPort_f, p_info);
            user_info->port_type = CTC_IPFIX_PORT_TYPE_METADATA;
            break;

        default:
            user_info->port_type = CTC_IPFIX_PORT_TYPE_NONE;
            break;
    }

    use_cvlan = GetDmaToCpuIpfixAccL2KeyFifo(V, flowL2KeyUseCvlan_f, p_info);
    if (use_cvlan)
    {
        user_info->cvlan_prio = GetDmaToCpuIpfixAccL2KeyFifo(V, vtagCos_f, p_info);
        user_info->cvlan = GetDmaToCpuIpfixAccL2KeyFifo(V, vlanId_f, p_info);
        user_info->cvlan_cfi = GetDmaToCpuIpfixAccL2KeyFifo(V, vtagCfi_f, p_info);
        if (GetDmaToCpuIpfixAccL2KeyFifo(V, vlanIdValid_f, p_info))
        {
            user_info->flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
        }
    }
    else
    {
        user_info->svlan_prio = GetDmaToCpuIpfixAccL2KeyFifo(V, vtagCos_f, p_info);
        user_info->svlan = GetDmaToCpuIpfixAccL2KeyFifo(V, vlanId_f, p_info);
        user_info->svlan_cfi = GetDmaToCpuIpfixAccL2KeyFifo(V, vtagCfi_f, p_info);
        if (GetDmaToCpuIpfixAccL2KeyFifo(V, vlanIdValid_f, p_info))
        {
            user_info->flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
        }
    }

    user_info->ether_type = GetDmaToCpuIpfixAccL2KeyFifo(V, etherType_f, p_info);
    GetDmaToCpuIpfixAccL2KeyFifo(A, macDa_f, p_info, mac_da);
    if(GetIpfixL2HashFieldSelect(V, macDaUseDstCategoryId_f , &hash_field))
    {
        user_info->dst_cid =  mac_da[0]&0xFF;
        if(mac_da[0] & (1<<8))
        {
            user_info->flags |= CTC_IPFIX_DATA_DST_CID_VALID;
        }
    }
    else
    {
        SYS_USW_SET_USER_MAC(user_info->dst_mac, mac_da);
    }

    GetDmaToCpuIpfixAccL2KeyFifo(A, macSa_f, p_info, mac_sa);
    if(GetIpfixL2HashFieldSelect(V, macSaUseSrcCategoryId_f, &hash_field))
    {
        user_info->src_cid = mac_sa[0]&0xFF;
        if(mac_sa[0] & (1<<8))
        {
            user_info->flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
        }
    }
    else
    {
        SYS_USW_SET_USER_MAC(user_info->src_mac, mac_sa);
    }

    if (GetDmaToCpuIpfixAccL2KeyFifo(V, isEgressKey_f, p_info))
    {
        user_info->dir = CTC_EGRESS;
    }
    else
    {
        user_info->dir = CTC_INGRESS;
    }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_ipfix_parser_l2l3_info(uint8 lchip, DmaToCpuIpfixAccL2L3KeyFifo_m* p_info, ctc_ipfix_data_t* user_info)
{
    hw_mac_addr_t             mac_sa   = { 0 };
    hw_mac_addr_t             mac_da   = { 0 };
    hw_mac_addr_t             radio_mac = { 0 };
    uint8 use_cvlan = 0;
    ctc_ipfix_hash_field_sel_t ctc_field_sel;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&ctc_field_sel, 0, sizeof(ctc_ipfix_hash_field_sel_t));

    user_info->key_type = CTC_IPFIX_KEY_HASH_L2_L3;
    user_info->field_sel_id = GetDmaToCpuIpfixAccL2L3KeyFifo(V, flowFieldSel_f, p_info);
    user_info->profile_id = GetDmaToCpuIpfixAccL2L3KeyFifo(V, ipfixCfgProfileId_f, p_info);
    CTC_ERROR_RETURN(sys_usw_ipfix_get_hash_field_sel(lchip, user_info->field_sel_id, CTC_IPFIX_KEY_HASH_L2_L3, &ctc_field_sel));

    if(ctc_field_sel.u.l2_l3.gport)
    {
        user_info->port_type = CTC_IPFIX_PORT_TYPE_GPORT;
        user_info->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDmaToCpuIpfixAccL2L3KeyFifo(V, globalPort_f, p_info));
    }
    else if(ctc_field_sel.u.l2_l3.logic_port)
    {
        user_info->logic_port = GetDmaToCpuIpfixAccL2L3KeyFifo(V, globalPort_f, p_info);
        user_info->port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }
    else if(ctc_field_sel.u.l2_l3.metadata)
    {
        user_info->logic_port = GetDmaToCpuIpfixAccL2L3KeyFifo(V, globalPort_f, p_info);
        user_info->port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }
    else
    {
        user_info->port_type = CTC_IPFIX_PORT_TYPE_NONE;
    }
    use_cvlan = GetDmaToCpuIpfixAccL2L3KeyFifo(V, flowL2KeyUseCvlan_f,p_info);
    if(use_cvlan)
    {
        if (GetDmaToCpuIpfixAccL2L3KeyFifo(V, vlanIdValid_f , p_info))
        {
            user_info->cvlan = GetDmaToCpuIpfixAccL2L3KeyFifo(V, vlanId_f, p_info);
            user_info->flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
        }
        user_info->cvlan_prio = GetDmaToCpuIpfixAccL2L3KeyFifo(V, vtagCos_f, p_info);
        user_info->cvlan_cfi = GetDmaToCpuIpfixAccL2L3KeyFifo(V, vtagCfi_f, p_info);
    }
    else
    {
        if (GetDmaToCpuIpfixAccL2L3KeyFifo(V, vlanIdValid_f , p_info))
        {
            user_info->svlan = GetDmaToCpuIpfixAccL2L3KeyFifo(V, vlanId_f, p_info);
            user_info->flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
        }
        user_info->svlan_prio = GetDmaToCpuIpfixAccL2L3KeyFifo(V, vtagCos_f, p_info);
        user_info->svlan_cfi = GetDmaToCpuIpfixAccL2L3KeyFifo(V, vtagCfi_f, p_info);
    }

    user_info->ether_type = GetDmaToCpuIpfixAccL2L3KeyFifo(V, etherType_f, p_info);
    GetDmaToCpuIpfixAccL2L3KeyFifo(A, macSa_f, p_info, mac_sa);
    GetDmaToCpuIpfixAccL2L3KeyFifo(A, macDa_f, p_info, mac_da);
    SYS_USW_SET_USER_MAC(user_info->src_mac, mac_sa);
    SYS_USW_SET_USER_MAC(user_info->dst_mac, mac_da);

    if (GetDmaToCpuIpfixAccL2L3KeyFifo(V, isEgressKey_f, p_info))
    {
        user_info->dir = CTC_EGRESS;
    }
    else
    {
        user_info->dir = CTC_INGRESS;
    }

    if(ctc_field_sel.u.l2_l3.dst_cid)
    {
        user_info->dst_cid = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_ipDa_f , p_info)&0xFF;
        if(GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_ipDa_f , p_info)&(1<<8))
        {
            user_info->flags |= CTC_IPFIX_DATA_DST_CID_VALID;
        }
    }
    else if(ctc_field_sel.u.l2_l3.ip_da)
    {
        user_info->l3_info.ipv4.ipda = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_ipDa_f , p_info);
        user_info->l3_info.ipv4.ipda_masklen = ctc_field_sel.u.l2_l3.ip_da_mask;
    }

    if(ctc_field_sel.u.l2_l3.src_cid)
    {
        user_info->src_cid = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_ipSa_f , p_info)&0xFF;
        if(GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_ipSa_f , p_info)&(1<<8))
        {
            user_info->flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
        }
    }
    else if(ctc_field_sel.u.l2_l3.ip_sa)
    {
        user_info->l3_info.ipv4.ipsa = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_ipSa_f , p_info);
        user_info->l3_info.ipv4.ipsa_masklen = ctc_field_sel.u.l2_l3.ip_sa_mask;
    }

    if(ctc_field_sel.u.l2_l3.ecn)
    {
        user_info->l3_info.ipv4.ecn = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_ecn_f , p_info);
    }
    if(ctc_field_sel.u.l2_l3.dscp)
    {
        user_info->l3_info.ipv4.dscp = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_dscp_f , p_info);
    }
    if(ctc_field_sel.u.l2_l3.ttl)
    {
        user_info->l3_info.ipv4.ttl = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_ttl_f , p_info);
    }
    if(ctc_field_sel.u.l2_l3.label_num)
    {
        user_info->l3_info.mpls.label_num = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_labelNum_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label0_exp)
    {
        user_info->l3_info.mpls.label[0].exp = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsExp0_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label0_label)
    {
        user_info->l3_info.mpls.label[0].label = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsLabel0_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label0_s)
    {
        user_info->l3_info.mpls.label[0].sbit = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsSbit0_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label0_ttl)
    {
        user_info->l3_info.mpls.label[0].ttl = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsTtl0_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label1_ttl)
    {
        user_info->l3_info.mpls.label[1].exp = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsExp1_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label1_label)
    {
        user_info->l3_info.mpls.label[1].label = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsLabel1_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label1_s)
    {
        user_info->l3_info.mpls.label[1].sbit = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsSbit1_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label1_ttl)
    {
        user_info->l3_info.mpls.label[1].ttl = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsTtl1_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label2_exp)
    {
        user_info->l3_info.mpls.label[2].exp = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsExp2_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label2_label)
    {
        user_info->l3_info.mpls.label[2].label = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsLabel2_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label2_s)
    {
        user_info->l3_info.mpls.label[2].sbit = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsSbit2_f, p_info);
    }
    if(ctc_field_sel.u.l2_l3.mpls_label2_ttl)
    {
        user_info->l3_info.mpls.label[2].ttl = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gMpls_mplsTtl2_f, p_info);
    }

    /* process l4 information */
    user_info->l4_info.aware_tunnel_info_en = GetDmaToCpuIpfixAccL2L3KeyFifo(V, isMergeKey_f, p_info);
    if (ctc_field_sel.u.l2_l3.l4_dst_port)
    {
        user_info->l4_info.l4_port.dest_port = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL4_gPort_l4DestPort_f, p_info);
    }
    if (ctc_field_sel.u.l2_l3.l4_src_port)
    {
        user_info->l4_info.l4_port.source_port = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info);
    }
    if (ctc_field_sel.u.l2_l3.icmp_code)
    {
       user_info->l4_info.icmp.icmpcode = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info)&0xff;
    }
    if (ctc_field_sel.u.l2_l3.icmp_type)
    {
       user_info->l4_info.icmp.icmp_type = (GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info)>>8)&0xff;
    }

    if(ctc_field_sel.u.l2_l3.igmp_type)
    {
        user_info->l4_info.igmp.igmp_type = (GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info)>>8)&0xff;
    }
    if (user_info->l4_info.aware_tunnel_info_en)
    {
        switch(GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_mergeDataType_f, p_info))
        {
            case 1:
                user_info->l4_info.vni = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_mergeData_f, p_info);
                break;
            case 2:
                user_info->l4_info.gre_key = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_mergeData_f, p_info);
                if (ctc_field_sel.u.l2_l3.gre_key)
                {
                    user_info->l4_info.gre_key = user_info->l4_info.gre_key | (GetDmaToCpuIpfixAccL2L3KeyFifo(V, mergeDataExt3_f, p_info) << 24);
                }
                break;
            case 3:
                user_info->l4_info.wlan.radio_id = GetDmaToCpuIpfixAccL2L3KeyFifo(V, mergeDataExt4_f, p_info)&0x1F;
                radio_mac[0] = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL3_gIpv4_mergeData_f,p_info) & 0xFFFFFF;
                radio_mac[0] = radio_mac[0] | ((GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL4_gPort_tcpFlags_f,p_info)&0xFF) << 24);
                radio_mac[1] = GetDmaToCpuIpfixAccL2L3KeyFifo(V, layer3HeaderProtocol_f,p_info)&0xFF;
                radio_mac[1] = radio_mac[1] | ((GetDmaToCpuIpfixAccL2L3KeyFifo(V, mergeDataExt3_f,p_info)&0xFF) << 8);
                SYS_USW_SET_USER_MAC(user_info->l4_info.wlan.radio_mac, radio_mac);
                user_info->l4_info.wlan.is_ctl_pkt = (GetDmaToCpuIpfixAccL2L3KeyFifo(V, mergeDataExt4_f, p_info)>> 5) & 0x01;
                break;
            default:
                break;
        }
    }
    else
    {
        if(ctc_field_sel.u.l2_l3.ip_protocol)
        {
            user_info->l4_info.type.ip_protocol = GetDmaToCpuIpfixAccL2L3KeyFifo(V, layer3HeaderProtocol_f,p_info);
        }
        if(ctc_field_sel.u.l2_l3.gre_key || ctc_field_sel.u.l2_l3.nvgre_key)
        {
            user_info->l4_info.gre_key = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL4_gKey_greKey_f, p_info);
        }
        if (ctc_field_sel.u.l2_l3.vxlan_vni)
        {
            user_info->l4_info.vni = GetDmaToCpuIpfixAccL2L3KeyFifo(V, uL4_gVxlan_vni_f, p_info);
        }
        if (ctc_field_sel.u.l2_l3.tcp_flags)
        {
            user_info->l4_info.tcp_flags =  GetDmaToCpuIpfixAccL2L3KeyFifo(V,uL4_gPort_tcpFlags_f, p_info);
        }
        if (ctc_field_sel.u.l2_l3.ip_frag)
        {
            user_info->l3_info.ip_frag =  SYS_IPFIX_UNMAP_IP_FRAG(GetDmaToCpuIpfixAccL2L3KeyFifo(V,uL3_gIpv4_mergeDataType_f, p_info));
        }
        if (ctc_field_sel.u.l2_l3.ip_identification)
        {
            user_info->l3_info.ipv4.ip_identification =  GetDmaToCpuIpfixAccL2L3KeyFifo(V,uL3_gIpv4_mergeData_f, p_info);
        }
        if (ctc_field_sel.u.l2_l3.vrfid)
        {
            uint16 vrfid =  GetDmaToCpuIpfixAccL2L3KeyFifo(V, mergeDataExt3_f, p_info);
            vrfid |= (GetDmaToCpuIpfixAccL2L3KeyFifo(V, mergeDataExt4_f, p_info)&0x3F << 8);
            user_info->l3_info.vrfid =  vrfid;
        }

    }
    return CTC_E_NONE;

}

STATIC int32
_sys_usw_ipfix_parser_l3_ipv4_info(uint8 lchip, DmaToCpuIpfixAccL3Ipv4KeyFifo_m* p_info, ctc_ipfix_data_t* user_info)
{
    uint32 cmd = 0;
    ctc_ipfix_hash_field_sel_t ctc_field_sel;
    IpfixL3Ipv4HashFieldSelect_m  hash_field;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&ctc_field_sel, 0, sizeof(ctc_ipfix_hash_field_sel_t));

    user_info->key_type = CTC_IPFIX_KEY_HASH_IPV4;
    user_info->field_sel_id = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, flowFieldSel_f, p_info);
    user_info->profile_id = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, ipfixCfgProfileId_f, p_info);

    CTC_ERROR_RETURN(sys_usw_ipfix_get_hash_field_sel(lchip, user_info->field_sel_id, CTC_IPFIX_KEY_HASH_IPV4, &ctc_field_sel));

    if(ctc_field_sel.u.ipv4.gport)
    {
        user_info->port_type = CTC_IPFIX_PORT_TYPE_GPORT;
        user_info->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, globalPort_f, p_info));
    }
    else if(ctc_field_sel.u.ipv4.logic_port)
    {
        user_info->logic_port = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, globalPort_f, p_info);
        user_info->port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }
    else if(ctc_field_sel.u.ipv4.metadata)
    {
        user_info->logic_port = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, globalPort_f, p_info);
        user_info->port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }
    else
    {
        user_info->port_type = CTC_IPFIX_PORT_TYPE_NONE;
    }

    if (GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, isEgressKey_f, p_info))
    {
        user_info->dir = CTC_EGRESS;
    }
    else
    {
        user_info->dir = CTC_INGRESS;
    }

    /*ipv4 key*/
    if(ctc_field_sel.u.ipv4.dst_cid)
    {
        user_info->dst_cid = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uIpDa_g2_destCategoryId_f , p_info);
        if(GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uIpDa_g2_destCategoryIdClassfied_f , p_info))
        {
            user_info->flags |= CTC_IPFIX_DATA_DST_CID_VALID;
        }
    }
    else
    {
        user_info->l3_info.ipv4.ipda = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uIpDa_g1_ipDa_f , p_info);
        user_info->l3_info.ipv4.ipda_masklen = ctc_field_sel.u.ipv4.ip_da_mask;
    }

    if(ctc_field_sel.u.ipv4.src_cid)
    {
        user_info->src_cid = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uIpSa_g2_srcCategoryId_f , p_info);
        if(GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uIpSa_g2_srcCategoryIdClassfied_f , p_info))
        {
            user_info->flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
        }
    }
    else
    {
        user_info->l3_info.ipv4.ipsa = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uIpSa_g1_ipSa_f , p_info);
        user_info->l3_info.ipv4.ipsa_masklen = ctc_field_sel.u.ipv4.ip_sa_mask;

    }
    user_info->l3_info.ipv4.ttl = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, ttl_f, p_info);
    user_info->l3_info.ipv4.dscp = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, dscp_f, p_info);
    user_info->l3_info.ipv4.ecn = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, ecn_f, p_info);
    /* process l4 information */
    user_info->l4_info.type.ip_protocol = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, layer3HeaderProtocol_f, p_info);

    if (ctc_field_sel.u.ipv4.vxlan_vni)
    {
        user_info->l4_info.vni = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uL4_gVxlan_vni_f, p_info);
    }
    else if(ctc_field_sel.u.ipv4.gre_key || ctc_field_sel.u.ipv4.nvgre_key)
    {
        user_info->l4_info.gre_key = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uL4_gKey_greKey_f, p_info);
    }
    else if (ctc_field_sel.u.ipv4.icmp_code || ctc_field_sel.u.ipv4.icmp_type)
    {
       user_info->l4_info.icmp.icmpcode = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info)&0xff;
       user_info->l4_info.icmp.icmp_type = (GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info)>>8)&0xff;
    }
    else if(ctc_field_sel.u.ipv4.igmp_type)
    {
        user_info->l4_info.igmp.igmp_type = (GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info)>>8)&0xff;
    }
    else if(ctc_field_sel.u.ipv4.l4_dst_port || ctc_field_sel.u.ipv4.l4_src_port)
    {
        user_info->l4_info.l4_port.dest_port = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uL4_gPort_l4DestPort_f, p_info);
        user_info->l4_info.l4_port.source_port = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info);
    }
    /*share field*/
    cmd = DRV_IOR(IpfixL3Ipv4HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, user_info->field_sel_id, cmd, &hash_field));
    switch(GetIpfixL3Ipv4HashFieldSelect(V, shareFieldMode_f, &hash_field))
    {
        case 0:
            user_info->l3_info.ip_frag = SYS_IPFIX_UNMAP_IP_FRAG(GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uShareField_g1_fragInfo_f,p_info));
            user_info->l4_info.tcp_flags = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uShareField_g1_tcpFlags_f,p_info);
            break;
        case 1:
            user_info->l3_info.ipv4.ip_identification = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uShareField_g2_ipIdentification_f,p_info);
            break;
        case 2:
            user_info->l3_info.ip_frag = SYS_IPFIX_UNMAP_IP_FRAG(GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uShareField_g3_fragInfo_f,p_info));
            user_info->l3_info.ipv4.ip_pkt_len = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uShareField_g3_ipLength_f,p_info);
            break;
        default:
            user_info->l3_info.ip_frag = SYS_IPFIX_UNMAP_IP_FRAG(GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uShareField_g4_fragInfo_f,p_info));
            user_info->l3_info.vrfid = GetDmaToCpuIpfixAccL3Ipv4KeyFifo(V, uShareField_g4_vrfId_f,p_info);
            break;
    }
    return CTC_E_NONE;

}

STATIC int32
_sys_usw_ipfix_parser_l3_ipv6_info(uint8 lchip, DmaToCpuIpfixAccL3Ipv6KeyFifo_m* p_info, ctc_ipfix_data_t* user_info)
{
    uint32 cmd = 0;
    IpfixL3Ipv6HashFieldSelect_m  hash_field;
    ctc_ipfix_hash_field_sel_t field_sel;  /*used for get ip mask*/
    uint32 ipv6_address[4] = {0, 0, 0, 0};
    uint32 merge_data[2] = {0};

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&field_sel, 0, sizeof(ctc_ipfix_hash_field_sel_t));

    user_info->key_type = CTC_IPFIX_KEY_HASH_IPV6;
    user_info->field_sel_id = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, flowFieldSel_f, p_info);
    user_info->profile_id = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, ipfixCfgProfileId_f, p_info);
    cmd = DRV_IOR(IpfixL3Ipv6HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, user_info->field_sel_id, cmd, &hash_field));

    CTC_ERROR_RETURN(sys_usw_ipfix_get_hash_field_sel(lchip, user_info->field_sel_id, CTC_IPFIX_KEY_HASH_IPV6, &field_sel));

    switch(GetIpfixL3Ipv6HashFieldSelect(V, globalPortType_f, &hash_field))
    {
        case 1:
            user_info->port_type = CTC_IPFIX_PORT_TYPE_GPORT;
            user_info->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, globalPort_f, p_info));
            break;
        case 2:
            user_info->logic_port = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, globalPort_f, p_info);
            user_info->port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
            break;
        case 3:
        case 4:
            user_info->logic_port = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, globalPort_f, p_info);
            user_info->port_type = CTC_IPFIX_PORT_TYPE_METADATA;
            break;
        default:
            user_info->port_type = CTC_IPFIX_PORT_TYPE_NONE;
            break;
    }

    if (GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, isEgressKey_f, p_info))
    {
        user_info->dir = CTC_EGRESS;
    }
    else
    {
        user_info->dir = CTC_INGRESS;
    }
    user_info->l3_info.ipv6.dscp = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, dscp_f, p_info);

    /* parser l4 info */
    user_info->l4_info.type.ip_protocol = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, layer3HeaderProtocol_f, p_info);
    user_info->l4_info.aware_tunnel_info_en = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, isMergeKey_f, p_info);

    if (field_sel.u.ipv6.icmp_code || field_sel.u.ipv6.icmp_type)
    {
        user_info->l4_info.icmp.icmpcode = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info)&0xff;
        user_info->l4_info.icmp.icmp_type = (GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info) >> 8)&0xff;
    }
    else if(field_sel.u.ipv6.igmp_type)
    {
        user_info->l4_info.igmp.igmp_type = (GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info) >> 8)&0xff;
    }
    else if(field_sel.u.ipv6.l4_dst_port || field_sel.u.ipv6.l4_src_port)
    {
        user_info->l4_info.l4_port.dest_port = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uL4_gPort_l4DestPort_f, p_info);
        user_info->l4_info.l4_port.source_port = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uL4_gPort_l4SourcePort_f, p_info);
    }

    if(!user_info->l4_info.aware_tunnel_info_en)
    {
        if(field_sel.u.ipv6.gre_key || field_sel.u.ipv6.nvgre_key)
        {
            user_info->l4_info.gre_key = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uL4_gKey_greKey_f, p_info);
        }
        else if(field_sel.u.ipv6.vxlan_vni)
        {
            user_info->l4_info.vni = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uL4_gVxlan_vni_f, p_info);
        }
    }

    if(!GetIpfixL3Ipv6HashFieldSelect(V, ipAddressMode_f, &hash_field))
    {
        GetDmaToCpuIpfixAccL3Ipv6KeyFifo(A, uIpSa_g1_ipSa_f , p_info,  ipv6_address);
        user_info->l3_info.ipv6.ipsa[0] = ipv6_address[3];
        user_info->l3_info.ipv6.ipsa[1] = ipv6_address[2];
        user_info->l3_info.ipv6.ipsa[2] = ipv6_address[1];
        user_info->l3_info.ipv6.ipsa[3] = ipv6_address[0];

        GetDmaToCpuIpfixAccL3Ipv6KeyFifo(A, uIpDa_g1_ipDa_f , p_info, ipv6_address);
        user_info->l3_info.ipv6.ipda[0] = ipv6_address[3];
        user_info->l3_info.ipv6.ipda[1] = ipv6_address[2];
        user_info->l3_info.ipv6.ipda[2] = ipv6_address[1];
        user_info->l3_info.ipv6.ipda[3] = ipv6_address[0];

        user_info->l3_info.ipv6.ipda_masklen = field_sel.u.ipv6.ip_da_mask;
        user_info->l3_info.ipv6.ipsa_masklen = field_sel.u.ipv6.ip_sa_mask;
    }
    else
    {
        if(field_sel.u.ipv6.src_cid)
        {
            user_info->src_cid = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uIpSa_g2_srcCategoryId_f, p_info);
            if(GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uIpSa_g2_srcCategoryIdClassfied_f, p_info))
            {
                user_info->flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
            }
        }
        if(field_sel.u.ipv6.dst_cid)
        {
            user_info->dst_cid = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uIpSa_g2_destCategoryId_f, p_info);
            if(GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uIpSa_g2_destCategoryIdClassfied_f, p_info))
            {
                user_info->flags |= CTC_IPFIX_DATA_DST_CID_VALID;
            }
        }
        if(field_sel.u.ipv6.ip_frag)
        {
            user_info->l3_info.ip_frag = SYS_IPFIX_UNMAP_IP_FRAG(GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uIpSa_g2_fragInfo_f, p_info));
        }
        if(field_sel.u.ipv6.ecn)
        {
            user_info->l3_info.ipv6.ecn = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uIpSa_g2_ecn_f, p_info);
        }
        if(field_sel.u.ipv6.ttl)
        {
            user_info->l3_info.ipv6.ttl = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uIpSa_g2_ttl_f, p_info);
        }
        if(field_sel.u.ipv6.flow_label)
        {
            user_info->l3_info.ipv6.flow_label = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uIpSa_g2_ipv6FlowLabel_f, p_info);
        }
        if(field_sel.u.ipv6.tcp_flags)
        {
            user_info->l4_info.tcp_flags = GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V, uIpSa_g2_tcpFlags_f, p_info);
        }

        if (user_info->l4_info.aware_tunnel_info_en)
        {
            GetDmaToCpuIpfixAccL3Ipv6KeyFifo(A, uIpDa_g2_mergeData_f, p_info,merge_data);
            switch(GetDmaToCpuIpfixAccL3Ipv6KeyFifo(V,uIpDa_g2_mergeDataType_f,p_info))
            {
                case 1:
                    user_info->l4_info.vni = merge_data[0]&0xFFFFFF;
                    break;
                case 2:
                    user_info->l4_info.gre_key = merge_data[0];
                    break;
                case 3:
                    SYS_USW_SET_USER_MAC(user_info->l4_info.wlan.radio_mac, merge_data);
                    user_info->l4_info.wlan.radio_id = (merge_data[1]>>16)&0x1F;
                    user_info->l4_info.wlan.is_ctl_pkt = (merge_data[1]>>21)&0x01;
                    break;
                default:
                    break;
            }
        }
        /*get ipv6 address high bits*/
        GetDmaToCpuIpfixAccL3Ipv6KeyFifo(A, uIpSa_g2_ipSaPrefix_f , p_info,  ipv6_address);
        user_info->l3_info.ipv6.ipsa[0] = ipv6_address[1];
        user_info->l3_info.ipv6.ipsa[1] = ipv6_address[0];

        GetDmaToCpuIpfixAccL3Ipv6KeyFifo(A, uIpDa_g2_ipDaPrefix_f , p_info, ipv6_address);
        user_info->l3_info.ipv6.ipda[0] = ipv6_address[1];
        user_info->l3_info.ipv6.ipda[1] = ipv6_address[0];

        user_info->l3_info.ipv6.ipda_masklen = field_sel.u.ipv6.ip_da_mask;
        user_info->l3_info.ipv6.ipsa_masklen = field_sel.u.ipv6.ip_sa_mask;

    }
    return CTC_E_NONE;

}

STATIC int32
_sys_usw_ipfix_parser_l3_mpls_info(uint8 lchip, DmaToCpuIpfixAccL3MplsKeyFifo_m* p_info, ctc_ipfix_data_t* user_info)
{
    uint32 cmd = 0;
    IpfixL3MplsHashFieldSelect_m  hash_field;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    user_info->key_type = CTC_IPFIX_KEY_HASH_MPLS;
    user_info->field_sel_id = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, flowFieldSel_f, p_info);
    user_info->profile_id = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, ipfixCfgProfileId_f, p_info);

    cmd = DRV_IOR(IpfixL3MplsHashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, user_info->field_sel_id, cmd, &hash_field));

    switch(GetIpfixL3MplsHashFieldSelect(V, globalPortType_f, &hash_field))
    {
        case 1:
            user_info->port_type = CTC_IPFIX_PORT_TYPE_GPORT;
            user_info->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDmaToCpuIpfixAccL3MplsKeyFifo(V, globalPort_f, p_info));
            break;
        case 2:
            user_info->logic_port = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, globalPort_f, p_info);
            user_info->port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
            break;
        case 3:
        case 4:
            user_info->logic_port = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, globalPort_f, p_info);
            user_info->port_type = CTC_IPFIX_PORT_TYPE_METADATA;
            break;
        default:
            user_info->port_type = CTC_IPFIX_PORT_TYPE_NONE;
            break;
    }
    if (GetDmaToCpuIpfixAccL3MplsKeyFifo(V, isEgressKey_f, p_info))
    {
        user_info->dir = CTC_EGRESS;
    }
    else
    {
        user_info->dir = CTC_INGRESS;
    }


    user_info->l3_info.mpls.label_num = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, labelNum_f, p_info);
    if (user_info->l3_info.mpls.label_num > CTC_IPFIX_LABEL_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    user_info->l3_info.mpls.label[0].exp = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsExp0_f, p_info);
    user_info->l3_info.mpls.label[0].label = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsLabel0_f, p_info);
    user_info->l3_info.mpls.label[0].sbit = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsSbit0_f, p_info);
    user_info->l3_info.mpls.label[0].ttl = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsTtl0_f, p_info);
    user_info->l3_info.mpls.label[1].exp = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsExp1_f, p_info);
    user_info->l3_info.mpls.label[1].label = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsLabel1_f, p_info);
    user_info->l3_info.mpls.label[1].sbit = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsSbit1_f, p_info);
    user_info->l3_info.mpls.label[1].ttl = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsTtl1_f, p_info);
    user_info->l3_info.mpls.label[2].exp = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsExp2_f, p_info);
    user_info->l3_info.mpls.label[2].label = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsLabel2_f, p_info);
    user_info->l3_info.mpls.label[2].sbit = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsSbit2_f, p_info);
    user_info->l3_info.mpls.label[2].ttl = GetDmaToCpuIpfixAccL3MplsKeyFifo(V, mplsTtl2_f, p_info);
    return CTC_E_NONE;
}

/**
   @brief sync dma ipfix info
 */
int32
sys_usw_ipfix_sync_data(uint8 lchip, void* p_dma_info)
{
    uint16 entry_num = 0;
    uint16 index = 0;
    sys_dma_info_t* p_info = (sys_dma_info_t*)p_dma_info;
    ctc_ipfix_data_t user_info;
    DmaToCpuIpfixAccFifo_m* p_temp_fifo = NULL;
    uint32 key_type = 0;
    uint32    rst_hit;
    uint32    key_index;
    uint32    ret = 0;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    entry_num = p_info->entry_num;

    for (index = 0; index < entry_num; index++)
    {
        SYS_IPFIX_INIT_CHECK(lchip);

        sal_memset(&user_info, 0, sizeof(user_info));
        p_temp_fifo = ((DmaToCpuIpfixAccFifo_m*)p_info->p_data+index);

        key_type = GetDmaToCpuIpfixAccL2KeyFifo(V, hashKeyType_f, p_temp_fifo);

        switch(key_type)
        {
            case SYS_IPFIX_HASH_TYPE_L2:
                CTC_ERROR_RETURN(_sys_usw_ipfix_parser_l2_info(lchip, (DmaToCpuIpfixAccL2KeyFifo_m*)p_temp_fifo, &user_info));
                break;

            case SYS_IPFIX_HASH_TYPE_L2L3:
                CTC_ERROR_RETURN(_sys_usw_ipfix_parser_l2l3_info(lchip, (DmaToCpuIpfixAccL2L3KeyFifo_m*)p_temp_fifo, &user_info));
                break;

            case SYS_IPFIX_HASH_TYPE_IPV4:
                CTC_ERROR_RETURN(_sys_usw_ipfix_parser_l3_ipv4_info(lchip, (DmaToCpuIpfixAccL3Ipv4KeyFifo_m*)p_temp_fifo, &user_info));
                break;

            case SYS_IPFIX_HASH_TYPE_IPV6:
                CTC_ERROR_RETURN(_sys_usw_ipfix_parser_l3_ipv6_info(lchip, (DmaToCpuIpfixAccL3Ipv6KeyFifo_m*)p_temp_fifo, &user_info));
                break;

            case SYS_IPFIX_HASH_TYPE_MPLS:
                CTC_ERROR_RETURN(_sys_usw_ipfix_parser_l3_mpls_info(lchip, (DmaToCpuIpfixAccL3MplsKeyFifo_m*)p_temp_fifo, &user_info));
                break;

            default:
                SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Invalid IPfix Record!! \n");
                break;

        }


        CTC_ERROR_RETURN(_sys_usw_ipfix_parser_ad_info(lchip, &user_info, p_temp_fifo));

        ret = _sys_usw_ipfix_get_entry_by_key(lchip, &user_info, &rst_hit, &key_index);
        if(ret && ret != CTC_E_NOT_EXIST)
        {
            return ret;
        }
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"is_hit: %d, key_index:%d\n", rst_hit, key_index);
        SYS_IPFIX_INIT_CHECK(lchip);

        SYS_IPFIX_LOCK;
        _sys_usw_ipfix_export_stats(lchip, user_info.export_reason, user_info.pkt_count, user_info.byte_count);
        SYS_IPFIX_UNLOCK;
        if (p_usw_ipfix_master[lchip]->ipfix_cb)
        {
            p_usw_ipfix_master[lchip]->ipfix_cb(&user_info, p_usw_ipfix_master[lchip]->user_data);
        }

    }
    return CTC_E_NONE;
}

#define __CPU_INTERFACE__
STATIC int32
_sys_usw_ipfix_decode_l2_hashkey(uint8 lchip, ctc_ipfix_data_t* p_data, DsIpfixL2HashKey_m* p_key)
{
    uint32 cmd = 0;
    IpfixL2HashFieldSelect_m  hash_field;
    hw_mac_addr_t             mac_sa   = { 0 };
    hw_mac_addr_t             mac_da   = { 0 };

    p_data->field_sel_id = GetDsIpfixL2HashKey(V, flowFieldSel_f , p_key);

    cmd = DRV_IOR(IpfixL2HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_data->field_sel_id, cmd, &hash_field));

    p_data->dir = GetDsIpfixL2HashKey(V, isEgressKey_f, p_key)?1:0;
    p_data->ether_type = GetDsIpfixL2HashKey(V, etherType_f, p_key);
    p_data->profile_id = GetDsIpfixL2HashKey(V, ipfixCfgProfileId_f, p_key);

    if (GetDsIpfixL2HashKey(V, flowL2KeyUseCvlan_f, p_key))
    {
        p_data->cvlan = GetDsIpfixL2HashKey(V, vlanId_f, p_key);
        p_data->cvlan_cfi = GetDsIpfixL2HashKey(V, vtagCfi_f, p_key);
        p_data->cvlan_prio = GetDsIpfixL2HashKey(V, vtagCos_f, p_key);
        if (GetDsIpfixL2HashKey(V, vlanIdValid_f, p_key))
        {
            p_data->flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
        }
    }
    else
    {
        p_data->svlan = GetDsIpfixL2HashKey(V, vlanId_f, p_key);
        p_data->svlan_cfi = GetDsIpfixL2HashKey(V, vtagCfi_f, p_key);
        p_data->svlan_prio = GetDsIpfixL2HashKey(V, vtagCos_f, p_key);
        if (GetDsIpfixL2HashKey(V, vlanIdValid_f, p_key))
        {
            p_data->flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
        }
    }

    switch(GetIpfixL2HashFieldSelect(V, globalPortType_f, &hash_field))
    {
        case 1:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_GPORT;
            p_data->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDsIpfixL2HashKey(V, globalPort_f, p_key));
            break;
        case 2:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
            p_data->logic_port = GetDsIpfixL2HashKey(V, globalPort_f, p_key);
            break;
        case 3:
        case 4:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_METADATA;
            p_data->logic_port = GetDsIpfixL2HashKey(V, globalPort_f, p_key);
            break;
        default:
            break;
    }
    GetDsIpfixL2HashKey(A, macDa_f, p_key, mac_da);
    if(GetIpfixL2HashFieldSelect(V, macDaUseDstCategoryId_f, &hash_field))
    {
        p_data->dst_cid = mac_da[0]&0xFF;
        if(mac_da[0] & (1<<8))
        {
            p_data->flags |= CTC_IPFIX_DATA_DST_CID_VALID;
        }
    }
    else
    {
        SYS_USW_SET_USER_MAC(p_data->dst_mac, mac_da);
    }

    GetDsIpfixL2HashKey(A, macSa_f, p_key, mac_sa);
    if(GetIpfixL2HashFieldSelect(V, macSaUseSrcCategoryId_f, &hash_field))
    {
        p_data->src_cid = mac_sa[0]&0xFF;
        if(mac_sa[0] & (1<<8))
        {
            p_data->flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
        }
    }
    else
    {
        SYS_USW_SET_USER_MAC(p_data->src_mac, mac_sa);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_decode_l2l3_hashkey(uint8 lchip, ctc_ipfix_data_t* p_data, DsIpfixL2L3HashKey_m* p_key)
{
    uint32 cmd = 0;
    IpfixL2L3HashFieldSelect_m  hash_field;
    hw_mac_addr_t             mac_sa   = { 0 };
    hw_mac_addr_t             mac_da   = { 0 };
    hw_mac_addr_t             radio_mac = { 0 };
    ctc_ipfix_l3_info_t* p_l3_info;
    ctc_ipfix_l4_info_t* p_l4_info;
    ctc_ipfix_hash_field_sel_t field_sel;

    p_l3_info = (ctc_ipfix_l3_info_t*)&(p_data->l3_info);
    p_l4_info = (ctc_ipfix_l4_info_t*)&(p_data->l4_info);
    sal_memset(&field_sel, 0, sizeof(ctc_ipfix_hash_field_sel_t));

    p_data->field_sel_id = GetDsIpfixL2L3HashKey(V, flowFieldSel_f , p_key);
    p_data->profile_id = GetDsIpfixL2L3HashKey(V, ipfixCfgProfileId_f , p_key);
    cmd = DRV_IOR(IpfixL2L3HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_data->field_sel_id, cmd, &hash_field));

    p_data->dir = GetDsIpfixL2L3HashKey(V, isEgressKey_f, p_key)?1:0;
    p_data->ether_type = GetDsIpfixL2L3HashKey(V, etherType_f, p_key);

    if(GetDsIpfixL2L3HashKey(V, flowL2KeyUseCvlan_f, p_key))
    {
        if (GetIpfixL2L3HashFieldSelect(V, vlanIdValidEn_f, &hash_field))
        {
            p_data->cvlan = GetDsIpfixL2L3HashKey(V, vlanId_f, p_key);
            p_data->flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
        }
        p_data->cvlan_prio = GetDsIpfixL2L3HashKey(V, vtagCos_f, p_key);
        p_data->cvlan_cfi = GetDsIpfixL2L3HashKey(V, vtagCfi_f, p_key);
    }
    else
    {
        if (GetIpfixL2L3HashFieldSelect(V, vlanIdValidEn_f, &hash_field))
        {
            p_data->svlan = GetDsIpfixL2L3HashKey(V, vlanId_f, p_key);
            p_data->flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
        }
        p_data->svlan_prio = GetDsIpfixL2L3HashKey(V, vtagCos_f, p_key);
        p_data->svlan_cfi = GetDsIpfixL2L3HashKey(V, vtagCfi_f, p_key);

    }
    switch(GetIpfixL2L3HashFieldSelect(V, globalPortType_f, &hash_field))
    {
        case 1:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_GPORT;
            p_data->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDsIpfixL2L3HashKey(V, globalPort_f, p_key));
            break;
        case 2:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
            p_data->logic_port = GetDsIpfixL2L3HashKey(V, globalPort_f, p_key);
            break;
        case 3:
        case 4:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_METADATA;
            p_data->logic_port = GetDsIpfixL2L3HashKey(V, globalPort_f, p_key);
            break;
        default:
            break;
    }

    GetDsIpfixL2L3HashKey(A, macSa_f, p_key, mac_sa);
    GetDsIpfixL2L3HashKey(A, macDa_f, p_key, mac_da);
    SYS_USW_SET_USER_MAC(p_data->src_mac, mac_sa);
    SYS_USW_SET_USER_MAC(p_data->dst_mac, mac_da);

    CTC_ERROR_RETURN(sys_usw_ipfix_get_hash_field_sel(lchip, p_data->field_sel_id, CTC_IPFIX_KEY_HASH_L2_L3, &field_sel));
    /* parser l3 information */
    if(field_sel.u.l2_l3.dst_cid)
    {
        p_data->dst_cid = GetDsIpfixL2L3HashKey(V, uL3_gIpv4_ipDa_f , p_key)&0xFF;
        if(GetDsIpfixL2L3HashKey(V, uL3_gIpv4_ipDa_f , p_key) & (1<<8))
        {
            p_data->flags |= CTC_IPFIX_DATA_DST_CID_VALID;
        }
    }
    else
    {
        p_l3_info->ipv4.ipda = GetDsIpfixL2L3HashKey(V, uL3_gIpv4_ipDa_f , p_key);
        p_l3_info->ipv4.ipda_masklen = field_sel.u.l2_l3.ip_da_mask;
    }
    if(field_sel.u.l2_l3.src_cid)
    {
        p_data->src_cid = GetDsIpfixL2L3HashKey(V, uL3_gIpv4_ipSa_f , p_key)&0xFF;
        if(GetDsIpfixL2L3HashKey(V, uL3_gIpv4_ipSa_f , p_key) & (1<<8))
        {
            p_data->flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
        }
    }
    else
    {
        p_l3_info->ipv4.ipsa = GetDsIpfixL2L3HashKey(V, uL3_gIpv4_ipSa_f , p_key);
        p_l3_info->ipv4.ipsa_masklen = field_sel.u.l2_l3.ip_sa_mask;
    }
    if(field_sel.u.l2_l3.ttl)
    {
        p_l3_info->ipv4.ttl = GetDsIpfixL2L3HashKey(V, uL3_gIpv4_ttl_f, p_key);
    }
    if(field_sel.u.l2_l3.dscp)
    {
        p_l3_info->ipv4.dscp = GetDsIpfixL2L3HashKey(V, uL3_gIpv4_dscp_f, p_key);
    }
    if(field_sel.u.l2_l3.ecn)
    {
        p_l3_info->ipv4.ecn = GetDsIpfixL2L3HashKey(V, uL3_gIpv4_ecn_f, p_key);
    }

    if(field_sel.u.l2_l3.label_num)
    {
        p_l3_info->mpls.label_num = GetDsIpfixL2L3HashKey(V, uL3_gMpls_labelNum_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label0_exp)
    {
        p_l3_info->mpls.label[0].exp = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsExp0_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label0_label)
    {
        p_l3_info->mpls.label[0].label = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsLabel0_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label0_s)
    {
        p_l3_info->mpls.label[0].sbit = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsSbit0_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label0_ttl)
    {
        p_l3_info->mpls.label[0].ttl = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsTtl0_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label1_exp)
    {
        p_l3_info->mpls.label[1].exp = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsExp1_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label1_label)
    {
        p_l3_info->mpls.label[1].label = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsLabel1_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label1_s)
    {
        p_l3_info->mpls.label[1].sbit = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsSbit1_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label1_ttl)
    {
        p_l3_info->mpls.label[1].ttl = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsTtl1_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label2_exp)
    {
        p_l3_info->mpls.label[2].exp = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsExp2_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label2_label)
    {
        p_l3_info->mpls.label[2].label = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsLabel2_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label2_s)
    {
        p_l3_info->mpls.label[2].sbit = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsSbit2_f, p_key);
    }
    if(field_sel.u.l2_l3.mpls_label2_ttl)
    {
        p_l3_info->mpls.label[2].ttl = GetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsTtl2_f, p_key);
    }

    if (field_sel.u.l2_l3.icmp_code || field_sel.u.l2_l3.icmp_type)
    {
        p_l4_info->icmp.icmpcode = GetDsIpfixL2L3HashKey(V, uL4_gPort_l4SourcePort_f, p_key)&0xFF;
        p_l4_info->icmp.icmp_type = (GetDsIpfixL2L3HashKey(V, uL4_gPort_l4SourcePort_f, p_key) >> 8)&0xFF;
    }
    else if(field_sel.u.l2_l3.igmp_type)
    {
        p_l4_info->igmp.igmp_type = (GetDsIpfixL2L3HashKey(V, uL4_gPort_l4SourcePort_f, p_key) >> 8)&0xFF;
    }
    else if(field_sel.u.l2_l3.l4_src_port || field_sel.u.l2_l3.l4_dst_port)
    {
        p_l4_info->l4_port.dest_port = GetDsIpfixL2L3HashKey(V, uL4_gPort_l4DestPort_f, p_key);
        p_l4_info->l4_port.source_port = GetDsIpfixL2L3HashKey(V, uL4_gPort_l4SourcePort_f, p_key);
    }

    /* process l4 information */
    p_l4_info->aware_tunnel_info_en = GetDsIpfixL2L3HashKey(V, isMergeKey_f, p_key);
    if (p_l4_info->aware_tunnel_info_en)
    {
        switch(GetDsIpfixL2L3HashKey(V, uL3_gIpv4_mergeDataType_f, p_key))
        {
            case 1:
                p_l4_info->vni = GetDsIpfixL2L3HashKey(V, uL3_gIpv4_mergeData_f, p_key);
                break;
            case 2:
                p_l4_info->gre_key = GetDsIpfixL2L3HashKey(V, uL3_gIpv4_mergeData_f, p_key);
                if (field_sel.u.l2_l3.gre_key)
                {
                    p_l4_info->gre_key = p_l4_info->gre_key | GetDsIpfixL2L3HashKey(V, mergeDataExt3_f, p_key) << 24;
                }
                break;
            case 3:
            case 4:
                p_l4_info->wlan.radio_id = GetDsIpfixL2L3HashKey(V, mergeDataExt4_f, p_key)&0x1F;
                p_l4_info->wlan.is_ctl_pkt = (GetDsIpfixL2L3HashKey(V, mergeDataExt4_f, p_key)>>5)&0x01;
                radio_mac[0] = GetDsIpfixL2L3HashKey(V, uL3_gIpv4_mergeData_f,p_key);
                radio_mac[0] = radio_mac[0] | ((GetDsIpfixL2L3HashKey(V, uL4_gPort_tcpFlags_f,p_key))<< 24);
                radio_mac[1] = GetDsIpfixL2L3HashKey(V, layer3HeaderProtocol_f,p_key);
                radio_mac[1] = radio_mac[1] | ((GetDsIpfixL2L3HashKey(V, mergeDataExt3_f,p_key)) << 8);
                SYS_USW_SET_USER_MAC(p_l4_info->wlan.radio_mac, radio_mac);
                break;
            default:
                break;
        }
    }
    else
    {
        p_l4_info->type.ip_protocol = GetDsIpfixL2L3HashKey(V, layer3HeaderProtocol_f,p_key);

        if(field_sel.u.l2_l3.gre_key || field_sel.u.l2_l3.nvgre_key)
        {
            p_l4_info->gre_key = GetDsIpfixL2L3HashKey(V, uL4_gKey_greKey_f, p_key);
        }
        else if(field_sel.u.l2_l3.vxlan_vni)
        {
            p_l4_info->vni = GetDsIpfixL2L3HashKey(V, uL4_gVxlan_vni_f, p_key);
        }

        if(field_sel.u.l2_l3.tcp_flags)
        {
            p_l4_info->tcp_flags = GetDsIpfixL2L3HashKey(V, uL4_gPort_tcpFlags_f ,p_key);
        }
        if (field_sel.u.l2_l3.ip_frag)
        {
            p_l3_info->ip_frag =  SYS_IPFIX_UNMAP_IP_FRAG(GetDsIpfixL2L3HashKey(V,uL3_gIpv4_mergeDataType_f, p_key));
        }
        if (field_sel.u.l2_l3.ip_identification)
        {
            p_l3_info->ipv4.ip_identification =  GetDsIpfixL2L3HashKey(V,uL3_gIpv4_mergeData_f, p_key);
        }
        if (field_sel.u.l2_l3.vrfid)
        {
            uint16 vrfid =  GetDsIpfixL2L3HashKey(V, mergeDataExt3_f, p_key);
            vrfid |= (GetDsIpfixL2L3HashKey(V, mergeDataExt4_f, p_key)&0x3F << 8);
            p_l3_info->vrfid =  vrfid;
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_decode_ipv4_hashkey(uint8 lchip, ctc_ipfix_data_t* p_data, DsIpfixL3Ipv4HashKey_m* p_key)
{
    uint32 cmd = 0;
    IpfixL3Ipv4HashFieldSelect_m  hash_field;
    ctc_ipfix_l3_info_t* p_l3_info;
    ctc_ipfix_l4_info_t* p_l4_info;
    ctc_ipfix_hash_field_sel_t field_sel;

    p_l3_info = (ctc_ipfix_l3_info_t*)&(p_data->l3_info);
    p_l4_info = (ctc_ipfix_l4_info_t*)&(p_data->l4_info);
    sal_memset(&field_sel, 0, sizeof(ctc_ipfix_hash_field_sel_t));

    p_data->field_sel_id = GetDsIpfixL3Ipv4HashKey(V, flowFieldSel_f, p_key);
    p_data->profile_id = GetDsIpfixL3Ipv4HashKey(V, ipfixCfgProfileId_f, p_key);
    cmd = DRV_IOR(IpfixL3Ipv4HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_data->field_sel_id, cmd, &hash_field));
    CTC_ERROR_RETURN(sys_usw_ipfix_get_hash_field_sel(lchip, p_data->field_sel_id, CTC_IPFIX_KEY_HASH_IPV4, &field_sel));

    p_data->dir = GetDsIpfixL3Ipv4HashKey(V, isEgressKey_f, p_key)?1:0;
    switch(GetIpfixL3Ipv4HashFieldSelect(V, globalPortType_f, &hash_field))
    {
        case 1:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_GPORT;
            p_data->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDsIpfixL3Ipv4HashKey(V, globalPort_f, p_key));
            break;
        case 2:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
            p_data->logic_port = GetDsIpfixL3Ipv4HashKey(V, globalPort_f, p_key);
            break;
        case 3:
        case 4:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_METADATA;
            p_data->logic_port = GetDsIpfixL3Ipv4HashKey(V, globalPort_f, p_key);
            break;
        default:
            break;
    }
    p_l3_info->ipv4.dscp = GetDsIpfixL3Ipv4HashKey(V, dscp_f , p_key);
    p_l3_info->ipv4.ttl = GetDsIpfixL3Ipv4HashKey(V, ttl_f , p_key);
    p_l3_info->ipv4.ecn = GetDsIpfixL3Ipv4HashKey(V, ecn_f , p_key);

    if(GetIpfixL3Ipv4HashFieldSelect(V, ipDaUseDstCategoryId_f, &hash_field))
    {
        p_data->dst_cid = GetDsIpfixL3Ipv4HashKey(V, uIpDa_g2_destCategoryId_f , p_key);
        if(GetDsIpfixL3Ipv4HashKey(V, uIpDa_g2_destCategoryIdClassfied_f , p_key))
        {
            p_data->flags |= CTC_IPFIX_DATA_DST_CID_VALID;
        }
    }
    else
    {
        p_l3_info->ipv4.ipda = GetDsIpfixL3Ipv4HashKey(V, uIpDa_g1_ipDa_f , p_key);
        p_l3_info->ipv4.ipda_masklen = field_sel.u.ipv4.ip_da_mask;
    }

    if(GetIpfixL3Ipv4HashFieldSelect(V, ipSaUseSrcCategoryId_f, &hash_field))
    {
        p_data->src_cid = GetDsIpfixL3Ipv4HashKey(V, uIpSa_g2_srcCategoryId_f , p_key);
        if(GetDsIpfixL3Ipv4HashKey(V, uIpSa_g2_srcCategoryIdClassfied_f , p_key))
        {
            p_data->flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
        }
    }
    else
    {
        p_l3_info->ipv4.ipsa = GetDsIpfixL3Ipv4HashKey(V, uIpSa_g1_ipSa_f , p_key);
        p_l3_info->ipv4.ipsa_masklen = field_sel.u.ipv4.ip_sa_mask;
    }

    p_l4_info->type.ip_protocol = GetDsIpfixL3Ipv4HashKey(V, layer3HeaderProtocol_f, p_key);

    if (field_sel.u.ipv4.vxlan_vni)
    {
        p_l4_info->vni = GetDsIpfixL3Ipv4HashKey(V, uL4_gVxlan_vni_f, p_key);
    }
    else if(field_sel.u.ipv4.gre_key || field_sel.u.ipv4.nvgre_key)
    {
        p_l4_info->gre_key = GetDsIpfixL3Ipv4HashKey(V, uL4_gKey_greKey_f, p_key);
    }
    else if (field_sel.u.ipv4.icmp_code || field_sel.u.ipv4.icmp_type)
    {
        p_l4_info->icmp.icmpcode = GetDsIpfixL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, p_key)&0xff;
        p_l4_info->icmp.icmp_type = (GetDsIpfixL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, p_key)>>8)&0xff;
    }
    else if(field_sel.u.ipv4.igmp_type)
    {
        p_l4_info->igmp.igmp_type = (GetDsIpfixL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, p_key)>>8)&0xff;
    }
    else if (field_sel.u.ipv4.l4_src_port || field_sel.u.ipv4.l4_dst_port)
    {
        p_l4_info->l4_port.dest_port = GetDsIpfixL3Ipv4HashKey(V, uL4_gPort_l4DestPort_f, p_key);
        p_l4_info->l4_port.source_port = GetDsIpfixL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, p_key);
    }

    /*share field*/
    switch(GetIpfixL3Ipv4HashFieldSelect(V, shareFieldMode_f, &hash_field))
    {
        case 0:
            p_l3_info->ip_frag = SYS_IPFIX_UNMAP_IP_FRAG(GetDsIpfixL3Ipv4HashKey(V, uShareField_g1_fragInfo_f,p_key));
            p_l4_info->tcp_flags = GetDsIpfixL3Ipv4HashKey(V, uShareField_g1_tcpFlags_f,p_key);
            break;
        case 1:
            p_l3_info->ipv4.ip_identification = GetDsIpfixL3Ipv4HashKey(V, uShareField_g2_ipIdentification_f,p_key);
            break;
        case 2:
            p_l3_info->ip_frag = SYS_IPFIX_UNMAP_IP_FRAG(GetDsIpfixL3Ipv4HashKey(V, uShareField_g3_fragInfo_f,p_key));
            p_l3_info->ipv4.ip_pkt_len = GetDsIpfixL3Ipv4HashKey(V, uShareField_g3_ipLength_f,p_key);
            break;
        default:
            p_l3_info->ip_frag = SYS_IPFIX_UNMAP_IP_FRAG(GetDmaIpfixAccL3Ipv4KeyFifo(V, uShareField_g4_fragInfo_f,p_key));
            p_l3_info->vrfid = GetDsIpfixL3Ipv4HashKey(V, uShareField_g4_vrfId_f,p_key);
            break;
    }
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_ipfix_decode_ipv6_hashkey(uint8 lchip, ctc_ipfix_data_t* p_data, DsIpfixL3Ipv6HashKey_m* p_key)
{
    uint32 cmd = 0;
    IpfixL3Ipv6HashFieldSelect_m  hash_field;
    ctc_ipfix_hash_field_sel_t field_sel;
    uint32 ipv6_address[4] = {0, 0, 0, 0};
    uint32 merge_data[2] = {0};

    sal_memset(&field_sel, 0, sizeof(ctc_ipfix_hash_field_sel_t));

    p_data->field_sel_id = GetDsIpfixL3Ipv6HashKey(V, flowFieldSel_f , p_key);
    p_data->profile_id = GetDsIpfixL3Ipv6HashKey(V, ipfixCfgProfileId_f, p_key);

    cmd = DRV_IOR(IpfixL3Ipv6HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_data->field_sel_id, cmd, &hash_field));
    CTC_ERROR_RETURN(sys_usw_ipfix_get_hash_field_sel(lchip, p_data->field_sel_id, CTC_IPFIX_KEY_HASH_IPV6, &field_sel));

    p_data->dir = GetDsIpfixL3Ipv6HashKey(V, isEgressKey_f, p_key)?1:0;
    switch(GetIpfixL3Ipv6HashFieldSelect(V, globalPortType_f, &hash_field))
    {
        case 1:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_GPORT;
            p_data->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDsIpfixL3Ipv6HashKey(V, globalPort_f, p_key));
            break;
        case 2:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
            p_data->logic_port = GetDsIpfixL3Ipv6HashKey(V, globalPort_f, p_key);
            break;
        case 3:
        case 4:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_METADATA;
            p_data->logic_port = GetDsIpfixL3Ipv6HashKey(V, globalPort_f, p_key);
            break;
        default:
            break;
    }
    p_data->l3_info.ipv6.dscp = GetDsIpfixL3Ipv6HashKey(V, dscp_f, p_key);
    p_data->l4_info.type.ip_protocol = GetDsIpfixL3Ipv6HashKey(V, layer3HeaderProtocol_f, p_key);
    p_data->l4_info.aware_tunnel_info_en = GetDsIpfixL3Ipv6HashKey(V, isMergeKey_f, p_key);

    if (field_sel.u.ipv6.icmp_type || field_sel.u.ipv6.icmp_code)
    {
        p_data->l4_info.icmp.icmpcode = GetDsIpfixL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, p_key)&0xff;
        p_data->l4_info.icmp.icmp_type = (GetDsIpfixL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, p_key) >> 8)&0xff;
    }
    else if(field_sel.u.ipv6.igmp_type)
    {
        p_data->l4_info.igmp.igmp_type = (GetDsIpfixL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, p_key) >> 8)&0xff;
    }
    else if(field_sel.u.ipv6.l4_src_port || field_sel.u.ipv6.l4_dst_port)
    {
        p_data->l4_info.l4_port.dest_port = GetDsIpfixL3Ipv6HashKey(V, uL4_gPort_l4DestPort_f, p_key);
        p_data->l4_info.l4_port.source_port = GetDsIpfixL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, p_key);
    }

    if(!p_data->l4_info.aware_tunnel_info_en)
    {
        if (field_sel.u.ipv6.gre_key || field_sel.u.ipv6.nvgre_key)
        {
            p_data->l4_info.gre_key = GetDsIpfixL3Ipv6HashKey(V, uL4_gKey_greKey_f, p_key);
        }
        else if (field_sel.u.ipv6.vxlan_vni)
        {
            p_data->l4_info.vni = GetDsIpfixL3Ipv6HashKey(V, uL4_gVxlan_vni_f, p_key);
        }
    }

    if(!GetIpfixL3Ipv6HashFieldSelect(V, ipAddressMode_f, &hash_field))
    {
        GetDsIpfixL3Ipv6HashKey(A, uIpSa_g2_ipSaPrefix_f , p_key,  ipv6_address);
        p_data->l3_info.ipv6.ipsa[0] = ipv6_address[3];
        p_data->l3_info.ipv6.ipsa[1] = ipv6_address[2];
        p_data->l3_info.ipv6.ipsa[2] = ipv6_address[1];
        p_data->l3_info.ipv6.ipsa[3] = ipv6_address[0];

        GetDsIpfixL3Ipv6HashKey(A, uIpDa_g2_ipDaPrefix_f , p_key, ipv6_address);
        p_data->l3_info.ipv6.ipda[0] = ipv6_address[3];
        p_data->l3_info.ipv6.ipda[1] = ipv6_address[2];
        p_data->l3_info.ipv6.ipda[2] = ipv6_address[1];
        p_data->l3_info.ipv6.ipda[3] = ipv6_address[0];


        p_data->l3_info.ipv6.ipda_masklen = field_sel.u.ipv6.ip_da_mask;
        p_data->l3_info.ipv6.ipsa_masklen = field_sel.u.ipv6.ip_sa_mask;
    }
    else
    {
        if(field_sel.u.ipv6.src_cid)
        {
            p_data->src_cid = GetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_srcCategoryId_f, p_key);
            if(GetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_srcCategoryIdClassfied_f, p_key))
            {
                p_data->flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
            }
        }
        if(field_sel.u.ipv6.dst_cid)
        {
            p_data->dst_cid = GetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_destCategoryId_f, p_key);
            if(GetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_destCategoryIdClassfied_f, p_key))
            {
                p_data->flags |= CTC_IPFIX_DATA_DST_CID_VALID;
            }
        }
        if(field_sel.u.ipv6.ip_frag)
        {
            p_data->l3_info.ip_frag = SYS_IPFIX_UNMAP_IP_FRAG(GetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_fragInfo_f, p_key));
        }
        if(field_sel.u.ipv6.ecn)
        {
            p_data->l3_info.ipv6.ecn = GetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_ecn_f, p_key);
        }
        if(field_sel.u.ipv6.ttl)
        {
            p_data->l3_info.ipv6.ttl = GetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_ttl_f, p_key);
        }
        if(field_sel.u.ipv6.flow_label)
        {
            p_data->l3_info.ipv6.flow_label = GetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_ipv6FlowLabel_f, p_key);
        }
        if(field_sel.u.ipv6.tcp_flags)
        {
            p_data->l4_info.tcp_flags = GetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_tcpFlags_f, p_key);
        }

        if (p_data->l4_info.aware_tunnel_info_en)
        {
            GetDsIpfixL3Ipv6HashKey(A, uIpDa_g2_mergeData_f, p_key,merge_data);
            switch(GetDsIpfixL3Ipv6HashKey(V,uIpDa_g2_mergeDataType_f,p_key))
            {
                case SYS_IPFIX_MERGE_TYPE_VXLAN:
                    p_data->l4_info.vni = merge_data[0]&0xFFFFFF;
                    break;
                case SYS_IPFIX_MERGE_TYPE_GRE:
                    p_data->l4_info.gre_key = merge_data[0];
                    break;
                case SYS_IPFIX_MERGE_TYPE_WLAN:
                    SYS_USW_SET_USER_MAC(p_data->l4_info.wlan.radio_mac, merge_data);
                    p_data->l4_info.wlan.radio_id = (merge_data[1]>>16)&0x1F;
                    p_data->l4_info.wlan.is_ctl_pkt = (merge_data[1]>>21)&0x01;
                    break;
                default:
                    break;
            }
        }
        GetDsIpfixL3Ipv6HashKey(A, uIpSa_g1_ipSa_f , p_key,  ipv6_address);
        p_data->l3_info.ipv6.ipsa[0] = ipv6_address[1];
        p_data->l3_info.ipv6.ipsa[1] = ipv6_address[0];

        GetDsIpfixL3Ipv6HashKey(A, uIpDa_g1_ipDa_f , p_key, ipv6_address);
        p_data->l3_info.ipv6.ipda[0] = ipv6_address[1];
        p_data->l3_info.ipv6.ipda[1] = ipv6_address[0];


        p_data->l3_info.ipv6.ipda_masklen = field_sel.u.ipv6.ip_da_mask;
        p_data->l3_info.ipv6.ipsa_masklen = field_sel.u.ipv6.ip_sa_mask;

    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_decode_mpls_hashkey(uint8 lchip, ctc_ipfix_data_t* p_data, DsIpfixL3MplsHashKey_m* p_key)
{
    uint32 cmd = 0;
    IpfixL3MplsHashFieldSelect_m  hash_field;
    ctc_ipfix_l3_info_t* p_l3_info;

    p_l3_info = (ctc_ipfix_l3_info_t*)&(p_data->l3_info);

    p_data->dir = GetDsIpfixL3MplsHashKey(V, isEgressKey_f, p_key)?1:0;
    p_data->field_sel_id = GetDsIpfixL3MplsHashKey(V, flowFieldSel_f , p_key);
    p_data->profile_id = GetDsIpfixL3MplsHashKey(V, ipfixCfgProfileId_f, p_key);

    cmd = DRV_IOR(IpfixL3MplsHashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_data->field_sel_id, cmd, &hash_field));

    switch(GetIpfixL3MplsHashFieldSelect(V, globalPortType_f, &hash_field))
    {
        case 1:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_GPORT;
            p_data->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDsIpfixL3MplsHashKey(V, globalPort_f, p_key));
            break;
        case 2:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
            p_data->logic_port = GetDsIpfixL3MplsHashKey(V, globalPort_f, p_key);
            break;
        case 3:
        case 4:
            p_data->port_type = CTC_IPFIX_PORT_TYPE_METADATA;
            p_data->logic_port = GetDsIpfixL3MplsHashKey(V, globalPort_f, p_key);
            break;
        default:
            break;
    }

    p_l3_info->mpls.label_num = GetDsIpfixL3MplsHashKey(V, labelNum_f, p_key);
    if (p_l3_info->mpls.label_num > CTC_IPFIX_LABEL_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    p_l3_info->mpls.label[0].exp = GetDsIpfixL3MplsHashKey(V, mplsExp0_f, p_key);
    p_l3_info->mpls.label[0].label = GetDsIpfixL3MplsHashKey(V, mplsLabel0_f, p_key);
    p_l3_info->mpls.label[0].sbit = GetDsIpfixL3MplsHashKey(V, mplsSbit0_f, p_key);
    p_l3_info->mpls.label[0].ttl = GetDsIpfixL3MplsHashKey(V, mplsTtl0_f, p_key);
    p_l3_info->mpls.label[1].exp = GetDsIpfixL3MplsHashKey(V, mplsExp1_f, p_key);
    p_l3_info->mpls.label[1].label = GetDsIpfixL3MplsHashKey(V, mplsLabel1_f, p_key);
    p_l3_info->mpls.label[1].sbit = GetDsIpfixL3MplsHashKey(V, mplsSbit1_f, p_key);
    p_l3_info->mpls.label[1].ttl = GetDsIpfixL3MplsHashKey(V, mplsTtl1_f, p_key);
    p_l3_info->mpls.label[2].exp = GetDsIpfixL3MplsHashKey(V, mplsExp2_f, p_key);
    p_l3_info->mpls.label[2].label = GetDsIpfixL3MplsHashKey(V, mplsLabel2_f, p_key);
    p_l3_info->mpls.label[2].sbit = GetDsIpfixL3MplsHashKey(V, mplsSbit2_f, p_key);
    p_l3_info->mpls.label[2].ttl = GetDsIpfixL3MplsHashKey(V, mplsTtl2_f, p_key);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_decode_ad_info(uint8 lchip, ctc_ipfix_data_t* p_data, DsIpfixSessionRecord_m* p_ad)
{
    uint32 byte_cnt[2] = {0};
    uint32 dest_type = 0;
    uint8 dest_chip= 0;
    uint8 sub_dest_type = 0;
    uint8 jitter_mode = 0;
    uint8 latency_mode = 0;

    dest_type = GetDsIpfixSessionRecord(V, destinationType_f, p_ad);
    if (!DRV_IS_DUET2(lchip) && p_data->dir == CTC_EGRESS)
    {
        jitter_mode = (dest_type == 4);
        latency_mode = (dest_type == 2);
        dest_type = SYS_IPFIX_UNICAST_DEST;
    }

    if (latency_mode == 1) /* latency*/
    {
        p_data->min_latency =  ((GetDsIpfixSessionRecord(V, mfpToken_f , p_ad) << 8) |
                                     GetDsIpfixSessionRecord(V, minTtl_f , p_ad));

        p_data->max_latency =  (((GetDsIpfixSessionRecord(V, fragment_f , p_ad)&0x1) << 31) |
                                  ((GetDsIpfixSessionRecord(V, nonFragment_f , p_ad)&0x1) << 30) |
                                  ((GetDsIpfixSessionRecord(V, lastTs_f , p_ad)&0x3FFFFF) << 8) |
                                   GetDsIpfixSessionRecord(V, maxTtl_f , p_ad));
    }
    else
    {
        if (GetDsIpfixSessionRecord(V, fragment_f , p_ad))
        {
            p_data->flags |= CTC_IPFIX_DATA_FRAGMENT_PKT_DETECTED;
        }

        p_data->start_timestamp = GetDsIpfixSessionRecord(V, firstTs_f , p_ad);
        p_data->last_timestamp = GetDsIpfixSessionRecord(V, lastTs_f , p_ad);

        if (jitter_mode == 1)
        {
            p_data->max_jitter =  (GetDsIpfixSessionRecord(V, minTtl_f , p_ad) << 8 |
                                         GetDsIpfixSessionRecord(V, maxTtl_f , p_ad));

            p_data->min_jitter = GetDsIpfixSessionRecord(V, uDestinationInfo_gL2Mcast_l2McGroupId_f , p_ad);
        }
        else
        {
            GetDsIpfixSessionRecord(A, byteCount_f , p_ad, byte_cnt);
            p_data->byte_count = 0;
            p_data->byte_count = byte_cnt[1];
            p_data->byte_count <<= 32;
            p_data->byte_count |= byte_cnt[0];

            p_data->max_ttl = GetDsIpfixSessionRecord(V, maxTtl_f , p_ad);
            p_data->min_ttl = GetDsIpfixSessionRecord(V, minTtl_f , p_ad);
        }
    }

    p_data->export_reason = GetDsIpfixSessionRecord(V, exportReason_f , p_ad);
    p_data->tcp_flags = GetDsIpfixSessionRecord(V, tcpFlagsStatus_f , p_ad);
    p_data->pkt_count = GetDsIpfixSessionRecord(V, packetCount_f , p_ad);
    if (GetDsIpfixSessionRecord(V, droppedPacket_f, p_ad))
    {
        p_data->flags |= CTC_IPFIX_DATA_DROP_DETECTED;
    }

    switch(dest_type)
    {
        case SYS_IPFIX_UNICAST_DEST:
            dest_chip = GetDsIpfixSessionRecord(V, uDestinationInfo_gUcastToNormal_destChipId_f , p_ad);
            p_data->dest_gport = GetDsIpfixSessionRecord(V, uDestinationInfo_gUcastToNormal_destId_f , p_ad);
            p_data->dest_gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(dest_chip,  p_data->dest_gport);
            break;
        case SYS_IPFIX_L2MC_DEST:
            p_data->dest_group_id = GetDsIpfixSessionRecord(V, uDestinationInfo_gL2Mcast_l2McGroupId_f , p_ad);
            p_data->flags |= CTC_IPFIX_DATA_L2_MCAST_DETECTED;
            break;
        case SYS_IPFIX_L3MC_DEST:
            p_data->dest_group_id = GetDsIpfixSessionRecord(V, uDestinationInfo_gL3Mcast_l3McGroupId_f , p_ad);
            p_data->flags |= CTC_IPFIX_DATA_L3_MCAST_DETECTED;
            break;
        case SYS_IPFIX_BCAST_DEST:
            p_data->dest_group_id = GetDsIpfixSessionRecord(V, uDestinationInfo_gBcast_floodingId_f, p_ad);
            p_data->flags |= CTC_IPFIX_DATA_BCAST_DETECTED;
            break;
        case SYS_IPFIX_UNKNOW_PKT_DEST:
            p_data->dest_group_id = GetDsIpfixSessionRecord(V, uDestinationInfo_gUnknownPkt_floodingId_f, p_ad);
            p_data->flags |= CTC_IPFIX_DATA_UNKNOW_PKT_DETECTED;
            break;
        case SYS_IPFIX_UNION_DEST:
            sub_dest_type = GetDsIpfixSessionRecord(V, uDestinationInfo_gApsGroup_destType_f , p_ad);
            switch(sub_dest_type)
            {
                case SYS_IPFIX_SUB_APS_DEST:
                    p_data->dest_group_id = GetDsIpfixSessionRecord(V, uDestinationInfo_gApsGroup_apsGroupId_f , p_ad);
                    p_data->flags |= CTC_IPFIX_DATA_APS_DETECTED;
                    break;
                case SYS_IPFIX_SUB_ECMP_DEST:
                    dest_chip = GetDsIpfixSessionRecord(V, uDestinationInfo_gUcastToNormal_destChipId_f , p_ad);
                    p_data->dest_gport = GetDsIpfixSessionRecord(V, uDestinationInfo_gUcastToNormal_destId_f , p_ad);
                    p_data->dest_gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(dest_chip, p_data->dest_gport);
                    p_data->flags |= CTC_IPFIX_DATA_ECMP_DETECTED;
                    break;
                case SYS_IPFIX_SUB_LINKAGG_DEST:
                    p_data->dest_group_id = GetDsIpfixSessionRecord(V, uDestinationInfo_gUcastToLagg_linkaggGroupId_f , p_ad);
                    p_data->flags |= CTC_IPFIX_DATA_LINKAGG_DETECTED;
                    break;
                default:
                    return CTC_E_INVALID_PARAM;
            }
        default:
            return CTC_E_INVALID_PARAM;
    }
    if(GetDsIpfixSessionRecord(V, u1Type_f , p_ad) && GetDsIpfixSessionRecord(V, mfpProfId_f , p_ad))
    {
        p_data->flags |= CTC_IPFIX_DATA_MFP_VALID;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_decode_hashkey_ad(uint8 lchip, ctc_ipfix_data_t* p_data, uint8 is_ad, void* p_key)
{
    uint32 type = p_data->key_type;

    if (is_ad)
    {
        CTC_ERROR_RETURN(_sys_usw_ipfix_decode_ad_info(lchip, p_data, (DsIpfixSessionRecord_m*)p_key));
    }
    else
    {
        switch(type)
        {
            case CTC_IPFIX_KEY_HASH_MAC:
                CTC_ERROR_RETURN(_sys_usw_ipfix_decode_l2_hashkey(lchip, p_data, (DsIpfixL2HashKey_m*)p_key));
                break;
            case CTC_IPFIX_KEY_HASH_IPV4:
                CTC_ERROR_RETURN(_sys_usw_ipfix_decode_ipv4_hashkey(lchip, p_data, (DsIpfixL3Ipv4HashKey_m*)p_key));
                break;
            case CTC_IPFIX_KEY_HASH_MPLS:
                CTC_ERROR_RETURN(_sys_usw_ipfix_decode_mpls_hashkey(lchip, p_data, (DsIpfixL3MplsHashKey_m*)p_key));
                break;
            case CTC_IPFIX_KEY_HASH_IPV6:
                CTC_ERROR_RETURN(_sys_usw_ipfix_decode_ipv6_hashkey(lchip, p_data, (DsIpfixL3Ipv6HashKey_m*)p_key));
                break;
            case CTC_IPFIX_KEY_HASH_L2_L3:
                CTC_ERROR_RETURN(_sys_usw_ipfix_decode_l2l3_hashkey(lchip, p_data, (DsIpfixL2L3HashKey_m*)p_key));
                break;
            default:
                return CTC_E_INVALID_PARAM;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_encode_l2_hashkey(uint8 lchip, ctc_ipfix_data_t* p_key, void* p_data)
{
    DsIpfixL2HashKey_m l2_key;
    IpfixL2HashFieldSelect_m  hash_field;
    uint32  mac[2] = {0};
    uint32 cmd = 0;
    uint16 global_port = 0;
    sal_memset(&l2_key, 0, sizeof(l2_key));

    /* process gport, logic-port, metadata */
    if (p_key->port_type == CTC_IPFIX_PORT_TYPE_GPORT)
    {
        SYS_GLOBAL_PORT_CHECK(p_key->gport);
        global_port = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_key->gport);
    }
    else if(p_key->port_type == CTC_IPFIX_PORT_TYPE_LOGIC_PORT)
    {
        CTC_MAX_VALUE_CHECK(p_key->logic_port, 0x3FFF);
        global_port = p_key->logic_port;
    }
    else if(p_key->port_type == CTC_IPFIX_PORT_TYPE_METADATA)
    {
        CTC_MAX_VALUE_CHECK(p_key->logic_port, 0x3FFF);
        global_port = p_key->logic_port;
    }


    SetDsIpfixL2HashKey(V, hashKeyType_f, &l2_key, SYS_IPFIX_HASH_TYPE_L2);
    SetDsIpfixL2HashKey(V, flowFieldSel_f, &l2_key, p_key->field_sel_id);
    SetDsIpfixL2HashKey(V, isEgressKey_f, &l2_key, ((p_key->dir==CTC_EGRESS)?1:0));
    SetDsIpfixL2HashKey(V, etherType_f, &l2_key, p_key->ether_type);
    SetDsIpfixL2HashKey(V, globalPort_f, &l2_key, global_port);
    SetDsIpfixL2HashKey(V, ipfixCfgProfileId_f, &l2_key, p_key->profile_id);

    cmd = DRV_IOR(IpfixL2HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_key->field_sel_id, cmd, &hash_field));


    if((p_key->flags&CTC_IPFIX_DATA_CVLAN_TAGGED) && !(p_key->flags&CTC_IPFIX_DATA_SVLAN_TAGGED))
    {
        SetDsIpfixL2HashKey(V, flowL2KeyUseCvlan_f, &l2_key, 1);
        SetDsIpfixL2HashKey(V, vlanId_f, &l2_key, p_key->cvlan);
        SetDsIpfixL2HashKey(V, vlanIdValid_f, &l2_key, 1);
        SetDsIpfixL2HashKey(V, vtagCfi_f, &l2_key, p_key->cvlan_cfi);
        SetDsIpfixL2HashKey(V, vtagCos_f, &l2_key, p_key->cvlan_prio);
    }
    else if(!(p_key->flags&CTC_IPFIX_DATA_CVLAN_TAGGED) && (p_key->flags&CTC_IPFIX_DATA_SVLAN_TAGGED))
    {
        SetDsIpfixL2HashKey(V, flowL2KeyUseCvlan_f, &l2_key, 0);
        SetDsIpfixL2HashKey(V, vlanId_f, &l2_key, p_key->svlan);
        SetDsIpfixL2HashKey(V, vlanIdValid_f, &l2_key, 1);
        SetDsIpfixL2HashKey(V, vtagCfi_f, &l2_key, p_key->svlan_cfi);
        SetDsIpfixL2HashKey(V, vtagCos_f, &l2_key, p_key->svlan_prio);
    }
    else if((p_key->flags&CTC_IPFIX_DATA_CVLAN_TAGGED) && (p_key->flags&CTC_IPFIX_DATA_SVLAN_TAGGED))
    {
        return CTC_E_PARAM_CONFLICT;
    }
    if(GetIpfixL2HashFieldSelect(V,macDaUseDstCategoryId_f,&hash_field))
    {
        uint8  cid_valid = CTC_FLAG_ISSET(p_key->flags, CTC_IPFIX_DATA_DST_CID_VALID);
        mac[0] = (cid_valid<<8)| (p_key->dst_cid);
    }
    else
    {
        SYS_USW_SET_HW_MAC(mac, p_key->dst_mac);
    }
    SetDsIpfixL2HashKey(A, macDa_f, &l2_key, mac);
    if(GetIpfixL2HashFieldSelect(V,macSaUseSrcCategoryId_f,&hash_field))
    {
        uint8  cid_valid = CTC_FLAG_ISSET(p_key->flags, CTC_IPFIX_DATA_SRC_CID_VALID);
        mac[0] = (cid_valid<<8)| (p_key->src_cid);
    }
    else
    {
        SYS_USW_SET_HW_MAC(mac, p_key->src_mac);

    }
    SetDsIpfixL2HashKey(A, macSa_f, &l2_key, mac);
    sal_memcpy((uint8*)p_data, (uint8*)&l2_key, sizeof(l2_key));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_encode_l2l3_hashkey(uint8 lchip, ctc_ipfix_data_t* p_key, void* p_data)
{
    DsIpfixL2L3HashKey_m l2l3_key;
    IpfixL2L3HashFieldSelect_m  hash_field;
    uint32  mac[2] = {0};
    uint32 cmd = 0;
    uint16 l4_srcport = 0;
    uint16 global_port = 0;
    uint32 cid_field =0;
    uint8 tcp_flag_mask = 0;
    hw_mac_addr_t hw_mac;
    ctc_ipfix_hash_field_sel_t field_sel;

    sal_memset(&l2l3_key, 0, sizeof(l2l3_key));
    sal_memset(&field_sel, 0, sizeof(field_sel));

    CTC_ERROR_RETURN(sys_usw_ipfix_get_hash_field_sel(lchip, p_key->field_sel_id, CTC_IPFIX_KEY_HASH_L2_L3, &field_sel));

    /* process gport, logic-port, metadata */
    if (p_key->port_type == CTC_IPFIX_PORT_TYPE_GPORT)
    {
        SYS_GLOBAL_PORT_CHECK(p_key->gport);
        global_port = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_key->gport);
    }
    else if(p_key->port_type == CTC_IPFIX_PORT_TYPE_LOGIC_PORT)
    {
        CTC_MAX_VALUE_CHECK(p_key->logic_port, 0x3FFF);
        global_port = p_key->logic_port;
    }
    else if(p_key->port_type == CTC_IPFIX_PORT_TYPE_METADATA)
    {
        CTC_MAX_VALUE_CHECK(p_key->logic_port, 0x3FFF);
        global_port = p_key->logic_port;
    }


    SetDsIpfixL2L3HashKey(V, hashKeyType0_f, &l2l3_key, SYS_IPFIX_HASH_TYPE_L2L3);
    SetDsIpfixL2L3HashKey(V, hashKeyType1_f, &l2l3_key, SYS_IPFIX_HASH_TYPE_L2L3);
    SetDsIpfixL2L3HashKey(V, flowFieldSel_f, &l2l3_key, p_key->field_sel_id);
    SetDsIpfixL2L3HashKey(V, isEgressKey_f, &l2l3_key, ((p_key->dir==CTC_EGRESS)?1:0));
    SetDsIpfixL2L3HashKey(V, globalPort_f, &l2l3_key, global_port);
    SetDsIpfixL2L3HashKey(V, ipfixCfgProfileId_f, &l2l3_key, p_key->profile_id);
    cmd = DRV_IOR( IpfixL2L3HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_key->field_sel_id, cmd, &hash_field));
    SetDsIpfixL2L3HashKey(V, etherType_f, &l2l3_key, p_key->ether_type);
    SetDsIpfixL2L3HashKey(V, isMergeKey_f, &l2l3_key, p_key->l4_info.aware_tunnel_info_en);
    SetDsIpfixL2L3HashKey(V, isVxlan_f, &l2l3_key, (p_key->l4_info.aware_tunnel_info_en) ? 0 : (!!p_key->l4_info.vni));

    if((p_key->flags&CTC_IPFIX_DATA_CVLAN_TAGGED) && !(p_key->flags&CTC_IPFIX_DATA_SVLAN_TAGGED))
    {
        SetDsIpfixL2L3HashKey(V, flowL2KeyUseCvlan_f, &l2l3_key, 1);
        SetDsIpfixL2L3HashKey(V, vlanId_f, &l2l3_key, p_key->cvlan);
        SetDsIpfixL2L3HashKey(V, vlanIdValid_f, &l2l3_key, 1);
        SetDsIpfixL2L3HashKey(V, vtagCfi_f, &l2l3_key, p_key->cvlan_cfi);
        SetDsIpfixL2L3HashKey(V, vtagCos_f, &l2l3_key, p_key->cvlan_prio);
    }
    else if(!(p_key->flags&CTC_IPFIX_DATA_CVLAN_TAGGED) && (p_key->flags&CTC_IPFIX_DATA_SVLAN_TAGGED))
    {
        SetDsIpfixL2L3HashKey(V, flowL2KeyUseCvlan_f, &l2l3_key, 0);
        SetDsIpfixL2L3HashKey(V, vlanId_f, &l2l3_key, p_key->svlan);
        SetDsIpfixL2L3HashKey(V, vlanIdValid_f, &l2l3_key, 1);
        SetDsIpfixL2L3HashKey(V, vtagCfi_f, &l2l3_key, p_key->svlan_cfi);
        SetDsIpfixL2L3HashKey(V, vtagCos_f, &l2l3_key, p_key->svlan_prio);
    }
    else if((p_key->flags&CTC_IPFIX_DATA_CVLAN_TAGGED) && (p_key->flags&CTC_IPFIX_DATA_SVLAN_TAGGED))
    {
        return CTC_E_PARAM_CONFLICT;
    }

    SYS_USW_SET_HW_MAC(mac, p_key->dst_mac);
    SetDsIpfixL2L3HashKey(A, macDa_f, &l2l3_key, mac);
    SYS_USW_SET_HW_MAC(mac, p_key->src_mac);
    SetDsIpfixL2L3HashKey(A, macSa_f, &l2l3_key, mac);

    if (field_sel.u.l2_l3.dst_cid)
    {
        uint8  cid_valid = CTC_FLAG_ISSET(p_key->flags, CTC_IPFIX_DATA_DST_CID_VALID);
        cid_field = (cid_valid<<8)|p_key->dst_cid;
        SetDsIpfixL2L3HashKey(V, uL3_gIpv4_ipDa_f, &l2l3_key, cid_field);
    }
    else if(p_key->l3_info.ipv4.ipda != 0)
    {
        IPFIX_IPV4_MASK_LEN_CHECK(p_key->l3_info.ipv4.ipda_masklen);
        IPFIX_IPV4_MASK(p_key->l3_info.ipv4.ipda, p_key->l3_info.ipv4.ipda_masklen);
        SetDsIpfixL2L3HashKey(V, uL3_gIpv4_ipDa_f, &l2l3_key, p_key->l3_info.ipv4.ipda);
    }
    if (field_sel.u.l2_l3.src_cid)
    {
        uint8  cid_valid = CTC_FLAG_ISSET(p_key->flags, CTC_IPFIX_DATA_SRC_CID_VALID);
        cid_field = (cid_valid<<8)|p_key->src_cid;
        SetDsIpfixL2L3HashKey(V, uL3_gIpv4_ipSa_f, &l2l3_key, cid_field);
    }
    else if(p_key->l3_info.ipv4.ipsa != 0 )
    {
        IPFIX_IPV4_MASK_LEN_CHECK(p_key->l3_info.ipv4.ipsa_masklen);
        IPFIX_IPV4_MASK(p_key->l3_info.ipv4.ipsa, p_key->l3_info.ipv4.ipsa_masklen);
        SetDsIpfixL2L3HashKey(V, uL3_gIpv4_ipSa_f, &l2l3_key, p_key->l3_info.ipv4.ipsa);
    }
    if(field_sel.u.l2_l3.ttl)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gIpv4_ttl_f, &l2l3_key, p_key->l3_info.ipv4.ttl);
    }
    if(field_sel.u.l2_l3.dscp)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gIpv4_dscp_f, &l2l3_key, p_key->l3_info.ipv4.dscp);
    }
    if(field_sel.u.l2_l3.ecn)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gIpv4_ecn_f, &l2l3_key, p_key->l3_info.ipv4.ecn);
    }
    if(field_sel.u.l2_l3.label_num)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_labelNum_f, &l2l3_key, p_key->l3_info.mpls.label_num);
    }
    if(field_sel.u.l2_l3.mpls_label0_exp)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsExp0_f, &l2l3_key, p_key->l3_info.mpls.label[0].exp);
    }
    if(field_sel.u.l2_l3.mpls_label0_label)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsLabel0_f, &l2l3_key, p_key->l3_info.mpls.label[0].label);
    }
    if(field_sel.u.l2_l3.mpls_label0_s)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsSbit0_f, &l2l3_key, p_key->l3_info.mpls.label[0].sbit);
    }
    if(field_sel.u.l2_l3.mpls_label0_ttl)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsTtl0_f, &l2l3_key, p_key->l3_info.mpls.label[0].ttl);
    }
    if(field_sel.u.l2_l3.mpls_label1_exp)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsExp1_f, &l2l3_key, p_key->l3_info.mpls.label[1].exp);
    }
    if(field_sel.u.l2_l3.mpls_label1_label)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsLabel1_f, &l2l3_key, p_key->l3_info.mpls.label[1].label);
    }
    if(field_sel.u.l2_l3.mpls_label1_s)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsSbit1_f, &l2l3_key, p_key->l3_info.mpls.label[1].sbit);
    }
    if(field_sel.u.l2_l3.mpls_label1_ttl)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsTtl1_f, &l2l3_key, p_key->l3_info.mpls.label[1].ttl);
    }
    if(field_sel.u.l2_l3.mpls_label2_exp)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsExp2_f, &l2l3_key, p_key->l3_info.mpls.label[2].exp);
    }
    if(field_sel.u.l2_l3.mpls_label2_label)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsLabel2_f, &l2l3_key, p_key->l3_info.mpls.label[2].label);
    }
    if(field_sel.u.l2_l3.mpls_label2_s)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsSbit2_f, &l2l3_key, p_key->l3_info.mpls.label[2].sbit);
    }
    if(field_sel.u.l2_l3.mpls_label2_ttl)
    {
        SetDsIpfixL2L3HashKey(V, uL3_gMpls_mplsTtl2_f, &l2l3_key, p_key->l3_info.mpls.label[2].ttl);
    }

    if (field_sel.u.l2_l3.icmp_code || field_sel.u.l2_l3.icmp_type)
    {
        l4_srcport = (p_key->l4_info.icmp.icmpcode) & 0xff;
        l4_srcport |=  (((p_key->l4_info.icmp.icmp_type) & 0xff) << 8);
        SetDsIpfixL2L3HashKey(V, uL4_gPort_l4SourcePort_f, &l2l3_key, l4_srcport);
    }
    else if (field_sel.u.l2_l3.igmp_type)
    {
        l4_srcport = p_key->l4_info.igmp.igmp_type << 8;
        SetDsIpfixL2L3HashKey(V, uL4_gPort_l4SourcePort_f, &l2l3_key, l4_srcport);
    }
    else if(field_sel.u.l2_l3.l4_src_port || field_sel.u.l2_l3.l4_dst_port)
    {
        SetDsIpfixL2L3HashKey(V, uL4_gPort_l4DestPort_f, &l2l3_key, p_key->l4_info.l4_port.dest_port);
        SetDsIpfixL2L3HashKey(V, uL4_gPort_l4SourcePort_f, &l2l3_key, p_key->l4_info.l4_port.source_port);
    }

    /*gen l4 key*/
    if(field_sel.u.l2_l3.aware_tunnel_info_en)
    {
        if(field_sel.u.l2_l3.vxlan_vni)
        {
            SetDsIpfixL2L3HashKey(V, uL3_gIpv4_mergeDataType_f, &l2l3_key, 1);
            SetDsIpfixL2L3HashKey(V, uL3_gIpv4_mergeData_f, &l2l3_key, p_key->l4_info.vni);
        }
        else if(field_sel.u.l2_l3.gre_key || field_sel.u.l2_l3.nvgre_key)
        {
            SetDsIpfixL2L3HashKey(V, uL3_gIpv4_mergeDataType_f, &l2l3_key, 2);
            SetDsIpfixL2L3HashKey(V, uL3_gIpv4_mergeData_f, &l2l3_key, (p_key->l4_info.gre_key)&0xFFFFFF);
            if (field_sel.u.l2_l3.gre_key)
            {
                SetDsIpfixL2L3HashKey(V, mergeDataExt3_f, &l2l3_key, (p_key->l4_info.gre_key >> 24)&0xFF);
            }
        }

        else
        {
            uint8 value = 0;
            SetDsIpfixL2L3HashKey(V, uL3_gIpv4_mergeDataType_f, &l2l3_key, 3);
            if(field_sel.u.l2_l3.wlan_radio_mac)
            {
                SYS_USW_SET_HW_MAC(hw_mac, p_key->l4_info.wlan.radio_mac);
                SetDsIpfixL2L3HashKey(V, uL3_gIpv4_mergeData_f, &l2l3_key,hw_mac[0]&0xFFFFFF);
                SetDsIpfixL2L3HashKey(V, uL4_gPort_tcpFlags_f, &l2l3_key,(hw_mac[0] >> 24)&0xFF);
                SetDsIpfixL2L3HashKey(V, layer3HeaderProtocol_f, &l2l3_key, hw_mac[1] & 0xFF);
                SetDsIpfixL2L3HashKey(V, mergeDataExt3_f, &l2l3_key, (hw_mac[1]>>8 & 0xFF));
            }
            if(field_sel.u.l2_l3.wlan_radio_id)
            {
                value =  p_key->l4_info.wlan.radio_id;
            }
            if(field_sel.u.l2_l3.wlan_ctl_packet)
            {
                value |= (p_key->l4_info.wlan.is_ctl_pkt<<5);
            }
            SetDsIpfixL2L3HashKey(V, mergeDataExt4_f, &l2l3_key, value);
        }
    }
    else
    {
        SetDsIpfixL2L3HashKey(V, layer3HeaderProtocol_f, &l2l3_key,p_key->l4_info.type.ip_protocol);
        tcp_flag_mask = GetIpfixL2L3HashFieldSelect(V, g2_tcpFlagsEn_f,&hash_field);

        if (field_sel.u.l2_l3.gre_key || field_sel.u.l2_l3.nvgre_key)
        {
            SetDsIpfixL2L3HashKey(V, uL4_gKey_greKey_f, &l2l3_key, p_key->l4_info.gre_key);
        }
        else if (field_sel.u.l2_l3.vxlan_vni)
        {
            SetDsIpfixL2L3HashKey(V, uL4_gVxlan_vni_f, &l2l3_key, p_key->l4_info.vni);
        }

        if(tcp_flag_mask)
        {
            SetDsIpfixL2L3HashKey(V, uL4_gPort_tcpFlags_f, &l2l3_key, (p_key->l4_info.tcp_flags & tcp_flag_mask));
        }

        if (field_sel.u.l2_l3.ip_frag)
        {
            SetDsIpfixL2L3HashKey(V,uL3_gIpv4_mergeDataType_f, &l2l3_key, SYS_IPFIX_MAP_IP_FRAG(p_key->l3_info.ip_frag));
        }
        if (field_sel.u.l2_l3.ip_identification)
        {
            SetDsIpfixL2L3HashKey(V,uL3_gIpv4_mergeData_f, &l2l3_key, p_key->l3_info.ipv4.ip_identification);
        }
        if (field_sel.u.l2_l3.vrfid)
        {
            uint16 vrfid =  p_key->l3_info.vrfid;

            SetDsIpfixL2L3HashKey(V, mergeDataExt3_f, &l2l3_key, vrfid & 0xFF);
            SetDsIpfixL2L3HashKey(V, mergeDataExt4_f, &l2l3_key, (vrfid>>8 & 0x3F));
        }

    }

    sal_memcpy((uint8*)p_data, (uint8*)&l2l3_key, sizeof(l2l3_key));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_encode_ipv4_hashkey(uint8 lchip, ctc_ipfix_data_t* p_key, void* p_data)
{
    DsIpfixL3Ipv4HashKey_m ipv4_key;
    IpfixL3Ipv4HashFieldSelect_m hash_field;
    uint16 l4_srcport = 0;
    uint32 cmd = 0;
    uint16 global_port = 0;
    ctc_ipfix_hash_field_sel_t field_sel;
    uint8 tcp_flag_mask = 0;

    sal_memset(&ipv4_key, 0, sizeof(DsIpfixL3Ipv4HashKey_m));
    sal_memset(&hash_field, 0, sizeof(IpfixL3Ipv4HashFieldSelect_m));
    sal_memset(&field_sel, 0, sizeof(ctc_ipfix_hash_field_sel_t));

    CTC_ERROR_RETURN(sys_usw_ipfix_get_hash_field_sel(lchip, p_key->field_sel_id, CTC_IPFIX_KEY_HASH_IPV4, &field_sel));
    /* process gport, logic-port, metadata */
    if (p_key->port_type == CTC_IPFIX_PORT_TYPE_GPORT)
    {
        SYS_GLOBAL_PORT_CHECK(p_key->gport);
        global_port = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_key->gport);
    }
    else if(p_key->port_type == CTC_IPFIX_PORT_TYPE_LOGIC_PORT)
    {
        CTC_MAX_VALUE_CHECK(p_key->logic_port, 0x3FFF);
        global_port = p_key->logic_port;
    }
    else if(p_key->port_type == CTC_IPFIX_PORT_TYPE_METADATA)
    {
        CTC_MAX_VALUE_CHECK(p_key->logic_port, 0x3FFF);
        global_port = p_key->logic_port;
    }

    SetDsIpfixL3Ipv4HashKey(V, hashKeyType_f, &ipv4_key, SYS_IPFIX_HASH_TYPE_IPV4);
    SetDsIpfixL3Ipv4HashKey(V, flowFieldSel_f, &ipv4_key, p_key->field_sel_id);
    SetDsIpfixL3Ipv4HashKey(V, isEgressKey_f, &ipv4_key, ((p_key->dir==CTC_EGRESS)?1:0));
    SetDsIpfixL3Ipv4HashKey(V, globalPort_f, &ipv4_key, global_port);
    SetDsIpfixL3Ipv4HashKey(V, ipfixCfgProfileId_f, &ipv4_key, p_key->profile_id);
    cmd = DRV_IOR( IpfixL3Ipv4HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_key->field_sel_id, cmd, &hash_field));

    SetDsIpfixL3Ipv4HashKey(V, dscp_f, &ipv4_key, p_key->l3_info.ipv4.dscp);
    SetDsIpfixL3Ipv4HashKey(V, ttl_f, &ipv4_key, p_key->l3_info.ipv4.ttl);
    SetDsIpfixL3Ipv4HashKey(V, ecn_f, &ipv4_key, p_key->l3_info.ipv4.ecn);
    SetDsIpfixL3Ipv4HashKey(V, isVxlan_f, &ipv4_key, p_key->l4_info.vni?1:0);
    /*gen l3 key*/
    if (GetIpfixL3Ipv4HashFieldSelect(V, ipDaUseDstCategoryId_f,&hash_field))
    {
        SetDsIpfixL3Ipv4HashKey(V, uIpDa_g2_destCategoryId_f, &ipv4_key, p_key->dst_cid);
        SetDsIpfixL3Ipv4HashKey(V, uIpDa_g2_destCategoryIdClassfied_f, &ipv4_key, CTC_FLAG_ISSET(p_key->flags, CTC_IPFIX_DATA_DST_CID_VALID));
    }
    else if(p_key->l3_info.ipv4.ipda != 0)
    {
        IPFIX_IPV4_MASK_LEN_CHECK(p_key->l3_info.ipv4.ipda_masklen);
        IPFIX_IPV4_MASK(p_key->l3_info.ipv4.ipda, p_key->l3_info.ipv4.ipda_masklen);
        SetDsIpfixL3Ipv4HashKey(V, uIpDa_g1_ipDa_f, &ipv4_key, p_key->l3_info.ipv4.ipda);
    }
    if (GetIpfixL3Ipv4HashFieldSelect(V, ipSaUseSrcCategoryId_f,&hash_field))
    {
        SetDsIpfixL3Ipv4HashKey(V, uIpSa_g2_srcCategoryId_f, &ipv4_key, p_key->src_cid);
        SetDsIpfixL3Ipv4HashKey(V, uIpSa_g2_srcCategoryIdClassfied_f, &ipv4_key, CTC_FLAG_ISSET(p_key->flags, CTC_IPFIX_DATA_SRC_CID_VALID));
    }
    else if(p_key->l3_info.ipv4.ipsa != 0 )
    {
        IPFIX_IPV4_MASK_LEN_CHECK(p_key->l3_info.ipv4.ipsa_masklen);
        IPFIX_IPV4_MASK(p_key->l3_info.ipv4.ipsa, p_key->l3_info.ipv4.ipsa_masklen);
        SetDsIpfixL3Ipv4HashKey(V, uIpSa_g1_ipSa_f, &ipv4_key, p_key->l3_info.ipv4.ipsa);
    }

    /*gen l4 key*/
    SetDsIpfixL3Ipv4HashKey(V, layer3HeaderProtocol_f, &ipv4_key, p_key->l4_info.type.ip_protocol);

    if (field_sel.u.ipv4.icmp_code || field_sel.u.ipv4.icmp_type)
    {
        l4_srcport = (p_key->l4_info.icmp.icmpcode) & 0xff;
        l4_srcport |=  (((p_key->l4_info.icmp.icmp_type) & 0xff)<<8);
        SetDsIpfixL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, &ipv4_key, l4_srcport);
    }
    else if(field_sel.u.ipv4.igmp_type)
    {
        l4_srcport = p_key->l4_info.igmp.igmp_type<<8;
        SetDsIpfixL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, &ipv4_key, l4_srcport);
    }
    else if (field_sel.u.ipv4.gre_key || field_sel.u.ipv4.nvgre_key)
    {
        SetDsIpfixL3Ipv4HashKey(V, uL4_gKey_greKey_f, &ipv4_key, p_key->l4_info.gre_key);
    }
    else if (field_sel.u.ipv4.vxlan_vni)
    {
        SetDsIpfixL3Ipv4HashKey(V, uL4_gVxlan_vni_f, &ipv4_key, p_key->l4_info.vni);
    }
    else if(field_sel.u.ipv4.l4_src_port || field_sel.u.ipv4.l4_dst_port)
    {
        SetDsIpfixL3Ipv4HashKey(V, uL4_gPort_l4DestPort_f, &ipv4_key, p_key->l4_info.l4_port.dest_port);
        SetDsIpfixL3Ipv4HashKey(V, uL4_gPort_l4SourcePort_f, &ipv4_key, p_key->l4_info.l4_port.source_port);
    }

    /*share mode*/
    switch(GetIpfixL3Ipv4HashFieldSelect(V,shareFieldMode_f,&hash_field))
    {
        case 0:
            tcp_flag_mask = GetIpfixL3Ipv4HashFieldSelect(V, u1_g1_tcpFlagsEn_f, &hash_field);
            SetDsIpfixL3Ipv4HashKey(V, uShareField_g1_fragInfo_f, &ipv4_key, SYS_IPFIX_MAP_IP_FRAG(p_key->l3_info.ip_frag));
            SetDsIpfixL3Ipv4HashKey(V, uShareField_g1_tcpFlags_f, &ipv4_key, (p_key->l4_info.tcp_flags & tcp_flag_mask));
            break;
        case 1:
            SetDsIpfixL3Ipv4HashKey(V, uShareField_g2_ipIdentification_f, &ipv4_key, p_key->l3_info.ipv4.ip_identification);
            break;
        case 2:
            SetDsIpfixL3Ipv4HashKey(V, uShareField_g3_fragInfo_f, &ipv4_key, SYS_IPFIX_MAP_IP_FRAG(p_key->l3_info.ip_frag));
            SetDsIpfixL3Ipv4HashKey(V, uShareField_g3_ipLength_f, &ipv4_key, p_key->l3_info.ipv4.ip_pkt_len);
            break;
        default:
            SetDsIpfixL3Ipv4HashKey(V, uShareField_g4_fragInfo_f, &ipv4_key, SYS_IPFIX_MAP_IP_FRAG(p_key->l3_info.ip_frag));
            SetDsIpfixL3Ipv4HashKey(V, uShareField_g4_vrfId_f, &ipv4_key, p_key->l3_info.vrfid);
            break;
    }
    sal_memcpy((uint8*)p_data, (uint8*)&ipv4_key, sizeof(ipv4_key));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_encode_mpls_hashkey(uint8 lchip, ctc_ipfix_data_t* p_key, void* p_data)
{
    DsIpfixL3MplsHashKey_m mpls_key;
    uint16 global_port = 0;

    sal_memset(&mpls_key, 0, sizeof(mpls_key));

    /* process gport, logic-port, metadata */
    if (p_key->port_type == CTC_IPFIX_PORT_TYPE_GPORT)
    {
        SYS_GLOBAL_PORT_CHECK(p_key->gport);
        global_port = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_key->gport);
    }
    else if(p_key->port_type == CTC_IPFIX_PORT_TYPE_LOGIC_PORT)
    {
        CTC_MAX_VALUE_CHECK(p_key->logic_port, 0x3FFF);
        global_port = p_key->logic_port;
    }
    else if(p_key->port_type == CTC_IPFIX_PORT_TYPE_METADATA)
    {
        CTC_MAX_VALUE_CHECK(p_key->logic_port, 0x3FFF);
        global_port = p_key->logic_port;
    }


    SetDsIpfixL3MplsHashKey(V, hashKeyType_f, &mpls_key, SYS_IPFIX_HASH_TYPE_MPLS);
    SetDsIpfixL3MplsHashKey(V, flowFieldSel_f, &mpls_key, p_key->field_sel_id);
    SetDsIpfixL3MplsHashKey(V, isEgressKey_f, &mpls_key, ((p_key->dir==CTC_EGRESS)?1:0));
    SetDsIpfixL3MplsHashKey(V, globalPort_f, &mpls_key, global_port);
    SetDsIpfixL3MplsHashKey(V, ipfixCfgProfileId_f, &mpls_key, p_key->profile_id);

    /*gen mpls key*/
    SetDsIpfixL3MplsHashKey(V, labelNum_f, &mpls_key, p_key->l3_info.mpls.label_num);

    SetDsIpfixL3MplsHashKey(V, mplsExp0_f, &mpls_key, p_key->l3_info.mpls.label[0].exp);
    SetDsIpfixL3MplsHashKey(V, mplsLabel0_f, &mpls_key, p_key->l3_info.mpls.label[0].label);
    SetDsIpfixL3MplsHashKey(V, mplsSbit0_f, &mpls_key, p_key->l3_info.mpls.label[0].sbit);
    SetDsIpfixL3MplsHashKey(V, mplsTtl0_f, &mpls_key, p_key->l3_info.mpls.label[0].ttl);
    SetDsIpfixL3MplsHashKey(V, mplsExp1_f, &mpls_key, p_key->l3_info.mpls.label[1].exp);
    SetDsIpfixL3MplsHashKey(V, mplsLabel1_f, &mpls_key, p_key->l3_info.mpls.label[1].label);
    SetDsIpfixL3MplsHashKey(V, mplsSbit1_f, &mpls_key, p_key->l3_info.mpls.label[1].sbit);
    SetDsIpfixL3MplsHashKey(V, mplsTtl1_f, &mpls_key, p_key->l3_info.mpls.label[1].ttl);
    SetDsIpfixL3MplsHashKey(V, mplsExp2_f, &mpls_key, p_key->l3_info.mpls.label[2].exp);
    SetDsIpfixL3MplsHashKey(V, mplsLabel2_f, &mpls_key, p_key->l3_info.mpls.label[2].label);
    SetDsIpfixL3MplsHashKey(V, mplsSbit2_f, &mpls_key, p_key->l3_info.mpls.label[2].sbit);
    SetDsIpfixL3MplsHashKey(V, mplsTtl2_f, &mpls_key, p_key->l3_info.mpls.label[2].ttl);

    sal_memcpy((uint8*)p_data, (uint8*)&mpls_key, sizeof(mpls_key));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_encode_ipv6_hashkey(uint8 lchip, ctc_ipfix_data_t* p_key, void* p_data)
{
    DsIpfixL3Ipv6HashKey_m ipv6_key;
    IpfixL3Ipv6HashFieldSelect_m  hash_field;
    uint16 global_port = 0;
    ipv6_addr_t hw_ip6;
    uint16 l4_srcport = 0;
    uint32 cmd = 0;
    uint32 wlan[2] = {0};
    uint32 merge_data[2] = {0};
    uint8 tcp_flag_mask = 0;
    ctc_ipfix_hash_field_sel_t field_sel;

    sal_memset(&ipv6_key, 0, sizeof(ipv6_key));
    sal_memset(&hash_field, 0, sizeof(hash_field));
    sal_memset(&field_sel, 0, sizeof(field_sel));

    CTC_ERROR_RETURN(sys_usw_ipfix_get_hash_field_sel(lchip, p_key->field_sel_id, CTC_IPFIX_KEY_HASH_IPV6, &field_sel));
    /* process gport, logic-port, metadata */
    if (p_key->port_type == CTC_IPFIX_PORT_TYPE_GPORT)
    {
       SYS_GLOBAL_PORT_CHECK(p_key->gport);
        global_port = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_key->gport);
    }
    else if(p_key->port_type == CTC_IPFIX_PORT_TYPE_LOGIC_PORT)
    {
        CTC_MAX_VALUE_CHECK(p_key->logic_port, 0x3FFF);
        global_port = p_key->logic_port;
    }
    else if(p_key->port_type == CTC_IPFIX_PORT_TYPE_METADATA)
    {
        CTC_MAX_VALUE_CHECK(p_key->logic_port, 0x3FFF);
        global_port = p_key->logic_port;
    }


    SetDsIpfixL3Ipv6HashKey(V, hashKeyType0_f, &ipv6_key, SYS_IPFIX_HASH_TYPE_IPV6);
    SetDsIpfixL3Ipv6HashKey(V, hashKeyType1_f, &ipv6_key, SYS_IPFIX_HASH_TYPE_IPV6);
    SetDsIpfixL3Ipv6HashKey(V, flowFieldSel_f, &ipv6_key, p_key->field_sel_id);
    SetDsIpfixL3Ipv6HashKey(V, isEgressKey_f, &ipv6_key, ((p_key->dir==CTC_EGRESS)?1:0));
    SetDsIpfixL3Ipv6HashKey(V, globalPort_f, &ipv6_key, global_port);
    SetDsIpfixL3Ipv6HashKey(V, ipfixCfgProfileId_f, &ipv6_key, p_key->profile_id);
    SetDsIpfixL3Ipv6HashKey(V, dscp_f, &ipv6_key, p_key->l3_info.ipv6.dscp);

    cmd = DRV_IOR( IpfixL3Ipv6HashFieldSelect_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_key->field_sel_id, cmd, &hash_field));

    SetDsIpfixL3Ipv6HashKey(V, isMergeKey_f, &ipv6_key, p_key->l4_info.aware_tunnel_info_en);
    SetDsIpfixL3Ipv6HashKey(V, isVxlan_f, &ipv6_key, (p_key->l4_info.aware_tunnel_info_en) ? 0 : (!!p_key->l4_info.vni));
    SetDsIpfixL3Ipv6HashKey(V, layer3HeaderProtocol_f, &ipv6_key, p_key->l4_info.type.ip_protocol);

    if (field_sel.u.ipv6.icmp_code || field_sel.u.ipv6.icmp_type)
    {
        l4_srcport = (p_key->l4_info.icmp.icmpcode) & 0xff;
        l4_srcport |= (((p_key->l4_info.icmp.icmp_type) & 0xff) << 8);
        SetDsIpfixL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, &ipv6_key, l4_srcport);
    }
    else if(field_sel.u.ipv6.igmp_type)
    {
        SetDsIpfixL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, &ipv6_key, (p_key->l4_info.igmp.igmp_type << 8));
    }
    else if(field_sel.u.ipv6.l4_src_port || field_sel.u.ipv6.l4_dst_port)
    {
        SetDsIpfixL3Ipv6HashKey(V, uL4_gPort_l4DestPort_f, &ipv6_key, p_key->l4_info.l4_port.dest_port);
        SetDsIpfixL3Ipv6HashKey(V, uL4_gPort_l4SourcePort_f, &ipv6_key, p_key->l4_info.l4_port.source_port);
    }

    if(!p_key->l4_info.aware_tunnel_info_en)
    {
        if (field_sel.u.ipv6.gre_key || field_sel.u.ipv6.nvgre_key)
        {
            SetDsIpfixL3Ipv6HashKey(V, uL4_gKey_greKey_f, &ipv6_key, p_key->l4_info.gre_key);
        }
        else if(field_sel.u.ipv6.vxlan_vni)
        {
            SetDsIpfixL3Ipv6HashKey(V, uL4_gVxlan_vni_f, &ipv6_key, p_key->l4_info.vni);
        }
    }

    if(!GetIpfixL3Ipv6HashFieldSelect(V,ipAddressMode_f,&hash_field))
    {
        IPFIX_IPV6_MASK_LEN_CHECK(p_key->l3_info.ipv6.ipsa_masklen);
        IPFIX_IPV6_MASK_LEN_CHECK(p_key->l3_info.ipv6.ipda_masklen);

        IPFIX_IPV6_MASK(p_key->l3_info.ipv6.ipsa, p_key->l3_info.ipv6.ipsa_masklen);
        IPFIX_IPV6_MASK(p_key->l3_info.ipv6.ipda, p_key->l3_info.ipv6.ipda_masklen);

        IPFIX_SET_HW_IP6(hw_ip6, p_key->l3_info.ipv6.ipda);
        SetDsIpfixL3Ipv6HashKey(A, uIpDa_g1_ipDa_f, &ipv6_key, hw_ip6);

        IPFIX_SET_HW_IP6(hw_ip6, p_key->l3_info.ipv6.ipsa);
        SetDsIpfixL3Ipv6HashKey(A, uIpSa_g1_ipSa_f, &ipv6_key, hw_ip6);
    }
    else
    {
        if(p_key->l4_info.aware_tunnel_info_en)
        {
            SetDsIpfixL3Ipv6HashKey(V, isMergeKey_f, &ipv6_key, 1);
            if(field_sel.u.ipv6.vxlan_vni)
            {
                SetDsIpfixL3Ipv6HashKey(V, uIpDa_g2_mergeDataType_f, &ipv6_key,SYS_IPFIX_MERGE_TYPE_VXLAN);
                merge_data[0] = p_key->l4_info.vni;
                SetDsIpfixL3Ipv6HashKey(A, uIpDa_g2_mergeData_f, &ipv6_key, merge_data);
            }
            else if(field_sel.u.ipv6.gre_key || field_sel.u.ipv6.nvgre_key)
            {
                SetDsIpfixL3Ipv6HashKey(V, uIpDa_g2_mergeDataType_f, &ipv6_key,SYS_IPFIX_MERGE_TYPE_GRE);
                merge_data[0] = p_key->l4_info.gre_key;
                SetDsIpfixL3Ipv6HashKey(A, uIpDa_g2_mergeData_f, &ipv6_key, merge_data);
            }
            else
            {
                SetDsIpfixL3Ipv6HashKey(V, uIpDa_g2_mergeDataType_f, &ipv6_key,SYS_IPFIX_MERGE_TYPE_WLAN);
                if(field_sel.u.ipv6.wlan_radio_mac)
                {
                    SYS_USW_SET_HW_MAC(wlan, p_key->l4_info.wlan.radio_mac);
                }
                if(field_sel.u.ipv6.wlan_radio_id)
                {
                    wlan[1] = (wlan[1] & 0xFFFF) | (p_key->l4_info.wlan.radio_id & 0x1F) <<16;
                }
                if(field_sel.u.ipv6.wlan_ctl_packet)
                {
                    wlan[1] = (wlan[1] & 0x1FFFFF) | (p_key->l4_info.wlan.is_ctl_pkt) << 21;
                }
                SetDsIpfixL3Ipv6HashKey(A, uIpDa_g2_mergeData_f, &ipv6_key, wlan);
            }
        }
        if(field_sel.u.ipv6.src_cid)
        {
            SetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_srcCategoryIdClassfied_f,&ipv6_key, CTC_FLAG_ISSET(p_key->flags, CTC_IPFIX_DATA_SRC_CID_VALID));
            SetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_srcCategoryId_f, &ipv6_key,p_key->src_cid);
        }
        if(field_sel.u.ipv6.dst_cid)
        {
            SetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_destCategoryIdClassfied_f,&ipv6_key,CTC_FLAG_ISSET(p_key->flags, CTC_IPFIX_DATA_DST_CID_VALID));
            SetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_destCategoryId_f, &ipv6_key,p_key->dst_cid);
        }
        if(field_sel.u.ipv6.ip_frag)
        {
            SetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_fragInfo_f, &ipv6_key, SYS_IPFIX_MAP_IP_FRAG(p_key->l3_info.ip_frag));
        }
        if(field_sel.u.ipv6.ttl)
        {
            SetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_ttl_f, &ipv6_key,p_key->l3_info.ipv6.ttl);
        }
        if(field_sel.u.ipv6.ecn)
        {
            SetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_ecn_f, &ipv6_key,p_key->l3_info.ipv6.ecn);
        }
        if(field_sel.u.ipv6.flow_label)
        {
            SetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_ipv6FlowLabel_f, &ipv6_key,p_key->l3_info.ipv6.flow_label);
        }

        tcp_flag_mask = GetIpfixL3Ipv6HashFieldSelect(V, g1_tcpFlagsEn_f, &hash_field);
        SetDsIpfixL3Ipv6HashKey(V, uIpSa_g2_tcpFlags_f, &ipv6_key, (p_key->l4_info.tcp_flags & tcp_flag_mask));

        IPFIX_IPV6_MASK_LEN_CHECK(p_key->l3_info.ipv6.ipsa_masklen);
        IPFIX_IPV6_MASK_LEN_CHECK(p_key->l3_info.ipv6.ipda_masklen);

        IPFIX_IPV6_MASK(p_key->l3_info.ipv6.ipsa, p_key->l3_info.ipv6.ipsa_masklen);
        IPFIX_IPV6_MASK(p_key->l3_info.ipv6.ipda, p_key->l3_info.ipv6.ipda_masklen);

        if(field_sel.u.ipv6.ip_da && field_sel.u.ipv6.ip_da_mask <= 64)
        {
            IPFIX_SET_HW_IP6(hw_ip6, p_key->l3_info.ipv6.ipda);
            SetDsIpfixL3Ipv6HashKey(A, uIpDa_g2_ipDaPrefix_f, &ipv6_key, &(hw_ip6[2]));
        }
        if(field_sel.u.ipv6.ip_sa && field_sel.u.ipv6.ip_sa_mask <= 64)
        {
            IPFIX_SET_HW_IP6(hw_ip6, p_key->l3_info.ipv6.ipsa);
            SetDsIpfixL3Ipv6HashKey(A, uIpSa_g2_ipSaPrefix_f, &ipv6_key, &(hw_ip6[2]));
        }

    }


    sal_memcpy((uint8*)p_data, (uint8*)&ipv6_key, sizeof(ipv6_key));
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_ipfix_check_hash_key_valid(uint8 lchip, ctc_ipfix_data_t* p_key)
{
    uint8 loop;
    CTC_MAX_VALUE_CHECK(p_key->svlan, CTC_MAX_VLAN_ID);
    CTC_MAX_VALUE_CHECK(p_key->svlan_prio, CTC_MAX_COS-1);
    CTC_MAX_VALUE_CHECK(p_key->svlan_cfi, 1);
    CTC_MAX_VALUE_CHECK(p_key->cvlan, CTC_MAX_VLAN_ID);
    CTC_MAX_VALUE_CHECK(p_key->cvlan_prio, CTC_MAX_COS-1);
    CTC_MAX_VALUE_CHECK(p_key->cvlan_cfi, 1);

    CTC_MAX_VALUE_CHECK(p_key->field_sel_id, MCHIP_CAP(SYS_CAP_IPFIX_MAX_HASH_SEL_ID));
    CTC_MAX_VALUE_CHECK(p_key->dir, CTC_EGRESS);

    /*l3*/
    CTC_MAX_VALUE_CHECK(p_key->l3_info.ip_frag, CTC_IP_FRAG_MAX);
    if(p_key->l3_info.ip_frag == CTC_IP_FRAG_NON_OR_FIRST || p_key->l3_info.ip_frag == CTC_IP_FRAG_YES)
    {
        return CTC_E_INVALID_PARAM;
    }
    CTC_MAX_VALUE_CHECK(p_key->l3_info.vrfid, MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID));

    CTC_MAX_VALUE_CHECK(p_key->l3_info.ipv4.dscp, 0x3F);
    CTC_MAX_VALUE_CHECK(p_key->l3_info.ipv4.ecn, 0x3);
    CTC_MAX_VALUE_CHECK(p_key->l3_info.ipv4.ipda_masklen, 32);
    CTC_MAX_VALUE_CHECK(p_key->l3_info.ipv4.ipsa_masklen, 32);
    CTC_MAX_VALUE_CHECK(p_key->l3_info.ipv4.ip_pkt_len, 0x3FFF);

    CTC_MAX_VALUE_CHECK(p_key->l3_info.ipv6.dscp, 0x3F);
    CTC_MAX_VALUE_CHECK(p_key->l3_info.ipv6.ecn, 0x3);
    CTC_MAX_VALUE_CHECK(p_key->l3_info.ipv6.ipda_masklen, 128);
    CTC_MAX_VALUE_CHECK(p_key->l3_info.ipv6.ipsa_masklen, 128);
    CTC_MAX_VALUE_CHECK(p_key->l3_info.ipv6.flow_label, 0xFFFFF);

    CTC_MAX_VALUE_CHECK(p_key->l3_info.mpls.label_num, CTC_IPFIX_LABEL_NUM);
    for(loop=0; loop < p_key->l3_info.mpls.label_num; loop++)
    {
        CTC_MAX_VALUE_CHECK(p_key->l3_info.mpls.label[loop].exp, 0x7);
        CTC_MAX_VALUE_CHECK(p_key->l3_info.mpls.label[loop].sbit, 1);
        CTC_MAX_VALUE_CHECK(p_key->l3_info.mpls.label[loop].label, 0xFFFFF);
    }

    CTC_MAX_VALUE_CHECK(p_key->l4_info.wlan.radio_id, 0x1f);
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_ipfix_encode_hash_key(uint8 lchip, ctc_ipfix_data_t* p_key, drv_acc_in_t* p_in)
{
    CTC_MAX_VALUE_CHECK(p_key->field_sel_id, MCHIP_CAP(SYS_CAP_IPFIX_MAX_HASH_SEL_ID));
    switch(p_key->key_type)
    {
        case CTC_IPFIX_KEY_HASH_MAC:
            CTC_ERROR_RETURN(_sys_usw_ipfix_encode_l2_hashkey(lchip, p_key, p_in->data));
            break;
        case CTC_IPFIX_KEY_HASH_IPV4:
            CTC_ERROR_RETURN(_sys_usw_ipfix_encode_ipv4_hashkey(lchip, p_key, p_in->data));
            break;
        case CTC_IPFIX_KEY_HASH_IPV6:
            CTC_ERROR_RETURN(_sys_usw_ipfix_encode_ipv6_hashkey(lchip, p_key, p_in->data));
            break;
        case CTC_IPFIX_KEY_HASH_MPLS:
            CTC_ERROR_RETURN(_sys_usw_ipfix_encode_mpls_hashkey(lchip, p_key, p_in->data));
            break;
        case CTC_IPFIX_KEY_HASH_L2_L3:
            CTC_ERROR_RETURN(_sys_usw_ipfix_encode_l2l3_hashkey(lchip, p_key, p_in->data));
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipfix_encode_hash_ad(uint8 lchip, ctc_ipfix_data_t* p_key, drv_acc_in_t* p_in)
{
    DsIpfixSessionRecord_m ad_rec;
    uint32 dest_type = SYS_IPFIX_UNICAST_DEST;
    uint32 dest_chip_id = 0;
    uint32 lport = 0;
    uint32 byte_cnt[2] = {0};

    sal_memset(&ad_rec, 0, sizeof(ad_rec));
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_key->dest_gport);
    dest_chip_id = SYS_MAP_CTC_GPORT_TO_GCHIP(p_key->dest_gport);
    if (p_key->flags & CTC_IPFIX_DATA_L2_MCAST_DETECTED)
    {
        SetDsIpfixSessionRecord(V, uDestinationInfo_gL2Mcast_l2McGroupId_f , &ad_rec, p_key->dest_group_id);
        dest_type = SYS_IPFIX_L2MC_DEST;
    }
    if (p_key->flags & CTC_IPFIX_DATA_L3_MCAST_DETECTED)
    {
        SetDsIpfixSessionRecord(V, uDestinationInfo_gL3Mcast_l3McGroupId_f , &ad_rec, p_key->dest_group_id);
        dest_type = SYS_IPFIX_L3MC_DEST;
    }
    if (p_key->flags & CTC_IPFIX_DATA_BCAST_DETECTED)
    {
        SetDsIpfixSessionRecord(V, uDestinationInfo_gBcast_floodingId_f, &ad_rec, p_key->dest_group_id);
        dest_type = SYS_IPFIX_BCAST_DEST;
    }
    if (p_key->flags & CTC_IPFIX_DATA_APS_DETECTED)
    {
        SetDsIpfixSessionRecord(V, uDestinationInfo_gApsGroup_apsGroupId_f, &ad_rec, p_key->dest_group_id);
        SetDsIpfixSessionRecord(V, uDestinationInfo_gApsGroup_destType_f, &ad_rec, SYS_IPFIX_SUB_APS_DEST);
        dest_type = SYS_IPFIX_UNION_DEST;
    }
    if (p_key->flags & CTC_IPFIX_DATA_ECMP_DETECTED)
    {
        SetDsIpfixSessionRecord(V, uDestinationInfo_gEcmpGroup_ecmpGroupId_f, &ad_rec, p_key->dest_group_id);
        SetDsIpfixSessionRecord(V, uDestinationInfo_gEcmpGroup_destType_f, &ad_rec, SYS_IPFIX_SUB_ECMP_DEST);
        dest_type = SYS_IPFIX_UNION_DEST;
    }
    if (p_key->flags & CTC_IPFIX_DATA_LINKAGG_DETECTED)
    {
        SetDsIpfixSessionRecord(V, uDestinationInfo_gUcastToLagg_linkaggGroupId_f, &ad_rec, p_key->dest_group_id);
        SetDsIpfixSessionRecord(V, uDestinationInfo_gUcastToLagg_destType_f, &ad_rec, SYS_IPFIX_SUB_LINKAGG_DEST);
        dest_type = SYS_IPFIX_UNION_DEST;
    }
    else
    {
        SetDsIpfixSessionRecord(V, uDestinationInfo_gUcastToNormal_destChipId_f, &ad_rec, dest_chip_id);
        SetDsIpfixSessionRecord(V, uDestinationInfo_gUcastToNormal_destId_f, &ad_rec, lport);
    }

    byte_cnt[0] = (uint32)p_key->byte_count;
    byte_cnt[1] = (uint32)(p_key->byte_count >> 32);


    SetDsIpfixSessionRecord(V, exportReason_f, &ad_rec, p_key->export_reason);
    SetDsIpfixSessionRecord(V, expired_f, &ad_rec, (p_key->export_reason==CTC_IPFIX_REASON_EXPIRED)?1:0);
    SetDsIpfixSessionRecord(V, droppedPacket_f , &ad_rec, (p_key->flags&CTC_IPFIX_DATA_DROP_DETECTED));
    SetDsIpfixSessionRecord(V, destinationType_f, &ad_rec, dest_type);
    SetDsIpfixSessionRecord(A, byteCount_f, &ad_rec, byte_cnt);
    SetDsIpfixSessionRecord(V, packetCount_f, &ad_rec, p_key->pkt_count);
    SetDsIpfixSessionRecord(V, fragment_f , &ad_rec, (p_key->flags&CTC_IPFIX_DATA_FRAGMENT_PKT_DETECTED)?1:0);
    SetDsIpfixSessionRecord(V, minTtl_f, &ad_rec, p_key->min_ttl);
    SetDsIpfixSessionRecord(V, maxTtl_f, &ad_rec, p_key->max_ttl);
    SetDsIpfixSessionRecord(V, nonFragment_f, &ad_rec, (p_key->flags&CTC_IPFIX_DATA_FRAGMENT_PKT_DETECTED)?0:1);
    SetDsIpfixSessionRecord(V, tcpFlagsStatus_f, &ad_rec, p_key->tcp_flags);
    SetDsIpfixSessionRecord(V, isMonitorEntry_f, &ad_rec, 1);
    sal_memcpy((uint8*)p_in->data, (uint8*)&ad_rec, sizeof(ad_rec));
    return CTC_E_NONE;
}

/**
   @brief only used for write key by key
*/
STATIC int32
_sys_usw_ipfix_add_entry_by_index(uint8 lchip, ctc_ipfix_data_t* p_key, uint32 index, uint8 is_key)
{
    int32 ret = 0;
    drv_acc_in_t in;
    drv_acc_out_t out;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));
    in.data = mem_malloc(MEM_IPFIX_MODULE, 16*4);
    if (in.data == NULL)
    {
        return CTC_E_NO_MEMORY;
    }

    if(is_key)
    {
        CTC_ERROR_GOTO(_sys_usw_ipfix_encode_hash_key(lchip, p_key, &in), ret, error);
        in.tbl_id = SYS_IPFIX_GET_TAB_BY_TYPE(p_key->key_type);
    }
    else
    {
        CTC_ERROR_GOTO(_sys_usw_ipfix_encode_hash_ad(lchip, p_key, &in), ret, error);
        in.tbl_id = DsIpfixSessionRecord_t;
    }

    in.tbl_id = SYS_IPFIX_GET_TABLE(in.tbl_id, p_key->dir);
    in.index = index;
    in.type = DRV_ACC_TYPE_ADD;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    CTC_ERROR_GOTO(drv_acc_api(lchip,&in, &out), ret, error);

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Ipfix acc out, key_index:0x%x, conflict:%d, hit:%d\n",
                                                                out.key_index, out.is_conflict, out.is_hit);
error:
    mem_free(in.data);
    return ret;
}


STATIC int32
_sys_usw_ipfix_delete_entry_by_index(uint8 lchip, uint8 key_type, uint32 index, uint8 dir, uint8 is_key)
{
    int32 ret = 0;
    drv_acc_in_t in;
    drv_acc_out_t out;
    void* l2_key;
    uint32 hash_key_type;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));
    in.data = mem_malloc(MEM_IPFIX_MODULE, 16*4);
    if (in.data == NULL)
    {
        return CTC_E_NO_MEMORY;
    }

    if(is_key)
    {
        in.tbl_id = SYS_IPFIX_GET_TAB_BY_TYPE(key_type);

        in.index = index;  /*duet2 and TsingMa*/
        in.type = DRV_ACC_TYPE_LOOKUP;
        in.op_type = DRV_ACC_OP_BY_INDEX;
        in.tbl_id = SYS_IPFIX_GET_TABLE(in.tbl_id, dir);
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "tbl_id:%d\n",in.tbl_id);

        ret = drv_acc_api(lchip, &in, &out);
        if (ret < 0)
        {
            SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"ipfix acc failed! \n");
            goto error;
        }

        l2_key = (DsIpfixL2HashKey_m*)(out.data);
        hash_key_type = GetDsIpfixL2HashKey(V, hashKeyType_f, l2_key);

        if ((!out.is_hit) || (hash_key_type == 0))
        {
            SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Entry not exist, out.is_hit:%d, hash_key_type:%d\n",
                              out.is_hit, hash_key_type);
            ret = CTC_E_NOT_EXIST;
            goto error;
        }

        if ( hash_key_type !=0 && (key_type+1) != hash_key_type)
        {
            SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Entry exist, but hash key type is error\n");
            ret = CTC_E_NOT_EXIST;
            goto error;
        }
    }
    else
    {
        in.tbl_id = DsIpfixSessionRecord_t;
    }
    in.tbl_id = SYS_IPFIX_GET_TABLE(in.tbl_id, dir);
    sal_memset(in.data, 0, 16*4);
    in.index = index;
    in.type = DRV_ACC_TYPE_ADD;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    CTC_ERROR_GOTO(drv_acc_api(lchip,&in, &out), ret, error);

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Ipfix acc out, key_index:0x%x, conflict:%d, hit:%d\n",
                                                                out.key_index, out.is_conflict, out.is_hit);
error:
    mem_free(in.data);
    return ret;
}

/**
   @brief only used for lookup key by key
*/
STATIC int32
_sys_usw_ipfix_get_entry_by_key(uint8 lchip, ctc_ipfix_data_t* p_key, uint32* rst_hit, uint32* rst_key_index)
{
    int32 ret = 0;
    drv_acc_in_t in;
    drv_acc_out_t out;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (p_key->key_type >= CTC_IPFIX_KEY_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));

    in.data = mem_malloc(MEM_IPFIX_MODULE, 16*4);
    if (in.data == NULL)
    {
        return CTC_E_NO_MEMORY;
    }

    CTC_ERROR_GOTO(_sys_usw_ipfix_encode_hash_key(lchip, p_key, &in), ret, error);
    in.tbl_id = SYS_IPFIX_GET_TAB_BY_TYPE(p_key->key_type);
    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_KEY;
    in.tbl_id = SYS_IPFIX_GET_TABLE(in.tbl_id, p_key->dir);
    CTC_ERROR_GOTO(drv_acc_api(lchip,&in, &out), ret, error);

    (*rst_key_index) = out.key_index;
    if(out.is_conflict)
    {
        (*rst_hit) = 0;
        ret = CTC_E_HASH_CONFLICT;
    }
    else if (out.is_hit)
    {
        (*rst_hit) = 1;
    }
    else
    {
        (*rst_hit) = 0;
        ret = CTC_E_NOT_EXIST;
    }

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Ipfix acc out, key_index:0x%x, conflict:%d, hit:%d\n",
        out.key_index, out.is_conflict, out.is_hit);
error:
    mem_free(in.data)
    return ret;
}
STATIC int32
_sys_usw_ipfix_entry_is_local_check(uint8 lchip, ctc_ipfix_data_t* p_key)
{
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 field_id = 0;
    ds_t   hash_field = {0};
    uint32 global_port_type = 0;
    uint8  mapped_lchip = 0;

    CTC_MAX_VALUE_CHECK(p_key->field_sel_id, MCHIP_CAP(SYS_CAP_IPFIX_MAX_HASH_SEL_ID));

    switch (p_key->key_type)
    {
        case CTC_IPFIX_KEY_HASH_MAC:
            tbl_id = IpfixL2HashFieldSelect_t;
            field_id = IpfixL2HashFieldSelect_globalPortType_f;
            break;
        case CTC_IPFIX_KEY_HASH_L2_L3:
            tbl_id = IpfixL2L3HashFieldSelect_t;
            field_id = IpfixL2L3HashFieldSelect_globalPortType_f;
            break;
        case CTC_IPFIX_KEY_HASH_IPV4:
            tbl_id = IpfixL3Ipv4HashFieldSelect_t;
            field_id = IpfixL3Ipv4HashFieldSelect_globalPortType_f;
            break;
        case CTC_IPFIX_KEY_HASH_IPV6:
            tbl_id = IpfixL3Ipv6HashFieldSelect_t;
            field_id = IpfixL3Ipv6HashFieldSelect_globalPortType_f;
            break;
        case CTC_IPFIX_KEY_HASH_MPLS:
            tbl_id = IpfixL3MplsHashFieldSelect_t;
            field_id = IpfixL3MplsHashFieldSelect_globalPortType_f;
            break;

    }
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_key->field_sel_id, cmd, &hash_field));

    drv_get_field(lchip, tbl_id, field_id, &hash_field, &global_port_type);
    if(CTC_IPFIX_PORT_TYPE_GPORT == global_port_type)
    {
        if(!CTC_IS_LINKAGG_PORT(p_key->gport) && \
            (sys_usw_get_local_chip_id(SYS_MAP_CTC_GPORT_TO_GCHIP(p_key->gport), &mapped_lchip) || lchip != mapped_lchip))
        {
            return 0;
        }

    }

    return 1;
}

#define __cpu_interface_api__
/**
   @brief only used for lookup key by key
*/
int32
sys_usw_ipfix_get_entry_by_key(uint8 lchip, ctc_ipfix_data_t* p_key, uint32* p_rst_hit, uint32* p_rst_key_index)
{
    SYS_IPFIX_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_key);
    SYS_IPFIX_CHECK_DIR(p_key->dir);

    if (p_key->key_type >= CTC_IPFIX_KEY_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_ipfix_get_entry_by_key(lchip, p_key, p_rst_hit, p_rst_key_index));

    return CTC_E_NONE;
}

/**
   @brief  used for lookup key by index
*/
int32
sys_usw_ipfix_get_entry_by_index(uint8 lchip, uint32 index, uint8 dir, uint8 key_type, ctc_ipfix_data_t* p_out)
{
    int32 ret = 0;
    drv_acc_in_t in;
    drv_acc_out_t out;
    int8 hashKeyType =0;
    DsIpfixL2HashKey_m* l2_key =NULL;
    uint32 ipfix_max_entry_num = 0;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_IPFIX_INIT_CHECK(lchip);
    p_out->key_type = key_type;
    CTC_PTR_VALID_CHECK(p_out);
    if (key_type >= CTC_IPFIX_KEY_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsIpfixL2HashKey_t, &ipfix_max_entry_num));
    if (index > (ipfix_max_entry_num-1))
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "type:%d, index:%d\n", p_out->key_type, index);

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));

    in.tbl_id = SYS_IPFIX_GET_TAB_BY_TYPE(key_type);
    in.tbl_id = SYS_IPFIX_GET_TABLE(in.tbl_id, dir);
    in.index = index;
    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "tbl_id:%d\n",in.tbl_id);

    ret = drv_acc_api(lchip, &in, &out);
    if (ret < 0)
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"ipfix acc failed! \n");
        return ret;
    }

    l2_key = (DsIpfixL2HashKey_m*)(out.data);
    hashKeyType = GetDsIpfixL2HashKey(V, hashKeyType_f, l2_key);

    if ((!out.is_hit) || (hashKeyType == 0))
    {
        return CTC_E_NOT_EXIST;
    }

    if ( hashKeyType !=0 && (key_type+1) != hashKeyType)
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Entry exist, but hash key type is error\n");
        return CTC_E_NOT_EXIST;
    }

    CTC_ERROR_RETURN(_sys_usw_ipfix_decode_hashkey_ad(lchip, p_out, 0, (void*)out.data));

    return CTC_E_NONE;
}


/**
   @brief  used for lookup ad by index
*/
int32
sys_usw_ipfix_get_ad_by_index(uint8 lchip, uint32 index, ctc_ipfix_data_t* p_out)
{
    int32 ret = 0;
    drv_acc_in_t in;
    drv_acc_out_t out;
    uint32 ipfix_max_entry_num = 0;

    SYS_IPFIX_INIT_CHECK(lchip);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "index:%d\n", index);

    CTC_PTR_VALID_CHECK(p_out);
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsIpfixL2HashKey_t, &ipfix_max_entry_num));
    if (index > (ipfix_max_entry_num-1))
    {
        return CTC_E_INVALID_PARAM;

    }

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));

    in.tbl_id = SYS_IPFIX_GET_TABLE(DsIpfixSessionRecord_t, p_out->dir);
    in.index = index;
    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    ret = drv_acc_api(lchip, &in, &out);
    if (ret < 0)
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"ipfix acc failed! \n");
        return ret;
    }

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Ipfix acc out, key_index:0x%x, conflict:%d, hit:%d\n",
        out.key_index, out.is_conflict, out.is_hit);

    if (!out.is_hit)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_ipfix_decode_hashkey_ad(lchip, p_out, 1, (void*)out.data));

    return CTC_E_NONE;
}

/**
   @brief only used for write key by key
*/
int32
sys_usw_ipfix_add_entry_by_key(uint8 lchip, ctc_ipfix_data_t* p_key)
{
    int32 ret = CTC_E_NONE;
    uint32 rst_hit = 0;
    uint32 rst_hit_key_index = 0;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_IPFIX_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_key);
    SYS_IPFIX_CHECK_DIR(p_key->dir);

    SYS_IPFIX_LOCK;

    /* entry is not local, do nothing*/
    if(!_sys_usw_ipfix_entry_is_local_check(lchip, p_key))
    {
        SYS_IPFIX_UNLOCK;
        return CTC_E_NONE;
    }

    if (p_key->key_type >= CTC_IPFIX_KEY_NUM)
    {
        SYS_IPFIX_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }

    if (sys_usw_ipfix_get_flow_cnt(lchip, 0) >= SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_IPFIX))
    {
        SYS_IPFIX_UNLOCK;
        return CTC_E_NO_RESOURCE;
    }
    CTC_ERROR_GOTO(_sys_usw_ipfix_check_hash_key_valid(lchip, p_key), ret, error_pro);

    ret = _sys_usw_ipfix_get_entry_by_key(lchip, p_key, &rst_hit, &rst_hit_key_index);
    if(ret && ret != CTC_E_NOT_EXIST)
    {
        goto error_pro;
    }

    if(!rst_hit)
    {
        CTC_ERROR_GOTO(_sys_usw_ipfix_add_entry_by_index(lchip, p_key, rst_hit_key_index, 0), ret, error_pro);
        CTC_ERROR_GOTO(_sys_usw_ipfix_add_entry_by_index(lchip, p_key, rst_hit_key_index, 1), ret, error_pro);
    }
    else
    {
        ret = CTC_E_EXIST;
    }

error_pro:
    SYS_IPFIX_UNLOCK;
    return ret;
}

/**
   @brief only used for delete key by key
*/
int32
sys_usw_ipfix_delete_entry_by_key(uint8 lchip, ctc_ipfix_data_t* p_key)
{
    uint32 type = 0;
    uint32 rst_hit = 0;
    uint32 rst_key_index = 0;
    int32 ret = CTC_E_NONE;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_IPFIX_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_key);
    SYS_IPFIX_CHECK_DIR(p_key->dir);
    IPFIX_IPV4_MASK(p_key->l3_info.ipv4.ipsa, p_key->l3_info.ipv4.ipsa_masklen);
    IPFIX_IPV4_MASK(p_key->l3_info.ipv4.ipda, p_key->l3_info.ipv4.ipda_masklen);
    IPFIX_IPV6_MASK(p_key->l3_info.ipv6.ipsa, p_key->l3_info.ipv6.ipsa_masklen);
    IPFIX_IPV6_MASK(p_key->l3_info.ipv6.ipda, p_key->l3_info.ipv6.ipda_masklen);

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "type:%d\n", type);

    SYS_IPFIX_LOCK;
    /* entry is not local, do nothing*/
    if(!_sys_usw_ipfix_entry_is_local_check(lchip, p_key))
    {
        SYS_IPFIX_UNLOCK;
        return CTC_E_NONE;
    }

    type = p_key->key_type;
    if (type >= CTC_IPFIX_KEY_NUM)
    {
        SYS_IPFIX_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }
    CTC_ERROR_GOTO(_sys_usw_ipfix_check_hash_key_valid(lchip, p_key), ret, error_pro);
    /* get entry by key */
    CTC_ERROR_GOTO(_sys_usw_ipfix_get_entry_by_key(lchip, p_key, &rst_hit, &rst_key_index), ret, error_pro);

    if (rst_hit)
    {
        CTC_ERROR_GOTO(_sys_usw_ipfix_delete_entry_by_index(lchip, p_key->key_type, rst_key_index, p_key->dir, 1), ret, error_pro);
        CTC_ERROR_GOTO(_sys_usw_ipfix_delete_entry_by_index(lchip, p_key->key_type, rst_key_index, p_key->dir, 0), ret, error_pro);
    }
    else
    {
        ret = CTC_E_NOT_EXIST;
    }

error_pro:
    SYS_IPFIX_UNLOCK;
    return ret;
}

/**
   @brief only used for delete ad by index
*/
int32
sys_usw_ipfix_delete_entry_by_index(uint8 lchip, uint8 type, uint32 index, uint8 dir)
{
    uint32 ipfix_max_entry_num = 0;

    SYS_IPFIX_INIT_CHECK(lchip);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"index:0x%x, type:%d\n", index, type);

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsIpfixL2HashKey_t, &ipfix_max_entry_num));
    if (index > (ipfix_max_entry_num-1))
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_ipfix_delete_entry_by_index(lchip, type, index, dir, 1));
    return CTC_E_NONE;

}

int32
sys_usw_ipfix_delete_ad_by_index(uint8 lchip, uint32 index, uint8 dir)
{
    uint32 ipfix_max_entry_num = 0;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_IPFIX_INIT_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsIpfixL2HashKey_t, &ipfix_max_entry_num));
    if (index > (ipfix_max_entry_num-1))
    {
        return CTC_E_INVALID_PARAM;
    }
    CTC_ERROR_RETURN(_sys_usw_ipfix_delete_entry_by_index(lchip, 0, index, dir, 0));

    return CTC_E_NONE;
}
/*
    only duet2 board environment need call this function,else need use _sys_usw_ipfix_traverse_dma
*/
STATIC int32
_sys_usw_ipfix_traverse_acc(uint8 lchip, ctc_ipfix_traverse_fn fn, ctc_ipfix_traverse_t* p_data, uint8 is_remove)
{
    int32 ret = 0;
    uint32 index = 0;
    uint32 count = 0;
    void* p_key = NULL;
    void* p_ad = NULL;
    ctc_ipfix_data_t ipfix_data;
    uint32 ipfix_entry_num;
    uint32 ipfix_max_entry_num = 0;
	uint32 dump_end_entry_num = 0;
    uint32 next_index = 0;
    uint16 traverse_action = 0;
    uint8 sub_idx = 0;
    uint8 hash_type[2] = {0};
    drv_acc_in_t in;
    drv_acc_out_t out;
    uint32 temp_idx;
	uint8 dir = 0;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(p_data->user_data != NULL)
    {
        traverse_action = *(uint16*)p_data->user_data;
    }

    sal_memset(&ipfix_data, 0, sizeof(ctc_ipfix_data_t));
    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));
    p_key = (uint32*)mem_malloc(MEM_IPFIX_MODULE, sizeof(DsIpfixL3Ipv6HashKey_m));
    if (NULL == p_key)
    {
        return CTC_E_NO_MEMORY;
    }


    /* get 85bits bits entry num */
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, SYS_IPFIX_GET_TABLE(DsIpfixL2HashKey_t,dir), &ipfix_entry_num), ret, end_pro);

    if (!DRV_IS_DUET2(lchip))
    {
        ipfix_max_entry_num =  ipfix_entry_num*4; /*Ingress + Egress*/

        if (p_data->start_index < ipfix_entry_num*2 )
        {
            dir = CTC_INGRESS;
            dump_end_entry_num = ipfix_entry_num*2; /*ingress*/
        }
        else
        {
            dir = CTC_EGRESS;
            dump_end_entry_num = ipfix_max_entry_num;
        }
    }
    else
    {
        ipfix_max_entry_num = ipfix_entry_num;
        dump_end_entry_num = ipfix_max_entry_num;

    }

    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    if (p_data->start_index%4)
    {

        in.tbl_id = SYS_IPFIX_GET_TABLE(DsIpfixL2HashKey_t, dir);
        in.index = p_data->start_index;
        in.index = DRV_IS_DUET2(lchip)? in.index: ((dir == CTC_INGRESS)?in.index / 2:in.index / 2 - 8192);
        CTC_ERROR_GOTO(drv_acc_api(lchip, &in, &out), ret, end_pro);
        sal_memcpy(p_key, out.data, sizeof(DsIpfixL2HashKey_m));
        hash_type[0] = GetDsIpfixL2HashKey(V, hashKeyType_f, p_key);

        if ((hash_type[0] == SYS_IPFIX_HASH_TYPE_L2) ||
            (hash_type[0] == SYS_IPFIX_HASH_TYPE_IPV4) ||
            (hash_type[0] == SYS_IPFIX_HASH_TYPE_MPLS) )
        {
            in.tbl_id = SYS_IPFIX_GET_TABLE(DsIpfixSessionRecord_t, dir);
            sal_memset(&out, 0, sizeof(out));
            CTC_ERROR_GOTO(drv_acc_api(lchip, &in, &out), ret, end_pro);
            p_ad = out.data;

            ipfix_data.key_type = __sys_usw_ipfix_unmapping_key_type(hash_type[0]);
            _sys_usw_ipfix_decode_hashkey_ad(lchip, &ipfix_data, 0, p_key);
            _sys_usw_ipfix_decode_hashkey_ad(lchip, &ipfix_data, 1, p_ad);

            IPFIX_TRAVERSE_TYPE_PROCESS(p_data->start_index, ipfix_data.key_type, is_remove,in.dir);
            count++;
            if (count >= p_data->entry_num)
            {
                next_index = p_data->start_index + p_data->start_index%4;
                goto end_pro;
            }
        }

    }


    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipfix_max_entry_num = %d\n", ipfix_max_entry_num);
    index = p_data->start_index + p_data->start_index%4;
    if(index >= ipfix_max_entry_num)
    {
        goto end_pro;
    }

    do
    {

        sal_memset(&out, 0, sizeof(out));
        in.tbl_id = SYS_IPFIX_GET_TABLE(DsIpfixL3Ipv6HashKey_t, dir);
        in.index = index;
        in.index = DRV_IS_DUET2(lchip)? in.index: ((dir == CTC_INGRESS)?in.index / 2:in.index / 2 - 8192);
        CTC_ERROR_GOTO(drv_acc_api(lchip, &in, &out), ret, end_pro);
        sal_memcpy(p_key, out.data, sizeof(DsIpfixL3Ipv6HashKey_m));

        hash_type[0] = GetDsIpfixL3Ipv6HashKey(V, hashKeyType0_f, p_key);
        hash_type[1] = GetDsIpfixL3Ipv6HashKey(V, hashKeyType1_f, p_key);
        if ((hash_type[0] != SYS_IPFIX_HASH_TYPE_INVALID) || (hash_type[1] != SYS_IPFIX_HASH_TYPE_INVALID))
        {
            for(sub_idx=0; sub_idx < 2; sub_idx++)
            {
                if(hash_type[sub_idx] == SYS_IPFIX_HASH_TYPE_INVALID)
                {
                    continue;
                }
                sal_memset(&ipfix_data, 0, sizeof(ctc_ipfix_data_t));
                sal_memset(&out, 0, sizeof(out));
                in.tbl_id = SYS_IPFIX_GET_TABLE(DsIpfixSessionRecord_t, dir);
                in.index = index + 2*sub_idx;
                in.index = DRV_IS_DUET2(lchip)? in.index: ((dir == CTC_INGRESS)?in.index / 2:in.index / 2 - 8192);
                CTC_ERROR_GOTO(drv_acc_api(lchip, &in, &out), ret, end_pro);
                p_ad = out.data;

                ipfix_data.key_type = __sys_usw_ipfix_unmapping_key_type(hash_type[sub_idx]);
                _sys_usw_ipfix_decode_hashkey_ad(lchip, &ipfix_data, 0, (DsIpfixL2HashKey_m*)p_key + sub_idx);
                _sys_usw_ipfix_decode_hashkey_ad(lchip, &ipfix_data, 1, p_ad);

                temp_idx = index+2*sub_idx;
                IPFIX_TRAVERSE_TYPE_PROCESS(temp_idx,ipfix_data.key_type, is_remove,in.dir);
                count += (hash_type[sub_idx] == SYS_IPFIX_HASH_TYPE_L2L3 || hash_type[sub_idx] == SYS_IPFIX_HASH_TYPE_IPV6) ? 2:1;

                if (index + 2*sub_idx >= ipfix_max_entry_num)
                {
                    p_data->is_end = 1;
                    goto end_pro;
                }
                else if (count >= p_data->entry_num)
                {
                    next_index = index + 2*(sub_idx +1);
                    goto end_pro;
                }

                if(hash_type[sub_idx] == SYS_IPFIX_HASH_TYPE_L2L3 || hash_type[sub_idx] == SYS_IPFIX_HASH_TYPE_IPV6)
                {
                    break;
                }
            }
        }

        index += 4;
        next_index = index;
    }while(index < dump_end_entry_num && count < p_data->entry_num);

end_pro:
    if(index >= ipfix_max_entry_num)
    {
        p_data->is_end = 1;
    }

    p_data->next_traverse_index = next_index;

    if(p_key)
    {
        mem_free(p_key);
    }

    return ret;
}
STATIC int32
_sys_usw_ipfix_traverse_dma(uint8 lchip, ctc_ipfix_traverse_fn fn, ctc_ipfix_traverse_t* p_data, uint8 is_remove)
{
    int32 ret = 0;
    uint32 index = 0;
    uint32 count = 0;
    void* p_key = NULL;
    void* p_ad = NULL;
    void* p_buffer = NULL;
    void* p_buffer_ad = NULL;
    ctc_ipfix_data_t ipfix_data;
    uint32 ipfix_entry_num = 0;
    uint32 ipfix_max_entry_num = 0;
	uint32 dump_end_entry_num = 0;
    uint32 next_index = 0;
    uint16 traverse_action = 0;
    uint8 sub_idx = 0;
    uint8 hash_type[2] = {0};
    uint32 temp_idx;
    sys_dma_tbl_rw_t dma_rw;
    drv_work_platform_type_t platform_type;
    uint32 cfg_addr = 0;
    uint32 cfg_ad_addr = 0;
    uint32 entry_cnt = 0;
    uint16 loop = 0;
	uint16 key_index = 0;
	uint8 dir = 0;

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(p_data->user_data != NULL)
    {
        traverse_action = *(uint16*)p_data->user_data;
    }


    sal_memset(&ipfix_data, 0, sizeof(ctc_ipfix_data_t));
    MALLOC_ZERO(MEM_IPFIX_MODULE, p_buffer, 100*sizeof(DsIpfixL3Ipv6HashKey_m));
    if (NULL == p_buffer)
    {
        return CTC_E_NO_MEMORY;
    }
    MALLOC_ZERO(MEM_IPFIX_MODULE, p_buffer_ad, 200*sizeof(DsIpfixSessionRecord_m));
    if (NULL == p_buffer_ad)
    {
        mem_free(p_buffer);
        return CTC_E_NO_MEMORY;
    }


    /* get 85bits bits entry num */
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, SYS_IPFIX_GET_TABLE(DsIpfixL2HashKey_t,dir), &ipfix_entry_num), ret, end_pro);

    if (!DRV_IS_DUET2(lchip))
    {
        ipfix_max_entry_num =  ipfix_entry_num*4; /*Ingress + Egress*/

        if (p_data->start_index < ipfix_entry_num*2 )
        {
            dir = CTC_INGRESS;
            dump_end_entry_num = ipfix_entry_num*2; /*ingress*/
        }
        else
        {
            dir = CTC_EGRESS;
            dump_end_entry_num = ipfix_max_entry_num;
        }
    }
    else
    {
        ipfix_max_entry_num = ipfix_entry_num;
        dump_end_entry_num = ipfix_max_entry_num;

    }


    if (p_data->start_index%4)
    {
        drv_acc_in_t in;
        drv_acc_out_t out;

        p_key = p_buffer;
        in.tbl_id = SYS_IPFIX_GET_TABLE(DsIpfixL2HashKey_t, dir);
        in.index = p_data->start_index;
        in.index = DRV_IS_DUET2(lchip)? in.index: ((dir == CTC_INGRESS)?in.index / 2:in.index / 2 - 8192);
        CTC_ERROR_GOTO(drv_acc_api(lchip, &in, &out), ret, end_pro);
        sal_memcpy(p_key, out.data, sizeof(DsIpfixL2HashKey_m));
        hash_type[0] = GetDsIpfixL2HashKey(V, hashKeyType_f, p_key);

        if ((hash_type[0] == SYS_IPFIX_HASH_TYPE_L2) ||
            (hash_type[0] == SYS_IPFIX_HASH_TYPE_IPV4) ||
            (hash_type[0] == SYS_IPFIX_HASH_TYPE_MPLS) )
        {
            in.tbl_id = SYS_IPFIX_GET_TABLE(DsIpfixSessionRecord_t, dir);
            sal_memset(&out, 0, sizeof(out));
            CTC_ERROR_GOTO(drv_acc_api(lchip, &in, &out), ret, end_pro);
            p_ad = out.data;

            ipfix_data.key_type = __sys_usw_ipfix_unmapping_key_type(hash_type[0]);
            _sys_usw_ipfix_decode_hashkey_ad(lchip, &ipfix_data, 0, p_key);
            _sys_usw_ipfix_decode_hashkey_ad(lchip, &ipfix_data, 1, p_ad);

            IPFIX_TRAVERSE_TYPE_PROCESS(p_data->start_index, ipfix_data.key_type, is_remove,in.dir);
            count++;
            if (count >= p_data->entry_num)
            {
                next_index = p_data->start_index + p_data->start_index%4;
                goto end_pro;
            }
        }

    }


    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipfix_max_entry_num = %d\n", ipfix_max_entry_num);
    index = p_data->start_index + p_data->start_index%4;
    if(index >= ipfix_max_entry_num)
    {
        goto end_pro;
    }

    sal_memset(&dma_rw, 0, sizeof(dma_rw));
    do
    {
        /*Notice: start_index and next_traverse_index is 85bits encoding index, for dma performance, every
        Dma operation is by 340bits */
        entry_cnt = ((index + 100*4) <= dump_end_entry_num)?100:(dump_end_entry_num - index) / 4;


		key_index = (DRV_IS_DUET2(lchip))?index:index/4;
        drv_get_platform_type(lchip, &platform_type);
        if (platform_type == HW_PLATFORM)
        {
            CTC_ERROR_GOTO(drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, SYS_IPFIX_GET_TABLE(DsIpfixL3Ipv6HashKey_t, dir), SYS_IPFIX_GET_INDEX(key_index, dir), &cfg_addr), ret, end_pro);
            CTC_ERROR_GOTO(drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, SYS_IPFIX_GET_TABLE(DsIpfixSessionRecord_t,dir), SYS_IPFIX_GET_ADINDEX(index/2, dir), &cfg_ad_addr), ret, end_pro);
        }
        else
        {
            /*special process for uml*/
            cfg_addr = (SYS_IPFIX_GET_TABLE(DsIpfixL3Ipv6HashKey_t, dir) << 18)|SYS_IPFIX_GET_INDEX(key_index, dir);
            cfg_ad_addr = (SYS_IPFIX_GET_TABLE(DsIpfixSessionRecord_t,dir) << 18)| (SYS_IPFIX_GET_ADINDEX(index/2, dir));
        }

        /*Using Dma to read data, using 340bits entry read*/
        dma_rw.rflag = 1;
        dma_rw.tbl_addr = cfg_addr;
        dma_rw.entry_num = entry_cnt;
        dma_rw.entry_len = sizeof(DsIpfixL3Ipv6HashKey_m);
        dma_rw.buffer = p_buffer;
        CTC_ERROR_GOTO(sys_usw_dma_rw_table(lchip, &dma_rw), ret, end_pro);

        dma_rw.rflag = 1;
        dma_rw.tbl_addr = cfg_ad_addr;
        dma_rw.entry_num = entry_cnt*2;
        dma_rw.entry_len = sizeof(DsIpfixSessionRecord_m);
        dma_rw.buffer = p_buffer_ad;
        CTC_ERROR_GOTO(sys_usw_dma_rw_table(lchip, &dma_rw), ret, end_pro);

        for(loop=0; loop < entry_cnt; loop++)
        {
            p_key = (DsIpfixL3Ipv6HashKey_m*)p_buffer + loop;
            p_ad = (DsIpfixL3Ipv6HashKey_m*)p_buffer_ad + loop;
            hash_type[0] = GetDsIpfixL3Ipv6HashKey(V, hashKeyType0_f, p_key);
            hash_type[1] = GetDsIpfixL3Ipv6HashKey(V, hashKeyType1_f, p_key);
            if ((hash_type[0] == SYS_IPFIX_HASH_TYPE_INVALID)&&(hash_type[1] == SYS_IPFIX_HASH_TYPE_INVALID))
            {
                continue;
            }

            for(sub_idx=0; sub_idx < 2; sub_idx++)
            {
                if(hash_type[sub_idx] == SYS_IPFIX_HASH_TYPE_INVALID)
                {
                    continue;
                }
                sal_memset(&ipfix_data, 0, sizeof(ctc_ipfix_data_t));

                ipfix_data.key_type = __sys_usw_ipfix_unmapping_key_type(hash_type[sub_idx]);
                _sys_usw_ipfix_decode_hashkey_ad(lchip, &ipfix_data, 0, (DsIpfixL2HashKey_m*)p_key + sub_idx);
                _sys_usw_ipfix_decode_hashkey_ad(lchip, &ipfix_data, 1, (DsIpfixSessionRecord_m*)p_ad+sub_idx);

                temp_idx = index+loop*4+2*sub_idx;
                IPFIX_TRAVERSE_TYPE_PROCESS(temp_idx,ipfix_data.key_type, is_remove,dir);
                count += (hash_type[sub_idx] == SYS_IPFIX_HASH_TYPE_L2L3 || hash_type[sub_idx] == SYS_IPFIX_HASH_TYPE_IPV6) ? 2:1;

                if (index + 2*sub_idx >= ipfix_max_entry_num)
                {
                    p_data->is_end = 1;
                    goto end_pro;
                }
                else if (count >= p_data->entry_num)
                {

                    next_index = (index + loop*4) + 2*(sub_idx + 1);
                    goto end_pro;
                }

                if(hash_type[sub_idx] == SYS_IPFIX_HASH_TYPE_L2L3 || hash_type[sub_idx] == SYS_IPFIX_HASH_TYPE_IPV6)
                {
                    break;
                }
            }
            next_index = (index+loop*4);
        }
        index += 4*entry_cnt;
        next_index = index;

    }while(index < dump_end_entry_num && count < p_data->entry_num);

end_pro:
    if(index >= ipfix_max_entry_num)
    {
        p_data->is_end = 1;
    }

    p_data->next_traverse_index = next_index;

    if(p_buffer)
    {
        mem_free(p_buffer);
    }

    if(p_buffer_ad)
    {
        mem_free(p_buffer_ad);
    }

    return ret;
}
/**
   @brief ipfix traverse interface
*/
int32
sys_usw_ipfix_traverse(uint8 lchip, void* fn, ctc_ipfix_traverse_t* p_data, uint8 is_remove)
{
    drv_work_platform_type_t platform_type;
    int32 ret = 0;
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(fn);
    CTC_PTR_VALID_CHECK(p_data);
    SYS_IPFIX_INIT_CHECK(lchip);
    SYS_LOCAL_CHIPID_CHECK(lchip);

    if (p_data->entry_num > SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_IPFIX))
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_IPFIX_LOCK;

    drv_get_platform_type(lchip, &platform_type);

    if (DRV_IS_DUET2(lchip) && (platform_type == HW_PLATFORM))
    {
        ret = _sys_usw_ipfix_traverse_acc(lchip, fn, p_data, is_remove);
    }
    else
    {
        ret = _sys_usw_ipfix_traverse_dma(lchip, fn, p_data, is_remove);
    }

    SYS_IPFIX_UNLOCK;
    return ret;
}

int32
sys_usw_ipfix_show_status(uint8 lchip, void *user_data)
{
    uint32 count = 0;
    uint8 flow_detail = 0;
    ctc_ipfix_global_cfg_t ipfix_cfg;
    char* str[2] = {"Enable", "Disable"};
    char* str2[2] = {"Using group id", "Using vlan id"};

    SYS_IPFIX_INIT_CHECK(lchip);
    sal_memset(&ipfix_cfg, 0, sizeof(ctc_ipfix_global_cfg_t));
    CTC_ERROR_RETURN(sys_usw_ipfix_get_global_cfg(lchip, &ipfix_cfg));

    count = sys_usw_ipfix_get_flow_cnt(lchip, flow_detail);

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %u\n", "Exist Flow Cnt(160bit)", count);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %u\n", "Aging Interval(s)", (ipfix_cfg.aging_interval));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %u\n", "Pkt Cnt Threshold", (ipfix_cfg.pkt_cnt));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %"PRIu64"\n", "Bytes Cnt Threshold", (ipfix_cfg.bytes_cnt));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %u\n", "Times Interval(ms)", (ipfix_cfg.times_interval));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %s\n", "Conflict Export", (ipfix_cfg.conflict_export)?str[0]:str[1]);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %s\n", "Tcp End Detect Enable", (ipfix_cfg.tcp_end_detect_en)?str[0]:str[1]);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %s\n", "New Flow Export Enable", (ipfix_cfg.new_flow_export_en)?str[0]:str[1]);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %s\n", "Sw Learning Enable", (ipfix_cfg.sw_learning_en)?str[0]:str[1]);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %s\n", "Unkown Pkt Dest Type", (ipfix_cfg.unkown_pkt_dest_type)?str2[1]:str2[0]);
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %u\n", "Threshold", (ipfix_cfg.threshold));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %u\n", "Latency type", (ipfix_cfg.latency_type));
    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: %u\n", "Conflict Stats", (ipfix_cfg.conflict_cnt));

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-22s: 0x%x\n", "sample profile bmp", p_usw_ipfix_master[lchip]->sip_idx_bmp);

    SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n");

    return CTC_E_NONE;
}

int32
sys_usw_ipfix_show_stats(uint8 lchip, void *user_data)
{
    char* str1[8] = {"Expired stats", "Tcp Close stats", "PktCnt Overflow stats", "ByteCnt Overflow stats",
                     "Ts Overflow stats", "Conflict stats", "New Flow stats", "Dest Change stats"};
    uint8 cnt = 0;

    SYS_IPFIX_INIT_CHECK(lchip);
    for(cnt=0; cnt <8; cnt++)
    {
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s\n", str1[cnt]);
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-24s: %d\n", "--Export count", p_usw_ipfix_master[lchip]->exp_cnt_stats[cnt]);
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-24s: %"PRId64"\n", "--Packet count", p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats[cnt]);
        SYS_IPFIX_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-24s: %"PRId64"\n\n", "--Packet byte", p_usw_ipfix_master[lchip]->exp_pkt_byte_stats[cnt]);
    }
    return CTC_E_NONE;
}

int32
sys_usw_ipfix_clear_stats(uint8 lchip, void *user_data)
{
    uint8 cnt = 0;

    SYS_IPFIX_INIT_CHECK(lchip);
    for(cnt=0; cnt <8; cnt++)
    {
        p_usw_ipfix_master[lchip]->exp_cnt_stats[cnt] = 0;
        p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats[cnt] = 0;
        p_usw_ipfix_master[lchip]->exp_pkt_byte_stats[cnt]= 0;
    }
    return CTC_E_NONE;
}


#define __WARMBOOT__
int32
sys_usw_ipfix_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_ipfix_master_t* p_wb_ipfix_master;

    SYS_USW_FTM_CHECK_NEED_SYNC(lchip);
    /*syncup matser*/
    wb_data.buffer = mem_malloc(MEM_IPFIX_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_ipfix_master_t, CTC_FEATURE_IPFIX, SYS_WB_APPID_IPFIX_SUBID_MASTER);

    p_wb_ipfix_master = (sys_wb_ipfix_master_t *)wb_data.buffer;
    p_wb_ipfix_master->lchip = lchip;
    p_wb_ipfix_master->version = SYS_WB_VERSION_IPFIX;
    p_wb_ipfix_master->aging_interval = p_usw_ipfix_master[lchip]->aging_interval;
    p_wb_ipfix_master->max_ptr = p_usw_ipfix_master[lchip]->max_ptr;
    sal_memcpy(p_wb_ipfix_master->exp_cnt_stats, p_usw_ipfix_master[lchip]->exp_cnt_stats,
                                                                    sizeof(p_usw_ipfix_master[lchip]->exp_cnt_stats));
    sal_memcpy(p_wb_ipfix_master->exp_pkt_cnt_stats, p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats,
                                                                sizeof(p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats));
    sal_memcpy(p_wb_ipfix_master->exp_pkt_byte_stats, p_usw_ipfix_master[lchip]->exp_pkt_byte_stats,
                                                                sizeof(p_usw_ipfix_master[lchip]->exp_pkt_byte_stats));
    sal_memcpy(p_wb_ipfix_master->sip_interval, p_usw_ipfix_master[lchip]->sip_interval, sizeof(uint32)*8);
    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }
    return ret;
}
int32
sys_usw_ipfix_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_ipfix_master_t wb_ipfix_master;
    sys_wb_ipfix_master_t* p_wb_ipfix_master = &wb_ipfix_master;
    uint32  loop = 0;
    uint32  cmd = 0;
    DsIngressIpfixConfig_m igr_cfg;
    IpfixIngSamplingProfile_m  ipfix_sampling_cfg;
    uint32  tbl_num;
    uint32  step = IpfixIngSamplingProfile_array_1_ingSamplingMode_f - IpfixIngSamplingProfile_array_0_ingSamplingMode_f;
    uint8   dir;
    uint32  tab_id;
    uint32  sample_tab_id;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_IPFIX_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_ipfix_master_t, CTC_FEATURE_IPFIX, SYS_WB_APPID_IPFIX_SUBID_MASTER);

    /* set default value for new added fields, the default value may not be zeros */
    sal_memset(&wb_ipfix_master, 0, sizeof(sys_wb_ipfix_master_t));

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    /*restore  master*/
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "query ipfix master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_NONE;
        goto done;
    }

    sal_memcpy((uint8*)&wb_ipfix_master, (uint8*)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_IPFIX, p_wb_ipfix_master->version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    if (p_usw_ipfix_master[lchip]->max_ptr != p_wb_ipfix_master->max_ptr)
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    p_usw_ipfix_master[lchip]->aging_interval = p_wb_ipfix_master->aging_interval;
    sal_memcpy(p_usw_ipfix_master[lchip]->exp_cnt_stats, p_wb_ipfix_master->exp_cnt_stats,
                                                                    sizeof(p_usw_ipfix_master[lchip]->exp_cnt_stats));
    sal_memcpy(p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats, p_wb_ipfix_master->exp_pkt_cnt_stats,
                                                                sizeof(p_usw_ipfix_master[lchip]->exp_pkt_cnt_stats));
    sal_memcpy(p_usw_ipfix_master[lchip]->exp_pkt_byte_stats, p_wb_ipfix_master->exp_pkt_byte_stats,
                                                                sizeof(p_usw_ipfix_master[lchip]->exp_pkt_byte_stats));
    sal_memcpy(p_usw_ipfix_master[lchip]->sip_interval, p_wb_ipfix_master->sip_interval, sizeof(uint32)*8);


    /*restore spool*/
    for(dir=0; dir < CTC_BOTH_DIRECTION; dir++)
    {
        if(dir == CTC_INGRESS)
        {
            if (DRV_IS_DUET2(lchip))
            {
                tab_id = DsIngressIpfixConfig_t;
                sample_tab_id = IpfixIngSamplingProfile_t;
            }
            else
            {
                tab_id = DsIpfixConfig0_t;
                sample_tab_id = IpfixSamplingProfile0_t;
            }

        }
        else
        {
            if (DRV_IS_DUET2(lchip))
            {
                tab_id = DsEgressIpfixConfig_t;
                sample_tab_id = IpfixEgrSamplingProfile_t;
            }
            else
            {
                tab_id = DsIpfixConfig1_t;
                sample_tab_id = IpfixSamplingProfile1_t;
            }
        }
        cmd = DRV_IOR(sample_tab_id, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ipfix_sampling_cfg), ret, done);
        CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, tab_id, &tbl_num), ret, done);
        cmd = DRV_IOR(tab_id, DRV_ENTRY_FLAG);

        for(loop=0; loop < tbl_num; loop++)
        {
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &igr_cfg), ret, done);
            if(GetDsIngressIpfixConfig(V, samplingEn_f, &igr_cfg))
            {
                uint8   sample_profile_idx = GetDsIngressIpfixConfig(V, samplingProfileIndex_f, &igr_cfg);
                sys_ipfix_sample_profile_t sample_prf;

                sample_prf.dir = dir;
                if(dir == CTC_INGRESS)
                {
                    sample_prf.interval = p_usw_ipfix_master[lchip]->sip_interval[sample_profile_idx];
                }
                else
                {
                    sample_prf.interval = p_usw_ipfix_master[lchip]->sip_interval[sample_profile_idx+MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE)];
                }
                sample_prf.mode = GetIpfixIngSamplingProfile(V, \
                                            array_0_ingSamplingMode_f+step*sample_profile_idx, &ipfix_sampling_cfg);
                sample_prf.mode |= GetIpfixIngSamplingProfile(V, \
                                            array_0_ingCountBasedSamplingMode_f+step*sample_profile_idx, &ipfix_sampling_cfg) << 1;
                sample_prf.ad_index = (dir==CTC_EGRESS) ? (sample_profile_idx+MCHIP_CAP(SYS_CAP_IPFIX_MAX_SAMPLE_PROFILE)) : sample_profile_idx;

                CTC_ERROR_GOTO(ctc_spool_add(p_usw_ipfix_master[lchip]->sample_spool, &sample_prf, NULL, NULL), ret, done);

            }

        }
    }

done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return ret;
}

#endif

