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
#include "tss.h"
#include "mmu.h"

#define ARCH_GDT_DESCRIPTORS            6
#define ARCH_IDT_DESCRIPTORS            256
#define ARCH_TSS_DESCRIPTORS            1
#define ARCH_KSPACE_BASE                ARCH_BIOS_BASE
#define ARCH_KSPACE_LIMIT               ARCH_TABLE_USTACK_LIMIT
#define ARCH_KSPACE_SIZE                (ARCH_KSPACE_LIMIT - ARCH_KSPACE_BASE)
#define ARCH_TASKS                      64
#define ARCH_TASK_CODESIZE              (ARCH_UCODE_SIZE / ARCH_TASKS)
#define ARCH_TASK_STACKSIZE             (ARCH_USTACK_SIZE / ARCH_TASKS)
#define ARCH_TASK_STACKLIMIT            0x80000000
#define ARCH_TASK_STACKBASE             (ARCH_TASK_STACKLIMIT - ARCH_TASK_STACKSIZE)

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

static struct arch_task
{

    struct task base;
    unsigned int index;
    struct cpu_general general;

} tasks[ARCH_TASKS];

static struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_BASE;
static struct mmu_table *kcode = (struct mmu_table *)ARCH_TABLE_KCODE_BASE;
static struct mmu_table *ucode = (struct mmu_table *)ARCH_TABLE_UCODE_BASE;
static struct mmu_table *ustack = (struct mmu_table *)ARCH_TABLE_USTACK_BASE;

static void activate_task(struct task *t)
{

    struct arch_task *task = (struct arch_task *)t;

    task_sched_use(t);
    memory_clear(&directories[task->index], sizeof (struct mmu_directory));
    mmu_map(&directories[task->index], &kcode[0], ARCH_KSPACE_BASE, ARCH_KSPACE_BASE, ARCH_KSPACE_SIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_load(&directories[task->index]);

}

static void init_task(struct arch_task *task, unsigned int index)
{

    memory_clear(task, sizeof (struct arch_task));
    task_init(&task->base, 0, ARCH_TASK_STACKLIMIT);

    task->index = index;

}

static unsigned int spawn(struct container *self, struct task *task, void *stack)
{

    struct parameters {void *caller; unsigned int index;} args;
    struct task *next = task_sched_find_free_task();

    if (!next)
        return 0;

    memory_copy(&args, stack, sizeof (struct parameters));
    memory_copy(next->descriptors, task->descriptors, sizeof (struct task_descriptor) * TASK_DESCRIPTORS);
    activate_task(next);

    return self->calls[CONTAINER_CALL_EXECUTE](self, next, &args);

}

unsigned short arch_schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct arch_task *current = (struct arch_task *)state.container->current;
    struct arch_task *next = (struct arch_task *)task_sched_find_next_task();

    if (current)
    {

        if (current == next)
            return state.uselector.data;

        current->base.registers.ip = interrupt->eip;
        current->base.registers.sp = interrupt->esp;

        memory_copy(&current->general, general, sizeof (struct cpu_general));

    }

    if (next)
    {

        mmu_load(&directories[next->index]);

        interrupt->code = state.uselector.code;
        interrupt->eip = next->base.registers.ip;
        interrupt->esp = next->base.registers.sp;

        memory_copy(general, &next->general, sizeof (struct cpu_general));

        state.container->current = &next->base;

        return state.uselector.data;

    }

    interrupt->code = state.kselector.code;
    interrupt->eip = (unsigned int)arch_halt;
    interrupt->esp = ARCH_KSTACK_LIMIT;

    state.container->current = 0;

    return state.kselector.data;

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

        struct arch_task *task = (struct arch_task *)task_sched_find_next_task();
        unsigned int address = cpu_get_cr2();

        mmu_map(&directories[task->index], &ucode[task->index], ARCH_UCODE_BASE + (task->index * ARCH_TASK_CODESIZE), address, ARCH_TASK_CODESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
        mmu_map(&directories[task->index], &ustack[task->index], ARCH_USTACK_BASE + (task->index * ARCH_TASK_STACKSIZE), ARCH_TASK_STACKBASE, ARCH_TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

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

static struct task *arch_setup_tasks()
{

    unsigned int i;

    for (i = 1; i < ARCH_TASKS; i++)
    {

        init_task(&tasks[i], i);
        task_sched_add(&tasks[i].base);

    }

    return task_sched_find_free_task();

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
    state.tselector.info = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_TSS, (unsigned long)state.tss.pointer.descriptors, (unsigned long)state.tss.pointer.descriptors + state.tss.pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, 0);

    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_GF, arch_isr_generalfault, state.kselector.code, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_PF, arch_isr_pagefault, state.kselector.code, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_SYSCALL, arch_isr_syscall, state.kselector.code, IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_TYPE32INT);
    tss_set_descriptor(&state.tss.pointer, TSS_INDEX_DEFAULT, state.kselector.data, ARCH_KSTACK_LIMIT);
    cpu_set_gdt(&state.gdt.pointer);
    cpu_set_idt(&state.idt.pointer);
    cpu_set_tss(state.tselector.info);

    state.container = kernel_setup();
    state.container->calls[CONTAINER_CALL_SPAWN] = spawn;
    state.container->current = arch_setup_tasks();

    activate_task(state.container->current);
    mmu_enable();
    kernel_setup_modules(count, modules);
    arch_usermode(state.uselector.code, state.uselector.data, state.container->current->registers.ip, state.container->current->registers.sp);

}

