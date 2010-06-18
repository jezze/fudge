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
#include <kernel/mmu.h>
#include <kernel/cpu.h>
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

        screen_puts("MEMORY\n");

        screen_puts("Lower memory: ");
        screen_puts_dec(header->memoryLower);
        screen_puts("KB\n");

        screen_puts("Upper memory: ");
        screen_puts_dec(header->memoryUpper);
        screen_puts("KB\n");

        screen_puts("\n");

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

    isr_register_handler(0x0E, mmu_handler);
    isr_register_handler(0x80, syscall_handler);
    
    irq_register_handler(0x00, pit_handler);
    irq_register_handler(0x01, kbd_handler);

    isr_enable();

}

void kernel_main(multiboot_header_t *header, uint32_t magic)
{

    gdt_init();
    idt_init();
    isr_init();
    screen_init();

    ASSERT(magic == 0x2BADB002);
    ASSERT(header->modulesCount > 0);

    irq_init();
    pit_init();
    kbd_init();

    heap_init(*((uint32_t *)(header->modulesAddresses + 4)));
    mmu_init();
    syscall_init();

    kernel_register_handlers();
    kernel_memory(header);

    fsRoot = initrd_init(*((uint32_t *)header->modulesAddresses));

    cpu_init();
    shell_init();

    for (;;);

}

