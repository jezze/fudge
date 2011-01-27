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

struct gdt_ptr
{

    unsigned short limit;
    unsigned int base;

} __attribute__((packed));

struct tss_entry
{

    unsigned int previous;
    unsigned int esp0;
    unsigned int ss0;
    unsigned int esp1;
    unsigned int ss1;
    unsigned int esp2;
    unsigned int ss2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    unsigned int fs;
    unsigned int gs;
    unsigned int ldt;
    unsigned short trap;
    unsigned short iomap_base;

} __attribute__((packed));

extern void gdt_flush(struct gdt_ptr *pointer);
extern void tss_flush();

extern void gdt_set_gate(unsigned char index, unsigned int base, unsigned int limit, unsigned char access, unsigned char granularity);
extern void gdt_init();

extern void usermode_init();

#endif

