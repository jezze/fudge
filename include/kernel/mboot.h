#ifndef MBOOT_H
#define MBOOT_H

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
