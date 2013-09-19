#include <fudge/module.h>
#include <base/base.h>
#include <base/block.h>
#include "block.h"

void block_register_protocol(struct block_protocol *protocol)
{

}

void block_unregister_protocol(struct block_protocol *protocol)
{

}

void block_init_protocol(struct block_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct block_protocol));

    protocol->name = name;

}

void init()
{

}

void destroy()
{

}

