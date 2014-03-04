#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "binary.h"

static struct resource_list protocols;

struct binary_protocol *binary_find_protocol(struct vfs_channel *channel, unsigned int id)
{

    struct list_item *current;

    for (current = protocols.list.head; current; current = current->next)
    {

        struct binary_protocol *protocol = current->data;

        if (protocol->match(channel, id))
            return protocol;

    }

    return 0;

}

void binary_init_protocol(struct binary_protocol *protocol, unsigned int (*match)(struct vfs_channel *channel, unsigned int id), unsigned long (*find_symbol)(struct vfs_channel *channel, unsigned int id, unsigned int count, const char *symbol), unsigned long (*copy_program)(struct vfs_channel *channel, unsigned int id), unsigned int (*relocate)(struct vfs_channel *channel, unsigned int id, unsigned int address))
{

    memory_clear(protocol, sizeof (struct binary_protocol));
    resource_init_item(&protocol->resource, protocol);

    protocol->match = match;
    protocol->find_symbol = find_symbol;
    protocol->copy_program = copy_program;
    protocol->relocate = relocate;

}

void binary_setup()
{

    resource_init_list(&protocols, RESOURCE_TYPE_BINARYPROTOCOL, "binaryprotocols");
    resource_register_list(&protocols);

}

