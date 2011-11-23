#include <lib/file.h>
#include <kernel/vfs.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>
#include <kernel/arch/x86/syscall.h>

static void *syscallRoutines[SYSCALL_ROUTINE_SLOTS];

static void syscall_attach_handler(struct syscall_registers *registers)
{

    unsigned int index = registers->ebx;
    void (*handler)() = (void *)registers->ecx;

    registers->eax = syscall_attach(index, handler);

}

static void syscall_close_handler(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;

    registers->eax = syscall_close(fd);

}

static void syscall_detach_handler(struct syscall_registers *registers)
{

    unsigned int index = registers->ebx;

    registers->eax = syscall_detach(index);

}

static void syscall_execute_handler(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;
    unsigned int argc = registers->ecx;
    char **argv = (char **)registers->ebx;

    registers->eax = syscall_execute(path, argc, argv);

}

static void syscall_exit_handler(struct syscall_registers *registers)
{

    registers->eax = syscall_exit();

}

static void syscall_load_handler(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;

    registers->eax = syscall_load(path);

}

static void syscall_open_handler(struct syscall_registers *registers)
{

    char *view = (char *)registers->esi;
    char *name = (char *)registers->edi;

    registers->eax = syscall_open(view, name);

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

    registers->eax = syscall_reboot();

}

static void syscall_unload_handler(struct syscall_registers *registers)
{

    registers->eax = syscall_unload();

}

static void syscall_wait_handler(struct syscall_registers *registers)
{

    registers->eax = syscall_wait();

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

    if (!handler)
        return;

    struct runtime_task *task = runtime_get_running_task();

    if (task)
    {

        void *ip = (void *)registers->eip;
        void *sp = (void *)registers->useresp;
        void *sb = (void *)registers->ebp;

        task->save_registers(task, ip, sp, sb);

    }

    handler(registers);

    struct runtime_task *atask = runtime_get_running_task();

    if (atask)
    {

        registers->eip = (unsigned int)atask->registers.ip;
        registers->useresp = (unsigned int)atask->registers.sp;
        registers->ebp = (unsigned int)atask->registers.sb;

    }

}

void syscall_init()
{

    syscall_register_handler(SYSCALL_ROUTINE_OPEN, syscall_open_handler);
    syscall_register_handler(SYSCALL_ROUTINE_CLOSE, syscall_close_handler);
    syscall_register_handler(SYSCALL_ROUTINE_READ, syscall_read_handler);
    syscall_register_handler(SYSCALL_ROUTINE_WRITE, syscall_write_handler);
    syscall_register_handler(SYSCALL_ROUTINE_EXECUTE, syscall_execute_handler);
    syscall_register_handler(SYSCALL_ROUTINE_EXIT, syscall_exit_handler);
    syscall_register_handler(SYSCALL_ROUTINE_WAIT, syscall_wait_handler);
    syscall_register_handler(SYSCALL_ROUTINE_LOAD, syscall_load_handler);
    syscall_register_handler(SYSCALL_ROUTINE_UNLOAD, syscall_unload_handler);
    syscall_register_handler(SYSCALL_ROUTINE_REBOOT, syscall_reboot_handler);
    syscall_register_handler(SYSCALL_ROUTINE_ATTACH, syscall_attach_handler);
    syscall_register_handler(SYSCALL_ROUTINE_DETACH, syscall_detach_handler);

}

