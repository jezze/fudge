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

static unsigned int runtime_load(struct runtime_task *task, char *path, unsigned int argc, char **argv)
{

    struct mmu_header *header = mmu_get_program_header(task->pid);

    struct vfs_node *node = vfs_find(path);

    if (!(node && node->operations.read))
        return 0;

    node->operations.read(node, 0x10000, header->address);

    void *entry = elf_get_entry(header->address);

    if (!entry)
        return 0;

    void *virtual = elf_get_virtual(header->address);

    char **sa = header->address + 0xFC00;
    void *ss = header->address + 0xFD00;

    unsigned int i;
    unsigned int offset = 0;

    for (i = 0; i < argc; i++)
    {

        sa[i] = virtual + 0xFD00 + offset;

        unsigned int length = string_length(argv[i]);
        string_copy(ss + offset, argv[i]);

        offset += length + 2;

    }

    unsigned int argvn = (unsigned int)virtual + 0xFC00;
    unsigned int eip = 0;

    memory_set(header->address + 0xFFFF, ((unsigned int)argvn & 0xFF000000) >> 24, 1);
    memory_set(header->address + 0xFFFE, ((unsigned int)argvn & 0x00FF0000) >> 16, 1);
    memory_set(header->address + 0xFFFD, ((unsigned int)argvn & 0x0000FF00) >> 8, 1);
    memory_set(header->address + 0xFFFC, ((unsigned int)argvn & 0x000000FF) >> 0, 1);
    memory_set(header->address + 0xFFFB, (argc & 0xFF000000) >> 24, 1);
    memory_set(header->address + 0xFFFA, (argc & 0x00FF0000) >> 16, 1);
    memory_set(header->address + 0xFFF9, (argc & 0x0000FF00) >> 8, 1);
    memory_set(header->address + 0xFFF8, (argc & 0x000000FF) >> 0, 1);
    memory_set(header->address + 0xFFF7, (eip & 0xFF000000) >> 24, 1);
    memory_set(header->address + 0xFFF6, (eip & 0x00FF0000) >> 16, 1);
    memory_set(header->address + 0xFFF5, (eip & 0x0000FF00) >> 8, 1);
    memory_set(header->address + 0xFFF4, (eip & 0x000000FF) >> 0, 1);

    mmu_map(header, virtual, 0x10000, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);

    task->used = 1;
    task->eip = entry;
    task->esp = virtual + 0xFFF4;

    return 1;

}

static void unload(struct runtime_task *task)
{

    task->used = 0;

}

static struct vfs_descriptor *runtime_add_descriptor(struct runtime_task *task, struct vfs_node *node)
{

    unsigned int i;

    for (i = 0; i < 16; i++)
    {

        if (!task->descriptors[i].node)
        {

            task->descriptors[i].index = i;
            task->descriptors[i].node = node;
            task->descriptors[i].permissions = 0;

            return &task->descriptors[i];

        }

    }

    return 0;

}

static struct vfs_descriptor *runtime_get_descriptor(struct runtime_task *task, unsigned int index)
{

    return &task->descriptors[index];

}

static void runtime_remove_descriptor(struct runtime_task *task, unsigned int index)
{

    memory_set((void *)&task->descriptors[index], 0, sizeof (struct vfs_descriptor));

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
        runtimeTasks[i].add_descriptor = runtime_add_descriptor;
        runtimeTasks[i].get_descriptor = runtime_get_descriptor;
        runtimeTasks[i].remove_descriptor = runtime_remove_descriptor;

    }

}

