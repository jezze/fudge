#include <lib/cbuffer.h>
#include <lib/vfs.h>
#include <lib/call.h>
#include <kernel/arch/i386/arch.h>
#include <kernel/assert.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/mboot.h>
#include <kernel/shell.h>

struct kernel kernel;
struct vfs_node *fsRoot = 0;

static void kernel_init_shell()
{

    struct vfs_node *node = call_vfs_find("shell");

    char *buffer = (char *)0x1F0000;

    vfs_read(node, 0, 2000, buffer);
        
    void (*func)(int argc, char *argv[]) = (void (*)(int argc, char *argv[]))0x1F0000;

    func(0, 0);

}

void kernel_main(struct mboot_info *header, unsigned int magic)
{

    arch_init();

    ASSERT(magic == MBOOT_MAGIC);
    ASSERT(header->modulesCount);

    fsRoot = initrd_init(*((unsigned int *)header->modulesAddresses));

    mboot_init(header);

    shell_init();

    for (;;);

}

