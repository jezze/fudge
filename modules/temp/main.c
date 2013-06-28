#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <kernel/vfs.h>
#include "temp.h"
#include "filesystem.h"

static struct vfs_backend backend;
static struct vfs_protocol protocol;

struct vfs_backend *get_backend()
{

    return &backend;

}

void init()
{

    temp_init_backend(&backend);
    temp_init_protocol(&protocol);
    vfs_register_protocol(&protocol);

}

void destroy()
{

}

