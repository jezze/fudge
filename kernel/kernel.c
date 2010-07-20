#include <lib/vfs.h>
#include <kernel/assert.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/mboot.h>
#include <kernel/shell.h>

struct vfs_node *fsRoot = 0;

void kernel_main(struct mboot_info *header, unsigned int magic)
{

    arch_init();
    arch_init_syscalls();
    arch_init_devices();
    arch_init_interrupts();
    arch_enable_interrupts();

    ASSERT(magic == MBOOT_MAGIC);
    ASSERT(header->modulesCount);

    fsRoot = initrd_init(*((unsigned int *)header->modulesAddresses));

    mboot_init(header);

    shell_init();

    for (;;);

}

