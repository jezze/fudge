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

static void run(void)
{

    struct message message;
    char data[MESSAGE_SIZE];
    unsigned int frame = 0;

    setup();

    while (channel_pick(0, &message, MESSAGE_SIZE, data))
    {

        switch (message.event)
        {

        case EVENT_TIMERTICK:
            if (framebuffer)
                render(frame++);

            break;

        case EVENT_KEYPRESS:
            break;

        case EVENT_KEYRELEASE:
            break;

        default:
            channel_dispatch(0, &message, data);

            break;

        }

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    option_setdecimal("wm-service", lookup(option_getstring("wm-service")));
    channel_send(0, option_getdecimal("wm-service"), EVENT_WMGRAB);
    channel_wait(0, option_getdecimal("wm-service"), EVENT_WMACK);
    channel_send(0, option_getdecimal("wm-service"), EVENT_WMMAP);

    while (channel_process(0));

    channel_send(0, option_getdecimal("wm-service"), EVENT_WMUNMAP);
    channel_send(0, option_getdecimal("wm-service"), EVENT_WMUNGRAB);
    channel_wait(0, option_getdecimal("wm-service"), EVENT_WMACK);

}

static void onvideoinfo(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videoinfo *videoinfo = mdata;

    framebuffer = videoinfo->framebuffer;
    wmax = videoinfo->width;
    hmax = videoinfo->height;
    wmid = wmax / 2;
    hmid = hmax / 2;

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videoconf videoconf;

    videoconf.width = option_getdecimal("width");
    videoconf.height = option_getdecimal("height");
    videoconf.bpp = option_getdecimal("bpp");

    option_setdecimal("keyboard-service", lookup(option_getstring("keyboard-service")));
    option_setdecimal("timer-service", lookup(option_getstring("timer-service")));
    option_setdecimal("video-service", lookup(option_getstring("video-service")));
    channel_send(0, option_getdecimal("keyboard-service"), EVENT_LINK);
    channel_send(0, option_getdecimal("timer-service"), EVENT_LINK);
    channel_send(0, option_getdecimal("video-service"), EVENT_LINK);
    channel_send_buffer(0, option_getdecimal("video-service"), EVENT_VIDEOCONF, sizeof (struct event_videoconf), &videoconf);
    channel_wait(0, option_getdecimal("video-service"), EVENT_VIDEOINFO);
    run();
    channel_send(0, option_getdecimal("video-service"), EVENT_UNLINK);
    channel_send(0, option_getdecimal("timer-service"), EVENT_UNLINK);
    channel_send(0, option_getdecimal("keyboard-service"), EVENT_UNLINK);

}

void init(void)
{

    option_add("width", "640");
    option_add("height", "480");
    option_add("bpp", "4");
    option_add("keyboard-service", "keyboard:0");
    option_add("timer-service", "timer0");
    option_add("video-service", "video:1");
    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_VIDEOINFO, onvideoinfo);
    channel_bind(EVENT_WMINIT, onwminit);

}

