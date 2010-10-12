#include <lib/vfs.h>
#include <lib/call.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/kernel/isr.h>
#include <arch/x86/kernel/pit.h>
#include <arch/x86/kernel/rtc.h>
#include <arch/x86/kernel/screen.h>
#include <arch/x86/kernel/syscall.h>
#include <kernel/kernel.h>
#include <kernel/vfs.h>

void *syscallRoutines[SYSCALL_ROUTINES_SIZE];

void syscall_register_handler(unsigned char index, void (*handler)(struct syscall_registers *registers))
{

    syscallRoutines[index] = handler;

}

void syscall_unregister_handler(unsigned char index)
{

    syscallRoutines[index] = 0;

}

void syscall_open(struct syscall_registers *registers)
{

    registers->eax = (unsigned int)vfs_find(vfs_get_root(), (char *)registers->esi);

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

