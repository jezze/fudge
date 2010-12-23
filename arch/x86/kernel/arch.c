#include <lib/cbuffer.h>
#include <lib/file.h>
#include <lib/session.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/kernel/fpu.h>
#include <arch/x86/kernel/gdt.h>
#include <arch/x86/kernel/idt.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/irq.h>
#include <arch/x86/kernel/mmu.h>
#include <arch/x86/kernel/pit.h>
#include <arch/x86/kernel/rtc.h>
#include <arch/x86/kernel/syscall.h>
#include <arch/x86/modules/ata/ata.h>
#include <arch/x86/modules/kbd/kbd.h>
#include <arch/x86/modules/vga/vga.h>
#include <kernel/dev.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/mboot.h>
#include <kernel/vfs.h>

void arch_assert(unsigned int condition, char *message, char *file, unsigned int line)
{

    if (condition)
        return;

    arch_interrupts_disable();

    file_write_string(session_get_out(), "ASSERTION FAIL (");
    file_write_string(session_get_out(), message);
    file_write_string(session_get_out(), ") at (");
    file_write_string(session_get_out(), file);
    file_write_string(session_get_out(), ":");
    file_write_dec(session_get_out(), line);
    file_write_string(session_get_out(), ")\n");

    for (;;);

}

void arch_panic(char *message, char *file, unsigned int line)
{

    arch_interrupts_disable();

    file_write_string(session_get_out(), "KERNEL PANIC (");
    file_write_string(session_get_out(), message);
    file_write_string(session_get_out(), ") at (");
    file_write_string(session_get_out(), file);
    file_write_string(session_get_out(), ":");
    file_write_dec(session_get_out(), line);
    file_write_string(session_get_out(), ")\n");

    for (;;);

}

static void arch_init_base()
{

    gdt_init();
    idt_init();
    fpu_init();
    mmu_init();
    vfs_init();

}

static void arch_init_devices()
{

    dev_init();
    io_init();
    vga_init();
    pit_init();
    kbd_init();
    rtc_init();
    ata_init();

}

static void arch_init_interrupts()
{

    isr_init();
    isr_register_handler(ISR_ROUTINE_PAGEFAULT, mmu_handler);

    irq_init();
    irq_register_handler(IRQ_ROUTINE_PIT, pit_handler);
    irq_register_handler(IRQ_ROUTINE_KBD, kbd_handler);

    isr_enable();

}

static void arch_init_syscalls()
{

    syscall_init();
    syscall_register_handler(SYSCALL_ROUTINE_OPEN, syscall_open);
    syscall_register_handler(SYSCALL_ROUTINE_REBOOT, syscall_reboot);
    syscall_register_handler(SYSCALL_ROUTINE_EXECUTE, syscall_execute);
    syscall_register_handler(SYSCALL_ROUTINE_LOAD, syscall_load);
    syscall_register_handler(SYSCALL_ROUTINE_UNLOAD, syscall_unload);

}

void arch_init(struct mboot_info *header, unsigned int magic)
{

    arch_init_base();
    arch_init_syscalls();
    arch_init_interrupts();
    arch_init_devices();

    arch_assert(magic == MBOOT_MAGIC, "MBOOT_MAGIC is not correct", __FILE__, __LINE__);
    arch_assert(header->modulesCount, "Module does not exist", __FILE__, __LINE__);

    initrd_init(*((unsigned int *)header->modulesAddresses));

    kernel_init();

}

void arch_interrupts_disable()
{

    isr_disable();

}

void arch_interrupts_enable()
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

