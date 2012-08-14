#include <memory.h>
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

static struct arch_x86 x86;

static void setup(struct kernel_arch *self)
{

    struct arch_x86 *x86 = (struct arch_x86 *)self;
    unsigned int cs;
    unsigned int ss;

    mboot_setup(x86->header);
    gdt_setup();
    idt_setup();

    cs = gdt_get_segment(GDT_INDEX_KCODE);
    ss = gdt_get_segment(GDT_INDEX_KDATA);

    tss_setup(ss, 0x00400000);
    isr_setup(cs);
    cpu_disable_apic();
    mmu_setup();
    syscall_setup();

}

void arch_x86_init(struct arch_x86 *x86, struct mboot_header *header, unsigned int magic)
{

    memory_clear(x86, sizeof (struct arch_x86));

    kernel_arch_init(&x86->base, setup, cpu_halt, cpu_enable_interrupts, cpu_disable_interrupts, cpu_enter_usermode, header->modules.count, header->modules.address);

    x86->header = header;
    x86->magic = magic;

}

void arch_setup(struct mboot_header *header, unsigned int magic)
{

    arch_x86_init(&x86, header, magic);

    x86.base.start(&x86.base);

}

