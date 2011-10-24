#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/stream/stream.h>

void stream_device_init(struct stream_device *device, char *name, void *read, void *write)
{

    device->base.module.type = MODULES_TYPE_DEVICE;
    device->base.type = STREAM_DEVICE_TYPE;
    device->node.operations.read = read;
    device->node.operations.write = write;
    string_copy(device->name, name);

}

