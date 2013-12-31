#include <fudge/kernel.h>
#include "vfs.h"
#include "binary.h"

static struct list protocols;

struct binary_protocol *binary_find_protocol(struct vfs_session *session, unsigned int id)
{

    struct list_item *current;

    for (current = protocols.head; current; current = current->next)
    {

        struct binary_protocol *protocol = current->self;

        if (protocol->match(session, id))
            return protocol;

    }

    return 0;

}

void binary_register_protocol(struct binary_protocol *protocol)
{

    list_add(&protocols, &protocol->item);

}

void binary_unregister_protocol(struct binary_protocol *protocol)
{

    list_remove(&protocols, &protocol->item);

}

void binary_init_protocol(struct binary_protocol *protocol, unsigned int (*match)(struct vfs_session *session, unsigned int id), unsigned long (*find_symbol)(struct vfs_session *session, unsigned int id, unsigned int count, const char *symbol), unsigned long (*copy_program)(struct vfs_session *session, unsigned int id), unsigned int (*relocate)(struct vfs_session *session, unsigned int id, unsigned int address))
{

    memory_clear(protocol, sizeof (struct binary_protocol));
    list_init_item(&protocol->item, protocol);

    protocol->match = match;
    protocol->find_symbol = find_symbol;
    protocol->copy_program = copy_program;
    protocol->relocate = relocate;

}

void binary_setup()
{

    list_init(&protocols);

}

