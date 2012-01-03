#include <kernel/kernel.h>
#include <kernel/arch/x86/acpi.h>
#include <kernel/arch/x86/arch.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/fpu.h>
#include <kernel/arch/x86/gdt.h>
#include <kernel/arch/x86/idt.h>
#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/mboot.h>
#include <kernel/arch/x86/mmu.h>
#include <kernel/arch/x86/syscall.h>
#include <kernel/arch/x86/tss.h>

static struct arch_x86 x86;

static void arch_x86_reboot()
{

    cpu_disable_interrupts();

    unsigned char ready = 0x02;

    while ((ready & 0x02))
        ready = io_inb(0x64);

    io_outb(0x64, 0xFE);

}

static void arch_x86_setup(struct kernel_arch *arch)
{

    struct arch_x86 *x86 = (struct arch_x86 *)arch;

    mboot_init(x86->mboot);
    gdt_init();
    tss_init();
    idt_init();
    fpu_init();
    isr_init();
    irq_init();
    acpi_init();
    syscall_init();

}

void arch_init(struct mboot_header *header, unsigned int magic, void *stack)
{

    x86.base.setup = arch_x86_setup;
    x86.base.setup_mmu = mmu_setup;
    x86.base.reboot = arch_x86_reboot;
    x86.base.halt = cpu_halt;
    x86.base.enable_interrupts = cpu_enable_interrupts;
    x86.base.disable_interrupts = cpu_disable_interrupts;
    x86.base.enter_usermode = cpu_enter_usermode;
    x86.base.stack = stack;
    x86.base.set_stack = tss_set_stack;
    x86.base.initrdc = header->modules.count;
    x86.base.initrdv = header->modules.address;
    x86.mboot = header;
    x86.magic = magic;

    kernel_init(&x86.base);

}

