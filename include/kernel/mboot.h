#ifndef MBOOT_H
#define MBOOT_H

#define MBOOT_FLAG_MEM     0x001
#define MBOOT_FLAG_DEVICE  0x002
#define MBOOT_FLAG_CMDLINE 0x004
#define MBOOT_FLAG_MODS    0x008
#define MBOOT_FLAG_AOUT    0x010
#define MBOOT_FLAG_ELF     0x020
#define MBOOT_FLAG_MMAP    0x040
#define MBOOT_FLAG_CONFIG  0x080
#define MBOOT_FLAG_LOADER  0x100
#define MBOOT_FLAG_APM     0x200
#define MBOOT_FLAG_VBE     0x400

#define MBOOT_MAGIC 0x2BADB002

typedef struct mboot_info
{

    uint32_t flags;
    uint32_t memoryLower;
    uint32_t memoryUpper;
    uint32_t device;
    uint32_t cmdline;
    uint32_t modulesCount;
    uint32_t modulesAddresses;
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
    uint32_t mmapLength;
    uint32_t mmapAddress;
    uint32_t drivesLength;
    uint32_t drivesAddresses;
    uint32_t configTable;
    uint32_t name;
    uint32_t apmTable;
    uint32_t vbeControlInfo;
    uint32_t vbeModeInfo;
    uint32_t vbeInterfaceSegment;
    uint32_t vbeInterfaceOffset;
    uint32_t vbeInterfaceLength;

} __attribute__((packed)) mboot_info_t;

typedef struct mboot_mmap
{

    uint32_t size;
    uint32_t baseLow;
    uint32_t baseHigh;
    uint32_t lengthLow;
    uint32_t lengthHigh;
    uint32_t type;

} mboot_mmap_t;

typedef struct mboot_module
{

    uint32_t base;
    uint32_t length;
    uint32_t string;
    uint32_t reserved;

} mboot_module_t;

extern void mboot_init(mboot_info_t *mboot);

#endif
