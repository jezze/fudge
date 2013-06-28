#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <kernel/vfs.h>
#include "temp.h"
#include "filesystem.h"

static struct temp_filesystem filesystem;

struct vfs_backend *get_backend()
{

    return 0;

}

struct vfs_protocol *get_protocol()
{

    return &filesystem.base;

}

void init()
{

    temp_init_filesystem(&filesystem);

}

void destroy()
{

}

