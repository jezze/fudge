#include <kernel/vfs.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>
#include <kernel/arch/x86/syscall.h>

static void *routines[SYSCALL_ROUTINE_SLOTS];

static void attach(struct syscall_registers *registers)
{

    unsigned int index = registers->ebx;
    void (*handler)() = (void *)registers->ecx;

    registers->eax = syscall_attach(index, handler);

}

static void close(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;

    registers->eax = syscall_close(fd);

}

static void detach(struct syscall_registers *registers)
{

    unsigned int index = registers->ebx;

    registers->eax = syscall_detach(index);

}

static void execute(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;
    unsigned int argc = registers->ecx;
    char **argv = (char **)registers->ebx;

    registers->eax = syscall_execute(path, argc, argv);

}

static void exit(struct syscall_registers *registers)
{

    registers->eax = syscall_exit();

}

static void load(struct syscall_registers *registers)
{

    char *path = (char *)registers->esi;

    registers->eax = syscall_load(path);

}

static void open(struct syscall_registers *registers)
{

    char *view = (char *)registers->esi;
    char *name = (char *)registers->edi;

    registers->eax = syscall_open(view, name);

}

static void read(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    unsigned int count = registers->ecx;
    char *buffer = (char *)registers->esi;

    registers->eax = syscall_read(fd, count, buffer);

}

static void reboot(struct syscall_registers *registers)
{

    registers->eax = syscall_reboot();

}

static void unload(struct syscall_registers *registers)
{

    registers->eax = syscall_unload();

}

static void wait(struct syscall_registers *registers)
{

    registers->eax = syscall_wait();

}

static void write(struct syscall_registers *registers)
{

    unsigned int fd = registers->ebx;
    unsigned int count = registers->ecx;
    char *buffer = (char *)registers->esi;

    registers->eax = syscall_write(fd, count, buffer);

}

static void register_routine(unsigned char index, void (*handler)(struct syscall_registers *registers))
{

    routines[index] = handler;

}

void syscall_handler(struct syscall_registers *registers)
{

    void (*handler)(struct syscall_registers *registers) = routines[registers->eax];

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

    register_routine(SYSCALL_ROUTINE_OPEN, open);
    register_routine(SYSCALL_ROUTINE_CLOSE, close);
    register_routine(SYSCALL_ROUTINE_READ, read);
    register_routine(SYSCALL_ROUTINE_WRITE, write);
    register_routine(SYSCALL_ROUTINE_EXECUTE, execute);
    register_routine(SYSCALL_ROUTINE_EXIT, exit);
    register_routine(SYSCALL_ROUTINE_WAIT, wait);
    register_routine(SYSCALL_ROUTINE_LOAD, load);
    register_routine(SYSCALL_ROUTINE_UNLOAD, unload);
    register_routine(SYSCALL_ROUTINE_REBOOT, reboot);
    register_routine(SYSCALL_ROUTINE_ATTACH, attach);
    register_routine(SYSCALL_ROUTINE_DETACH, detach);

}

