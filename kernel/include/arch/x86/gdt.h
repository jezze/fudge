#ifndef ARCH_GDT_H
#define ARCH_GDT_H

#define GDT_ENTRY_SLOTS                 6

#define GDT_INDEX_NULL                  0x00
#define GDT_INDEX_KCODE                 0x01
#define GDT_INDEX_KDATA                 0x02
#define GDT_INDEX_UCODE                 0x03
#define GDT_INDEX_UDATA                 0x04
#define GDT_INDEX_TSS                   0x05

#define GDT_ACCESS_ACCESSED             (0x01 << 0)
#define GDT_ACCESS_RW                   (0x01 << 1)
#define GDT_ACCESS_DC                   (0x01 << 2)
#define GDT_ACCESS_EXECUTE              (0x01 << 3)
#define GDT_ACCESS_ALWAYS1              (0x01 << 4)
#define GDT_ACCESS_RING0                (0x00 << 5)
#define GDT_ACCESS_RING1                (0x01 << 5)
#define GDT_ACCESS_RING2                (0x02 << 5)
#define GDT_ACCESS_RING3                (0x03 << 5)
#define GDT_ACCESS_PRESENT              (0x01 << 7)

#define GDT_FLAG_32BIT                  (0x01 << 6)
#define GDT_FLAG_GRANULARITY            (0x01 << 7)

struct gdt_entry
{

    unsigned short limitLow;
    unsigned short baseLow;
    unsigned char baseMiddle;
    unsigned char access;
    unsigned char limitHigh;
    unsigned char baseHigh;

} __attribute__((packed));

struct gdt_pointer
{

    unsigned short limit;
    struct gdt_entry *base;

} __attribute__((packed));

unsigned short gdt_get_selector(unsigned int index);
void gdt_set_entry(unsigned int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char granularity);
void gdt_setup();

#endif

