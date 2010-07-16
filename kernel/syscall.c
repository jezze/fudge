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

static void syscall_vfs_walk(syscall_registers_t *registers)
{

    registers->eax = (unsigned int)vfs_walk(fsRoot, registers->ecx);

}

static void syscall_vfs_find(syscall_registers_t *registers)
{

    registers->eax = (unsigned int)vfs_find(fsRoot, (char *)registers->ecx);

}

static void syscall_reboot(syscall_registers_t *registers)
{

    kernel_reboot();

}

void syscall_handler(syscall_registers_t registers)
{

    void (*handler)(syscall_registers_t *registers) = syscallRoutines[registers.eax];

    if (handler)
        handler(&registers);

}

void syscall_init()
{

    syscallRoutines[CALL_VFS_WALK] = syscall_vfs_walk;
    syscallRoutines[CALL_VFS_FIND] = syscall_vfs_find;
    syscallRoutines[CALL_REBOOT] = syscall_reboot;

}

