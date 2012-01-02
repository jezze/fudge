#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/mmu.h>
#include <kernel/vfs.h>
#include <kernel/runtime.h>

static struct runtime_task tasks[RUNTIME_TASK_SLOTS];
static struct runtime_task *running;

unsigned int runtime_get_slot()
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

    return running;

}

void runtime_activate(struct runtime_task *task)
{

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

void runtime_set_state(unsigned int ip, unsigned int sp, unsigned int sb)
{

    struct runtime_task *task = runtime_get_running_task();

    if (!task)
        return;

    task->registers.ip = ip;
    task->registers.sp = sp;
    task->registers.sb = sb;

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
    self->registers.ip = (unsigned int)entry;
    self->registers.sp = (unsigned int)(stack + reloc);
    self->registers.sb = (unsigned int)(stack + reloc);

    return 1;

}

static void runtime_task_unload(struct runtime_task *self)
{

    self->used = 0;

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_DESCRIPTOR_SLOTS; i++)
        self->descriptors[i].node = 0;

}

static struct runtime_descriptor *runtime_task_get_descriptor(struct runtime_task *self, unsigned int index)
{

    if (!index)
        return 0;

    return &self->descriptors[index];

}

static struct runtime_descriptor *runtime_task_add_descriptor(struct runtime_task *self, struct vfs_node *node)
{

    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_DESCRIPTOR_SLOTS; i++)
    {

        if (!self->descriptors[i].node)
        {

            self->descriptors[i].node = node;

            return &self->descriptors[i];

        }

    }

    return 0;

}

static void runtime_task_remove_descriptor(struct runtime_task *self, unsigned int index)
{

    struct runtime_descriptor *descriptor = runtime_task_get_descriptor(self, index);
    descriptor->node = 0;

}

void runtime_descriptor_init(struct runtime_descriptor *descriptor, unsigned int id, struct vfs_node *node, unsigned int permissions)
{

    descriptor->id = id;
    descriptor->node = node;
    descriptor->permissions = permissions;

}

void runtime_task_init(struct runtime_task *task, unsigned int id)
{

    task->id = id;
    task->used = 0;
    task->load = runtime_task_load;
    task->unload = runtime_task_unload;
    task->add_descriptor = runtime_task_add_descriptor;
    task->get_descriptor = runtime_task_get_descriptor;
    task->remove_descriptor = runtime_task_remove_descriptor;

    mmu_memory_init(&task->memory, (void *)(0x00300000 + task->id * 0x10000), (void *)0x00000000, 0x10000);

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_DESCRIPTOR_SLOTS; i++)
        runtime_descriptor_init(&task->descriptors[i], i, 0, 0);

}

void runtime_init()
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_SLOTS; i++)
        runtime_task_init(&tasks[i], i);

    running = 0;

}

