#include <kernel/kernel.h>
#include <kernel/log.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/kernel/cpu.h>
#include <arch/x86/kernel/fpu.h>
#include <arch/x86/kernel/gdt.h>
#include <arch/x86/kernel/idt.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/irq.h>
#include <arch/x86/kernel/mboot.h>
#include <arch/x86/kernel/mmu.h>
#include <arch/x86/kernel/syscall.h>
#include <arch/x86/kernel/tss.h>

static void arch_init_base()
{

    log_init();
    gdt_init();
    tss_init();
    idt_init();
    fpu_init();
    isr_init();
    irq_init();
    mmu_init();
    syscall_init();

    cpu_interrupts_on();

}

void arch_init(struct mboot_info *header, unsigned int magic, unsigned int stack)
{

    arch_init_base();

    kernel_assert(magic == MBOOT_MAGIC, "MBOOT_MAGIC is not correct", __FILE__, __LINE__);
    kernel_assert(header->modulesCount, "Module does not exist", __FILE__, __LINE__);
    kernel_set_initrd((unsigned int *)header->modulesAddresses);
    kernel_init(stack);

}

void arch_interrupts_disable()
{

    cpu_interrupts_off();

}

void arch_interrupts_enable()
{

    cpu_interrupts_on();

}

void arch_reboot()
{

    cpu_interrupts_off();
/*
    unsigned char ready = 0x02;

    while ((ready & 0x02) != 0)
        ready = io_inb(0x64);

    io_outb(0x64, 0xFE);
*/
}

void arch_set_stack(unsigned int address)
{

    tss_set_stack(address);

}
void arch_usermode(unsigned int address)
{

    cpu_usermode(address);

}


