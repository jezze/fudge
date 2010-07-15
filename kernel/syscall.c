#include <lib/types.h>
#include <lib/vfs.h>
#include <lib/call.h>
#include <kernel/kernel.h>
#include <kernel/isr.h>
#include <kernel/screen.h>
#include <kernel/rtc.h>
#include <kernel/pit.h>
#include <kernel/syscall.h>

void *syscallRoutines[SYSCALL_TABLE_SIZE];

void syscall_register_handler(unsigned char index, unsigned int (*handler)(syscall_registers_t *registers))
{

    syscallRoutines[index] = handler;

}

void syscall_unregister_handler(unsigned char index)
{

    syscallRoutines[index] = 0;

}

static unsigned int syscall_vfs_walk(syscall_registers_t *registers)
{

    return (unsigned int)vfs_walk(fsRoot, registers->ecx);

}

static unsigned int syscall_vfs_find(syscall_registers_t *registers)
{

    return (unsigned int)vfs_find(fsRoot, (char *)registers->ecx);

}

static unsigned int syscall_reboot(syscall_registers_t *registers)
{

    kernel_reboot();

    return 0;

}

unsigned int syscall_handler(syscall_registers_t registers)
{

    unsigned int (*handler)(syscall_registers_t *registers) = syscallRoutines[registers.eax];

    if (handler)
        return handler(&registers);

    return 0;

}

void syscall_init()
{

    syscallRoutines[CALL_VFS_WALK] = syscall_vfs_walk;
    syscallRoutines[CALL_VFS_FIND] = syscall_vfs_find;
    syscallRoutines[CALL_REBOOT] = syscall_reboot;

}

