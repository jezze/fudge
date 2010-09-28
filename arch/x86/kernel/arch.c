#include <lib/call.h>
#include <lib/cbuffer.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/kernel/ata.h>
#include <arch/x86/kernel/fpu.h>
#include <arch/x86/kernel/gdt.h>
#include <arch/x86/kernel/idt.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/irq.h>
#include <arch/x86/kernel/kbd.h>
#include <arch/x86/kernel/mmu.h>
#include <arch/x86/kernel/pit.h>
#include <arch/x86/kernel/rtc.h>
#include <arch/x86/kernel/screen.h>
#include <arch/x86/kernel/syscall.h>
#include <kernel/assert.h>

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
    fpu_init();

}

void arch_init_devices()
{

    screen_init();
    mmu_init();
    pit_init();
    kbd_init();
    rtc_init();
    ata_init();

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

