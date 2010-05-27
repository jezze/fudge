#include <system.h>
#include <regs.h>
#include <mboot.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <irq.h>
#include <screen.h>
#include <pit.h>
#include <kbd.h>
#include <heap.h>
#include <paging.h>
#include <fs.h>
#include <initrd.h>
#include <shell.h>
#include <syscall.h>
#include <task.h>

extern uint32_t heap_address;

void cli()
{

    __asm__ __volatile__ ("cli");

}

void sti()
{

    __asm__ __volatile__ ("sti");

}

void kernel_panic(char *message, char *file, uint32_t line)
{

    cli();

    screen_puts("KERNEL PANIC (");
    screen_puts(message);
    screen_puts(") at (");
    screen_puts(file);
    screen_puts(":");
    screen_puts_dec(line);
    screen_puts(")\n");

    for (;;);

}

void kernel_assert(char *message, char *file, uint32_t line)
{

    cli();

    screen_puts("ASSERTION FAIL (");
    screen_puts(message);
    screen_puts(") at (");
    screen_puts(file);
    screen_puts(":");
    screen_puts_dec(line);
    screen_puts(")\n");

    for (;;);

}

void kernel_main(mboot_header_t *mbootHeader)
{

    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    screen_init();
    pit_init(PIT_FREQUENCY);
    kbd_init();

    // BE SURE INITRD MODULE IS LOADED
    ASSERT(mbootHeader->modulesCount > 0);

    heap_init(*((uint32_t *)(mbootHeader->modulesAddresses + 4)));

    paging_init();
    syscall_init();

    sti();

    fsRoot = initrd_init(*((uint32_t *)mbootHeader->modulesAddresses));

//    switch_to_user_mode();

//    syscall_write("Hello user world!\n");

    shell_init();

    for (;;);

}

