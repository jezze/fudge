#include <lib/types.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/screen.h>
#include <kernel/vfs.h>
#include <kernel/rtc.h>
#include <kernel/pit.h>
#include <kernel/syscall.h>

unsigned int syscall_handler(unsigned int eax, unsigned int ebx, unsigned int ecx, unsigned int edx, unsigned int edi)
{

    if (eax == SYSCALL_SCREEN_PUTS)
        screen_puts((char *)ecx);

    if (eax == SYSCALL_SCREEN_PUTS_DEC)
        screen_puts_dec(ecx);

    if (eax == SYSCALL_SCREEN_PUTS_HEX)
        screen_puts_hex(ecx);

    if (eax == SYSCALL_VFS_READ)
        return vfs_read((vfs_node_t *)ebx, 0, 400, (char *)edi);

    if (eax == SYSCALL_VFS_WALK)
        return (unsigned int)vfs_walk(fsRoot, ecx);

    if (eax == SYSCALL_VFS_FIND)
        return (unsigned int)vfs_find(fsRoot, (char *)ecx);

    return 0;

}

void syscall_init()
{

}

