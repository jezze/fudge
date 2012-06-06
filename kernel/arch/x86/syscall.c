#include <kernel/runtime.h>
#include <kernel/syscall.h>
#include <kernel/arch/x86/syscall.h>

static unsigned int (*routines[SYSCALL_TABLE_SLOTS])(struct runtime_task *task, unsigned int stack);

static unsigned int handle_attach(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);
    unsigned int callback = *(unsigned int *)(stack + 8);

    return syscall_attach(task, index, callback);

}

static unsigned int handle_close(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return syscall_close(task, index);

}

static unsigned int handle_detach(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return syscall_detach(task, index);

}

static unsigned int handle_halt(struct runtime_task *task, unsigned int stack)
{

    return syscall_halt(task);

}

static unsigned int handle_execute(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return syscall_execute(task, index);

}

static unsigned int handle_exit(struct runtime_task *task, unsigned int stack)
{

    return syscall_exit(task);

}

static unsigned int handle_load(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return syscall_load(task, index);

}

static unsigned int handle_open(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);
    char *path = *(char **)(stack + 8);

    return syscall_open(task, index, path);

}

static unsigned int handle_read(struct runtime_task *task, unsigned int stack)
{

    unsigned int id = *(unsigned int *)(stack + 4);
    unsigned int offset = *(unsigned int *)(stack + 8);
    unsigned int count = *(unsigned int *)(stack + 12);
    char *buffer = *(char **)(stack + 16);

    return syscall_read(task, id, offset, count, buffer);

}

static unsigned int handle_reboot(struct runtime_task *task, unsigned int stack)
{

    return syscall_reboot(task);

}

static unsigned int handle_unload(struct runtime_task *task, unsigned int stack)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return syscall_unload(task, index);

}

static unsigned int handle_wait(struct runtime_task *task, unsigned int stack)
{

    return syscall_wait(task);

}

static unsigned int handle_write(struct runtime_task *task, unsigned int stack)
{

    unsigned int id = *(unsigned int *)(stack + 4);
    unsigned int offset = *(unsigned int *)(stack + 8);
    unsigned int count = *(unsigned int *)(stack + 12);
    char *buffer = *(char **)(stack + 16);

    return syscall_write(task, id, offset, count, buffer);

}

static void register_routine(unsigned char index, unsigned int (*routine)(struct runtime_task *task, unsigned int stack))
{

    routines[index] = routine;

}

unsigned int syscall_raise(unsigned int index, struct runtime_task *task)
{

    unsigned int (*routine)(struct runtime_task *task, unsigned int stack) = routines[index];

    if (!routine)
        return 0;

    return routine(task, task->registers.sp);

}

void syscall_init()
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
    register_routine(SYSCALL_INDEX_HALT, handle_halt);
    register_routine(SYSCALL_INDEX_REBOOT, handle_reboot);
    register_routine(SYSCALL_INDEX_ATTACH, handle_attach);
    register_routine(SYSCALL_INDEX_DETACH, handle_detach);

}

