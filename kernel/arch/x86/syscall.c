#include <lib/file.h>
#include <kernel/vfs.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>
#include <kernel/arch/x86/syscall.h>

static void *syscallRoutines[SYSCALL_ROUTINES_SIZE];

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
    void *eip = (void *)registers->eip;
    void *esp = (void *)registers->useresp;
    void *ebp = (void *)registers->ebp;

    registers->eax = syscall_execute(path, argc, argv, eip, esp, ebp);

    syscall_handle_event(0x03);

    struct runtime_task *task = runtime_get_running_task();

    registers->eip = (unsigned int)task->eip;
    registers->useresp = (unsigned int)task->esp;
    registers->ebp = (unsigned int)task->ebp;

}

static void syscall_exit_handler(struct syscall_registers *registers)
{

    registers->eax = syscall_exit();

    struct runtime_task *task = runtime_get_running_task();

    registers->eip = (unsigned int)task->eip;
    registers->useresp = (unsigned int)task->esp;
    registers->ebp = (unsigned int)task->ebp;

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

    registers->eax = syscall_load(path);

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

    registers->eax = syscall_reboot();

}

static void syscall_unload_handler(struct syscall_registers *registers)
{

    registers->eax = syscall_unload();

}

static void syscall_wait_handler(struct syscall_registers *registers)
{

    void *eip = (void *)registers->eip;
    void *esp = (void *)registers->useresp;
    void *ebp = (void *)registers->ebp;

    registers->eax = syscall_wait(eip, esp, ebp);

    struct runtime_task *task = runtime_get_running_task();

    registers->eip = (unsigned int)task->eip;
    registers->useresp = (unsigned int)task->esp;
    registers->ebp = (unsigned int)task->ebp;

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

