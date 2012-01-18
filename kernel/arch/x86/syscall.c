#include <kernel/mmu.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>
#include <kernel/arch/x86/syscall.h>

static void *routines[SYSCALL_ROUTINE_SLOTS];

static unsigned int syscall_handle_attach(unsigned int stack, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(stack + 4);
    void (*routine)() = *(void **)(stack + 8);

    return syscall_attach(task, index, routine);

}

static unsigned int syscall_handle_close(unsigned int stack, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return syscall_close(task, index);

}

static unsigned int syscall_handle_detach(unsigned int stack, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(stack + 4);

    return syscall_detach(task, index);

}

static unsigned int syscall_handle_halt(unsigned int stack, struct runtime_task *task)
{

    return syscall_halt(task);

}

static unsigned int syscall_handle_execute(unsigned int stack, struct runtime_task *task)
{

    char *path = *(char **)(stack + 24);
    unsigned int argc = *(unsigned int *)(stack + 28);
    char **argv = *(char ***)(stack + 32);

    return syscall_execute(task, path, argc, argv);

}

static unsigned int syscall_handle_exit(unsigned int stack, struct runtime_task *task)
{

    return syscall_exit(task);

}

static unsigned int syscall_handle_load(unsigned int stack, struct runtime_task *task)
{

    char *path = *(char **)(stack + 4);

    return syscall_load(task, path);

}

static unsigned int syscall_handle_open(unsigned int stack, struct runtime_task *task)
{

    char *path = *(char **)(stack + 4);

    return syscall_open(task, path);

}

static unsigned int syscall_handle_read(unsigned int stack, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(stack + 4);
    char *buffer = *(char **)(stack + 8);
    unsigned int count = *(unsigned int *)(stack + 12);

    return syscall_read(task, index, count, buffer);

}

static unsigned int syscall_handle_reboot(unsigned int stack, struct runtime_task *task)
{

    return syscall_reboot(task);

}

static unsigned int syscall_handle_unload(unsigned int stack, struct runtime_task *task)
{

    char *path = *(char **)(stack + 4);

    return syscall_unload(task, path);

}

static unsigned int syscall_handle_wait(unsigned int stack, struct runtime_task *task)
{

    return syscall_wait(task);

}

static unsigned int syscall_handle_write(unsigned int stack, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(stack + 4);
    char *buffer = *(char **)(stack + 8);
    unsigned int count = *(unsigned int *)(stack + 12);

    return syscall_write(task, index, count, buffer);

}

static void syscall_register_routine(unsigned char index, unsigned int (*routine)(unsigned int stack, struct runtime_task *task))
{

    routines[index] = routine;

}

static void syscall_save_state(struct runtime_task *task, struct syscall_registers *registers)
{

    task->registers.ip = registers->eip;
    task->registers.sp = registers->useresp;
    task->registers.sb = registers->ebp;

}

static void syscall_load_state(struct runtime_task *task, struct syscall_registers *registers)
{

    registers->eip = task->registers.ip;
    registers->useresp = task->registers.sp;
    registers->ebp = task->registers.sb;

}

void syscall_handle(struct syscall_registers *registers)
{

    unsigned int (*routine)(unsigned int stack, struct runtime_task *task) = routines[registers->eax];

    if (!routine)
        return;

    struct runtime_task *task = runtime_get_running_task();

    syscall_save_state(task, registers);
    registers->eax = routine(registers->useresp, task);
    syscall_load_state(runtime_get_running_task(), registers);

}

void syscall_init()
{

    syscall_register_routine(SYSCALL_ROUTINE_OPEN, syscall_handle_open);
    syscall_register_routine(SYSCALL_ROUTINE_CLOSE, syscall_handle_close);
    syscall_register_routine(SYSCALL_ROUTINE_READ, syscall_handle_read);
    syscall_register_routine(SYSCALL_ROUTINE_WRITE, syscall_handle_write);
    syscall_register_routine(SYSCALL_ROUTINE_EXECUTE, syscall_handle_execute);
    syscall_register_routine(SYSCALL_ROUTINE_EXIT, syscall_handle_exit);
    syscall_register_routine(SYSCALL_ROUTINE_WAIT, syscall_handle_wait);
    syscall_register_routine(SYSCALL_ROUTINE_LOAD, syscall_handle_load);
    syscall_register_routine(SYSCALL_ROUTINE_UNLOAD, syscall_handle_unload);
    syscall_register_routine(SYSCALL_ROUTINE_HALT, syscall_handle_halt);
    syscall_register_routine(SYSCALL_ROUTINE_REBOOT, syscall_handle_reboot);
    syscall_register_routine(SYSCALL_ROUTINE_ATTACH, syscall_handle_attach);
    syscall_register_routine(SYSCALL_ROUTINE_DETACH, syscall_handle_detach);

}

