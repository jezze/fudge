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

struct mboot_modules_header
{

    unsigned int count;
    void *address;

};

struct mboot_elf_header
{

    unsigned int num;
    unsigned int size;
    unsigned int addr;
    unsigned int shndx;

};

struct mboot_mmap_header
{

    unsigned int count;
    void *address;

};

struct mboot_drives_header
{

    unsigned int count;
    void *address;

};

struct mboot_vbe_header
{

    unsigned int cinfo;
    unsigned int minfo;
    unsigned short mode;
    unsigned short isegment;
    unsigned short ioffset;
    unsigned short ilength;

} __attribute__((packed));

struct mboot_info
{

    unsigned int flags;
    unsigned int mlower;
    unsigned int mupper;
    unsigned int device;
    unsigned int cmdline;
    struct mboot_modules_header modules;
    struct mboot_elf_header elf;
    struct mboot_mmap_header mmap;
    struct mboot_drives_header drives;
    unsigned int ctable;
    unsigned int name;
    unsigned int atable;
    struct mboot_vbe_header vbe;

} __attribute__((packed));

struct mboot_mmap
{

    unsigned int size;
    unsigned int blow;
    unsigned int bhigh;
    unsigned int llow;
    unsigned int lhigh;
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

