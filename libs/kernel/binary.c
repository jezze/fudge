#include <fudge/kernel.h>
#include "vfs.h"
#include "binary.h"

static struct binary_interface *interfaces;

struct binary_interface *binary_get_interface(struct vfs_interface *interface, unsigned int id)
{

    struct binary_interface *current;

    for (current = interfaces; current; current = current->sibling)
    {

        if (current->match(interface, id))
            return current;

    }

    return 0;

}

void binary_register_interface(struct binary_interface *interface)
{

    struct binary_interface *current;

    if (!interfaces)
    {

        interfaces = interface;

        return;

    }

    for (current = interfaces; current; current = current->sibling)
    {

        if (current->sibling)
            continue;

        current->sibling = interface;

        return;

    }

}

void binary_unregister_interface(struct binary_interface *interface)
{

    struct binary_interface *current;

    if (interfaces == interface)
    {

        interfaces = interfaces->sibling;

        return;

    }

    for (current = interfaces; current; current = current->sibling)
    {

        if (current->sibling != interface)
            continue;

        current->sibling = current->sibling->sibling;

        return;

    }

}

void binary_init_interface(struct binary_interface *interface, unsigned int (*match)(struct vfs_interface *interface, unsigned int id), unsigned int (*find_symbol)(struct vfs_interface *interface, unsigned int id, unsigned int count, const char *symbol), unsigned int (*copy_program)(struct vfs_interface *interface, unsigned int id), unsigned int (*relocate)(struct vfs_interface *interface, unsigned int id, unsigned int address))
{

    memory_clear(interface, sizeof (struct binary_interface));

    interface->match = match;
    interface->find_symbol = find_symbol;
    interface->copy_program = copy_program;
    interface->relocate = relocate;

}

void binary_setup()
{

    interfaces = 0;

}

