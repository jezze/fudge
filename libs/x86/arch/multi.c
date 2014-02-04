#include <fudge/kernel.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/container.h>
#include <kernel/kernel.h>
#include "cpu.h"
#include "arch.h"
#include "mmu.h"
#include "multi.h"

#define KSPACE_BASE                     ARCH_BIOS_BASE
#define KSPACE_LIMIT                    ARCH_TABLE_USTACK_LIMIT
#define KSPACE_SIZE                     (KSPACE_LIMIT - KSPACE_BASE)
#define TASKS                           64
#define TASK_CODESIZE                   (ARCH_UCODE_SIZE / TASKS)
#define TASK_STACKSIZE                  (ARCH_USTACK_SIZE / TASKS)
#define TASK_STACKLIMIT                 0x80000000
#define TASK_STACKBASE                  (TASK_STACKLIMIT - TASK_STACKSIZE)

static struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_BASE;
static struct mmu_table *kcode = (struct mmu_table *)ARCH_TABLE_KCODE_BASE;
static struct mmu_table *ucode = (struct mmu_table *)ARCH_TABLE_UCODE_BASE;
static struct mmu_table *ustack = (struct mmu_table *)ARCH_TABLE_USTACK_BASE;
static struct list active;
static struct list blocked;
static struct list free;

static struct multi_task
{

    struct task base;
    struct list_item item;
    unsigned int index;
    struct cpu_general general;

} tasks[TASKS];

static struct multi_task *find_next_task()
{

    unsigned int i;

    for (i = TASKS - 1; i > 0; i--)
    {

        if (!(tasks[i].base.state & TASK_STATE_USED))
            continue;

        if (tasks[i].base.state & TASK_STATE_BLOCKED)
            continue;

        return &tasks[i];

    }

    return 0;

}

static struct multi_task *find_free_task(struct multi_task *task)
{

    unsigned int i;

    for (i = task->index; i < TASKS; i++)
    {

        if (tasks[i].base.state & TASK_STATE_USED)
            continue;

        return &tasks[i];

    }

    return 0;

}

static void map_kernel(struct multi_task *task)
{

    memory_clear(&directories[task->index], sizeof (struct mmu_directory));
    mmu_map(&directories[task->index], &kcode[0], KSPACE_BASE, KSPACE_BASE, KSPACE_SIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE);

}

static void map_user(struct multi_task *task, unsigned int address)
{

    mmu_map(&directories[task->index], &ucode[task->index], ARCH_UCODE_BASE + (task->index * TASK_CODESIZE), address, TASK_CODESIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);
    mmu_map(&directories[task->index], &ustack[task->index], ARCH_USTACK_BASE + (task->index * TASK_STACKSIZE), TASK_STACKBASE, TASK_STACKSIZE, MMU_TFLAG_PRESENT | MMU_TFLAG_WRITEABLE | MMU_TFLAG_USERMODE, MMU_PFLAG_PRESENT | MMU_PFLAG_WRITEABLE | MMU_PFLAG_USERMODE);

}

void multi_map(unsigned int address)
{

    struct multi_task *task = find_next_task();

    map_user(task, address);

}

struct task *multi_schedule(struct task *task, struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct multi_task *current = (struct multi_task *)task;
    struct multi_task *next = find_next_task();

    if (current)
    {

        if (current == next)
            return &current->base;

        current->base.registers.ip = interrupt->eip;
        current->base.registers.sp = interrupt->esp;

        memory_copy(&current->general, general, sizeof (struct cpu_general));

    }

    if (!next)
        return 0;

    interrupt->eip = next->base.registers.ip;
    interrupt->esp = next->base.registers.sp;

    memory_copy(general, &next->general, sizeof (struct cpu_general));
    mmu_load(&directories[next->index]);

    return &next->base;

}

static void init_task(struct multi_task *task, unsigned int index)
{

    memory_clear(task, sizeof (struct multi_task));
    task_init(&task->base, 0, 0, TASK_STACKLIMIT);
    list_init_item(&task->item, task);

    task->index = index;

}

static void activate_task(struct multi_task *task)
{

    task->base.state = TASK_STATE_USED;

    map_kernel(task);
    mmu_load(&directories[task->index]);

}

static unsigned int spawn(struct container *self, struct task *task, void *stack)
{

    struct parameters {void *caller; unsigned int index;} args;
    struct multi_task *current = (struct multi_task *)task;
    struct multi_task *next = find_free_task(current);

    if (!next)
        return 0;

    memory_copy(&args, stack, sizeof (struct parameters));
    memory_copy(next->base.descriptors, task->descriptors, sizeof (struct task_descriptor) * TASK_DESCRIPTORS);
    activate_task(next);

    return self->calls[CONTAINER_CALL_EXECUTE](self, &next->base, &args);

}

struct task *multi_setup(struct container *container)
{

    unsigned int i;

    container->calls[CONTAINER_CALL_SPAWN] = spawn;

    list_init(&active);
    list_init(&blocked);
    list_init(&free);

    for (i = 1; i < TASKS; i++)
    {

        init_task(&tasks[i], i);
        list_add(&free, &tasks[i].item);

    }

    activate_task(&tasks[1]);
    mmu_enable();

    return &tasks[1].base;

}

