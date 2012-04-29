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

void runtime_activate(struct runtime_task *task, struct runtime_task *ptask)
{

    if (ptask)
        task->parentid = ptask->id;

    mmu_load_memory(task->id);

    running = task;

}

static void copy_args(unsigned int argc, char **argv, void *buffer)
{

    char **nargv = (char **)(buffer);
    unsigned int offset = (unsigned int)nargv + argc * 4;

    unsigned int i;

    for (i = 0; i < argc; i++)
    {

        nargv[i] = (void *)offset;

        unsigned int count = string_length(argv[i]) + 1;

        memory_copy(nargv[i], argv[i], count);

        offset += count;

    }

}

static unsigned int load(struct runtime_task *self, void (*entry)(), unsigned int argc, char **argv)
{

    char temp[128];

    copy_args(argc, argv, temp);

    self->used = 1;

    struct runtime_memory *memory = &self->memory;

    mmu_map_user_memory(self->id, memory->paddress, memory->vaddress, memory->size);
    mmu_load_memory(self->id);

    unsigned int stack = memory->vaddress + memory->size;
    unsigned int address = stack - 0x200;

    copy_args(argc, (char **)(temp), (void *)address);

    stack = (unsigned int)memory_copy((void *)(stack - 0x4), &address, 4);
    stack = (unsigned int)memory_copy((void *)(stack - 0x4), &argc, 4);
    stack = (unsigned int)memory_copy((void *)(stack - 0x4), &entry, 4);

    runtime_registers_init(&self->registers, (unsigned int)entry, stack, stack);

    return 1;

}

static void unload(struct runtime_task *self)
{

    self->used = 0;

    mmu_unmap_memory(self->id);

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

