#include <lib/types.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/screen.h>
#include <kernel/vfs.h>
#include <kernel/rtc.h>
#include <kernel/pit.h>
#include <kernel/syscall.h>

unsigned int syscall_handler(unsigned int num, void *data)
{

    if (num == SYSCALL_SCREEN_PUTS)
        screen_puts((char *)data);

    if (num == SYSCALL_SCREEN_PUTS_DEC)
        screen_puts_dec(*((unsigned int *)data));

    if (num == SYSCALL_SCREEN_PUTS_HEX)
        screen_puts_hex(*((unsigned int *)data));

    if (num == SYSCALL_RTC_DATE)
        rtc_init();

    if (num == SYSCALL_PIT_READ)
        return pitTimer;

    if (num == SYSCALL_VFS_WALK)
        return (unsigned int)vfs_walk(fsRoot, *((unsigned int *)data));

    return 0;

}

void syscall_init()
{

}

