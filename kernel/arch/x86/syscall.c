#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/runtime.h>
#include <kernel/arch/x86/arch.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/mmu.h>
#include <kernel/arch/x86/syscall.h>

static void *syscallRoutines[SYSCALL_ROUTINES_SIZE];

void syscall_register_handler(unsigned char index, void (*handler)(struct syscall_registers *registers))
{

    syscallRoutines[index] = handler;

}

void syscall_unregister_handler(unsigned char index)
{

    syscallRoutines[index] = 0;

}

void syscall_handler(struct syscall_registers *registers)
{

    void (*handler)(struct syscall_registers *registers) = syscallRoutines[registers->eax];

    if (handler)
        handler(registers);

}

static void syscall_open(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;

    struct runtime_task *task = runtime_get_running_task();    

    struct vfs_node *node = vfs_find(path);

    if (!node)
    {

        registers->eax = -1;

        return;

    }

    struct vfs_descriptor *descriptor = task->add_descriptor(task, node);

    if (!descriptor)
    {

        registers->eax = -1;

        return;

    }

    registers->eax = descriptor->index;

}

static void syscall_close(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;

    struct runtime_task *task = runtime_get_running_task();    

    task->remove_descriptor(task, fd);

}

static void syscall_read(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    char *buffer = (char *)registers->esi;
    unsigned int count = registers->ecx;

    struct runtime_task *task = runtime_get_running_task();    

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!(node && node->operations.read))
    {

        registers->eax = 0;

        return;

    }

    registers->eax = node->operations.read(node, count, buffer);

}

static void syscall_write(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    char *buffer = (char *)registers->esi;
    unsigned int count = registers->ecx;

    struct runtime_task *task = runtime_get_running_task();    

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!(node && node->operations.write))
    {

        registers->eax = 0;

        return;

    }

    registers->eax = node->operations.write(node, count, buffer);

}

static void syscall_info(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    struct file_info *info = (struct file_info *)registers->edi;
    
    struct runtime_task *task = runtime_get_running_task();

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!node)
    {

        registers->eax = 0;

        return;

    }

    info->id = node->id;
    info->length = 0;

    registers->eax = 1;

}

static void syscall_execute(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;
    unsigned int argc = registers->ecx;
    char **argv = (char **)registers->ebx;

    struct runtime_task *task = runtime_get_running_task();

    if (!task->load(task, path, argc, argv))
    {

        registers->eax = 0;

        return;

    }

    runtime_activate(task);

    registers->eip = (unsigned int)task->eip;
    registers->useresp = (unsigned int)task->esp;
    registers->eax = 0;

}

static void syscall_exit(struct syscall_registers *registers)
{

    struct runtime_task *task = runtime_get_running_task();

    registers->eax = 0;

}

static void syscall_reboot(struct syscall_registers *registers)
{

    arch_reboot();

}

void syscall_init()
{

    syscall_register_handler(SYSCALL_ROUTINE_OPEN, syscall_open);
    syscall_register_handler(SYSCALL_ROUTINE_CLOSE, syscall_close);
    syscall_register_handler(SYSCALL_ROUTINE_READ, syscall_read);
    syscall_register_handler(SYSCALL_ROUTINE_WRITE, syscall_write);
    syscall_register_handler(SYSCALL_ROUTINE_INFO, syscall_info);
    syscall_register_handler(SYSCALL_ROUTINE_EXECUTE, syscall_execute);
    syscall_register_handler(SYSCALL_ROUTINE_EXIT, syscall_exit);
    syscall_register_handler(SYSCALL_ROUTINE_REBOOT, syscall_reboot);

}

