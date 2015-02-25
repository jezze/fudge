#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "mmu.h"

#define ARCH_CONTAINERS                 8
#define ARCH_TASKS                      64
#define ARCH_GDT_DESCRIPTORS            6
#define ARCH_IDT_DESCRIPTORS            256
#define ARCH_TSS_DESCRIPTORS            1
#define ARCH_DIRECTORY_SIZE             0x1000
#define ARCH_TABLE_SIZE                 0x1000
#define ARCH_KERNEL_BASE                0x00000000
#define ARCH_KERNEL_SIZE                0x00400000
#define ARCH_KERNEL_LIMIT               (ARCH_KERNEL_BASE + ARCH_KERNEL_SIZE)
#define ARCH_CONTAINER_DIRECTORY_COUNT  1
#define ARCH_CONTAINER_DIRECTORY_BASE   ARCH_KERNEL_LIMIT
#define ARCH_CONTAINER_DIRECTORY_LIMIT  (ARCH_CONTAINER_DIRECTORY_BASE + ARCH_CONTAINERS * (ARCH_DIRECTORY_SIZE * ARCH_CONTAINER_DIRECTORY_COUNT))
#define ARCH_CONTAINER_TABLE_COUNT      8
#define ARCH_CONTAINER_TABLE_BASE       ARCH_CONTAINER_DIRECTORY_LIMIT
#define ARCH_CONTAINER_TABLE_LIMIT      (ARCH_CONTAINER_TABLE_BASE + ARCH_CONTAINERS * (ARCH_TABLE_SIZE * ARCH_CONTAINER_TABLE_COUNT))
#define ARCH_TASK_DIRECTORY_COUNT       1
#define ARCH_TASK_DIRECTORY_BASE        ARCH_CONTAINER_TABLE_LIMIT
#define ARCH_TASK_DIRECTORY_LIMIT       (ARCH_TASK_DIRECTORY_BASE + ARCH_TASKS * (ARCH_DIRECTORY_SIZE * ARCH_TASK_DIRECTORY_COUNT))
#define ARCH_TASK_TABLE_COUNT           2
#define ARCH_TASK_TABLE_BASE            ARCH_TASK_DIRECTORY_LIMIT
#define ARCH_TASK_TABLE_LIMIT           (ARCH_TASK_TABLE_BASE + ARCH_TASKS * (ARCH_TABLE_SIZE * ARCH_TASK_TABLE_COUNT))
#define ARCH_TASK_CODE_BASE             0x01000000
#define ARCH_TASK_CODE_SIZE             0x00080000
#define ARCH_TASK_STACK_BASE            (ARCH_TASK_CODE_BASE + ARCH_TASKS * ARCH_TASK_CODE_SIZE)
#define ARCH_TASK_STACK_SIZE            0x00010000
#define ARCH_TASK_VSTACK_LIMIT          0x80000000

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
    unsigned int mapping[ARCH_TASK_TABLE_COUNT];

} tasks[ARCH_TASKS];

static struct
{

    struct container *container;
    struct task *task;

} current;

static void containermaptext(struct container *container)
{

    struct arch_container *acontainer = (struct arch_container *)container;

    mmu_map(acontainer->directory, &acontainer->table[0], ARCH_KERNEL_BASE, ARCH_KERNEL_BASE, ARCH_KERNEL_SIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(acontainer->directory, &acontainer->table[1], 0x00400000, 0x00400000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(acontainer->directory, &acontainer->table[2], 0x00800000, 0x00800000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

static void containeractivate(struct container *container)
{

    struct arch_container *acontainer = (struct arch_container *)container;

    mmu_load(acontainer->directory);

}

static void taskmapcontainer(struct task *task, struct container *container)
{

    struct arch_task *atask = (struct arch_task *)task;
    struct arch_container *acontainer = (struct arch_container *)container;

    memory_copy(atask->directory, acontainer->directory, sizeof (struct mmu_directory));

}

static void taskmaptext(struct task *task, unsigned int address)
{

    struct arch_task *atask = (struct arch_task *)task;

    mmu_map(atask->directory, &atask->table[0], atask->mapping[0], address, ARCH_TASK_CODE_SIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

static void taskmapstack(struct task *task, unsigned int address)
{

    struct arch_task *atask = (struct arch_task *)task;

    mmu_map(atask->directory, &atask->table[1], atask->mapping[1], address, ARCH_TASK_STACK_SIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

static void taskactivate(struct task *task)
{

    struct arch_task *atask = (struct arch_task *)task;

    mmu_load(atask->directory);

}

static void taskprepare(struct task *task)
{

    struct vfs_descriptor *descriptor = &task->descriptors[0x00];
    struct binary_protocol *protocol;

    if (!descriptor->id || !descriptor->channel)
        return;

    protocol = binary_findprotocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return;

    task->state.registers.ip = protocol->findentry(descriptor->channel, descriptor->id);
    task->state.registers.sp = ARCH_TASK_VSTACK_LIMIT;

}

static void taskcopyprogram(struct task *task)
{

    struct vfs_descriptor *descriptor = &task->descriptors[0x00];
    struct binary_protocol *protocol;

    if (!descriptor->id || !descriptor->channel)
        return;

    protocol = binary_findprotocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return;

    protocol->copyprogram(descriptor->channel, descriptor->id);

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

static void copytask(struct container *container, struct task *next, struct task *task)
{

    unsigned int i;

    for (i = 0x00; i < 0x08; i++)
    {

        next->descriptors[i + 0x00].channel = task->descriptors[i + 0x08].channel;
        next->descriptors[i + 0x00].id = task->descriptors[i + 0x08].id;
        next->descriptors[i + 0x08].channel = task->descriptors[i + 0x08].channel;
        next->descriptors[i + 0x08].id = task->descriptors[i + 0x08].id;
        next->descriptors[i + 0x10].channel = 0;
        next->descriptors[i + 0x10].id = 0;
        next->descriptors[i + 0x18].channel = 0;
        next->descriptors[i + 0x18].id = 0;

    }

    scheduler_use(next);
    taskmapcontainer(next, container);
    taskactivate(next);
    taskprepare(next);

}

static unsigned int spawn(struct container *container, struct task *task, void *stack)
{

    struct task *next = scheduler_findinactive();

    if (!next)
        return 0;

    copytask(container, next, task);

    return 1;

}

static unsigned int despawn(struct container *container, struct task *task, void *stack)
{

    scheduler_unuse(task);

    return 0;

}

unsigned short arch_getsegment()
{

    return selector.kdata;

}

void arch_setinterrupt(unsigned int index, void (*callback)())
{

    idt_setdescriptor(&idt.pointer, index, callback, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

}

unsigned short arch_schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    if (current.task)
    {

        current.task->state.registers.ip = interrupt->eip;
        current.task->state.registers.sp = interrupt->esp;

        tasksave(current.task, general);

    }

    current.task = scheduler_findactive();

    if (current.task)
    {

        interrupt->code = selector.ucode;
        interrupt->eip = current.task->state.registers.ip;
        interrupt->esp = current.task->state.registers.sp;

        taskload(current.task, general);
        taskactivate(current.task);

        return selector.udata;

    }

    else
    {

        interrupt->code = selector.kcode;
        interrupt->eip = (unsigned int)arch_halt;
        interrupt->esp = ARCH_KERNEL_LIMIT;

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
    unsigned int address = cpu_getcr2();

    taskmaptext(current.task, address);
    taskmapstack(current.task, ARCH_TASK_VSTACK_LIMIT - ARCH_TASK_STACK_SIZE);
    taskcopyprogram(current.task);

    return arch_schedule(&registers->general, &registers->interrupt);

}

unsigned short arch_syscall(void *stack)
{

    struct {struct cpu_general general; struct cpu_interrupt interrupt;} *registers = stack;

    registers->general.eax = kernel_call(registers->general.eax, current.container, current.task, (void *)registers->interrupt.esp);

    return arch_schedule(&registers->general, &registers->interrupt);

}

static void setupcontainer(struct arch_container *container, unsigned int i, struct mmu_directory *directories, struct mmu_table *tables)
{

    container_init(&container->base);

    container->directory = directories + i * ARCH_CONTAINER_DIRECTORY_COUNT;
    container->table = tables + i * ARCH_CONTAINER_TABLE_COUNT;

}

static struct container *setupcontainers()
{

    unsigned int i;

    for (i = 0; i < ARCH_CONTAINERS; i++)
        setupcontainer(&containers[i], i, (struct mmu_directory *)ARCH_CONTAINER_DIRECTORY_BASE, (struct mmu_table *)ARCH_CONTAINER_TABLE_BASE);

    return &containers[0].base;

}

static void setuptask(struct arch_task *task, unsigned int i, struct mmu_directory *directories, struct mmu_table *tables)
{

    task_init(&task->base);

    task->directory = directories + i * ARCH_TASK_DIRECTORY_COUNT;
    task->table = tables + i * ARCH_TASK_TABLE_COUNT;
    task->mapping[0] = ARCH_TASK_CODE_BASE + ARCH_TASK_CODE_SIZE * i;
    task->mapping[1] = ARCH_TASK_STACK_BASE + ARCH_TASK_STACK_SIZE * i;

    scheduler_register_task(&task->base);

}

static struct task *setuptasks()
{

    unsigned int i;

    for (i = 0; i < ARCH_TASKS; i++)
        setuptask(&tasks[i], i, (struct mmu_directory *)ARCH_TASK_DIRECTORY_BASE, (struct mmu_table *)ARCH_TASK_TABLE_BASE);

    return &tasks[0].base;

}

void arch_setup(struct vfs_backend *backend)
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
    tss_setdescriptor(&tss.pointer, TSS_INDEX_DEFAULT, selector.kdata, ARCH_KERNEL_LIMIT);
    cpu_setgdt(&gdt.pointer, selector.kcode, selector.kdata);
    cpu_setidt(&idt.pointer);
    cpu_settss(selector.tlink);
    kernel_setup(spawn, despawn);

    current.container = setupcontainers();

    containermaptext(current.container);
    containeractivate(current.container);
    mmu_enable();

    current.task = setuptasks();

    kernel_setupramdisk(current.container, current.task, backend);
    copytask(current.container, current.task, current.task);
    arch_usermode(selector.ucode, selector.udata, current.task->state.registers.ip, current.task->state.registers.sp);

}

