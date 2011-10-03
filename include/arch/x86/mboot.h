#ifndef ARCH_MBOOT_H
#define ARCH_MBOOT_H

#define MBOOT_FLAG_MEMORY  1 << 0
#define MBOOT_FLAG_DEVICE  1 << 1
#define MBOOT_FLAG_CMDLINE 1 << 2
#define MBOOT_FLAG_MODULES 1 << 3
#define MBOOT_FLAG_AOUT    1 << 4
#define MBOOT_FLAG_ELF     1 << 5
#define MBOOT_FLAG_MMAP    1 << 6
#define MBOOT_FLAG_CONFIG  1 << 7
#define MBOOT_FLAG_LOADER  1 << 8
#define MBOOT_FLAG_APM     1 << 9
#define MBOOT_FLAG_VBE     1 << 10

#define MBOOT_MAGIC 0x2BADB002

struct mboot_info
{

    unsigned int flags;
    unsigned int memoryLower;
    unsigned int memoryUpper;
    unsigned int device;
    unsigned int cmdline;
    unsigned int modulesCount;
    unsigned int modulesAddresses;
    unsigned int num;
    unsigned int size;
    unsigned int addr;
    unsigned int shndx;
    unsigned int mmapLength;
    unsigned int mmapAddress;
    unsigned int drivesLength;
    unsigned int drivesAddresses;
    unsigned int configTable;
    unsigned int name;
    unsigned int apmTable;
    unsigned int vbeControllerInfo;
    unsigned int vbeModeInfo;
    unsigned short vbeMode;
    unsigned short vbeInterfaceSegment;
    unsigned short vbeInterfaceOffset;
    unsigned short vbeInterfaceLength;

} __attribute__((packed));

struct mboot_mmap
{

    unsigned int size;
    unsigned int baseLow;
    unsigned int baseHigh;
    unsigned int lengthLow;
    unsigned int lengthHigh;
    unsigned int type;

};

struct mboot_module
{

    unsigned int base;
    unsigned int length;
    unsigned int name;
    unsigned int reserved;

};

extern void mboot_init(struct mboot_info *info);

#endif

