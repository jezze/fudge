#ifndef MODULES_VIDEO_H
#define MODULES_VIDEO_H

struct video_interface
{

    struct modules_driver *driver;
    unsigned int (*read_bpp)(struct video_interface *self);
    unsigned int (*read_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*read_xres)(struct video_interface *self);
    unsigned int (*read_yres)(struct video_interface *self);
    void (*write_bpp)(struct video_interface *self, unsigned int bpp);
    unsigned int (*write_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer);
    void (*write_xres)(struct video_interface *self, unsigned int xres);
    void (*write_yres)(struct video_interface *self, unsigned int yres);

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

void video_register_interface(struct video_interface *interface, struct modules_driver *driver, unsigned int (*read_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_bpp)(struct video_interface *self), void (*write_bpp)(struct video_interface *self, unsigned int bpp), unsigned int (*read_xres)(struct video_interface *self), void (*write_xres)(struct video_interface *self, unsigned int xres), unsigned int (*read_yres)(struct video_interface *self), void (*write_yres)(struct video_interface *self, unsigned int yres));
void video_register_protocol(struct video_protocol *protocol, char *name);
void video_unregister_interface(struct video_interface *interface);
void video_unregister_protocol(struct video_protocol *protocol);
void video_interface_init(struct video_interface *interface);
void video_filesystem_init(struct video_filesystem *filesystem);

#endif

