#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "null.h"

static struct system_stream root;

unsigned int read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

unsigned int write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void init()
{

    system_init_stream(&root, "null", read, write);
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

