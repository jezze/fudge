#ifndef MODULES_VIDEO_H
#define MODULES_VIDEO_H

struct video_interface
{

    struct modules_driver *driver;
    unsigned int (*read)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct video_protocol
{

    char *name;

};

struct video_filesystem
{

    struct vfs_filesystem base;
    struct video_interface *interfaces[8];
    unsigned int interfacesCount;
    struct video_protocol *protocols[16];
    unsigned int protocolsCount;
    void (*register_interface)(struct video_filesystem *self, struct video_interface *interface, struct modules_driver *driver, unsigned int (*read)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer));
    void (*register_protocol)(struct video_filesystem *self, struct video_protocol *protocol, char *name);
    void (*unregister_interface)(struct video_filesystem *self, struct video_interface *interface);
    void (*unregister_protocol)(struct video_filesystem *self, struct video_protocol *protocol);

};

void video_register_interface(struct video_interface *interface, struct modules_driver *driver, unsigned int (*read)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer));
void video_register_protocol(struct video_protocol *protocol, char *name);
void video_unregister_interface(struct video_interface *interface);
void video_unregister_protocol(struct video_protocol *protocol);
void video_interface_init(struct video_interface *interface);
void video_filesystem_init(struct video_filesystem *filesystem);

#endif

