#include <fudge/module.h>
#include <kernel/vfs.h>
#include <block/block.h>
#include "ext2.h"

static struct ext2_protocol protocol;

void init()
{

    ext2_init_protocol(&protocol);
    block_register_protocol(&protocol.base);

}

void destroy()
{

    block_unregister_protocol(&protocol.base);

}

