#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/ext2/ext2.h>

static struct ext2_driver *driver;
static struct vfs_filesystem filesystem;

static struct vfs_node *get_node(struct vfs_filesystem *self, unsigned int index)
{

    return 0;

}

static struct vfs_node *find_node(struct vfs_filesystem *self, char *name)
{

    return 0;

}

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

    vfs_filesystem_init(&filesystem, 2, get_node, find_node, walk); 
    vfs_register_filesystem(&filesystem);

}

