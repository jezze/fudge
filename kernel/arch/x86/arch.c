#include <kernel/kernel.h>
#include <kernel/arch/x86/arch.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/fpu.h>
#include <kernel/arch/x86/gdt.h>
#include <kernel/arch/x86/idt.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/mboot.h>
#include <kernel/arch/x86/mmu.h>
#include <kernel/arch/x86/syscall.h>
#include <kernel/arch/x86/tss.h>

static struct kernel_arch arch;

void arch_reboot()
{

    cpu_disable_interrupts();

    /*
    unsigned char ready = 0x02;

    while ((ready & 0x02) != 0)
        ready = io_inb(0x64);

    io_outb(0x64, 0xFE);
    */

}

static void arch_setup(struct kernel_arch *arch)
{

    mboot_init(arch->mboot);
    gdt_init();
    tss_init();
    idt_init();
    fpu_init();
    isr_init();
    irq_init();
    mmu_init();
    syscall_init();

}

void arch_init(struct mboot_header *header, unsigned int magic, void *stack)
{

    arch.setup = arch_setup;
    arch.disable_interrupts = cpu_disable_interrupts;
    arch.enable_interrupts = cpu_enable_interrupts;
    arch.enter_usermode = cpu_enter_usermode;
    arch.set_stack = tss_set_stack;
    arch.register_irq = irq_register_handler;
    arch.unregister_irq = irq_unregister_handler;
    arch.get_task_memory = mmu_get_paddress;
    arch.load_task_memory = mmu_set_directory;
    arch.map_task_memory = mmu_map;
    arch.stack = stack;
    arch.mboot = header;
    arch.initrdc = header->modules.count;
    arch.initrdv = header->modules.address;

    kernel_init(&arch);

}

