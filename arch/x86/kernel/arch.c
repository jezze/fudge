#include <lib/file.h>
#include <lib/session.h>
#include <kernel/kernel.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/kernel/fpu.h>
#include <arch/x86/kernel/gdt.h>
#include <arch/x86/kernel/idt.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/irq.h>
#include <arch/x86/kernel/mboot.h>
#include <arch/x86/kernel/mmu.h>
#include <arch/x86/kernel/syscall.h>

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
    isr_init();
    irq_init();
    mmu_init();
    syscall_init();

    isr_enable();

}

void arch_init(struct mboot_info *header, unsigned int magic, unsigned int stack)
{

    arch_init_base();

    arch_assert(magic == MBOOT_MAGIC, "MBOOT_MAGIC is not correct", __FILE__, __LINE__);
    arch_assert(header->modulesCount, "Module does not exist", __FILE__, __LINE__);

    kernel_set_initrd(*(unsigned int *)header->modulesAddresses);
    kernel_init(stack);

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
/*
    unsigned char ready = 0x02;

    while ((ready & 0x02) != 0)
        ready = io_inb(0x64);

    io_outb(0x64, 0xFE);
*/
}

