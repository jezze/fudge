#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/stream/stream.h>

void stream_device_init(struct stream_device *device, char *name, void *read, void *write)
{

    modules_device_init(&device->base, STREAM_DEVICE_TYPE);
    vfs_node_init(&device->node, 0, 0, 0, read, write);
    string_write(device->name, name);

}

