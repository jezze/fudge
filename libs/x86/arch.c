#include <fudge/kernel.h>
#include <kernel/error.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/container.h>
#include <kernel/kernel.h>
#include "arch.h"
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "mmu.h"
#include "multi.h"
#include "tss.h"

#define ARCH_KERNEL_BASE                0x00000000
#define ARCH_KERNEL_SIZE                0x00400000
#define ARCH_STACK_BASE                 0x00400000
#define ARCH_MMU_TABLES                 3
#define ARCH_GDT_DESCRIPTORS            6
#define ARCH_IDT_DESCRIPTORS            256
#define ARCH_TSS_DESCRIPTORS            1

static struct
{

    struct mmu_directory directory;
    struct mmu_table tables[ARCH_MMU_TABLES];
    struct container *container;
    struct {struct gdt_pointer pointer; struct gdt_descriptor descriptors[ARCH_GDT_DESCRIPTORS];} gdt;
    struct {struct idt_pointer pointer; struct idt_descriptor descriptors[ARCH_IDT_DESCRIPTORS];} idt;
    struct {struct tss_pointer pointer; struct tss_descriptor descriptors[ARCH_TSS_DESCRIPTORS];} tss;
    struct {unsigned short kcode; unsigned short kdata; unsigned short ucode; unsigned short udata;} selectors;

} state;

unsigned short arch_genfault(void *stack)
{

    struct {struct cpu_general general; unsigned int selector; struct cpu_interrupt interrupt;} *registers = stack;

    return registers->interrupt.data;

}

unsigned short arch_pagefault(void *stack)
{

    struct {struct cpu_general general; unsigned int type; struct cpu_interrupt interrupt;} *registers = stack;
    unsigned int address = cpu_get_cr2();

    state.container->running->state = 0;

    state.container->schedule(state.container);

    if (state.container->running->state & TASK_STATE_USED)
    {

        registers->interrupt.code = state.selectors.ucode;
        registers->interrupt.eip = state.container->running->registers.ip;
        registers->interrupt.esp = state.container->running->registers.sp;
        registers->general.ebp = state.container->running->registers.fp;
        registers->general.eax = state.container->running->registers.status;

        return state.selectors.udata;

    }

    error_register(1, address);
    error_register(2, registers->type);
    error_panic("PAGE FAULT", __FILE__, __LINE__);

    return state.selectors.kdata;

}

unsigned short arch_syscall(void *stack)
{

    struct {struct cpu_general general; struct cpu_interrupt interrupt;} *registers = stack;

    if (!state.container->calls[registers->general.eax])
        return state.selectors.udata;

    state.container->running->registers.ip = registers->interrupt.eip;
    state.container->running->registers.sp = registers->interrupt.esp;
    state.container->running->registers.fp = registers->general.ebp;
    state.container->running->registers.status = state.container->calls[registers->general.eax](state.container, state.container->running, (void *)registers->interrupt.esp);

    state.container->schedule(state.container);

    if (state.container->running->state & TASK_STATE_USED)
    {

        registers->interrupt.code = state.selectors.ucode;
        registers->interrupt.eip = state.container->running->registers.ip;
        registers->interrupt.esp = state.container->running->registers.sp;
        registers->general.ebp = state.container->running->registers.fp;
        registers->general.eax = state.container->running->registers.status;

        return state.selectors.udata;

    }

    registers->interrupt.code = state.selectors.kcode;
    registers->interrupt.eip = (unsigned int)arch_halt;
    registers->interrupt.esp = ARCH_STACK_BASE;
    registers->general.ebp = 0;
    registers->general.eax = 0;

    return state.selectors.kdata;

}

void arch_setup(unsigned int count, struct kernel_module *modules)
{

    unsigned int segment;

    gdt_init_pointer(&state.gdt.pointer, ARCH_GDT_DESCRIPTORS, state.gdt.descriptors);
    idt_init_pointer(&state.idt.pointer, ARCH_IDT_DESCRIPTORS, state.idt.descriptors);
    tss_init_pointer(&state.tss.pointer, ARCH_TSS_DESCRIPTORS, state.tss.descriptors);

    state.selectors.kcode = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.selectors.ucode = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.selectors.kdata = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.selectors.udata = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);

    tss_set_descriptor(&state.tss.pointer, TSS_INDEX_DEFAULT, state.selectors.kdata, ARCH_STACK_BASE);

    segment = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_TSS, (unsigned int)state.tss.pointer.descriptors, (unsigned int)state.tss.pointer.descriptors + state.tss.pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, 0x00);

    cpu_set_gdt(&state.gdt.pointer);
    cpu_set_idt(&state.idt.pointer);
    cpu_set_tss(segment);
    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_GP, arch_isr_genfault, state.selectors.kcode, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_PF, arch_isr_pagefault, state.selectors.kcode, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_SYSCALL, arch_isr_syscall, state.selectors.kcode, IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_TYPE32INT);
    mmu_map_memory(&state.directory, &state.tables[0], ARCH_KERNEL_BASE, ARCH_KERNEL_BASE, ARCH_KERNEL_SIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map_memory(&state.directory, &state.tables[1], TASKADDRESS_PHYSICAL, TASKADDRESS_VIRTUAL, TASKADDRESS_SIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_map_memory(&state.directory, &state.tables[2], STACKADDRESS_PHYSICAL, STACKADDRESS_VIRTUAL - STACKADDRESS_SIZE, STACKADDRESS_SIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_load_memory(&state.directory);
    mmu_enable();

    state.container = kernel_setup(count, modules);

    multi_setup(state.container);
    arch_usermode(state.selectors.ucode, state.selectors.udata, state.container->running->registers.ip, state.container->running->registers.sp);

}

