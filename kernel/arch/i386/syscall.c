#include <lib/vfs.h>
#include <lib/call.h>
#include <kernel/arch/i386/arch.h>
#include <kernel/arch/i386/io.h>
#include <kernel/arch/i386/isr.h>
#include <kernel/arch/i386/pit.h>
#include <kernel/arch/i386/rtc.h>
#include <kernel/arch/i386/screen.h>
#include <kernel/arch/i386/syscall.h>
#include <kernel/kernel.h>

void *syscallRoutines[SYSCALL_TABLE_SIZE];

void syscall_register_handler(unsigned char index, void (*handler)(struct syscall_registers *registers))
{

    syscallRoutines[index] = handler;

}

void syscall_unregister_handler(unsigned char index)
{

    syscallRoutines[index] = 0;

}

void syscall_vfs_walk(struct syscall_registers *registers)
{

    registers->eax = (unsigned int)vfs_walk(fsRoot, registers->ecx);

}

void syscall_vfs_find(struct syscall_registers *registers)
{

    registers->eax = (unsigned int)vfs_find(fsRoot, (char *)registers->esi);

}

void syscall_reboot(struct syscall_registers *registers)
{

    arch_reboot();

}

void syscall_handler(struct syscall_registers *registers)
{

    void (*handler)(struct syscall_registers *registers) = syscallRoutines[registers->eax];

    if (handler)
        handler(registers);

}

void syscall_init()
{

}

