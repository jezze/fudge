#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "binary.h"

struct binary_protocol *binary_findprotocol(struct vfs_channel *channel, unsigned int id)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_BINARYPROTOCOL)))
    {

        struct binary_protocol *protocol = current->data;

        if (protocol->match(channel, id))
            return protocol;

    }

    return 0;

}

void binary_initprotocol(struct binary_protocol *protocol, unsigned int (*match)(struct vfs_channel *channel, unsigned int id), unsigned long (*findsymbol)(struct vfs_channel *channel, unsigned int id, unsigned int count, const char *symbol), unsigned long (*copyprogram)(struct vfs_channel *channel, unsigned int id), unsigned int (*relocate)(struct vfs_channel *channel, unsigned int id, unsigned int address))
{

    resource_init(&protocol->resource, RESOURCE_BINARYPROTOCOL, protocol);

    protocol->match = match;
    protocol->findsymbol = findsymbol;
    protocol->copyprogram = copyprogram;
    protocol->relocate = relocate;

}

