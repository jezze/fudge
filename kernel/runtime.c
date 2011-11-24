#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/kernel.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>

struct runtime_task runtimeTasks[RUNTIME_TASK_SLOTS];

struct runtime_task *runtime_get_task(unsigned int id)
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_SLOTS; i++)
    {

        if (runtimeTasks[i].id == id)
            return &runtimeTasks[i];

    }

    return 0;

}

struct runtime_task *runtime_get_running_task()
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_SLOTS; i++)
    {

        if (runtimeTasks[i].running)
            return &runtimeTasks[i];

    }

    return 0;

}

struct runtime_task *runtime_get_slot()
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_SLOTS; i++)
    {

        if (!runtimeTasks[i].used)
            return &runtimeTasks[i];

    }

    return 0;

}

void runtime_activate(struct runtime_task *task)
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_SLOTS; i++)
        runtimeTasks[i].running = 0;

    void *paddress = kernel_get_task_memory(task->id);

    kernel_load_task_memory(paddress);

    task->running = 1;

}

static void *runtime_task_load_argv(void *paddress, void *vaddress, unsigned int argc, char **argv)
{

    unsigned int start = 0xFD00;
    char **nargv = paddress + start;

    unsigned int i;
    unsigned int off = argc * 4;

    for (i = 0; i < argc; i++)
    {

        nargv[i] = vaddress + start + off;
        string_write(paddress + start + off, argv[i]);
        off += string_length(argv[i]) + 2;

    }

    return vaddress + start;

}

static void *runtime_task_load_stack(void *pstack, void *vstack, void *ip, unsigned int argc, char **argv)
{

    memory_copy(pstack - 0x4, &argv, 4);
    memory_copy(pstack - 0x8, &argc, 4);
    memory_copy(pstack - 0xC, &ip, 4);

    return vstack - 0xC;

}

static unsigned int runtime_task_load(struct runtime_task *self, void *paddress, void *vaddress, unsigned int limit, void *entry, unsigned int argc, char **argv)
{

    self->used = 1;
    self->registers.ip = entry;

    void *vargv = runtime_task_load_argv(paddress, vaddress, argc, argv);
    self->registers.sp = runtime_task_load_stack(paddress + limit, vaddress + limit, self->registers.ip, argc, vargv);

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_DESCRIPTOR_SLOTS; i++)
        self->descriptors[i].node = 0;

    return 1;

}

static void runtime_task_unload(struct runtime_task *self)
{

    self->used = 0;

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
    task->running = 0;
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

void runtime_init()
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_SLOTS; i++)
        runtime_task_init(&runtimeTasks[i], i + 1);

}

