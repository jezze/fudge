#ifndef MODULES_VIDEO_H
#define MODULES_VIDEO_H

struct video_interface
{

    struct base_driver *driver;
    unsigned int bpp;
    unsigned int xres;
    unsigned int yres;
    void (*enable)(struct video_interface *self);
    unsigned int (*read_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct video_protocol
{

    char *name;

};

struct video_filesystem
{

    struct vfs_interface base;
    struct video_interface *interfaces[8];
    unsigned int interfacesCount;
    struct video_protocol *protocols[16];
    unsigned int protocolsCount;

};

void video_register_interface(struct video_interface *interface);
void video_register_protocol(struct video_protocol *protocol);
void video_unregister_interface(struct video_interface *interface);
void video_unregister_protocol(struct video_protocol *protocol);
void video_init_filesystem(struct video_filesystem *filesystem);
void video_init_interface(struct video_interface *interface, struct base_driver *driver, void (*enable)(struct video_interface *self), unsigned int (*read_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer));

#endif

