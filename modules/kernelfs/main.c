#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include "backend.h"
#include "protocol.h"

static struct vfs_backend backend;
static struct vfs_protocol protocol;

void init()
{

    kernelfs_init_backend(&backend);
    kernelfs_init_protocol(&protocol);
    resource_register(&backend.resource);
    resource_register(&protocol.resource);

}

void destroy()
{

}

