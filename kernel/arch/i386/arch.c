#include <lib/cbuffer.h>
#include <lib/vfs.h>
#include <kernel/arch/i386/arch.h>
#include <kernel/arch/i386/gdt.h>
#include <kernel/arch/i386/idt.h>
#include <kernel/arch/i386/isr.h>
#include <kernel/arch/i386/irq.h>
#include <kernel/arch/i386/kbd.h>
#include <kernel/arch/i386/mmu.h>
#include <kernel/arch/i386/pit.h>
#include <kernel/arch/i386/screen.h>
#include <kernel/arch/i386/syscall.h>
#include <kernel/assert.h>

struct vfs_node *arch_get_stdin()
{

    return kbd_get_node();

}

struct vfs_node *arch_get_stdout()
{

    return screen_get_node();

}

void arch_disable_interrupts()
{

    isr_disable();

}

void arch_enable_interrupts()
{

    isr_enable();

}

void arch_init()
{

    gdt_init();
    idt_init();
    screen_init();
    mmu_init();

}

void arch_init_devices()
{

    pit_init();
    kbd_init();

}

void arch_init_interrupts()
{

    isr_init();
    isr_register_handler(0x0E, mmu_handler);

    irq_init();
    irq_register_handler(0x00, pit_handler);
    irq_register_handler(0x01, kbd_handler);

    syscall_init();
    
    isr_enable();

}

