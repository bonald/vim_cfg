#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <asm/byteorder.h>
#include <linux/types.h>
#include <linux/version.h>

static int dev=4;
module_param(dev, int, S_IRUGO);
MODULE_PARM_DESC(dev, "MTD device number to use");

static int ec_limit=0;
module_param(ec_limit, int, S_IRUGO);
MODULE_PARM_DESC(ec_limit, "erase counter limit to print");

#define EC_INFO KERN_ALERT
/* Erase counter header magic number (ASCII "UBI#") */
#define UBI_EC_HDR_MAGIC  0x55424923
/* Volume identifier header magic number (ASCII "UBI!") */
#define UBI_VID_HDR_MAGIC 0x55424921

struct ubi_vid_hdr {
	__be32  magic;
	__u8    version;
	__u8    vol_type;
	__u8    copy_flag;
	__u8    compat;
	__be32  vol_id;
	__be32  lnum;
	__u8    padding1[4];
	__be32  data_size;
	__be32  used_ebs;
	__be32  data_pad;
	__be32  data_crc;
	__u8    padding2[4];
	__be64  sqnum;
	__u8    padding3[12];
	__be32  hdr_crc;
} __attribute__ ((packed));

struct ubi_ec_hdr {
	__be32  magic;
	__u8    version;
	__u8    padding1[3];
	__be64  ec; /* Warning: the current limit is 31-bit anyway! */
	__be32  vid_hdr_offset;
	__be32  data_offset;
	__be32  image_seq;
	__u8    padding2[32];
	__be32  hdr_crc;
} __attribute__ ((packed));



static int __init mtd_ec_init(void)
{
	int err;
	int i;
    loff_t addr;
	uint64_t tmp;
    size_t retlen;
    struct ubi_ec_hdr *ech;
    struct ubi_vid_hdr *vidh;
    struct mtd_info *mtd;
    unsigned char *readbuf;
    int pgsize;
    int bufsize;
    int ebcnt;
    int pgcnt;
    long long ec;
    uint32_t magic, lnum; 

	printk(EC_INFO "\n");
	printk(EC_INFO "=================================================\n");
	printk(EC_INFO "MTD device: %d\n", dev);

	mtd = get_mtd_device(NULL, dev);
	if (IS_ERR(mtd)) 
    {
		err = PTR_ERR(mtd);
		printk(EC_INFO "error: cannot get MTD device\n");
		return err;
	}

	pgsize = mtd->writesize;

	tmp = mtd->size;
	do_div(tmp, mtd->erasesize);
	ebcnt = tmp;
	pgcnt = mtd->erasesize / pgsize;

	printk(EC_INFO "MTD device size %llu, eraseblock size %u, "
	       "page size %u, count of eraseblocks %u, pages per "
	       "eraseblock %u, OOB size %u\n",
	       (unsigned long long)mtd->size, mtd->erasesize,
	       pgsize, ebcnt, pgcnt, mtd->oobsize);

	bufsize = mtd->writesize * 2;
    readbuf = vmalloc(bufsize);
    if (!readbuf)
    {
        printk(EC_INFO "error: cannot allocate memory\n");
        return 0;
    }

    printk(EC_INFO "block    addr         ec      leb     \n");
    
    for (i = 0; i < ebcnt; i++)
    {
        addr = i * mtd->erasesize;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,13)
    	err = mtd->_block_isbad(mtd, addr);
#else
        err = mtd->block_isbad(mtd, addr);
#endif
    	if (err)
        {
            printk(EC_INFO "%d bad\n", i);
            continue;
        }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,13)
    	mtd->_read(mtd, addr, bufsize, &retlen, readbuf);
#else
        mtd->read(mtd, addr, bufsize, &retlen, readbuf);
#endif
        ech = (struct ubi_ec_hdr *)readbuf;
        vidh = (struct ubi_vid_hdr *)(readbuf + mtd->writesize);
        
        magic = be32_to_cpu(ech->magic);
        if(magic != UBI_EC_HDR_MAGIC)
        {
            ec = 0xffffffff;
        }
        else
        {
            ec = be64_to_cpu(ech->ec);
        }

        magic = be32_to_cpu(vidh->magic);
        if(magic != UBI_VID_HDR_MAGIC)
        {
            lnum = 0xffffffff;
        }
        else
        {
            lnum = be32_to_cpu(vidh->lnum);
        }
        
        if((ec == 0xffffffff) && (lnum == 0xffffffff))
        {
            printk(EC_INFO "%-8d %-12x unknow  unmap   \n", i, (unsigned int)addr);
        }
        
        else if (ec == 0xffffffff)
        {
            printk(EC_INFO "%-8d %-12x unknow  %-8d\n", i, (unsigned int)addr, lnum);   
        }
        else if (lnum == 0xffffffff)
        {
            if (ec >= ec_limit)
                printk(EC_INFO "%-8d %-12x %-7lld unmap   \n", i, (unsigned int)addr, ec);
        }
        else
        {
            if (ec >= ec_limit)
                printk(EC_INFO "%-8d %-12x %-7lld %-8d\n", i, (unsigned int)addr, ec, lnum);
        }
    }
    vfree(readbuf);

	return 0;
}

static void __exit mtd_ec_exit(void)
{
	return;
}
module_init(mtd_ec_init);
module_exit(mtd_ec_exit);

MODULE_LICENSE("GPL");

