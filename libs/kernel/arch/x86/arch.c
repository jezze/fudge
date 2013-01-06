#include <kernel/runtime.h>
#include <kernel/kernel.h>
#include "arch.h"
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "mmu.h"
#include "syscall.h"
#include "tss.h"

static struct runtime_task task;

void arch_setup(unsigned int ramdiskc, void **ramdiskv)
{

    unsigned short cs0;
    unsigned short ds0;
    unsigned short cs3;
    unsigned short ds3;

    gdt_setup();
    idt_setup();

    cs0 = gdt_get_selector(GDT_INDEX_KCODE);
    ds0 = gdt_get_selector(GDT_INDEX_KDATA);
    cs3 = gdt_get_selector(GDT_INDEX_UCODE);
    ds3 = gdt_get_selector(GDT_INDEX_UDATA);

    tss_setup(ds0, ARCH_STACK_BASE);

    runtime_init_task(&task);

    isr_setup(cs0);
    mmu_setup_arch(cs0);
    syscall_setup_arch(&task, cs0);
    kernel_setup(&task, ramdiskc, ramdiskv);

    isr_usermode(cs3, ds3, task.registers.ip, task.registers.sp);

}

