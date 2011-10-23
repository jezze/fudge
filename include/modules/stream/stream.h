#ifndef MODULES_STREAM_H
#define MODULES_STREAM_H

#define STREAM_DEVICE_TYPE 4000 

struct stream_device
{

    struct modules_device base;
    char name[128];
    struct vfs_node node;

};

extern void stream_init(struct stream_device *self, char *name);

#endif

