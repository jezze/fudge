#include <system.h>
#include <mboot.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <irq.h>
#include <screen.h>
#include <pit.h>
#include <kbd.h>
#include <paging.h>
#include <fs.h>
#include <initrd.h>
#include <shell.h>
#include <syscall.h>
#include <task.h>

extern uint32_t placement_address;

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

void kmain(mboot_header_t *mboot_ptr)
{

	gdt_init();
	idt_init();
	isr_init();
	irq_init();
	screen_init();
	pit_init(PIT_FREQUENCY);
	kbd_init();

	//MULTIBOOT
	ASSERT(mboot_ptr->mods_count > 0);
	uint32_t initrd_location = *((uint32_t*)mboot_ptr->mods_addr);
	uint32_t initrd_end = *(uint32_t*)(mboot_ptr->mods_addr + 4);

	placement_address = initrd_end;

	paging_init();
	syscall_init();

	sti();

	fs_root = initrd_init(initrd_location);

//	switch_to_user_mode();

//	syscall_write("Hello user world!\n");

	shell_init();

	for (;;);

}

