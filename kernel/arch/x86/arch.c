#include <lib/memory.h>
#include <kernel/kernel.h>
#include <kernel/arch/x86/arch.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/fpu.h>
#include <kernel/arch/x86/gdt.h>
#include <kernel/arch/x86/idt.h>
#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mboot.h>
#include <kernel/arch/x86/mmu.h>
#include <kernel/arch/x86/syscall.h>
#include <kernel/arch/x86/tss.h>

struct arch_x86 arch_x86;

static void setup(struct kernel_arch *arch)
{

    struct arch_x86 *x86 = (struct arch_x86 *)arch;

    mboot_init(x86->header);
    gdt_init();
    tss_init(x86->stack);
    idt_init();
    fpu_init();
    isr_init();
    irq_init();
    syscall_init();
    mmu_setup();

}

void arch_x86_init(struct arch_x86 *x86, struct mboot_header *header, unsigned int magic, unsigned int stack)
{

    memory_clear(x86, sizeof (struct arch_x86));

    kernel_arch_init(&x86->base, setup, cpu_halt, cpu_enable_interrupts, cpu_disable_interrupts, cpu_enter_usermode, header->modules.count, header->modules.address);

    x86->header = header;
    x86->magic = magic;
    x86->stack = stack;

}

