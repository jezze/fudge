#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/ext2/ext2.h>

static struct ext2_driver *driver;
static struct vfs_filesystem filesystem;

static struct vfs_node *filesystem_get_node(struct vfs_filesystem *self, unsigned int index)
{

    return 0;

}

static struct vfs_node *filesystem_find_node(struct vfs_filesystem *self, char *name)
{

    return 0;

}

static unsigned int filesystem_walk(struct vfs_filesystem *self, unsigned int index)
{

    return 0;

}

void ext2_filesystem_init(struct modules_module *module)
{

    driver = (struct ext2_driver *)module;

    vfs_filesystem_init(&filesystem, filesystem_get_node, filesystem_find_node, filesystem_walk); 
    filesystem.firstIndex = 2;

    vfs_register_filesystem(&filesystem);

}

