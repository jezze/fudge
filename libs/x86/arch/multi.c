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

struct mmu_directory *directories = (struct mmu_directory *)ARCH_DIRECTORY_BASE;
struct mmu_table *kcode = (struct mmu_table *)ARCH_TABLE_KCODE_BASE;
struct mmu_table *ucode = (struct mmu_table *)ARCH_TABLE_UCODE_BASE;
struct mmu_table *ustack = (struct mmu_table *)ARCH_TABLE_USTACK_BASE;

static struct multi_task
{

    struct task base;
    unsigned int index;
    struct {unsigned int ebx; unsigned int esi; unsigned int edi;} registers;

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

static struct multi_task *find_free_task()
{

    unsigned int i;

    for (i = 1; i < TASKS; i++)
    {

        if (tasks[i].base.state & TASK_STATE_USED)
            continue;

        tasks[i].index = i;

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

void multi_map(struct container *self, unsigned int address)
{

    struct multi_task *task = find_next_task();

    if (!task)
        return;

    map_user(task, address);

}

struct task *multi_schedule(struct container *self, struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct multi_task *current = (struct multi_task *)self->current;
    struct multi_task *task = find_next_task();

    if (!task)
        return &current->base;

    if (current == task)
        return &current->base;

    current->base.registers.ip = interrupt->eip;
    current->base.registers.sp = interrupt->esp;
    current->base.registers.fp = general->ebp;
    current->base.status = general->eax;
    current->registers.ebx = general->ebx;
    current->registers.esi = general->esi;
    current->registers.edi = general->edi;

    mmu_load(&directories[task->index]);

    interrupt->eip = task->base.registers.ip;
    interrupt->esp = task->base.registers.sp;
    general->ebp = task->base.registers.fp;
    general->eax = task->base.status;
    general->ebx = task->registers.ebx;
    general->esi = task->registers.esi;
    general->edi = task->registers.edi;

    return &task->base;

}

static unsigned int spawn(struct container *self, struct task *task, void *stack)
{

    struct parameters {void *caller; unsigned int index;} temp, *args = stack;
    struct multi_task *parent = (struct multi_task *)task;
    struct multi_task *child = find_free_task();

    if (!child)
        return 0;

    memory_copy(&temp, args, sizeof (struct parameters));

    task_init(&child->base, 0, TASK_STACKLIMIT, TASK_STACKLIMIT);
    map_kernel(child);
    mmu_load(&directories[child->index]);

    memory_copy(child->base.descriptors, parent->base.descriptors, sizeof (struct task_descriptor) * TASK_DESCRIPTORS);

    child->base.status = self->calls[CONTAINER_CALL_EXECUTE](self, &child->base, &temp);

    return 0;

}

struct task *multi_setup(struct container *container)
{

    struct multi_task *task = find_free_task();

    container->calls[CONTAINER_CALL_SPAWN] = spawn;

    task_init(&task->base, 0, TASK_STACKLIMIT, TASK_STACKLIMIT);
    map_kernel(task);
    mmu_load(&directories[task->index]);
    mmu_enable();

    return &task->base;

}

