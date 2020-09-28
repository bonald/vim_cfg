#include "libbb.h"
#include "onie_tlvinfo.h"
#include "sys_eeprom.h"
#include <mtd/mtd-user.h>

/*
 * read_sys_eeprom - read the hwinfo from MTD EEPROM
 */
int read_sys_eeprom(void *eeprom_data, int offset, int len)
{
    struct mtd_info_user mtdinfo;
    int rc;
    int ret = 0;
    int fd;
    u_int8_t *c;

    c = eeprom_data;

    fd = open(SYS_EEPROM_MTD_DEVICE, O_RDONLY);
    if (fd < 0) {
	fprintf (stderr, "Can't open %s: %s\n",
		SYS_EEPROM_MTD_DEVICE, strerror (errno));
	return -1;
    }

    rc = ioctl(fd, MEMGETINFO, &mtdinfo);

    if (rc < 0) {
        perror ("Cannot get MTD information");
        return -1;
    }

    if (mtdinfo.type != MTD_NORFLASH) {
        fprintf (stderr, "Unsupported flash type %u\n", mtdinfo.type);
        return -1;
    }

    if (lseek (fd, offset + SYS_EEPROM_OFFSET, SEEK_SET) == -1) {
	fprintf (stderr, "Seek error on %s: %s\n",
		SYS_EEPROM_MTD_DEVICE, strerror (errno));
	return -1;
    }

    rc = read (fd, c, len);
    if (rc != len) {
        fprintf (stderr, "Read error on %s: %s\n",
		SYS_EEPROM_MTD_DEVICE, strerror (errno));
        return -1;
    }

    if (close (fd)) {
	fprintf (stderr, "I/O error on %s: %s\n",
		SYS_EEPROM_MTD_DEVICE, strerror (errno));
	return -1;
    }

    return ret;
}

/*
 * write_sys_eeprom - write the hwinfo to MTD EEPROM
 */
int write_sys_eeprom(void *eeprom_data, int len)
{
    struct mtd_info_user mtdinfo;
    struct erase_info_user erase;
    int rc;
    int ret = 0;
    int fd;
    u_int8_t *c;

    c = eeprom_data;

    fd = open(SYS_EEPROM_MTD_DEVICE, O_RDWR);
    if (fd < 0) {
	fprintf (stderr, "Can't open %s: %s\n",
		SYS_EEPROM_MTD_DEVICE, strerror (errno));
	return -1;
    }

    rc = ioctl(fd, MEMGETINFO, &mtdinfo);

    if (rc < 0) {
        perror ("Cannot get MTD information");
        return -1;
    }

    if (mtdinfo.type != MTD_NORFLASH) {
        fprintf (stderr, "Unsupported flash type %u\n", mtdinfo.type);
        return -1;
    }

    /*
     * erase entire sys_eeprom
     */
    erase.length = mtdinfo.size;
    erase.start = 0;
    ioctl (fd, MEMUNLOCK, &erase);

    if (ioctl (fd, MEMERASE, &erase) != 0) {
	fprintf (stderr, "MTD erase error on %s: %s\n",
		SYS_EEPROM_MTD_DEVICE, strerror (errno));
	return -1;
    }

    if (lseek (fd, SYS_EEPROM_OFFSET, SEEK_SET) == -1) {
	fprintf (stderr, "Seek error on %s: %s\n",
		SYS_EEPROM_MTD_DEVICE, strerror (errno));
	return -1;
    }

    rc = write (fd, c, len);
    if (rc != len) {
        fprintf (stderr, "Write error on %s: %s\n",
		SYS_EEPROM_MTD_DEVICE, strerror (errno));
        return -1;
    }

    if (close (fd)) {
	fprintf (stderr, "I/O error on %s: %s\n",
		SYS_EEPROM_MTD_DEVICE, strerror (errno));
	return -1;
    }

    return ret;
}
