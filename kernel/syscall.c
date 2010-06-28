#include <lib/types.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/screen.h>
#include <kernel/vfs.h>
#include <kernel/rtc.h>
#include <kernel/pit.h>
#include <kernel/syscall.h>

unsigned int syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{

    if (eax == SYSCALL_SCREEN_PUTS)
        screen_puts((char *)ecx);

    if (eax == SYSCALL_SCREEN_PUTS_DEC)
        screen_puts_dec(ecx);

    if (eax == SYSCALL_SCREEN_PUTS_HEX)
        screen_puts_hex(ecx);

    if (eax == SYSCALL_VFS_WALK)
        return (unsigned int)vfs_walk(fsRoot, ecx);

    return 0;

}

void syscall_init()
{

}

