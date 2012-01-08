#include <kernel/mmu.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>
#include <kernel/arch/x86/syscall.h>

static void *routines[SYSCALL_ROUTINE_SLOTS];

static void syscall_handle_attach(struct syscall_registers *registers, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(registers->useresp + 4);
    void (*routine)() = *(unsigned int **)(registers->useresp + 8);

    registers->eax = syscall_attach(task, index, routine);

}

static void syscall_handle_close(struct syscall_registers *registers, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(registers->useresp + 4);

    registers->eax = syscall_close(task, index);

}

static void syscall_handle_detach(struct syscall_registers *registers, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(registers->useresp + 4);

    registers->eax = syscall_detach(task, index);

}

static void syscall_handle_halt(struct syscall_registers *registers, struct runtime_task *task)
{

    registers->eax = syscall_halt(task);

}

static void syscall_handle_execute(struct syscall_registers *registers, struct runtime_task *task)
{

    char *path = (char *)registers->esi;
    unsigned int argc = registers->ecx;
    char **argv = (char **)registers->ebx;

    registers->eax = syscall_execute(task, path, argc, argv);

}

static void syscall_handle_exit(struct syscall_registers *registers, struct runtime_task *task)
{

    registers->eax = syscall_exit(task);

}

static void syscall_handle_load(struct syscall_registers *registers, struct runtime_task *task)
{

    char *path = *(char **)(registers->useresp + 4);

    registers->eax = syscall_load(task, path);

}

static void syscall_handle_open(struct syscall_registers *registers, struct runtime_task *task)
{

    char *view = *(char **)(registers->useresp + 4);
    char *name = *(char **)(registers->useresp + 8);

    registers->eax = syscall_open(task, view, name);

}

static void syscall_handle_read(struct syscall_registers *registers, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(registers->useresp + 4);
    char *buffer = *(char **)(registers->useresp + 8);
    unsigned int count = *(unsigned int *)(registers->useresp + 12);

    registers->eax = syscall_read(task, index, count, buffer);

}

static void syscall_handle_reboot(struct syscall_registers *registers, struct runtime_task *task)
{

    registers->eax = syscall_reboot(task);

}

static void syscall_handle_unload(struct syscall_registers *registers, struct runtime_task *task)
{

    char *path = (char *)registers->esi;

    registers->eax = syscall_unload(task, path);

}

static void syscall_handle_wait(struct syscall_registers *registers, struct runtime_task *task)
{

    registers->eax = syscall_wait(task);

}

static void syscall_handle_write(struct syscall_registers *registers, struct runtime_task *task)
{

    unsigned int index = *(unsigned int *)(registers->useresp + 4);
    char *buffer = *(char **)(registers->useresp + 8);
    unsigned int count = *(unsigned int *)(registers->useresp + 12);

    registers->eax = syscall_write(task, index, count, buffer);

}

static void syscall_register_routine(unsigned char index, void (*routine)(struct syscall_registers *registers, struct runtime_task *task))
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

    void (*routine)(struct syscall_registers *registers, struct runtime_task *task) = routines[registers->eax];

    if (!routine)
        return;

    struct runtime_task *task = runtime_get_running_task();

    syscall_save_state(task, registers);
    routine(registers, task);
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

