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

typedef struct mboot_header
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
    uint32_t mmapAddresses;
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

} __attribute__((packed)) mboot_header_t;

typedef struct mboot_memorymap
{

    uint32_t size;
    uint32_t baseAddressLow;
    uint32_t baseAddressHigh;
    uint32_t lengthLow;
    uint32_t lengthHigh;
    uint32_t type;

} mboot_memorymap_t;

#endif
