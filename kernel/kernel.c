#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/assert.h>
#include <kernel/dev.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/mboot.h>
#include <kernel/shell.h>
#include <kernel/vfs.h>

struct vfs_node *vfsRoot;

struct vfs_node *kernel_get_vfs_root()
{

    return vfsRoot;

}

void kernel_init(struct mboot_info *header, unsigned int magic)
{

    vfsRoot = vfs_init();

    arch_init();
    arch_init_syscalls();
    arch_init_interrupts();
    arch_enable_interrupts();

    dev_init();
    arch_init_devices();

    ASSERT(magic == MBOOT_MAGIC);
    ASSERT(header->modulesCount);

    initrd_init(*((unsigned int *)header->modulesAddresses));

    shell_init();

    for (;;);

}

