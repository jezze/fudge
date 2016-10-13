#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/block/block.h>
#include "part.h"

static struct part part;
static struct block_interface interface;

static unsigned int blockinterface_rdata(void *buffer, unsigned int count, unsigned int offset)
{

    offset += part.start;

    if (offset + count > part.end)
        count = part.end - offset;

    return part.parent->rdata(buffer, count, offset);

}

static unsigned int blockinterface_wdata(void *buffer, unsigned int count, unsigned int offset)
{

    offset += part.start;

    if (offset + count > part.end)
        count = part.end - offset;

    return part.parent->wdata(buffer, count, offset);

}

void module_init(void)
{

    block_initinterface(&interface, blockinterface_rdata, blockinterface_wdata);
    part_init(&part);

}

void module_register(void)
{

    part_register(&part);

}

void module_unregister(void)
{

    part_unregister(&part);

}

