#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/block/block.h>

static struct block_protocol blockprotocol;

static void blockprotocol_notify(void *buffer, unsigned int count)
{

    kernel_notify(&blockprotocol.data.states, EVENT_DATA, buffer, count);

}

void module_init(void)
{

    block_initprotocol(&blockprotocol, "cpio", blockprotocol_notify);

}

void module_register(void)
{

    block_registerprotocol(&blockprotocol);

}

void module_unregister(void)
{

    block_unregisterprotocol(&blockprotocol);

}

