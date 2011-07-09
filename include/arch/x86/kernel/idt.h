#ifndef ARCH_IDT_H
#define ARCH_IDT_H

#define IDT_TABLE_SIZE 256

struct idt_entry
{

    unsigned short baseLow;
    unsigned short selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short baseHigh;

} __attribute__((packed));

struct idt_pointer
{

    unsigned short limit;
    struct idt_entry *base;

} __attribute__((packed));

extern void idt_flush(struct idt_pointer *pointer);

extern void idt_set_gate(unsigned char index, void (*callback)(), unsigned short selector, unsigned char flags);
extern void idt_init();

#endif

