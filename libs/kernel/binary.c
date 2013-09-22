#include <fudge/kernel.h>
#include "vfs.h"
#include "binary.h"

static struct binary_protocol *protocols;

struct binary_protocol *binary_find_protocol(struct vfs_session *session, unsigned int id)
{

    struct binary_protocol *current;

    for (current = protocols; current; current = current->sibling)
    {

        if (current->match(session, id))
            return current;

    }

    return 0;

}

static struct binary_protocol *find_presibling(struct binary_protocol *protocol)
{

    struct binary_protocol *current = protocols;

    if (current == protocol)
        return 0;

    while (current->sibling != protocol)
        current = current->sibling;

    return current;

}

void binary_register_protocol(struct binary_protocol *protocol)
{

    struct binary_protocol *current = find_presibling(0);

    if (current)
        current->sibling = protocol;
    else
        protocols = protocol;

}

void binary_unregister_protocol(struct binary_protocol *protocol)
{

    struct binary_protocol *current = find_presibling(protocol);

    if (current)
        current->sibling = current->sibling->sibling;
    else
        protocols = protocols->sibling;

}

void binary_init_protocol(struct binary_protocol *protocol, unsigned int (*match)(struct vfs_session *session, unsigned int id), unsigned int (*find_symbol)(struct vfs_session *session, unsigned int id, unsigned int count, const char *symbol), unsigned int (*copy_program)(struct vfs_session *session, unsigned int id), unsigned int (*relocate)(struct vfs_session *session, unsigned int id, unsigned int address))
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

