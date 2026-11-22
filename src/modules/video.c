#include <fudge.h>
#include <kernel.h>
#include "video.h"

#define PREFERREDWIDTH                  1920
#define PREFERREDHEIGHT                 1080
#define MODES                           14

static struct unitsrv unitsrv;
static struct message_videoconf modes[MODES] = {
    {320, 200, 4},
    {320, 240, 4},
    {640, 480, 4},
    {800, 600, 4},
    {1024, 768, 4},
    {1280, 720, 4},
    {1280, 1024, 4},
    {1366, 768, 4},
    {1600, 900, 4},
    {1920, 1080, 4},
    {1920, 1200, 4},
    {2560, 1440, 4},
    {2560, 1600, 4},
    {3840, 2160, 4}
};

static unsigned int oninfo(struct video_interface *interface, unsigned int source)
{

    if (interface->framebuffer)
    {

        struct message_videoinfo videoinfo;

        videoinfo.framebuffer = interface->framebuffer;
        videoinfo.width = interface->width;
        videoinfo.height = interface->height;
        videoinfo.bpp = interface->bpp;
        videoinfo.connected = interface->connected;

        return kernel_placereply(source, interface->resource.inode, MESSAGE_TYPE_VIDEOINFO, sizeof (struct message_videoinfo), &videoinfo);

    }

    return MESSAGE_FAILED;

}

static void addmode(struct message_videomodes *videomodes, unsigned int width, unsigned int height, unsigned int bpp)
{

    unsigned int i;
    unsigned int j;

    if (!width || !height || videomodes->count >= MESSAGE_VIDEOMODES)
        return;

    for (i = 0; i < videomodes->count; i++)
    {

        struct message_videoconf *mode = &videomodes->modes[i];

        if (mode->width == width && mode->height == height && mode->bpp == bpp)
            return;

        if (mode->width > width || (mode->width == width && mode->height > height))
            break;

    }

    for (j = videomodes->count; j > i; j--)
        buffer_copy(&videomodes->modes[j], &videomodes->modes[j - 1], sizeof (struct message_videoconf));

    videomodes->modes[i].width = width;
    videomodes->modes[i].height = height;
    videomodes->modes[i].bpp = bpp;
    videomodes->count++;

}

static unsigned int onvideomoderequest(struct video_interface *interface, unsigned int source)
{

    struct message_videomodes videomodes;
    unsigned int i;

    buffer_clear(&videomodes, sizeof (struct message_videomodes));

    if (interface->checkmode)
    {

        for (i = 0; i < MODES; i++)
        {

            if (interface->checkmode(modes[i].width, modes[i].height, modes[i].bpp))
                addmode(&videomodes, modes[i].width, modes[i].height, modes[i].bpp);

        }

        if (interface->checkmode(interface->preferredwidth, interface->preferredheight, 4))
            addmode(&videomodes, interface->preferredwidth, interface->preferredheight, 4);

    }

    addmode(&videomodes, interface->width, interface->height, interface->bpp);

    return kernel_placereply(source, interface->resource.inode, MESSAGE_TYPE_VIDEOMODERESPONSE, sizeof (struct message_videomodes), &videomodes);

}

static unsigned int onvideocmap(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->setcmap)
    {

        interface->setcmap(count / 3, data);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int onvideoconf(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct message_videoconf *videoconf = message_validate(MESSAGE_TYPE_VIDEOCONF, data, count);

    if (videoconf && !videoconf->bpp)
    {

        if (interface->disable)
            interface->disable();

        return MESSAGE_OK;

    }

    if (interface->setmode && videoconf)
    {

        if (videoconf->width && videoconf->height)
            interface->setmode(videoconf->width, videoconf->height, videoconf->bpp);
        else
            interface->setmode(interface->preferredwidth, interface->preferredheight, videoconf->bpp);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int onvideoflush(struct video_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct message_videoflush *videoflush = message_validate(MESSAGE_TYPE_VIDEOFLUSH, data, count);

    if (interface->flush && videoflush)
    {

        interface->flush(videoflush->x, videoflush->y, videoflush->width, videoflush->height);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct video_interface *interface = resource->data;

        if (interface->detect && (event == MESSAGE_TYPE_INFO || event == MESSAGE_TYPE_VIDEOCONF || event == MESSAGE_TYPE_VIDEOMODEREQUEST))
            interface->detect();

        switch (event)
        {

        case MESSAGE_TYPE_INFO:
            return oninfo(interface, source);

        case MESSAGE_TYPE_VIDEOCMAP:
            return onvideocmap(interface, source, count, data);

        case MESSAGE_TYPE_VIDEOCONF:
            return onvideoconf(interface, source, count, data);

        case MESSAGE_TYPE_VIDEOFLUSH:
            return onvideoflush(interface, source, count, data);

        case MESSAGE_TYPE_VIDEOMODEREQUEST:
            return onvideomoderequest(interface, source);

        }

    }

    return MESSAGE_FAILED;

}

void video_registerinterface(struct video_interface *interface)
{

    resource_register(&interface->resource);

}

void video_unregisterinterface(struct video_interface *interface)
{

    resource_unregister(&interface->resource);

}

void video_initinterface(struct video_interface *interface, unsigned long framebuffer, unsigned int (*setcmap)(unsigned int count, void *data), void (*setmode)(unsigned int width, unsigned int height, unsigned int bpp))
{

    resource_init(&interface->resource, RESOURCE_VIDEOINTERFACE, interface, operands_place);

    interface->width = 0;
    interface->height = 0;
    interface->bpp = 0;
    interface->connected = 1;
    interface->preferredwidth = PREFERREDWIDTH;
    interface->preferredheight = PREFERREDHEIGHT;
    interface->framebuffer = framebuffer;
    interface->setcmap = setcmap;
    interface->setmode = setmode;
    interface->flush = 0;
    interface->checkmode = 0;
    interface->detect = 0;
    interface->disable = 0;

}

void module_init(void)
{

    unitsrv_init(&unitsrv, RESOURCE_VIDEOINTERFACE, "video");

}

void module_register(void)
{

    unitsrv_register(&unitsrv);

}

void module_unregister(void)
{

    unitsrv_unregister(&unitsrv);

}

