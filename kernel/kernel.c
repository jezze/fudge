#include <lib/vfs.h>
#include <kernel/assert.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/mboot.h>
#include <kernel/shell.h>

struct vfs_node *fsRoot = 0;

static void shell_load()
{

    vfs_read(vfs_find(fsRoot, "shell"), 0, 5000, (void *)0x300000);

    void (*func)(int argc, char *argv[]) = (void (*)(int argc, char *argv[]))0x300000;

    func(0, 0);

}

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

    //shell_load();
    shell_init();

    for (;;);

}

