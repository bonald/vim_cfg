#ifndef __REBOOT_RECORD_H__
#define __REBOOT_RECORD_H__

struct reboot_device_s
{
    uint8 type;
    uint32 epld_addr;
    void * eeprom_handle;
};
typedef struct reboot_device_s reboot_device_t;

#endif

