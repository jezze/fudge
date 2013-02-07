#include <fudge/kernel.h>
#include "vfs.h"
#include "binary.h"

static struct binary_format *formats;

struct binary_format *binary_get_format(struct vfs_interface *interface, unsigned int id)
{

    struct binary_format *current;

    for (current = formats; current; current = current->sibling)
    {

        if (current->match(interface, id))
            return current;

    }

    return 0;

}

void binary_register_format(struct binary_format *format)
{

    struct binary_format *current;

    if (!formats)
    {

        formats = format;

        return;

    }

    for (current = formats; current; current = current->sibling)
    {

        if (current->sibling)
            continue;

        current->sibling = format;

        return;

    }

}

void binary_unregister_format(struct binary_format *format)
{

    struct binary_format *current;

    if (formats == format)
    {

        formats = formats->sibling;

        return;

    }

    for (current = formats; current; current = current->sibling)
    {

        if (current->sibling != format)
            continue;

        current->sibling = current->sibling->sibling;

        return;

    }

}

void binary_init_format(struct binary_format *format, unsigned int (*match)(struct vfs_interface *interface, unsigned int id), unsigned int (*find_symbol)(struct vfs_interface *interface, unsigned int id, unsigned int count, const char *symbol), unsigned int (*copy_program)(struct vfs_interface *interface, unsigned int id), unsigned int (*relocate)(struct vfs_interface *interface, unsigned int id, unsigned int address))
{

    memory_clear(format, sizeof (struct binary_format));

    format->match = match;
    format->find_symbol = find_symbol;
    format->copy_program = copy_program;
    format->relocate = relocate;

}

void binary_setup()
{

    formats = 0;

}

