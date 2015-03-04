#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "mmu.h"

#define CONTAINERS                      8
#define TASKS                           64
#define GDTDESCRIPTORS                  6
#define IDTDESCRIPTORS                  256
#define TSSDESCRIPTORS                  1
#define KERNELBASE                      0x00000000
#define KERNELSIZE                      0x00400000
#define KERNELLIMIT                     (KERNELBASE + KERNELSIZE)
#define MMUALIGN                        0x1000
#define CONTAINERDIRECTORYCOUNT         1
#define CONTAINERDIRECTORYBASE          KERNELLIMIT
#define CONTAINERDIRECTORYLIMIT         (CONTAINERDIRECTORYBASE + CONTAINERS * (MMUALIGN * CONTAINERDIRECTORYCOUNT))
#define CONTAINERTABLECOUNT             8
#define CONTAINERTABLEBASE              CONTAINERDIRECTORYLIMIT
#define CONTAINERTABLELIMIT             (CONTAINERTABLEBASE + CONTAINERS * (MMUALIGN * CONTAINERTABLECOUNT))
#define TASKDIRECTORYCOUNT              1
#define TASKDIRECTORYBASE               CONTAINERTABLELIMIT
#define TASKDIRECTORYLIMIT              (TASKDIRECTORYBASE + TASKS * (MMUALIGN * TASKDIRECTORYCOUNT))
#define TASKTABLECOUNT                  2
#define TASKTABLEBASE                   TASKDIRECTORYLIMIT
#define TASKTABLELIMIT                  (TASKTABLEBASE + TASKS * (MMUALIGN * TASKTABLECOUNT))
#define TASKCODEBASE                    0x01000000
#define TASKCODESIZE                    0x00080000
#define TASKSTACKBASE                   (TASKCODEBASE + TASKS * TASKCODESIZE)
#define TASKSTACKSIZE                   0x00010000
#define TASKVSTACKLIMIT                 0x80000000

static struct
{

    struct gdt_pointer pointer;
    struct gdt_descriptor descriptors[GDTDESCRIPTORS];

} gdt;

static struct
{

    struct idt_pointer pointer;
    struct idt_descriptor descriptors[IDTDESCRIPTORS];

} idt;

static struct
{

    struct tss_pointer pointer;
    struct tss_descriptor descriptors[TSSDESCRIPTORS];

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

} containers[CONTAINERS];

static struct arch_task
{

    struct task base;
    struct cpu_general general;
    struct mmu_directory *directory;
    struct mmu_table *table;
    unsigned int mapping[TASKTABLECOUNT];

} tasks[TASKS];

static struct
{

    struct container *container;
    struct task *task;

} current;

static void containermaptext(struct container *container)
{

    struct arch_container *acontainer = (struct arch_container *)container;

    mmu_map(acontainer->directory, &acontainer->table[0], KERNELBASE, KERNELBASE, KERNELSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(acontainer->directory, &acontainer->table[1], 0x00400000, 0x00400000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);
    mmu_map(acontainer->directory, &acontainer->table[2], 0x00800000, 0x00800000, 0x00400000, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

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

    mmu_map(atask->directory, &atask->table[0], atask->mapping[0], address, TASKCODESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

static void taskmapstack(struct task *task, unsigned int address)
{

    struct arch_task *atask = (struct arch_task *)task;

    mmu_map(atask->directory, &atask->table[1], atask->mapping[1], address, TASKSTACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

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

static unsigned int spawn(struct container *container, struct task *task, void *stack)
{

    struct task *next = scheduler_findinactive();

    if (!next)
        return 0;

    kernel_copytask(task, next);
    kernel_setuptask(next, TASKVSTACKLIMIT);
    taskmapcontainer(next, container);
    scheduler_use(next);

    return 1;

}

static unsigned int despawn(struct container *container, struct task *task, void *stack)
{

    scheduler_unuse(task);

    return 1;

}

unsigned short arch_getsegment()
{

    return selector.kdata;

}

void arch_setinterrupt(unsigned char index, void (*callback)())
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
        interrupt->esp = KERNELLIMIT;

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
    taskmapstack(current.task, TASKVSTACKLIMIT - TASKSTACKSIZE);
    kernel_copyprogram(current.task);

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

    container->directory = directories + i * CONTAINERDIRECTORYCOUNT;
    container->table = tables + i * CONTAINERTABLECOUNT;

}

static struct container *setupcontainers()
{

    unsigned int i;

    for (i = 0; i < CONTAINERS; i++)
        setupcontainer(&containers[i], i, (struct mmu_directory *)CONTAINERDIRECTORYBASE, (struct mmu_table *)CONTAINERTABLEBASE);

    return &containers[0].base;

}

static void setuptask(struct arch_task *task, unsigned int i, struct mmu_directory *directories, struct mmu_table *tables)
{

    task_init(&task->base);

    task->directory = directories + i * TASKDIRECTORYCOUNT;
    task->table = tables + i * TASKTABLECOUNT;
    task->mapping[0] = TASKCODEBASE + TASKCODESIZE * i;
    task->mapping[1] = TASKSTACKBASE + TASKSTACKSIZE * i;

    scheduler_register_task(&task->base);

}

static struct task *setuptasks()
{

    unsigned int i;

    for (i = 0; i < TASKS; i++)
        setuptask(&tasks[i], i, (struct mmu_directory *)TASKDIRECTORYBASE, (struct mmu_table *)TASKTABLEBASE);

    return &tasks[0].base;

}

void arch_setup(struct vfs_backend *backend)
{

    gdt_initpointer(&gdt.pointer, GDTDESCRIPTORS, gdt.descriptors);
    idt_initpointer(&idt.pointer, IDTDESCRIPTORS, idt.descriptors);
    tss_initpointer(&tss.pointer, TSSDESCRIPTORS, tss.descriptors);

    selector.kcode = gdt_setdescriptor(&gdt.pointer, GDT_INDEX_KCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.kdata = gdt_setdescriptor(&gdt.pointer, GDT_INDEX_KDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.ucode = gdt_setdescriptor(&gdt.pointer, GDT_INDEX_UCODE, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW | GDT_ACCESS_EXECUTE, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.udata = gdt_setdescriptor(&gdt.pointer, GDT_INDEX_UDATA, 0x00000000, 0xFFFFFFFF, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_ALWAYS1 | GDT_ACCESS_RW, GDT_FLAG_GRANULARITY | GDT_FLAG_32BIT);
    selector.tlink = gdt_setdescriptor(&gdt.pointer, GDT_INDEX_TLINK, (unsigned long)tss.pointer.descriptors, (unsigned long)tss.pointer.descriptors + tss.pointer.limit, GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTE | GDT_ACCESS_ACCESSED, GDT_FLAG_32BIT);

    idt_setdescriptor(&idt.pointer, IDT_INDEX_GF, arch_isrgeneralfault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, IDT_INDEX_PF, arch_isrpagefault, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt.pointer, IDT_INDEX_SYSCALL, arch_isrsyscall, selector.kcode, IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT | IDT_FLAG_RING3);
    tss_setdescriptor(&tss.pointer, TSS_INDEX_DEFAULT, selector.kdata, KERNELLIMIT);
    cpu_setgdt(&gdt.pointer, selector.kcode, selector.kdata);
    cpu_setidt(&idt.pointer);
    cpu_settss(selector.tlink);
    kernel_setup(spawn, despawn);

    current.container = setupcontainers();
    current.task = setuptasks();

    kernel_setupramdisk(current.container, current.task, backend);
    kernel_copytask(current.task, current.task);
    kernel_setuptask(current.task, TASKVSTACKLIMIT);
    scheduler_use(current.task);
    containermaptext(current.container);
    taskmapcontainer(current.task, current.container);
    taskactivate(current.task);
    mmu_enable();
    arch_usermode(selector.ucode, selector.udata, current.task->state.registers.ip, current.task->state.registers.sp);

}

