#include <runtime.h>
#include <kernel.h>
#include <arch/x86/arch.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/mmu.h>
#include <arch/x86/syscall.h>
#include <arch/x86/tss.h>

static struct runtime_task task;

void arch_setup(unsigned int ramdiskc, void **ramdiskv)
{

    unsigned short cs;
    unsigned short ds;

    gdt_setup();
    idt_setup();

    cs = gdt_get_selector(GDT_INDEX_KCODE);
    ds = gdt_get_selector(GDT_INDEX_KDATA);

    tss_setup(ds, ARCH_STACK_BASE);
    isr_setup(cs);

    mmu_setup_arch(cs);
    syscall_setup_arch(cs);

    runtime_init_task(&task);
    kernel_setup(&task, ramdiskc, ramdiskv);

    isr_set_task(&task);

    cpu_enter_usermode(task.registers.ip, task.registers.sp);

}

