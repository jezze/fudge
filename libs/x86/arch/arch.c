#include <kernel.h>
#include <kernel/error.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/scheduler.h>
#include <kernel/container.h>
#include <kernel/kernel.h>
#include "cpu.h"
#include "arch.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "mmu.h"

#define ARCH_BIOS_BASE                  0x00000000
#define ARCH_BIOS_LIMIT                 0x00100000
#define ARCH_BIOS_SIZE                  (ARCH_BIOS_LIMIT - ARCH_BIOS_BASE)
#define ARCH_KCODE_BASE                 0x00100000
#define ARCH_KCODE_LIMIT                0x00280000
#define ARCH_KCODE_SIZE                 (ARCH_KCODE_LIMIT - ARCH_KCODE_BASE)
#define ARCH_KSTACK_BASE                0x00280000
#define ARCH_KSTACK_LIMIT               0x00300000
#define ARCH_KSTACK_SIZE                (ARCH_KSTACK_LIMIT - ARCH_KSTACK_BASE)
#define ARCH_DIRECTORY_KCODE_BASE       0x00300000
#define ARCH_DIRECTORY_KCODE_LIMIT      0x00310000
#define ARCH_DIRECTORY_KCODE_SIZE       (ARCH_DIRECTORY_KCODE_LIMIT - ARCH_DIRECTORY_KCODE_BASE)
#define ARCH_TABLE_KCODE_BASE           0x00310000
#define ARCH_TABLE_KCODE_LIMIT          0x00340000
#define ARCH_TABLE_KCODE_SIZE           (ARCH_TABLE_KCODE_LIMIT - ARCH_TABLE_KCODE_BASE)
#define ARCH_DIRECTORY_UCODE_BASE       0x00340000
#define ARCH_DIRECTORY_UCODE_LIMIT      0x00380000
#define ARCH_DIRECTORY_UCODE_SIZE       (ARCH_DIRECTORY_UCODE_LIMIT - ARCH_DIRECTORY_UCODE_BASE)
#define ARCH_TABLE_UCODE_BASE           0x00380000
#define ARCH_TABLE_UCODE_LIMIT          0x00400000
#define ARCH_TABLE_UCODE_SIZE           (ARCH_TABLE_UCODE_LIMIT - ARCH_TABLE_UCODE_BASE)
#define ARCH_UCODE_BASE                 0x01000000
#define ARCH_UCODE_LIMIT                0x01400000
#define ARCH_UCODE_SIZE                 (ARCH_UCODE_LIMIT - ARCH_UCODE_BASE)
#define ARCH_USTACK_BASE                0x01400000
#define ARCH_USTACK_LIMIT               0x01800000
#define ARCH_USTACK_SIZE                (ARCH_USTACK_LIMIT - ARCH_USTACK_BASE)
#define ARCH_GDT_DESCRIPTORS            6
#define ARCH_IDT_DESCRIPTORS            256
#define ARCH_TSS_DESCRIPTORS            1
#define ARCH_CONTAINERS                 16
#define ARCH_CONTAINER_MAPPINGS         3
#define ARCH_TASKS                      64
#define ARCH_TASK_MAPPINGS              2
#define ARCH_TASK_CODESIZE              (ARCH_UCODE_SIZE / ARCH_TASKS)
#define ARCH_TASK_STACKSIZE             (ARCH_USTACK_SIZE / ARCH_TASKS)
#define ARCH_TASK_STACKLIMIT            0x80000000
#define ARCH_TASK_STACKBASE             (ARCH_TASK_STACKLIMIT - ARCH_TASK_STACKSIZE)

struct arch_container
{

    struct container base;
    struct mmu_directory *directory;
    struct mmu_table *table;

};

struct arch_task
{

    struct task base;
    struct cpu_general general;
    struct mmu_directory *directory;
    struct mmu_table *table;
    unsigned int mapping[ARCH_TASK_MAPPINGS];

};

static struct
{

    struct container *container;
    struct arch_container containers[ARCH_CONTAINERS];
    struct task *task;
    struct arch_task tasks[ARCH_TASKS];
    struct {struct gdt_pointer pointer; struct gdt_descriptor descriptors[ARCH_GDT_DESCRIPTORS];} gdt;
    struct {struct idt_pointer pointer; struct idt_descriptor descriptors[ARCH_IDT_DESCRIPTORS];} idt;
    struct {struct tss_pointer pointer; struct tss_descriptor descriptors[ARCH_TSS_DESCRIPTORS];} tss;
    unsigned short kcode;
    unsigned short kdata;
    unsigned short ucode;
    unsigned short udata;
    unsigned short tlink;

} state;

static void activate_task(struct container *c, struct task *t)
{

    struct arch_container *container = (struct arch_container *)c;
    struct arch_task *task = (struct arch_task *)t;

    memory_copy(task->directory, container->directory, sizeof (struct mmu_directory));
    mmu_load(task->directory);

}

static unsigned int spawn(struct container *self, struct task *task, void *stack)
{

    struct parameters {void *caller; unsigned int index;} args;
    struct task *next = scheduler_find_free_task();
    unsigned int i;

    if (!next)
        return 0;

    memory_copy(&args, stack, sizeof (struct parameters));

    next->registers.ip = 0;
    next->registers.sp = ARCH_TASK_STACKLIMIT;

    activate_task(self, next);
    scheduler_use(next);

    for (i = 0; i < 4; i++)
        vfs_init_descriptor(&next->descriptors[i], task->descriptors[i].channel, task->descriptors[i].id);

    for (i = 4; i < 22; i++)
        vfs_init_descriptor(&next->descriptors[i], task->descriptors[i + 2].channel, task->descriptors[i + 2].id);

    for (i = 22; i < TASK_DESCRIPTORS; i++)
        vfs_init_descriptor(&next->descriptors[i], 0, 0);

    vfs_init_descriptor(&next->descriptors[args.index], task->descriptors[args.index].channel, task->descriptors[args.index].id);

    return self->calls[CONTAINER_CALL_EXECUTE](self, next, &args);

}

static unsigned int exit(struct container *self, struct task *task, void *stack)
{

    scheduler_unuse(task);

    return 0;

}

unsigned short arch_segment()
{

    return state.kdata;

}

unsigned short arch_schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct arch_task *task = (struct arch_task *)state.task;
    struct arch_task *next = (struct arch_task *)scheduler_find_used_task();

    if (task)
    {

        if (task == next)
            return state.udata;

        task->base.registers.ip = interrupt->eip;
        task->base.registers.sp = interrupt->esp;

        memory_copy(&task->general, general, sizeof (struct cpu_general));

    }

    if (next)
    {

        mmu_load(next->directory);

        interrupt->code = state.ucode;
        interrupt->eip = next->base.registers.ip;
        interrupt->esp = next->base.registers.sp;

        memory_copy(general, &next->general, sizeof (struct cpu_general));

        state.task = &next->base;

        return state.udata;

    }

    else
    {

        interrupt->code = state.kcode;
        interrupt->eip = (unsigned int)arch_halt;
        interrupt->esp = ARCH_KSTACK_LIMIT;

        state.task = 0;

        return state.kdata;

    }

}

unsigned short arch_generalfault(void *stack)
{

    struct {struct cpu_general general; unsigned int selector; struct cpu_interrupt interrupt;} *registers = stack;

    return arch_schedule(&registers->general, &registers->interrupt);

}

unsigned short arch_pagefault(void *stack)
{

    struct {struct cpu_general general; unsigned int type; struct cpu_interrupt interrupt;} *registers = stack;

    if (registers->interrupt.code == state.kcode)
    {

        struct arch_task *task = (struct arch_task *)scheduler_find_used_task();
        unsigned int address = cpu_get_cr2();

        mmu_map(task->directory, &task->table[0], task->mapping[0], address, ARCH_TASK_CODESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
        mmu_map(task->directory, &task->table[1], task->mapping[1], ARCH_TASK_STACKBASE, ARCH_TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

        return state.kdata;

    }

    return arch_schedule(&registers->general, &registers->interrupt);

}

unsigned short arch_syscall(void *stack)
{

    struct {struct cpu_general general; struct cpu_interrupt interrupt;} *registers = stack;

    registers->general.eax = (state.container->calls[registers->general.eax]) ? state.container->calls[registers->general.eax](state.container, state.task, (void *)registers->interrupt.esp) : 0;

    return arch_schedule(&registers->general, &registers->interrupt);

}

static void arch_setup_entities()
{

    unsigned int i;

    for (i = 0; i < ARCH_CONTAINERS; i++)
    {

        struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_KCODE_BASE;
        struct mmu_table *tables = (struct mmu_table *)ARCH_TABLE_KCODE_BASE;

        container_init(&state.containers[i].base, spawn, exit);
        resource_register_item(&state.containers[i].base.resource);

        state.containers[i].directory = directories + i;
        state.containers[i].table = tables + i * ARCH_CONTAINER_MAPPINGS;

        mmu_map(state.containers[i].directory, &state.containers[i].table[0], ARCH_BIOS_BASE, ARCH_BIOS_BASE, ARCH_TABLE_UCODE_LIMIT - ARCH_BIOS_BASE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

    }

    for (i = 0; i < ARCH_TASKS; i++)
    {

        struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_UCODE_BASE;
        struct mmu_table *tables = (struct mmu_table *)ARCH_TABLE_UCODE_BASE;

        task_init(&state.tasks[i].base, 0, ARCH_TASK_STACKLIMIT);
        resource_register_item(&state.tasks[i].base.resource);
        scheduler_register_task(&state.tasks[i].base);

        state.tasks[i].directory = directories + i;
        state.tasks[i].table = tables + i * ARCH_TASK_MAPPINGS;
        state.tasks[i].mapping[0] = ARCH_UCODE_BASE + ARCH_TASK_CODESIZE * i;
        state.tasks[i].mapping[1] = ARCH_USTACK_BASE + ARCH_TASK_STACKSIZE * i;

    }

    state.container = &state.containers[0].base;
    state.task = &state.tasks[0].base;

    activate_task(state.container, state.task);
    scheduler_use(state.task);

}

void arch_setup(unsigned int count, struct kernel_module *modules)
{

    gdt_init_pointer(&state.gdt.pointer, ARCH_GDT_DESCRIPTORS, state.gdt.descriptors);
    idt_init_pointer(&state.idt.pointer, ARCH_IDT_DESCRIPTORS, state.idt.descriptors);
    tss_init_pointer(&state.tss.pointer, ARCH_TSS_DESCRIPTORS, state.tss.descriptors);

    state.kcode = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.kdata = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.ucode = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.udata = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    state.tlink = gdt_set_descriptor(&state.gdt.pointer, GDT_INDEX_TLINK, (unsigned long)state.tss.pointer.descriptors, (unsigned long)state.tss.pointer.descriptors + state.tss.pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);

    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_GF, arch_isr_generalfault, state.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_PF, arch_isr_pagefault, state.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&state.idt.pointer, IDT_INDEX_SYSCALL, arch_isr_syscall, state.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    tss_set_descriptor(&state.tss.pointer, TSS_INDEX_DEFAULT, state.kdata, ARCH_KSTACK_LIMIT);
    cpu_set_gdt(&state.gdt.pointer, state.kcode, state.kdata);
    cpu_set_idt(&state.idt.pointer);
    cpu_set_tss(state.tlink);
    kernel_setup();
    arch_setup_entities();
    mmu_enable();
    kernel_setup_modules(state.container, state.task, count, modules);
    arch_usermode(state.ucode, state.udata, state.task->registers.ip, state.task->registers.sp);

}

