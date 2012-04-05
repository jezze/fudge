#include <lib/memory.h>
#include <kernel/kernel.h>
#include <kernel/arch/x86/arch.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/fpu.h>
#include <kernel/arch/x86/gdt.h>
#include <kernel/arch/x86/idt.h>
#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mboot.h>
#include <kernel/arch/x86/mmu.h>
#include <kernel/arch/x86/tss.h>

struct arch_x86 arch_x86;

static void reboot()
{

    cpu_disable_interrupts();

    unsigned char ready = 0x02;

    while ((ready & 0x02))
        ready = io_inb(0x64);

    io_outb(0x64, 0xFE);

}

static void setup(struct kernel_arch *arch)
{

    struct arch_x86 *x86 = (struct arch_x86 *)arch;

    mboot_init(x86->mboot);
    gdt_init();
    tss_init();
    idt_init();
    fpu_init();
    isr_init();
    mmu_setup();

}

void arch_x86_init(struct arch_x86 *x86, struct mboot_header *header, unsigned int magic, void *stack)
{

    memory_clear(x86, sizeof (struct arch_x86));

    x86->base.setup = setup;
    x86->base.reboot = reboot;
    x86->base.halt = cpu_halt;
    x86->base.enable_interrupts = cpu_enable_interrupts;
    x86->base.disable_interrupts = cpu_disable_interrupts;
    x86->base.enter_usermode = cpu_enter_usermode;
    x86->base.stack = stack;
    x86->base.set_stack = tss_set_stack;
    x86->base.ramdiskc = header->modules.count;
    x86->base.ramdiskv = header->modules.address;

    x86->mboot = header;
    x86->magic = magic;

}

