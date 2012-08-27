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

static struct arch_interface interface;

static void setup(struct kernel_interface *self)
{

    struct arch_interface *interface = (struct arch_interface *)self;
    unsigned int cs;
    unsigned int ss;

    mboot_setup(interface->header);
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

void arch_interface_init(struct arch_interface *interface, struct mboot_header *header, unsigned int magic)
{

    memory_clear(interface, sizeof (struct arch_interface));

    kernel_interface_init(&interface->base, setup, cpu_enter_usermode, header->modules.count, header->modules.address);

    interface->header = header;
    interface->magic = magic;

}

void arch_setup(struct mboot_header *header, unsigned int magic)
{

    arch_interface_init(&interface, header, magic);

    interface.base.start(&interface.base);

}

