#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/msr/msr.h>
#include "syse.h"

void syse_resume(struct cpu_general *general)
{

    struct core *core = kernel_getcore();

    if (core->itask)
    {

        struct task_thread *thread = kernel_getthread(core->itask);

        thread->ip = general->edx.value;
        thread->sp = general->ecx.value;

    }

    core->itask = kernel_schedule(core);

    if (core->itask)
    {

        struct task_thread *thread = kernel_getthread(core->itask);

        general->edx.value = thread->ip;
        general->ecx.value = thread->sp;

    }

    else
    {

        general->edx.value = (unsigned int)cpu_halt;
        general->ecx.value = core->sp;

    }

}

void syse_syscall(struct cpu_general general)
{

    struct core *core = kernel_getcore();

    general.eax.value = abi_call(general.eax.value, core->itask, (void *)(general.ecx.value + 8));

    syse_resume(&general);

}

void module_init(void)
{

    struct msr_data msrdata;

    msrdata.eax = 8;
    msrdata.edx = 0;

    msr_set(0x174, &msrdata);

    msrdata.eax = 0x00400000;
    msrdata.edx = 0;

    msr_set(0x175, &msrdata);

    msrdata.eax = (unsigned int)&syse_interrupt;
    msrdata.edx = 0;

    msr_set(0x176, &msrdata);

}

