#include <memory.h>
#include <isr.h>
#include <kernel.h>
#include <arch/x86/arch.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/mboot.h>
#include <arch/x86/mmu.h>
#include <arch/x86/syscall.h>
#include <arch/x86/tss.h>

static struct arch_x86 arch_x86;

static void setup(struct kernel_arch *self)
{

    struct arch_x86 *x86 = (struct arch_x86 *)self;

    mboot_init(x86->header);
    gdt_init();
    tss_init(x86->stack);
    idt_init();

    self->context = isr_init();

    cpu_disable_apic();
    mmu_setup();
    syscall_init();

}

void arch_init(struct mboot_header *header, unsigned int magic)
{

    memory_clear(&arch_x86, sizeof (struct arch_x86));

    kernel_arch_init(&arch_x86.base, setup, cpu_halt, cpu_enable_interrupts, cpu_disable_interrupts, cpu_enter_usermode, header->modules.count, header->modules.address);

    arch_x86.header = header;
    arch_x86.magic = magic;
    arch_x86.stack = 0x00400000;

    arch_x86.base.start(&arch_x86.base);

}

