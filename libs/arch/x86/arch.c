#include <fudge.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/binary.h>
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
#define ARCH_KSTACK_BASE                0x00380000
#define ARCH_KSTACK_LIMIT               0x00400000
#define ARCH_KSTACK_SIZE                (ARCH_KSTACK_LIMIT - ARCH_KSTACK_BASE)
#define ARCH_DIRECTORY_KCODE_BASE       0x00400000
#define ARCH_DIRECTORY_KCODE_LIMIT      0x00420000
#define ARCH_DIRECTORY_KCODE_SIZE       (ARCH_DIRECTORY_KCODE_LIMIT - ARCH_DIRECTORY_KCODE_BASE)
#define ARCH_TABLE_KCODE_BASE           0x00420000
#define ARCH_TABLE_KCODE_LIMIT          0x00500000
#define ARCH_TABLE_KCODE_SIZE           (ARCH_TABLE_KCODE_LIMIT - ARCH_TABLE_KCODE_BASE)
#define ARCH_DIRECTORY_UCODE_BASE       0x00500000
#define ARCH_DIRECTORY_UCODE_LIMIT      0x00540000
#define ARCH_DIRECTORY_UCODE_SIZE       (ARCH_DIRECTORY_UCODE_LIMIT - ARCH_DIRECTORY_UCODE_BASE)
#define ARCH_TABLE_UCODE_BASE           0x00540000
#define ARCH_TABLE_UCODE_LIMIT          0x00620000
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
#define ARCH_CONTAINERS                 32
#define ARCH_CONTAINER_MAPPINGS         4
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

static void containermaptext(struct container *container)
{

    struct arch_container *acontainer = (struct arch_container *)container;

    mmu_map(acontainer->directory, &acontainer->table[0], ARCH_BIOS_BASE, ARCH_BIOS_BASE, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(acontainer->directory, &acontainer->table[1], ARCH_DIRECTORY_KCODE_BASE, ARCH_DIRECTORY_KCODE_BASE, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

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

    mmu_map(atask->directory, &atask->table[0], atask->mapping[0], address, ARCH_TASK_CODESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

static void taskmapstack(struct task *task, unsigned int address)
{

    struct arch_task *atask = (struct arch_task *)task;

    mmu_map(atask->directory, &atask->table[1], atask->mapping[1], address, ARCH_TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

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
    task->state.registers.sp = ARCH_TASK_STACKLIMIT;

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
    unsigned int address = cpu_getcr2();

    taskmaptext(current.task, address);
    taskmapstack(current.task, ARCH_TASK_STACKBASE);
    taskcopyprogram(current.task);

    return arch_schedule(&registers->general, &registers->interrupt);

}

unsigned short arch_syscall(void *stack)
{

    struct {struct cpu_general general; struct cpu_interrupt interrupt;} *registers = stack;

    registers->general.eax = kernel_call(registers->general.eax, current.container, current.task, (void *)registers->interrupt.esp);

    return arch_schedule(&registers->general, &registers->interrupt);

}

static void setupcontainer(struct arch_container *container, unsigned int i)
{

    struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_KCODE_BASE;
    struct mmu_table *tables = (struct mmu_table *)ARCH_TABLE_KCODE_BASE;

    container_init(&container->base);

    container->directory = directories + i;
    container->table = tables + i * ARCH_CONTAINER_MAPPINGS;

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

void arch_setup(unsigned int count, struct kernel_module *modules)
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
    kernel_setup(spawn, despawn);

    current.container = setupcontainers();

    containermaptext(current.container);
    containeractivate(current.container);
    mmu_enable();

    current.task = setuptasks();

    kernel_setupmodules(current.container, current.task, count, modules);
    copytask(current.container, current.task, current.task);
    arch_usermode(selector.ucode, selector.udata, current.task->state.registers.ip, current.task->state.registers.sp);

}

