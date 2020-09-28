

#define OPM_PTR_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    {\
        sal_printf("Invaid pointer, %s:%d", __FUNCTION__, __LINE__);\
        return(PM_E_INVALID_PARAM); \
    }\
}while(0)

