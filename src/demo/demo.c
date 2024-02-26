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

    while (channel_pick(&message, data))
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

    if (!call_walk_absolute(FILE_L0, option_getstring("keyboard")))
        PANIC();

    if (!call_walk_relative(FILE_G0, FILE_L0, "event"))
        PANIC();

    if (!call_walk_absolute(FILE_L0, option_getstring("timer")))
        PANIC();

    if (!call_walk_relative(FILE_G1, FILE_L0, "event1"))
        PANIC();

    if (!call_walk_absolute(FILE_L0, option_getstring("video")))
        PANIC();

    if (!call_walk_relative(FILE_G2, FILE_L0, "event"))
        PANIC();

    if (!call_walk_relative(FILE_G3, FILE_L0, "ctrl"))
        PANIC();

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

    struct ctrl_videosettings settings;

    settings.width = option_getdecimal("width");
    settings.height = option_getdecimal("height");
    settings.bpp = option_getdecimal("bpp");

    channel_send(option_getdecimal("wm-service"), EVENT_WMGRAB);
    channel_wait_any(EVENT_WMACK);
    call_link(FILE_G0, 8000);
    call_link(FILE_G1, 8001);
    call_link(FILE_G2, 8002);
    call_notify(FILE_G3, EVENT_CONFIG, sizeof (struct ctrl_videosettings), &settings);
    run();
    call_unlink(FILE_G2);
    call_unlink(FILE_G1);
    call_unlink(FILE_G0);
    channel_send(option_getdecimal("wm-service"), EVENT_WMUNGRAB);
    channel_wait_any(EVENT_WMACK);

}

void init(void)
{

    option_add("width", "640");
    option_add("height", "480");
    option_add("bpp", "4");
    option_add("keyboard", "system:keyboard");
    option_add("timer", "system:timer/if.0");
    option_add("video", "system:video/if.0");
    option_add("wm-service", "12345");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_VIDEOMODE, onvideomode);
    channel_bind(EVENT_WMINIT, onwminit);

}

