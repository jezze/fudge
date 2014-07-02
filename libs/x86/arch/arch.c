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
#define ARCH_UCODE_LIMIT                0x03000000
#define ARCH_UCODE_SIZE                 (ARCH_UCODE_LIMIT - ARCH_UCODE_BASE)
#define ARCH_USTACK_BASE                0x03000000
#define ARCH_USTACK_LIMIT               0x03400000
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

static struct
{

    struct gdt_pointer pointer;
    struct gdt_descriptor descriptors[ARCH_GDT_DESCRIPTORS];

} gdt;

static struct
{

    struct idt_pointer pointer;
    struct idt_descriptor descriptors[ARCH_IDT_DESCRIPTORS];

} idt;

static struct
{

    struct tss_pointer pointer;
    struct tss_descriptor descriptors[ARCH_TSS_DESCRIPTORS];

} tss;

static struct
{

    unsigned short kcode;
    unsigned short kdata;
    unsigned short ucode;
    unsigned short udata;
    unsigned short tlink;

} selector;

static struct arch_container
{

    struct container base;
    struct mmu_directory *directory;
    struct mmu_table *table;

} containers[ARCH_CONTAINERS];

static struct arch_task
{

    struct task base;
    struct cpu_general general;
    struct mmu_directory *directory;
    struct mmu_table *table;
    unsigned int mapping[ARCH_TASK_MAPPINGS];

} tasks[ARCH_TASKS];

static struct
{

    struct container *container;
    struct task *task;

} current;

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

    activate_task(self, next);
    scheduler_use(next);

    next->registers.ip = 0;
    next->registers.sp = ARCH_TASK_STACKLIMIT;

    memory_copy(next->descriptors, task->descriptors, sizeof (struct vfs_descriptor) * 4);
    memory_copy(next->descriptors + 4, task->descriptors + 6, sizeof (struct vfs_descriptor) * 18);
    memory_clear(next->descriptors + 22, sizeof (struct vfs_descriptor) * 10);

    return self->calls[CONTAINER_CALL_EXECUTE](self, next, &args);

}

static unsigned int exit(struct container *self, struct task *task, void *stack)
{

    scheduler_unuse(task);

    return 0;

}

static void arch_init_container(struct arch_container *container, struct mmu_directory *directory, struct mmu_table *table)
{

    memory_clear(container, sizeof (struct arch_container));
    container_init(&container->base, spawn, exit);

    container->directory = directory;
    container->table = table;

}

static void arch_init_task(struct arch_task *task, struct mmu_directory *directory, struct mmu_table *table, unsigned int code, unsigned int stack)
{

    memory_clear(task, sizeof (struct arch_task));
    task_init(&task->base, 0, ARCH_TASK_STACKLIMIT);

    task->directory = directory;
    task->table = table;
    task->mapping[0] = code;
    task->mapping[1] = stack;

}

unsigned short arch_segment()
{

    return selector.kdata;

}

/* Should be removed */
static void arch_task_save(struct cpu_general *general)
{

    struct arch_task *task = (struct arch_task *)current.task;

    memory_copy(&task->general, general, sizeof (struct cpu_general));

}

/* Should be removed */
static void arch_task_load(struct cpu_general *general)
{

    struct arch_task *task = (struct arch_task *)current.task;

    /* This mmu_load should be handled by a page fault */
    mmu_load(task->directory);
    memory_copy(general, &task->general, sizeof (struct cpu_general));

}

unsigned short arch_schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    if (current.task)
    {

        current.task->registers.ip = interrupt->eip;
        current.task->registers.sp = interrupt->esp;

        arch_task_save(general);

    }

    current.task = scheduler_find_used_task();

    if (current.task)
    {

        interrupt->code = selector.ucode;
        interrupt->eip = current.task->registers.ip;
        interrupt->esp = current.task->registers.sp;

        arch_task_load(general);

        return selector.udata;

    }

    else
    {

        interrupt->code = selector.kcode;
        interrupt->eip = (unsigned int)arch_halt;
        interrupt->esp = ARCH_KSTACK_LIMIT;

        return selector.kdata;

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

    if (registers->interrupt.code == selector.kcode)
    {

        struct arch_task *task = (struct arch_task *)scheduler_find_used_task();
        unsigned int address = cpu_get_cr2();

        mmu_map(task->directory, &task->table[0], task->mapping[0], address, ARCH_TASK_CODESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
        mmu_map(task->directory, &task->table[1], task->mapping[1], ARCH_TASK_STACKBASE, ARCH_TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

        return selector.kdata;

    }

    return arch_schedule(&registers->general, &registers->interrupt);

}

unsigned short arch_syscall(void *stack)
{

    struct {struct cpu_general general; struct cpu_interrupt interrupt;} *registers = stack;

    registers->general.eax = (current.container->calls[registers->general.eax]) ? current.container->calls[registers->general.eax](current.container, current.task, (void *)registers->interrupt.esp) : 0;

    return arch_schedule(&registers->general, &registers->interrupt);

}

static void arch_setup_basic()
{

    gdt_init_pointer(&gdt.pointer, ARCH_GDT_DESCRIPTORS, gdt.descriptors);
    idt_init_pointer(&idt.pointer, ARCH_IDT_DESCRIPTORS, idt.descriptors);
    tss_init_pointer(&tss.pointer, ARCH_TSS_DESCRIPTORS, tss.descriptors);

    selector.kcode = gdt_set_descriptor(&gdt.pointer, GDT_INDEX_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.kdata = gdt_set_descriptor(&gdt.pointer, GDT_INDEX_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.ucode = gdt_set_descriptor(&gdt.pointer, GDT_INDEX_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.udata = gdt_set_descriptor(&gdt.pointer, GDT_INDEX_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.tlink = gdt_set_descriptor(&gdt.pointer, GDT_INDEX_TLINK, (unsigned long)tss.pointer.descriptors, (unsigned long)tss.pointer.descriptors + tss.pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);

    idt_set_descriptor(&idt.pointer, IDT_INDEX_GF, arch_isr_generalfault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&idt.pointer, IDT_INDEX_PF, arch_isr_pagefault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(&idt.pointer, IDT_INDEX_SYSCALL, arch_isr_syscall, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    tss_set_descriptor(&tss.pointer, TSS_INDEX_DEFAULT, selector.kdata, ARCH_KSTACK_LIMIT);
    cpu_set_gdt(&gdt.pointer, selector.kcode, selector.kdata);
    cpu_set_idt(&idt.pointer);
    cpu_set_tss(selector.tlink);

}

static void arch_setup_containers()
{

    unsigned int i;

    for (i = 0; i < ARCH_CONTAINERS; i++)
    {

        struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_KCODE_BASE;
        struct mmu_table *tables = (struct mmu_table *)ARCH_TABLE_KCODE_BASE;

        arch_init_container(&containers[i], directories + i, tables + i * ARCH_CONTAINER_MAPPINGS);
        resource_register(&containers[i].base.resource);
        mmu_map(containers[i].directory, &containers[i].table[0], ARCH_BIOS_BASE, ARCH_BIOS_BASE, ARCH_TABLE_UCODE_LIMIT - ARCH_BIOS_BASE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

    }

}

static void arch_setup_tasks()
{

    unsigned int i;

    for (i = 0; i < ARCH_TASKS; i++)
    {

        struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_UCODE_BASE;
        struct mmu_table *tables = (struct mmu_table *)ARCH_TABLE_UCODE_BASE;

        arch_init_task(&tasks[i], directories + i, tables + i * ARCH_TASK_MAPPINGS, ARCH_UCODE_BASE + ARCH_TASK_CODESIZE * i, ARCH_USTACK_BASE + ARCH_TASK_STACKSIZE * i);
        resource_register(&tasks[i].base.resource);
        scheduler_register_task(&tasks[i].base);

    }

}

void arch_setup(unsigned int count, struct kernel_module *modules)
{

    arch_setup_basic();
    kernel_setup();
    arch_setup_containers();
    arch_setup_tasks();

    current.container = &containers[0].base;
    current.task = &tasks[0].base;

    activate_task(current.container, current.task);
    scheduler_use(current.task);
    mmu_enable();
    kernel_setup_modules(current.container, current.task, count, modules);
    arch_usermode(selector.ucode, selector.udata, current.task->registers.ip, current.task->registers.sp);

}

