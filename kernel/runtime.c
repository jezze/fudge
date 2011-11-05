#include <lib/elf.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/kernel.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <modules/elf/elf.h>

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

struct runtime_task *runtime_get_free_task()
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

static void *runtime_copy_argv(void *paddress, void *vaddress, unsigned int argc, char **argv)
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

static void runtime_task_create_stack(struct runtime_task *self, void *paddress, void *vaddress, unsigned int argc, char **argv)
{

    void *nargv = runtime_copy_argv(paddress, vaddress, argc, argv);

    memory_set(paddress + 0xFFFF, ((unsigned int)nargv & 0xFF000000) >> 24, 1);
    memory_set(paddress + 0xFFFE, ((unsigned int)nargv & 0x00FF0000) >> 16, 1);
    memory_set(paddress + 0xFFFD, ((unsigned int)nargv & 0x0000FF00) >> 8, 1);
    memory_set(paddress + 0xFFFC, ((unsigned int)nargv & 0x000000FF) >> 0, 1);
    memory_set(paddress + 0xFFFB, (argc & 0xFF000000) >> 24, 1);
    memory_set(paddress + 0xFFFA, (argc & 0x00FF0000) >> 16, 1);
    memory_set(paddress + 0xFFF9, (argc & 0x0000FF00) >> 8, 1);
    memory_set(paddress + 0xFFF8, (argc & 0x000000FF) >> 0, 1);
    memory_set(paddress + 0xFFF7, ((unsigned int)self->registers.ip & 0xFF000000) >> 24, 1);
    memory_set(paddress + 0xFFF6, ((unsigned int)self->registers.ip & 0x00FF0000) >> 16, 1);
    memory_set(paddress + 0xFFF5, ((unsigned int)self->registers.ip & 0x0000FF00) >> 8, 1);
    memory_set(paddress + 0xFFF4, ((unsigned int)self->registers.ip & 0x000000FF) >> 0, 1);

    self->registers.sp = vaddress + 0xFFF4;

}

static unsigned int runtime_task_load(struct runtime_task *self, char *path, unsigned int argc, char **argv)
{

    void *paddress = kernel_get_task_memory(self->id);

    struct vfs_node *node = vfs_find(path);

    if (!(node && node->read))
        return 0;

    node->read(node, 0x10000, paddress);

    void *vaddress = elf_get_virtual(paddress);

    if (!vaddress)
        return 0;

    void *entry = elf_get_entry(paddress);

    if (!entry)
        return 0;

    self->used = 1;
    self->registers.ip = entry;
    self->create_stack(self, paddress, vaddress, argc, argv);

    kernel_map_task_memory(paddress, vaddress, 0x10000, 0x7, 0x7);

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_DESCRIPTOR_SLOTS; i++)
        self->descriptors[i].node = 0;

    struct vfs_node *sin = vfs_find("/stdin");
    struct vfs_node *sout = vfs_find("/stdout");
    struct vfs_node *serror = vfs_find("/stderr");

    self->add_descriptor(self, sin);
    self->add_descriptor(self, sout);
    self->add_descriptor(self, serror);

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

static struct vfs_descriptor *runtime_task_add_descriptor(struct runtime_task *self, struct vfs_node *node)
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

static struct vfs_descriptor *runtime_task_get_descriptor(struct runtime_task *self, unsigned int id)
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

    struct vfs_descriptor *descriptor = runtime_task_get_descriptor(self, id);

    descriptor->node = 0;

}

void runtime_task_init(struct runtime_task *task, unsigned int id)
{

    task->id = id;
    task->running = 0;
    task->used = 0;
    task->create_stack = runtime_task_create_stack;
    task->load = runtime_task_load;
    task->unload = runtime_task_unload;
    task->save_registers = runtime_task_save_registers;
    task->add_descriptor = runtime_task_add_descriptor;
    task->get_descriptor = runtime_task_get_descriptor;
    task->remove_descriptor = runtime_task_remove_descriptor;

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_DESCRIPTOR_SLOTS; i++)
        vfs_descriptor_init(&task->descriptors[i], i + 1, 0, 0);

}

void runtime_relocate(void *paddress)
{

    struct elf_header *header = elf_get_header(paddress);

//    struct elf_section_header *shstrHeader = elf_get_section_header_by_index(node->physical + header->shoffset, header->shsize, header->shstringindex);
//    char *strtbl = (char *)(paddress + shstrHeader->offset);

//    elf_print_sections(paddress + header->shoffset, header->shsize, header->shcount, strtbl);

//    struct elf_section_header *symHeader = elf_get_section_header_by_type(paddress + header->shoffset, header->shsize, header->shcount, ELF_SECTION_TYPE_SYMTAB, 0);

//    struct elf_section_header *strHeader = elf_get_section_header_by_type(paddress + header->shoffset, header->shsize, header->shcount, ELF_SECTION_TYPE_STRTAB, 0x2);
//    struct elf_section_header *strHeader = elf_get_section_header_by_index(paddress + header->shoffset, header->shsize, 11);
//    char *strtbl2 = (char *)(paddress + strHeader->offset);

//    elf_print_symtab(paddress, symHeader, strtbl2);

    elf_relocate(paddress, header);

}

void runtime_init()
{

    unsigned int i;

    for (i = 0; i < RUNTIME_TASK_SLOTS; i++)
        runtime_task_init(&runtimeTasks[i], i + 1);

}

