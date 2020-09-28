#include "sal.h"
#include <linux/fs.h>
#include <linux/uaccess.h>

#if defined(_SAL_LINUX_KM)

struct sal_file {
    struct file *fp;
};
sal_file_t
ctc_sal_fopen(const char *pathname, const char *mode)
{
    return ctc_sal_open(pathname, O_RDWR, 0);
}

sal_file_t
ctc_sal_open(const char *pathname, int flags, sal_file_mode_t mode)
{
    struct sal_file *psf = NULL;
    mm_segment_t old_fs = get_fs();

    if (NULL == pathname) {
        return NULL;
    }

    set_fs(KERNEL_DS);

    psf = ctc_sal_calloc(sizeof(struct sal_file));
    if (NULL == psf) {
        printk("sal_open file %s error: out of memory\n", pathname);
        goto err_out;
    }

    psf->fp = filp_open(pathname, flags, mode);
    if (IS_ERR(psf->fp)) {
         /*printk("sal_open file %s error\n", pathname);*/
        goto err_out;
    }

    set_fs(old_fs);
    return psf;

err_out:
    if (NULL != psf) {
        if (psf->fp && !IS_ERR(psf->fp)) {
            filp_close(psf->fp, NULL);
        }

        sal_free(psf);
    }

    set_fs(old_fs);
    return NULL;
}

int
ctc_sal_fclose(sal_file_t ft)
{
    mm_segment_t old_fs = get_fs();

    set_fs(KERNEL_DS);

    if (NULL != ft) {
        if (ft->fp && !IS_ERR(ft->fp)) {
            filp_close(ft->fp, NULL);
        }
        sal_free(ft);
    }

    set_fs(old_fs);
    return 0;
}

ssize_t
ctc_sal_read(sal_file_t ft, void *buf, size_t count)
{
    mm_segment_t old_fs = get_fs();
    ssize_t nbytes = -1;

    if (NULL == ft || NULL == ft->fp) {
        return -1;
    }

    set_fs(KERNEL_DS);
    nbytes = vfs_read(ft->fp, buf, count, &ft->fp->f_pos);
    /*nbytes = ft->fp->f_op->read(ft->fp, buf, count, &ft->fp->f_pos);*/
    if (nbytes < 0) {
         printk("sal_read error %"PRIUPTR"\n", nbytes);
    }

    set_fs(old_fs);
    return nbytes;
}

ssize_t
ctc_sal_eof(sal_file_t ft)
{
    mm_segment_t old_fs = get_fs();
    ssize_t nbytes = -1;
    char buf;

    if (NULL == ft || NULL == ft->fp) {
        return -1;
    }

    set_fs(KERNEL_DS);
    nbytes = ft->fp->f_op->read(ft->fp, &buf, 1, &ft->fp->f_pos);

    set_fs(old_fs);
    if(nbytes == 0){/*EOF*/
        return 1;
    }else{
        return 0;
    }
}

ssize_t
ctc_sal_write(sal_file_t ft, void *buf, size_t count)
{
    mm_segment_t old_fs = get_fs();
    ssize_t nbytes = -1;

    if (NULL == ft || NULL == ft->fp) {
        return -1;
    }

    set_fs(KERNEL_DS);
    nbytes = ft->fp->f_op->write(ft->fp, buf, count, &ft->fp->f_pos);
    if (nbytes < 0) {
         printk("sal_write error\n");
    }

    set_fs(old_fs);
    return nbytes;
}

char *ctc_sal_fgets(char *buf, int size, sal_file_t ft)
{
    mm_segment_t old_fs = get_fs();
    ssize_t nbytes = -1;
    char rbuf;
    int idx = 0;

    if (NULL == ft || NULL == ft->fp) {
        return NULL;
    }

    set_fs(KERNEL_DS);

    buf[0] = '\0';
    while (idx < (size - 1)) {
        nbytes = ft->fp->f_op->read(ft->fp, &rbuf, 1, &ft->fp->f_pos);
        if (1 != nbytes) {
            if (0 == nbytes) {
                /* EOF */
                break;
            }

            printk("sal_fgets error\n");
            goto err_out;
        }

        buf[idx++] = rbuf;

        if ('\n' == rbuf) {
            break;
        }
    }


    set_fs(old_fs);
    if (0 == idx) {
        return NULL;
    }

    buf[idx] = '\0';
    return buf;

err_out:
    set_fs(old_fs);
    return NULL;
}

int ctc_sal_fseek(sal_file_t ft, long offset, int fromwhere)
{
    return generic_file_llseek(ft->fp,offset,fromwhere);
}

long ctc_sal_ftell(sal_file_t ft)
{
    return generic_file_llseek(ft->fp,0, SEEK_CUR);
}
#endif /* _SAL_LINUX_KM */
