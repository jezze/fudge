#include <vfs.h>
#include <base/base.h>
#include <dev/dev.h>

static struct dev_filesystem filesystem;

struct vfs_interface *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    dev_init_filesystem(&filesystem);

}

void destroy()
{

}

