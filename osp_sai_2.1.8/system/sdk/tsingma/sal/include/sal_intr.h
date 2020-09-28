#ifndef __SAL_INTR_H__
#define __SAL_INTR_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file sal_intr.h
 */

/**
 * @defgroup intr Interrupt Locking
 * @{
 */

/**
 * Interrupt flags
 */
typedef unsigned long sal_iflags_t;


#if defined(_SAL_LINUX_KM)
static INLINE sal_iflags_t
ctc_sal_hwi_lock(void)
{
    unsigned long iflags;

    local_irq_save(iflags);

    return iflags;
}

static INLINE void
ctc_sal_hwi_unlock(sal_iflags_t iflags)
{
    local_irq_restore(iflags);
}

static INLINE void
ctc_sal_swi_lock(void)
{
    local_bh_disable();
}

static INLINE void
ctc_sal_swi_unlock(void)
{
    local_bh_enable();
}

#elif defined(_SAL_LINUX_UM) /* _SAL_LINUX_UM */
static INLINE sal_iflags_t
ctc_sal_hwi_lock(void)
{
    return 0;
}

static INLINE void
ctc_sal_hwi_unlock(sal_iflags_t iflags)
{
    (void)iflags;
}

static INLINE void
ctc_sal_swi_lock(void)
{
}

static INLINE void
ctc_sal_swi_unlock(void)
{
}

#endif

/**@}*/ /* End of @defgroup intr */

#ifdef __cplusplus
}
#endif

#endif /* !__SAL_INTR_H__ */

