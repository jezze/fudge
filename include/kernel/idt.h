#ifndef IDT_H
#define IDT_H

#define IDT_TABLE_SIZE 256

typedef struct idt_entry
{

    uint16_t baseLow;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t baseHigh;

} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr
{

    uint16_t limit;
    uint32_t base;

} __attribute__((packed)) idt_ptr_t;

extern void idt_flush();

extern void idt_set_gate(uint8_t index, uint32_t base, uint16_t selector, uint8_t flags);
extern void idt_init();

#endif

