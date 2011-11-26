#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/mmu.h>
#include <kernel/vfs.h>
#include <kernel/runtime.h>

static struct runtime_control runtimeControl;

struct runtime_task *runtime_get_slot()
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_SLOTS; i++)
    {

        if (!runtimeControl.tasks[i].used)
            return &runtimeControl.tasks[i];

    }

    return 0;

}

struct runtime_task *runtime_get_task(unsigned int id)
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_SLOTS; i++)
    {

        if (runtimeControl.tasks[i].id == id)
            return &runtimeControl.tasks[i];

    }

    return 0;

}

struct runtime_task *runtime_get_running_task()
{

    return runtimeControl.running;

}

void runtime_activate(struct runtime_task *task)
{

    void *paddress = mmu_get_task_memory(task->id);

    mmu_load_task_memory(paddress);

    runtimeControl.running = task;

}

static unsigned int runtime_task_load(struct runtime_task *self, void *paddress, void *vaddress, unsigned int limit, void *entry, unsigned int argc, char **argv)
{

    int reloc = (int)vaddress - (int)paddress;

    char **pargv = (char **)(paddress + limit - 0x200);
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

    void *stack = paddress + limit;

    stack = memory_copy(stack - 0x4, &vargv, 4);
    stack = memory_copy(stack - 0x4, &vargc, 4);
    stack = memory_copy(stack - 0x4, &entry, 4);

    self->used = 1;
    self->registers.ip = entry;
    self->registers.sp = stack + reloc;
    self->registers.sb = stack + reloc;

    return 1;

}

static void runtime_task_unload(struct runtime_task *self)
{

    self->used = 0;

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_DESCRIPTOR_SLOTS; i++)
        self->descriptors[i].node = 0;

}

static void runtime_task_save_registers(struct runtime_task *self, void *ip, void *sp, void *sb)
{

    self->registers.ip = ip;
    self->registers.sp = sp;
    self->registers.sb = sb;

}

static struct runtime_descriptor *runtime_task_add_descriptor(struct runtime_task *self, struct vfs_node *node)
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_DESCRIPTOR_SLOTS; i++)
    {

        if (!self->descriptors[i].node)
        {

            self->descriptors[i].node = node;

            return &self->descriptors[i];

        }

    }

    return 0;

}

static struct runtime_descriptor *runtime_task_get_descriptor(struct runtime_task *self, unsigned int id)
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_DESCRIPTOR_SLOTS; i++)
    {

        if (self->descriptors[i].id == id && self->descriptors[i].node)
            return &self->descriptors[i];

    }

    return 0;

}

static void runtime_task_remove_descriptor(struct runtime_task *self, unsigned int id)
{

    struct runtime_descriptor *descriptor = runtime_task_get_descriptor(self, id);

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
    task->save_registers = runtime_task_save_registers;
    task->add_descriptor = runtime_task_add_descriptor;
    task->get_descriptor = runtime_task_get_descriptor;
    task->remove_descriptor = runtime_task_remove_descriptor;

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_DESCRIPTOR_SLOTS; i++)
        runtime_descriptor_init(&task->descriptors[i], i + 1, 0, 0);

}

void runtime_control_init(struct runtime_control *control)
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_SLOTS; i++)
        runtime_task_init(&control->tasks[i], i + 1);

    control->running = 0;

}

void runtime_init()
{

    runtime_control_init(&runtimeControl);

}

