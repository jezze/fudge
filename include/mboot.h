#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#define MULTIBOOT_FLAG_MEM	0x001
#define MULTIBOOT_FLAG_DEVICE	0x002
#define MULTIBOOT_FLAG_CMDLINE	0x004
#define MULTIBOOT_FLAG_MODS	0x008
#define MULTIBOOT_FLAG_AOUT	0x010
#define MULTIBOOT_FLAG_ELF	0x020
#define MULTIBOOT_FLAG_MMAP	0x040
#define MULTIBOOT_FLAG_CONFIG	0x080
#define MULTIBOOT_FLAG_LOADER	0x100
#define MULTIBOOT_FLAG_APM	0x200
#define MULTIBOOT_FLAG_VBE	0x400

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

#endif
