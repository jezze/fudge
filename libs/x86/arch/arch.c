#include <fudge/kernel.h>
#include <kernel/error.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/container.h>
#include <kernel/kernel.h>
#include "cpu.h"
#include "arch.h"
#include "gdt.h"
#include "idt.h"
#include "multi.h"
#include "tss.h"

#define ARCH_GDT_DESCRIPTORS            6
#define ARCH_IDT_DESCRIPTORS            256
#define ARCH_TSS_DESCRIPTORS            1

static struct
{

    struct container *container;
    struct {struct gdt_pointer pointer; struct gdt_descriptor descriptors[ARCH_GDT_DESCRIPTORS];} gdt;
    struct {struct idt_pointer pointer; struct idt_descriptor descriptors[ARCH_IDT_DESCRIPTORS];} idt;
    struct {struct tss_pointer pointer; struct tss_descriptor descriptors[ARCH_TSS_DESCRIPTORS];} tss;
    struct {unsigned short code; unsigned short data;} kselector;
    struct {unsigned short code; unsigned short data;} uselector;
    struct {unsigned short info;} tselector;

} state;

unsigned short arch_schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    state.container->current = multi_schedule(state.container, general, interrupt);

    return state.uselector.data;

}

unsigned short arch_generalfault(void *stack)
{

    struct {struct cpu_general general; unsigned int selector; struct cpu_interrupt interrupt;} *registers = stack;

    return arch_schedule(&registers->general, &registers->interrupt);

}

unsigned short arch_pagefault(void *stack)
{

    struct {struct cpu_general general; unsigned int type; struct cpu_interrupt interrupt;} *registers = stack;

    if (registers->interrupt.code == state.kselector.code)
    {

        multi_map(state.container, cpu_get_cr2());

        return state.kselector.data;

    }

    return arch_schedule(&registers->general, &registers->interrupt);

}

unsigned short arch_syscall(void *stack)
{

    struct {struct cpu_general general; struct cpu_interrupt interrupt;} *registers = stack;

    registers->general.eax = (state.container->calls[registers->general.eax]) ? state.container->calls[registers->general.eax](state.container, state.container->current, (void *)registers->interrupt.esp) : 0;

    return arch_schedule(&registers->general, &registers->interrupt);

}

void arch_setup(unsigned int count, struct kernel_module *modules)
{

    gdt_init_pointer(&state.gdt.pointer, ARCH_GDT_DESCRIPTORS, state.gdt.descriptors);
    idt_init_pointer(&state.idt.pointer, ARCH_IDT_DESCRIPTORS, state.idt.descriptors);
    tss_init_pointer(&state.tss.pointer, ARCH_TSS_DESCRIPTORS, state.tss.descriptors);

    state.kselector.code = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.kselector.data = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.uselector.code = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.uselector.data = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.tselector.info = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_TSS, (unsigned int)state.tss.pointer.descriptors, (unsigned int)state.tss.pointer.descriptors + state.tss.pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, 0);

    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_GF, arch_isr_generalfault, state.kselector.code, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_PF, arch_isr_pagefault, state.kselector.code, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_SYSCALL, arch_isr_syscall, state.kselector.code, IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_TYPE32INT);
    tss_set_descriptor(&state.tss.pointer, TSS_INDEX_DEFAULT, state.kselector.data, ARCH_KSTACK_LIMIT);
    cpu_set_gdt(&state.gdt.pointer);
    cpu_set_idt(&state.idt.pointer);
    cpu_set_tss(state.tselector.info);

    state.container = kernel_setup();
    state.container->current = multi_setup(state.container);

    kernel_setup_modules(count, modules);
    arch_usermode(state.uselector.code, state.uselector.data, state.container->current->registers.ip, state.container->current->registers.sp);

}

