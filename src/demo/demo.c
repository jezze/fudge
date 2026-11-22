#include <fudge.h>
#include <math.h>
#include <abi.h>
#include "types.h"
#include "vector2.h"
#include "vector3.h"
#include "matrix3x3.h"
#include "scene/plasma.h"
#include "scene/cube.h"

unsigned int *framebuffer;
unsigned int wmax, hmax, wmid, hmid;
static unsigned int framecount;

unsigned int inperiod(unsigned int frame, unsigned int start, unsigned int end)
{

    return (frame >= start && frame < end);

}

static struct scene scenelist[] = {
    {1, 0, 0, 400, plasma_setup, plasma_render},
    {1, 0, 0, 1000, cube_setup, cube_render}
};
static unsigned int totaltime;
static unsigned int loopframe;

static void setup(void)
{

    unsigned int i;

    for (i = 0; i < 2; i++)
    {

        struct scene *scene = &scenelist[i];

        if (!scene->used)
            continue;

        scene->framestart = totaltime;
        scene->framestop = scene->framestart + scene->duration;
        scene->setup(scene);

        totaltime += scene->duration;

    }

}

static void render(unsigned int frame)
{

    unsigned int i;

    loopframe++;

    if (loopframe > totaltime)
        loopframe = 0;

    for (i = 0; i < 2; i++)
    {

        struct scene *scene = &scenelist[i];

        if (!scene->used)
            continue;

        if (loopframe >= scene->framestart && loopframe < scene->framestop)
            scene->render(scene, loopframe - scene->framestart, loopframe, frame);

    }

}

static void ontimertick(struct message *message)
{

    if (framebuffer)
        render(framecount++);

}

static void ungrab(void)
{

    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        channel_send(1, wm, MESSAGE_TYPE_WMUNGRAB, 0, 0);
        channel_wait(1, wm, MESSAGE_TYPE_WMACK, 0, 0);

    }

}

static void onmain(struct message *message)
{

    unsigned int keyboard = channel_lookup(option_getstring("keyboard-service"));
    unsigned int timer = channel_lookup(option_getstring("timer-service"));
    unsigned int video = channel_lookup(option_getstring("video-service"));
    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        channel_send(1, wm, MESSAGE_TYPE_WMGRAB, 0, 0);
        channel_wait(1, wm, MESSAGE_TYPE_WMACK, 0, 0);

    }

    if (keyboard && timer && video)
    {

        struct message_videoconf videoconf;

        videoconf.width = option_getdecimal("width");
        videoconf.height = option_getdecimal("height");
        videoconf.bpp = option_getdecimal("bpp");

        channel_send(0, video, MESSAGE_TYPE_VIDEOCONF, sizeof (struct message_videoconf), &videoconf);
        channel_send(1, video, MESSAGE_TYPE_INFO, 0, 0);
        channel_wait(1, video, MESSAGE_TYPE_VIDEOINFO, 0, 0);
        channel_send(0, keyboard, MESSAGE_TYPE_LINK, 0, 0);
        channel_send(0, timer, MESSAGE_TYPE_LINK, 0, 0);
        channel_hold(0);
        channel_send(0, keyboard, MESSAGE_TYPE_UNLINK, 0, 0);
        channel_send(0, timer, MESSAGE_TYPE_UNLINK, 0, 0);

    }

    ungrab();

}

static void onvideoinfo(struct message *message)
{

    struct message_videoinfo *videoinfo = message_validate(MESSAGE_TYPE_VIDEOINFO, message->data, message->length);

    if (videoinfo)
    {

        framebuffer = (unsigned int *)videoinfo->framebuffer;
        wmax = videoinfo->width;
        hmax = videoinfo->height;
        wmid = wmax / 2;
        hmid = hmax / 2;

    }

}

void init(void)
{

    setup();
    option_add("width", "640");
    option_add("height", "480");
    option_add("bpp", "4");
    option_add("keyboard-service", "keyboard:0");
    option_add("timer-service", "timer:0");
    option_add("video-service", "video:0");
    option_add("wm-service", "wm");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_TIMERTICK, ontimertick);
    channel_bind(MESSAGE_TYPE_VIDEOINFO, onvideoinfo);

}

