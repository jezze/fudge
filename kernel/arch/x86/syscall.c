#include <kernel/runtime.h>
#include <kernel/syscall.h>
#include <kernel/arch/x86/syscall.h>

static void *routines[SYSCALL_ROUTINE_SLOTS];

static void syscall_handle_attach(struct syscall_registers *registers)
{

    unsigned int index = registers->ebx;
    void (*routine)() = (void *)registers->ecx;

    registers->eax = syscall_attach(index, routine);

}

static void syscall_handle_close(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;

    registers->eax = syscall_close(fd);

}

static void syscall_handle_detach(struct syscall_registers *registers)
{

    unsigned int index = registers->ebx;

    registers->eax = syscall_detach(index);

}

static void syscall_handle_halt(struct syscall_registers *registers)
{

    registers->eax = syscall_halt();

}

static void syscall_handle_execute(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;
    unsigned int argc = registers->ecx;
    char **argv = (char **)registers->ebx;

    registers->eax = syscall_execute(path, argc, argv);

}

static void syscall_handle_exit(struct syscall_registers *registers)
{

    registers->eax = syscall_exit();

}

static void syscall_handle_load(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;

    registers->eax = syscall_load(path);

}

static void syscall_handle_open(struct syscall_registers *registers)
{

    char *view = (char *)registers->esi;
    char *name = (char *)registers->edi;

    registers->eax = syscall_open(view, name);

}

static void syscall_handle_read(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    unsigned int count = registers->ecx;
    char *buffer = (char *)registers->esi;

    registers->eax = syscall_read(fd, count, buffer);

}

static void syscall_handle_reboot(struct syscall_registers *registers)
{

    registers->eax = syscall_reboot();

}

static void syscall_handle_unload(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;

    registers->eax = syscall_unload(path);

}

static void syscall_handle_wait(struct syscall_registers *registers)
{

    registers->eax = syscall_wait();

}

static void syscall_handle_write(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    unsigned int count = registers->ecx;
    char *buffer = (char *)registers->esi;

    registers->eax = syscall_write(fd, count, buffer);

}

static void syscall_register_routine(unsigned char index, void (*routine)(struct syscall_registers *registers))
{

    routines[index] = routine;

}

void syscall_handle(struct syscall_registers *registers)
{

    void (*routine)(struct syscall_registers *registers) = routines[registers->eax];

    if (!routine)
        return;

    runtime_set_state(registers->eip, registers->useresp, registers->ebp);
    routine(registers);
    runtime_get_state(&registers->eip, &registers->useresp, &registers->ebp);

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
    syscall_register_routine(SYSCALL_ROUTINE_REBOOT, syscall_handle_reboot);
    syscall_register_routine(SYSCALL_ROUTINE_ATTACH, syscall_handle_attach);
    syscall_register_routine(SYSCALL_ROUTINE_DETACH, syscall_handle_detach);

}

