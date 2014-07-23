#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "video.h"

static struct interface_node
{

    struct system_group base;
    struct base_video_interface *interface;
    struct base_bus *bus;
    unsigned int id;
    struct system_stream data;
    struct system_stream colormap;
    struct system_stream info;
    struct system_stream mode;

} inode[8];

static struct system_group root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;

    return node->interface->read_data(node->bus, node->id, offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;

    return node->interface->write_data(node->bus, node->id, offset, count, buffer);

}

static unsigned int colormap_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;

    return node->interface->read_colormap(node->bus, node->id, offset, count, buffer);

}

static unsigned int colormap_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;

    return node->interface->write_colormap(node->bus, node->id, offset, count, buffer);

}

static unsigned int info_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int mode_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;

    node->interface->xres = 320;
    node->interface->yres = 200;
    node->interface->bpp = 8;

    node->interface->mode(node->bus, node->id);

    return count;

}

static unsigned int find_inode()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!inode[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_inode(struct interface_node *node, struct base_video_interface *interface, struct base_bus *bus, unsigned int id)
{

    memory_clear(node, sizeof (struct interface_node));
    system_init_group(&node->base, bus->name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->colormap, "colormap");
    system_init_stream(&node->info, "info");
    system_init_stream(&node->mode, "mode");

    node->interface = interface;
    node->bus = bus;
    node->id = id;
    node->data.node.read = data_read;
    node->data.node.write = data_write;
    node->colormap.node.read = colormap_read;
    node->colormap.node.write = colormap_write;
    node->info.node.read = info_read;
    node->mode.node.write = mode_write;

}

void base_video_register_interface(struct base_video_interface *interface, struct base_bus *bus, unsigned int id)
{

    unsigned int index = find_inode();

    if (!index)
        return;

    init_inode(&inode[index], interface, bus, id);
    system_group_add(&root, &inode[index].base.node);
    system_group_add(&inode[index].base, &inode[index].data.node);
    system_group_add(&inode[index].base, &inode[index].colormap.node);
    system_group_add(&inode[index].base, &inode[index].info.node);
    system_group_add(&inode[index].base, &inode[index].mode.node);

}

void base_video_unregister_interface(struct base_video_interface *interface)
{

}

void base_video_init_interface(struct base_video_interface *interface, void (*mode)(struct base_bus *bus, unsigned int id), unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_colormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_colormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_video_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_OTHER);

    interface->mode = mode;
    interface->read_data = read_data;
    interface->write_data = write_data;
    interface->read_colormap = read_colormap;
    interface->write_colormap = write_colormap;

}

void base_video_setup()
{

    memory_clear(inode, sizeof (struct interface_node) * 8);
    system_init_group(&root, "video");
    system_register_node(&root.node);

}

