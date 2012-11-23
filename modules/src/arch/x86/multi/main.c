#include <memory.h>
#include <vfs.h>
#include <binary.h>
#include <runtime.h>
#include <syscall.h>
#include <arch/x86/arch.h>
#include <arch/x86/isr.h>
#include <arch/x86/mmu.h>
#include <arch/x86/multi/multi.h>

static struct runtime_task tasks[MULTI_TASK_SLOTS];

static struct runtime_task *get_task(unsigned int index)
{

    if (!index || index >= MULTI_TASK_SLOTS)
        return 0;

    return &tasks[index];

}

static unsigned int get_task_slot()
{

    unsigned int i;

    for (i = 1; i < MULTI_TASK_SLOTS; i++)
    {

        if (!tasks[i].status.used)
            return i;

    }

    return 0;

}

static void schedule()
{

    unsigned int i;
    struct mmu_directory *directory;

    for (i = MULTI_TASK_SLOTS - 1; i > 0; i--)
    {

        if (!tasks[i].status.used)
            continue;

        if (tasks[i].status.idle)
            continue;

        isr_set_task(&tasks[i]);

        directory = (struct mmu_directory *)(MMU_ADDRESS_DIRECTORIES + MMU_PAGE_SIZE * i);

        mmu_load_memory(directory);

        break;

    }

}

static void notify_interrupt(struct runtime_task *task, unsigned int index)
{

    unsigned int i;

    for (i = 1; i < MULTI_TASK_SLOTS - 1; i++)
    {

        if (!tasks[i].status.used)
            continue;

        if (!tasks[i].status.idle)
            continue;

        if (!tasks[i].events[index].callback)
            continue;

        tasks[i].status.idle = 0;
        runtime_init_registers(&tasks[i].registers, tasks[i].events[index].callback, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, tasks[i].registers.status);

    }

}

static void notify_complete(struct runtime_task *task)
{

    schedule();

}

static unsigned int spawn(struct runtime_task *task, void *stack)
{

    struct multi_spawn_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    unsigned int index = get_task_slot();
    struct runtime_task *ntask = get_task(index);
    struct mmu_directory *directory = (struct mmu_directory *)(MMU_ADDRESS_DIRECTORIES + MMU_PAGE_SIZE * index);
    struct mmu_table *ktable = (struct mmu_table *)(MMU_ADDRESS_KTABLES + MMU_PAGE_SIZE * index);
    struct mmu_table *utable = (struct mmu_table *)(MMU_ADDRESS_UTABLES + MMU_PAGE_SIZE * index);
    unsigned int entry;

    if (!descriptor || !descriptor->interface->read)
        return 0;

    if (!ntask)
        return 0;

    memory_clear(directory, sizeof (struct mmu_directory));
    memory_clear(ktable, sizeof (struct mmu_table));
    memory_clear(utable, sizeof (struct mmu_table));

    mmu_map_memory(directory, ktable, ARCH_KERNEL_BASE, ARCH_KERNEL_BASE, ARCH_KERNEL_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE);
    mmu_map_memory(directory, utable, RUNTIME_TASK_PADDRESS_BASE + index * RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE, RUNTIME_TASK_ADDRESS_SIZE, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);
    mmu_load_memory(directory);

    entry = binary_copy_program(descriptor->interface, descriptor->id);

    if (!entry)
        return 0;

    memory_copy(ntask, task, sizeof (struct runtime_task));

    runtime_init_registers(&ntask->registers, entry, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, index);

    ntask->notify_interrupt = notify_interrupt;
    ntask->notify_complete = notify_complete;

    schedule();

    return index;

}

void init()
{

    memory_clear(tasks, sizeof (struct runtime_task) * MULTI_TASK_SLOTS);

    syscall_set_routine(SYSCALL_INDEX_SPAWN, spawn);

}

void destroy()
{

}

