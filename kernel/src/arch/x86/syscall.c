#include <kernel.h>
#include <runtime.h>
#include <syscall.h>
#include <arch/x86/isr.h>
#include <arch/x86/syscall.h>

static unsigned int (*routines[SYSCALL_TABLE_SLOTS])(struct runtime_task *task);

static unsigned int handle_attach(struct runtime_task *task)
{

    struct syscall_attach_args *args = (struct syscall_attach_args *)task->registers.sp;

    return syscall_attach(task, args);

}

static unsigned int handle_close(struct runtime_task *task)
{

    struct syscall_close_args *args = (struct syscall_close_args *)task->registers.sp;

    return syscall_close(task, args);

}

static unsigned int handle_detach(struct runtime_task *task)
{

    struct syscall_detach_args *args = (struct syscall_detach_args *)task->registers.sp;

    return syscall_detach(task, args);

}

static unsigned int handle_execute(struct runtime_task *task)
{

    struct syscall_execute_args *args = (struct syscall_execute_args *)task->registers.sp;

    return syscall_execute(task, args);

}

static unsigned int handle_exit(struct runtime_task *task)
{

    struct syscall_exit_args *args = (struct syscall_exit_args *)task->registers.sp;

    return syscall_exit(task, args);

}

static unsigned int handle_load(struct runtime_task *task)
{

    struct syscall_load_args *args = (struct syscall_load_args *)task->registers.sp;

    return syscall_load(task, args);

}

static unsigned int handle_open(struct runtime_task *task)
{

    struct syscall_open_args *args = (struct syscall_open_args *)task->registers.sp;

    return syscall_open(task, args);

}

static unsigned int handle_read(struct runtime_task *task)
{

    struct syscall_read_args *args = (struct syscall_read_args *)task->registers.sp;

    return syscall_read(task, args);

}

static unsigned int handle_unload(struct runtime_task *task)
{

    struct syscall_unload_args *args = (struct syscall_unload_args *)task->registers.sp;

    return syscall_unload(task, args);

}

static unsigned int handle_wait(struct runtime_task *task)
{

    struct syscall_wait_args *args = (struct syscall_wait_args *)task->registers.sp;

    return syscall_wait(task, args);

}

static unsigned int handle_write(struct runtime_task *task)
{

    struct syscall_write_args *args = (struct syscall_write_args *)task->registers.sp;

    return syscall_write(task, args);

}

static void register_routine(unsigned char index, unsigned int (*routine)(struct runtime_task *task))
{

    routines[index] = routine;

}

static void handle_interrupt(struct runtime_task *task, struct isr_cpu_registers *registers)
{

    if (!routines[registers->error])
    {

        registers->general.eax = 0;

        return;

    }

    registers->general.eax = routines[registers->error](task);

}

void syscall_setup()
{

    register_routine(SYSCALL_INDEX_OPEN, handle_open);
    register_routine(SYSCALL_INDEX_CLOSE, handle_close);
    register_routine(SYSCALL_INDEX_READ, handle_read);
    register_routine(SYSCALL_INDEX_WRITE, handle_write);
    register_routine(SYSCALL_INDEX_EXECUTE, handle_execute);
    register_routine(SYSCALL_INDEX_EXIT, handle_exit);
    register_routine(SYSCALL_INDEX_WAIT, handle_wait);
    register_routine(SYSCALL_INDEX_LOAD, handle_load);
    register_routine(SYSCALL_INDEX_UNLOAD, handle_unload);
    register_routine(SYSCALL_INDEX_ATTACH, handle_attach);
    register_routine(SYSCALL_INDEX_DETACH, handle_detach);

    isr_register_routine(ISR_INDEX_SYSCALL, handle_interrupt);

}

