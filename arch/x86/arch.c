#include <kernel/kernel.h>
#include <kernel/log.h>
#include <arch/x86/arch.h>
#include <arch/x86/cpu.h>
#include <arch/x86/fpu.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/irq.h>
#include <arch/x86/mboot.h>
#include <arch/x86/mmu.h>
#include <arch/x86/syscall.h>
#include <arch/x86/tss.h>

static struct kernel_arch arch;

static void arch_disable_interrupts()
{

    cpu_interrupts_off();

}

static void arch_enable_interrupts()
{

    cpu_interrupts_on();

}

static void arch_enable_usermode(unsigned int address)
{

    cpu_usermode(address);

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

static void arch_set_stack(unsigned int address)
{

    tss_set_stack(address);

}

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

    arch.enable_interrupts();

}

void arch_init(struct mboot_info *header, unsigned int magic, unsigned int stack)
{

    arch.disable_interrupts = arch_disable_interrupts;
    arch.enable_interrupts = arch_enable_interrupts;
    arch.enable_usermode = arch_enable_usermode;
    arch.set_stack = arch_set_stack;
    arch.stackAddress = stack;
    arch.mbootAddress = (unsigned int *)header;
    arch.initrdAddress = (unsigned int *)header->modulesAddresses;

    arch_init_base();

    mboot_init(header);

    kernel_init(&arch);

}

