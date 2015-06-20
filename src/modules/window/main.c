#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/video/video.h>

static struct system_node root;
static struct system_node data;
static struct system_node ctrl;
static void *gaddress = (void *)0x000A0000;

static struct video_interface *findinterface()
{

    struct resource *resource = resource_findtype(0, RESOURCE_VIDEOINTERFACE);

    return (resource) ? resource->data : 0;

}

static unsigned int ctrl_read(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct video_interface *interface = findinterface();
    struct ctrl_videosettings *settings = buffer;

    if (!interface)
        return 0;

    if (size == sizeof (struct ctrl_header))
    {

        settings->header.type = CTRL_TYPE_VIDEO;

        return 1;

    }

    if (size == sizeof (struct ctrl_videosettings))
    {

        settings->header.type = CTRL_TYPE_VIDEO;
        settings->w = interface->w;
        settings->h = interface->h / 2;
        settings->bpp = interface->bpp;

        return 1;

    }

    return 0;

}

static unsigned int ctrl_write(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct video_interface *interface = findinterface();
    struct ctrl_videosettings *settings = buffer;

    if (!interface)
        return 0;

    if (size == sizeof (struct ctrl_videosettings))
    {

        interface->setmode(settings);

        return 1;

    }

    return 0;

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    return memory_write((unsigned char *)gaddress + 320 * 100, 320 * 200, buffer, count, size, offset);

}

void module_init()
{

    system_initnode(&data, SYSTEM_NODETYPE_NORMAL, "data");

    data.write = data_write;

    system_initnode(&ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");

    ctrl.read = ctrl_read;
    ctrl.write = ctrl_write;

    system_initnode(&root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "window");
    system_addchild(&root, &data);
    system_addchild(&root, &ctrl);

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

