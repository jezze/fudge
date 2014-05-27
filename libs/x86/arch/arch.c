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

struct arch_table
{

    struct {struct gdt_pointer pointer; struct gdt_descriptor descriptors[ARCH_GDT_DESCRIPTORS];} gdt;
    struct {struct idt_pointer pointer; struct idt_descriptor descriptors[ARCH_IDT_DESCRIPTORS];} idt;
    struct {struct tss_pointer pointer; struct tss_descriptor descriptors[ARCH_TSS_DESCRIPTORS];} tss;
    unsigned short kcode;
    unsigned short kdata;
    unsigned short ucode;
    unsigned short udata;
    unsigned short tlink;

};

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

    struct arch_table table;
    struct arch_container containers[ARCH_CONTAINERS];
    struct arch_task tasks[ARCH_TASKS];
    struct {struct container *container; struct task *task;} current;

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

    if (!next)
        return 0;

    memory_copy(&args, stack, sizeof (struct parameters));

    args.index -= 2;

    activate_task(self, next);
    scheduler_use(next);

    next->registers.ip = 0;
    next->registers.sp = ARCH_TASK_STACKLIMIT;

    memory_clear(next->descriptors, sizeof (struct vfs_descriptor) * TASK_DESCRIPTORS);
    memory_copy(next->descriptors, task->descriptors, sizeof (struct vfs_descriptor) * 4);
    memory_copy(next->descriptors + 4, task->descriptors + 6, sizeof (struct vfs_descriptor) * 22);

    return self->calls[CONTAINER_CALL_EXECUTE](self, next, &args);

}

static unsigned int exit(struct container *self, struct task *task, void *stack)
{

    scheduler_unuse(task);

    return 0;

}

unsigned short arch_segment()
{

    return state.table.kdata;

}

unsigned short arch_schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct arch_task *task = (struct arch_task *)state.current.task;
    struct arch_task *next = (struct arch_task *)scheduler_find_used_task();

    if (task)
    {

        if (task == next)
            return state.table.udata;

        task->base.registers.ip = interrupt->eip;
        task->base.registers.sp = interrupt->esp;

        memory_copy(&task->general, general, sizeof (struct cpu_general));

    }

    if (next)
    {

        mmu_load(next->directory);

        interrupt->code = state.table.ucode;
        interrupt->eip = next->base.registers.ip;
        interrupt->esp = next->base.registers.sp;

        memory_copy(general, &next->general, sizeof (struct cpu_general));

        state.current.task = &next->base;

        return state.table.udata;

    }

    else
    {

        interrupt->code = state.table.kcode;
        interrupt->eip = (unsigned int)arch_halt;
        interrupt->esp = ARCH_KSTACK_LIMIT;

        state.current.task = 0;

        return state.table.kdata;

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

    if (registers->interrupt.code == state.table.kcode)
    {

        struct arch_task *task = (struct arch_task *)scheduler_find_used_task();
        unsigned int address = cpu_get_cr2();

        mmu_map(task->directory, &task->table[0], task->mapping[0], address, ARCH_TASK_CODESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
        mmu_map(task->directory, &task->table[1], task->mapping[1], ARCH_TASK_STACKBASE, ARCH_TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

        return state.table.kdata;

    }

    return arch_schedule(&registers->general, &registers->interrupt);

}

unsigned short arch_syscall(void *stack)
{

    struct {struct cpu_general general; struct cpu_interrupt interrupt;} *registers = stack;

    registers->general.eax = (state.current.container->calls[registers->general.eax]) ? state.current.container->calls[registers->general.eax](state.current.container, state.current.task, (void *)registers->interrupt.esp) : 0;

    return arch_schedule(&registers->general, &registers->interrupt);

}

static void arch_setup_table(struct arch_table *table)
{

    gdt_init_pointer(&table->gdt.pointer, ARCH_GDT_DESCRIPTORS, table->gdt.descriptors);
    idt_init_pointer(&table->idt.pointer, ARCH_IDT_DESCRIPTORS, table->idt.descriptors);
    tss_init_pointer(&table->tss.pointer, ARCH_TSS_DESCRIPTORS, table->tss.descriptors);

    table->kcode = gdt_set_descriptor(&table->gdt.pointer, GDT_INDEX_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    table->kdata = gdt_set_descriptor(&table->gdt.pointer, GDT_INDEX_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    table->ucode = gdt_set_descriptor(&table->gdt.pointer, GDT_INDEX_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    table->udata = gdt_set_descriptor(&table->gdt.pointer, GDT_INDEX_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    table->tlink = gdt_set_descriptor(&table->gdt.pointer, GDT_INDEX_TLINK, (unsigned long)table->tss.pointer.descriptors, (unsigned long)table->tss.pointer.descriptors + table->tss.pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);

    idt_set_descriptor(&table->idt.pointer, IDT_INDEX_GF, arch_isr_generalfault, table->kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&table->idt.pointer, IDT_INDEX_PF, arch_isr_pagefault, table->kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&table->idt.pointer, IDT_INDEX_SYSCALL, arch_isr_syscall, table->kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    tss_set_descriptor(&table->tss.pointer, TSS_INDEX_DEFAULT, table->kdata, ARCH_KSTACK_LIMIT);
    cpu_set_gdt(&table->gdt.pointer, table->kcode, table->kdata);
    cpu_set_idt(&table->idt.pointer);
    cpu_set_tss(table->tlink);

}

static struct container *arch_setup_containers(struct arch_container *containers)
{

    unsigned int i;

    for (i = 0; i < ARCH_CONTAINERS; i++)
    {

        struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_KCODE_BASE;
        struct mmu_table *tables = (struct mmu_table *)ARCH_TABLE_KCODE_BASE;

        container_init(&containers[i].base, spawn, exit);
        resource_register_item(&containers[i].base.resource);

        containers[i].directory = directories + i;
        containers[i].table = tables + i * ARCH_CONTAINER_MAPPINGS;

        mmu_map(containers[i].directory, &containers[i].table[0], ARCH_BIOS_BASE, ARCH_BIOS_BASE, ARCH_TABLE_UCODE_LIMIT - ARCH_BIOS_BASE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

    }

    return &containers[0].base;

}

static struct task *arch_setup_tasks(struct arch_task *tasks)
{

    unsigned int i;

    for (i = 0; i < ARCH_TASKS; i++)
    {

        struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_UCODE_BASE;
        struct mmu_table *tables = (struct mmu_table *)ARCH_TABLE_UCODE_BASE;

        task_init(&tasks[i].base, 0, ARCH_TASK_STACKLIMIT);
        resource_register_item(&tasks[i].base.resource);
        scheduler_register_task(&tasks[i].base);

        tasks[i].directory = directories + i;
        tasks[i].table = tables + i * ARCH_TASK_MAPPINGS;
        tasks[i].mapping[0] = ARCH_UCODE_BASE + ARCH_TASK_CODESIZE * i;
        tasks[i].mapping[1] = ARCH_USTACK_BASE + ARCH_TASK_STACKSIZE * i;

    }

    return &tasks[0].base;

}

void arch_setup(unsigned int count, struct kernel_module *modules)
{

    arch_setup_table(&state.table);
    kernel_setup();

    state.current.container = arch_setup_containers(state.containers);
    state.current.task = arch_setup_tasks(state.tasks);

    activate_task(state.current.container, state.current.task);
    scheduler_use(state.current.task);
    mmu_enable();
    kernel_setup_modules(state.current.container, state.current.task, count, modules);
    arch_usermode(state.table.ucode, state.table.udata, state.current.task->registers.ip, state.current.task->registers.sp);

}

