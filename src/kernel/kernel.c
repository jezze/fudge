#include <lib/types.h>
#include <lib/cbuffer.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
#include <kernel/kernel.h>
#include <kernel/regs.h>
#include <kernel/mboot.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/irq.h>
#include <kernel/pit.h>
#include <kernel/kbd.h>
#include <kernel/heap.h>
#include <kernel/paging.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>
#include <kernel/shell.h>
#include <kernel/syscall.h>
#include <kernel/task.h>

extern uint32_t heap_address;

kernel_t kernel;

void cli()
{

    __asm__ __volatile__ ("cli");

}

void sti()
{

    __asm__ __volatile__ ("sti");

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

    // 16MB RAM
    paging_init(0x1000000);
    syscall_init();

    sti();

    fsRoot = initrd_init(*((uint32_t *)mbootHeader->modulesAddresses));

//  switch_to_user_mode();

//  syscall_write("Hello user world!\n");

    shell_init();

    for (;;);

}

