#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/mmu.h>
#include <kernel/runtime.h>
#include <kernel/vfs/proc.h>

static struct runtime_task tasks[RUNTIME_TASK_SLOTS];
static struct runtime_task *running;

unsigned int runtime_get_task_slot()
{

    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_SLOTS - 1; i++)
    {

        if (!tasks[i].used)
            return i;

    }

    return 0;

}

struct runtime_task *runtime_get_task(unsigned int index)
{

    if (!index)
        return 0;

    return &tasks[index];

}

struct runtime_task *runtime_get_running_task()
{

    return running;

}

void runtime_activate(struct runtime_task *task)
{

    mmu_load_memory(task->id);

    running = task;

}

static unsigned int load(struct runtime_task *self, void (*entry)())
{

    struct runtime_memory *memory = &self->memory;
    unsigned int stack = memory->vaddress + memory->size;

    self->used = 1;

    mmu_map_user_memory(self->id, memory->paddress, memory->vaddress, memory->size);

    runtime_registers_init(&self->registers, (unsigned int)entry, stack, stack);

    return 1;

}

static unsigned int unload(struct runtime_task *self)
{

    self->used = 0;

    return 1;

}

static unsigned int get_descriptor_slot(struct runtime_task *self)
{

    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_DESCRIPTOR_SLOTS - 1; i++)
    {

        if (!self->descriptors[i].id)
            return i;

    }

    return 0;

}

static struct runtime_descriptor *get_descriptor(struct runtime_task *self, unsigned int index)
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

void runtime_descriptor_init(struct runtime_descriptor *descriptor, unsigned int id, struct modules_filesystem *filesystem, unsigned int permissions)
{

    memory_clear(descriptor, sizeof (struct runtime_descriptor));

    descriptor->id = id;
    descriptor->filesystem = filesystem;
    descriptor->permissions = permissions;

}

void runtime_memory_init(struct runtime_memory *memory, unsigned int paddress, unsigned int vaddress, unsigned int size)
{

    memory_clear(memory, sizeof (struct runtime_memory));

    memory->paddress = paddress;
    memory->vaddress = vaddress;
    memory->size = size;

}

void runtime_task_init(struct runtime_task *task, unsigned int id)
{

    memory_clear(task, sizeof (struct runtime_task));

    task->id = id;
    task->load = load;
    task->unload = unload;
    task->get_descriptor_slot = get_descriptor_slot;
    task->get_descriptor = get_descriptor;

    unsigned int address = RUNTIME_TASK_ADDRESS_BASE + task->id * RUNTIME_TASK_ADDRESS_SIZE;

    runtime_memory_init(&task->memory, address, address, RUNTIME_TASK_ADDRESS_SIZE);

}

void runtime_task_clone(struct runtime_task *task, struct runtime_task *original, unsigned int id)
{

    memory_copy(task, original, sizeof (struct runtime_task));

    task->id = id;

    unsigned int address = RUNTIME_TASK_ADDRESS_BASE + task->id * RUNTIME_TASK_ADDRESS_SIZE;

    runtime_memory_init(&task->memory, address, address, RUNTIME_TASK_ADDRESS_SIZE);

}

void runtime_init()
{

    memory_clear(tasks, sizeof (struct runtime_task) * RUNTIME_TASK_SLOTS);

    running = 0;

    vfs_proc_init(tasks);

}

