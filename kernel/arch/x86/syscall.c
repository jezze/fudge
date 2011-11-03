#include <lib/file.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>
#include <kernel/arch/x86/arch.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/syscall.h>

static void *syscallRoutines[SYSCALL_ROUTINES_SIZE];

static void syscall_run_event(struct syscall_registers *registers, unsigned int index, struct runtime_task *task)
{

    task->save(task, (void *)registers->eip, (void *)registers->useresp, (void *)registers->ebp);

    struct event_event *event = event_get(index);

    if (!event->pid)
        return;

    struct runtime_task *oldtask = runtime_get_task(event->pid);

    runtime_activate(oldtask);

    registers->eip = (unsigned int)event->handler;
    registers->useresp = (unsigned int)oldtask->esp;
    registers->ebp = (unsigned int)oldtask->ebp;

    oldtask->parentpid = task->pid;

}

static void syscall_attach_handler(struct syscall_registers *registers)
{

    unsigned int index = registers->ebx;
    void (*handler)() = (void *)registers->ecx;

    syscall_attach(index, handler);

    registers->eax = 1;

}

static void syscall_close_handler(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;

    syscall_close(fd);

    registers->eax = 1;

}

static void syscall_detach_handler(struct syscall_registers *registers)
{

    unsigned int index = registers->ebx;

    syscall_detach(index);

    registers->eax = 1;

}

static void syscall_execute_handler(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;
    unsigned int argc = registers->ecx;
    char **argv = (char **)registers->ebx;

    struct runtime_task *oldtask = runtime_get_running_task();
    oldtask->save(oldtask, (void *)registers->eip, (void *)registers->useresp, (void *)registers->ebp);

    struct runtime_task *task = runtime_get_free_task();
    task->parentpid = oldtask->pid;

    if (!task->load(task, path, argc, argv))
    {

        registers->eax = 0;

        return;

    }

    runtime_activate(task);

    registers->eip = (unsigned int)task->eip;
    registers->useresp = (unsigned int)task->esp;
    registers->ebp = (unsigned int)task->ebp;

    registers->eax = 1;

    syscall_run_event(registers, 0x03, task);

}

static void syscall_exit_handler(struct syscall_registers *registers)
{

    struct runtime_task *oldtask = runtime_get_running_task();
    oldtask->unload(oldtask);

    struct runtime_task *task = runtime_get_task(oldtask->parentpid);

    runtime_activate(task);

    registers->eip = (unsigned int)task->eip;
    registers->useresp = (unsigned int)task->esp;
    registers->ebp = (unsigned int)task->ebp;

    registers->eax = 1;

}

static void syscall_info_handler(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    struct file_info *info = (struct file_info *)registers->edi;

    registers->eax = syscall_info(fd, info);

}

static void syscall_load_handler(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;

    syscall_load(path);

    registers->eax = 1;

}

static void syscall_open_handler(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;

    registers->eax = syscall_open(path);

}

static void syscall_read_handler(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    unsigned int count = registers->ecx;
    char *buffer = (char *)registers->esi;

    registers->eax = syscall_read(fd, count, buffer);

}

static void syscall_reboot_handler(struct syscall_registers *registers)
{

    syscall_reboot();

    registers->eax = 1;

}

static void syscall_unload_handler(struct syscall_registers *registers)
{

    syscall_unload();

    registers->eax = 1;

}

static void syscall_wait_handler(struct syscall_registers *registers)
{

    struct runtime_task *oldtask = runtime_get_running_task();
    oldtask->save(oldtask, (void *)registers->eip, (void *)registers->useresp, (void *)registers->ebp);

    struct runtime_task *task = runtime_get_task(oldtask->parentpid);

    runtime_activate(task);

    registers->eip = (unsigned int)task->eip;
    registers->useresp = (unsigned int)task->esp;
    registers->ebp = (unsigned int)task->ebp;

    registers->eax = 1;

}

static void syscall_write_handler(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    unsigned int count = registers->ecx;
    char *buffer = (char *)registers->esi;

    registers->eax = syscall_write(fd, count, buffer);

}

static void syscall_register_handler(unsigned char index, void (*handler)(struct syscall_registers *registers))
{

    syscallRoutines[index] = handler;

}

void syscall_handler(struct syscall_registers *registers)
{

    void (*handler)(struct syscall_registers *registers) = syscallRoutines[registers->eax];

    if (handler)
        handler(registers);

}

void syscall_init()
{

    syscall_register_handler(SYSCALL_ROUTINE_OPEN, syscall_open_handler);
    syscall_register_handler(SYSCALL_ROUTINE_CLOSE, syscall_close_handler);
    syscall_register_handler(SYSCALL_ROUTINE_READ, syscall_read_handler);
    syscall_register_handler(SYSCALL_ROUTINE_WRITE, syscall_write_handler);
    syscall_register_handler(SYSCALL_ROUTINE_INFO, syscall_info_handler);
    syscall_register_handler(SYSCALL_ROUTINE_EXECUTE, syscall_execute_handler);
    syscall_register_handler(SYSCALL_ROUTINE_EXIT, syscall_exit_handler);
    syscall_register_handler(SYSCALL_ROUTINE_WAIT, syscall_wait_handler);
    syscall_register_handler(SYSCALL_ROUTINE_LOAD, syscall_load_handler);
    syscall_register_handler(SYSCALL_ROUTINE_UNLOAD, syscall_unload_handler);
    syscall_register_handler(SYSCALL_ROUTINE_REBOOT, syscall_reboot_handler);
    syscall_register_handler(SYSCALL_ROUTINE_ATTACH, syscall_attach_handler);
    syscall_register_handler(SYSCALL_ROUTINE_DETACH, syscall_detach_handler);

}

