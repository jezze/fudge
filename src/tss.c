#include <system.h>
#include <mem.h>
#include <gdt.h>
#include <tss.h>

extern void tss_flush();

tss_entry_t tss_entry;

static void tss_init(int32_t num, uint16_t ss0, uint32_t esp0)
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

