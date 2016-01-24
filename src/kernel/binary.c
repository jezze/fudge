#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "vfs.h"
#include "binary.h"

struct binary_format *binary_findformat(struct vfs_channel *channel, struct task *task, unsigned int descriptor, unsigned int id)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_BINARYFORMAT)))
    {

        struct binary_format *format = current->data;

        if (format->match(channel, task, descriptor, id))
            return format;

    }

    return 0;

}

void binary_initformat(struct binary_format *format, unsigned int (*match)(struct vfs_channel *channel, struct task *task, unsigned int descriptor, unsigned int id), unsigned long (*findsymbol)(struct vfs_channel *channel, struct task *task, unsigned int descriptor, unsigned int id, unsigned int count, char *symbol), unsigned long (*findentry)(struct vfs_channel *channel, struct task *task, unsigned int descriptor, unsigned int id), unsigned long (*findbase)(struct vfs_channel *channel, struct task *task, unsigned int descriptor, unsigned int id, unsigned long address), unsigned int (*copyprogram)(struct vfs_channel *channel, struct task *task, unsigned int descriptor, unsigned int id), unsigned int (*relocate)(struct vfs_channel *channel, struct task *task, unsigned int descriptor, unsigned int id, unsigned int address))
{

    resource_init(&format->resource, RESOURCE_BINARYFORMAT, format);

    format->match = match;
    format->findsymbol = findsymbol;
    format->findentry = findentry;
    format->findbase = findbase;
    format->copyprogram = copyprogram;
    format->relocate = relocate;

}

