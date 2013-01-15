#include <fudge/kernel.h>
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

    struct gdt_pointer *gdtp = gdt_setup();
    struct idt_pointer *idtp = idt_setup();
    unsigned short cs0 = gdt_get_selector(GDT_INDEX_KCODE);
    unsigned short ds0 = gdt_get_selector(GDT_INDEX_KDATA);
    unsigned short cs3 = gdt_get_selector(GDT_INDEX_UCODE);
    unsigned short ds3 = gdt_get_selector(GDT_INDEX_UDATA);

    cpu_set_gdt(gdtp);
    cpu_set_idt(idtp);
    tss_setup(ds0, ARCH_STACK_BASE);
    isr_setup(cs0);
    mmu_setup_arch(cs0);
    runtime_init_task(&task);
    syscall_setup_arch(&task, cs0);
    kernel_setup(&task, ramdiskc, ramdiskv);
    isr_usermode(cs3, ds3, task.registers.ip, task.registers.sp);

}

