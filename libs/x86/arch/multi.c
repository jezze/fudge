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

static struct multi_task
{

    struct task base;
    unsigned int index;
    struct cpu_general registers;

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

static unsigned int find_free_task()
{

    unsigned int i;

    for (i = 1; i < TASKS; i++)
    {

        if (tasks[i].base.state & TASK_STATE_USED)
            continue;

        return i;

    }

    return 0;

}

static void init_task(struct multi_task *task, unsigned int index)
{

    memory_clear(task, sizeof (struct multi_task));
    task_init(&task->base, 0, TASK_STACKLIMIT, TASK_STACKLIMIT);

    task->index = index;

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

struct task *multi_schedule(struct task *running, struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct multi_task *current = (struct multi_task *)running;
    struct multi_task *task = find_next_task();

    if (current == task)
        return running;

    current->base.registers.ip = interrupt->eip;
    current->base.registers.sp = interrupt->esp;

    /* For safety - REMOVE LATER */
    memory_copy(&current->registers, general, sizeof (struct cpu_general));

    mmu_load(&directories[task->index]);

    /* For safety - REMOVE LATER */
    memory_copy(general, &task->registers, sizeof (struct cpu_general));

    interrupt->eip = task->base.registers.ip;
    interrupt->esp = task->base.registers.sp;

    return &task->base;

}

static unsigned int spawn(struct container *self, struct task *task, void *stack)
{

    struct parameters {void *caller; unsigned int index;} temp, *args = stack;
    unsigned int c = find_free_task();

    if (!c)
        return 0;

    memory_copy(&temp, args, sizeof (struct parameters));

    init_task(&tasks[c], c);
    map_kernel(&tasks[c]);
    mmu_load(&directories[c]);

    memory_copy(tasks[c].base.descriptors, task->descriptors, sizeof (struct task_descriptor) * TASK_DESCRIPTORS);

    return self->calls[CONTAINER_CALL_EXECUTE](self, &tasks[c].base, &temp);

}

struct task *multi_setup(struct container *container)
{

    struct multi_task *task = &tasks[1];

    container->calls[CONTAINER_CALL_SPAWN] = spawn;

    init_task(task, 1);
    map_kernel(task);
    mmu_load(&directories[1]);
    mmu_enable();

    return &task->base;

}

