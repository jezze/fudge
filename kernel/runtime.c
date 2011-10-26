#include <lib/elf.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <kernel/arch/x86/mmu.h>
#include <modules/elf/elf.h>

struct runtime_task runtimeTasks[8];

struct runtime_task *runtime_get_task(unsigned int pid)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        if (runtimeTasks[i].pid == pid)
            return &runtimeTasks[i];

    }

    return 0;

}

struct runtime_task *runtime_get_running_task()
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        if (runtimeTasks[i].running)
            return &runtimeTasks[i];

    }

    return 0;

}

struct runtime_task *runtime_get_free_task()
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        if (!runtimeTasks[i].used)
            return &runtimeTasks[i];

    }

    return 0;

}

void runtime_activate(struct runtime_task *task)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
        runtimeTasks[i].running = 0;

    struct mmu_header *header = mmu_get_program_header(task->pid);

    mmu_set_directory(&header->directory);

    task->running = 1;

}

static void *runtime_copy_argv(void *address, void *virtual, unsigned int argc, char **argv)
{

    unsigned int start = 0xFD00;
    char **nargv = address + start;

    unsigned int i;
    unsigned int off = argc * 4;

    for (i = 0; i < argc; i++)
    {

        nargv[i] = virtual + start + off;
        string_copy(address + start + off, argv[i]);
        off += string_length(argv[i]) + 2;

    }

    return virtual + start;

}

static void *runtime_create_stack(struct runtime_task *self, void *address, void *virtual, unsigned int argc, char **argv)
{

    void *nargv = runtime_copy_argv(address, virtual, argc, argv);

    memory_set(address + 0xFFFF, ((unsigned int)nargv & 0xFF000000) >> 24, 1);
    memory_set(address + 0xFFFE, ((unsigned int)nargv & 0x00FF0000) >> 16, 1);
    memory_set(address + 0xFFFD, ((unsigned int)nargv & 0x0000FF00) >> 8, 1);
    memory_set(address + 0xFFFC, ((unsigned int)nargv & 0x000000FF) >> 0, 1);
    memory_set(address + 0xFFFB, (argc & 0xFF000000) >> 24, 1);
    memory_set(address + 0xFFFA, (argc & 0x00FF0000) >> 16, 1);
    memory_set(address + 0xFFF9, (argc & 0x0000FF00) >> 8, 1);
    memory_set(address + 0xFFF8, (argc & 0x000000FF) >> 0, 1);
    memory_set(address + 0xFFF7, ((unsigned int)self->eip & 0xFF000000) >> 24, 1);
    memory_set(address + 0xFFF6, ((unsigned int)self->eip & 0x00FF0000) >> 16, 1);
    memory_set(address + 0xFFF5, ((unsigned int)self->eip & 0x0000FF00) >> 8, 1);
    memory_set(address + 0xFFF4, ((unsigned int)self->eip & 0x000000FF) >> 0, 1);

    return virtual + 0xFFF4;

}

static unsigned int runtime_load(struct runtime_task *self, char *path, unsigned int argc, char **argv)
{

    struct mmu_header *header = mmu_get_program_header(self->pid);

    struct vfs_node *node = vfs_find(path);

    if (!(node && node->operations.read))
        return 0;

    node->operations.read(node, 0x10000, header->address);

    void *entry = elf_get_entry(header->address);

    if (!entry)
        return 0;

    void *virtual = elf_get_virtual(header->address);

    self->used = 1;
    self->eip = entry;
    self->esp = runtime_create_stack(self, header->address, virtual, argc, argv);

    mmu_map(header, virtual, 0x10000, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

    memory_set(self->descriptors, 0, sizeof (struct vfs_descriptor) * 16);

    struct vfs_node *sin = vfs_find("/stdin");
    struct vfs_node *sout = vfs_find("/stdout");
    struct vfs_node *serror = vfs_find("/stderr");

    self->add_descriptor(self, sin);
    self->add_descriptor(self, sout);
    self->add_descriptor(self, serror);

    return 1;

}

static void runtime_unload(struct runtime_task *self)
{

    self->used = 0;

}

static struct vfs_descriptor *runtime_add_descriptor(struct runtime_task *self, struct vfs_node *node)
{

    unsigned int i;

    for (i = 0; i < 16; i++)
    {

        if (!self->descriptors[i].node)
        {

            self->descriptors[i].index = i;
            self->descriptors[i].node = node;
            self->descriptors[i].permissions = 0;

            return &self->descriptors[i];

        }

    }

    return 0;

}

static struct vfs_descriptor *runtime_get_descriptor(struct runtime_task *self, unsigned int index)
{

    return &self->descriptors[index];

}

static void runtime_remove_descriptor(struct runtime_task *self, unsigned int index)
{

    memory_set((void *)&self->descriptors[index], 0, sizeof (struct vfs_descriptor));

}

void runtime_init()
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        runtimeTasks[i].pid = i;
        runtimeTasks[i].running = 0;
        runtimeTasks[i].used = 0;
        runtimeTasks[i].load = runtime_load;
        runtimeTasks[i].unload = runtime_unload;
        runtimeTasks[i].add_descriptor = runtime_add_descriptor;
        runtimeTasks[i].get_descriptor = runtime_get_descriptor;
        runtimeTasks[i].remove_descriptor = runtime_remove_descriptor;

    }

}

