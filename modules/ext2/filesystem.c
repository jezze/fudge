#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/ext2/ext2.h>

static struct ext2_driver *driver;
static struct vfs_filesystem filesystem;

static unsigned int read(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    driver->read_node(driver, id, buffer);

    return 4;

}

static unsigned int find(struct vfs_filesystem *self, char *name)
{

    if (string_find("ext2file1", name))
        return 14;

    if (string_find("ext2file2", name))
        return 15;

    return 0;

}

static char *get_name(struct vfs_filesystem *self, unsigned int id)
{

    if (id == 14)
        return "ext2file1";

    if (id == 15)
        return "ext2file2";

    return 0;

}

void ext2_filesystem_init(struct modules_module *module)
{

    driver = (struct ext2_driver *)module;

    vfs_filesystem_init(&filesystem, "/hda", 0, 0, read, 0, find, get_name, 0); 
    vfs_register_filesystem(&filesystem);

}

