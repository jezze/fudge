#ifndef ARCH_GDT_H
#define ARCH_GDT_H

#define GDT_TABLE_SIZE 6

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

extern void gdt_flush(struct gdt_pointer *pointer);
extern void gdt_init();
extern void gdt_set_gate(unsigned char index, unsigned int base, unsigned int limit, unsigned char access, unsigned char granularity);

#endif

