#include <lib/vfs.h>
#include <lib/call.h>
#include <kernel/arch/i386/io.h>
#include <kernel/arch/i386/isr.h>
#include <kernel/arch/i386/pit.h>
#include <kernel/arch/i386/rtc.h>
#include <kernel/arch/i386/screen.h>
#include <kernel/arch/i386/syscall.h>
#include <kernel/kernel.h>

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

unsigned int kernel_reboot()
{

    isr_disable();

    unsigned char ready = 0x02;

    while ((ready & 0x02) != 0)
        ready = inb(0x64);

    outb(0x64, 0xFE);

    return 0;

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

