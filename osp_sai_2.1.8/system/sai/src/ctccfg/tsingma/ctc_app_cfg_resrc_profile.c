#include "ctc_error.h"
#include "ctc_macro.h"
#include "ctc_app_cfg_resrc_profile.h"
#include "ctc_app_cfg_parse.h"


typedef struct ctc_key_name_value_pair_s
{
    const char* key_name;
    uint32      key_value;

} ctc_key_name_value_pair_t;

ctc_key_name_value_pair_t g_igs_pool_index[] =
{
        {"IGS_DEFAULT_POOL", CTC_QOS_IGS_RESRC_DEFAULT_POOL},
        {"IGS_NON_DROP_POOL", CTC_QOS_IGS_RESRC_NON_DROP_POOL},
        {"IGS_MIN_POOL", CTC_QOS_IGS_RESRC_MIN_POOL},
        {"IGS_C2C_POOL", CTC_QOS_IGS_RESRC_C2C_POOL},
        {"IGS_ROUND_TRIP_POOL", CTC_QOS_IGS_RESRC_ROUND_TRIP_POOL},
        {"IGS_CONTROL_POOL", CTC_QOS_IGS_RESRC_CONTROL_POOL}
};


ctc_key_name_value_pair_t g_egs_pool_index[] =
{
        {"EGS_DEFAULT_POOL", CTC_QOS_EGS_RESRC_DEFAULT_POOL},
        {"EGS_NON_DROP_POOL", CTC_QOS_EGS_RESRC_NON_DROP_POOL},
        {"EGS_SPAN_POOL", CTC_QOS_EGS_RESRC_SPAN_POOL},
        {"EGS_CONTROL_POOL", CTC_QOS_EGS_RESRC_CONTROL_POOL},
        {"EGS_C2C_POOL", CTC_QOS_EGS_RESRC_C2C_POOL}
};


/*--------------------------------Define--------------------------------------*/

int32
ctc_app_get_resrc_profile_global_info(ctc_app_parse_file_t* pfile,
                        ctc_qos_resrc_pool_cfg_t* profile_info)
{
    uint8  result_num = 0;
    uint32 val = 0;
    const char* field_sub = NULL;

    result_num = 1;
    ctc_app_parse_file(pfile,
                       "IGS_POOL_MODE",
                       field_sub,
                       &val,
                       &result_num);

    profile_info->igs_pool_mode = val;

    result_num = 1;
    ctc_app_parse_file(pfile,
                       "EGS_POOL_MODE",
                       field_sub,
                       &val,
                       &result_num);

    profile_info->egs_pool_mode = val;

    return CTC_E_NONE;
}



int32
ctc_app_get_igs_resrc_pool(ctc_app_parse_file_t* pfile,
                        ctc_qos_resrc_pool_cfg_t* profile_info)
{
    uint8   index                  = 0;
    uint8   pool_index             = 0;
    int32   ret                    = 0;
    uint8   result_num             = 0;


    for(index = 0; index < 6; index++)
    {
        pool_index = g_igs_pool_index[index].key_value;

        result_num = 1;
        ret = ctc_app_parse_file(pfile,
                       g_igs_pool_index[index].key_name,
                       "POOL_SIZE",
                       &profile_info->igs_pool_size[pool_index],
                       &result_num);
    }

    return ret;
}



int32
ctc_app_get_egs_resrc_pool(ctc_app_parse_file_t* pfile,
                        ctc_qos_resrc_pool_cfg_t* profile_info)
{
    uint8   index                  = 0;
    uint8   pool_index             = 0;
    uint8   congest_level          = 0;
    int32   ret                    = 0;
    uint8   result_num             = 0;
    int32   arr_congest_thrd[CTC_RESRC_MAX_CONGEST_LEVEL_NUM];
    int32   arr_g_thrd[CTC_RESRC_MAX_CONGEST_LEVEL_NUM];
    int32   arr_y_thrd[CTC_RESRC_MAX_CONGEST_LEVEL_NUM];
    int32   arr_r_thrd[CTC_RESRC_MAX_CONGEST_LEVEL_NUM];
    int32   arr_c_thrd[CTC_RESRC_MAX_CONGEST_LEVEL_NUM];
    ctc_qos_resrc_drop_profile_t *p_drop_profile;

    for(index = 0; index < 5; index++)
    {
        sal_memset(arr_congest_thrd, 0, sizeof(arr_congest_thrd));
        sal_memset(arr_g_thrd, 0, sizeof(arr_g_thrd));
        sal_memset(arr_y_thrd, 0, sizeof(arr_y_thrd));
        sal_memset(arr_r_thrd, 0, sizeof(arr_r_thrd));
        sal_memset(arr_c_thrd, 0, sizeof(arr_c_thrd));

        pool_index = g_egs_pool_index[index].key_value;

        result_num = 1;
        ret = ctc_app_parse_file(pfile,
                       g_egs_pool_index[index].key_name,
                       "POOL_SIZE",
                       &profile_info->egs_pool_size[pool_index],
                       &result_num);

        result_num = CTC_RESRC_MAX_CONGEST_LEVEL_NUM;
        ret = ctc_app_parse_file(pfile,
                       g_egs_pool_index[index].key_name,
                       "CONGEST_THRD",
                       arr_congest_thrd,
                       &result_num);
        ret = ctc_app_parse_file(pfile,
                       g_egs_pool_index[index].key_name,
                       "G_THRD",
                       arr_g_thrd,
                       &result_num);
        ret = ctc_app_parse_file(pfile,
                       g_egs_pool_index[index].key_name,
                       "Y_THRD",
                       arr_y_thrd,
                       &result_num);
        ret = ctc_app_parse_file(pfile,
                       g_egs_pool_index[index].key_name,
                       "R_THRD",
                       arr_r_thrd,
                       &result_num);
        ret = ctc_app_parse_file(pfile,
                       g_egs_pool_index[index].key_name,
                       "C_THRD",
                       arr_c_thrd,
                       &result_num);


        p_drop_profile = &profile_info->drop_profile[pool_index];

        for (congest_level = 0; congest_level < result_num; congest_level++)
        {
            p_drop_profile->congest_level_num = result_num;
            p_drop_profile->congest_threshold[congest_level] = arr_congest_thrd[congest_level];
            p_drop_profile->queue_drop[congest_level].max_th[0] = arr_r_thrd[congest_level];
            p_drop_profile->queue_drop[congest_level].max_th[1] = arr_y_thrd[congest_level];
            p_drop_profile->queue_drop[congest_level].max_th[2] = arr_g_thrd[congest_level];
            p_drop_profile->queue_drop[congest_level].max_th[3] = arr_c_thrd[congest_level];
        }

    }

    return ret;
}


int32
ctc_app_get_resrc_profile(const uint8* file_name,
                     ctc_qos_resrc_pool_cfg_t* profile_info)
{
    int32   ret;
    ctc_app_parse_file_t file;

    CTC_PTR_VALID_CHECK(file_name);
    CTC_PTR_VALID_CHECK(profile_info);

    ret = ctc_app_parse_open_file((const char*)file_name, &file);
    if (ret != CTC_E_NONE)
    {
        return ret;
    }

    ctc_app_get_resrc_profile_global_info(&file, profile_info);
    ctc_app_get_igs_resrc_pool(&file, profile_info);
    ctc_app_get_egs_resrc_pool(&file, profile_info);
    ctc_app_parse_close_file(&file);

    return CTC_E_NONE;
}


