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

void syscall_register_handler(unsigned char index, void (*handler)(syscall_registers_t *registers))
{

    syscallRoutines[index] = handler;

}

void syscall_unregister_handler(unsigned char index)
{

    syscallRoutines[index] = 0;

}

unsigned int syscall_handler(syscall_registers_t registers)
{

    if (registers.eax == SYSCALL_SCREEN_PUTS)
        screen_puts((char *)registers.ecx);

    if (registers.eax == SYSCALL_SCREEN_PUTS_DEC)
        screen_puts_dec(registers.ecx);

    if (registers.eax == SYSCALL_SCREEN_PUTS_HEX)
        screen_puts_hex(registers.ecx);

    if (registers.eax == SYSCALL_VFS_READ)
        return vfs_read((vfs_node_t *)registers.ebx, 0, 400, (char *)registers.edi);

    if (registers.eax == SYSCALL_VFS_WALK)
        return (unsigned int)vfs_walk(fsRoot, registers.ecx);

    if (registers.eax == SYSCALL_VFS_FIND)
        return (unsigned int)vfs_find(fsRoot, (char *)registers.ecx);

    if (registers.eax == SYSCALL_KERNEL_REBOOT)
        kernel_reboot();

    return 0;

}

void syscall_init()
{

}

