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

void kernel_memory(multiboot_header_t *header)
{

    if (header->flags & MULTIBOOT_FLAG_MEM)
    {

        screen_puts(&screen, "Lower memory: ");
        screen_puts_dec(&screen, header->memoryLower);
        screen_puts(&screen, "KB\n");

        screen_puts(&screen, "Upper memory: ");
        screen_puts_dec(&screen, header->memoryUpper);
        screen_puts(&screen, "KB\n");

    }

    if (header->flags & MULTIBOOT_FLAG_MMAP)
    {

    }

    if (header->flags & MULTIBOOT_FLAG_VBE)
    {


    }

}

void kernel_main(multiboot_header_t *header, uint32_t magic)
{

    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    screen_init();
    pit_init(PIT_FREQUENCY);
    kbd_init();

    // BE SURE INITRD MODULE IS LOADED
    ASSERT(header->modulesCount > 0);

    heap_init(*((uint32_t *)(header->modulesAddresses + 4)));

    // 16MB RAM
    paging_init(0x1000000);
    syscall_init();

    sti();

    fsRoot = initrd_init(*((uint32_t *)header->modulesAddresses));

//  switch_to_user_mode();

//  syscall_write("Hello user world!\n");

    kernel_memory(header);

    shell_init();

    for (;;);

}

