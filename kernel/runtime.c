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

    mmu_load_memory(&task->memory);

    running = task;

}

void runtime_get_state(unsigned int *ip, unsigned int *sp, unsigned int *sb)
{

    struct runtime_task *task = runtime_get_running_task();

    if (!task)
        return;

    *ip = task->registers.ip;
    *sp = task->registers.sp;
    *sb = task->registers.sb;

}

static unsigned int runtime_task_load(struct runtime_task *self, void *entry, unsigned int argc, char **argv)
{

    struct mmu_memory *memory = &self->memory;

    int reloc = (int)memory->vaddress - (int)memory->paddress;

    char **pargv = (char **)(memory->paddress + memory->size - 0x200);
    void *offset = pargv + argc * 4;

    unsigned int i;

    for (i = 0; i < argc; i++)
    {

        pargv[i] = offset + reloc;
        string_write(offset, argv[i]);
        offset += string_length(argv[i]) + 2;

    }

    unsigned int vargc = argc;
    unsigned int vargv = (unsigned int)pargv + reloc;

    void *stack = memory->paddress + memory->size;

    stack = memory_copy(stack - 0x4, &vargv, 4);
    stack = memory_copy(stack - 0x4, &vargc, 4);
    stack = memory_copy(stack - 0x4, &entry, 4);

    self->used = 1;

    runtime_registers_init(&self->registers, (unsigned int)entry, (unsigned int)(stack + reloc), (unsigned int)(stack + reloc));

    return 1;

}

static void runtime_task_unload(struct runtime_task *self)
{

    self->used = 0;

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

