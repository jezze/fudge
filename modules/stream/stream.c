#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/stream/stream.h>

void stream_init(struct stream_device *self, char *name)
{

    string_copy(self->base.name, name);
    self->base.module.type = MODULES_TYPE_DEVICE;
    self->base.type = STREAM_DEVICE_TYPE;
    string_copy(self->name, name);

}

