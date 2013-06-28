#include <fudge/kernel.h>
#include "vfs.h"
#include "binary.h"

static struct binary_protocol *protocols;

struct binary_protocol *binary_get_protocol(struct vfs_protocol *protocol, unsigned int id)
{

    struct binary_protocol *current;

    for (current = protocols; current; current = current->sibling)
    {

        if (current->match(protocol, id))
            return current;

    }

    return 0;

}

void binary_register_protocol(struct binary_protocol *protocol)
{

    struct binary_protocol *current;

    if (!protocols)
    {

        protocols = protocol;

        return;

    }

    for (current = protocols; current; current = current->sibling)
    {

        if (current->sibling)
            continue;

        current->sibling = protocol;

        return;

    }

}

void binary_unregister_protocol(struct binary_protocol *protocol)
{

    struct binary_protocol *current;

    if (protocols == protocol)
    {

        protocols = protocols->sibling;

        return;

    }

    for (current = protocols; current; current = current->sibling)
    {

        if (current->sibling != protocol)
            continue;

        current->sibling = current->sibling->sibling;

        return;

    }

}

void binary_init_protocol(struct binary_protocol *protocol, unsigned int (*match)(struct vfs_protocol *protocol, unsigned int id), unsigned int (*find_symbol)(struct vfs_protocol *protocol, unsigned int id, unsigned int count, const char *symbol), unsigned int (*copy_program)(struct vfs_protocol *protocol, unsigned int id), unsigned int (*relocate)(struct vfs_protocol *protocol, unsigned int id, unsigned int address))
{

    memory_clear(protocol, sizeof (struct binary_protocol));

    protocol->match = match;
    protocol->find_symbol = find_symbol;
    protocol->copy_program = copy_program;
    protocol->relocate = relocate;

}

void binary_setup()
{

    protocols = 0;

}

