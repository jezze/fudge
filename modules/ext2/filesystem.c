#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/ext2/ext2.h>

static struct ext2_driver *driver;
static struct vfs_filesystem filesystem;

static unsigned int walk(struct vfs_filesystem *self, unsigned int index)
{

    static char mem[1024];
    void *buffer = mem;

    driver->read_node(driver, 2, buffer);

    for (;;)
    {

        struct ext2_directory *directory = buffer;

        if (!directory->length)
            return 0;

        buffer += directory->size;

    }

    return 0;

}

void ext2_filesystem_init(struct modules_module *module)
{

    driver = (struct ext2_driver *)module;

    vfs_filesystem_init(&filesystem, "hda/", 2, 0, 0, 0, 0, 0, 0, 0, walk, 0); 
    vfs_register_filesystem(&filesystem);

}

