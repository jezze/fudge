#include <fudge/memory.h>
#include <base/base.h>
#include "block.h"

void block_init_protocol(struct block_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct block_protocol));

    protocol->name = name;

}

