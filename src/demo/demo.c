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

    while (channel_pick(&message, MESSAGE_SIZE, data))
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
            channel_dispatch(&message, data);

            break;

        }

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(option_getdecimal("wm-service"), EVENT_WMMAP);

    while (channel_process());

    channel_send(option_getdecimal("wm-service"), EVENT_WMUNMAP);

}

static void onvideomode(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videomode *videomode = mdata;

    framebuffer = videomode->framebuffer;
    wmax = videomode->w;
    hmax = videomode->h;
    wmid = wmax / 2;
    hmid = hmax / 2;

}

static void onwminit(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int videoservice = fsp_auth(option_getstring("video"));
    unsigned int videoctrl = fsp_walk(videoservice, fsp_walk(videoservice, 0, option_getstring("video")), "ctrl");
    unsigned int videoevent = fsp_walk(videoservice, fsp_walk(videoservice, 0, option_getstring("video")), "event");
    struct ctrl_videosettings settings;

    settings.width = option_getdecimal("width");
    settings.height = option_getdecimal("height");
    settings.bpp = option_getdecimal("bpp");

    channel_send(option_getdecimal("wm-service"), EVENT_WMGRAB);
    channel_wait(EVENT_WMACK);
    channel_send(option_getdecimal("keyboard-service"), EVENT_LINK);
    channel_send(option_getdecimal("timer-service"), EVENT_LINK);
    fsp_link(videoservice, videoevent);
    fsp_write(videoservice, videoctrl, &settings, sizeof (struct ctrl_videosettings), 0);
    run();
    fsp_unlink(videoservice, videoevent);
    channel_send(option_getdecimal("timer-service"), EVENT_UNLINK);
    channel_send(option_getdecimal("keyboard-service"), EVENT_UNLINK);
    channel_send(option_getdecimal("wm-service"), EVENT_WMUNGRAB);
    channel_wait(EVENT_WMACK);

}

void init(void)
{

    option_add("width", "640");
    option_add("height", "480");
    option_add("bpp", "4");
    option_add("keyboard-service", "110");
    option_add("timer-service", "410");
    option_add("video", "system:video/if.0");
    option_add("wm-service", "12345");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_VIDEOMODE, onvideomode);
    channel_bind(EVENT_WMINIT, onwminit);

}

