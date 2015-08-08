#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/video/video.h>

static struct system_node root;
static struct system_node data;
static struct system_node ctrl;

static struct video_interface *findinterface(void)
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

    count = interface->ctrl.read(&interface->ctrl, offset, size, count, buffer);

    if (count && size == sizeof (struct ctrl_videosettings))
        settings->h = settings->h / 2;

    return count;

}

static unsigned int ctrl_write(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct video_interface *interface = findinterface();

    return interface->ctrl.write(&interface->ctrl, offset, size, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct video_interface *interface = findinterface();

    return interface->data.write(&interface->data, offset + 320 * 100, size, count, buffer);

}

void module_init(void)
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

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

