#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/assert.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/mboot.h>
#include <kernel/shell.h>

struct vfs_node vfsRoot;

static void kernel_init_vfs()
{

    string_copy(vfsRoot.name, "root");
    vfsRoot.inode = 0;
    vfsRoot.length = 0;
    vfsRoot.open = 0;
    vfsRoot.close = 0;
    vfsRoot.read = 0;
    vfsRoot.write = 0;
    vfsRoot.walk = 0;

}

struct vfs_node *kernel_get_vfs_root()
{

    return &vfsRoot;

}

void kernel_main(struct mboot_info *header, unsigned int magic)
{

    kernel_init_vfs();

    struct vfs_node *initrd = initrd_init(*((unsigned int *)header->modulesAddresses));

    vfsRoot.walk = initrd->walk;

    ASSERT(magic == MBOOT_MAGIC);
    ASSERT(header->modulesCount);

    arch_init();
    arch_init_syscalls();
    arch_init_devices();
    arch_init_interrupts();
    arch_enable_interrupts();

    shell_init();

    for (;;);

}

