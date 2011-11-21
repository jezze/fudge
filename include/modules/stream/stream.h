#ifndef MODULES_STREAM_H
#define MODULES_STREAM_H

#define STREAM_DEVICE_TYPE 4000 

struct stream_device
{

    struct modules_device base;
    struct vfs_node node;

};

extern void stream_device_init(struct stream_device *device, char *name, void *read, void *write);

#endif

