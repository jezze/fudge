#ifndef IDT_H
#define IDT_H

#define IDT_TABLE_SIZE 256

typedef struct idt_entry
{

	uint16_t base_lo;
	uint16_t sel;
	uint8_t always0;
	uint8_t flags;
	uint16_t base_hi;

} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr
{

	uint16_t limit;
	uint32_t base;

} __attribute__((packed)) idt_ptr_t;

extern void idt_set_gate(uint8_t, uint32_t, uint16_t, uint8_t);
extern void idt_init();

#endif

