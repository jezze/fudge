#include <fudge/module.h>
#include <base/base.h>
#include <base/block.h>
#include <block/block.h>
#include "ext2.h"

static struct ext2_protocol protocol;

void module_init()
{

    ext2_init_protocol(&protocol);

}

void module_register()
{

    block_register_protocol(&protocol.base);

}

void module_unregister()
{

    block_unregister_protocol(&protocol.base);

}

