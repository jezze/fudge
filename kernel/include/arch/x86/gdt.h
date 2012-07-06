#ifndef ARCH_GDT_H
#define ARCH_GDT_H

#define GDT_TABLE_SLOTS      6
#define GDT_INDEX_NULL       0x00
#define GDT_INDEX_KERNELCODE 0x01
#define GDT_INDEX_KERNELDATA 0x02
#define GDT_INDEX_USERCODE   0x03
#define GDT_INDEX_USERDATA   0x04
#define GDT_INDEX_TSS        0x05

struct gdt_entry
{

    unsigned short limitLow;
    unsigned short baseLow;
    unsigned char baseMiddle;
    unsigned char access;
    unsigned char granularity;
    unsigned char baseHigh;

} __attribute__((packed));

struct gdt_pointer
{

    unsigned short limit;
    struct gdt_entry *base;

} __attribute__((packed));

void gdt_set_gate(unsigned int index, unsigned int base, unsigned int limit, unsigned char access, unsigned char granularity);
void gdt_setup();

#endif

