#include <system.h>
#include <multiboot.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <irq.h>
#include <screen.h>
#include <pit.h>
#include <keyboard.h>
#include <paging.h>
#include <fs.h>
#include <initrd.h>
#include <console.h>

extern uint32_t placement_address;

void *memcpy(void *dest, const void *src, int32_t count)
{

	const int8_t *sp = (const int8_t *)src;
	int8_t *dp = (int8_t *)dest;
	for (; count != 0; count--) *dp++ = *sp++;

}

void *memset(void *dest, int8_t val, int32_t count)
{

	int8_t *temp = (int8_t *)dest;
	for (; count != 0; count--) *temp++ = val;

}

uint16_t *memsetw(uint16_t *dest, uint16_t val, int32_t count)
{

	uint16_t *temp = (uint16_t *)dest;
	for (; count != 0; count--) *temp++ = val;
	return dest;

}

uint8_t inb(uint16_t port)
{

	uint8_t rv;
	__asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
	return rv;

}

uint16_t inw(uint16_t port)
{

	uint16_t rv;
	__asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (port));
	return rv;

}

void outb(uint16_t port, uint8_t data)
{

	__asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));

}

void cli()
{

	__asm__ __volatile__ ("cli");

}

void sti()
{

	__asm__ __volatile__ ("sti");

}

void kpanic(char *message, char *file, uint32_t line)
{

	cli();

	puts("PANIC(");
	puts(message);
	puts(") at ( ");
	puts(file);
	puts(":");
	puts_dec(line);
	puts("\n");

	for (;;);

}

void kpanic_assert(char *file, uint32_t line, char *description)
{

	cli();

	puts("ASSERTION FAIL(");
	puts(description);
	puts(") at (");
	puts(file);
	puts(":");
	puts_dec(line);
	puts("\n");

	for (;;);

}

void kmain(struct multiboot *mboot_ptr)
{

	gdt_init();
	idt_init();
	isr_init();
	irq_init();
	screen_init();
	pit_init(PIT_FREQUENCY);
	keyboard_init();

	//MULTIBOOT
	ASSERT(mboot_ptr->mods_count > 0);
	uint32_t initrd_location = *((uint32_t*)mboot_ptr->mods_addr);
	uint32_t initrd_end = *(uint32_t*)(mboot_ptr->mods_addr + 4);

	placement_address = initrd_end;

	paging_init();

	sti();

	fs_root = initialise_initrd(initrd_location);

	console_init();

	for (;;);

}

