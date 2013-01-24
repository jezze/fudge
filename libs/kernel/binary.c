#include <fudge/kernel.h>
#include "vfs.h"
#include "binary.h"

static struct binary_format *formats[BINARY_FORMAT_SLOTS];

static struct binary_format *get_format(struct vfs_interface *interface, unsigned int id)
{

    unsigned int i;

    for (i = 0; i < BINARY_FORMAT_SLOTS; i++)
    {

        if (!formats[i])
            continue;

        if (formats[i]->match(interface, id))
            return formats[i];

    };

    return 0;


}

unsigned int binary_find_symbol(struct vfs_interface *interface, unsigned int id, unsigned int count, const char *symbol)
{

    struct binary_format *format = get_format(interface, id);

    if (!format)
        return 0;

    return format->find_symbol(interface, id, count, symbol);

}

unsigned int binary_copy_program(struct vfs_interface *interface, unsigned int id)
{

    struct binary_format *format = get_format(interface, id);

    if (!format)
        return 0;

    return format->copy_program(interface, id);

}

unsigned int binary_relocate(struct vfs_interface *interface, unsigned int id, unsigned int address)
{

    struct binary_format *format = get_format(interface, id);

    if (!format)
        return 0;

    return format->relocate(interface, id, address);

}

void binary_register_format(struct binary_format *format)
{

    unsigned int i;

    for (i = 0; i < BINARY_FORMAT_SLOTS; i++)
    {

        if (formats[i])
            continue;

        formats[i] = format;

        return;

    }

}

void binary_unregister_format(struct binary_format *format)
{

    unsigned int i;

    for (i = 0; i < BINARY_FORMAT_SLOTS; i++)
    {

        if (formats[i] != format)
            continue;

        formats[i] = 0;

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

    memory_clear(formats, sizeof (struct binary_format *) * BINARY_FORMAT_SLOTS);

}

