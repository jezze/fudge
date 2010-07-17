#include <lib/vfs.h>
#include <lib/call.h>
#include <kernel/kernel.h>
#include <kernel/isr.h>
#include <kernel/screen.h>
#include <kernel/rtc.h>
#include <kernel/pit.h>
#include <kernel/syscall.h>

void *syscallRoutines[SYSCALL_TABLE_SIZE];

void syscall_register_handler(unsigned char index, unsigned int (*handler)(struct syscall_registers *registers))
{

    syscallRoutines[index] = handler;

}

void syscall_unregister_handler(unsigned char index)
{

    syscallRoutines[index] = 0;

}

static void syscall_vfs_walk(struct syscall_registers *registers)
{

    registers->eax = (unsigned int)vfs_walk(fsRoot, registers->ecx);

}

static void syscall_vfs_find(struct syscall_registers *registers)
{

    registers->eax = (unsigned int)vfs_find(fsRoot, (char *)registers->esi);

}

static void syscall_reboot(struct syscall_registers *registers)
{

    kernel_reboot();

}

void syscall_handler(struct syscall_registers *registers)
{

    void (*handler)(struct syscall_registers *registers) = syscallRoutines[registers->eax];

    if (handler)
        handler(registers);

}

void syscall_init()
{

    syscallRoutines[CALL_VFS_WALK] = syscall_vfs_walk;
    syscallRoutines[CALL_VFS_FIND] = syscall_vfs_find;
    syscallRoutines[CALL_REBOOT] = syscall_reboot;

}

