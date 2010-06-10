#include <lib/types.h>
#include <lib/cbuffer.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
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
#include <kernel/vbe.h>
#include <kernel/kernel.h>

kernel_t kernel;

static void kernel_memory(multiboot_header_t *header)
{

    if (header->flags & MULTIBOOT_FLAG_MEM)
    {

        screen_puts("Lower memory: ");
        screen_puts_dec(header->memoryLower);
        screen_puts("KB\n");

        screen_puts("Upper memory: ");
        screen_puts_dec(header->memoryUpper);
        screen_puts("KB\n");

    }

    if (header->flags & MULTIBOOT_FLAG_MMAP)
    {

    }

    if (header->flags & MULTIBOOT_FLAG_VBE)
    {


    }

}

static void kernel_register_handlers()
{

    isr_register_handler(14, paging_handler);
    isr_register_handler(0x80, syscall_handler);
    
    irq_register_handler(0, pit_handler);
    irq_register_handler(1, kbd_handler);

    isr_enable();

}

void kernel_main(multiboot_header_t *header, uint32_t magic)
{

    gdt_init();
    idt_init();
    isr_init();
    irq_init();
    pit_init();
    kbd_init();
    screen_init();

    // BE SURE INITRD MODULE IS LOADED
    ASSERT(header->modulesCount > 0);

    heap_init(*((uint32_t *)(header->modulesAddresses + 4)));

    // 16MB RAM
    paging_init(0x1000000);
    syscall_init();

    kernel_memory(header);
    kernel_register_handlers();

    fsRoot = initrd_init(*((uint32_t *)header->modulesAddresses));

//  switch_to_user_mode();

//  syscall_write("Hello user world!\n");

    shell_init();

    for (;;);

}

