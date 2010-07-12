#include <lib/types.h>
#include <lib/cbuffer.h>
#include <lib/vfs.h>
#include <kernel/assert.h>
#include <kernel/screen.h>
#include <kernel/mboot.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/irq.h>
#include <kernel/pit.h>
#include <kernel/kbd.h>
#include <kernel/mmu.h>
#include <kernel/initrd.h>
#include <kernel/shell.h>
#include <kernel/syscall.h>
#include <kernel/rtc.h>
#include <kernel/kernel.h>

kernel_t kernel;

void kernel_main(mboot_info_t *header, uint32_t magic)
{

    gdt_init();
    idt_init();

    screen_init();

    ASSERT(magic == MBOOT_MAGIC);
    ASSERT(header->modulesCount);

    isr_init();
    isr_register_handler(0x0E, mmu_handler);
    idt_set_gate(0x80, (uint32_t)syscall_interrupt, 0x08, 0x8E);

    irq_init();
    irq_register_handler(0x00, pit_handler);
    irq_register_handler(0x01, kbd_handler);

    syscall_init();

    mmu_init();
    
    pit_init();
    kbd_init();

    isr_enable();

    fsRoot = initrd_init(*((unsigned int *)header->modulesAddresses));

    shell_init();

    for (;;);

}

