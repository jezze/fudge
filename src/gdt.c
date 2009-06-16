#include <system.h>
#include <tss.h>
#include <gdt.h>

extern void gdt_flush();

gdt_entry_t gdt[5];
gdt_ptr_t gdtp;

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

void gdt_init()
{

	gdtp.limit = (sizeof (gdt_entry_t) * 5) - 1;
	gdtp.base = (uint32_t)&gdt;

	memset(&gdt, 0, sizeof (gdt_entry_t) * 5);

	gdt_set_gate(0, 0, 0, 0, 0);			// Null segment
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);	// Kernel code segment
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);	// Kernel data segment
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);	// User code segment
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);	// User data segment

	gdt_flush();

}

