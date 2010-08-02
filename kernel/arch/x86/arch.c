#include <lib/call.h>
#include <lib/cbuffer.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/arch/x86/arch.h>
#include <kernel/arch/x86/gdt.h>
#include <kernel/arch/x86/idt.h>
#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/kbd.h>
#include <kernel/arch/x86/mmu.h>
#include <kernel/arch/x86/pit.h>
#include <kernel/arch/x86/rtc.h>
#include <kernel/arch/x86/screen.h>
#include <kernel/arch/x86/syscall.h>
#include <kernel/assert.h>

void arch_clear()
{

    screen_clear();

}

void arch_disable_interrupts()
{

    isr_disable();

}

void arch_enable_interrupts()
{

    isr_enable();

}

void arch_reboot()
{

    isr_disable();

    unsigned char ready = 0x02;

    while ((ready & 0x02) != 0)
        ready = io_inb(0x64);

    io_outb(0x64, 0xFE);

}

void arch_init()
{

    gdt_init();
    idt_init();

}

void arch_init_devices()
{

    screen_init();
    mmu_init();
    pit_init();
    kbd_init();
    rtc_init();

}

void arch_init_interrupts()
{

    isr_init();
    isr_register_handler(0x0E, mmu_handler);

    irq_init();
    irq_register_handler(0x00, pit_handler);
    irq_register_handler(0x01, kbd_handler);

}

void arch_init_syscalls()
{

    syscall_init();
    syscall_register_handler(CALL_VFS_FIND, syscall_vfs_find);
    syscall_register_handler(CALL_REBOOT, syscall_reboot);

}

