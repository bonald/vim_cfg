#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h> 
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#include "libfdt_env.h"
#include "fdt.h"
#include "libfdt.h"
#include "libfdt_internal.h"


#if 0
#define DEBUG(x,args...)  printf(x,##args)
#else
#define DEBUG(x,args...) 
#endif

#define uswap_16(x) \
	((((x) & 0xff00) >> 8) | \
	 (((x) & 0x00ff) << 8))
#define uswap_32(x) \
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24))
#define _uswap_64(x, sfx) \
	((((x) & 0xff00000000000000##sfx) >> 56) | \
	 (((x) & 0x00ff000000000000##sfx) >> 40) | \
	 (((x) & 0x0000ff0000000000##sfx) >> 24) | \
	 (((x) & 0x000000ff00000000##sfx) >>  8) | \
	 (((x) & 0x00000000ff000000##sfx) <<  8) | \
	 (((x) & 0x0000000000ff0000##sfx) << 24) | \
	 (((x) & 0x000000000000ff00##sfx) << 40) | \
	 (((x) & 0x00000000000000ff##sfx) << 56))
	 
# define cpu_to_le16(x)		(x)
# define cpu_to_le32(x)		(x)
# define cpu_to_le64(x)		(x)
# define le16_to_cpu(x)		(x)
# define le32_to_cpu(x)		(x)
# define le64_to_cpu(x)		(x)
# define cpu_to_be16(x)		uswap_16(x)
# define cpu_to_be32(x)		uswap_32(x)
# define cpu_to_be64(x)		uswap_64(x)
# define be16_to_cpu(x)		uswap_16(x)
# define be32_to_cpu(x)		uswap_32(x)
# define be64_to_cpu(x)		uswap_64(x)
	 
#define uimage_to_cpu(x)		be32_to_cpu(x)
#define cpu_to_uimage(x)		cpu_to_be32(x)
#define IMAGE_INDENT_STRING	"   "

#define FIT_IMAGES_PATH		"/images"
#define FIT_CONFS_PATH		"/configurations"

/* hash/signature node */
#define FIT_HASH_NODENAME	"hash"
#define FIT_ALGO_PROP		"algo"
#define FIT_VALUE_PROP		"value"
#define FIT_IGNORE_PROP		"uboot-ignore"
#define FIT_SIG_NODENAME	"signature"

/* image node */
#define FIT_DATA_PROP		"data"
#define FIT_DATA_OFFSET_PROP	"data-offset"
#define FIT_DATA_SIZE_PROP	"data-size"
#define FIT_TIMESTAMP_PROP	"timestamp"
#define FIT_DESC_PROP		"description"
#define FIT_ARCH_PROP		"arch"
#define FIT_TYPE_PROP		"type"
#define FIT_OS_PROP		"os"
#define FIT_COMP_PROP		"compression"
#define FIT_ENTRY_PROP		"entry"
#define FIT_LOAD_PROP		"load"

/* configuration node */
#define FIT_KERNEL_PROP		"kernel"
#define FIT_RAMDISK_PROP	"ramdisk"
#define FIT_FDT_PROP		"fdt"
#define FIT_LOADABLE_PROP	"loadables"
#define FIT_DEFAULT_PROP	"default"
#define FIT_SETUP_PROP		"setup"
#define FIT_FPGA_PROP		"fpga"

#define HASH_MAX_DIGEST_SIZE	32
#define FIT_MAX_HASH_LEN	HASH_MAX_DIGEST_SIZE

#define IMAGE_ENABLE_TIMESTAMP 1
#define IMAGE_ENABLE_VERIFY 0

/* crc 32 table */
const long ctclib_crc_table[256] = 
{
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

#define DO1(buf) crc = ctclib_crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);
/*******************************************************************************
 * Name:    ctclib_gen_crc32
 * Purpose:   calculate crc32
 * Input: 
 *   crc: crc
 *   buf: buffer
 *   len: buffer length
 * Output: 
 * Return: crc
 * Note: 
 ******************************************************************************/
uint32_t
ctclib_gen_crc32(uint32_t crc, const unsigned char *buf, size_t len)
{
    crc = crc ^ 0xffffffffL;
    while (len >= 8)
    {
      DO8(buf);
      len -= 8;
    }
    if (len) do {
      DO1(buf);
    } while (--len);
    return crc ^ 0xffffffffL;
}

static inline const char *fit_get_name(const void *fit_hdr,
		int noffset, int *len)
{
	return fdt_get_name(fit_hdr, noffset, len);
}

static void fit_get_debug(const void *fit, int noffset,
		char *prop_name, int err)
{
	DEBUG("Can't get '%s' property from FIT 0x%08lx, node: offset %d, name %s (%s)\n",
	      prop_name, (ulong)fit, noffset, fit_get_name(fit, noffset, NULL),
	      fdt_strerror(err));
}

int fit_get_timestamp(const void *fit, int noffset, time_t *timestamp)
{
	int len;
	const void *data;

	data = fdt_getprop(fit, noffset, FIT_TIMESTAMP_PROP, &len);
	if (data == NULL) {
		fit_get_debug(fit, noffset, FIT_TIMESTAMP_PROP, len);
		return -1;
	}
	if (len != sizeof(uint32_t)) {
		DEBUG("FIT timestamp with incorrect size of (%u)\n", len);
		return -2;
	}

	*timestamp = uimage_to_cpu(*((uint32_t *)data));
	return 0;
}
int fit_get_desc(const void *fit, int noffset, char **desc)
{
	int len;

	*desc = (char *)fdt_getprop(fit, noffset, FIT_DESC_PROP, &len);
	if (*desc == NULL) {
		fit_get_debug(fit, noffset, FIT_DESC_PROP, len);
		return -1;
	}

	return 0;
}

int fit_image_hash_get_algo(const void *fit, int noffset, char **algo)
{
	int len;

	*algo = (char *)fdt_getprop(fit, noffset, FIT_ALGO_PROP, &len);
	if (*algo == NULL) {
		fit_get_debug(fit, noffset, FIT_ALGO_PROP, len);
		return -1;
	}

	return 0;
}

int fit_image_hash_get_value(const void *fit, int noffset, uint8_t **value,
				int *value_len)
{
	int len;

	*value = (uint8_t *)fdt_getprop(fit, noffset, FIT_VALUE_PROP, &len);
	if (*value == NULL) {
		fit_get_debug(fit, noffset, FIT_VALUE_PROP, len);
		*value_len = 0;
		return -1;
	}

	*value_len = len;
	return 0;
}

static void fit_image_print_data(const void *fit, int noffset, const char *p,
				 const char *type)
{
	const char *keyname;
	uint8_t *value;
	int value_len;
	char *algo;
	int required;
	int ret, i;

	printf("%s  %s algo:    ", p, type);
	if (fit_image_hash_get_algo(fit, noffset, &algo)) {
		printf("invalid/unsupported\n");
		return;
	}
	printf("%s", algo);
	keyname = fdt_getprop(fit, noffset, "key-name-hint", NULL);
	required = fdt_getprop(fit, noffset, "required", NULL) != NULL;
	if (keyname)
		printf(":%s", keyname);
	if (required)
		printf(" (required)");
	printf("\n");

	ret = fit_image_hash_get_value(fit, noffset, &value,
					&value_len);
	printf("%s  %s value:   ", p, type);
	if (ret) {
		printf("unavailable\n");
	} else {
		for (i = 0; i < value_len; i++)
			printf("%02x", value[i]);
		printf("\n");
	}

	DEBUG("%s  %s len:     %d\n", p, type, value_len);

	/* Signatures have a time stamp */
	if (IMAGE_ENABLE_TIMESTAMP && keyname) {
		time_t timestamp;

		printf("%s  Timestamp:    ", p);
		if (fit_get_timestamp(fit, noffset, &timestamp))
			printf("unavailable\n");
		else
			printf("%s", ctime(&timestamp));
	}
}

static void fit_image_print_verification_data(const void *fit, int noffset,
				       const char *p)
{
	const char *name;

	/*
	 * Check subnode name, must be equal to "hash" or "signature".
	 * Multiple hash/signature nodes require unique unit node
	 * names, e.g. hash@1, hash@2, signature@1, signature@2, etc.
	 */
	name = fit_get_name(fit, noffset, NULL);
	if (!strncmp(name, FIT_HASH_NODENAME, strlen(FIT_HASH_NODENAME))) {
		fit_image_print_data(fit, noffset, p, "Hash");
	} else if (!strncmp(name, FIT_SIG_NODENAME,
				strlen(FIT_SIG_NODENAME))) {
		fit_image_print_data(fit, noffset, p, "Sign");
	}
}

void fit_image_print(const void *fit, int image_noffset, const char *p)
{
	char *desc;
	//uint8_t type, arch, os, comp;
	size_t size = 0;
	//ulong load, entry;
	//const void *data;
	int noffset;
	int ndepth;
	int ret;

	/* Mandatory properties */
	ret = fit_get_desc(fit, image_noffset, &desc);
	printf("%s  Description:  ", p);
	if (ret)
		printf("unavailable\n");
	else
		printf("%s\n", desc);

	if (IMAGE_ENABLE_TIMESTAMP) {
		time_t timestamp;

		ret = fit_get_timestamp(fit, 0, &timestamp);
		printf("%s  Created:      ", p);
		if (ret)
			printf("unavailable\n");
		else
			printf("%s", ctime(&timestamp));
	}

	//fit_image_get_type(fit, image_noffset, &type);
	//printf("%s  Type:         %s\n", p, genimg_get_type_name(type));

	//fit_image_get_comp(fit, image_noffset, &comp);
	//printf("%s  Compression:  %s\n", p, genimg_get_comp_name(comp));

	//ret = fit_image_get_data(fit, image_noffset, &data, &size);


	printf("%s  Data Size:    ", p);
	if (ret)
		printf("unavailable\n");
	else
		printf("%d Bytes = %.2f KiB = %.2f MiB\n",
			(int)size, (double)size / 1.024e3,
			(double)size / 1.048576e6);

	/* Process all hash subnodes of the component image node */
	for (ndepth = 0, noffset = fdt_next_node(fit, image_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/* Direct child node of the component image node */
			fit_image_print_verification_data(fit, noffset, p);
		}
	}
}

void fit_conf_print(const void *fit, int noffset, const char *p)
{
	char *desc;
	const char *uname;
	int ret;
	int loadables_index;

	/* Mandatory properties */
	ret = fit_get_desc(fit, noffset, &desc);
	printf("%s  Description:  ", p);
	if (ret)
		printf("unavailable\n");
	else
		printf("%s\n", desc);

	uname = fdt_getprop(fit, noffset, FIT_KERNEL_PROP, NULL);
	printf("%s  Kernel:       ", p);
	if (uname == NULL)
		printf("unavailable\n");
	else
		printf("%s\n", uname);

	/* Optional properties */
	uname = fdt_getprop(fit, noffset, FIT_RAMDISK_PROP, NULL);
	if (uname)
		printf("%s  Init Ramdisk: %s\n", p, uname);

	uname = fdt_getprop(fit, noffset, FIT_FDT_PROP, NULL);
	if (uname)
		printf("%s  FDT:          %s\n", p, uname);

	uname = fdt_getprop(fit, noffset, FIT_FPGA_PROP, NULL);
	if (uname)
		printf("%s  FPGA:         %s\n", p, uname);

	/* Print out all of the specified loadables */
	for (loadables_index = 0;
	     uname = fdt_stringlist_get(fit, noffset, FIT_LOADABLE_PROP,
					loadables_index, NULL), uname;
	     loadables_index++) {
		if (loadables_index == 0) {
			printf("%s  Loadables:    ", p);
		} else {
			printf("%s                ", p);
		}
		printf("%s\n", uname);
	}
}

void fit_print_contents(const void *fit)
{
	char *desc;
	char *uname;
	int images_noffset;
	int confs_noffset;
	int noffset;
	int ndepth;
	int count = 0;
	int ret;
	const char *p;
	time_t timestamp;

	/* Indent string is defined in header image.h */
	p = IMAGE_INDENT_STRING;

	/* Root node properties */
	ret = fit_get_desc(fit, 0, &desc);
	printf("%sFIT description: ", p);
	if (ret)
		printf("unavailable\n");
	else
		printf("%s\n", desc);

	if (IMAGE_ENABLE_TIMESTAMP) {
		ret = fit_get_timestamp(fit, 0, &timestamp);
		printf("%sCreated:         ", p);
		if (ret)
			printf("unavailable\n");
		else
			printf("%s", ctime(&timestamp));
	}

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return;
	}

	/* Process its subnodes, print out component images details */
	for (ndepth = 0, count = 0,
		noffset = fdt_next_node(fit, images_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the images parent node,
			 * i.e. component image node.
			 */
			printf("%s Image %u (%s)\n", p, count++,
			       fit_get_name(fit, noffset, NULL));

			fit_image_print(fit, noffset, p);
		}
	}

	/* Find configurations parent node offset */
	confs_noffset = fdt_path_offset(fit, FIT_CONFS_PATH);
	if (confs_noffset < 0) {
		DEBUG("Can't get configurations parent node '%s' (%s)\n",
		      FIT_CONFS_PATH, fdt_strerror(confs_noffset));
		return;
	}

	/* get default configuration unit name from default property */
	uname = (char *)fdt_getprop(fit, noffset, FIT_DEFAULT_PROP, NULL);
	if (uname)
		printf("%s Default Configuration: '%s'\n", p, uname);

	/* Process its subnodes, print out configurations details */
	for (ndepth = 0, count = 0,
		noffset = fdt_next_node(fit, confs_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the configurations parent node,
			 * i.e. configuration node.
			 */
			printf("%s Configuration %u (%s)\n", p, count++,
			       fit_get_name(fit, noffset, NULL));

			fit_conf_print(fit, noffset, p);
		}
	}
}

int fit_image_get_node(const void *fit, const char *image_uname)
{
	int noffset, images_noffset;

	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		DEBUG("Can't find images parent node '%s' (%s)\n",
		      FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return images_noffset;
	}

	noffset = fdt_subnode_offset(fit, images_noffset, image_uname);
	if (noffset < 0) {
		DEBUG("Can't get node offset for image unit name: '%s' (%s)\n",
		      image_uname, fdt_strerror(noffset));
	}

	return noffset;
}


int fit_image_get_data(const void *fit, int noffset,
		const void **data, size_t *size)
{
	int len;

	*data = fdt_getprop(fit, noffset, FIT_DATA_PROP, &len);
	if (*data == NULL) {
		fit_get_debug(fit, noffset, FIT_DATA_PROP, len);
		*size = 0;
		return -1;
	}

	*size = len;
	return 0;
}

int calculate_hash(const void *data, int data_len, const char *algo,
			uint8_t *value, int *value_len)
{
	if (strcmp(algo, "crc32") == 0) {
		*((uint32_t *)value) = ctclib_gen_crc32(0, data, data_len);
		*((uint32_t *)value) = cpu_to_uimage(*((uint32_t *)value));
		*value_len = 4;
	} else {
		DEBUG("Unsupported hash alogrithm\n");
		return -1;
	}
	return 0;
}

static int fit_image_check_hash(const void *fit, int noffset, const void *data,
				size_t size, char **err_msgp)
{
	uint8_t value[FIT_MAX_HASH_LEN];
	int value_len;
	char *algo;
	uint8_t *fit_value;
	int fit_value_len;

	*err_msgp = NULL;

	if (fit_image_hash_get_algo(fit, noffset, &algo)) {
		*err_msgp = "Can't get hash algo property";
		return -1;
	}
	//printf("%s", algo);


	if (fit_image_hash_get_value(fit, noffset, &fit_value,
				     &fit_value_len)) {
		*err_msgp = "Can't get hash value property";
		return -1;
	}

	if (calculate_hash(data, size, algo, value, &value_len)) {
		*err_msgp = "Unsupported hash algorithm";
		return -1;
	}

	if (value_len != fit_value_len) {
		*err_msgp = "Bad hash value len";
		return -1;
	} else if (memcmp(value, fit_value, value_len) != 0) {
		*err_msgp = "Bad hash value";
		return -1;
	}

	return 0;
}

int fit_image_verify(const void *fit, int image_noffset)
{
	const void	*data;
	size_t		size;
	int		noffset = 0;
	char		*err_msg = "";

	/* Get image data and data length */
	if (fit_image_get_data(fit, image_noffset, &data, &size)) {
		err_msg = "Can't get image data/size";
		goto error;
	}

	/* Process all hash subnodes of the component image node */
	fdt_for_each_subnode(noffset, fit, image_noffset) {
		const char *name = fit_get_name(fit, noffset, NULL);

		/*
		 * Check subnode name, must be equal to "hash".
		 * Multiple hash nodes require unique unit node
		 * names, e.g. hash@1, hash@2, etc.
		 */
		if (!strncmp(name, FIT_HASH_NODENAME,
			     strlen(FIT_HASH_NODENAME))) {
			if (fit_image_check_hash(fit, noffset, data, size,
						 &err_msg))
				goto error;
		}
	}

	if (noffset == -FDT_ERR_TRUNCATED || noffset == -FDT_ERR_BADSTRUCTURE) {
		err_msg = "Corrupted or truncated tree";
		goto error;
	}

	return 1;

error:
	printf(" error!\n%s for '%s' hash node in '%s' image node\n",
	       err_msg, fit_get_name(fit, noffset, NULL),
	       fit_get_name(fit, image_noffset, NULL));
	return 0;
}

int fit_conf_get_node(const void *fit, const char *conf_uname)
{
	int noffset, confs_noffset;
	int len;

	confs_noffset = fdt_path_offset(fit, FIT_CONFS_PATH);
	if (confs_noffset < 0) {
		DEBUG("Can't find configurations parent node '%s' (%s)\n",
		      FIT_CONFS_PATH, fdt_strerror(confs_noffset));
		return confs_noffset;
	}

	if (conf_uname == NULL) {
		/* get configuration unit name from the default property */
		DEBUG("No configuration specified, trying default...\n");
		conf_uname = (char *)fdt_getprop(fit, confs_noffset,
						 FIT_DEFAULT_PROP, &len);
		if (conf_uname == NULL) {
			fit_get_debug(fit, confs_noffset, FIT_DEFAULT_PROP,
				      len);
			return len;
		}
		DEBUG("Found default configuration: '%s'\n", conf_uname);
	}

	noffset = fdt_subnode_offset(fit, confs_noffset, conf_uname);
	if (noffset < 0) {
		DEBUG("Can't get node offset for configuration unit name: '%s' (%s)\n",
		      conf_uname, fdt_strerror(noffset));
	}

	return noffset;
}

int fit_conf_get_prop_node(const void *fit, int noffset,
		const char *prop_name)
{
	char *uname;
	int len;

	/* get kernel image unit name from configuration kernel property */
	uname = (char *)fdt_getprop(fit, noffset, prop_name, &len);
	if (uname == NULL)
		return len;

	return fit_image_get_node(fit, uname);
}

int main(int argc, char *argv[])
{    
	int dfd;
	struct stat sbuf;
	unsigned char *ptr;
	char *datafile;
	int ret = 0;
    int noffset,cfg_noffset;
	datafile = argv[1];
    time_t timestamp;
    struct tm * local_time =NULL;
    char str[32];
	if ((dfd = open(datafile, O_RDONLY)) < 0) {
		DEBUG ("Can't open %s: %s\n",datafile, strerror(errno));
		exit(1); /* for bug 51023 modified by liuyang 2019-5-22 */
	}

	if (fstat(dfd, &sbuf) < 0) {
		DEBUG ("Can't stat %s: %s\n",datafile, strerror(errno));
		ret = -1;
        goto errorout1;
	}

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == MAP_FAILED) {
		DEBUG ("Can't read %s: %s\n",datafile, strerror(errno));
		ret = -1;
        goto errorout1;
	}

    /* for bug 51023 modified by liuyang 2019-5-22 */
    FILE* tmp;
    if(argc <3)
    {
        tmp=fopen("/tmp/.tmp_check_fit_image","w");
    }
    else
    {
        tmp=fopen(argv[2],"w");
    }
    //FILE* tmp=fopen(argv[2],"w");
    if(tmp==NULL)
    {
        DEBUG("creat file %s failed!\n",argv[2]);
		ret = -1;
        goto errorout1;
    }

	ret = fdt_check_header(ptr);
	DEBUG ("check_image %s\n",ret?"FAIL":"OK");
    fprintf(tmp,"check_image %d\n",ret);
    if(ret)
    {
		ret = -1;
        goto errorout0;
    }

#if 0
    char *desc;
    ret = fit_get_desc(ptr, 0, &desc);
	DEBUG("version ");
    fprintf(tmp,"version ");
	if (ret)
	{
		DEBUG("unknown\n");
        fprintf(tmp,"unknown\n");
	}
	else
	{
		DEBUG("%s\n", desc);
        fprintf(tmp,"%s\n", desc);
	}
#endif
    char buf[32];
    int i,n=0;
    lseek(dfd, -32, SEEK_END);
    int len = read(dfd, buf,32);
    if(len != 32)
    {
        DEBUG("version unknown\n");
    }
    for(i=0;i<len;i++)
    {
        if(i>=7)
        {
            if((buf[i-7]=='v')&&(buf[i-6]=='e')&&(buf[i-5]=='r')&&(buf[i-4]=='s')
                &&(buf[i-3]=='i')&&(buf[i-2]=='o')&&(buf[i-1]=='n')&&(buf[i]==':'))
            {
                break;
            }
        }
    }
    if(i == len)
    {
        DEBUG("version unknown\n");
        fprintf(tmp,"version unknown\n");
    }
    else
    {
        while((buf[++i] != '#')&&(i<len))
            str[n++] = buf[i];
        str[n] = '\0';
        DEBUG("version %s\n",str);
        fprintf(tmp,"version %s\n",str);
    }
    memset(str,0,strlen(str));
    if (IMAGE_ENABLE_TIMESTAMP) {
		ret = fit_get_timestamp(ptr, 0, &timestamp);
		DEBUG("created_time ");
        fprintf(tmp,"created_time ");
		if (ret)
		{
			DEBUG("unknown\n");
            fprintf(tmp,"unknown\n");
		}
		else
		{
            local_time = localtime(&timestamp);
            strftime(str,sizeof(str),"%Y-%m-%d#%H:%M:%S",local_time);
			DEBUG("%s\n", str);
            fprintf(tmp,"%ld\n", timestamp);
		}
	}
	//fit_print_contents(ptr);

#if (defined _CTC_ARM_LS023A_)
    cfg_noffset = fit_conf_get_node(ptr,"ctc_ls1023a");
#elif (defined _CTC_ARM_CTC5236_)
    /* TMP CODE: TODO */
    cfg_noffset = fit_conf_get_node(ptr,"e530-24x2c");
#else
    cfg_noffset = fit_conf_get_node(ptr,"ctc_ls1023a");
#endif
    if(cfg_noffset < 0)
    {
        DEBUG("Fail to get image node ");
        fprintf(tmp,"Fail to get image node ");        
		ret = -1;
        goto errorout0;
    }
    noffset = fit_conf_get_prop_node(ptr, cfg_noffset,FIT_FDT_PROP);
    if(noffset < 0)
    {
        DEBUG("Fail to get fdt node ");
        fprintf(tmp,"Fail to get fdt node ");        
		ret = -1;
        goto errorout0;
    }    
    DEBUG("verifying ");
    fprintf(tmp,"verifying ");
	if (!fit_image_verify(ptr, noffset)) {
		printf("FAIL\n");
        fprintf(tmp,"1\n");
	}
	DEBUG("OK\n");
    fprintf(tmp,"0\n");

errorout0:
    if(tmp!=NULL)
    {
        fclose(tmp);
    }       
    
	(void) munmap((void *)ptr, sbuf.st_size);
errorout1:    
	if (close(dfd)) {
		DEBUG ("Close error on %s: %s\n", datafile, strerror(errno));
		exit(1);
	}    
	exit(-ret);
}
