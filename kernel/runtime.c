#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/mmu.h>
#include <kernel/vfs.h>
#include <kernel/runtime.h>

static struct runtime_task tasks[RUNTIME_TASK_SLOTS];
static struct runtime_task *running;

unsigned int runtime_get_task_slot()
{

    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_SLOTS; i++)
    {

        if (!tasks[i].used)
            return i;

    }

    return 0;

}

struct runtime_task *runtime_get_task(unsigned int index)
{

    return &tasks[index];

}

struct runtime_task *runtime_get_running_task()
{

    if (!running->used)
        return 0;

    return running;

}

void runtime_activate(struct runtime_task *task, struct runtime_task *ptask)
{

    if (ptask)
        task->parentid = ptask->id;

    mmu_load_memory(&task->memory);

    running = task;

}

static void runtime_copy_args(unsigned int argc, char **argv, void *buffer)
{

    char **nargv = (char **)(buffer);
    void *offset = nargv + argc * 4;

    unsigned int i;

    for (i = 0; i < argc; i++)
    {

        nargv[i] = offset;
        string_write(offset, argv[i]);
        offset += string_length(argv[i]) + 1;

    }

}

static unsigned int runtime_task_load(struct runtime_task *self, void *entry, unsigned int argc, char **argv)
{

    char temp[128];

    runtime_copy_args(argc, argv, temp);

    self->used = 1;

    struct mmu_memory *memory = &self->memory;

    mmu_map_user_memory(self->id, memory);
    mmu_load_memory(memory);

    void *stack = memory->vaddress + memory->size;
    void *address = stack - 0x200;

    runtime_copy_args(argc, (char **)(temp), address);

    stack = memory_copy(stack - 0x4, &address, 4);
    stack = memory_copy(stack - 0x4, &argc, 4);
    stack = memory_copy(stack - 0x4, &entry, 4);

    runtime_registers_init(&self->registers, (unsigned int)entry, (unsigned int)stack, (unsigned int)stack);

    return 1;

}

static void runtime_task_unload(struct runtime_task *self)
{

    self->used = 0;

    struct mmu_memory *memory = &self->memory;

    mmu_unmap_memory(memory);

}

static unsigned int runtime_task_get_descriptor_slot(struct runtime_task *self)
{

    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_DESCRIPTOR_SLOTS; i++)
    {

        if (!self->descriptors[i].node)
            return i;

    }

    return 0;

}

static struct runtime_descriptor *runtime_task_get_descriptor(struct runtime_task *self, unsigned int index)
{

    if (!index)
        return 0;

    return &self->descriptors[index];

}

void runtime_registers_init(struct runtime_registers *registers, unsigned int ip, unsigned int sp, unsigned int sb)
{

    memory_clear(registers, sizeof (struct runtime_registers));

    registers->ip = ip;
    registers->sp = sp;
    registers->sb = sb;

}

void runtime_descriptor_init(struct runtime_descriptor *descriptor, struct vfs_node *node, unsigned int permissions)
{

    memory_clear(descriptor, sizeof (struct runtime_descriptor));

    descriptor->node = node;
    descriptor->permissions = permissions;

}

void runtime_task_init(struct runtime_task *task, unsigned int id)
{

    memory_clear(task, sizeof (struct runtime_task));

    task->id = id;
    task->used = 0;
    task->event = 0;
    task->load = runtime_task_load;
    task->unload = runtime_task_unload;
    task->get_descriptor_slot = runtime_task_get_descriptor_slot;
    task->get_descriptor = runtime_task_get_descriptor;

    void *address = (void *)(RUNTIME_TASK_ADDRESS_BASE + task->id * RUNTIME_TASK_ADDRESS_SIZE);

    mmu_memory_init(&task->memory, address, address, RUNTIME_TASK_ADDRESS_SIZE);

}

void runtime_init()
{

    running = 0;

}

