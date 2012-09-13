#ifndef ARCH_IDT_H
#define ARCH_IDT_H

#define IDT_TABLE_SLOTS 256

#define IDT_FLAG_TYPE32TASK  (0x05 << 0)
#define IDT_FLAG_TYPE16INT   (0x06 << 0)
#define IDT_FLAG_TYPE16TRAP  (0x07 << 0)
#define IDT_FLAG_TYPE32INT   (0x0E << 0)
#define IDT_FLAG_TYPE32TRAP  (0x0F << 0)
#define IDT_FLAG_STORAGE     (0x01 << 4)
#define IDT_FLAG_RING0       (0x00 << 5)
#define IDT_FLAG_RING1       (0x01 << 5)
#define IDT_FLAG_RING2       (0x02 << 5)
#define IDT_FLAG_RING3       (0x03 << 5)
#define IDT_FLAG_PRESENT     (0x01 << 7)

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

void idt_set_entry(unsigned int index, void (*callback)(), unsigned short selector, unsigned char flags);
void idt_setup();

#endif

