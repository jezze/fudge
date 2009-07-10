#include <system.h>
#include <mem.h>
#include <gdt.h>

extern void gdt_flush();
extern void tss_flush();

gdt_entry_t gdt[6];
gdt_ptr_t gdtp;

tss_entry_t tss_entry;

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{

	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);

	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;	

}

void tss_write(int32_t num, uint16_t ss0, uint32_t esp0)
{

	uint32_t base = (uint32_t)&tss_entry;
	uint32_t limit = base + sizeof (tss_entry);

	gdt_set_gate(num, base, limit, 0xE9, 0x00);

	memset(&tss_entry, 0, sizeof (tss_entry));

	tss_entry.ss0 = ss0;
	tss_entry.esp0 = esp0;
	tss_entry.cs = 0x0b;
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;

}

void set_kernel_stack(uint32_t stack)
{

	tss_entry.esp0 = stack;

}

void gdt_init()
{

	gdtp.limit = (sizeof (gdt_entry_t) * 6) - 1;
	gdtp.base = (uint32_t)&gdt;

	memset(&gdt, 0, sizeof (gdt_entry_t) * 6);

	gdt_set_gate(0, 0, 0, 0, 0);			// Null segment
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);	// Kernel code segment
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);	// Kernel data segment
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);	// User code segment
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);	// User data segment

	tss_write(5, 0x10, 0x0);

	gdt_flush();
	tss_flush();

}

