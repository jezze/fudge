#include <kernel/vfs.h>
#include <base/base.h>
#include "system.h"

static struct system_filesystem filesystem;

struct vfs_interface *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    system_init_filesystem(&filesystem);

}

void destroy()
{

}

