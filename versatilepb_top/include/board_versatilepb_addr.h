//#include<config.h>

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#define ADDR_KERNEL 0x00010000  // the address to loading kernel image
#define ADDR_INITRD 0x00800000  // the address to loading initrd image
#define ADDR_ATAG   0x00000100  // the address of ATAG table for Linux booting
#define ADDR_PROG   0x00000000  // the address of application program loading

/* Linux kernel 2.6 ATAG list for ARM */
#define ATAG_NONE       0x00000000
#define ATAG_CORE       0x54410001
#define ATAG_MEM        0x54410002
#define ATAG_VIDEOTEXT  0x54410003
#define ATAG_RAMDISK    0x54410004
#define ATAG_INITRD2    0x54420005
#define ATAG_SERIAL     0x54410006
#define ATAG_REVISION   0x54410007
#define ATAG_VIDEOLFB   0x54410008
#define ATAG_CMDLINE    0x54410009

#endif
