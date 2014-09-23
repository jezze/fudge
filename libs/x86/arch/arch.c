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

static void taskconnect(struct container *container, struct task *task)
{

    struct arch_container *acontainer = (struct arch_container *)container;
    struct arch_task *atask = (struct arch_task *)task;

    memory_copy(atask->directory, acontainer->directory, sizeof (struct mmu_directory));

}

static void taskactivate(struct task *task)
{

    struct arch_task *atask = (struct arch_task *)task;

    mmu_load(atask->directory);

}

static void tasksave(struct task *task, struct cpu_general *general)
{

    struct arch_task *atask = (struct arch_task *)task;

    memory_copy(&atask->general, general, sizeof (struct cpu_general));

}

static void taskload(struct task *task, struct cpu_general *general)
{

    struct arch_task *atask = (struct arch_task *)task;

    memory_copy(general, &atask->general, sizeof (struct cpu_general));

}

static unsigned int spawn(struct container *self, struct task *task, void *stack)
{

    struct parameters {void *caller; unsigned int index;} args;
    struct task *next = scheduler_findfreetask();
    unsigned int i;

    if (!next)
        return 0;

    memory_copy(&args, stack, sizeof (struct parameters));

    for (i = 0; i < 4; i++)
    {

        next->descriptors[i].channel = task->descriptors[i].channel;
        next->descriptors[i].id = task->descriptors[i].id;

    }

    for (i = 4; i < 20; i++)
    {

        next->descriptors[i].channel = task->descriptors[i + 2].channel;
        next->descriptors[i].id = task->descriptors[i + 2].id;

    }

    for (i = 20; i < 32; i++)
    {

        next->descriptors[i].channel = 0;
        next->descriptors[i].id = 0;

    }

    taskconnect(self, next);
    taskactivate(next);
    scheduler_use(next);

    next->registers.ip = self->calls[CONTAINER_CALL_EXECUTE](self, next, &args);
    next->registers.sp = ARCH_TASK_STACKLIMIT;

    return 1;

}

static unsigned int exit(struct container *self, struct task *task, void *stack)
{

    scheduler_unuse(task);

    return 0;

}

unsigned short arch_segment()
{

    return selector.kdata;

}

unsigned short arch_schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    if (current.task)
    {

        current.task->registers.ip = interrupt->eip;
        current.task->registers.sp = interrupt->esp;

        tasksave(current.task, general);

    }

    current.task = scheduler_findusedtask();

    if (current.task)
    {

        interrupt->code = selector.ucode;
        interrupt->eip = current.task->registers.ip;
        interrupt->esp = current.task->registers.sp;

        taskload(current.task, general);
        taskactivate(current.task);

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

        struct arch_task *task = (struct arch_task *)scheduler_findusedtask();
        unsigned int address = cpu_getcr2();

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

static void setupbasic()
{

    gdt_initpointer(&gdt.pointer, ARCH_GDT_DESCRIPTORS, gdt.descriptors);
    idt_initpointer(&idt.pointer, ARCH_IDT_DESCRIPTORS, idt.descriptors);
    tss_initpointer(&tss.pointer, ARCH_TSS_DESCRIPTORS, tss.descriptors);

    selector.kcode = gdt_setdescriptor(&gdt.pointer, GDT_INDEX_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.kdata = gdt_setdescriptor(&gdt.pointer, GDT_INDEX_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.ucode = gdt_setdescriptor(&gdt.pointer, GDT_INDEX_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.udata = gdt_setdescriptor(&gdt.pointer, GDT_INDEX_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.tlink = gdt_setdescriptor(&gdt.pointer, GDT_INDEX_TLINK, (unsigned long)tss.pointer.descriptors, (unsigned long)tss.pointer.descriptors + tss.pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);

    idt_setdescriptor(&idt.pointer, IDT_INDEX_GF, arch_isrgeneralfault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, IDT_INDEX_PF, arch_isrpagefault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, IDT_INDEX_SYSCALL, arch_isrsyscall, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    tss_setdescriptor(&tss.pointer, TSS_INDEX_DEFAULT, selector.kdata, ARCH_KSTACK_LIMIT);
    cpu_setgdt(&gdt.pointer, selector.kcode, selector.kdata);
    cpu_setidt(&idt.pointer);
    cpu_settss(selector.tlink);

}

static void setupcontainer(struct arch_container *container, unsigned int i)
{

    struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_KCODE_BASE;
    struct mmu_table *tables = (struct mmu_table *)ARCH_TABLE_KCODE_BASE;

    container_init(&container->base, spawn, exit);

    container->directory = directories + i;
    container->table = tables + i * ARCH_CONTAINER_MAPPINGS;

    mmu_map(container->directory, &container->table[0], ARCH_BIOS_BASE, ARCH_BIOS_BASE, ARCH_TABLE_UCODE_LIMIT - ARCH_BIOS_BASE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

static struct container *setupcontainers()
{

    unsigned int i;

    for (i = 0; i < ARCH_CONTAINERS; i++)
        setupcontainer(&containers[i], i);

    return &containers[0].base;

}

static void setuptask(struct arch_task *task, unsigned int i)
{

    struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_UCODE_BASE;
    struct mmu_table *tables = (struct mmu_table *)ARCH_TABLE_UCODE_BASE;

    task_init(&task->base, 0, ARCH_TASK_STACKLIMIT);

    task->directory = directories + i;
    task->table = tables + i * ARCH_TASK_MAPPINGS;
    task->mapping[0] = ARCH_UCODE_BASE + ARCH_TASK_CODESIZE * i;
    task->mapping[1] = ARCH_USTACK_BASE + ARCH_TASK_STACKSIZE * i;

    scheduler_register_task(&task->base);

}

static struct task *setuptasks()
{

    unsigned int i;

    for (i = 0; i < ARCH_TASKS; i++)
        setuptask(&tasks[i], i);

    return &tasks[0].base;

}

static void setupmmu(struct container *container, struct task *task)
{

    taskconnect(container, task);
    taskactivate(task);
    mmu_enable();

}

void arch_setup(unsigned int count, struct kernel_module *modules)
{

    setupbasic();
    kernel_setup();

    current.container = setupcontainers();
    current.task = setuptasks();

    scheduler_use(current.task);
    setupmmu(current.container, current.task);
    kernel_setupmodules(current.container, current.task, count, modules);
    arch_usermode(selector.ucode, selector.udata, current.task->registers.ip, current.task->registers.sp);

}

