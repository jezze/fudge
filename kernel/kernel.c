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

static void kernel_init_tables()
{

    gdt_init();
    idt_init();
    isr_init();
    irq_init();

}

static void kernel_init_handlers()
{

    isr_register_handler(0x0E, mmu_handler);
    irq_register_handler(0x00, pit_handler);
    irq_register_handler(0x01, kbd_handler);

}

void kernel_main(mboot_info_t *header, uint32_t magic)
{

    kernel_init_tables();
    kernel_init_handlers();

    screen_init();

    ASSERT(magic == 0x2BADB002);
    ASSERT(header->modulesCount > 0);

    pit_init();
    kbd_init();
    mmu_init();

    fsRoot = initrd_init(*((uint32_t *)header->modulesAddresses));

    mboot_init(header);
    cpu_init();

    isr_enable();

    shell_init();

    for (;;);

}

