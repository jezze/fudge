#ifndef IDT_H
#define IDT_H

#define IDT_TABLE_SIZE 256

struct idt_entry
{

    uint16_t baseLow;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t baseHigh;

} __attribute__((packed));

struct idt_ptr
{

    uint16_t limit;
    uint32_t base;

} __attribute__((packed));

extern void idt_flush(struct idt_ptr *pointer);

extern void idt_set_gate(unsigned char index, uint32_t base, uint16_t selector, uint8_t flags);
extern void idt_init();

#endif

