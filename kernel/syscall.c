#include <lib/types.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/screen.h>
#include <kernel/vfs.h>
#include <kernel/syscall.h>

unsigned int syscall_handler(uint32_t num, void *data)
{

    if (num == SYSCALL_STRING_PUTS)
        screen_puts((char *)data);

    if (num == SYSCALL_VFS_WALK)
        return (unsigned int)vfs_walk(fsRoot, *((unsigned int *)data));

    return 0;

}

void syscall_init()
{

}

